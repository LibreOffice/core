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
#include <strsafe.h>

#include <systools/win32/uwinapi.h>
#include <../tools/seterror.hxx>

//----------------------------------------------------------
#ifdef DEBUG
inline void OutputDebugStringFormat( LPCSTR pFormat, ... )
{
    CHAR    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfA( buffer, sizeof(buffer), pFormat, args );
    OutputDebugStringA( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCSTR, ... )
{
}
#endif


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

static BOOL RemoveCompleteDirectory( std::_tstring sPath )
{
    bool bDirectoryRemoved = true;

    std::_tstring mystr;
    std::_tstring sPattern = sPath + TEXT("\\") + TEXT("*.*");
    WIN32_FIND_DATA aFindData;

    // Finding all content in sPath

    HANDLE hFindContent = FindFirstFile( sPattern.c_str(), &aFindData );

    if ( hFindContent != INVALID_HANDLE_VALUE )
    {
        bool fNextFile = false;
        std::_tstring sCurrentDir = TEXT(".");
        std::_tstring sParentDir = TEXT("..");

        do
        {
            std::_tstring sFileName = aFindData.cFileName;

            mystr = "Current short file: " + sFileName;
            // MessageBox(NULL, mystr.c_str(), "Current Content", MB_OK);

            if (( strcmp(sFileName.c_str(),sCurrentDir.c_str()) != 0 ) &&
                ( strcmp(sFileName.c_str(),sParentDir.c_str()) != 0 ))
            {
                std::_tstring sCompleteFileName = sPath + TEXT("\\") + sFileName;

                if ( aFindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
                {
                    bool fSuccess = RemoveCompleteDirectory(sCompleteFileName);
                    if ( fSuccess )
                    {
                        mystr = "Successfully removed content of dir " + sCompleteFileName;
                        // MessageBox(NULL, mystr.c_str(), "Removed Directory", MB_OK);
                    }
                    else
                    {
                        mystr = "An error occurred during removing content of " + sCompleteFileName;
                        // MessageBox(NULL, mystr.c_str(), "Error removing directory", MB_OK);
                    }
                }
                else
                {
                    bool fSuccess = DeleteFile( sCompleteFileName.c_str() );
                    if ( fSuccess )
                    {
                        mystr = "Successfully removed file " + sCompleteFileName;
                        // MessageBox(NULL, mystr.c_str(), "Removed File", MB_OK);
                    }
                    else
                    {
                        mystr = "An error occurred during removal of file " + sCompleteFileName;
                        // MessageBox(NULL, mystr.c_str(), "Error removing file", MB_OK);
                    }
                }
            }

            fNextFile = FindNextFile( hFindContent, &aFindData );

        } while ( fNextFile );

        FindClose( hFindContent );

        // empty directory can be removed now
        // RemoveDirectory is only successful, if the last handle to the directory is closed
        // -> first removing content -> closing handle -> remove empty directory

        bool fRemoveDirSuccess = RemoveDirectory(sPath.c_str());

        if ( fRemoveDirSuccess )
        {
            mystr = "Successfully removed dir " + sPath;
            // MessageBox(NULL, mystr.c_str(), "Removed Directory", MB_OK);
        }
        else
        {
            mystr = "An error occurred during removal of empty directory " + sPath;
            // MessageBox(NULL, mystr.c_str(), "Error removing directory", MB_OK);
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

//    MessageBox(NULL, sRenameSrc.c_str(), "INSTALLLOCATION", MB_OK);

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

#if 0
    if ( !bSuccess )
        MessageBox(NULL, "Renaming folder failed", "RenamePrgFolder", MB_OK);
    else
        MessageBox(NULL, "Renaming folder successful", "RenamePrgFolder", MB_OK);
#endif

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall RemovePrgFolder( MSIHANDLE handle )
{
    std::_tstring sOfficeInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));
    std::_tstring sRemoveDir = sOfficeInstallPath + TEXT("program_old");

//    MessageBox(NULL, sRemoveDir.c_str(), "REMOVING OLD DIR", MB_OK);

    bool bSuccess = RemoveCompleteDirectory( sRemoveDir );

    TCHAR sAppend[2] = TEXT("0");
    for ( int i = 0; i < 10; i++ )
    {
        sRemoveDir = sOfficeInstallPath + TEXT("program_old") + sAppend;
        bSuccess = RemoveCompleteDirectory( sRemoveDir );
        sAppend[0] += 1;
    }

#if 0
    if ( bSuccess )
        MessageBox(NULL, "Removing folder successful", "RemovePrgFolder", MB_OK);
    else
        MessageBox(NULL, "Removing folder failed", "RemovePrgFolder", MB_OK);
#endif

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
