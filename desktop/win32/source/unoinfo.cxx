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

#include "tools/pathutils.hxx"

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

namespace {

wchar_t * getBrandPath(wchar_t * path) {
    DWORD n = GetModuleFileNameW(NULL, path, MAX_PATH);
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
    if (end == NULL) {
        exit(EXIT_FAILURE);
    }
    std::size_t n = (end - path) * sizeof (wchar_t);
    if (fwrite(path, 1, n, stdout) != n) {
        exit(EXIT_FAILURE);
    }
}

}

#ifdef __MINGW32__
int main(int argc, char ** argv, char **) {
    if (argc == 2 && strcmp(argv[1], "c++") == 0) {
#else
int wmain(int argc, wchar_t ** argv, wchar_t **) {
    if (argc == 2 && wcscmp(argv[1], L"c++") == 0) {
#endif
        wchar_t path[MAX_PATH];
        wchar_t * pathEnd = getBrandPath(path);
        if (tools::buildPath(path, path, pathEnd, MY_STRING(L"..\\basis-link"))
            == NULL)
        {
            exit(EXIT_FAILURE);
        }
        pathEnd = tools::resolveLink(path);
        if (pathEnd == NULL ||
            (tools::buildPath(path, path, pathEnd, MY_STRING(L"\\ure-link")) ==
             NULL))
        {
            exit(EXIT_FAILURE);
        }
        pathEnd = tools::resolveLink(path);
        if (pathEnd == NULL) {
            exit(EXIT_FAILURE);
        }
        writePath(path, pathEnd, MY_STRING(L"\\bin"));
#ifdef __MINGW32__
    } else if (argc == 2 && strcmp(argv[1], "java") == 0) {
#else
    } else if (argc == 2 && wcscmp(argv[1], L"java") == 0) {
#endif
        if (fwrite("1", 1, 1, stdout) != 1) {
            exit(EXIT_FAILURE);
        }
        wchar_t path[MAX_PATH];
        wchar_t * pathEnd = getBrandPath(path);
        writePath(path, pathEnd, MY_STRING(L""));
        if (tools::buildPath(path, path, pathEnd, MY_STRING(L"..\\basis-link"))
            == NULL)
        {
            exit(EXIT_FAILURE);
        }
        pathEnd = tools::resolveLink(path);
        if (pathEnd == NULL) {
            exit(EXIT_FAILURE);
        }
        writeNull();
        writePath(path, pathEnd, MY_STRING(L"\\program\\classes\\unoil.jar"));
        if (tools::buildPath(path, path, pathEnd, MY_STRING(L"\\ure-link")) ==
            NULL)
        {
            exit(EXIT_FAILURE);
        }
        pathEnd = tools::resolveLink(path);
        if (pathEnd == NULL) {
            exit(EXIT_FAILURE);
        }
        writeNull();
        writePath(path, pathEnd, MY_STRING(L"\\java\\ridl.jar"));
        writeNull();
        writePath(path, pathEnd, MY_STRING(L"\\java\\jurt.jar"));
        writeNull();
        writePath(path, pathEnd, MY_STRING(L"\\java\\juh.jar"));
    } else {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
