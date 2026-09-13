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

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <cpo/uno/XComponentContext.hpp>

#include <config_folders.h>

#include <comphelper/kit.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <linguistic/misc.hxx>
#include <osl/file.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <unicode/regex.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <algorithm>

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

// Watches the option node so a change made in the Options dialog reaches a
// document without reloading it. Holds the checker weakly, by raw pointer,
// which ~Lightproof clears before it removes the listener.
class ConfigurationListener final : public cppu::WeakImplHelper<css::util::XChangesListener>
{
public:
    explicit ConfigurationListener(Lightproof* pOwner)
        : m_pOwner(pOwner)
    {
    }

    void detach() { m_pOwner = nullptr; }

    void changesOccurred(const css::util::ChangesEvent&) override
    {
        osl::MutexGuard aGuard(linguistic::GetLinguMutex());
        if (m_pOwner)
            m_pOwner->reloadOptions();
    }

    void disposing(const lang::EventObject&) override
    {
        osl::MutexGuard aGuard(linguistic::GetLinguMutex());
        m_pOwner = nullptr;
    }

private:
    Lightproof* m_pOwner;
};

Lightproof::Lightproof() = default;

Lightproof::~Lightproof()
{
    if (!m_xConfigListener)
        return;
    static_cast<ConfigurationListener*>(m_xConfigListener.get())->detach();
    try
    {
        uno::Reference<util::XChangesNotifier> xNotifier(m_xConfigNode, uno::UNO_QUERY);
        if (xNotifier)
            xNotifier->removeChangesListener(m_xConfigListener);
    }
    catch (const cpo::uno::Exception&)
    {
    }
}

void Lightproof::loadOptions(Package& rPackage)
{
    const RuleFile& rFile = *rPackage.pFile;

    rPackage.aOptions.clear();
    rPackage.aOptions.reserve(rFile.getOptionCount());
    for (sal_uInt32 i = 0; i < rFile.getOptionCount(); ++i)
        rPackage.aOptions.push_back(rFile.getOption(i).nDefault != 0);

    if (!m_xConfigNode)
    {
        try
        {
            uno::Reference<lang::XMultiServiceFactory> xProvider(
                css::configuration::theDefaultProvider::get(
                    comphelper::getProcessComponentContext()));
            // The configuration node holding one group of option flags per
            // rule package.
            const beans::NamedValue aPath(
                u"nodepath"_ustr,
                cpo::uno::Any(
                    u"/org.openoffice.Office.Linguistic/GrammarChecking/SentenceChecking"_ustr));
            m_xConfigNode.set(xProvider->createInstanceWithArguments(
                                  u"com.sun.star.configuration.ConfigurationAccess"_ustr,
                                  { cpo::uno::Any(aPath) }),
                              uno::UNO_QUERY);

            uno::Reference<util::XChangesNotifier> xNotifier(m_xConfigNode, uno::UNO_QUERY);
            if (xNotifier)
            {
                m_xConfigListener.set(new ConfigurationListener(this));
                xNotifier->addChangesListener(m_xConfigListener);
            }
        }
        catch (const cpo::uno::Exception&)
        {
            SAL_INFO("lingucomponent.lightproof", "no option configuration, using defaults");
        }
    }

    if (!m_xConfigNode)
        return;

    try
    {
        if (!m_xConfigNode->hasByName(rFile.getPackage()))
            return;
        uno::Reference<container::XNameAccess> xGroup(
            m_xConfigNode->getByName(rFile.getPackage()), uno::UNO_QUERY);
        if (!xGroup)
            return;
        for (sal_uInt32 i = 0; i < rFile.getOptionCount(); ++i)
        {
            const OUString aName = rFile.getString(rFile.getOption(i).nName);
            bool bValue = false;
            if (xGroup->hasByName(aName) && (xGroup->getByName(aName) >>= bValue))
                rPackage.aOptions[i] = bValue;
        }
    }
    catch (const cpo::uno::Exception&)
    {
        SAL_WARN("lingucomponent.lightproof",
                 "cannot read options for " << rFile.getPackage() << ", using defaults");
    }
}

