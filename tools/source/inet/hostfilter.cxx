/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/hostfilter.hxx>
#include <osl/file.hxx>
#include <regex>
#include <vector>

static std::regex g_AllowedHostsRegex("");
static OUString g_ExceptVerifyHost;
static bool g_AllowedHostsSet = false;
static bool g_AllowedHostsExemptVerifyHost = false;

void HostFilter::setAllowedHostsRegex(const char* sAllowedRegex)
{
    g_AllowedHostsSet = sAllowedRegex && sAllowedRegex[0] != '\0';
    if (g_AllowedHostsSet)
        g_AllowedHostsRegex = sAllowedRegex;
}

void HostFilter::setAllowedHostsExemptVerifyHost(bool allowedHostsExemptVerifyHost)
{
    g_AllowedHostsExemptVerifyHost = allowedHostsExemptVerifyHost;
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
    if (rHost == g_ExceptVerifyHost)
        return true;

    if (g_AllowedHostsSet && g_AllowedHostsExemptVerifyHost)
        return std::regex_match(OUString(rHost).toUtf8().getStr(), g_AllowedHostsRegex);

    return false;
}

static bool g_AllowedExtRefPathsConfigured = false;
static std::vector<OUString> g_AllowedExtRefPaths;

void HostFilter::resetAllowedExtRefPaths()
{
    g_AllowedExtRefPaths.clear();
    g_AllowedExtRefPathsConfigured = false;
}

void HostFilter::setAllowedExtRefPaths(const char* sPaths)
{
    g_AllowedExtRefPathsConfigured = true;
    g_AllowedExtRefPaths.clear();

    if (!sPaths || sPaths[0] == '\0')
        return;

    OString sPathList(sPaths);
    sal_Int32 nIndex = 0;
    do
    {
        OString sPath = sPathList.getToken(0, ':', nIndex);
        if (sPath.isEmpty())
            continue;

        OUString aSysPath = OStringToOUString(sPath, RTL_TEXTENCODING_UTF8);
        OUString aFileUrl;
        if (osl::FileBase::getFileURLFromSystemPath(aSysPath, aFileUrl) != osl::FileBase::E_None)
            continue;

        // Normalize relative paths and .. segments (does not resolve symlinks)
        OUString aNormalized;
        if (osl::FileBase::getAbsoluteFileURL(OUString(), aFileUrl, aNormalized)
            == osl::FileBase::E_None)
        {
            if (!aNormalized.endsWith("/"))
                aNormalized += "/";
            g_AllowedExtRefPaths.push_back(aNormalized);
        }
        else
        {
            if (!aFileUrl.endsWith("/"))
                aFileUrl += "/";
            g_AllowedExtRefPaths.push_back(aFileUrl);
        }
    } while (nIndex >= 0);
}

bool HostFilter::isFileUrlForbidden(const OUString& rFileUrl)
{
    if (!g_AllowedExtRefPathsConfigured)
        return false;

    if (!rFileUrl.startsWithIgnoreAsciiCase("file:"))
        return false;

    // Normalize relative paths and .. segments (does not resolve symlinks)
    OUString aNormalized;
    if (osl::FileBase::getAbsoluteFileURL(OUString(), rFileUrl, aNormalized)
        != osl::FileBase::E_None)
        return true;

    // Case-sensitive comparison: assumes a case-sensitive filesystem (i.e. Linux).
    for (const auto& rAllowed : g_AllowedExtRefPaths)
    {
        if (aNormalized.startsWith(rAllowed))
            return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
