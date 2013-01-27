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

#define _WIN32_WINNT 0x0401

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
#include <assert.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>
#include <queue>
#include <stdio.h>

#include <systools/win32/uwinapi.h>
#include <../tools/seterror.hxx>

static std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring result;
    TCHAR szDummy[1] = TEXT("");
    DWORD nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return result;
}

static void UnsetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty)
{
    MsiSetProperty(handle, sProperty.c_str(), NULL);
}

static void SetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty, const std::_tstring&)
{
    MsiSetProperty(handle, sProperty.c_str(), TEXT("1"));
}

extern "C" UINT __stdcall CheckInstallDirectory(MSIHANDLE handle)
{
    std::_tstring sInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));
    std::_tstring sOfficeHostnamePath = GetMsiProperty(handle, TEXT("OFFICEDIRHOSTNAME"));

    // MessageBox(NULL, sInstallPath.c_str(), "DEBUG", MB_OK);

    // unsetting all properties

    UnsetMsiProperty( handle, TEXT("DIRECTORY_NOT_EMPTY") );

    // 1. Searching for file setup.ini

    std::_tstring sSetupIniPath = sInstallPath + sOfficeHostnamePath + TEXT("\\program\\setup.ini");

    WIN32_FIND_DATA data;
    HANDLE hdl = FindFirstFile(sSetupIniPath.c_str(), &data);

    // std::_tstring mystr = "Searching for " + sSetupIniPath;
    // MessageBox(NULL, mystr.c_str(), "DEBUG", MB_OK);

    if ( IsValidHandle(hdl) )
    {
        // setup.ini found -> directory cannot be used for installation.
        SetMsiProperty( handle, TEXT("DIRECTORY_NOT_EMPTY"), TEXT("1") );
        SetMsiErrorCode( MSI_ERROR_DIRECTORY_NOT_EMPTY );
        // std::_tstring notEmptyStr = "Directory is not empty. Please choose another installation directory.";
        // std::_tstring notEmptyTitle = "Directory not empty";
        // MessageBox(NULL, notEmptyStr.c_str(), notEmptyTitle.c_str(), MB_OK);
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