void Lightproof::reloadOptions()
{
    for (const std::pair<const OUString, std::unique_ptr<Package>>& rEntry : m_aPackages)
        loadOptions(*rEntry.second);

    const linguistic2::LinguServiceEvent aEvent(
        static_cast<cppu::OWeakObject*>(this), linguistic2::LinguServiceEventFlags::PROOFREAD_AGAIN);
    const std::vector<uno::Reference<linguistic2::XLinguServiceEventListener>> aListeners(
        m_aEventListeners);
    for (const uno::Reference<linguistic2::XLinguServiceEventListener>& rListener : aListeners)
    {
        try
        {
            rListener->processLinguServiceEvent(aEvent);
        }
        catch (const cpo::uno::Exception&)
        {
        }
    }
}

bool Lightproof::addLinguServiceEventListener(
    const uno::Reference<linguistic2::XLinguServiceEventListener>& xListener)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    if (!xListener)
        return false;
    if (std::find(m_aEventListeners.begin(), m_aEventListeners.end(), xListener)
        != m_aEventListeners.end())
        return false;
    m_aEventListeners.push_back(xListener);
    return true;
}

bool Lightproof::removeLinguServiceEventListener(
    const uno::Reference<linguistic2::XLinguServiceEventListener>& xListener)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    const std::vector<uno::Reference<linguistic2::XLinguServiceEventListener>>::iterator aFound
        = std::find(m_aEventListeners.begin(), m_aEventListeners.end(), xListener);
    if (aFound == m_aEventListeners.end())
        return false;
    m_aEventListeners.erase(aFound);
    return true;
}

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

    Package* pResult = pPackage.get();
    loadOptions(*pResult);
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
    m_pRunningPackage = pPackage;
    Context aContext{ rFile, aLocale, aText, pPackage->aOptions, nullptr, *this };

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
        aContext.pMatcher = &rMatcher;

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

    m_pRunningPackage = nullptr;
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

bool Lightproof::spell(const lang::Locale& rLocale, const OUString& rWord)
{
    if (rWord.isEmpty())
        return false;
    if (!m_bSpellCheckerTried)
    {
        m_bSpellCheckerTried = true;
        try
        {
            const uno::Reference<cpo::uno::XComponentContext>& xContext(
                comphelper::getProcessComponentContext());
            m_xSpellChecker.set(xContext->getServiceManager()->createInstanceWithContext(
                                    u"com.sun.star.linguistic2.SpellChecker"_ustr, xContext),
                                uno::UNO_QUERY);
        }
        catch (const cpo::uno::Exception&)
        {
        }
    }
    if (!m_xSpellChecker)
        return false;
    try
    {
        return m_xSpellChecker->isValid(rWord, rLocale, {});
    }
    catch (const cpo::uno::Exception&)
    {
        return false;
    }
}

const std::vector<OUString>& Lightproof::getAnalyses(const lang::Locale& rLocale,
                                                     const OUString& rWord)
{
    static const std::vector<OUString> aNone;

    const std::pair<OUString, OUString> aKey(LanguageTag(rLocale).getBcp47(), rWord);
    std::map<std::pair<OUString, OUString>, std::vector<OUString>>::const_iterator aFound
        = m_aAnalyses.find(aKey);
    if (aFound != m_aAnalyses.end())
        return aFound->second;

    // Make sure the dispatcher exists before the SPELLML query goes through it.
    spell(rLocale, u"a"_ustr);
    if (!m_xSpellChecker)
        return aNone;

    std::vector<OUString> aResult;
    try
    {
        const uno::Reference<linguistic2::XSpellAlternatives> xAlternatives
            = m_xSpellChecker->spell(u"<?xml?><query type=\'analyze\'><word>"_ustr + rWord
                                         + u"</word></query>"_ustr,
                                     rLocale, {});
        if (xAlternatives)
        {
            const cpo::uno::Sequence<OUString> aWords(xAlternatives->getAlternatives());
            if (aWords.hasElements())
            {
                // The analyses come back in one string, each closed by </a>.
                sal_Int32 nIndex = 0;
                const OUString& rPacked = aWords[0];
                while (nIndex >= 0 && nIndex < rPacked.getLength())
                {
                    const sal_Int32 nEnd = rPacked.indexOf(u"</a>", nIndex);
                    if (nEnd < 0)
                        break;
                    aResult.push_back(rPacked.copy(nIndex, nEnd - nIndex));
                    nIndex = nEnd + 4;
                }
            }
        }
    }
    catch (const cpo::uno::Exception&)
    {
    }

    return m_aAnalyses.emplace(aKey, std::move(aResult)).first->second;
}

