/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#define _SVTOOLS_SVDDE_DDEWRAP_CXX_

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include "ddewrap.hxx"

//------------------------------------------------------------------------

HSZ WINAPI DdeCreateStringHandleW_9x( DWORD idInst, LPCWSTR pszString, int )
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

DWORD WINAPI DdeQueryStringW_9x( DWORD idInst, HSZ hsz, LPWSTR pszString, DWORD cchMax, int )
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

