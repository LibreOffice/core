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
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <../tools/msiprop.hxx>
#include <shellapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <assert.h>
#include <string.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>


extern "C" UINT __stdcall copyExtensionData(MSIHANDLE handle) {

    std::_tstring sSourceDir = GetMsiPropValue( handle, TEXT("SourceDir") );
    std::_tstring sExtensionDir = sSourceDir + TEXT("extension\\");
    std::_tstring sPattern = sExtensionDir + TEXT("*.oxt");

    // Finding all oxt files in sExtensionDir

    WIN32_FIND_DATA aFindFileData;

    HANDLE hFindOxt = FindFirstFile( sPattern.c_str(), &aFindFileData );

    if ( hFindOxt != INVALID_HANDLE_VALUE )
    {
        bool fNextFile = false;
        bool bFailIfExist = true;

        std::_tstring sDestDir = GetMsiPropValue( handle, TEXT("INSTALLLOCATION") );
        std::_tstring sShareInstallDir = sDestDir + TEXT("share\\extension\\install\\");

        // creating directories
        std::_tstring sShareDir = sDestDir + TEXT("share");
        std::_tstring sExtDir = sShareDir + TEXT("\\extension");
        std::_tstring sExtInstDir = sExtDir + TEXT("\\install");
        CreateDirectory(sShareDir.c_str(), NULL);
        CreateDirectory(sExtDir.c_str(), NULL);
        CreateDirectory(sExtInstDir.c_str(), NULL);

        do
        {
            std::_tstring sOxtFile = aFindFileData.cFileName;

            std::_tstring sSourceFile = sExtensionDir + sOxtFile;
            std::_tstring sDestFile = sShareInstallDir + sOxtFile;

            CopyFile( sSourceFile.c_str(), sDestFile.c_str(), bFailIfExist );

            fNextFile = FindNextFile( hFindOxt, &aFindFileData );

        } while ( fNextFile );

        FindClose( hFindOxt );
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
