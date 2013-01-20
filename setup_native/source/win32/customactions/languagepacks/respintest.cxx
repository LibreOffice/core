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

#undef UNICODE
#undef _UNICODE

#define _WIN32_WINDOWS 0x0410

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <../tools/msiprop.hxx>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <assert.h>

#include <tchar.h>
#include <string>
#include <sal/macros.h>

#include <systools/win32/uwinapi.h>
#include <../tools/seterror.hxx>

using namespace std;

extern "C" UINT __stdcall GetUserInstallMode(MSIHANDLE handle)
{
    string sOfficeInstallPath = GetMsiPropValue(handle, TEXT("INSTALLLOCATION"));

    // MessageBox(NULL, sOfficeInstallPath.c_str(), "DEBUG", MB_OK);

    // unsetting all properties

    MsiSetProperty( handle, TEXT("INVALIDDIRECTORY"), NULL );
    MsiSetProperty( handle, TEXT("ISWRONGPRODUCT"), NULL );
    MsiSetProperty( handle, TEXT("PATCHISOLDER"), NULL );
    MsiSetProperty( handle, TEXT("ALLUSERS"), NULL );

    // 1. Searching for "ProductCode" in setup.ini

    string sSetupiniPath = sOfficeInstallPath + TEXT("program\\setup.ini");

    TCHAR szValue[32767];

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("ProductCode"),
        TEXT("INVALIDDIRECTORY"),
        szValue,
        SAL_N_ELEMENTS(szValue),
        sSetupiniPath.c_str()
        );

    if ( !_tcsicmp( szValue, TEXT("INVALIDDIRECTORY") ) )
    {
        // No setup.ini or no "ProductCode" in setup.ini. This is an invalid directory.
        MsiSetProperty( handle, TEXT("INVALIDDIRECTORY"), TEXT("YES") );
        // MessageBox(NULL, "INVALIDDIRECTORY set, no setup.ini or ProductCode in setup.ini.", "DEBUG", MB_OK);
        SetMsiErrorCode( MSI_ERROR_INVALIDDIRECTORY );
        return ERROR_SUCCESS;
    }

    // 2. Comparing first three characters of "PRODUCTMAJOR" from property table and "buildid" from InfoFile

    szValue[0] = '\0';

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("buildid"),
        TEXT("ISWRONGPRODUCT"),
        szValue,
        SAL_N_ELEMENTS(szValue),
        sSetupiniPath.c_str()
        );

    if ( !_tcsicmp( szValue, TEXT("ISWRONGPRODUCT") ) )
    {
        MsiSetProperty( handle, TEXT("ISWRONGPRODUCT"), TEXT("YES") );
        // MessageBox(NULL, "ISWRONGPRODUCT 1 set after searching buildid", "DEBUG", MB_OK);
        SetMsiErrorCode( MSI_ERROR_ISWRONGPRODUCT );
        return ERROR_SUCCESS;
    }

    string ProductMajor = GetMsiPropValue(handle, TEXT("PRODUCTMAJOR"));

    // Comparing the first three characters, for example "680"
    // If not equal, this version is not suited for patch or language pack

    if (_tcsnicmp(ProductMajor.c_str(), szValue, 3))
    {
        MsiSetProperty( handle, TEXT("ISWRONGPRODUCT"), TEXT("YES") );
        // MessageBox(NULL, "ISWRONGPRODUCT 2 set after searching PRODUCTMAJOR", "DEBUG", MB_OK);
        SetMsiErrorCode( MSI_ERROR_ISWRONGPRODUCT );
        return ERROR_SUCCESS;
    }

    // 3. Setting property ALLUSERS with value from "setup.ini"

    szValue[0] = '\0';

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("ALLUSERS"),
        TEXT(""),
        szValue,
        SAL_N_ELEMENTS(szValue),
        sSetupiniPath.c_str()
        );

    if ( szValue[0] )
    {
        MsiSetProperty( handle, TEXT("ALLUSERS"), szValue );
        // MessageBox(NULL, "ALLUSERS set", "DEBUG", MB_OK);
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
