/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
