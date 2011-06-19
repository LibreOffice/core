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
#include <sal/macros.h>

#include <systools/win32/uwinapi.h>
#include <../tools/seterror.hxx>

#define WININIT_FILENAME    "wininit.ini"
#define RENAME_SECTION      "rename"

#ifdef DEBUG
inline void OutputDebugStringFormat( LPCTSTR pFormat, ... )
{
    _TCHAR  buffer[1024];
    va_list args;

    va_start( args, pFormat );
    _vsntprintf( buffer, SAL_N_ELEMENTS(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCTSTR, ... )
{
}
#endif

static std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
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

static inline bool IsSetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty)
{
    std::_tstring value = GetMsiProperty(handle, sProperty);
    return (value.length() > 0);
}

static inline void UnsetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty)
{
    MsiSetProperty(handle, sProperty.c_str(), NULL);
}

static inline void SetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty)
{
    MsiSetProperty(handle, sProperty.c_str(), TEXT("1"));
}

static BOOL MoveFileEx9x( LPCSTR lpExistingFileNameA, LPCSTR lpNewFileNameA, DWORD dwFlags )
{
    BOOL    fSuccess = FALSE;   // assume failure

    // Windows 9x has a special mechanism to move files after reboot

    if ( dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT )
    {
        CHAR    szExistingFileNameA[MAX_PATH];
        CHAR    szNewFileNameA[MAX_PATH] = "NUL";

        // Path names in WININIT.INI must be in short path name form

        if (
            GetShortPathNameA( lpExistingFileNameA, szExistingFileNameA, MAX_PATH ) &&
            (!lpNewFileNameA || GetShortPathNameA( lpNewFileNameA, szNewFileNameA, MAX_PATH ))
            )
        {
            CHAR    szBuffer[32767];    // The buffer size must not exceed 32K
            DWORD   dwBufLen = GetPrivateProfileSectionA( RENAME_SECTION, szBuffer, SAL_N_ELEMENTS(szBuffer), WININIT_FILENAME );

            CHAR    szRename[MAX_PATH]; // This is enough for at most to times 67 chracters
            strcpy( szRename, szNewFileNameA );
            strcat( szRename, "=" );
            strcat( szRename, szExistingFileNameA );
            size_t  lnRename = strlen(szRename);

            if ( dwBufLen + lnRename + 2 <= SAL_N_ELEMENTS(szBuffer) )
            {
                CopyMemory( &szBuffer[dwBufLen], szRename, lnRename );
                szBuffer[dwBufLen + lnRename ] = 0;
                szBuffer[dwBufLen + lnRename + 1 ] = 0;

                fSuccess = WritePrivateProfileSectionA( RENAME_SECTION, szBuffer, WININIT_FILENAME );
            }
            else
                SetLastError( ERROR_BUFFER_OVERFLOW );
        }
    }
    else
    {

        fSuccess = MoveFileA( lpExistingFileNameA, lpNewFileNameA );

        if ( !fSuccess && GetLastError() != ERROR_ACCESS_DENIED &&
            0 != (dwFlags & (MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) )
        {
            BOOL    bFailIfExist = 0 == (dwFlags & MOVEFILE_REPLACE_EXISTING);

            fSuccess = CopyFileA( lpExistingFileNameA, lpNewFileNameA, bFailIfExist );

            if ( fSuccess )
                fSuccess = DeleteFileA( lpExistingFileNameA );
        }

    }

    return fSuccess;
}

static BOOL MoveFileExImpl( LPCSTR lpExistingFileNameA, LPCSTR lpNewFileNameA, DWORD dwFlags )
{
    if ( 0 > ((LONG)GetVersion())) // High order bit indicates Win 9x
        return MoveFileEx9x( lpExistingFileNameA, lpNewFileNameA, dwFlags );
    else
        return MoveFileExA( lpExistingFileNameA, lpNewFileNameA, dwFlags );
}

extern "C" UINT __stdcall IsOfficeRunning( MSIHANDLE handle )
{
    std::_tstring   sInstDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring   sResourceDir = sInstDir + TEXT("Basis\\program\\resource\\");
    std::_tstring   sPattern = sResourceDir + TEXT("vcl*.res");

    WIN32_FIND_DATA aFindFileData;
    HANDLE  hFind = FindFirstFile( sPattern.c_str(), &aFindFileData );

    if ( IsValidHandle(hFind) )
    {
        BOOL    fSuccess = false;
        bool    fRenameSucceeded;

        do
        {
            std::_tstring   sResourceFile = sResourceDir + aFindFileData.cFileName;
            std::_tstring   sIntermediate = sResourceFile + TEXT(".tmp");

            fRenameSucceeded = MoveFileExImpl( sResourceFile.c_str(), sIntermediate.c_str(), MOVEFILE_REPLACE_EXISTING );
            if ( fRenameSucceeded )
            {
                MoveFileExImpl( sIntermediate.c_str(), sResourceFile.c_str(), 0 );
                fSuccess = FindNextFile( hFind, &aFindFileData );
            }
        } while ( fSuccess && fRenameSucceeded );

        if ( !fRenameSucceeded )
        {
            MsiSetProperty(handle, TEXT("OFFICERUNS"), TEXT("1"));
            SetMsiErrorCode( MSI_ERROR_OFFICE_IS_RUNNING );
        }

        FindClose( hFind );
    }

    return ERROR_SUCCESS;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
