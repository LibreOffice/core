/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove

#define _SVTOOLS_SVDDE_DDEWRAP_CXX_

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include "ddewrap.hxx"

namespace binfilter
{

//------------------------------------------------------------------------

HSZ WINAPI DdeCreateStringHandleW_9x( DWORD idInst, LPCWSTR pszString, int )
{
    HSZ		hszResult;
    LPSTR	pszANSIString;
    int		nSize;

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

DWORD WINAPI DdeQueryStringW_9x( DWORD idInst, HSZ hsz, LPWSTR pszString, DWORD cchMax, int )
{
    DWORD	dwResult;
    LPSTR	pszANSIString;

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

}
