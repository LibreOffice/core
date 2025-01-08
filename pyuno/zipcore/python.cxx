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

#include <config_python.h>

#include <cstddef>
#include <stdlib.h>
#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <systools/win32/extended_max_path.hxx>
#include <tools/pathutils.hxx>

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

static wchar_t * encode(wchar_t * buffer, wchar_t const * text) {
    *buffer++ = L'"';
    std::size_t n = 0;
    for (;;) {
        wchar_t c = *text++;
        if (c == L'\0') {
            break;
        } else if (c == L'"') {
            // Double any preceding backslashes as required by Windows:
            for (std::size_t i = 0; i < n; ++i) {
                *buffer++ = L'\\';
            }
            *buffer++ = L'\\';
            *buffer++ = L'"';
            n = 0;
        } else if (c == L'\\') {
            *buffer++ = L'\\';
            ++n;
        } else {
            *buffer++ = c;
            n = 0;
        }
    }
    // The command line will continue with a double quote, so double any
    // preceding backslashes as required by Windows:
    for (std::size_t i = 0; i < n; ++i) {
        *buffer++ = L'\\';
    }
    *buffer++ = L'"';
    return buffer;
}

int wmain(int argc, wchar_t ** argv, wchar_t **) {
    wchar_t path[EXTENDED_MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, path, std::size(path));
    if (n == 0 || n >= std::size(path)) {
        exit(EXIT_FAILURE);
    }
    wchar_t * pathEnd = tools::filename(path);
    *pathEnd = L'\0';
    n = GetEnvironmentVariableW(L"UNO_PATH", nullptr, 0);
    if (n == 0) {
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND ||
            !SetEnvironmentVariableW(L"UNO_PATH", path))
        {
            exit(EXIT_FAILURE);
        }
    }
    std::wstring bootstrap = tools::buildPath({ path, pathEnd }, L"fundamental.ini");
    if (bootstrap.empty()) {
        exit(EXIT_FAILURE);
    }
    bootstrap = L"vnd.sun.star.pathname:" + bootstrap;
    std::wstring pythonpath2 = tools::buildPath(
        { path, pathEnd },
        L"\\python-core-" PYTHON_VERSION_STRING L"\\lib");
    if (pythonpath2.empty()) {
        exit(EXIT_FAILURE);
    }
    std::wstring pythonpath3 = tools::buildPath(
        { path, pathEnd },
        L"\\python-core-" PYTHON_VERSION_STRING L"\\lib\\site-packages");
    if (pythonpath3.empty()) {
        exit(EXIT_FAILURE);
    }
    std::wstring pythonhome = tools::buildPath(
        { path, pathEnd },
        L"\\python-core-" PYTHON_VERSION_STRING);
    if (pythonhome.empty()) {
        exit(EXIT_FAILURE);
    }
    std::wstring pythonexe = tools::buildPath(
        { path, pathEnd },
        L"\\python-core-" PYTHON_VERSION_STRING L"\\bin\\python.exe");
    if (pythonexe.empty()) {
        exit(EXIT_FAILURE);
    }
    std::size_t clSize = MY_LENGTH(L"\"") + 4 * pythonexe.size() +
        MY_LENGTH(L"\"\0"); //TODO: overflow
        // 4 * len: each char preceded by backslash, each trailing backslash
        // doubled
    for (int i = 1; i < argc; ++i) {
        clSize += MY_LENGTH(L" \"") + 4 * wcslen(argv[i]) + MY_LENGTH(L"\"");
            //TODO: overflow
    }
    wchar_t * cl = new wchar_t[clSize];
    wchar_t * cp = encode(cl, pythonhome.data());
    for (int i = 1; i < argc; ++i) {
        *cp++ = L' ';
        cp = encode(cp, argv[i]);
    }
    *cp = L'\0';
    n = GetEnvironmentVariableW(L"PATH", nullptr, 0);
    wchar_t * orig;
    if (n == 0) {
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
            exit(EXIT_FAILURE);
        }
        orig = const_cast<wchar_t *>(L"");
    } else {
        orig = new wchar_t[n];
        if (GetEnvironmentVariableW(L"PATH", orig, n) != n - 1)
        {
            exit(EXIT_FAILURE);
        }
    }
    std::size_t len = (pathEnd - path) + (n == 0 ? 0 : MY_LENGTH(L";") +
        (n - 1)) + 1;
        //TODO: overflow
    wchar_t * value = new wchar_t[len];
    _snwprintf(
        value, len, L"%s%s%s", path, n == 0 ? L"" : L";", orig);
    if (!SetEnvironmentVariableW(L"PATH", value)) {
        exit(EXIT_FAILURE);
    }
    if (n != 0) {
        delete [] orig;
    }
    delete [] value;
    n = GetEnvironmentVariableW(L"PYTHONPATH", nullptr, 0);
    if (n == 0) {
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
            exit(EXIT_FAILURE);
        }
        orig = const_cast<wchar_t *>(L"");
    } else {
        orig = new wchar_t[n];
        if (GetEnvironmentVariableW(L"PYTHONPATH", orig, n) != n - 1)
        {
            exit(EXIT_FAILURE);
        }
    }
    len = (pathEnd - path) + MY_LENGTH(L";") + pythonpath2.size() +
        MY_LENGTH(L";") + pythonpath3.size() +
        (n == 0 ? 0 : MY_LENGTH(L";") + (n - 1)) + 1; //TODO: overflow
    value = new wchar_t[len];
    _snwprintf(
        value, len, L"%s;%s;%s%s%s", path, pythonpath2.c_str(), pythonpath3.c_str(),
        n == 0 ? L"" : L";", orig);
    if (!SetEnvironmentVariableW(L"PYTHONPATH", value)) {
        exit(EXIT_FAILURE);
    }
    if (n != 0) {
        delete [] orig;
    }
    delete [] value;
    if (!SetEnvironmentVariableW(L"PYTHONHOME", pythonhome.data())) {
        exit(EXIT_FAILURE);
    }
    n = GetEnvironmentVariableW(L"URE_BOOTSTRAP", nullptr, 0);
    if (n == 0) {
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND ||
            !SetEnvironmentVariableW(L"URE_BOOTSTRAP", bootstrap.data()))
        {
            exit(EXIT_FAILURE);
        }
    }
    STARTUPINFOW startinfo{ .cb = sizeof(startinfo) };
    PROCESS_INFORMATION procinfo;
    if (!CreateProcessW(
            pythonexe.data(), cl, nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT, nullptr,
            nullptr, &startinfo, &procinfo)) {
        exit(EXIT_FAILURE);
    }
    WaitForSingleObject(procinfo.hProcess,INFINITE);
    DWORD exitStatus;
    GetExitCodeProcess(procinfo.hProcess,&exitStatus);
    exit(exitStatus);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
