/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

{
    HKEY    hkeyLogon;
    HKEY    hkeyWorkgroup;
    DWORD   dwResult = 0;


    if ( ERROR_SUCCESS  == RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("Network\\Logon"),
        0, KEY_READ, &hkeyLogon ) )
    {
        DWORD   dwLogon = 0;
        DWORD   dwLogonSize = sizeof(dwLogon);
        RegQueryValueEx( hkeyLogon, TEXT("LMLogon"), 0, NULL, (LPBYTE)&dwLogon, &dwLogonSize );
        RegCloseKey( hkeyLogon );

        if ( dwLogon )
        {
            HKEY    hkeyNetworkProvider;

            if ( ERROR_SUCCESS  == RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Services\\MSNP32\\NetworkProvider"),
                0, KEY_READ, &hkeyNetworkProvider ) )
            {
                DWORD   dwBufferSize = nSize;
                LONG    lResult = RegQueryValueEx( hkeyNetworkProvider, TEXT("AuthenticatingAgent"), 0, NULL, (LPBYTE)lpBuffer, &dwBufferSize );

                if ( ERROR_SUCCESS == lResult || ERROR_MORE_DATA == lResult )
                    dwResult = dwBufferSize / sizeof(TCHAR);

                RegCloseKey( hkeyNetworkProvider );
            }
        }
    }
    else if ( ERROR_SUCCESS == RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\VxD\\VNETSUP"),
        0, KEY_READ, &hkeyWorkgroup ) )
    {
        DWORD   dwBufferSize = nSize;
        LONG    lResult = RegQueryValueEx( hkeyWorkgroup, TEXT("Workgroup"), 0, NULL, (LPBYTE)lpBuffer, &dwBufferSize );

        if ( ERROR_SUCCESS == lResult || ERROR_MORE_DATA == lResult )
            dwResult = dwBufferSize / sizeof(TCHAR);

        RegCloseKey( hkeyWorkgroup );
    }


    return dwResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
