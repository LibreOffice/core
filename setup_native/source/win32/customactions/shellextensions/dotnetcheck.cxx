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
#include <../tools/msiprop.hxx>
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
    string present(GetMsiPropValue(handle, TEXT("MsiNetAssemblySupport")));
    string required(GetMsiPropValue(handle, TEXT("REQUIRED_DOTNET_VERSION")));

    // string myText1 = TEXT("MsiNetAssemblySupport: ") + present;
    // string myText2 = TEXT("REQUIRED_DOTNET_VERSION: ") + required;
    // MessageBox(NULL, myText1.c_str(), "DEBUG", MB_OK);
    // MessageBox(NULL, myText2.c_str(), "DEBUG", MB_OK);

    MsiSetProperty(
        handle, TEXT("DOTNET_SUFFICIENT"),
        (present.empty() || compareVersions(present, required) == ORDER_LESS ?
         TEXT("0") : TEXT("1")));

    // string result(GetMsiPropValue(handle, TEXT("DOTNET_SUFFICIENT")));
    // string myText3 = TEXT("DOTNET_SUFFICIENT: ") + result;
    // MessageBox(NULL, myText3.c_str(), "DEBUG", MB_OK);


    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall ShowProperties(MSIHANDLE handle)
{
    string property = GetMsiPropValue(handle, TEXT("INSTALLLOCATION"));
    string myText = TEXT("INSTALLLOCATION: ") + property;
    MessageBox(NULL, myText.c_str(), "INSTALLLOCATION", MB_OK);

    property = GetMsiPropValue(handle, TEXT("Installed"));
    myText = TEXT("Installed: ") + property;
    MessageBox(NULL, myText.c_str(), "Installed", MB_OK);

    property = GetMsiPropValue(handle, TEXT("PATCH"));
    myText = TEXT("PATCH: ") + property;
    MessageBox(NULL, myText.c_str(), "PATCH", MB_OK);

    property = GetMsiPropValue(handle, TEXT("REMOVE"));
    myText = TEXT("REMOVE: ") + property;
    MessageBox(NULL, myText.c_str(), "REMOVE", MB_OK);

    property = GetMsiPropValue(handle, TEXT("ALLUSERS"));
    myText = TEXT("ALLUSERS: ") + property;
    MessageBox(NULL, myText.c_str(), "ALLUSERS", MB_OK);

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
