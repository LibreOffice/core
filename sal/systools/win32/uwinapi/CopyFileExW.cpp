#define _WIN32_WINNT 0x0400
#include "macros.h"

IMPLEMENT_THUNK( kernel32, WINDOWS, BOOL, WINAPI, CopyFileExW, ( LPCWSTR lpExistingFileNameW, LPCWSTR lpNewFileNameW, LPPROGRESS_ROUTINE  lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags ) )
{
    AUTO_WSTR2STR( lpExistingFileName );
    AUTO_WSTR2STR( lpNewFileName );

    return CopyFileExA( lpExistingFileNameA, lpNewFileNameA, lpProgressRoutine, lpData, pbCancel, dwCopyFlags );
}