icu::RegexMatcher* Lightproof::getMorphMatcher(const OUString& rPattern)
{
    std::map<OUString, std::unique_ptr<icu::RegexMatcher>>::const_iterator aFound
        = m_aMorphMatchers.find(rPattern);
    if (aFound != m_aMorphMatchers.end())
        return aFound->second.get();

    std::unique_ptr<icu::RegexMatcher> pMatcher;
    UErrorCode nStatus = U_ZERO_ERROR;
    pMatcher.reset(new icu::RegexMatcher(toIcu(rPattern), 0, nStatus));
    if (U_FAILURE(nStatus))
        pMatcher.reset();
    return m_aMorphMatchers.emplace(rPattern, std::move(pMatcher)).first->second.get();
}

OUString Lightproof::morph(const lang::Locale& rLocale, const OUString& rWord,
                           const OUString& rPattern, bool bAll, bool bOnlyAffix)
{
    if (rWord.isEmpty())
        return OUString();
    if (bOnlyAffix)
    {
        SAL_WARN("lingucomponent.lightproof", "affix() is not implemented");
        return OUString();
    }

    icu::RegexMatcher* pMatcher = getMorphMatcher(rPattern);
    if (pMatcher == nullptr)
        return OUString();

    OUString aResult;
    for (const OUString& rAnalysis : getAnalyses(rLocale, rWord))
    {
        const icu::UnicodeString aSubject = toIcu(rAnalysis);
        pMatcher->reset(aSubject);
        UErrorCode nStatus = U_ZERO_ERROR;
        if (pMatcher->find(nStatus) && U_SUCCESS(nStatus))
        {
            UErrorCode nGroupStatus = U_ZERO_ERROR;
            aResult = fromIcu(pMatcher->group(0, nGroupStatus));
            if (!bAll)
                return aResult;
        }
        else if (bAll)
        {
            // "all" asks that every analysis match.
            return OUString();
        }
    }
    return aResult;
}

