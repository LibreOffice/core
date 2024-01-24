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

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <o3tl/safeint.hxx>
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

std::wstring buildPath(std::wstring_view front, std::wstring_view back)
{
    // Remove leading ".." segments in the second path together with matching
    // segments in the first path that are neither empty nor "." nor ".." nor
    // end in ":" (which is not foolproof, as it can erroneously erase the start
    // of a UNC path, but only if the input is bad data):
    while (back.starts_with(L"..") &&
           (back.size() == 2 || back[2] == L'\\'))
    {
        if (front.size() < 2 || front.back() != L'\\' ||
            front[front.size() - 2] == L'\\' || front[front.size() - 2] == L':' ||
            (front[front.size() - 2] == L'.' &&
             (front.size() < 3 || front[front.size() - 3] == L'\\' ||
              (front[front.size() - 3] == L'.' &&
               (front.size() < 4 || front[front.size() - 4] == L'\\')))))
        {
            break;
        }
        auto p = front.end() - 1;
        while (p != front.begin() && p[-1] != L'\\') {
            --p;
        }
        if (p == front.begin()) {
            break;
        }
        front.remove_suffix(front.end() - p);
        if (back.size() == 2) {
            back = {};
        } else {
            back.remove_prefix(3);
        }
    }

    return std::wstring(front) + std::wstring(back);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
