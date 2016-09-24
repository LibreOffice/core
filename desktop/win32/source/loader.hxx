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

#ifndef INCLUDED_DESKTOP_WIN32_SOURCE_LOADER_HXX
#define INCLUDED_DESKTOP_WIN32_SOURCE_LOADER_HXX

#include <sal/config.h>

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

// @param binPath
// Must point to an array of size at least MAX_PATH.  Is filled with the null
// terminated full path to the "bin" file corresponding to the current
// executable.
// @param iniDirectory
// Must point to an array of size at least MAX_PATH.  Is filled with the null
// terminated full directory path (ending in "\") to the "ini" file
// corresponding to the current executable.
void getPaths(WCHAR * binPath, WCHAR * iniDirectory);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
