/*************************************************************************
 *
 *  $RCSfile: iconcache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-20 11:42:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>

#include <stdlib.h>

extern "C" UINT __stdcall RebuildShellIconCache(MSIHANDLE handle)
{
    // Rebuild icon cache on windows OS prior XP

    OSVERSIONINFO   osverinfo;

    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (
        GetVersionEx( &osverinfo ) &&
        VER_PLATFORM_WIN32_NT == osverinfo.dwPlatformId &&
        (
        5 < osverinfo.dwMajorVersion ||
        5 == osverinfo.dwMajorVersion && 0 < osverinfo.dwMinorVersion
        )
        )
    {
        return ERROR_SUCCESS;
    }

    HKEY    hKey;
    DWORD   dwDispostion;
    LONG    lError = RegCreateKeyEx( HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop\\WindowMetrics"), 0, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &hKey, &dwDispostion );

    if ( ERROR_SUCCESS == lError )
    {
        TCHAR   szValue[256];
        TCHAR   szTempValue[256];
        DWORD   cbValue = sizeof(szValue);
        DWORD   dwType;
        int     iSize = 0;

        lError = RegQueryValueEx( hKey, TEXT("Shell Icon Size"), 0, &dwType, (LPBYTE)szValue, &cbValue );

        if ( ERROR_SUCCESS == lError )
            iSize = atoi( szValue );

        if ( !iSize )
        {
            iSize = GetSystemMetrics( SM_CXICON );
            itoa( iSize, szValue, 10 );
            cbValue = strlen( szValue ) + 1;
            dwType = REG_SZ;
        }

        itoa( iSize + 1, szTempValue, 10 );
        lError = RegSetValueEx( hKey, TEXT("Shell Icon Size"), 0, dwType, (LPBYTE)szTempValue, strlen( szTempValue ) + 1 );

        LRESULT lResult = SendMessageTimeout(
             HWND_BROADCAST,
             WM_SETTINGCHANGE,
             SPI_SETNONCLIENTMETRICS,
             (LPARAM)TEXT("WindowMetrics"),
             SMTO_NORMAL|SMTO_ABORTIFHUNG,
             0, NULL);

        lError = RegSetValueEx( hKey, TEXT("Shell Icon Size"), 0, dwType, (LPBYTE)szValue, cbValue );

        lResult = SendMessageTimeout(
             HWND_BROADCAST,
             WM_SETTINGCHANGE,
             SPI_SETNONCLIENTMETRICS,
             (LPARAM)TEXT("WindowMetrics"),
             SMTO_NORMAL|SMTO_ABORTIFHUNG,
             0, NULL);

        lError = RegCloseKey( hKey );
    }

    return ERROR_SUCCESS;
}
