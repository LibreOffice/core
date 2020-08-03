// REVIEW QUESTION:
// This feels horrible, but I wasn't able to find a cleaner way
// to pass information from Desktop::GetCommandLineArgs()
// to CloseDispatcher.

#ifndef INCLUDED_FRAMEWORK_QUICKSTART_HXX
#define INCLUDED_FRAMEWORK_QUICKSTART_HXX

namespace framework {
  __attribute__((visibility("default"))) bool GetIsQuickstart();
  __attribute__((visibility("default"))) void SetIsQuickstart(bool v);
}

#endif
