/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registry_win.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-03-26 13:42:11 $
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
#include "precompiled_automation.hxx"

////////////////////////////////////////////////////////////////////////////
////
////                  Windows ONLY
////
////////////////////////////////////////////////////////////////////////////


#include <tools/prewin.h>
#include "winreg.h"
#include <tools/postwin.h>
// as we define it ourselves further down the line we remove it here
#ifdef IS_ERROR
//    #undef IS_ERROR
#endif

//#include <tchar.h>
#include "registry_win.hxx"
#include <osl/thread.h>


String ReadRegistry( String aKey, String aValueName )
{
    String aRet;
    HKEY hBaseKey;
    if ( aKey.GetToken( 0, '\\' ).EqualsIgnoreCaseAscii( "HKEY_CURRENT_USER" ) )
        hBaseKey = HKEY_CURRENT_USER;
    else if ( aKey.GetToken( 0, '\\' ).EqualsIgnoreCaseAscii( "HKEY_LOCAL_MACHINE" ) )
        hBaseKey = HKEY_LOCAL_MACHINE;
    else
    {
        return aRet;
    }

    aKey.Erase( 0, aKey.Search( '\\' )+1 );

    HKEY hRegKey;

    if ( RegOpenKeyW( hBaseKey,
                reinterpret_cast<LPCWSTR>(aKey.GetBuffer()),
                &hRegKey ) == ERROR_SUCCESS )
    {
        LONG lRet;
        sal_Unicode PathW[_MAX_PATH];
        DWORD lSize = sizeof(PathW) / sizeof( sal_Unicode );
        DWORD Type = REG_SZ;

        lRet = RegQueryValueExW(hRegKey, reinterpret_cast<LPCWSTR>(aValueName.GetBuffer()), NULL, &Type, (LPBYTE)PathW, &lSize);

        if ((lRet == ERROR_SUCCESS) && (Type == REG_SZ))
        {
            aRet = String( PathW );
        }

        RegCloseKey(hRegKey);
    }
    return aRet;
}


