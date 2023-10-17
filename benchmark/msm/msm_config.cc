#include "benchmark/msm/msm_config.h"

#include <algorithm>

#include "tachyon/base/console/iostream.h"
#include "tachyon/base/flag/flag_parser.h"

namespace tachyon {
namespace base {

template <>
class FlagValueTraits<MSMConfig::TestSet> {
 public:
  static bool ParseValue(std::string_view input, MSMConfig::TestSet* value,
                         std::string* reason) {
    if (input == "random") {
      *value = MSMConfig::TestSet::kRandom;
    } else if (input == "non_uniform") {
      *value = MSMConfig::TestSet::kNonUniform;
    } else {
      *reason = absl::Substitute("Unknown test set: $0", input);
      return false;
    }
    return true;
  }
};

template <>
class FlagValueTraits<MSMConfig::Vendor> {
 public:
  static bool ParseValue(std::string_view input, MSMConfig::Vendor* value,
                         std::string* reason) {
    if (input == "arkworks") {
      *value = MSMConfig::Vendor::kArkworks;
    } else if (input == "bellman") {
      *value = MSMConfig::Vendor::kBellman;
    } else if (input == "halo2") {
      *value = MSMConfig::Vendor::kHalo2;
    } else {
      *reason = absl::Substitute("Unknown vendor: $0", input);
      return false;
    }
    return true;
  }
};

}  // namespace base

// static
std::string MSMConfig::VendorToString(MSMConfig::Vendor vendor) {
  switch (vendor) {
    case MSMConfig::Vendor::kArkworks:
      return "arkworks";
    case MSMConfig::Vendor::kBellman:
      return "bellman";
    case MSMConfig::Vendor::kHalo2:
      return "halo2";
  }
  NOTREACHED();
  return "";
}

bool MSMConfig::Parse(int argc, char** argv,
                      const MSMConfig::Options& options) {
  base::FlagParser parser;
  // clang-format off
  parser.AddFlag<base::Flag<std::vector<uint64_t>>>(&degrees_)
      .set_short_name("-n")
      .set_required()
      .set_help("Specify the exponent 'n' where the number of points to test is 2ⁿ.");
  // clang-format on
  parser.AddFlag<base::BoolFlag>(&check_results_)
      .set_long_name("--check_results")
      .set_help("Whether checks results generated by each msm runner.");
  parser.AddFlag<base::Flag<TestSet>>(&test_set_)
      .set_long_name("--test_set")
      .set_help(
          "Testset to be benchmarked with. (supported testset: random, "
          "non_uniform)");
  if (options.include_vendors) {
    parser.AddFlag<base::Flag<std::vector<Vendor>>>(&vendors_)
        .set_long_name("--vendor")
        .set_help(
            "Vendors to be benchmarked with. (supported vendors: arkworks, "
            "bellman, halo2)");
  }
  if (options.include_algos) {
    parser
        .AddFlag<base::IntFlag>(
            [this](std::string_view arg, std::string* reason) {
              if (arg == "bellman_msm") {
                algorithm_ = 0;
                return true;
              } else if (arg == "cuzk") {
                algorithm_ = 1;
                return true;
              }
              *reason = absl::Substitute("Not supported algorithm: $0", arg);
              return false;
            })
        .set_long_name("--algo")
        .set_help(
            "Algorithms to be benchmarked with. (supported algorithms: "
            "bellman_msm, cuzk)");
  }
  {
    std::string error;
    if (!parser.Parse(argc, argv, &error)) {
      tachyon_cerr << error << std::endl;
      return false;
    }
  }

  base::ranges::sort(degrees_);
  return true;
}

std::vector<uint64_t> MSMConfig::GetPointNums() const {
  std::vector<uint64_t> point_nums;
  for (uint64_t degree : degrees_) {
    point_nums.push_back(1 << degree);
  }
  return point_nums;
}

}  // namespace tachyon
