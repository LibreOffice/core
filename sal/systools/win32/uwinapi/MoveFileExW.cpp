#include "macros.h"

IMPLEMENT_THUNK( kernel32, WINDOWS, BOOL, WINAPI, MoveFileExW, ( LPCWSTR lpExistingFileNameW, LPCWSTR lpNewFileNameW, DWORD dwFlags ) )
{
    AUTO_WSTR2STR( lpExistingFileName );
    AUTO_WSTR2STR( lpNewFileName );

    return MoveFileExA( lpExistingFileNameA, lpNewFileNameA, dwFlags );
}


