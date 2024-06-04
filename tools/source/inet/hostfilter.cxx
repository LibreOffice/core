/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/hostfilter.hxx>
#include <regex>

static std::regex g_AllowedHostsRegex("");
static OUString g_ExceptVerifyHost;
static bool g_AllowedHostsSet = false;

void HostFilter::setAllowedHostsRegex(const char* sAllowedRegex)
{
    g_AllowedHostsSet = sAllowedRegex && sAllowedRegex[0] != '\0';
    if (g_AllowedHostsSet)
        g_AllowedHostsRegex = sAllowedRegex;
}

bool HostFilter::isForbidden(const OUString& rHost)
{
    if (!g_AllowedHostsSet)
        return false;

    return !std::regex_match(rHost.toUtf8().getStr(), g_AllowedHostsRegex);
}

void HostFilter::setExemptVerifyHost(const OUString& rExemptVerifyHost)
{
    g_ExceptVerifyHost = rExemptVerifyHost;
}

bool HostFilter::isExemptVerifyHost(const std::u16string_view rHost)
{
    return rHost == g_ExceptVerifyHost;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
