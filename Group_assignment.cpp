#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <map>
#include <algorithm>
#include <limits>
#include <conio.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <filesystem>

class BaseEntity {
protected:
    std::string id;
    time_t createdAt;
    time_t updatedAt;

    void updateTimestamp() {
        updatedAt = std::time(nullptr);
    }

public:
    BaseEntity() {
        createdAt = std::time(nullptr);
        updatedAt = createdAt;
        id = std::to_string(createdAt);
    }

    virtual ~BaseEntity() = default;

    const std::string& getId() const { return id; }
    time_t getCreatedAt() const { return createdAt; }
    time_t getUpdatedAt() const { return updatedAt; }
};

class SimpleEncryption {
public:
    static std::string encrypt(const std::string& input, const std::string& key) {
        std::string encrypted = input;
        for (size_t i = 0; i < input.length(); ++i) {
            encrypted[i] = input[i] ^ key[i % key.length()];
        }
        return encrypted;
    }

    static std::string decrypt(const std::string& input, const std::string& key) {
        return encrypt(input, key);
    }
};

enum class SpendingCategory {
    FOOD,
    TRANSPORT,
    HOUSING,
    ENTERTAINMENT,
    UTILITIES,
    HEALTHCARE,
    EDUCATION,
    MISCELLANEOUS
};

std::string getCategoryString(SpendingCategory category) {
    switch(category) {
        case SpendingCategory::FOOD: return "Food";
        case SpendingCategory::TRANSPORT: return "Transport";
        case SpendingCategory::HOUSING: return "Housing";
        case SpendingCategory::ENTERTAINMENT: return "Entertainment";
        case SpendingCategory::UTILITIES: return "Utilities";
        case SpendingCategory::HEALTHCARE: return "Healthcare";
        case SpendingCategory::EDUCATION: return "Education";
        case SpendingCategory::MISCELLANEOUS: return "Miscellaneous";
        default: return "Unknown";
    }
}

class FileManager {
public:
    static void saveToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << content;
            file.close();
        } else {
            throw std::runtime_error("Unable to open file: " + filename);
        }
    }

    static std::string readFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            return buffer.str();
        }
        return "";
    }

    static void createDirectory(const std::string& path) {
        std::filesystem::create_directories(path);
    }
};

class Transaction : public BaseEntity {
private:
    double amount;
    SpendingCategory category;
    std::string description;
    time_t transactionDate;

public:
    Transaction(double amt, SpendingCategory cat, const std::string& desc = "")
        : BaseEntity(), amount(amt), category(cat), description(desc) 
        { transactionDate = std::time(nullptr); }

    std::string serialize() const {
        std::stringstream ss;
        ss << id << "," << amount << "," << static_cast<int>(category) << ","
           << description << "," << transactionDate << "," << createdAt << "," << updatedAt;
        return ss.str();
    }

    static Transaction deserialize(const std::string& data) {
        std::stringstream ss(data);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 7) {
            Transaction trans(std::stod(tokens[1]), 
                            static_cast<SpendingCategory>(std::stoi(tokens[2])), 
                            tokens[3]);
            trans.id = tokens[0];
            trans.transactionDate = std::stoll(tokens[4]);
            trans.createdAt = std::stoll(tokens[5]);
            trans.updatedAt = std::stoll(tokens[6]);
            return trans;
        }
        throw std::runtime_error("Invalid transaction data format");
    }

    double getAmount() const { return amount; }
    SpendingCategory getCategory() const { return category; }
    std::string getDescription() const { return description; }
    time_t getDate() const { return transactionDate; }
};

class BudgetLimit {
public:
    SpendingCategory category;
    double limit;
    
    BudgetLimit(SpendingCategory cat, double lim) : category(cat), limit(lim) {}

    std::string serialize() const {
        return std::to_string(static_cast<int>(category)) + "," + std::to_string(limit);
    }

    static BudgetLimit deserialize(const std::string& data) {
        std::stringstream ss(data);
        std::string categoryStr, limitStr;
        std::getline(ss, categoryStr, ',');
        std::getline(ss, limitStr, ',');
        return BudgetLimit(
            static_cast<SpendingCategory>(std::stoi(categoryStr)),
            std::stod(limitStr)
        );
    }
};

