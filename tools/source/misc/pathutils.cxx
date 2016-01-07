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

#include <sal/config.h>

#if defined WNT

#include <cstddef>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sal/types.h>
#include <tools/pathutils.hxx>

namespace tools {

WCHAR * filename(WCHAR * path) {
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
    // end in ":" (which is not foolproof, as it can erroneously erase the start
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

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
