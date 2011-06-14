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

#define  UNICODE    1
#define _UNICODE    1

#ifndef _WINDOWS_
#   define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#   include <windows.h>
#   include <shellapi.h>
#   include <wchar.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#endif

#include "Resource.h"
#include <time.h>
#include "sal/config.h"
#include "tools/pathutils.hxx"

const DWORD PE_Signature = 0x00004550;

#define MY_LENGTH(s)        (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s)        (s), MY_LENGTH(s)
#define MAX_STR_CAPTION     256
#define MAX_TEXT_LENGTH     1024

static void failPath(wchar_t* pszAppTitle, wchar_t* pszMsg)
{
    MessageBoxW(NULL, pszMsg, pszAppTitle, MB_OK | MB_ICONERROR);
    TerminateProcess(GetCurrentProcess(), 255);
}

static void fail()
{
    LPWSTR buf = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
        GetLastError(), 0, reinterpret_cast< LPWSTR >(&buf), 0, NULL);
    MessageBoxW(NULL, buf, NULL, MB_OK | MB_ICONERROR);
    LocalFree(buf);
    TerminateProcess(GetCurrentProcess(), 255);
}

static LPVOID getVirtualBaseAddress( wchar_t* pszFilePath )
{
    HANDLE                hFile;
    HANDLE                hFileMapping;
    LPVOID                lpFileBase = 0;
    PIMAGE_DOS_HEADER     lpDosHeader;
    PIMAGE_NT_HEADERS     lpNTHeader;

    hFile = CreateFile(pszFilePath,
                       GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                       0);

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        return NULL;
    }

    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if ( hFileMapping == 0 )
    {
        CloseHandle(hFile);
        return NULL;
    }

    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if ( lpFileBase == 0 )
    {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return NULL;
    }

    lpDosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if ( lpDosHeader->e_magic == IMAGE_DOS_SIGNATURE )
    {
        lpNTHeader = (PIMAGE_NT_HEADERS)((char*)lpDosHeader + lpDosHeader->e_lfanew);
        if (lpNTHeader->Signature == PE_Signature )
            lpFileBase = reinterpret_cast<LPVOID>( lpNTHeader->OptionalHeader.ImageBase );
    }

    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);

    return lpFileBase;
}

static bool checkImageVirtualBaseAddress(wchar_t* pszFilePath, LPVOID lpVBA)
{
    LPVOID lpImageVBA = getVirtualBaseAddress(pszFilePath);
    if ( lpImageVBA == lpVBA )
        return true;
    else
        return false;
}

static wchar_t* getBrandPath(wchar_t * pszPath)
{
    DWORD n = GetModuleFileNameW(NULL, pszPath, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        exit(EXIT_FAILURE);
    }
    return tools::filename(pszPath);
}

extern "C" int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int )
{
    wchar_t* pAppTitle = new wchar_t[ MAX_STR_CAPTION ];
             pAppTitle[0]  = '\0';
    LoadString( hInst, IDS_APP_TITLE, pAppTitle, MAX_STR_CAPTION );

    wchar_t* pTextServer = new wchar_t[ MAX_TEXT_LENGTH ];
             pTextServer[0]  = '\0';
    LoadString( hInst, IDS_MSG_OPTIMIZED_FOR_SERVER, pTextServer, MAX_TEXT_LENGTH );

    wchar_t* pTextClient = new wchar_t[ MAX_TEXT_LENGTH ];
             pTextClient[0]  = '\0';
    LoadString( hInst, IDS_MSG_OPTIMIZED_FOR_CLIENT, pTextClient, MAX_TEXT_LENGTH );

    wchar_t* pTextNoInstallation = new wchar_t[ MAX_TEXT_LENGTH ];
             pTextNoInstallation[0]  = '\0';
    LoadString( hInst, IDS_MSG_NO_INSTALLATION_FOUND, pTextNoInstallation, MAX_TEXT_LENGTH );

    LPVOID  VBA = (void*)0x10000000;
    wchar_t path[MAX_PATH];

    wchar_t * pathEnd = getBrandPath(path);

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"libxml2.dll")) == NULL)
        fail();
    bool bFast = checkImageVirtualBaseAddress(path, VBA);

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"..\\basis-link")) == NULL)
        fail();
    pathEnd = tools::resolveLink(path);

    if (pathEnd == NULL)
        failPath(pAppTitle, pTextNoInstallation);

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"\\program\\vclmi.dll")) == NULL)
        fail();
    bFast &= checkImageVirtualBaseAddress(path, VBA);

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"\\ure-link")) == NULL)
        fail();
    pathEnd = tools::resolveLink(path);

    if (pathEnd == NULL)
        failPath(pAppTitle, pTextNoInstallation);

    if (tools::buildPath(path, path, pathEnd, MY_STRING(L"\\bin\\sal3.dll")) == NULL)
        fail();
    bFast &= checkImageVirtualBaseAddress(path, VBA);

    const wchar_t* pOutput = pTextClient;
    if (!bFast)
        pOutput = pTextServer;

    MessageBoxW( NULL, pOutput, pAppTitle, MB_OK );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