class Account : public BaseEntity {
private:
    std::string name;
    double balance;
    std::vector<Transaction> transactions;
    double monthlyBudget;
    std::vector<BudgetLimit> categoryBudgets;
    std::string dataPath;

    void saveTransactions() {
        std::string filename = dataPath + "/transactions_" + id + ".txt";
        std::stringstream ss;
        for (const auto& trans : transactions) {
            ss << trans.serialize() << "\n";
        }
        FileManager::saveToFile(filename, ss.str());
    }

    void loadTransactions() {
        std::string filename = dataPath + "/transactions_" + id + ".txt";
        std::string content = FileManager::readFromFile(filename);
        std::stringstream ss(content);
        std::string line;
        
        transactions.clear();
        while (std::getline(ss, line)) {
            if (!line.empty()) {
                transactions.push_back(Transaction::deserialize(line));
            }
        }
    }

    void saveBudgetLimits() {
        std::string filename = dataPath + "/budgets_" + id + ".txt";
        std::stringstream ss;
        for (const auto& budget : categoryBudgets) {
            ss << budget.serialize() << "\n";
        }
        FileManager::saveToFile(filename, ss.str());
    }

    void loadBudgetLimits() {
        std::string filename = dataPath + "/budgets_" + id + ".txt";
        std::string content = FileManager::readFromFile(filename);
        std::stringstream ss(content);
        std::string line;
        
        categoryBudgets.clear();
        while (std::getline(ss, line)) {
            if (!line.empty()) {
                categoryBudgets.push_back(BudgetLimit::deserialize(line));
            }
        }
    }

public:
    Account(const std::string& accountName, double initialBalance = 0.0, double budget = 0.0)
        : BaseEntity(), name(accountName), balance(initialBalance), monthlyBudget(budget) {
        dataPath = "data/accounts/" + id;
        FileManager::createDirectory(dataPath);
        saveTransactions();
        saveBudgetLimits();
    }

    std::vector<Transaction>& getTransactions() {
        loadTransactions();
        return transactions;
    }

    void addTransaction(const Transaction& transaction) {
        transactions.push_back(transaction);
        balance -= transaction.getAmount();
        updateTimestamp();
        saveTransactions();
    }

    void editTransaction(const std::string& transId, double newAmount, 
                        SpendingCategory newCategory, const std::string& newDescription) {
        for (auto& trans : transactions) {
            if (trans.getId() == transId) {
                balance += trans.getAmount();
                
                Transaction newTrans(newAmount, newCategory, newDescription);
                transactions.erase(
                    std::remove_if(transactions.begin(), transactions.end(),
                                 [&transId](const Transaction& t) { 
                                     return t.getId() == transId; 
                                 }),
                    transactions.end()
                );
                transactions.push_back(newTrans);
                
                balance -= newAmount;
                updateTimestamp();
                saveTransactions();
                return;
            }
        }
        throw std::runtime_error("Transaction not found");
    }

    void deleteTransaction(const std::string& transId) {
        for (const auto& trans : transactions) {
            if (trans.getId() == transId) {
                balance += trans.getAmount();
                transactions.erase(
                    std::remove_if(transactions.begin(), transactions.end(),
                                 [&transId](const Transaction& t) { 
                                     return t.getId() == transId; 
                                 }),
                    transactions.end()
                );
                updateTimestamp();
                saveTransactions();
                return;
            }
        }
        throw std::runtime_error("Transaction not found");
    }

    void setCategoryBudget(SpendingCategory category, double limit) {
        auto it = std::find_if(categoryBudgets.begin(), categoryBudgets.end(),
                              [category](const BudgetLimit& budget) {
                                  return budget.category == category;
                              });
        
        if (it != categoryBudgets.end()) {
            it->limit = limit;
        } else {
            categoryBudgets.push_back(BudgetLimit(category, limit));
        }
        saveBudgetLimits();
    }

