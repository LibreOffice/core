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

#define _WIN32_WINDOWS 0x0410
#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>

using namespace std;

namespace
{
    std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
    {
        std::_tstring   result;
        TCHAR   szDummy[1] = TEXT("");
        DWORD   nChars = 0;

        if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
        {
            DWORD nBytes = ++nChars * sizeof(TCHAR);
            LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
            ZeroMemory( buffer, nBytes );
            MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
            result = buffer;
        }

        return  result;
    }
} // namespace

extern "C" UINT __stdcall CompleteInstallPath( MSIHANDLE handle )
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

    std::_tstring   sInstallLocation = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring   sOfficeDirHostname = GetMsiProperty( handle, TEXT("OFFICEDIRHOSTNAME_") );

    // If sInstallLocation ends with (contains) the string sOfficeDirHostname,
    // INSTALLLOCATION is good and nothing has to be done here.

    bool pathCompletionRequired = true;

    if ( _tcsstr( sInstallLocation.c_str(), sOfficeDirHostname.c_str() ) )
    {
        pathCompletionRequired = false;  // nothing to do
    }

    // If the path INSTALLLOCATION does not end with this string, INSTALLLOCATION is maybe
    // transfered from an OOo 3.0, OOo 3.1 and OOo 3.2 and need to be changed therefore.

    if ( pathCompletionRequired )
    {
        std::_tstring   sManufacturer = GetMsiProperty( handle, TEXT("Manufacturer") );
        std::_tstring   sDefinedName = GetMsiProperty( handle, TEXT("DEFINEDPRODUCT") );
        std::_tstring   sUpgradeCode = GetMsiProperty( handle, TEXT("UpgradeCode") );

        // sUpdateVersion can be "3.0", "3.1" or "3.2"

        std::_tstring   sProductKey30 = "Software\\" + sManufacturer + "\\" + sDefinedName +
                                            "\\" + "3.0" + "\\" + sUpgradeCode;

        std::_tstring   sProductKey31 = "Software\\" + sManufacturer + "\\" + sDefinedName +
                                            "\\" + "3.1" + "\\" + sUpgradeCode;

        std::_tstring   sProductKey32 = "Software\\" + sManufacturer + "\\" + sDefinedName +
                                            "\\" + "3.2" + "\\" + sUpgradeCode;

        bool oldVersionExists = false;

        if ( ERROR_SUCCESS == RegOpenKey( HKEY_CURRENT_USER,  sProductKey30.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKey( HKEY_CURRENT_USER,  sProductKey31.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKey( HKEY_CURRENT_USER,  sProductKey32.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  sProductKey30.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  sProductKey31.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }
        else if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  sProductKey32.c_str(), &hKey ) )
        {
            oldVersionExists = true;
            RegCloseKey( hKey );
        }

        if ( oldVersionExists )
        {
            // Adding the new path content sOfficeDirHostname
            sInstallLocation = sInstallLocation + sOfficeDirHostname;
            // Setting the new property value
            MsiSetProperty(handle, TEXT("INSTALLLOCATION"), sInstallLocation.c_str());
        }
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
