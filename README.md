#include <iostream>
#include <string>
#include <unordered_map>
#include <deque>
#include <chrono>

struct Transaction {
    std::string cardId;
    double amount;
    std::chrono::steady_clock::time_point timestamp;
    std::string location;
};

class FraudDetector {
private:
    // Store recent timestamps of transactions per cardId
    std::unordered_map<std::string, std::deque<std::chrono::steady_clock::time_point>> cardHistory;
    
    // Store recent locations per cardId
    std::unordered_map<std::string, std::deque<std::string>> locationHistory;

    const size_t MAX_TRANSACTIONS_IN_WINDOW = 3;
    const std::chrono::seconds TIME_WINDOW{60}; // 60-second window

public:
    bool isFraudulent(const Transaction& txn) {
        auto now = txn.timestamp;

        // 1. Clean up old timestamps outside the time window
        auto& timestamps = cardHistory[txn.cardId];
        while (!timestamps.empty() && (now - timestamps.front()) > TIME_WINDOW) {
            timestamps.pop_front();
            locationHistory[txn.cardId].pop_front();
        }

        // 2. Check velocity rule (too many transactions too fast)
        if (timestamps.size() >= MAX_TRANSACTIONS_IN_WINDOW) {
            return true; // Flag as high-velocity fraud
        }

        // 3. Record current transaction
        timestamps.push_back(now);
        locationHistory[txn.cardId].push_back(txn.location);

        return false;
    }
};

int main() {
    FraudDetector detector;
    auto now = std::chrono::steady_clock::now();

    Transaction t1{"CARD-999", 50.25, now, "New York"};
    Transaction t2{"CARD-999", 10.00, now + std::chrono::seconds(5), "New York"};
    Transaction t3{"CARD-999", 100.00, now + std::chrono::seconds(10), "London"}; // Rapid change/frequency

    std::cout << "Txn 1 Fraud? " << (detector.isFraudulent(t1) ? "YES" : "NO") << '\n';
    std::cout << "Txn 2 Fraud? " << (detector.isFraudulent(t2) ? "YES" : "NO") << '\n';
    std::cout << "Txn 3 Fraud? " << (detector.isFraudulent(t3) ? "YES" : "NO") << '\n';

    return 0;
}
   
