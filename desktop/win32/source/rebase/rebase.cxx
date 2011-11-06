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
        wcsncpy( szPattern + len, TEXT("*.dll"), sizeof(szPattern)/sizeof(szPattern[0]) - len );
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
                wcsncpy( szLibFilePath + len, aFindFileData.cFileName, sizeof(szLibFilePath)/sizeof(szLibFilePath[0]) - len );
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
