#include "macros.h"

IMPLEMENT_THUNK( kernel32, WINDOWS, BOOL, WINAPI, GetDiskFreeSpaceExW,(
  LPCWSTR lpRootPathNameW,                  // directory name
  PULARGE_INTEGER lpFreeBytesAvailable,    // bytes available to caller
  PULARGE_INTEGER lpTotalNumberOfBytes,    // bytes on disk
  PULARGE_INTEGER lpTotalNumberOfFreeBytes // free bytes on disk
))
{
    AUTO_WSTR2STR( lpRootPathName );

    return GetDiskFreeSpaceExA( lpRootPathNameA, lpFreeBytesAvailable, lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes );
}

