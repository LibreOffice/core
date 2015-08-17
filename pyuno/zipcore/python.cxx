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
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "tools/pathutils.hxx"

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

wchar_t * encode(wchar_t * buffer, wchar_t const * text) {
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

#ifdef __MINGW32__
int main(int argc, char ** argv, char **) {
#else
int wmain(int argc, wchar_t ** argv, wchar_t **) {
#endif
    wchar_t path[MAX_PATH];
    DWORD n = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        exit(EXIT_FAILURE);
    }
    wchar_t * pathEnd = tools::filename(path);
    *pathEnd = L'\0';
    n = GetEnvironmentVariableW(L"UNO_PATH", NULL, 0);
    if (n == 0) {
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND ||
            !SetEnvironmentVariableW(L"UNO_PATH", path))
        {
            exit(EXIT_FAILURE);
        }
    }
    wchar_t bootstrap[MY_LENGTH(L"vnd.sun.star.pathname:") + MAX_PATH] =
        L"vnd.sun.star.pathname:"; //TODO: overflow
    wchar_t * bootstrapEnd = tools::buildPath(
        bootstrap + MY_LENGTH(L"vnd.sun.star.pathname:"), path, pathEnd,
        MY_STRING(L"fundamental.ini"));
    if (bootstrapEnd == NULL) {
        exit(EXIT_FAILURE);
    }
    wchar_t pythonpath2[MAX_PATH];
    wchar_t * pythonpath2End = tools::buildPath(
        pythonpath2, path, pathEnd,
        MY_STRING(L"\\python-core-" PYTHON_VERSION_STRING L"\\lib"));
    if (pythonpath2End == NULL) {
        exit(EXIT_FAILURE);
    }
    wchar_t pythonpath3[MAX_PATH];
    wchar_t * pythonpath3End = tools::buildPath(
        pythonpath3, path, pathEnd,
        MY_STRING(L"\\python-core-" PYTHON_VERSION_STRING L"\\lib\\site-packages"));
    if (pythonpath3End == NULL) {
        exit(EXIT_FAILURE);
    }
#ifdef __MINGW32__
    wchar_t pythonpath4[MAX_PATH];
    wchar_t * pythonpath4End = tools::buildPath(
        pythonpath4, path, pathEnd,
        MY_STRING(L"\\python-core-" PYTHON_VERSION_STRING L"\\lib\\lib-dynload"));
    if (pythonpath4End == NULL) {
        exit(EXIT_FAILURE);
    }
#endif
    wchar_t pythonhome[MAX_PATH];
    wchar_t * pythonhomeEnd = tools::buildPath(
        pythonhome, path, pathEnd, MY_STRING(L"\\python-core-" PYTHON_VERSION_STRING));
    if (pythonhomeEnd == NULL) {
        exit(EXIT_FAILURE);
    }
    wchar_t pythonexe[MAX_PATH];
    wchar_t * pythonexeEnd = tools::buildPath(
        pythonexe, path, pathEnd,
        MY_STRING(L"\\python-core-" PYTHON_VERSION_STRING L"\\bin\\python.exe"));
    if (pythonexeEnd == NULL) {
        exit(EXIT_FAILURE);
    }
    std::size_t clSize = MY_LENGTH(L"\"") + 4 * (pythonexeEnd - pythonexe) +
        MY_LENGTH(L"\"\0"); //TODO: overflow
        // 4 * len: each char preceded by backslash, each trailing backslash
        // doubled
    for (int i = 1; i < argc; ++i) {
#ifdef __MINGW32__
        clSize += MY_LENGTH(L" \"") + 4 * strlen(argv[i]) +
#else
        clSize += MY_LENGTH(L" \"") + 4 * wcslen(argv[i]) +
#endif
            MY_LENGTH(L"\""); //TODO: overflow
    }
    wchar_t * cl = new wchar_t[clSize];
    wchar_t * cp = encode(cl, pythonhome);
    for (int i = 1; i < argc; ++i) {
        *cp++ = L' ';
#ifdef __MINGW32__
        int nNeededWStrBuffSize = MultiByteToWideChar(CP_ACP, 0, argv[i], -1, NULL, 0);
        WCHAR *buff = new WCHAR[nNeededWStrBuffSize+1];
        MultiByteToWideChar(CP_ACP, 0, argv[i], -1, buff, nNeededWStrBuffSize);
        buff[nNeededWStrBuffSize] = 0;
        cp = encode(cp, buff);
        delete [] buff;
#else
        cp = encode(cp, argv[i]);
#endif
    }
    *cp = L'\0';
    n = GetEnvironmentVariableW(L"PATH", NULL, 0);
    wchar_t * orig;
    if (n == 0) {
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
            exit(EXIT_FAILURE);
        }
        orig = (wchar_t *)L"";
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
    n = GetEnvironmentVariableW(L"PYTHONPATH", NULL, 0);
    if (n == 0) {
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
            exit(EXIT_FAILURE);
        }
        orig = (wchar_t *)L"";
    } else {
        orig = new wchar_t[n];
        if (GetEnvironmentVariableW(L"PYTHONPATH", orig, n) != n - 1)
        {
            exit(EXIT_FAILURE);
        }
    }
#ifdef __MINGW32__
    len = (pathEnd - path) + MY_LENGTH(L";") + (pythonpath2End - pythonpath2) +
        MY_LENGTH(L";") + (pythonpath4End - pythonpath4) +
        MY_LENGTH(L";") + (pythonpath3End - pythonpath3) +
        (n == 0 ? 0 : MY_LENGTH(L";") + (n - 1)) + 1; //TODO: overflow
    value = new wchar_t[len];
    _snwprintf(
        value, len, L"%s;%s;%s;%s%s%s", path, pythonpath2, pythonpath4,
        pythonpath3, n == 0 ? L"" : L";", orig);
#else
    len = (pathEnd - path) + MY_LENGTH(L";") + (pythonpath2End - pythonpath2) +
        MY_LENGTH(L";") + (pythonpath3End - pythonpath3) +
        (n == 0 ? 0 : MY_LENGTH(L";") + (n - 1)) + 1; //TODO: overflow
    value = new wchar_t[len];
    _snwprintf(
        value, len, L"%s;%s;%s%s%s", path, pythonpath2, pythonpath3,
        n == 0 ? L"" : L";", orig);
#endif
    if (!SetEnvironmentVariableW(L"PYTHONPATH", value)) {
        exit(EXIT_FAILURE);
    }
    if (n != 0) {
        delete [] orig;
    }
    delete [] value;
    if (!SetEnvironmentVariableW(L"PYTHONHOME", pythonhome)) {
        exit(EXIT_FAILURE);
    }
    n = GetEnvironmentVariableW(L"URE_BOOTSTRAP", NULL, 0);
    if (n == 0) {
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND ||
            !SetEnvironmentVariableW(L"URE_BOOTSTRAP", bootstrap))
        {
            exit(EXIT_FAILURE);
        }
    }
    STARTUPINFOW startinfo;
    ZeroMemory(&startinfo, sizeof (STARTUPINFOW));
    startinfo.cb = sizeof (STARTUPINFOW);
    PROCESS_INFORMATION procinfo;
    if (!CreateProcessW(
            pythonexe, cl, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, NULL,
            NULL, &startinfo, &procinfo)) {
        exit(EXIT_FAILURE);
    }
    WaitForSingleObject(procinfo.hProcess,INFINITE);
    DWORD exitStatus;
    GetExitCodeProcess(procinfo.hProcess,&exitStatus);
    exit(exitStatus);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
