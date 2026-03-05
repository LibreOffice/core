/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TOOLS_HOSTFILTER_HXX
#define INCLUDED_TOOLS_HOSTFILTER_HXX

#include <rtl/ustring.hxx>
#include <tools/toolsdllapi.h>

// Helper for filtering allowed hosts for remote connections

class TOOLS_DLLPUBLIC HostFilter
{
public:
    static void setAllowedHostsRegex(const char* sAllowedRegex);

    static void setAllowedHostsExemptVerifyHost(bool allowedHostExceptVerifyHost);

    static bool isForbidden(const OUString& rHost);

    static void setExemptVerifyHost(const OUString& rExemptVerifyHost);

    static bool isExemptVerifyHost(const std::u16string_view rHost);

    /// A colon-separated list of directory paths that file:// external
    /// references are allowed to reach. An empty string means:
    /// "block all file URLs"
    static void setAllowedExtRefPaths(const char* sPaths);

    /// Reset AllowedExtRefPaths as if it was never called, back to
    /// default allow all state.
    static void resetAllowedExtRefPaths();

    /// Return true when rFileUrl is a file:// URL that is outside any
    /// directory registered with setAllowedExtRefPaths. Non-file URLs
    /// are always allowed.
    static bool isFileUrlForbidden(const OUString& rFileUrl);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
