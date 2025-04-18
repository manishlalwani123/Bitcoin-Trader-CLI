#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <fstream>

class Wallet {
private:
    const std::string ticker_ = "BTC";
    float cash_;
    float coin_;

public:
    explicit Wallet(float initialVal = 5400) : cash_{initialVal}, coin_{0} {}

    void print() {
        std::stringstream ss;
        ss << "===== Wallet =====" << std::endl
           << "Current cash: " << getCash() << " USD" << std::endl
           << "Current coin: " << getCoin() << " " << getTickerSymbol();
        std::cout << ss.str() << std::endl;
    }

    void addCoin(float value) {
        coin_ += value;
    }

    void removeCoin(float value) {
        coin_ -= value;
    }

    void removeCash(float value) {
        cash_ -= value;
    }

    void addCash(float value) {
        cash_ += value;
    }

    float getCash() const {
        return cash_;
    }

    float getCoin() const {
        return coin_;
    }

    std::string getTickerSymbol() {
        return ticker_;
    }
};

class Ledger {
private:
    std::vector<std::string> transactions;

public:
    void record(const std::string &transaction) {
        transactions.push_back(transaction);
    }

    void print() {
        std::stringstream ss;
        ss << "===== Ledger =====";
        std::cout << ss.str() << std::endl;
        for (std::string &transaction : transactions)
            std::cout << transaction << std::endl;
    }

    std::vector<std::string> getTransactions() const {
        return transactions;
    }

    void setTransaction(const std::vector<std::string>& newTransaction) {
        transactions = newTransaction;
    }
};

class GetLive {
private:
    static float getRandomFloat(float min, float max) {
        // Returns a float between min and max
        return ((max - min) * ((float) rand() / RAND_MAX)) + min;
    }

public:
    static float getBTCValue() {
        return getRandomFloat(5000, 7000);
    }

    // USD to BTC
    static float convertToBTC(float val, float btcVal = 0) {
        float curr = btcVal == 0 ? getBTCValue() : btcVal;
        return curr / val;
    }

    // BTC to USD
    static float convertToUSD(float val, float btcVal = 0) {
        float curr = btcVal == 0 ? getBTCValue() : btcVal;
        return curr * val;
    }

    static void printLive() {
        std::cout << "The current price of BTC is " << getBTCValue() << " USD." << std::endl;
    }
};

class MyDate {
public:
    static std::string getCurrentTime() {
        using namespace std::chrono;
        auto curr = system_clock::now();
        auto timer = system_clock::to_time_t(curr);
        tm bt = *std::localtime(&timer);
        std::ostringstream oss;
        oss << std::put_time(&bt, "%H:%M:%S (%m/%d/%y)"); // HH:MM:SS
        return oss.str();
    }
};

static bool isNumber(const std::string &s) {
    if (s.empty()) return false;
    int numDot = 0; // For decimals
    for (int i = s.front() == 0x002D ? 1 : 0; i < s.length(); i++) {
        // Checks if the character at i is a dot
        if (s.at(i) == 0x002E) {
            numDot++;
            if (numDot > 1)
                return false;
        } else if (!std::isdigit(int(s.at(i))))
            return false;
    }
    return true;
}

static std::vector<std::string> split(const std::string &str, const std::string &sep) {
    char *cStr = const_cast<char *>(str.c_str());
    char *current;
    std::vector<std::string> arr;
    current = strtok(cStr, sep.c_str());
    while (current != NULL) {
        arr.emplace_back(current);
        current = strtok(NULL, sep.c_str());
    }
    return arr;
}

class IO {
public:
    static void save(const std::string &fileName, const std::vector<std::string> &toSerialize) {
        std::ofstream output(fileName);
        for (const std::string &s : toSerialize)
            output << s << std::endl;
        output.close();
    }

    static std::vector<std::string> load(const std::string &fileName) {
        std::vector<std::string> out;
        std::ifstream input(fileName);
        std::string line;
        while (std::getline(input, line))
            out.emplace_back(line);
        return out;
    }
};

class BasicCommand {
protected:
    std::string name_;
    std::string argument_ = "";

public:
    virtual void onCommand(const std::vector<std::string> &args) {
    }

    std::string getName() {
        return name_;
    }

    std::string getArgument() {
        return argument_;
    }

    static void printInvalidArg() {
        std::cout << "You did not enter a valid argument!" << std::endl;
    }
};

class BitCoinCommand : public BasicCommand {
protected:
    Wallet *wallet_;
    Ledger *ledger_;

public:
    BitCoinCommand(Wallet *wallet, Ledger *ledger) : wallet_{wallet}, ledger_{ledger} {}
};

class Sell : public BitCoinCommand {
public:
    Sell(Wallet *wallet, Ledger *ledger) : BitCoinCommand{wallet, ledger} {
        name_ = "sell";
        argument_ = "<amount>";
    }

    void onCommand(const std::vector<std::string> &args) override {
        if (args.size() != 1 || !isNumber(args.front())) {
            printInvalidArg();
            return;
        }
        std::stringstream argStream(args.front());
        float currentBTCVal = GetLive::getBTCValue();
        float val = 0;
        argStream >> val;
        if (val > wallet_->getCoin()) {
            std::cout << "You don't have enough BTC to sell." << std::endl;
            std::cout << "Please a enter different amount." << std::endl;
        } else {
            float price = GetLive::convertToUSD(val, currentBTCVal);
            wallet_->removeCoin(val);
            wallet_->addCash(price);
            std::stringstream ss;
            ss << "You sold " << val << " BTC at " << MyDate::getCurrentTime();
            ledger_->record(ss.str());
            std::cout << ss.str() << std::endl;
        }
    }
};

