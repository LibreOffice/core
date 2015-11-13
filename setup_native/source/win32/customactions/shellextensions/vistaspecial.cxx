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
#include <strsafe.h>

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

static BOOL RemoveCompleteDirectory(const std::_tstring& rPath)
{
    bool bDirectoryRemoved = true;

    std::_tstring sPattern = rPath + TEXT("\\") + TEXT("*.*");
    WIN32_FIND_DATA aFindData;

    // Finding all content in rPath

    HANDLE hFindContent = FindFirstFile( sPattern.c_str(), &aFindData );

    if ( hFindContent != INVALID_HANDLE_VALUE )
    {
        bool fNextFile = false;
        std::_tstring sCurrentDir = TEXT(".");
        std::_tstring sParentDir = TEXT("..");

        do
        {
            std::_tstring sFileName = aFindData.cFileName;

            if (( strcmp(sFileName.c_str(),sCurrentDir.c_str()) != 0 ) &&
                ( strcmp(sFileName.c_str(),sParentDir.c_str()) != 0 ))
            {
                std::_tstring sCompleteFileName = rPath + TEXT("\\") + sFileName;

                if ( aFindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
                {
                    RemoveCompleteDirectory(sCompleteFileName);
                }
                else
                {
                    DeleteFile( sCompleteFileName.c_str() );
                }
            }

            fNextFile = FindNextFile( hFindContent, &aFindData );

        } while ( fNextFile );

        FindClose( hFindContent );

        // empty directory can be removed now
        // RemoveDirectory is only successful, if the last handle to the directory is closed
        // -> first removing content -> closing handle -> remove empty directory


        if( !( RemoveDirectory(rPath.c_str()) ) )
        {
            bDirectoryRemoved = false;
        }
    }

    return bDirectoryRemoved;
}

extern "C" UINT __stdcall RenamePrgFolder( MSIHANDLE handle )
{
    std::_tstring sOfficeInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));

    std::_tstring sRenameSrc = sOfficeInstallPath + TEXT("program");
    std::_tstring sRenameDst = sOfficeInstallPath + TEXT("program_old");

    bool bSuccess = MoveFile( sRenameSrc.c_str(), sRenameDst.c_str() );
    if ( !bSuccess )
    {
        TCHAR sAppend[2] = TEXT("0");
        for ( int i = 0; i < 10; i++ )
        {
            sRenameDst = sOfficeInstallPath + TEXT("program_old") + sAppend;
            bSuccess = MoveFile( sRenameSrc.c_str(), sRenameDst.c_str() );
            if ( bSuccess )
                break;
            sAppend[0] += 1;
        }
    }

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall RemovePrgFolder( MSIHANDLE handle )
{
    std::_tstring sOfficeInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));
    std::_tstring sRemoveDir = sOfficeInstallPath + TEXT("program_old");

    RemoveCompleteDirectory( sRemoveDir );

    TCHAR sAppend[2] = TEXT("0");
    for ( int i = 0; i < 10; i++ )
    {
        sRemoveDir = sOfficeInstallPath + TEXT("program_old") + sAppend;
        RemoveCompleteDirectory( sRemoveDir );
        sAppend[0] += 1;
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
