#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <cmath>
#include <iomanip>

// Mock structure for coordinates to compute "Impossible Travel"
struct GeoLocation {
    double latitude;
    double longitude;

    // Haversine formula to find distance in kilometers
    double distanceTo(const GeoLocation& other) const {
        double r = 6371.0; // Earth radius in km
        double dLat = (other.latitude - latitude) * M_PI / 180.0;
        double dLon = (other.longitude - longitude) * M_PI / 180.0;
        double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
                   std::cos(latitude * M_PI / 180.0) * std::cos(other.latitude * M_PI / 180.0) *
                   std::sin(dLon / 2) * std::sin(dLon / 2);
        return 2 * r * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    }
};

struct Transaction {
    std::string cardId;
    double amount;
    std::chrono::steady_clock::time_point timestamp;
    std::string locationName;
    GeoLocation coordinates;
};

class CardSecurityManager {
private:
    // Tracks previously processed transactions per card
    std::unordered_map<std::string, std::vector<Transaction>> internalLedger;
    
    // Core Blocklist: Flagged and locked cards
    std::unordered_set<std::string> flaggedCards;

    // Threshold Configs
    const double RISK_LIMIT = 70.0; 
    const double HIGH_AMOUNT_TRIGGER = 5000.00; // Trigger alert for heavy sums
    const double MAX_SPEED_KMH = 900.0;        // Rough commercial jet speed ceiling

public:
    void processTransaction(const Transaction& newTxn) {
        std::cout << "\n========================================\n";
        std::cout << "INCOMING TRANSACTION FOR CARD: " << newTxn.cardId << "\n";
        std::cout << "Amount: $" << std::fixed << std::setprecision(2) << newTxn.amount 
                  << " | Location: " << newTxn.locationName << "\n";
        std::cout << "----------------------------------------\n";

        // Check if the card is already completely locked down
        if (flaggedCards.count(newTxn.cardId)) {
            std::cout << "❌ TRANSACTION DECLINED: Card is permanently FLAGGED & LOCKED.\n";
            return;
        }

        double riskScore = 0.0;
        std::vector<std::string> warningSigns;

        // --- WARNING SIGN 1: High Dollar Amount Outlier ---
        if (newTxn.amount >= HIGH_AMOUNT_TRIGGER) {
            riskScore += 35.0;
            warningSigns.push_back("⚠️ High Outlier Value: Transaction exceeds normal single-charge limit.");
        }

        // Evaluate historical parameters if card has a transaction history
        if (internalLedger.count(newTxn.cardId) && !internalLedger[newTxn.cardId].empty()) {
            const auto& lastTxn = internalLedger[newTxn.cardId].back();
            
            // Calculate time delta in hours
            auto timeDelta = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(newTxn.timestamp - lastTxn.timestamp).count();
            
            // --- WARNING SIGN 2: High Velocity (Frequency Burst) ---
            if (timeDelta < (1.0 / 60.0)) { // Less than 1 minute apart
                riskScore += 40.0;
                warningSigns.push_back("⚠️ Transaction Velocity: Sequential hits detected within seconds.");
            }

            // --- WARNING SIGN 3: Impossible Travel Speed ---
            double distance = lastTxn.coordinates.distanceTo(newTxn.coordinates);
            if (timeDelta > 0.0) {
                double requiredSpeed = distance / timeDelta;
                if (requiredSpeed > MAX_SPEED_KMH) {
                    riskScore += 50.0;
                    warningSigns.push_back("⚠️ Geo-Impossible Travel: Movement speed of " + 
                                           std::to_string(static_cast<int>(requiredSpeed)) + " km/h exceeds human limits.");
                }
            }
        }

        // Print Risk Evaluation Details
        if (!warningSigns.empty()) {
            std::cout << "RISK METRICS TRIGGERED:\n";
            for (const auto& sign : warningSigns) {
                std::cout << "  " << sign << "\n";
            }
        } else {
            std::cout << "✅ No anomalous patterns detected.\n";
        }

        std::cout << "Calculated Risk Score: " << riskScore << " / 100.0\n";

        // Action assessment
        if (riskScore >= RISK_LIMIT) {
            flaggedCards.insert(newTxn.cardId);
            std::cout << "🚨 CRITICAL RISK THRESHOLD BREACHED! Card has been FLAGGED.\n";
            std::cout << "❌ status: TRANSACTION TERMINATED & CARD DEACTIVATED.\n";
        } else {
            // Commit to ledger if legitimate
            internalLedger[newTxn.cardId].push_back(newTxn);
            std::cout << "🟩 status: APPROVED & RECORDED TO LEDGER.\n";
        }
        std::cout << "========================================\n";
    }
};

int main() {
    CardSecurityManager mySecEngine;
    auto clockStartTime = std::chrono::steady_clock::now();

    // TARGET CARD: CARD-XYZ-88
    // Txn 1: Baseline regular transaction in New York
    Transaction txn1{
        "CARD-XYZ-88", 45.50, clockStartTime, 
        "New York Coffee Shop", GeoLocation{40.7128, -74.0060}
    };

    // Txn 2: Fraudulent attempt 2 minutes later from London for a massive amount
    Transaction txn2{
        "CARD-XYZ-88", 6500.00, clockStartTime + std::chrono::minutes(2), 
        "London Luxury Boutique", GeoLocation{51.5074, -0.1278}
    };

    // Txn 3: The fraudster attempts another charge on the same card after being flagged
    Transaction txn3{
        "CARD-XYZ-88", 12.00, clockStartTime + std::chrono::minutes(5), 
        "Online App Store", GeoLocation{51.5074, -0.1278}
    };

    // Run simulations through your engine
    mySecEngine.processTransaction(txn1);
    mySecEngine.processTransaction(txn2); // Should trigger multiple indicators and flag card
    mySecEngine.processTransaction(txn3); // Should instantly drop out due to flagged status

    return 0;
}
