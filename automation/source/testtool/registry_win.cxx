/*************************************************************************
 *
 *  $RCSfile: registry_win.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 11:41:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
                aKey.GetBuffer(),
                &hRegKey ) == ERROR_SUCCESS )
    {
        LONG lRet;
        sal_Unicode PathW[_MAX_PATH];
        DWORD lSize = sizeof(PathW) / sizeof( sal_Unicode );
        DWORD Type = REG_SZ;

        lRet = RegQueryValueExW(hRegKey, aValueName.GetBuffer(), NULL, &Type, (LPBYTE)PathW, &lSize);

        if ((lRet == ERROR_SUCCESS) && (Type == REG_SZ))
        {
            aRet = String( PathW );
        }

        RegCloseKey(hRegKey);
    }
    return aRet;
}