namespace
{
// The rules build the number to convert by concatenation, so it can be an
// expression as well as a literal: "5" or "5*12+3". Products bind tighter
// than sums, which is all the rules use.
bool evaluateNumber(const OUString& rText, double& rResult)
{
    const sal_Int32 nLength = rText.getLength();
    sal_Int32 nAt = 0;
    double nSum = 0.0;
    bool bAny = false;

    while (nAt < nLength)
    {
        double nProduct = 1.0;
        bool bFactor = false;
        for (;;)
        {
            rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
            sal_Int32 nParsedEnd = 0;
            const double nValue = rtl::math::stringToDouble(rText.subView(nAt), '.', 0, &eStatus,
                                                            &nParsedEnd);
            if (eStatus != rtl_math_ConversionStatus_Ok || nParsedEnd == 0)
                return false;
            nAt += nParsedEnd;
            nProduct *= nValue;
            bFactor = true;
            if (nAt < nLength && rText[nAt] == '*')
            {
                ++nAt;
                continue;
            }
            break;
        }
        if (!bFactor)
            return false;
        nSum += nProduct;
        bAny = true;
        if (nAt < nLength && rText[nAt] == '+')
        {
            ++nAt;
            continue;
        }
        break;
    }

    if (!bAny || nAt != nLength)
        return false;
    rResult = nSum;
    return true;
}

// The units the measurement rules convert between, with the factors Calc's
// CONVERT uses (scaddins/source/analysis/analysishelper.cxx). Doing this here
// rather than through FunctionAccess keeps the checker off the SolarMutex,
// which it must not take: it runs on the grammar checking thread while the
// main thread can be waiting on it.
struct Unit
{
    const char* pName;
    // What one base unit is worth in this unit.
    double fFactor;
    // Zero for everything but temperature, which needs an offset from the
    // base as well as a factor.
    double fOffset;
    char cClass;
    // The decimal exponent of an SI prefix already folded into the name.
    sal_Int16 nLevel;
};

constexpr Unit UNITS[] = {
    { "g", 1.0, 0.0, 'm', 0 },
    { "kg", 1.0, 0.0, 'm', 3 },
    { "lbm", 2.2046229146913400E-03, 0.0, 'm', 0 },

    { "m", 1.0, 0.0, 'l', 0 },
    { "mm", 1.0, 0.0, 'l', -3 },
    { "cm", 1.0, 0.0, 'l', -2 },
    { "km", 1.0, 0.0, 'l', 3 },
    { "mi", 6.2137119223733397E-04, 0.0, 'l', 0 },
    { "in", 3.9370078740157480E01, 0.0, 'l', 0 },
    { "ft", 3.2808398950131234E00, 0.0, 'l', 0 },
    { "yd", 1.0936132983377078E00, 0.0, 'l', 0 },

    { "l", 1.0, 0.0, 'v', 0 },
    { "dl", 1.0, 0.0, 'v', -1 },
    { "pt", 2.1133764188651873E00, 0.0, 'v', 0 },
    { "uk_pt", 1.7597539863927023E00, 0.0, 'v', 0 },
    { "gal", 2.6417205235814842E-01, 0.0, 'v', 0 },
    { "uk_gal", 2.1996924829908779E-01, 0.0, 'v', 0 },

    { "m/s", 1.0, 0.0, 's', 0 },
    { "m/h", 3.6000000000000000E03, 0.0, 's', 0 },
    { "km/h", 3.6000000000000000E03, 0.0, 's', 3 },
    { "mph", 2.2369362920544023E00, 0.0, 's', 0 },

    { "K", 1.0, 0.0, 't', 0 },
    { "C", 1.0, -2.7315000000000000E02, 't', 0 },
    { "F", 1.8000000000000000E00, -2.5537222222222222E02, 't', 0 },
};

const Unit* findUnit(std::u16string_view rName)
{
    for (const Unit& rUnit : UNITS)
    {
        if (o3tl::equalsAscii(rName, rUnit.pName))
            return &rUnit;
    }
    return nullptr;
}

// Calc's CONVERT for the units above: a factor and a prefix exponent, with
// temperature going through the base with its offset.
bool convertUnit(double fValue, std::u16string_view rFrom, std::u16string_view rTo, double& rResult)
{
    const Unit* pFrom = findUnit(rFrom);
    const Unit* pTo = findUnit(rTo);
    if (pFrom == nullptr || pTo == nullptr || pFrom->cClass != pTo->cClass)
        return false;

    if (pFrom->cClass == 't')
    {
        const double fBase = rtl::math::pow10Exp(fValue, pFrom->nLevel) / pFrom->fFactor
                             - pFrom->fOffset;
        rResult = rtl::math::pow10Exp((fBase + pTo->fOffset) * pTo->fFactor, -pTo->nLevel);
        return true;
    }

    rResult = rtl::math::pow10Exp(fValue * pTo->fFactor / pFrom->fFactor,
                                  pFrom->nLevel - pTo->nLevel);
    return true;
}

// Python's str() of a float, which always shows a decimal point.
OUString formatLikePython(double nValue)
{
    OUString aText = rtl::math::doubleToUString(nValue, rtl_math_StringFormat_Automatic,
                                                rtl_math_DecimalPlaces_Max, '.', true);
    if (aText.indexOf('.') < 0 && aText.indexOf('e') < 0 && aText.indexOf('E') < 0)
        aText += ".0";
    return aText;
}
}

