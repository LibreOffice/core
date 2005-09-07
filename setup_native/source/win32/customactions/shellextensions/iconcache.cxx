/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iconcache.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:39:53 $
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
