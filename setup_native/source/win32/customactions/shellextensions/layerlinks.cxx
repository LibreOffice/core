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

    string sOfficeInstallPath = sInstallPath;
    string sBasisInstallPath = sInstallPath + TEXT("Basis\\");
    string sUreInstallPath = sInstallPath + TEXT("URE\\");

    string sBasisLinkPath = sInstallPath + TEXT("basis-link");
    string sUreLinkPath = sInstallPath + TEXT("Basis\\ure-link");

    if ( IsSetMsiProperty(handle, TEXT("ADMININSTALL")) )
    {
        sBasisInstallPath = TEXT("Basis");
        sUreInstallPath = TEXT("..\\URE");
    }

    stripFinalBackslash(&sBasisInstallPath);
    stripFinalBackslash(&sUreInstallPath);

    // creating basis-link in brand layer

    HANDLE h1file = CreateFile(
        sBasisLinkPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (IsValidHandle(h1file))
    {
        DWORD dummy;

        // Converting string into UTF-8 encoding and writing into file "basis-link"

        int nCharsRequired = MultiByteToWideChar( CP_ACP, 0, sBasisInstallPath.c_str(), -1, NULL, 0 );
        if ( nCharsRequired )
        {
            LPWSTR  lpPathW = new WCHAR[nCharsRequired];
            if ( MultiByteToWideChar( CP_ACP, 0, sBasisInstallPath.c_str(), -1, lpPathW, nCharsRequired ) )
            {
                nCharsRequired = WideCharToMultiByte( CP_UTF8, 0, lpPathW, -1, NULL, 0, NULL, NULL );
                if ( nCharsRequired )
                {
                    LPSTR   lpPathUTF8 = new CHAR[nCharsRequired];
                    WideCharToMultiByte( CP_UTF8, 0, lpPathW, -1, lpPathUTF8, nCharsRequired, NULL, NULL );

                    WriteFile( h1file, lpPathUTF8, strlen(lpPathUTF8) ,&dummy, 0 );

                    delete lpPathUTF8;
                }
            }

            delete lpPathW;
        }

        CloseHandle(h1file);
    }

    // creating ure-link in basis layer

    HANDLE h2file = CreateFile(
        sUreLinkPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (IsValidHandle(h2file))
    {
        DWORD dummy;

        // Converting string into UTF-8 encoding and writing into file "basis-link"

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

                    WriteFile( h2file, lpPathUTF8, strlen(lpPathUTF8) ,&dummy, 0 );

                    delete lpPathUTF8;
                }
            }

            delete lpPathW;
        }

        CloseHandle(h2file);
    }

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall RemoveLayerLinks(MSIHANDLE handle)
{
    string sInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));

    string sOfficeInstallPath = sInstallPath;
    string sBasisInstallPath = sInstallPath + TEXT("Basis\\");
    string sUreInstallPath = sInstallPath + TEXT("URE\\");

    string sBasisLinkPath = sOfficeInstallPath + TEXT("basis-link");
    string sUreLinkPath = sBasisInstallPath + TEXT("ure-link");
    string sUreDirName = sUreInstallPath + TEXT("bin");

    // Deleting link to basis layer
    DeleteFile(sBasisLinkPath.c_str());

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
