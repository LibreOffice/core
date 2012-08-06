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

#include "sal/config.h"

#if defined WNT

#include <cstddef>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "sal/types.h"

namespace cli_ure {

SAL_DLLPUBLIC_EXPORT WCHAR * filename(WCHAR * path) {
    WCHAR * f = path;
    for (WCHAR * p = path;;) {
        switch (*p++) {
        case L'\0':
            return f;
        case L'\\':
            f = p;
            break;
        }
    }
}

WCHAR * buildPath(
    WCHAR * path, WCHAR const * frontBegin, WCHAR const * frontEnd,
    WCHAR const * backBegin, std::size_t backLength)
{
    // Remove leading ".." segments in the second path together with matching
    // segments in the first path that are neither empty nor "." nor ".." nor
    // end in ":" (which is not foolprove, as it can erroneously erase the start
    // of a UNC path, but only if the input is bad data):
    while (backLength >= 2 && backBegin[0] == L'.' && backBegin[1] == L'.' &&
           (backLength == 2 || backBegin[2] == L'\\'))
    {
        if (frontEnd - frontBegin < 2 || frontEnd[-1] != L'\\' ||
            frontEnd[-2] == L'\\' || frontEnd[-2] == L':' ||
            (frontEnd[-2] == L'.' &&
             (frontEnd - frontBegin < 3 || frontEnd[-3] == L'\\' ||
              (frontEnd[-3] == L'.' &&
               (frontEnd - frontBegin < 4 || frontEnd[-4] == L'\\')))))
        {
            break;
        }
        WCHAR const * p = frontEnd - 1;
        while (p != frontBegin && p[-1] != L'\\') {
            --p;
        }
        if (p == frontBegin) {
            break;
        }
        frontEnd = p;
        if (backLength == 2) {
            backBegin += 2;
            backLength -= 2;
        } else {
            backBegin += 3;
            backLength -= 3;
        }
    }
    if (backLength <
        static_cast< std::size_t >(MAX_PATH - (frontEnd - frontBegin)))
        // hopefully std::size_t is large enough
    {
        WCHAR * p;
        if (frontBegin == path) {
            p = const_cast< WCHAR * >(frontEnd);
        } else {
            p = path;
            while (frontBegin != frontEnd) {
                *p++ = *frontBegin++;
            }
        }
        for (; backLength > 0; --backLength) {
            *p++ = *backBegin++;
        }
        *p = L'\0';
        return p;
    } else {
        SetLastError(ERROR_FILENAME_EXCED_RANGE);
        return NULL;
    }
}

WCHAR * resolveLink(WCHAR * path) {
    HANDLE h = CreateFileW(
        path, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    char p1[MAX_PATH];
    DWORD n;
    BOOL ok = ReadFile(h, p1, MAX_PATH, &n, NULL);
    CloseHandle(h);
    if (!ok) {
        return NULL;
    }
    WCHAR p2[MAX_PATH];
    std::size_t n2 = 0;
    bool colon = false;
    for (DWORD i = 0; i < n;) {
        unsigned char c = static_cast< unsigned char >(p1[i++]);
        switch (c) {
        case '\0':
            SetLastError(ERROR_BAD_PATHNAME);
            return NULL;
        case '\x0A':
        case '\x0D':
            if (n2 == MAX_PATH) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return NULL;
            }
            p2[n2] = L'\0';
            break;
        case ':':
            colon = true;
            // fall through
        default:
            // Convert from UTF-8 to UTF-16:
            if (c <= 0x7F) {
                p2[n2++] = c;
            } else if (c >= 0xC2 && c <= 0xDF && i < n &&
                       static_cast< unsigned char >(p1[i]) >= 0x80 &&
                       static_cast< unsigned char >(p1[i]) <= 0xBF)
            {
                p2[n2++] = ((c & 0x1F) << 6) |
                    (static_cast< unsigned char >(p1[i++]) & 0x3F);
            } else if (n - i > 1 &&
                       ((c == 0xE0 &&
                         static_cast< unsigned char >(p1[i]) >= 0xA0 &&
                         static_cast< unsigned char >(p1[i]) <= 0xBF) ||
                        ((c >= 0xE1 && c <= 0xEC || c >= 0xEE && c <= 0xEF) &&
                         static_cast< unsigned char >(p1[i]) >= 0x80 &&
                         static_cast< unsigned char >(p1[i]) <= 0xBF) ||
                        (c == 0xED &&
                         static_cast< unsigned char >(p1[i]) >= 0x80 &&
                         static_cast< unsigned char >(p1[i]) <= 0x9F)) &&
                       static_cast< unsigned char >(p1[i + 1]) >= 0x80 &&
                       static_cast< unsigned char >(p1[i + 1]) <= 0xBF)
            {
                p2[n2++] = ((c & 0x0F) << 12) |
                    ((static_cast< unsigned char >(p1[i]) & 0x3F) << 6) |
                    (static_cast< unsigned char >(p1[i + 1]) & 0x3F);
                i += 2;
            } else if (n - 2 > 1 &&
                       ((c == 0xF0 &&
                         static_cast< unsigned char >(p1[i]) >= 0x90 &&
                         static_cast< unsigned char >(p1[i]) <= 0xBF) ||
                        (c >= 0xF1 && c <= 0xF3 &&
                         static_cast< unsigned char >(p1[i]) >= 0x80 &&
                         static_cast< unsigned char >(p1[i]) <= 0xBF) ||
                        (c == 0xF4 &&
                         static_cast< unsigned char >(p1[i]) >= 0x80 &&
                         static_cast< unsigned char >(p1[i]) <= 0x8F)) &&
                       static_cast< unsigned char >(p1[i + 1]) >= 0x80 &&
                       static_cast< unsigned char >(p1[i + 1]) <= 0xBF &&
                       static_cast< unsigned char >(p1[i + 2]) >= 0x80 &&
                       static_cast< unsigned char >(p1[i + 2]) <= 0xBF)
            {
                sal_Int32 u = ((c & 0x07) << 18) |
                    ((static_cast< unsigned char >(p1[i]) & 0x3F) << 12) |
                    ((static_cast< unsigned char >(p1[i + 1]) & 0x3F) << 6) |
                    (static_cast< unsigned char >(p1[i + 2]) & 0x3F);
                i += 3;
                p2[n2++] = static_cast< WCHAR >(((u - 0x10000) >> 10) | 0xD800);
                p2[n2++] = static_cast< WCHAR >(
                    ((u - 0x10000) & 0x3FF) | 0xDC00);
            } else {
                SetLastError(ERROR_BAD_PATHNAME);
                return NULL;
            }
            break;
        }
    }
    WCHAR * end;
    if (colon || p2[0] == L'\\') {
        // Interpret p2 as an absolute path:
        end = path;
    } else {
        // Interpret p2 as a relative path:
        end = filename(path);
    }
    return buildPath(path, path, end, p2, n2);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
