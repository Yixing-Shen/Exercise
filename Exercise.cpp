#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstdint>
#include <stdexcept>
#include <iostream>

using namespace std;

// Data structure to store statistics for each symbol
struct SymbolStats {
    uint64_t last_timestamp = 0;  // Timestamp of last trade
    uint64_t max_gap = 0;         // Maximum time gap between consecutive trades
    int64_t total_volume = 0;     // Total traded quantity
    int64_t weighted_sum = 0;     // Sum of (price * quantity) for weighted average
    int max_price = 0;            // Highest price encountered
};

// Process a single line of input
void process_line(const string& line, unordered_map<string, SymbolStats>& symbols) {
    vector<string> parts;
    istringstream ss(line);
    string part;
    
    // Split line into comma-separated components
    while (getline(ss, part, ',')) {
        parts.push_back(part);
    }

    // Validate column count
    if (parts.size() != 4) {
        cerr << "[WARN] Invalid line format (expected 4 columns): " << line << endl;
        return;
    }

    try {
        // Parse components
        uint64_t timestamp = stoull(parts[0]);
        const string& symbol = parts[1];
        int quantity = stoi(parts[2]);
        int price = stoi(parts[3]);

        // Get or create symbol entry
        auto& stats = symbols[symbol];
        
        // Calculate time gap (skip for first trade)
        if (stats.last_timestamp != 0) {
            uint64_t gap = timestamp - stats.last_timestamp;
            stats.max_gap = max(stats.max_gap, gap);
        }
        stats.last_timestamp = timestamp;

        // Update statistics
        stats.total_volume += quantity;
        stats.weighted_sum += quantity * price;
        stats.max_price = max(stats.max_price, price);
    }
    catch (const invalid_argument& e) {
        cerr << "[ERROR] Invalid numeric format: " << line << endl;
    }
    catch (const out_of_range& e) {
        cerr << "[ERROR] Numeric value out of range: " << line << endl;
    }
}

int main(int argc, char* argv[]) {
    // Validate command line arguments
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    unordered_map<string, SymbolStats> symbol_map;

    // Process input file
    ifstream input(argv[1]);
    if (!input.is_open()) {
        cerr << "Failed to open input file: " << argv[1] << endl;
        return 1;
    }

    string line;
    while (getline(input, line)) {
        process_line(line, symbol_map);
    }

    // Prepare sorted output
    vector<pair<string, SymbolStats>> sorted_symbols;
    sorted_symbols.reserve(symbol_map.size());
    for (auto& [symbol, stats] : symbol_map) {
        sorted_symbols.emplace_back(symbol, stats);
    }

    // Sort symbols lexicographically
    sort(sorted_symbols.begin(), sorted_symbols.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

    // Generate output file
    ofstream output(argv[2]);
    if (!output.is_open()) {
        cerr << "Failed to create output file: " << argv[2] << endl;
        return 1;
    }

    // Write results
    for (const auto& [symbol, stats] : sorted_symbols) {
        int avg_price = static_cast<int>(stats.weighted_sum / stats.total_volume);
        output << symbol << ","
               << stats.max_gap << ","
               << stats.total_volume << ","
               << avg_price << ","
               << stats.max_price << "\n";
    }

    return 0;
}
