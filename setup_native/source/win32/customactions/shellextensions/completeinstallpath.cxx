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

    TCHAR   szValue[8192];
    DWORD   nValueSize = sizeof(szValue);
    HKEY    hKey;
    std::_tstring   sInstDir;
    std::_tstring   mystr;

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
        // mystr = "Nothing to do, officedir is included into installlocation";
        // MessageBox( NULL, mystr.c_str(), "It is part of installlocation", MB_OK );
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

        // mystr = "ProductKey: " + sProductKey;
        // MessageBox( NULL, mystr.c_str(), "ProductKey", MB_OK );

        // mystr = "Checking registry";
        // MessageBox( NULL, mystr.c_str(), "registry search", MB_OK );

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
            // mystr = "Setting path to: " + sInstallLocation;
            // MessageBox( NULL, mystr.c_str(), "sInstallLocation", MB_OK );
        }
    }

    // mystr = "Ending with INSTALLLOCATION: " + sInstallLocation;
    // MessageBox( NULL, mystr.c_str(), "END", MB_OK );

    return ERROR_SUCCESS;
}
