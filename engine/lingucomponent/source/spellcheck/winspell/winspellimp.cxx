/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cpo/uno/Reference.h>
#include <cpo/uno/XComponentContext.hpp>

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <comphelper/kit.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <linguistic/misc.hxx>
#include <linguistic/spelldta.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <unotools/lingucfg.hxx>

#include <lingutil.hxx>
#include "winspellimp.hxx"

#include <set>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace ::cpo;
using namespace ::cpo::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;
using namespace sal::systools;

// XML-header of SPELLML queries
constexpr OUStringLiteral SPELL_XML = u"<?xml?>";

namespace
{
void LogComError(const char* pWhat, const ComError& rError)
{
    SAL_WARN("lingucomponent", "WinSpell: " << pWhat << ": " << rError.what() << " (hr=0x"
                                            << std::hex << rError.GetHresult() << ")");
}

// Reads every string of a COM string enumerator; each one is owned by the caller and has to
// be given back to the COM allocator.
std::vector<OUString> DrainStrings(IEnumString* pStrings)
{
    std::vector<OUString> aResult;
    if (!pStrings)
        return aResult;
    for (;;)
    {
        CoTaskMemAllocated<wchar_t> pString;
        ULONG nFetched = 0;
        if (pStrings->Next(1, &pString, &nFetched) != S_OK || nFetched != 1 || !pString)
            break;
        aResult.emplace_back(o3tl::toU(pString));
    }
    return aResult;
}
}

WinSpellChecker::WinSpellChecker()
    : m_bLocalesRead(false)
    , m_aEvtListeners(GetLinguMutex())
    , m_bDisposing(false)
{
}

WinSpellChecker::~WinSpellChecker()
{
    if (m_pPropHelper)
        m_pPropHelper->RemoveAsPropListener();
    // The checkers go before the factory, and both before the apartment they live in, which
    // has to be left on the thread that entered it in ReadLocales.
    DBG_TESTSOLARMUTEX();
    m_aCheckers.clear();
    m_xFactory.clear();
    m_oCoInit.reset();
}

PropertyHelper_Spelling& WinSpellChecker::GetPropHelper_Impl()
{
    if (!m_pPropHelper)
    {
        Reference<XLinguProperties> xPropSet = GetLinguProperties();

        m_pPropHelper.reset(
            new PropertyHelper_Spelling(static_cast<XSpellChecker*>(this), xPropSet));
        m_pPropHelper->AddAsPropListener(); //! after a reference is established
    }
    return *m_pPropHelper;
}

// Computes the claimed locales once per process, like the hunspell backend does: a language
// added in Windows Settings, or a dictionary removed, shows up on the next start.
void WinSpellChecker::ReadLocales()
{
    m_bLocalesRead = true;

    // The apartment is tied to this thread, and the destructor leaves it on the same one.
    DBG_TESTSOLARMUTEX();

    std::vector<OUString> aTags;
    try
    {
        m_oCoInit.emplace(COINIT_APARTMENTTHREADED);
        m_xFactory = COMReference<ISpellCheckerFactory>(__uuidof(SpellCheckerFactory), nullptr,
                                                        CLSCTX_INPROC_SERVER);
        COMReference<IEnumString> xTags;
        ThrowIfFailed(m_xFactory->get_SupportedLanguages(&xTags),
                      "ISpellCheckerFactory::get_SupportedLanguages failed");
        aTags = DrainStrings(xTags);
    }
    catch (const ComError& rError)
    {
        LogComError("no spell checker factory, service stays inert", rError);
        m_xFactory.clear();
        return;
    }

    // The locales an installed hunspell dictionary covers, as language ids so that the
    // spelling of the tag does not matter when comparing.
    std::set<LanguageType> aHunspellLanguages;
    for (const OUString& rName : GetSpellDicLocaleNames(GetSpellDics()))
        aHunspellLanguages.insert(LanguageTag(rName).getLanguageType());

    for (const OUString& rTag : aTags)
    {
        LanguageTag aLanguageTag(rTag);
        if (!aLanguageTag.isValidBcp47())
            continue;
        if (!comphelper::COKit::isAllowlistedLanguage(aLanguageTag.getBcp47()))
            continue;
        if (aHunspellLanguages.count(aLanguageTag.getLanguageType()))
            continue;
        m_aLocaleTags.push_back({ aLanguageTag.getLocale(), rTag });
    }

    m_aSuppLocales.realloc(m_aLocaleTags.size());
    Locale* pLocales = m_aSuppLocales.getArray();
    for (const LocaleTag& rLocaleTag : m_aLocaleTags)
    {
        SAL_INFO("lingucomponent", "WinSpell: claiming " << rLocaleTag.m_aTag);
        *pLocales++ = rLocaleTag.m_aLocale;
    }
}

