/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "lightproofimp.hxx"
#include "lpvm.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <cpo/uno/XComponentContext.hpp>

#include <config_folders.h>

#include <comphelper/kit.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <linguistic/misc.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <unicode/regex.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

using namespace css;
using namespace cpo;
using namespace css::linguistic2;

namespace lightproof
{
namespace
{
// The dictionary format name the rule packages register themselves under in
// their dictionaries.xcu.


icu::UnicodeString toIcu(const OUString& rString)
{
    return icu::UnicodeString(false, reinterpret_cast<const UChar*>(rString.getStr()),
                              rString.getLength());
}

OUString fromIcu(const icu::UnicodeString& rString)
{
    return OUString(reinterpret_cast<const sal_Unicode*>(rString.getBuffer()), rString.length());
}

// The capitalisation the rules apply to a suggestion when the match itself
// was capitalised: Python's str.capitalize(), with the dotted capital I for
// Turkish and Azerbaijani and the IJ digraph for Dutch.
OUString capitaliseSuggestion(const OUString& rString, const lang::Locale& rLocale)
{
    if (rString.isEmpty())
        return rString;

    if (rString[0] == 'i')
    {
        if (rLocale.Language == "tr" || rLocale.Language == "az")
            return OUStringChar(u'İ') + rString.subView(1);
        if (rLocale.Language == "nl" && rString.getLength() > 1 && rString[1] == 'j')
            return OUString::Concat("IJ") + rString.subView(2);
        return OUString::Concat("I") + rString.subView(1);
    }

    // toIcu() aliases the string's buffer rather than copying it, so the
    // source has to stay alive for as long as the UnicodeString does.
    const OUString aHead = rString.copy(0, 1);
    const OUString aTail = rString.copy(1);
    icu::UnicodeString aFirst(toIcu(aHead));
    icu::UnicodeString aRest(toIcu(aTail));
    aFirst.toUpper();
    aRest.toLower();
    return fromIcu(aFirst) + fromIcu(aRest);
}

// Expands a rule's replacement or message template against a match, the way
// Python's re expand() does: numbered backreferences, \g<name> and \g<number>,
// and the usual string escapes.
OUString expand(const OUString& rTemplate, icu::RegexMatcher& rMatcher, const RuleFile& rFile,
                const Rule& rRule)
{
    const sal_Int32 nLength = rTemplate.getLength();
    OUStringBuffer aResult(nLength);

    const auto groupByName = [&rFile, &rRule](std::u16string_view rName) -> sal_Int32 {
        for (sal_uInt32 i = 0; i < rRule.nGroupNameCount; ++i)
        {
            const GroupName& rGroup = rFile.getGroupName(rRule.nGroupNameFirst + i);
            if (rFile.getString(rGroup.nName) == rName)
                return static_cast<sal_Int32>(rGroup.nIndex);
        }
        return -1;
    };

    const auto appendGroup = [&aResult, &rMatcher](sal_Int32 nGroup) {
        UErrorCode nStatus = U_ZERO_ERROR;
        const icu::UnicodeString aGroup = rMatcher.group(nGroup, nStatus);
        if (U_SUCCESS(nStatus))
            aResult.append(fromIcu(aGroup));
    };

    sal_Int32 i = 0;
    while (i < nLength)
    {
        const sal_Unicode c = rTemplate[i];
        if (c != '\\' || i + 1 >= nLength)
        {
            aResult.append(c);
            ++i;
            continue;
        }

        const sal_Unicode cEscape = rTemplate[i + 1];
        if (cEscape >= '0' && cEscape <= '9')
        {
            sal_Int32 nGroup = 0;
            sal_Int32 j = i + 1;
            while (j < nLength && rTemplate[j] >= '0' && rTemplate[j] <= '9' && j - i <= 2)
                nGroup = nGroup * 10 + (rTemplate[j++] - '0');
            appendGroup(nGroup);
            i = j;
            continue;
        }

        if (cEscape == 'g' && i + 2 < nLength && rTemplate[i + 2] == '<')
        {
            const sal_Int32 nEnd = rTemplate.indexOf('>', i + 3);
            if (nEnd > 0)
            {
                const OUString aName = rTemplate.copy(i + 3, nEnd - i - 3);
                const sal_Int32 nGroup
                    = (aName.toInt32() != 0 || aName == "0") ? aName.toInt32() : groupByName(aName);
                if (nGroup >= 0)
                    appendGroup(nGroup);
                i = nEnd + 1;
                continue;
            }
        }

        switch (cEscape)
        {
            case 'n':
                aResult.append('\n');
                break;
            case 't':
                aResult.append('\t');
                break;
            case 'r':
                aResult.append('\r');
                break;
            case '\\':
                aResult.append('\\');
                break;
            default:
                aResult.append(cEscape);
                break;
        }
        i += 2;
    }

    return aResult.makeStringAndClear();
}

// The rules separate alternative suggestions with either a newline or a
// vertical bar.
std::vector<OUString> splitAlternatives(const OUString& rText)
{
    std::vector<OUString> aParts;
    sal_Int32 nIndex = 0;
    const OUString aNormalised = rText.replaceAll("|", "\n");
    do
    {
        aParts.push_back(aNormalised.getToken(0, '\n', nIndex));
    } while (nIndex >= 0);
    return aParts;
}
}

Lightproof::Lightproof() = default;
Lightproof::~Lightproof() = default;

void Lightproof::discoverPackages()
{
    if (m_bDiscovered)
        return;
    m_bDiscovered = true;

    // The rule files are installed together and name the languages they
    // serve themselves, so the directory is the whole of the registry. They
    // were once entries in a dictionary's dictionaries.xcu, which tied them
    // to the bundled dictionaries and to the per-profile extension
    // registration: a build configured to use the system dictionaries had no
    // rules at all, and a profile that had registered an older build kept its
    // answer. There is no system package to fall back on for sentence
    // checking the way there is for spelling.
    OUString aDir(u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/lightproof"_ustr);
    rtl::Bootstrap::expandMacros(aDir);

    osl::Directory aRules(aDir);
    if (aRules.open() != osl::FileBase::E_None)
    {
        SAL_WARN("lingucomponent.lightproof", "no rule directory <" << aDir << ">");
        m_aSupportedLocales = {};
        return;
    }

    std::vector<lang::Locale> aLocales;
    osl::DirectoryItem aItem;
    while (aRules.getNextItem(aItem) == osl::FileBase::E_None)
    {
        osl::FileStatus aStatus(osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_FileName);
        if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
            continue;
        if (!aStatus.getFileName().endsWithIgnoreAsciiCase(u".lpr"))
            continue;

        const OUString aLocation = aStatus.getFileURL();
        std::shared_ptr<const RuleFile> pFile = RuleFile::load(aLocation);
        if (!pFile)
        {
            SAL_WARN("lingucomponent.lightproof", "unreadable rule file <" << aLocation << ">");
            continue;
        }

        for (const OUString& rLocaleName : pFile->getLocales())
        {
            if (!comphelper::COKit::isAllowlistedLanguage(rLocaleName))
                continue;
            if (m_aLocationByLocale.emplace(rLocaleName, aLocation).second)
                aLocales.push_back(LanguageTag::convertToLocale(rLocaleName));
        }
    }

    m_aSupportedLocales = comphelper::containerToSequence(aLocales);
}

Package* Lightproof::getPackage(const lang::Locale& rLocale)
{
    discoverPackages();

    const OUString aTag = LanguageTag::convertToBcp47(rLocale, false);
    std::map<OUString, OUString>::const_iterator aLocation = m_aLocationByLocale.find(aTag);
    if (aLocation == m_aLocationByLocale.end())
        return nullptr;

    std::map<OUString, std::unique_ptr<Package>>::iterator aFound
        = m_aPackages.find(aLocation->second);
    if (aFound != m_aPackages.end())
        return aFound->second.get();

    std::shared_ptr<const RuleFile> pFile = RuleFile::load(aLocation->second);
    if (!pFile)
        return nullptr;

    std::unique_ptr<Package> pPackage(new Package);
    pPackage->pFile = pFile;
    pPackage->aRules.resize(pFile->getRuleCount());
    pPackage->aOptions.reserve(pFile->getOptionCount());
    for (sal_uInt32 i = 0; i < pFile->getOptionCount(); ++i)
        pPackage->aOptions.push_back(pFile->getOption(i).nDefault != 0);

    Package* pResult = pPackage.get();
    m_aPackages.emplace(aLocation->second, std::move(pPackage));
    return pResult;
}

cpo::uno::Sequence<lang::Locale> Lightproof::getLocales()
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    discoverPackages();
    return m_aSupportedLocales;
}

bool Lightproof::hasLocale(const lang::Locale& rLocale)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    discoverPackages();
    return m_aLocationByLocale.find(LanguageTag::convertToBcp47(rLocale, false))
           != m_aLocationByLocale.end();
}

