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

#include "precompiled_desktop.hxx"
#define UNICODE
#define _UNICODE

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <shellapi.h>
#include <imagehlp.h>
#include <wchar.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <time.h>
#include "sal/config.h"
#include <sal/macros.h>
#include "tools/pathutils.hxx"

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

const int   FORMAT_MESSAGE_SIZE = 4096;
const DWORD PE_Signature        = 0x00004550;
const DWORD BASEVIRTUALADDRESS  = 0x10000000;

namespace
{

bool IsValidHandle( HANDLE handle )
{
    return ((NULL != handle) && (INVALID_HANDLE_VALUE != handle));
}

void fail()
{
    LPWSTR buf = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
        GetLastError(), 0, reinterpret_cast< LPWSTR >(&buf), 0, NULL);
    MessageBoxW(NULL, buf, NULL, MB_OK | MB_ICONERROR);
    LocalFree(buf);
    TerminateProcess(GetCurrentProcess(), 255);
}

bool rebaseImage( wchar_t* pszFilePath, ULONG nNewImageBase)
{
    ULONG ulOldImageSize;
    ULONG_PTR lpOldImageBase;
    ULONG ulNewImageSize;
    ULONG_PTR lpNewImageBase  = nNewImageBase;
    ULONG     ulDateTimeStamp = 0;
    bool      bResult(false);

    char cszFilePath[_MAX_PATH+1] = {0};
    int nResult = WideCharToMultiByte(CP_ACP, 0, pszFilePath, -1, cszFilePath, _MAX_PATH, NULL, NULL);

    if (nResult != 0)
    {
        BOOL bResult = ReBaseImage(
            cszFilePath,
            "",
            TRUE,
            FALSE,
            FALSE,
            0,
            &ulOldImageSize,
            &lpOldImageBase,
            &ulNewImageSize,
            &lpNewImageBase,
            ulDateTimeStamp );
    }

    return bResult;
}

wchar_t* getBrandPath(wchar_t * path)
{
    DWORD n = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        exit(EXIT_FAILURE);
    }
    return tools::filename(path);
}

void rebaseImagesInFolder( wchar_t* pszFolder, DWORD nNewImageBase )
{
    wchar_t szPattern[MAX_PATH];
    wchar_t *lpLastSlash = wcsrchr( pszFolder, '\\' );
    if ( lpLastSlash )
    {
        size_t len = lpLastSlash - pszFolder + 1;
        wcsncpy( szPattern, pszFolder, len );
        wcsncpy( szPattern + len, TEXT("*.dll"), SAL_N_ELEMENTS(szPattern) - len );
    }

    WIN32_FIND_DATA aFindFileData;
    HANDLE  hFind = FindFirstFile( szPattern, &aFindFileData );

    if ( IsValidHandle(hFind) )
    {
        BOOL fSuccess = false;

        do
        {
            wchar_t szLibFilePath[MAX_PATH];
            wchar_t *lpLastSlash = wcsrchr( pszFolder, '\\' );
            if ( lpLastSlash )
            {
                size_t len = lpLastSlash - pszFolder + 1;
                wcsncpy( szLibFilePath, pszFolder, len );
                wcsncpy( szLibFilePath + len, aFindFileData.cFileName, SAL_N_ELEMENTS(szLibFilePath) - len );
            }

            rebaseImage( szLibFilePath, nNewImageBase );
            fSuccess = FindNextFile( hFind, &aFindFileData );
        }
        while ( fSuccess );

        FindClose( hFind );
    }
}

}

extern "C" int APIENTRY WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    wchar_t path[MAX_PATH];

    wchar_t * pathEnd = getBrandPath(path);

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"")) == NULL)
        fail();
    rebaseImagesInFolder(path, BASEVIRTUALADDRESS);

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"..\\basis-link")) == NULL)
        fail();
    pathEnd = tools::resolveLink(path);

    if ( pathEnd == NULL )
        return 0;

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"\\program\\")) == NULL)
        fail();
    rebaseImagesInFolder(path, BASEVIRTUALADDRESS);

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"\\ure-link")) == NULL)
        fail();
    pathEnd = tools::resolveLink(path);

    if ( pathEnd == NULL )
        return 0;

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"\\bin\\")) == NULL)
        fail();
    rebaseImagesInFolder(path, BASEVIRTUALADDRESS);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
