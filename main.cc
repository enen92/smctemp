#include <charconv>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

#include "smctemp.h"

void usage(char* prog) {
  std::cout << "Check Temperature by using Apple System Management Control (Smc) tool " << smctemp::kVersion << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << prog << " [options]" << std::endl;
  std::cout << "    -c         : list CPU temperatures (Celsius). You can provide an argument to retry for n times (e.g. -c3 for 3 retries)." << std::endl;
  std::cout << "    -h         : help" << std::endl;
  std::cout << "    -l         : list all keys and values" << std::endl;
  std::cout << "    -v         : version" << std::endl;
}

int main(int argc, char *argv[]) {
  int c;
  unsigned int attempts = 1;

  kern_return_t result;
  int           op = smctemp::kOpNone;
  smctemp::UInt32Char_t  key = { 0 };
  smctemp::SmcVal_t      val;

  while ((c = getopt(argc, argv, "c::lvh")) != -1) {
    switch(c) {
      case 'c':
        op = smctemp::kOpReadCpuTemp;
        if (optarg) {
          auto [ptr, ec] = std::from_chars(optarg, optarg + strlen(optarg), attempts);
          if (ec != std::errc()) {
            std::cerr << "Invalid argument provided for -c (integer is required)" << std::endl;
            return 1;
          }
        }
        break;
      case 'l':
        op = smctemp::kOpList;
        break;
      case 'v':
        std::cout << smctemp::kVersion << std::endl;
        return 0;
        break;
      case 'h':
      case '?':
        op = smctemp::kOpNone;
        break;
    }
  }

  if (op == smctemp::kOpNone) {
    usage(argv[0]);
    return 1;
  }

  smctemp::SmcAccessor smc_accessor = smctemp::SmcAccessor();
  smctemp::SmcTemp smc_temp = smctemp::SmcTemp();

  switch(op) {
    case smctemp::kOpList:
      result = smc_accessor.PrintAll();
      if (result != kIOReturnSuccess) {
        std::ios_base::fmtflags ef(std::cerr.flags());
        std::cerr << "Error: SmcPrintAll() = "
          << std::hex << result << std::endl;
        std::cerr.flags(ef);
      }
      break;
    case smctemp::kOpReadCpuTemp:
      double temp = 0.0;
      while (attempts > 0)
      {
        temp = smc_temp.GetCpuTemp();
        attempts--;
        if (attempts > 0 && temp == 0.0) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      }
      std::cout << std::fixed << std::setprecision(1) << temp;
      break;
  }

  return 0;
}

