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
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <assert.h>

#include <tchar.h>
#include <string>
#include <systools/win32/uwinapi.h>

#include <../tools/seterror.hxx>

using namespace std;

namespace
{
    string GetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        string  result;
        TCHAR   szDummy[1] = TEXT("");
        DWORD   nChars = 0;

        if (MsiGetProperty(handle, sProperty.c_str(), szDummy, &nChars) == ERROR_MORE_DATA)
        {
            DWORD nBytes = ++nChars * sizeof(TCHAR);
            LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
            ZeroMemory( buffer, nBytes );
            MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
            result = buffer;
        }
        return result;
    }

    inline bool IsSetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        return (GetMsiProperty(handle, sProperty).length() > 0);
    }

    inline void UnsetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        MsiSetProperty(handle, sProperty.c_str(), NULL);
    }

    inline void SetMsiProperty(MSIHANDLE handle, const string& sProperty, const string&)
    {
        MsiSetProperty(handle, sProperty.c_str(), TEXT("1"));
    }

    void stripFinalBackslash(std::string * path) {
        std::string::size_type i = path->size();
        if (i > 1) {
            --i;
            if ((*path)[i] == '\\') {
                path->erase(i);
            }
        }
    }
} // namespace

extern "C" UINT __stdcall CreateLayerLinks(MSIHANDLE handle)
{
    string sInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));

    string sUreInstallPath = sInstallPath + TEXT("URE");

    string sUreLinkPath = sInstallPath + TEXT("ure-link");

    if ( IsSetMsiProperty(handle, TEXT("ADMININSTALL")) )
    {
        sUreInstallPath = TEXT("..\\URE");
    }

    stripFinalBackslash(&sUreInstallPath);

    // creating ure-link

    HANDLE hfile = CreateFile(
        sUreLinkPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (IsValidHandle(hfile))
    {
        DWORD dummy;

        // Converting string into UTF-8 encoding and writing into file "ure-link"

        int nCharsRequired = MultiByteToWideChar( CP_ACP, 0, sUreInstallPath.c_str(), -1, NULL, 0 );
        if ( nCharsRequired )
        {
            LPWSTR  lpPathW = new WCHAR[nCharsRequired];
            if ( MultiByteToWideChar( CP_ACP, 0, sUreInstallPath.c_str(), -1, lpPathW, nCharsRequired ) )
            {
                nCharsRequired = WideCharToMultiByte( CP_UTF8, 0, lpPathW, -1, NULL, 0, NULL, NULL );
                if ( nCharsRequired )
                {
                    LPSTR   lpPathUTF8 = new CHAR[nCharsRequired];
                    WideCharToMultiByte( CP_UTF8, 0, lpPathW, -1, lpPathUTF8, nCharsRequired, NULL, NULL );

                    WriteFile( hfile, lpPathUTF8, strlen(lpPathUTF8) ,&dummy, 0 );

                    delete lpPathUTF8;
                }
            }

            delete lpPathW;
        }

        CloseHandle(hfile);
    }

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall RemoveLayerLinks(MSIHANDLE handle)
{
    string sInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));

    string sUreLinkPath = sInstallPath + TEXT("ure-link");
    string sUreDirName = sInstallPath + TEXT("URE\\bin");

    // Check, if URE is still installed
    bool ureDirExists = true;
    WIN32_FIND_DATA aFindData;
    HANDLE hFindContent = FindFirstFile( sUreDirName.c_str(), &aFindData );
    if ( hFindContent == INVALID_HANDLE_VALUE ) { ureDirExists = false; }
    FindClose( hFindContent );

    // Deleting link to URE layer, if URE dir no longer exists
    if ( ! ureDirExists )
    {
        DeleteFile(sUreLinkPath.c_str());
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
