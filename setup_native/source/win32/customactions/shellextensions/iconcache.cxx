/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iconcache.cxx,v $
 * $Revision: 1.6 $
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

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <stdlib.h>

extern "C" UINT __stdcall RebuildShellIconCache(MSIHANDLE)
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
