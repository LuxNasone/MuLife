#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <algorithm>
#include <cmath>
#include "TH1F.h"

namespace fs = std::filesystem;

void Cal(const char* folderPath) {

    TH1F* Pks = new TH1F("Peak", "Histogram of integrator signal height", 100, 0, 20000);

    std::vector<std::pair<int, std::vector<double>>> numberedFiles;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        std::string filename = entry.path().filename().string();

        if (filename.rfind("wavedump", 0) == 0 && entry.path().extension() == ".txt") {

            std::ifstream file(entry.path());
            if (!file.is_open()) continue;

            std::vector<double> values;
            double x;
            while (file >> x) values.push_back(x);

            size_t start = 8; // dopo "wavedump"
            size_t end = filename.find(".txt");
            if (end == std::string::npos || start >= end) continue; // controllo robusto

            std::string numberStr = filename.substr(start, end - start);

            try {
                int num = std::stoi(numberStr);
                numberedFiles.push_back({num, values});
            } catch (const std::invalid_argument& e) {
                continue; // ignora file con numero non valido
            } catch (const std::out_of_range& e) {
                continue; // ignora numeri troppo grandi
            }
        }
    }

    std::sort(numberedFiles.begin(), numberedFiles.end(),
              [](auto &a, auto &b){ return a.first < b.first; });

    if (numberedFiles.empty()) return;

    size_t numFiles = numberedFiles.size();
    size_t numRows = numberedFiles[0].second.size();

    std::vector<std::vector<double>> matrix(numRows, std::vector<double>(numFiles));

    for (size_t col = 0; col < numFiles; col++) {
        for (size_t row = 0; row < numRows; row++) {
            matrix[row][col] = numberedFiles[col].second[row];
        }
    }

    std::vector<double> colSums(numFiles, 0.0);
    for (size_t col = 0; col < numFiles; col++) {
        double sum = 0;
        for (size_t row = 0; row < numRows; row++) {
            sum += matrix[row][col];
        }
        colSums[col] = sum;
    }

    for (size_t k = 0; k + 1 < colSums.size(); k++) {
        double diff = std::abs(colSums[k+1] - colSums[k]);
        if (diff <= 3000) continue;
        Pks->Fill(diff);
    }

    Pks->Print();
}
