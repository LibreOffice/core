#include <framework/quickstart.hxx>

namespace framework {
  bool IsQuickstart = false;
  __attribute__((visibility("default"))) bool GetIsQuickstart() {
    return IsQuickstart;
  }
  __attribute__((visibility("default"))) void SetIsQuickstart(bool v) {
    IsQuickstart = v;
  }
}