    void deleteCategoryBudget(SpendingCategory category) {
        categoryBudgets.erase(
            std::remove_if(categoryBudgets.begin(), categoryBudgets.end(),
                          [category](const BudgetLimit& budget) {
                              return budget.category == category;
                          }),
            categoryBudgets.end()
        );
        saveBudgetLimits();
    }

    std::vector<BudgetLimit> getCategoryBudgets() const {
        return categoryBudgets;
    }

    bool isCategoryOverBudget(SpendingCategory category) const {
        auto it = std::find_if(categoryBudgets.begin(), categoryBudgets.end(),
                              [category](const BudgetLimit& budget) {
                                  return budget.category == category;
                              });
        
        if (it != categoryBudgets.end()) {
            double categorySpending = 0.0;
            time_t now = std::time(nullptr);
            std::tm* nowTm = std::localtime(&now);
            
            for (const auto& trans : transactions) {
                if (trans.getCategory() == category) {
                    time_t transTime = trans.getDate();
                    std::tm* transTm = std::localtime(&transTime);
                    
                    if (transTm && nowTm && 
                        transTm->tm_mon == nowTm->tm_mon && 
                        transTm->tm_year == nowTm->tm_year) {
                        categorySpending += trans.getAmount();
                    }
                }
            }
            return categorySpending > it->limit;
        }
        return false;
    }

    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            updateTimestamp();
        }
    }

    void setMonthlyBudget(double budget) {
        monthlyBudget = budget;
        updateTimestamp();
    }

    std::string getName() const { return name; }
    double getBalance() const { return balance; }

    double getMonthlyBudget() const {
        return monthlyBudget;
    }

    std::map<SpendingCategory, double> getCategorySpending() const {
        std::map<SpendingCategory, double> categorySpending;
        for (const auto& transaction : transactions) {
            categorySpending[transaction.getCategory()] += transaction.getAmount();
        }
        return categorySpending;
    }

    double getTotalMonthlySpending() const {
        double total = 0.0;
        time_t now = std::time(nullptr);
        std::tm* nowTm = std::localtime(&now);
        
        for (const auto& transaction : transactions) {
            time_t transTime = transaction.getDate();
            std::tm* transTm = std::localtime(&transTime);

            if (transTm && nowTm && 
                transTm->tm_mon == nowTm->tm_mon && 
                transTm->tm_year == nowTm->tm_year) {
                total += transaction.getAmount();
            }
        }
        return total;
    }

    bool isOverBudget() const {
        return getTotalMonthlySpending() > monthlyBudget;
    }

    void printFinancialReport() const {
        std::cout << "Financial Report for " << name << std::endl;
        std::cout << "Current Balance: $" << std::fixed << std::setprecision(2) << balance << std::endl;
        std::cout << "Monthly Budget: $" << monthlyBudget << std::endl;
        
        std::cout << "\nCategory Spending and Budgets:" << std::endl;
        auto categorySpending = getCategorySpending();
        for (const auto& [category, amount] : categorySpending) {
            std::cout << getCategoryString(category) << ": $" << amount;
            
            auto it = std::find_if(categoryBudgets.begin(), categoryBudgets.end(),
                                 [category](const BudgetLimit& budget) {
                                     return budget.category == category;
                                 });
            
            if (it != categoryBudgets.end()) {
                std::cout << " (Budget: $" << it->limit;
                if (amount > it->limit) {
                    std::cout << " - OVER BUDGET!";
                }
                std::cout << ")";
            }
            std::cout << std::endl;
        }

        std::cout << "\nTotal Monthly Spending: $" << getTotalMonthlySpending() << std::endl;
        if (isOverBudget()) {
            std::cout << "WARNING: Over Monthly Budget!" << std::endl;
        }
    }
};

class User : public BaseEntity {
private:
    std::string username;
    std::string passwordHash;
    std::vector<Account> accounts;

public:
    User(const std::string& name, const std::string& password)
        : BaseEntity(), username(name) {
        passwordHash = SimpleEncryption::encrypt(password, "FINANCE_APP_SALT");
    }

    std::string serialize() const {
        std::stringstream ss;
        ss << username << "," << passwordHash << "," << id << "," << createdAt << "," << updatedAt;
        return ss.str();
    }

