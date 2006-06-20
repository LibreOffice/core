/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: regpatchactivex.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:41:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ************************************************************************/

#define UNICODE

#pragma warning(push,1) // disable warnings within system headers
#include <windows.h>
#include <msiquery.h>
#pragma warning(pop)

#include <string.h>
#include <malloc.h>
#include <stdio.h>

//----------------------------------------------------------
BOOL UnicodeEquals( wchar_t* pStr1, wchar_t* pStr2 )
{
    if ( pStr1 == NULL && pStr2 == NULL )
        return TRUE;
    else if ( pStr1 == NULL || pStr2 == NULL )
        return FALSE;

    while( *pStr1 == *pStr2 && *pStr1 && *pStr2 )
        pStr1++, pStr2++;

    return ( *pStr1 == 0 && *pStr2 == 0 );
}

//----------------------------------------------------------
BOOL GetMsiProp( MSIHANDLE hMSI, const wchar_t* pPropName, wchar_t** ppValue )
{
    DWORD sz = 0;
       if ( MsiGetProperty( hMSI, pPropName, L"", &sz ) == ERROR_MORE_DATA )
       {
           sz++;
           DWORD nbytes = sz * sizeof( wchar_t );
           wchar_t* buff = reinterpret_cast<wchar_t*>( malloc( nbytes ) );
           ZeroMemory( buff, nbytes );
           MsiGetProperty( hMSI, pPropName, buff, &sz );
           *ppValue = buff;

        return TRUE;
    }

    return FALSE;
}

//----------------------------------------------------------
BOOL MakeInstallForAllUsers( MSIHANDLE hMSI )
{
    BOOL bResult = FALSE;
    wchar_t* pVal = NULL;
    if ( GetMsiProp( hMSI, L"ALLUSERS", &pVal ) && pVal )
    {
        bResult = UnicodeEquals( pVal , L"1" );
        free( pVal );
    }

    return bResult;
}

//----------------------------------------------------------
extern "C" UINT __stdcall PatchActiveXControl( MSIHANDLE hMSI )
{
    // MessageBox(NULL, L"PatchActiveXControl", L"Information", MB_OK | MB_ICONINFORMATION);

    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_o_Activexcontrol", &current_state, &future_state ) )
    {
        BOOL bInstallForAllUsers = MakeInstallForAllUsers( hMSI );

        if ( future_state == INSTALLSTATE_LOCAL
          || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
        {
            HKEY hkey = NULL;
            char* aSubKey = "Software\\Classes\\MIME\\DataBase\\Content Type\\application/vnd.sun.xml.base";
               if ( ERROR_SUCCESS == RegCreateKeyA(bInstallForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, &hkey) )
            {
                   RegDeleteValueA( hkey, "CLSID" );
                RegCloseKey( hkey ), hkey = NULL;
            }
        }
    }
    else
    {
        // assert( FALSE );
    }

    return ERROR_SUCCESS;
}