bool Lightproof::isSpellChecker() { return false; }

ProofreadingResult Lightproof::doProofreading(
    const OUString& aDocumentIdentifier, const OUString& aText, const lang::Locale& aLocale,
    sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition,
    const cpo::uno::Sequence<beans::PropertyValue>& /*aProperties*/)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());

    ProofreadingResult aResult;
    aResult.aDocumentIdentifier = aDocumentIdentifier;
    aResult.aText = aText;
    aResult.aLocale = aLocale;
    aResult.nStartOfSentencePosition = nStartOfSentencePosition;
    aResult.nBehindEndOfSentencePosition = nSuggestedBehindEndOfSentencePosition;
    aResult.nStartOfNextSentencePosition = nSuggestedBehindEndOfSentencePosition;
    aResult.xProofreader = this;

    // Lightproof rules match across sentence boundaries, so a paragraph is
    // checked as a single unit on the first request and the rest of the
    // paragraph's requests return nothing.
    if (nStartOfSentencePosition != 0)
        return aResult;

    aResult.nBehindEndOfSentencePosition = aText.getLength();
    aResult.nStartOfNextSentencePosition = aText.getLength();

    Package* pPackage = getPackage(aLocale);
    if (pPackage == nullptr || aText.isEmpty())
        return aResult;

    const RuleFile& rFile = *pPackage->pFile;
    const icu::UnicodeString aInput = toIcu(aText);
    const Context aContext{ rFile, aLocale, aText, pPackage->aOptions };

    std::vector<SingleProofreadingError> aErrors;
    for (sal_uInt32 nRuleIndex = 0; nRuleIndex < rFile.getRuleCount(); ++nRuleIndex)
    {
        const Rule& rRule = rFile.getRule(nRuleIndex);
        CompiledRule& rCompiled = pPackage->aRules[nRuleIndex];

        if (!rCompiled.bCompiled)
        {
            rCompiled.bCompiled = true;
            rCompiled.aIdentifier = rFile.getString(rRule.nPattern);
            UErrorCode nStatus = U_ZERO_ERROR;
            UParseError aParseError;
            rCompiled.pPattern.reset(icu::RegexPattern::compile(
                toIcu(rCompiled.aIdentifier), rRule.nFlags, aParseError, nStatus));
            if (U_FAILURE(nStatus))
            {
                SAL_WARN("lingucomponent.lightproof", "rule " << nRuleIndex << " of "
                                                              << rFile.getPackage()
                                                              << " does not compile");
                rCompiled.pPattern.reset();
            }
            else
            {
                rCompiled.pMatcher.reset(rCompiled.pPattern->matcher(nStatus));
                if (U_FAILURE(nStatus))
                    rCompiled.pMatcher.reset();
            }
        }
        if (!rCompiled.pMatcher)
            continue;
        if (!m_aIgnoredRules.empty() && m_aIgnoredRules.count(rCompiled.aIdentifier))
            continue;

        icu::RegexMatcher& rMatcher = *rCompiled.pMatcher;
        rMatcher.reset(aInput);

        UErrorCode nStatus = U_ZERO_ERROR;
        while (rMatcher.find(nStatus) && U_SUCCESS(nStatus))
        {
            if (rRule.nConditionCode != 0
                && !run(aContext, rFile.getCode(rRule.nConditionCode)).isTrue())
                continue;

            const sal_Int32 nGroup = static_cast<sal_Int32>(rRule.nGroup);
            UErrorCode nGroupStatus = U_ZERO_ERROR;
            const sal_Int32 nStart = rMatcher.start(nGroup, nGroupStatus);
            const sal_Int32 nEnd = rMatcher.end(nGroup, nGroupStatus);
            if (U_FAILURE(nGroupStatus) || nStart < 0 || nEnd < nStart)
                continue;

            SingleProofreadingError aError;
            aError.nErrorStart = nStartOfSentencePosition + nStart;
            aError.nErrorLength = nEnd - nStart;
            aError.nErrorType = text::TextMarkupType::PROOFREADING;
            aError.aRuleIdentifier = rCompiled.aIdentifier;

            const bool bCapitalise = (rRule.nFlags & UREGEX_CASE_INSENSITIVE) != 0 && nEnd > nStart
                                     && u_isupper(aText[nStart]);

            const OUString aTemplate
                = rRule.nReplacementCode != 0 ? OUString() : rFile.getString(rRule.nReplacement);
            if (rRule.nReplacementCode != 0 || aTemplate != "_")
            {
                const OUString aReplacement
                    = rRule.nReplacementCode != 0
                          ? run(aContext, rFile.getCode(rRule.nReplacementCode)).getString()
                          : expand(aTemplate, rMatcher, rFile, rRule);
                std::vector<OUString> aSuggestions = splitAlternatives(aReplacement);
                if (bCapitalise)
                {
                    for (OUString& rSuggestion : aSuggestions)
                        rSuggestion = capitaliseSuggestion(rSuggestion, aLocale);
                }
                aError.aSuggestions = comphelper::containerToSequence(aSuggestions);
            }

            OUString aComment = rRule.nMessageCode != 0
                                    ? run(aContext, rFile.getCode(rRule.nMessageCode)).getString()
                                    : expand(rFile.getString(rRule.nMessage), rMatcher, rFile, rRule);
            aComment = aComment.replaceAll("\\n", "\n");
            const std::vector<OUString> aCommentLines = splitAlternatives(aComment);
            aError.aShortComment = aCommentLines.front().trim();
            aError.aFullComment = aCommentLines.back().trim();

            if (aError.aFullComment.indexOf("://") >= 0)
            {
                beans::PropertyValue aProperty;
                aProperty.Name = u"FullCommentURL"_ustr;
                aProperty.Value <<= aError.aFullComment;
                aError.aFullComment = aError.aShortComment;
                aError.aProperties = { aProperty };
            }

            aErrors.push_back(aError);
        }
    }

    aResult.aErrors = comphelper::containerToSequence(aErrors);
    return aResult;
}

void Lightproof::ignoreRule(const OUString& aRuleIdentifier, const lang::Locale&)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    m_aIgnoredRules.insert(aRuleIdentifier);
}

void Lightproof::resetIgnoreRules()
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    m_aIgnoredRules.clear();
}

OUString Lightproof::getServiceDisplayName(const lang::Locale& rLocale)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    Package* pPackage = getPackage(rLocale);
    if (pPackage)
        return pPackage->pFile->getDisplayName();
    return u"Lightproof"_ustr;
}

OUString Lightproof::getImplementationName()
{
    return u"cpo.lingu.Lightproof"_ustr;
}

bool Lightproof::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence<OUString> Lightproof::getSupportedServiceNames()
{
    return { u"com.sun.star.linguistic2.Proofreader"_ustr };
}
}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
lingucomponent_Lightproof_get_implementation(cpo::uno::XComponentContext*,
                                             cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new lightproof::Lightproof());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
