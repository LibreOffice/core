/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _SVTOOLS_SVDDE_DDEWRAP_HXX_
#define _SVTOOLS_SVDDE_DDEWRAP_HXX_


namespace binfilter
{

#define DECLARE_WAPI_FUNC(func) \
    extern func##_PROC lpfn##func;

typedef HSZ (WINAPI *DdeCreateStringHandleW_PROC)( DWORD idInst, LPCWSTR pszString, int iCodePage );
typedef DWORD (WINAPI *DdeQueryStringW_PROC)( DWORD idInst, HSZ hsz, LPWSTR pszString, DWORD cchMax, int iCodePage );
typedef UINT (WINAPI *DdeInitializeW_PROC)( LPDWORD pidInst, PFNCALLBACK pfnCallback, DWORD afCmd, DWORD ulRes );


DECLARE_WAPI_FUNC( DdeCreateStringHandleW );
DECLARE_WAPI_FUNC( DdeQueryStringW );
DECLARE_WAPI_FUNC( DdeInitializeW );


#ifndef _SVTOOLS_SVDDE_DDEWRAP_CXX_
#define DdeCreateStringHandleW	lpfnDdeCreateStringHandleW
#define DdeQueryStringW			lpfnDdeQueryStringW
#define DdeInitializeW			lpfnDdeInitializeW
#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
