#define _SVTOOLS_SVDDE_DDEWRAP_CXX_

#include <windows.h>
#include "ddewrap.hxx"

//------------------------------------------------------------------------

HSZ WINAPI DdeCreateStringHandleW_9x( DWORD idInst, LPCWSTR pszString, int iCodePage )
{
    HSZ     hszResult;
    LPSTR   pszANSIString;
    int     nSize;

    nSize = pszString ? WideCharToMultiByte( CP_ACP, 0, pszString, -1, NULL, 0, NULL, NULL ) : 0;
    pszANSIString = nSize ? (LPSTR)HeapAlloc( GetProcessHeap(), 0, nSize * sizeof(CHAR) ) : NULL;

    if ( pszANSIString )
        WideCharToMultiByte( CP_ACP, 0, pszString, -1, pszANSIString, nSize, NULL, NULL );

    hszResult = DdeCreateStringHandleA( idInst, pszANSIString, CP_WINANSI );

    if ( pszANSIString )
        HeapFree( GetProcessHeap(), 0, pszANSIString );

    return hszResult;
}

//------------------------------------------------------------------------

DWORD WINAPI DdeQueryStringW_9x( DWORD idInst, HSZ hsz, LPWSTR pszString, DWORD cchMax, int iCodePage )
{
    DWORD   dwResult;
    LPSTR   pszANSIString;

    pszANSIString = cchMax ? (LPSTR)HeapAlloc( GetProcessHeap(), 0, cchMax * sizeof(CHAR) ) : NULL;

    dwResult = DdeQueryStringA( idInst, hsz, pszANSIString, cchMax, CP_WINANSI );

    if ( dwResult && pszANSIString )
        MultiByteToWideChar( CP_ACP, 0, pszANSIString, -1, pszString, cchMax );

    if ( pszANSIString )
        HeapFree( GetProcessHeap(), 0, pszANSIString );

    return dwResult;
}

//------------------------------------------------------------------------

UINT WINAPI DdeInitializeW_9x( LPDWORD pidInst, PFNCALLBACK pfnCallback, DWORD afCmd, DWORD ulRes )
{
    return DdeInitializeA( pidInst, pfnCallback, afCmd, ulRes );
}

//------------------------------------------------------------------------

#define DEFINE_WAPI_FUNC(func) \
func##_PROC lpfn##func = (LONG)GetVersion() >= 0 ? func : func##_9x;


DEFINE_WAPI_FUNC( DdeCreateStringHandleW );
DEFINE_WAPI_FUNC( DdeQueryStringW );
DEFINE_WAPI_FUNC( DdeInitializeW );

