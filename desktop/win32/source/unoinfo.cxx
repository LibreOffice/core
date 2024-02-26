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
#include <windows.h>

#include <systools/win32/extended_max_path.hxx>
#include <tools/pathutils.hxx>

namespace {

template<size_t N> std::wstring_view getBrandPath(wchar_t (&path)[N])
{
    DWORD n = GetModuleFileNameW(nullptr, path, N);
    if (n == 0 || n >= N) {
        exit(EXIT_FAILURE);
    }
    return { path, tools::filename(path) };
}

void writeNull() {
    if (fwrite("\0\0", 1, 2, stdout) != 2) {
        exit(EXIT_FAILURE);
    }
}

void writePath(std::wstring_view front, std::wstring_view back)
{
    std::wstring path = tools::buildPath(front, back);
    if (path.empty()) {
        exit(EXIT_FAILURE);
    }
    if (fwrite(path.data(), sizeof (wchar_t), path.size(), stdout) != path.size()) {
        exit(EXIT_FAILURE);
    }
}

}

int wmain(int argc, wchar_t ** argv, wchar_t **) {
    wchar_t path_buf[EXTENDED_MAX_PATH];
    if (argc == 2 && wcscmp(argv[1], L"c++") == 0) {
        auto path = getBrandPath(path_buf);
        writePath(path, L"");
    } else if (argc == 2 && wcscmp(argv[1], L"java") == 0) {
        if (fwrite("1", 1, 1, stdout) != 1) {
            exit(EXIT_FAILURE);
        }
        auto path = getBrandPath(path_buf);
        writePath(path, L"classes\\libreoffice.jar");
        writeNull();
        writePath(path, L"");
    } else {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
