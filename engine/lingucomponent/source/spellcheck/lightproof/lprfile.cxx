/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "lprfile.hxx"

#include <osl/file.hxx>
#include <sal/log.hxx>

#include <cassert>
#include <cstring>

namespace lightproof
{
namespace
{
constexpr char LPR_MAGIC[8] = { 'L', 'P', 'R', 'O', 'O', 'F', '\0', '\0' };
constexpr sal_uInt32 LPR_VERSION = 1;
constexpr sal_uInt32 LPR_HEADER_SIZE = 8 + 4 * 19;

sal_uInt32 readU32(const sal_uInt8* pAt)
{
    return static_cast<sal_uInt32>(pAt[0]) | (static_cast<sal_uInt32>(pAt[1]) << 8)
           | (static_cast<sal_uInt32>(pAt[2]) << 16) | (static_cast<sal_uInt32>(pAt[3]) << 24);
}
}

std::shared_ptr<const RuleFile> RuleFile::load(const OUString& rFileUrl)
{
    osl::File aFile(rFileUrl);
    if (aFile.open(osl_File_OpenFlag_Read) != osl::FileBase::E_None)
    {
        SAL_WARN("lingucomponent.lightproof", "cannot open <" << rFileUrl << ">");
        return nullptr;
    }

    sal_uInt64 nSize = 0;
    if (aFile.getSize(nSize) != osl::FileBase::E_None || nSize < LPR_HEADER_SIZE
        || nSize > SAL_MAX_UINT32)
    {
        SAL_WARN("lingucomponent.lightproof", "bad size for <" << rFileUrl << ">");
        return nullptr;
    }

    std::shared_ptr<RuleFile> pFile(new RuleFile);
    pFile->m_aData.resize(static_cast<size_t>(nSize));
    sal_uInt64 nRead = 0;
    if (aFile.read(pFile->m_aData.data(), nSize, nRead) != osl::FileBase::E_None || nRead != nSize)
    {
        SAL_WARN("lingucomponent.lightproof", "short read on <" << rFileUrl << ">");
        return nullptr;
    }

    if (!pFile->parse())
    {
        SAL_WARN("lingucomponent.lightproof", "not a usable rule package: <" << rFileUrl << ">");
        return nullptr;
    }

    SAL_INFO("lingucomponent.lightproof",
             "loaded " << pFile->m_nRuleCount << " rules for " << pFile->m_aPackage << " from <"
                       << rFileUrl << ">: " << pFile->m_aLicence);
    return pFile;
}

bool RuleFile::parse()
{
    const sal_uInt8* pBase = m_aData.data();
    const sal_uInt32 nTotal = static_cast<sal_uInt32>(m_aData.size());

    if (std::memcmp(pBase, LPR_MAGIC, sizeof(LPR_MAGIC)) != 0)
        return false;

    sal_uInt32 aHeader[19];
    for (int i = 0; i < 19; ++i)
        aHeader[i] = readU32(pBase + 8 + 4 * i);

    if (aHeader[0] != LPR_VERSION)
        return false;

    const sal_uInt32 nPackage = aHeader[2];
    const sal_uInt32 nDisplayName = aHeader[3];
    const sal_uInt32 nLicence = aHeader[4];
    const sal_uInt32 nLocaleCount = aHeader[5];
    const sal_uInt32 nLocaleOffset = aHeader[6];
    m_nOptionCount = aHeader[7];
    const sal_uInt32 nOptionOffset = aHeader[8];
    m_nRuleCount = aHeader[9];
    const sal_uInt32 nRuleOffset = aHeader[10];
    m_nGroupNameCount = aHeader[11];
    const sal_uInt32 nGroupNameOffset = aHeader[12];
    const sal_uInt32 nCodeOffset = aHeader[13];
    m_nCodeSize = aHeader[14];
    const sal_uInt32 nStringsOffset = aHeader[17];
    m_nStringsSize = aHeader[18];

    // Every section must lie inside the file, and the sizes must not overflow
    // while we check that.
    struct Span
    {
        sal_uInt32 nOffset;
        sal_uInt64 nBytes;
    };
    const Span aSpans[] = {
        { nLocaleOffset, sal_uInt64(nLocaleCount) * 4 },
        { nOptionOffset, sal_uInt64(m_nOptionCount) * sizeof(Option) },
        { nRuleOffset, sal_uInt64(m_nRuleCount) * sizeof(Rule) },
        { nGroupNameOffset, sal_uInt64(m_nGroupNameCount) * sizeof(GroupName) },
        { nCodeOffset, m_nCodeSize },
        { nStringsOffset, m_nStringsSize },
    };
    for (const Span& rSpan : aSpans)
    {
        if (rSpan.nOffset > nTotal || rSpan.nBytes > nTotal - rSpan.nOffset)
            return false;
    }

    // The string blob is read as NUL-terminated C strings, so it has to end
    // in one.
    if (m_nStringsSize == 0 || pBase[nStringsOffset + m_nStringsSize - 1] != 0)
        return false;

    m_pStrings = reinterpret_cast<const char*>(pBase + nStringsOffset);
    m_pRules = reinterpret_cast<const Rule*>(pBase + nRuleOffset);
    m_pOptions = reinterpret_cast<const Option*>(pBase + nOptionOffset);
    m_pGroupNames = reinterpret_cast<const GroupName*>(pBase + nGroupNameOffset);
    m_pCode = pBase + nCodeOffset;

    m_aPackage = getString(nPackage);
    m_aDisplayName = getString(nDisplayName);
    m_aLicence = getString(nLicence);
    m_aLocales.reserve(nLocaleCount);
    for (sal_uInt32 i = 0; i < nLocaleCount; ++i)
        m_aLocales.push_back(getString(readU32(pBase + nLocaleOffset + 4 * i)));
    for (sal_uInt32 i = 0; i < m_nOptionCount; ++i)
        m_aOptionIndex.emplace(getString(m_pOptions[i].nName), static_cast<sal_Int32>(i));

    return !m_aPackage.isEmpty() && m_nRuleCount != 0;
}

OUString RuleFile::getString(sal_uInt32 nOffset) const
{
    if (nOffset >= m_nStringsSize)
        return OUString();
    return OUString(m_pStrings + nOffset, std::strlen(m_pStrings + nOffset),
                    RTL_TEXTENCODING_UTF8);
}

const Rule& RuleFile::getRule(sal_uInt32 nIndex) const
{
    // A file naming an entry it does not hold is broken, and a build with no
    // assertions in it must answer rather than read past the array. What it
    // answers holds no offsets, so a rule made of it does nothing.
    static const Rule aNone = {};
    assert(nIndex < m_nRuleCount);
    if (nIndex >= m_nRuleCount)
        return aNone;
    return m_pRules[nIndex];
}

const Option& RuleFile::getOption(sal_uInt32 nIndex) const
{
    static const Option aNone = {};
    assert(nIndex < m_nOptionCount);
    if (nIndex >= m_nOptionCount)
        return aNone;
    return m_pOptions[nIndex];
}

const GroupName& RuleFile::getGroupName(sal_uInt32 nIndex) const
{
    static const GroupName aNone = {};
    assert(nIndex < m_nGroupNameCount);
    if (nIndex >= m_nGroupNameCount)
        return aNone;
    return m_pGroupNames[nIndex];
}

sal_Int32 RuleFile::findOption(std::u16string_view rName) const
{
    std::map<OUString, sal_Int32>::const_iterator aFound = m_aOptionIndex.find(OUString(rName));
    return aFound == m_aOptionIndex.end() ? -1 : aFound->second;
}

const sal_uInt8* RuleFile::getCode(sal_uInt32 nBiasedOffset) const
{
    if (nBiasedOffset == 0 || nBiasedOffset - 1 >= m_nCodeSize)
        return nullptr;
    return m_pCode + (nBiasedOffset - 1);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
