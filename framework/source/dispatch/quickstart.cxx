#include <framework/quickstart.hxx>

namespace framework
{
static bool IsQuickstart = false;
bool GetIsQuickstart() { return IsQuickstart; }
void SetIsQuickstart(bool v) { IsQuickstart = v; }
}