    static User deserialize(const std::string& data) {
        std::stringstream ss(data);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 5) {
            User user(tokens[0], "temp_password");
            user.passwordHash = tokens[1];
            user.id = tokens[2];
            user.createdAt = std::stoll(tokens[3]);
            user.updatedAt = std::stoll(tokens[4]);
            return user;
        }
        throw std::runtime_error("Invalid user data format");
    }

    bool validatePassword(const std::string& inputPassword) {
        return SimpleEncryption::encrypt(inputPassword, "FINANCE_APP_SALT") == passwordHash;
    }

    void addAccount(const Account& account) {
        accounts.push_back(account);
        updateTimestamp();
    }

    std::vector<Account>& getAccounts() {
        return accounts;
    }

    std::string getUsername() const {
        return username;
    }
};

class FinancialPlanner : public BaseEntity {
private:
    Account& account;

public:
    FinancialPlanner(Account& acc) 
        : BaseEntity(), account(acc) {}

    void projectSavings(double monthlyContribution, int months) {
        double currentBalance = account.getBalance();
        double projectedBalance = currentBalance;
        double totalMonthlySpending = account.getTotalMonthlySpending();

        std::cout << "\n=== Savings Projection ===" << std::endl;
        std::cout << "Starting Balance: $" << std::fixed << std::setprecision(2) << currentBalance << std::endl;
        std::cout << "Monthly Contribution: $" << monthlyContribution << std::endl;
        std::cout << "Projected Monthly Spending: $" << totalMonthlySpending << std::endl;

        std::cout << "\nMonth\tProjected Balance\tNet Savings" << std::endl;
        for (int month = 1; month <= months; ++month) {
            projectedBalance += monthlyContribution - totalMonthlySpending;
            double netSavings = projectedBalance - currentBalance;
            
            std::cout << month << "\t$" 
                      << projectedBalance << "\t\t$" 
                      << netSavings << std::endl;
        }
    }

    void provideSpendingInsights() {
        auto categorySpending = account.getCategorySpending();
        
        std::cout << "\n=== Spending Insights ===" << std::endl;
        
        double totalSpending = 0.0;
        for (const auto& [category, amount] : categorySpending) {
            totalSpending += amount;
        }

        std::cout << "Total Spending Analysis:" << std::endl;
        for (const auto& [category, amount] : categorySpending) {
            double percentage = (amount / totalSpending) * 100.0;
            std::cout << getCategoryString(category) << ": $" 
                      << amount << " (" << std::fixed << std::setprecision(2) 
                      << percentage << "%)" << std::endl;
        }

        std::cout << "\nRecommendations:" << std::endl;
        for (const auto& [category, amount] : categorySpending) {
            double percentage = (amount / totalSpending) * 100.0;
            if (percentage > 30.0) {
                std::cout << "- High spending in " << getCategoryString(category) 
                          << ". Consider reducing expenses." << std::endl;
            }
        }
    }
};

class UserManager : public BaseEntity {
private:
    std::vector<User> users;
    User* currentUser;
    std::string dataPath;

    void saveUsers() {
        std::string filename = dataPath + "/users.txt";
        std::stringstream ss;
        for (const auto& user : users) {
            ss << user.serialize() << "\n";
        }
        FileManager::saveToFile(filename, ss.str());
    }

    void loadUsers() {
        std::string filename = dataPath + "/users.txt";
        std::string content = FileManager::readFromFile(filename);
        std::stringstream ss(content);
        std::string line;
        
        users.clear();
        while (std::getline(ss, line)) {
            if (!line.empty()) {
                users.push_back(User::deserialize(line));
            }
        }
    }

public:
    UserManager() : BaseEntity(), currentUser(nullptr) {
        dataPath = "data/users";
        FileManager::createDirectory(dataPath);
        loadUsers();
    }

    bool registerUser(const std::string& username, const std::string& password) {
        for (const auto& user : users) {
            if (user.getUsername() == username) {
                return false;
            }
        }

        users.emplace_back(username, password);
        updateTimestamp();
        saveUsers();
        return true;
    }

