/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace lightproof
{
// One compiled rule. All members but nFlags, nGroup and the group-name range
// are offsets: into the string blob for text, into the bytecode blob (biased
// by one, so zero means "no expression") for code.
struct Rule
{
    sal_uInt32 nPattern;
    sal_uInt32 nFlags;
    sal_uInt32 nReplacement;
    sal_uInt32 nReplacementCode;
    sal_uInt32 nMessage;
    sal_uInt32 nMessageCode;
    sal_uInt32 nConditionCode;
    sal_uInt32 nGroup;
    sal_uInt32 nGroupNameFirst;
    sal_uInt32 nGroupNameCount;
};

struct Option
{
    sal_uInt32 nName;
    sal_uInt32 nDefault;
};

struct GroupName
{
    sal_uInt32 nName;
    sal_uInt32 nIndex;
};

// A constant the rule conditions look things up in: a sorted word set, a
// sorted string-to-string map, or one of the module-level patterns.
struct Constant
{
    enum Type
    {
        Set = 0,
        Map = 1,
        Regex = 2
    };

    sal_uInt32 nType;
    sal_uInt32 nCount;
    // For Set and Map an offset into the constant blob, for Regex the
    // pattern's offset into the string blob.
    sal_uInt32 nData;
    sal_uInt32 nFlags;
};

// A rule package as produced by lpcompile.py. Immutable once loaded, and
// shared between every document the checker serves.
class RuleFile
{
public:
    // Returns nullptr when the file is missing, truncated, or not a rule
    // package this build understands.
    static std::shared_ptr<const RuleFile> load(const OUString& rFileUrl);

    OUString getString(sal_uInt32 nOffset) const;

    const OUString& getPackage() const { return m_aPackage; }
    const OUString& getDisplayName() const { return m_aDisplayName; }
    const OUString& getLicence() const { return m_aLicence; }

    const std::vector<OUString>& getLocales() const { return m_aLocales; }

    sal_uInt32 getRuleCount() const { return m_nRuleCount; }
    const Rule& getRule(sal_uInt32 nIndex) const;

    sal_uInt32 getOptionCount() const { return m_nOptionCount; }
    const Option& getOption(sal_uInt32 nIndex) const;
    // The option's index in file order, or -1 when the package has no such
    // option.
    sal_Int32 findOption(std::u16string_view rName) const;

    const GroupName& getGroupName(sal_uInt32 nIndex) const;

    sal_uInt32 getConstantCount() const { return m_nConstantCount; }
    const Constant& getConstant(sal_uInt32 nIndex) const;
    // True if the set or map constant holds the string as a member or key.
    bool constantContains(sal_uInt32 nIndex, const OString& rUtf8Needle) const;
    // The value the map constant gives the key, or an empty optional when it
    // holds no such key.
    std::optional<OUString> constantLookup(sal_uInt32 nIndex, const OString& rUtf8Key) const;
    // The members of a set constant, in order.
    sal_uInt32 constantEntryCount(sal_uInt32 nIndex) const;
    OUString constantEntry(sal_uInt32 nIndex, sal_uInt32 nEntry) const;

    const sal_uInt8* getCode(sal_uInt32 nBiasedOffset) const;
    // One past the last byte of the code, so a running expression knows where
    // it has to stop.
    const sal_uInt8* getCodeEnd() const
    {
        return m_pCode == nullptr ? nullptr : m_pCode + m_nCodeSize;
    }

private:
    RuleFile() = default;
    bool parse();
    // The string at an offset that came out of the constant blob, empty when
    // the offset is not one the string blob holds.
    const char* getCString(sal_uInt32 nOffset) const;

    std::vector<sal_uInt8> m_aData;
    OUString m_aPackage;
    OUString m_aDisplayName;
    OUString m_aLicence;
    std::vector<OUString> m_aLocales;
    std::map<OUString, sal_Int32> m_aOptionIndex;

    const Rule* m_pRules = nullptr;
    sal_uInt32 m_nRuleCount = 0;
    const Option* m_pOptions = nullptr;
    sal_uInt32 m_nOptionCount = 0;
    const GroupName* m_pGroupNames = nullptr;
    sal_uInt32 m_nGroupNameCount = 0;
    const Constant* m_pConstants = nullptr;
    sal_uInt32 m_nConstantCount = 0;
    const sal_uInt8* m_pConstantData = nullptr;
    sal_uInt32 m_nConstantDataSize = 0;
    const sal_uInt8* m_pCode = nullptr;
    sal_uInt32 m_nCodeSize = 0;
    const char* m_pStrings = nullptr;
    sal_uInt32 m_nStringsSize = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
