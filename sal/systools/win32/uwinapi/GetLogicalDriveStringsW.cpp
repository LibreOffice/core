#include "macros.h"

IMPLEMENT_THUNK( kernel32, WINDOWS, DWORD, WINAPI, GetLogicalDriveStringsW, ( DWORD cchBuffer, LPWSTR lpBufferW ) )
{
    AUTO_STR( lpBuffer, cchBuffer );

    DWORD   dwResult = GetLogicalDriveStringsA( cchBuffer, lpBufferA );


    if ( dwResult && dwResult < cchBuffer )
        STRBUF2WSTR( lpBuffer, dwResult + 1, cchBuffer );

    return dwResult;
}
