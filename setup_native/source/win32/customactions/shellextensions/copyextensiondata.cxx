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


#undef UNICODE
#undef _UNICODE

#define _WIN32_WINDOWS 0x0410

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <msiquery.h>
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

extern "C" UINT __stdcall copyExtensionData(MSIHANDLE handle) {

    std::_tstring sSourceDir = GetMsiProperty( handle, TEXT("SourceDir") );
    std::_tstring sExtensionDir = sSourceDir + TEXT("extension\\");
    std::_tstring sPattern = sExtensionDir + TEXT("*.oxt");

    // Finding all oxt files in sExtensionDir

    WIN32_FIND_DATA aFindFileData;

    HANDLE hFindOxt = FindFirstFile( sPattern.c_str(), &aFindFileData );

    if ( hFindOxt != INVALID_HANDLE_VALUE )
    {
        bool fNextFile = false;
        bool bFailIfExist = true;

        std::_tstring sDestDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
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
