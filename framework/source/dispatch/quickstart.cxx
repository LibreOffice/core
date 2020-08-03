#include <framework/quickstart.hxx>

namespace framework
{
bool IsQuickstart = false;
bool GetIsQuickstart() { return IsQuickstart; }
void SetIsQuickstart(bool v) { IsQuickstart = v; }
}
