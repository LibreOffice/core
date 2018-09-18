/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "shlxtmsi.hxx"

#include <malloc.h>

extern "C" __declspec(dllexport) UINT __stdcall CompleteInstallPath( MSIHANDLE handle )
{
    // This CustomAction is necessary for updates from OOo 3.0, OOo 3.1 and OOo 3.2 to versions
    // OOo 3.3 or later. This is caused by a change of INSTALLLOCATION, that starting with OOo 3.3
    // contains the name of the product again (instead of only "c:\program files"). Unfortunately
    // this causes in an update installation, that INSTALLLOCATION is set to "c:\program files",
    // so that in an OOo 3.3 or later, the directory "program" or "share" are directly created
    // below "c:\program files".

    HKEY    hKey;

    // Reading property OFFICEDIRHOSTNAME_, that contains the part of the path behind
    // the program files folder.

    std::wstring   sInstallLocation = GetMsiPropertyW( handle, L"INSTALLLOCATION" );
    std::wstring   sOfficeDirHostname = GetMsiPropertyW( handle, L"OFFICEDIRHOSTNAME_" );

    // If sInstallLocation ends with (contains) the string sOfficeDirHostname,
    // INSTALLLOCATION is good and nothing has to be done here.

    bool pathCompletionRequired = true;

    if ( wcsstr( sInstallLocation.c_str(), sOfficeDirHostname.c_str() ) )
    {
        pathCompletionRequired = false;  // nothing to do
    }

    // If the path INSTALLLOCATION does not end with this string, INSTALLLOCATION is maybe
    // transferred from an OOo 3.0, OOo 3.1 and OOo 3.2 and need to be changed therefore.

    if ( pathCompletionRequired )
    {
        std::wstring   sManufacturer = GetMsiPropertyW( handle, L"Manufacturer" );
        std::wstring   sDefinedName = GetMsiPropertyW( handle, L"DEFINEDPRODUCT" );
        std::wstring   sUpgradeCode = GetMsiPropertyW( handle, L"UpgradeCode" );

        // sUpdateVersion can be "3.0", "3.1" or "3.2"

        std::wstring   sProductKey30 = L"Software\\" + sManufacturer + L"\\" + sDefinedName +
                                            L"\\" L"3.0" L"\\" + sUpgradeCode;

        std::wstring   sProductKey31 = L"Software\\" + sManufacturer + L"\\" + sDefinedName +
                                            L"\\" L"3.1" L"\\" + sUpgradeCode;

        std::wstring   sProductKey32 = L"Software\\" + sManufacturer + L"\\" + sDefinedName +
                                            L"\\" L"3.2" L"\\" + sUpgradeCode;

        bool oldVersionExists = false;

        if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_CURRENT_USER,  sProductKey30.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_CURRENT_USER,  sProductKey31.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_CURRENT_USER,  sProductKey32.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_LOCAL_MACHINE,  sProductKey30.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_LOCAL_MACHINE,  sProductKey31.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_LOCAL_MACHINE,  sProductKey32.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }

        if ( oldVersionExists )
        {
            // Adding the new path content sOfficeDirHostname
            sInstallLocation = sInstallLocation + sOfficeDirHostname;
            // Setting the new property value
            MsiSetPropertyW(handle, L"INSTALLLOCATION", sInstallLocation.c_str());
        }
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