class Buy : public BitCoinCommand {
public:
    Buy(Wallet *wallet, Ledger *ledger) : BitCoinCommand{wallet, ledger} {
        name_ = "buy";
        argument_ = "<amount>";
    }

    void onCommand(const std::vector<std::string> &args) override {
        if (args.size() != 1 || !isNumber(args.front())) {
            printInvalidArg();
            return;
        }
        std::stringstream argStream(args.front());
        float currentBTCVal = GetLive::getBTCValue();
        float val = 0;
        argStream >> val;
        float cost = GetLive::convertToUSD(val, currentBTCVal);
        if (cost > wallet_->getCash()) {
            std::cout << "You don't have enough cash to buy that amount of bitcoin" << std::endl;
            std::cout << "Please a enter different amount." << std::endl;
        } else {
            wallet_->removeCash(cost);
            wallet_->addCoin(val);
            std::stringstream ss;
            ss << "You bought " << val << " BTC at " << MyDate::getCurrentTime();
            ledger_->record(ss.str());
            std::cout << ss.str() << std::endl;
        }
    }
};

class Price : public BitCoinCommand {
public:
    Price(Wallet *wallet, Ledger *ledger) : BitCoinCommand{wallet, ledger} {
        name_ = "price";
    }

    void onCommand(const std::vector<std::string> &args) override {
        if (!args.empty()) {
            printInvalidArg();
            return;
        }
        GetLive::printLive();
    }
};

class WalletCmd : public BitCoinCommand {
public:
    WalletCmd(Wallet *wallet, Ledger *ledger) : BitCoinCommand{wallet, ledger} {
        name_ = "wallet";
    }

    void onCommand(const std::vector<std::string> &args) override {
        if (!args.empty()) {
            printInvalidArg();
            return;
        }
        wallet_->print();
    }
};

class History : public BitCoinCommand {
public:
    History(Wallet *wallet, Ledger *ledger) : BitCoinCommand{wallet, ledger} {
        name_ = "history";
    }

    void onCommand(const std::vector<std::string> &args) override {
        if (!args.empty()) {
            printInvalidArg();
            return;
        }
        ledger_->print();
    }
};

class Save : public BitCoinCommand {
public:
    Save(Wallet *wallet, Ledger *ledger) : BitCoinCommand{wallet, ledger} {
        name_ = "save";
    };

    void onCommand(const std::vector<std::string> &args) override {
        if (!args.empty()) {
            printInvalidArg();
            return;
        }
        IO::save("History.txt", ledger_->getTransactions());
        std::cout << "Saved transaction history to 'History.txt'." << std::endl;
    }
};

class Load : public BitCoinCommand {
public:
    Load(Wallet *wallet, Ledger *ledger) : BitCoinCommand{wallet, ledger} {
        name_ = "load";
    };

    void onCommand(const std::vector<std::string> &args) override {
        if (!args.empty()) {
            printInvalidArg();
            return;
        }
        ledger_->setTransaction(IO::load("History.txt"));
        std::cout << "Loaded transaction history to Ledger, enter 'history' to view." << std::endl;
    }
};

class CommandManager {
private:
    std::vector<BitCoinCommand *> registeredCommands;

public:
    void registerCommand(const BitCoinCommand *command) {
        registeredCommands.push_back(const_cast<BitCoinCommand *&&>(command));
    }

    void processCommand(const std::string &input) {
        std::vector<std::string> parsed = split(input, " ");
        // sell 1.5 -> [sell, 1.5]
        std::string front = parsed.front();
        for (BasicCommand *c : registeredCommands) {
            if (c->getName() == front) {
                parsed.erase(parsed.begin());
                c->onCommand(parsed);
                return;
            }
        }
    }
};

class CLIApp {
private:
    Wallet *wallet_;
    Ledger *ledger_;
    CommandManager manager_;
    bool running_;

public:
    CLIApp() : running_{false}, wallet_{new Wallet()}, ledger_{new Ledger()} {
        manager_.registerCommand(new Sell(wallet_, ledger_));
        manager_.registerCommand(new Buy(wallet_, ledger_));
        manager_.registerCommand(new Price(wallet_, ledger_));
        manager_.registerCommand(new History(wallet_, ledger_));
        manager_.registerCommand(new WalletCmd(wallet_, ledger_));
        manager_.registerCommand(new Save(wallet_, ledger_));
        manager_.registerCommand(new Load(wallet_, ledger_));
    }

    void start() {
        running_ = true;
        std::cout << "Welcome to Bitcoin CLI. The following commands are allowed:" << std::endl;
        std::cout << "buy <amount> | sell <amount> | price | wallet | history | save | load | quit" << std::endl;
        while (running_) {
            std::cout << ">";
            std::string input;
            std::getline(std::cin, input);
            if (input == "quit")
                return;
            manager_.processCommand(input);
        }
    }
};

int main() {
    CLIApp app = *new CLIApp;
    app.start();
    return 0;
}


