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
#include <assert.h>

#include <queue>
#include <stdio.h>

#include <systools/win32/uwinapi.h>
#include <../tools/seterror.hxx>

extern "C" UINT __stdcall CheckInstallDirectory(MSIHANDLE handle)
{
    std::wstring sInstallPath = GetMsiPropertyW(handle, L"INSTALLLOCATION");
    std::wstring sOfficeHostnamePath = GetMsiPropertyW(handle, L"OFFICEDIRHOSTNAME");

    // MessageBoxW(NULL, sInstallPath.c_str(), L"DEBUG", MB_OK);

    // unsetting all properties

    UnsetMsiPropertyW( handle, L"DIRECTORY_NOT_EMPTY" );

    // 1. Searching for file setup.ini

    std::wstring sSetupIniPath = sInstallPath + sOfficeHostnamePath + L"\\program\\setup.ini";

    WIN32_FIND_DATAW data;
    HANDLE hdl = FindFirstFileW(sSetupIniPath.c_str(), &data);

    // std::wstring mystr = L"Searching for " + sSetupIniPath;
    // MessageBoxW(NULL, mystr.c_str(), L"DEBUG", MB_OK);

    if ( IsValidHandle(hdl) )
    {
        // setup.ini found -> directory cannot be used for installation.
        SetMsiPropertyW( handle, L"DIRECTORY_NOT_EMPTY", L"1" );
        SetMsiErrorCode( MSI_ERROR_DIRECTORY_NOT_EMPTY );
        // std::wstring notEmptyStr = L"Directory is not empty. Please choose another installation directory.";
        // std::wstring notEmptyTitle = L"Directory not empty";
        // MessageBoxW(NULL, notEmptyStr.c_str(), notEmptyTitle.c_str(), MB_OK);
        FindClose(hdl);
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
