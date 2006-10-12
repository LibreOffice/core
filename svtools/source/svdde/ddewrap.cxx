/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ddewrap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 15:27:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
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