Sequence<Locale> WinSpellChecker::getLocales()
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_bLocalesRead)
        ReadLocales();

    return m_aSuppLocales;
}

const OUString* WinSpellChecker::FindTag(const Locale& rLocale) const
{
    for (const LocaleTag& rLocaleTag : m_aLocaleTags)
    {
        if (rLocaleTag.m_aLocale == rLocale)
            return &rLocaleTag.m_aTag;
    }
    return nullptr;
}

bool WinSpellChecker::hasLocale(const Locale& rLocale)
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_bLocalesRead)
        ReadLocales();

    return FindTag(rLocale) != nullptr;
}

ISpellChecker* WinSpellChecker::GetChecker(const Locale& rLocale)
{
    const OUString* pTag = FindTag(rLocale);
    if (!pTag || !m_xFactory)
        return nullptr;

    std::map<OUString, COMReference<ISpellChecker>>::iterator it = m_aCheckers.find(*pTag);
    if (it != m_aCheckers.end())
        return it->second.get();

    COMReference<ISpellChecker> xChecker;
    try
    {
        ThrowIfFailed(m_xFactory->CreateSpellChecker(o3tl::toW(pTag->getStr()), &xChecker),
                      "ISpellCheckerFactory::CreateSpellChecker failed");
    }
    catch (const ComError& rError)
    {
        LogComError("no checker for this language", rError);
        xChecker.clear();
    }
    return m_aCheckers.emplace(*pTag, xChecker).first->second.get();
}

// Asks Windows about one word. A failing call counts as a correct word, so a dead service
// never paints a document red.
sal_Int16 WinSpellChecker::GetSpellFailure(const OUString& rWord, const Locale& rLocale)
{
    ISpellChecker* pChecker = GetChecker(rLocale);
    if (!pChecker)
        return -1;

    try
    {
        COMReference<IEnumSpellingError> xErrors;
        ThrowIfFailed(pChecker->Check(o3tl::toW(rWord.getStr()), &xErrors),
                      "ISpellChecker::Check failed");
        if (!xErrors)
            return -1;

        COMReference<ISpellingError> xError;
        if (xErrors->Next(&xError) != S_OK || !xError)
            return -1;

        CORRECTIVE_ACTION eAction = CORRECTIVE_ACTION_NONE;
        ThrowIfFailed(xError->get_CorrectiveAction(&eAction),
                      "ISpellingError::get_CorrectiveAction failed");
        if (eAction == CORRECTIVE_ACTION_NONE)
            return -1;

        if (eAction == CORRECTIVE_ACTION_REPLACE)
        {
            // A replacement that differs from the word in case only is a capitalization
            // error, the same way the engine classifies it elsewhere.
            CoTaskMemAllocated<wchar_t> pReplacement;
            if (SUCCEEDED(xError->get_Replacement(&pReplacement)) && pReplacement
                && rWord.equalsIgnoreAsciiCase(OUString(o3tl::toU(pReplacement))))
                return SpellFailure::CAPTION_ERROR;
        }
        return SpellFailure::SPELLING_ERROR;
    }
    catch (const ComError& rError)
    {
        LogComError("checking a word", rError);
        return -1;
    }
}

// The failure of a word after the options that mute errors on upper case words and on words
// with digits have been applied; -1 when the word is accepted.
sal_Int16 WinSpellChecker::CheckWord(const OUString& rWord, const Locale& rLocale,
                                     const Sequence<PropertyValue>& rProperties)
{
    // Get property values to be used.
    // These are be the default values set in the SN_LINGU_PROPERTIES
    // PropertySet which are overridden by the supplied ones from the
    // last argument.
    PropertyHelper_Spelling& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals(rProperties);

    sal_Int16 nFailure = GetSpellFailure(rWord, rLocale);
    if (nFailure != -1)
    {
        LanguageType nLang = LinguLocaleToLanguage(rLocale);
        // postprocess result for errors that should be ignored
        const bool bIgnoreError = (!rHelper.IsSpellUpperCase() && IsUpper(rWord, nLang))
                                  || (!rHelper.IsSpellWithDigits() && HasDigits(rWord));
        if (bIgnoreError)
            nFailure = -1;
    }
    return nFailure;
}

bool WinSpellChecker::isValid(const OUString& rWord, const Locale& rLocale,
                              const Sequence<PropertyValue>& rProperties)
{
    MutexGuard aGuard(GetLinguMutex());

    if (rLocale == Locale() || rWord.isEmpty())
        return true;

    if (!hasLocale(rLocale))
        return true;

    // SPELLML queries are the hunspell backend's protocol for adding words at runtime.
    // Windows has nothing to add to, so the probe for that protocol is answered with no.
    if (rWord.match(SPELL_XML, 0))
        return false;

    return CheckWord(rWord, rLocale, rProperties) == -1;
}

