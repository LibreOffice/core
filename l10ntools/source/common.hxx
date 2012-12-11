/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_L10NTOOLS_SOURCE_COMMON_HXX
#define INCLUDED_L10NTOOLS_SOURCE_COMMON_HXX

#include "sal/config.h"

#include <cstdlib>
#include <iostream>

#include "osl/file.hxx"
#include "osl/process.h"
#include "osl/thread.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/uri.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

namespace common {

inline rtl::OUString pathnameToAbsoluteUrl(rtl::OUString const & pathname) {
    rtl::OUString url;
    if (osl::FileBase::getFileURLFromSystemPath(pathname, url)
        != osl::FileBase::E_None)
    {
        std::cerr << "Error: Cannot convert input pathname to URL\n";
        std::exit(EXIT_FAILURE);
    }
    rtl::OUString cwd;
    if (osl_getProcessWorkingDir(&cwd.pData) != osl_Process_E_None) {
        std::cerr << "Error: Cannot determine cwd\n";
        std::exit(EXIT_FAILURE);
    }
    if (osl::FileBase::getAbsoluteFileURL(cwd, url, url)
        != osl::FileBase::E_None)
    {
        std::cerr << "Error: Cannot convert input URL to absolute URL\n";
        std::exit(EXIT_FAILURE);
    }
    return url;
}

inline rtl::OString pathnameToken(char const * pathname, char const * root) {
    rtl::OUString full;
    if (!rtl_convertStringToUString(
            &full.pData, pathname, rtl_str_getLength(pathname),
            osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        std::cerr << "Error: Cannot convert input pathname to UTF-16\n";
        std::exit(EXIT_FAILURE);
    }
    full = pathnameToAbsoluteUrl(full);
    if (root == 0) {
        std::cerr << "Error: No project root argument\n";
        std::exit(EXIT_FAILURE);
    }
    rtl::OUString base;
    if (!rtl_convertStringToUString(
            &base.pData, root, rtl_str_getLength(root),
            osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        std::cerr << "Error: Cannot convert project root to UTF-16\n";
        std::exit(EXIT_FAILURE);
    }
    base = rtl::Uri::convertRelToAbs(full, base);
    if (full.getLength() <= base.getLength() || base.isEmpty()
        || base[base.getLength() - 1] != '/'
        || full[base.getLength() - 1] != '/')
    {
        std::cerr << "Error: Cannot extract suffix from input pathname\n";
        std::exit(EXIT_FAILURE);
    }
    full = full.copy(base.getLength()).replace('/', '\\');
    rtl::OString suffix;
    if (!full.convertToString(
            &suffix, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        std::cerr << "Error: Cannot convert suffix from UTF-16\n";
        std::exit(EXIT_FAILURE);
    }
    return suffix;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
