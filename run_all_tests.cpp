#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>   // setw, setfill
#include <fstream>   // ifstream (for existence check)
#include <cstdio>    // std::rename, std::remove
#include <cstdlib>   // std::system

// Change this to the name/path of your compiled simulator executable.
// Examples:
//   "./bin/interrupts_EP_RR"
//   "./bin/interrupts_RR"
//   "./bin/interrupts_EP"
const std::string EXECUTABLE = "./bin/interrupts_EP_RR interrupts_student1_student2_EP_RR";
const std::string EXECUTABLE = "./bin/interrupts_EP interrupts_student1_student2_EP";
const std::string EXECUTABLE = "./bin/interrupts_RR interrupts_student1_student2_RR";

// Simple helper to check if a file exists (C++14-compatible)
bool fileExists(const std::string &filename) {
    std::ifstream f(filename.c_str());
    return f.good();
}

int main() {
    // Run tests from 01 to 20
    for (int i = 1; i <= 20; ++i) {
        // Build test file name: test01.txt, test02.txt, ..., test20.txt
        std::ostringstream nameBuilder;
        nameBuilder << "test" << std::setw(2) << std::setfill('0') << i << ".txt";
        std::string testFile = nameBuilder.str();

        // Check if the test file exists; if not, skip with a warning
        if (!fileExists(testFile)) {
            std::cerr << "[SKIP] " << testFile << " not found.\n";
            continue;
        }

        std::cout << "=============================\n";
        std::cout << "Running: " << EXECUTABLE << " " << testFile << "\n";

        // Build the command: ./bin/interrupts_EP_RR testXX.txt
        std::string command = EXECUTABLE + " " + testFile;
        int ret = std::system(command.c_str());

        if (ret != 0) {
            std::cerr << "[ERROR] Command failed for " << testFile
                      << " with code " << ret << "\n";
            continue;
        }

        // After the simulator runs, it should have created "execution.txt".
        std::string baseExecFile = "execution.txt";
        if (!fileExists(baseExecFile)) {
            std::cerr << "[WARN] " << baseExecFile
                      << " not found after running " << testFile << "\n";
            continue;
        }

        // Build output filename: execution_01.txt, execution_02.txt, ...
        std::ostringstream outBuilder;
        outBuilder << "execution_"
                   << std::setw(2) << std::setfill('0') << i
                   << ".txt";
        std::string outFile = outBuilder.str();

        // If output file already exists, remove it first
        if (fileExists(outFile)) {
            std::remove(outFile.c_str());
        }

        if (std::rename(baseExecFile.c_str(), outFile.c_str()) != 0) {
            std::perror("[ERROR] Failed to rename execution.txt");
        } else {
            std::cout << "[OK] Output saved to " << outFile << "\n";
        }
    }

    std::cout << "Done running tests.\n";
    return 0;
}
