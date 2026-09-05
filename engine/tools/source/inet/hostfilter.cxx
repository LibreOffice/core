/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/hostfilter.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <charconv>
#include <cstring>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <prewin.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <postwin.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

namespace
{
/// A parsed IP network: the leading nPrefixLength bits of aBytes identify it.
struct IpNetwork
{
    /// IPv4 in the first 4 bytes, IPv6 in all 16.
    unsigned char aBytes[16];
    /// 4 for IPv4, 16 for IPv6.
    int nLength;
    int nPrefixLength;
};

/// Parses an IP address of either family into pOut and returns its length in
/// bytes, or 0 if the text is not an address. IPv4-mapped IPv6 addresses are
/// folded to IPv4 so that both spellings of one host compare equal.
int parseAddress(std::string_view rText, unsigned char* pOut)
{
    if (rText.size() >= 2 && rText.front() == '[' && rText.back() == ']')
        rText = rText.substr(1, rText.size() - 2);
    if (rText.empty() || rText.size() >= INET6_ADDRSTRLEN)
        return 0;

    char aText[INET6_ADDRSTRLEN];
    std::memcpy(aText, rText.data(), rText.size());
    aText[rText.size()] = '\0';

    if (rText.find(':') == std::string_view::npos)
        return inet_pton(AF_INET, aText, pOut) == 1 ? 4 : 0;

    if (inet_pton(AF_INET6, aText, pOut) != 1)
        return 0;

    static const unsigned char aMappedPrefix[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };
    if (std::memcmp(pOut, aMappedPrefix, sizeof(aMappedPrefix)) == 0)
    {
        std::memmove(pOut, pOut + 12, 4);
        return 4;
    }
    return 16;
}

/// Parses "address/prefix" CIDR notation.
bool parseNetwork(std::string_view rText, IpNetwork& rNetwork)
{
    const std::size_t nSlash = rText.find('/');
    if (nSlash == std::string_view::npos || nSlash == 0 || nSlash + 1 >= rText.size())
        return false;

    const std::string_view aPrefix = rText.substr(nSlash + 1);
    const char* const pEnd = aPrefix.data() + aPrefix.size();
    int nPrefixLength = 0;
    const std::from_chars_result aParsed = std::from_chars(aPrefix.data(), pEnd, nPrefixLength);
    if (aParsed.ec != std::errc() || aParsed.ptr != pEnd || nPrefixLength < 0)
        return false;

    rNetwork.nLength = parseAddress(rText.substr(0, nSlash), rNetwork.aBytes);
    if (rNetwork.nLength == 0 || nPrefixLength > rNetwork.nLength * 8)
        return false;
    rNetwork.nPrefixLength = nPrefixLength;
    return true;
}

bool prefixEqual(const unsigned char* pA, const unsigned char* pB, int nPrefixLength)
{
    const int nFullBytes = nPrefixLength / 8;
    if (std::memcmp(pA, pB, nFullBytes) != 0)
        return false;

    const int nRemainingBits = nPrefixLength % 8;
    if (nRemainingBits == 0)
        return true;

    const unsigned char nMask = static_cast<unsigned char>(0xff << (8 - nRemainingBits));
    return (pA[nFullBytes] & nMask) == (pB[nFullBytes] & nMask);
}
}

static std::regex g_AllowedHostsRegex("");
static std::vector<IpNetwork> g_AllowedNetworks;
static OUString g_ExceptVerifyHost;
static bool g_AllowedHostsSet = false;
static bool g_AllowedHostsExemptVerifyHost = false;

void HostFilter::setAllowedHosts(const char* sAllowlist)
{
    g_AllowedNetworks.clear();
    g_AllowedHostsSet = false;
    if (!sAllowlist || sAllowlist[0] == '\0')
        return;

    std::string aRegex;
    const std::string_view aList(sAllowlist);
    std::size_t nStart = 0;
    while (nStart <= aList.size())
    {
        std::size_t nEnd = aList.find('\n', nStart);
        if (nEnd == std::string_view::npos)
            nEnd = aList.size();
        const std::string_view aEntry = aList.substr(nStart, nEnd - nStart);
        nStart = nEnd + 1;
        if (aEntry.empty())
            continue;

        IpNetwork aNetwork;
        if (parseNetwork(aEntry, aNetwork))
        {
            g_AllowedNetworks.push_back(aNetwork);
            continue;
        }

        if (!aRegex.empty())
            aRegex += '|';
        aRegex += aEntry;
    }

    if (!aRegex.empty())
    {
        try
        {
            g_AllowedHostsRegex = std::regex(aRegex);
            g_AllowedHostsSet = true;
        }
        catch (const std::regex_error&)
        {
            SAL_WARN("tools", "ignoring invalid allowed hosts regex: " << aRegex);
        }
    }
}

void HostFilter::setAllowedHostsExemptVerifyHost(bool allowedHostsExemptVerifyHost)
{
    g_AllowedHostsExemptVerifyHost = allowedHostsExemptVerifyHost;
}

static bool isAllowedHost(const OString& rHost)
{
    if (g_AllowedHostsSet && std::regex_match(rHost.getStr(), g_AllowedHostsRegex))
        return true;

    if (g_AllowedNetworks.empty())
        return false;

    unsigned char aBytes[16];
    const int nLength = parseAddress(std::string_view(rHost.getStr(), rHost.getLength()), aBytes);
    if (nLength == 0)
        return false;

    for (const IpNetwork& rNetwork : g_AllowedNetworks)
    {
        if (rNetwork.nLength == nLength
            && prefixEqual(rNetwork.aBytes, aBytes, rNetwork.nPrefixLength))
            return true;
    }
    return false;
}

bool HostFilter::isForbidden(const OUString& rHost)
{
    if (!g_AllowedHostsSet && g_AllowedNetworks.empty())
        return false;

    return !isAllowedHost(rHost.toUtf8());
}

void HostFilter::setExemptVerifyHost(const OUString& rExemptVerifyHost)
{
    g_ExceptVerifyHost = rExemptVerifyHost;
}

bool HostFilter::isExemptVerifyHost(const std::u16string_view rHost)
{
    if (rHost == g_ExceptVerifyHost)
        return true;

    if ((g_AllowedHostsSet || !g_AllowedNetworks.empty()) && g_AllowedHostsExemptVerifyHost)
        return isAllowedHost(OUString(rHost).toUtf8());

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