OUString Lightproof::measurement(const OUString& rNumber, const OUString& rFrom,
                                 const OUString& rTo, const OUString& rSuffix,
                                 const OUString& rDecimal, const OUString& rRemove)
{
    OUString aNumber = rNumber;
    if (rFrom == "ft" || rFrom == "in" || rFrom == "mi")
    {
        aNumber = aNumber.replaceAll(u" 1/2"_ustr, u".5"_ustr)
                      .replaceAll(u" \u00BD"_ustr, u".5"_ustr)
                      .replaceAll(u"\u00BD"_ustr, u".5"_ustr);
    }
    if (!rRemove.isEmpty())
        aNumber = aNumber.replaceAll(rRemove, u""_ustr);
    if (!rDecimal.isEmpty())
        aNumber = aNumber.replaceAll(rDecimal, u"."_ustr);
    aNumber = aNumber.replaceAll(u"\u2212"_ustr, u"-"_ustr);

    double nValue = 0.0;
    if (!evaluateNumber(aNumber, nValue))
        return OUString();

    double nConverted = 0.0;
    if (!convertUnit(nValue, rFrom, rTo, nConverted))
    {
        SAL_WARN("lingucomponent.lightproof",
                 "no conversion from " << rFrom << " to " << rTo);
        return OUString();
    }

    // The same rounded forms the rules offer, shortest first and without
    // repeats.
    std::vector<OUString> aCandidates{
        rtl::math::doubleToUString(rtl::math::round(nConverted, 0), rtl_math_StringFormat_F, 0,
                                   '.', true),
        formatLikePython(rtl::math::round(nConverted, 1)),
        formatLikePython(rtl::math::round(nConverted, 2)),
        formatLikePython(nConverted)
    };
    std::vector<OUString> aUnique;
    for (const OUString& rCandidate : aCandidates)
    {
        if (std::find(aUnique.begin(), aUnique.end(), rCandidate) == aUnique.end())
            aUnique.push_back(rCandidate);
    }
    std::stable_sort(aUnique.begin(), aUnique.end(),
                     [](const OUString& rLeft, const OUString& rRight) {
                         return rLeft.getLength() < rRight.getLength();
                     });

    OUStringBuffer aResult;
    for (size_t i = 0; i < aUnique.size(); ++i)
    {
        if (i)
            aResult.append(rSuffix + "\n");
        aResult.append(aUnique[i]);
    }
    return aResult.makeStringAndClear()
               .replaceAll(u"."_ustr, rDecimal)
               .replaceAll(u"-"_ustr, u"\u2212"_ustr)
           + rSuffix;
}

icu::RegexMatcher* Lightproof::getConstantMatcher(sal_uInt32 nConstantIndex)
{
    if (m_pRunningPackage == nullptr)
        return nullptr;
    Package& rPackage = *m_pRunningPackage;
    const RuleFile& rFile = *rPackage.pFile;
    if (nConstantIndex >= rFile.getConstantCount())
        return nullptr;

    if (rPackage.aConstantMatchers.size() != rFile.getConstantCount())
        rPackage.aConstantMatchers.resize(rFile.getConstantCount());

    std::unique_ptr<icu::RegexMatcher>& rMatcher = rPackage.aConstantMatchers[nConstantIndex];
    if (!rMatcher)
    {
        const Constant& rConstant = rFile.getConstant(nConstantIndex);
        if (rConstant.nType != Constant::Regex)
            return nullptr;
        UErrorCode nStatus = U_ZERO_ERROR;
        rMatcher.reset(new icu::RegexMatcher(toIcu(rFile.getString(rConstant.nData)),
                                             rConstant.nFlags, nStatus));
        if (U_FAILURE(nStatus))
            rMatcher.reset();
    }
    return rMatcher.get();
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
