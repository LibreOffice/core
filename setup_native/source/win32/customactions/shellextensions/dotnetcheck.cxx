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

    inline void SetMsiProperty(MSIHANDLE handle, const string& sProperty, const string& sValue)
    {
        MsiSetProperty(handle, sProperty.c_str(), sValue.c_str());
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

// Copied more or less verbatim from
// desktop/source/deployment/inc/dp_version.hxx:1.5 and
// desktop/source/deployment/misc/dp_version.cxx:1.5:

enum Order { ORDER_LESS, ORDER_EQUAL, ORDER_GREATER };

string getElement(string const & version, string::size_type * index) {
    while (*index < version.size() && version[*index] == '0') {
        ++*index;
    }
    string::size_type i = *index;
    *index = version.find('.', i);
    if (*index == string::npos) {
        *index = version.size();
        return string(version, i);
    } else {
        ++*index;
        return string(version, i, *index - 1 - i);
    }
}

Order compareVersions(string const & version1, string const & version2) {
    for (string::size_type i1 = 0, i2 = 0;
         i1 < version1.size() || i2 < version2.size();)
    {
        string e1(getElement(version1, &i1));
        string e2(getElement(version2, &i2));

        // string myText1 = TEXT("e1: ") + e1;
        // string myText2 = TEXT("e2: ") + e2;
        // MessageBox(NULL, myText1.c_str(), "DEBUG", MB_OK);
        // MessageBox(NULL, myText2.c_str(), "DEBUG", MB_OK);

        if (e1.size() < e2.size()) {
            return ORDER_LESS;
        } else if (e1.size() > e2.size()) {
            return ORDER_GREATER;
        } else if (e1 < e2) {
            return ORDER_LESS;
        } else if (e1 > e2) {
            return ORDER_GREATER;
        }
    }
    return ORDER_EQUAL;
}

} // namespace

extern "C" UINT __stdcall DotNetCheck(MSIHANDLE handle) {
    string present(GetMsiProperty(handle, TEXT("MsiNetAssemblySupport")));
    string required(GetMsiProperty(handle, TEXT("REQUIRED_DOTNET_VERSION")));

    // string myText1 = TEXT("MsiNetAssemblySupport: ") + present;
    // string myText2 = TEXT("REQUIRED_DOTNET_VERSION: ") + required;
    // MessageBox(NULL, myText1.c_str(), "DEBUG", MB_OK);
    // MessageBox(NULL, myText2.c_str(), "DEBUG", MB_OK);

    SetMsiProperty(
        handle, TEXT("DOTNET_SUFFICIENT"),
        (present.empty() || compareVersions(present, required) == ORDER_LESS ?
         TEXT("0") : TEXT("1")));

    // string result(GetMsiProperty(handle, TEXT("DOTNET_SUFFICIENT")));
    // string myText3 = TEXT("DOTNET_SUFFICIENT: ") + result;
    // MessageBox(NULL, myText3.c_str(), "DEBUG", MB_OK);


    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall ShowProperties(MSIHANDLE handle)
{
    string property = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));
    string myText = TEXT("INSTALLLOCATION: ") + property;
    MessageBox(NULL, myText.c_str(), "INSTALLLOCATION", MB_OK);

    property = GetMsiProperty(handle, TEXT("Installed"));
    myText = TEXT("Installed: ") + property;
    MessageBox(NULL, myText.c_str(), "Installed", MB_OK);

    property = GetMsiProperty(handle, TEXT("PATCH"));
    myText = TEXT("PATCH: ") + property;
    MessageBox(NULL, myText.c_str(), "PATCH", MB_OK);

    property = GetMsiProperty(handle, TEXT("REMOVE"));
    myText = TEXT("REMOVE: ") + property;
    MessageBox(NULL, myText.c_str(), "REMOVE", MB_OK);

    property = GetMsiProperty(handle, TEXT("ALLUSERS"));
    myText = TEXT("ALLUSERS: ") + property;
    MessageBox(NULL, myText.c_str(), "ALLUSERS", MB_OK);

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
