#include "uwinapi.h"

EXTERN_C LPITEMIDLIST WINAPI SHSimpleIDListFromPathW_Failure( LPCWSTR lpPathW )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return NULL;
}

EXTERN_C LPITEMIDLIST WINAPI SHSimpleIDListFromPathW_WINDOWS( LPCWSTR lpPathW )
{
    AUTO_WSTR2STR( lpPath );

    return SHSimpleIDListFromPathA( lpPathA );
}


EXTERN_C void WINAPI ResolveThunk_SHSimpleIDListFromPathW( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion < 0 )
        *lppfn = (FARPROC)SHSimpleIDListFromPathW_WINDOWS;
    else
    {
        FARPROC lpfnResult = GetProcAddress( LoadLibraryA( lpLibFileName ), MAKEINTRESOURCE(162) );
        if ( !lpfnResult )
            lpfnResult = (FARPROC)SHSimpleIDListFromPathW_Failure;

        *lppfn = lpfnResult;
    }
}


DEFINE_CUSTOM_THUNK( kernel32, GetLongPathNameW, DWORD, WINAPI, GetLongPathNameW, ( LPCWSTR lpShortPathW, LPWSTR lpLongPathW, DWORD cchBuffer ) );
