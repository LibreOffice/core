/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GetLongPathNameW.cpp,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:17:35 $
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

#define UNICODE
#include "macros.h"

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
    if ( (LONG)GetVersion() < 0 )
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