Reference<XSpellAlternatives>
WinSpellChecker::GetProposals(const OUString& rWord, const Locale& rLocale, sal_Int16 nFailure)
{
    // Retrieves the return values for the 'spell' function call in case
    // of a misspelled word.
    // Especially it may give a list of suggested (correct) words:
    std::vector<OUString> aSuggestions;
    if (ISpellChecker* pChecker = GetChecker(rLocale))
    {
        try
        {
            COMReference<IEnumString> xSuggestions;
            ThrowIfFailed(pChecker->Suggest(o3tl::toW(rWord.getStr()), &xSuggestions),
                          "ISpellChecker::Suggest failed");
            aSuggestions = DrainStrings(xSuggestions);
        }
        catch (const ComError& rError)
        {
            LogComError("getting suggestions", rError);
        }
    }

    // an empty alternative for no suggestions or the list of alternatives if some found
    Sequence<OUString> aStr(aSuggestions.size());
    OUString* pStr = aStr.getArray();
    for (const OUString& rSuggestion : aSuggestions)
        *pStr++ = rSuggestion;
    return SpellAlternatives::CreateSpellAlternatives(rWord, LinguLocaleToLanguage(rLocale),
                                                      nFailure, aStr);
}

Reference<XSpellAlternatives> WinSpellChecker::spell(const OUString& rWord, const Locale& rLocale,
                                                     const Sequence<PropertyValue>& rProperties)
{
    MutexGuard aGuard(GetLinguMutex());

    if (rLocale == Locale() || rWord.isEmpty())
        return nullptr;

    if (!hasLocale(rLocale))
        return nullptr;

    if (rWord.match(SPELL_XML, 0))
        return nullptr;

    Reference<XSpellAlternatives> xAlt;
    sal_Int16 nFailure = CheckWord(rWord, rLocale, rProperties);
    if (nFailure != -1)
        xAlt = GetProposals(rWord, rLocale, nFailure);
    return xAlt;
}

bool WinSpellChecker::addLinguServiceEventListener(
    const Reference<XLinguServiceEventListener>& rxLstnr)
{
    MutexGuard aGuard(GetLinguMutex());

    bool bRes = false;
    if (!m_bDisposing && rxLstnr.is())
        bRes = GetPropHelper().addLinguServiceEventListener(rxLstnr);
    return bRes;
}

bool WinSpellChecker::removeLinguServiceEventListener(
    const Reference<XLinguServiceEventListener>& rxLstnr)
{
    MutexGuard aGuard(GetLinguMutex());

    bool bRes = false;
    if (!m_bDisposing && rxLstnr.is())
        bRes = GetPropHelper().removeLinguServiceEventListener(rxLstnr);
    return bRes;
}

OUString WinSpellChecker::getServiceDisplayName(const Locale& /*rLocale*/)
{
    return u"Windows Spell Checker"_ustr;
}

void WinSpellChecker::initialize(const Sequence<Any>& rArguments)
{
    MutexGuard aGuard(GetLinguMutex());

    if (m_pPropHelper)
        return;

    sal_Int32 nLen = rArguments.getLength();
    if (2 == nLen)
    {
        Reference<XLinguProperties> xPropSet;
        rArguments.getConstArray()[0] >>= xPropSet;
        // rArguments.getConstArray()[1] >>= xDicList;

        m_pPropHelper.reset(
            new PropertyHelper_Spelling(static_cast<XSpellChecker*>(this), xPropSet));
        m_pPropHelper->AddAsPropListener(); //! after a reference is established
    }
    else
    {
        OSL_FAIL("wrong number of arguments in sequence");
    }
}

void WinSpellChecker::dispose()
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_bDisposing)
    {
        m_bDisposing = true;
        EventObject aEvtObj(static_cast<XSpellChecker*>(this));
        m_aEvtListeners.disposeAndClear(aEvtObj);
        if (m_pPropHelper)
        {
            m_pPropHelper->RemoveAsPropListener();
            m_pPropHelper.reset();
        }
    }
}

void WinSpellChecker::addEventListener(const Reference<XEventListener>& rxListener)
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_bDisposing && rxListener.is())
        m_aEvtListeners.addInterface(rxListener);
}

void WinSpellChecker::removeEventListener(const Reference<XEventListener>& rxListener)
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_bDisposing && rxListener.is())
        m_aEvtListeners.removeInterface(rxListener);
}

// Service specific part
OUString WinSpellChecker::getImplementationName() { return u"cpo.lingu.WinSpellChecker"_ustr; }

bool WinSpellChecker::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> WinSpellChecker::getSupportedServiceNames() { return { SN_SPELLCHECKER }; }

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
lingucomponent_WinSpellChecker_get_implementation(cpo::uno::XComponentContext*,
                                                  cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new WinSpellChecker());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
