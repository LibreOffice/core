#include "macros.h"

IMPLEMENT_THUNK( kernel32, WINDOWS, DWORD, WINAPI, GetLongPathNameA, ( LPCTSTR lpShortPath, LPTSTR lpLongPath, DWORD cchBuffer ) )
#include "GetLongPathName.cpp"