    User* authenticateUser(const std::string& username, const std::string& password) {
        for (auto& user : users) {
            if (user.getUsername() == username && user.validatePassword(password)) {
                currentUser = &user;
                return currentUser;
            }
        }
        std::cout << "Invalid username or password!" << std::endl;
        return nullptr;
    }

    User* getCurrentUser() {
        return currentUser;
    }
};

class PersonalFinanceApp : public BaseEntity {
private:
    UserManager userManager;
    User* currentUser;
    Account* currentAccount;

    std::vector<std::string> mainMenu = {
        "Login",
        "Register",
        "Exit"
    };

    std::vector<std::string> userMenu = {
        "Create Account",
        "Select Account",
        "View Accounts",
        "Record Transaction",
        "Edit Transaction",
        "Delete Transaction",
        "Set Category Budget",
        "Delete Category Budget",
        "View Financial Report",
        "Financial Planning",
        "Deposit Money",
        "Logout"
    };
    
    void editTransaction() {
        if (!currentAccount) {
            std::cout << "Please select an account first." << std::endl;
            system("pause");
            return;
        }

        // Display recent transactions
        auto& transactions = currentAccount->getTransactions();
        std::cout << "Recent Transactions:" << std::endl;
        for (const auto& trans : transactions) {
            std::cout << "ID: " << trans.getId() 
                      << " Amount: $" << trans.getAmount()
                      << " Category: " << getCategoryString(trans.getCategory())
                      << " Description: " << trans.getDescription() << std::endl;
        }

        std::string transId;
        std::cout << "Enter transaction ID to edit: ";
        std::cin >> transId;

        double amount;
        int categoryChoice;
        std::string description;

        std::cout << "Enter new amount: $";
        std::cin >> amount;

        std::cout << "Select new category:" << std::endl;
        for (int i = 0; i < static_cast<int>(SpendingCategory::MISCELLANEOUS) + 1; ++i) {
            std::cout << i << ". " << getCategoryString(static_cast<SpendingCategory>(i)) << std::endl;
        }
        std::cout << "Enter category number: ";
        std::cin >> categoryChoice;

        std::cout << "Enter new description: ";
        std::cin.ignore();
        std::getline(std::cin, description);

        try {
            currentAccount->editTransaction(transId, amount, 
                                         static_cast<SpendingCategory>(categoryChoice),
                                         description);
            std::cout << "Transaction updated successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        system("pause");
    }

    void deleteTransaction() {
        if (!currentAccount) {
            std::cout << "Please select an account first." << std::endl;
            system("pause");
            return;
        }

        auto& transactions = currentAccount->getTransactions();
        std::cout << "Recent Transactions:" << std::endl;
        for (const auto& trans : transactions) {
            std::cout << "ID: " << trans.getId() 
                      << " Amount: $" << trans.getAmount()
                      << " Category: " << getCategoryString(trans.getCategory())
                      << " Description: " << trans.getDescription() << std::endl;
        }

        std::string transId;
        std::cout << "Enter transaction ID to delete: ";
        std::cin >> transId;

        try {
            currentAccount->deleteTransaction(transId);
            std::cout << "Transaction deleted successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        system("pause");
    }

    void setCategoryBudget() {
        if (!currentAccount) {
            std::cout << "Please select an account first." << std::endl;
            system("pause");
            return;
        }

        int categoryChoice;
        double limit;

        std::cout << "Select category to set budget:" << std::endl;
        for (int i = 0; i < static_cast<int>(SpendingCategory::MISCELLANEOUS) + 1; ++i) {
            std::cout << i << ". " << getCategoryString(static_cast<SpendingCategory>(i)) << std::endl;
        }
        std::cout << "Enter category number: ";
        std::cin >> categoryChoice;
        std::cout << "Enter budget limit for " 
                    << getCategoryString(static_cast<SpendingCategory>(categoryChoice)) 
                    << ": $";
        std::cin >> limit;

        try {
            currentAccount->setCategoryBudget(static_cast<SpendingCategory>(categoryChoice), limit);
            std::cout << "Category budget set successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        system("pause");
    }

    void deleteCategoryBudget() {
        if (!currentAccount) {
            std::cout << "Please select an account first." << std::endl;
            system("pause");
            return;
        }

        int categoryChoice;
        std::cout << "Select category to delete budget:" << std::endl;
        for (int i = 0; i < static_cast<int>(SpendingCategory::MISCELLANEOUS) + 1; ++i) {
            std::cout << i << ". " << getCategoryString(static_cast<SpendingCategory>(i)) << std::endl;
        }
        std::cout << "Enter category number: ";
        std::cin >> categoryChoice;

        try {
            currentAccount->deleteCategoryBudget(static_cast<SpendingCategory>(categoryChoice));
            std::cout << "Category budget deleted successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        system("pause");
    }

    int currentSelection = 0;

    void displayMenu(const std::vector<std::string>& menu) {
        system("cls");
        std::cout << "=== Personal Finance Management ===" << std::endl
                  << "W and S to go up and down" << std::endl;
        
        for (size_t i = 0; i < menu.size(); ++i) {
            if (i == currentSelection) {
                std::cout << "-> ";
            } else {
                std::cout << "   ";
            }
            std::cout << menu[i] << std::endl;
        }
    }

    void handleKeyInput(bool isMainMenu) {
        int key = _getch();
        std::vector<std::string>& currentMenuRef = isMainMenu ? mainMenu : userMenu;
        
        switch (key) {
            case 'w':
                if (currentSelection > 0) currentSelection--;
                break;
            case 's':
                if (currentSelection < currentMenuRef.size() - 1) currentSelection++;
                break;
            case 13:
                isMainMenu ? selectMainMenuItem() : selectUserMenuItem();
                break;
        }
    }

    void selectMainMenuItem() {
        switch (currentSelection) {
            case 0: loginUser(); break;
            case 1: registerUser(); break;
            case 2: exit(0); break;
        }
    }

    void selectUserMenuItem() {
        switch (currentSelection) {
            case 0: createAccount(); break;
            case 1: selectAccount(); break;
            case 2: viewAccounts(); break;
            case 3: recordTransaction(); break;
            case 4: editTransaction(); break;
            case 5: deleteTransaction(); break;
            case 6: setCategoryBudget(); break;
            case 7: deleteCategoryBudget(); break;
            case 8: viewFinancialReport(); break;
            case 9: financialPlanning(); break;
            case 10: depositMoney(); break;
            case 11: logout(); break;
        }
    }

    void loginUser() {
        std::string username, password;
        std::cout << "Enter username: ";
        std::cin >> username;
        std::cout << "Enter password: ";
        std::cin >> password;

        currentUser = userManager.authenticateUser(username, password);
        if (currentUser) {
            std::cout << "Login successful!" << std::endl;
            system("pause");
            currentSelection = 0;
        }
    }

    void registerUser() {
        std::string username, password;
        std::cout << "Choose a username: ";
        std::cin >> username;
        std::cout << "Choose a password: ";
        std::cin >> password;

        userManager.registerUser(username, password);
        system("pause");
    }

    void createAccount() {
        if (!currentUser) return;

        std::string accountName;
        double initialBalance;

        std::cout << "Enter account name: ";
        std::cin >> accountName;
        std::cout << "Enter initial balance: $";
        std::cin >> initialBalance;

        Account newAccount(accountName, initialBalance);
        currentUser->addAccount(newAccount);

        std::cout << "Account created successfully!" << std::endl;
        system("pause");
    }

    void selectAccount() {
        if (!currentUser || currentUser->getAccounts().empty()) {
            std::cout << "No accounts available. Create an account first." << std::endl;
            system("pause");
            return;
        }

        std::cout << "Select an account:" << std::endl;
        auto& accounts = currentUser->getAccounts();
        for (size_t i = 0; i < accounts.size(); ++i) {
            std::cout << i + 1 << ". " << accounts[i].getName() << std::endl;
        }

        int choice;
        std::cout << "Enter account number: ";
        std::cin >> choice;

        if (choice > 0 && choice <= accounts.size()) {
            currentAccount = &accounts[choice - 1];
            std::cout << "Account selected: " << currentAccount->getName() << std::endl;
        } else {
            std::cout << "Invalid account selection." << std::endl;
        }
        system("pause");
    }

    void viewAccounts() {
        if (!currentUser || currentUser->getAccounts().empty()) {
            std::cout << "No accounts available." << std::endl;
            system("pause");
            return;
        }

        std::cout << "=== Your Accounts ===" << std::endl;
        auto& accounts = currentUser->getAccounts();
        for (size_t i = 0; i < accounts.size(); ++i) {
            std::cout << "Account " << i + 1 << ": " 
                      << accounts[i].getName() 
                      << " - Balance: $" << accounts[i].getBalance() << std::endl;
        }
        system("pause");
    }

    void recordTransaction() {
        if (!currentAccount) {
            std::cout << "Please select an account first." << std::endl;
            system("pause");
            return;
        }

        double amount;
        int categoryChoice;

        std::cout << "Enter transaction amount: $";
        std::cin >> amount;

        std::cout << "Select Spending Category:" << std::endl;
        for (int i = 0; i < static_cast<int>(SpendingCategory::MISCELLANEOUS) + 1; ++i) {
            std::cout << i << ". " << getCategoryString(static_cast<SpendingCategory>(i)) << std::endl;
        }
        
        std::cout << "Enter category number: ";
        std::cin >> categoryChoice;

        SpendingCategory category = static_cast<SpendingCategory>(categoryChoice);
        
        std::string description;
        std::cout << "Enter transaction description (optional): ";
        std::cin.ignore();
        std::getline(std::cin, description);

        Transaction transaction(amount, category, description);
        currentAccount->addTransaction(transaction);

        std::cout << "Transaction recorded successfully!" << std::endl;
        system("pause");
    }

    void viewFinancialReport() {
        if (!currentAccount) {
            std::cout << "Please select an account first." << std::endl;
            system("pause");
            return;
        }

        currentAccount->printFinancialReport();
        system("pause");
    }

    void financialPlanning() {
        if (!currentAccount) {
            std::cout << "Please select an account first." << std::endl;
            system("pause");
            return;
        }

        FinancialPlanner planner(*currentAccount);
        
        int choice;
        std::cout << "Financial Planning Options:" << std::endl;
        std::cout << "1. Savings Projection" << std::endl;
        std::cout << "2. Spending Insights" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1: {
                double monthlyContribution;
                int months;
                std::cout << "Enter monthly contribution: $";
                std::cin >> monthlyContribution;
                std::cout << "Enter projection period (months): ";
                std::cin >> months;
                planner.projectSavings(monthlyContribution, months);
                break;
            }
            case 2:
                planner.provideSpendingInsights();
                break;
            default:
                std::cout << "Invalid choice." << std::endl;
        }
        system("pause");
    }

    void depositMoney() {
        if (!currentAccount) {
            std::cout << "Please select an account first." << std::endl;
            system("pause");
            return;
        }

        double amount;
        std::cout << "Enter amount to deposit: $";
        std::cin >> amount;

        if (amount > 0) {
            currentAccount->deposit(amount);
            std::cout << "Deposit successful!" << std::endl;
        } else {
            std::cout << "Invalid deposit amount." << std::endl;
        }
        system("pause");
    }

    void logout() {
        currentUser = nullptr;
        currentAccount = nullptr;
        currentSelection = 0;
        std::cout << "Logged out successfully!" << std::endl;
        system("pause");
    }

public:
    PersonalFinanceApp() : BaseEntity(), currentUser(nullptr), currentAccount(nullptr) {
        // Create data directory structure
        FileManager::createDirectory("data");
        FileManager::createDirectory("data/users");
        FileManager::createDirectory("data/accounts");
    }

    void run() {
        while (true) {
            if (!currentUser) {
                displayMenu(mainMenu);
                handleKeyInput(true);
            } else {
                displayMenu(userMenu);
                handleKeyInput(false);
            }
        }
    }
};

int main() {
    try {
        PersonalFinanceApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}