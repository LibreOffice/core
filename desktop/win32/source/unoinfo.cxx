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

#include <cstddef>
#include <stdio.h>
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

#include <tools/pathutils.hxx>

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

namespace {

wchar_t * getBrandPath(wchar_t * path) {
    DWORD n = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        exit(EXIT_FAILURE);
    }
    return tools::filename(path);
}

void writeNull() {
    if (fwrite("\0\0", 1, 2, stdout) != 2) {
        exit(EXIT_FAILURE);
    }
}

void writePath(
    wchar_t const * frontBegin, wchar_t const * frontEnd,
    wchar_t const * backBegin, std::size_t backLength)
{
    wchar_t path[MAX_PATH];
    wchar_t * end = tools::buildPath(
        path, frontBegin, frontEnd, backBegin, backLength);
    if (end == nullptr) {
        exit(EXIT_FAILURE);
    }
    std::size_t n = (end - path) * sizeof (wchar_t);
    if (fwrite(path, 1, n, stdout) != n) {
        exit(EXIT_FAILURE);
    }
}

}

int wmain(int argc, wchar_t ** argv, wchar_t **) {
    if (argc == 2 && wcscmp(argv[1], L"c++") == 0) {
        wchar_t path[MAX_PATH];
        wchar_t * pathEnd = getBrandPath(path);
        writePath(path, pathEnd, MY_STRING(L""));
    } else if (argc == 2 && wcscmp(argv[1], L"java") == 0) {
        if (fwrite("1", 1, 1, stdout) != 1) {
            exit(EXIT_FAILURE);
        }
        wchar_t path[MAX_PATH];
        wchar_t * pathEnd = getBrandPath(path);
        writePath(path, pathEnd, MY_STRING(L"classes\\ridl.jar"));
        writeNull();
        writePath(path, pathEnd, MY_STRING(L"classes\\jurt.jar"));
        writeNull();
        writePath(path, pathEnd, MY_STRING(L"classes\\juh.jar"));
        writeNull();
        writePath(path, pathEnd, MY_STRING(L"classes\\unoil.jar"));
        writeNull();
        writePath(path, pathEnd, MY_STRING(L""));
    } else {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
