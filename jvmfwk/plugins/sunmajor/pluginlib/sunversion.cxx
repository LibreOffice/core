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


#include "sunversion.hxx"
#include <osl/thread.h>
#include <rtl/character.hxx>
#include <rtl/ustring.hxx>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <o3tl/string_view.hxx>

namespace jfw_plugin  { //stoc_javadetect


SunVersion::SunVersion(std::u16string_view usVer):
    m_nUpdateSpecial(0), m_preRelease(Rel_NONE)
{
    OString sVersion= OUStringToOString(usVer, osl_getThreadTextEncoding());
    m_bValid = init(sVersion.getStr());
}

SunVersion::SunVersion(const char * szVer):
    m_nUpdateSpecial(0), m_preRelease(Rel_NONE)
{
    m_bValid = init(szVer);
}

const size_t nMaxParts = 8;

// Converts the first characters of a string to an int >= 0 and returns it, or returns -1
// if any of the characters were not digits. The number of characters consumed are
// passed back via nConsumed.
static int getUInt(std::u16string_view& rString, size_t& rConsumed)
{
    size_t i = 0;
    const size_t nMaxDigits = 5;

    // Count digits
    for (i = 0; i < rString.size() && i < nMaxDigits && rtl::isAsciiDigit(rString[i]); i++);
    // Error if no digits, or too many digits
    if (i == 0 || i >= nMaxDigits)
        return -1;

    rConsumed = i;
    return OUString(rString.data(), i).toInt32();
}

// Splits sStr into a vector of strings with each element containing any characters
// between successive separators. Empty elements are illegal and return an empty vector.
static std::vector<std::u16string_view> split(std::u16string_view sStr, const sal_Unicode cSeparator)
{
    std::vector<std::u16string_view> vec;
    for (std::size_t idx = 0; idx != std::u16string_view::npos;)
    {
        std::u16string_view kw = o3tl::trim(o3tl::getToken(sStr, cSeparator, idx));
        if (kw.empty())
            return std::vector<std::u16string_view>();
        vec.push_back(kw);
    }

    return vec;
}

/**Format major.minor.maintenance_update
 */
bool SunVersion::init(const char *szVersion)
{
    if (!szVersion || !szVersion[0] || szVersion[0] == '.')
        return false;

    OUString sToSplit = OUString::fromUtf8(szVersion);
    std::vector<std::u16string_view> aParts = split(sToSplit, '.');

    // Reject single component ("10") versions, versions with too many components, and
    // malformed versions with trailing or adjacent '.' (which cause an empty vector).
    if (aParts.size() < 2 || aParts.size() > nMaxParts)
        return false;

    // Parse all components except the last one; they must be numeric.
    size_t nIdx;
    for (nIdx = 0; nIdx < aParts.size() - 1; nIdx++)
    {
        assert (aParts[nIdx].size());
        size_t nConsumed = 0;
        int nVer = getUInt(aParts[nIdx], nConsumed);
        if (nConsumed != aParts[nIdx].size())
            return false;
        m_arVersionParts.emplace_back(nVer);
    }

    // Last component contains one or more alphanumeric sub-components separated by [-_p]
    // First characters of last component before any -/_ must be numeric
    size_t nConsumed = 0;
    int nMaintenance = getUInt(aParts[nIdx], nConsumed);
    if (nMaintenance < 0)
        return false;
    m_arVersionParts.emplace_back(nMaintenance);

    // If no update/patchlevel version is OK
    std::u16string_view sPatchlevel = aParts[nIdx].substr(nConsumed);
    if (!sPatchlevel.size())
        return true;

    // Now handle any patch or update version after the numeric part. Only a
    // single '_' is allowed, and if present must be the next character
    const size_t nUnderscorePos = sPatchlevel.find_first_of('_');
    if (nUnderscorePos != sPatchlevel.find_last_of('_'))
        return false;
    if (nUnderscorePos != std::u16string_view::npos && nUnderscorePos > 0)
        return false;

    // Only a single - allowed and it must come after any '_'
    const size_t nDashPos = sPatchlevel.find_first_of('-');
    if (nDashPos != std::u16string_view::npos)
    {
        if (nDashPos != sPatchlevel.find_last_of('-'))
            return false;
        if (nUnderscorePos != std::u16string_view::npos && nDashPos < nUnderscorePos)
            return false;
    }

    // '-' or '_' must be the first character of the patchlevel
    if (sPatchlevel[0] != '_' && sPatchlevel[0] != '-')
        return false;

    // If an underscore is first, next characters must be update level digits.
    // The update level digits may be followed by a pre-release string (eg, "-rc2"
    // or "-beta1") or a single 'a' (like "1.4.1_01a" on Solaris 9).
    if (nUnderscorePos == 0)
    {
        assert(sPatchlevel.starts_with(u"_"));
        sPatchlevel.remove_prefix(1);

        size_t nUpdateConsumed = 0;
        int nUpdate = getUInt(sPatchlevel, nUpdateConsumed);
        if (nUpdate < 0)
            return false;
        m_arVersionParts.emplace_back(nUpdate);
        sPatchlevel.remove_prefix(nUpdateConsumed);

        if (sPatchlevel.size() && (sPatchlevel[0] == 'a'))
        {
            m_nUpdateSpecial = 'a';
            // 'a' must be the last character
            return (sPatchlevel.size() == 1) ? true : false;
        }
    }

    // At this point there must be a pre-release string (eg "-beta1")
    if (nDashPos != std::u16string_view::npos)
    {
        assert(sPatchlevel.starts_with(u"-"));

        m_preRelease = getPreRelease(sPatchlevel);
        if (m_preRelease == Rel_NONE)
            return false;
        else if (m_preRelease == Rel_FreeBSD)
        {
            // Skip leading '-p'
            assert(sPatchlevel.starts_with(u"-p"));
            sPatchlevel.remove_prefix(2);

            size_t nFBSDConsumed = 0;
            int nFBSD = getUInt(sPatchlevel, nFBSDConsumed);
            if (nFBSD < 0)
                return false;

            // FreeBSD patchlevel must be last characters
            if (sPatchlevel.size() > nFBSDConsumed)
                return false;

            m_nUpdateSpecial = nFBSD+100; //hack for FBSD #i56953#
        }
    }

    return true;
}

SunVersion::PreRelease SunVersion::getPreRelease(const std::u16string_view sRelease)
{
    static const std::unordered_map<std::u16string_view, const enum PreRelease> aReleases{
        { u"-internal", Rel_INTERNAL },
        { u"-ea",       Rel_EA },
        { u"-ea1",      Rel_EA1 },
        { u"-ea2",      Rel_EA2 },
        { u"-ea3",      Rel_EA3 },
        { u"-beta",     Rel_BETA },
        { u"-beta1",    Rel_BETA1 },
        { u"-beta2",    Rel_BETA2 },
        { u"-beta3",    Rel_BETA3 },
        { u"-rc",       Rel_RC },
        { u"-rc1",      Rel_RC1 },
        { u"-rc2",      Rel_RC2 },
        { u"-rc3",      Rel_RC3 },
    };

    if (sRelease.starts_with(u"-p"))
        return Rel_FreeBSD;

    auto releaseIter = aReleases.find(sRelease);
    return (releaseIter == aReleases.end()) ? Rel_NONE : releaseIter->second;
}

/* Examples:
   a) 1.0 < 1.1
   b) 1.0 < 1.0.0
   c)  1.0 < 1.0_00

   returns false if both values are equal
*/
bool SunVersion::operator > (const SunVersion& ver) const
{
    if( &ver == this)
        return false;

    //compare major.minor.maintenance
    auto const nVerParts = m_arVersionParts.size();
    auto const nOtherVerParts = ver.m_arVersionParts.size();
    for (size_t i = 0; i < std::min(nVerParts, nOtherVerParts); i++)
    {
        // 1.4 > 1.3
        if(m_arVersionParts[i] > ver.m_arVersionParts[i])
        {
            return true;
        }
        else if (m_arVersionParts[i] < ver.m_arVersionParts[i])
        {
            return false;
        }
    }
    if (nVerParts != nOtherVerParts)
        return nVerParts > nOtherVerParts;

    //major.minor.maintenance_update are equal. Test for a trailing char
    if (m_nUpdateSpecial > ver.m_nUpdateSpecial)
    {
        return true;
    }

    //Until here the versions are equal
    //compare pre -release values
    if ((m_preRelease == Rel_NONE && ver.m_preRelease == Rel_NONE)
        ||
        (m_preRelease != Rel_NONE && ver.m_preRelease == Rel_NONE))
        return false;
    else if (m_preRelease == Rel_NONE && ver.m_preRelease != Rel_NONE)
        return true;
    else if (m_preRelease > ver.m_preRelease)
        return true;

    return false;
}

bool SunVersion::operator < (const SunVersion& ver) const
{
    return (! operator > (ver)) && (! operator == (ver));
}

bool SunVersion::operator == (const SunVersion& ver) const
{
    if (m_arVersionParts.size() != ver.m_arVersionParts.size())
        return false;

    for (size_t i = 0; i < m_arVersionParts.size(); i++)
    {
        if( m_arVersionParts[i] != ver.m_arVersionParts[i])
            return false;
    }
    bool bRet = m_nUpdateSpecial == ver.m_nUpdateSpecial;
    bRet = m_preRelease == ver.m_preRelease && bRet;
    return bRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
