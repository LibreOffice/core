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
#ifndef INCLUDED_TOOLS_PATHUTILS_HXX
#define INCLUDED_TOOLS_PATHUTILS_HXX

#include "sal/config.h"

#if defined WNT
#include <cstddef>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// The compiled code is not part of the tl dynamic library, but is delivered as
// pathutils-obj and pathutils-slo objects (it is linked into special
// executables and dynamic libraries that do not link against OOo libraries):
namespace tools {

/** Determine the filename part of a path.
    @param path
        A non-NULL pointer to a null-terminated path.
    @return
        A pointer to the trailing filename part of the given path.
*/
WCHAR * filename(WCHAR * path);

/** Concatenate two paths.

    Either the first path is empty and the second path is an absolute path. Or
    the first path is an absolute path that ends in a backslash and the second
    path is a relative path.  In the latter case, to avoid paths that grow too
    long, leading .. segments of the second path are removed together with
    trailing segments from the first path. This should not cause problems as long
    as there are no symbolic links on Windows (as with symbolic links, x\y\.. and
    x might denote different directories).

    @param path
        An output paremeter taking the resulting path; must point at a valid
        range of memory of size at least MAX_PATH.  If NULL is returned, the
        content is unspecified.
    @param frontBegin, frontEnd
        Forms a valid range [frontBegin .. frontEnd) of less than MAX_PATH size.
    @param backBegin, backLength
        Forms a valid range [backBeghin .. backBegin + backLength) of less than
        MAX_PATH size.
    @return
        A pointer to the terminating null character of the concatenation, or NULL
        if a failure occurred.
*/
WCHAR * buildPath(
    WCHAR * path, WCHAR const * frontBegin, WCHAR const * frontEnd,
    WCHAR const * backBegin, std::size_t backLength);

/** Resolve a link file.

    @param path
        An input/output parameter taking the path; must point at a valid range of
        memory of size at least MAX_PATH.  On input, contains the null-terminated
        full path of the link file.  On output, contains the null-terminated full
        path of the resolved link; if NULL is returned, the content is unspecified.
    @return
        A pointer to the terminating null character of path, or NULL if a failure
        occurred.
*/
WCHAR * resolveLink(WCHAR * path);

}

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
