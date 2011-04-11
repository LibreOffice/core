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

#ifndef INCLUDED_DESKTOP_WIN32_SOURCE_EXTENDLOADERENVIRONMENT_HXX
#define INCLUDED_DESKTOP_WIN32_SOURCE_EXTENDLOADERENVIRONMENT_HXX

#include "sal/config.h"

#include <cstddef>

#include <tchar.h>

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

namespace desktop_win32 {

inline WCHAR * commandLineAppend(
    WCHAR * buffer, WCHAR const * text, std::size_t length)
{
    wcsncpy(buffer, text, length + 1); // trailing null
    return buffer + length;
}

inline WCHAR * commandLineAppend(WCHAR * buffer, WCHAR const * text) {
    return commandLineAppend(buffer, text, wcslen(text));
}

inline WCHAR * commandLineAppendEncoded(WCHAR * buffer, WCHAR const * text) {
    std::size_t n = 0;
    for (;;) {
        WCHAR c = *text++;
        if (c == L'\0') {
            break;
        } else if (c == L'$') {
            buffer = commandLineAppend(buffer, MY_STRING(L"\\$"));
            n = 0;
        } else if (c == L'\\') {
            buffer = commandLineAppend(buffer, MY_STRING(L"\\\\"));
            n += 2;
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
    *buffer = L'\0';
    return buffer;
}

// Set the PATH environment variable in the current (loader) process, so that a
// following CreateProcess has the necessary environment:
// @param binPath
// Must point to an array of size at least MAX_PATH.  Is filled with the null
// terminated full path to the "bin" file corresponding to the current
// executable.
// @param iniDirectory
// Must point to an array of size at least MAX_PATH.  Is filled with the null
// terminated full directory path (ending in "\") to the "ini" file
// corresponding to the current executable.
void extendLoaderEnvironment(WCHAR * binPath, WCHAR * iniDirectory);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
