#define _UWINAPI_
#define UNICODE
#include "uwinapi.h"

EXTERN_C DWORD WINAPI GetLongPathNameW_NT( LPCWSTR lpShortPath, LPWSTR lpLongPath, DWORD cchBuffer )
#include "GetLongPathName.cpp"

EXTERN_C DWORD WINAPI GetLongPathNameW_WINDOWS( LPCWSTR lpShortPathW, LPWSTR lpLongPathW, DWORD cchBuffer )
{
    AUTO_WSTR2STR( lpShortPath );
    AUTO_STR( lpLongPath, cchBuffer );

    DWORD   dwResult = GetLongPathNameA( lpShortPathA, lpLongPathA, cchBuffer );

    if ( dwResult && dwResult < cchBuffer )
        STR2WSTR( lpLongPath, cchBuffer );

    return dwResult;
}


EXTERN_C void WINAPI ResolveThunk_GetLongPathNameW( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion < 0 )
        *lppfn = (FARPROC)GetLongPathNameW_WINDOWS;
    else
    {
        FARPROC lpfnResult = GetProcAddress( LoadLibraryA( lpLibFileName ), lpFuncName );
        if ( !lpfnResult )
            lpfnResult = (FARPROC)GetLongPathNameW_NT;

        *lppfn = lpfnResult;
    }
}


DEFINE_CUSTOM_THUNK( kernel32, GetLongPathNameW, DWORD, WINAPI, GetLongPathNameW, ( LPCWSTR lpShortPathW, LPWSTR lpLongPathW, DWORD cchBuffer ) );
