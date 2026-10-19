/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "remotechecker.hxx"

#include <algorithm>
#include <memory>

#include <boost/property_tree/ptree.hpp>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <linguistic/misc.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <unotools/weakref.hxx>

using namespace css::linguistic2;

namespace lingucomponent::remote
{
namespace
{
constexpr size_t MAX_SUGGESTIONS = 10;
constexpr std::chrono::seconds RETRY_DELAY(30);
}

void setLineColor(SingleProofreadingError& rError, Color aColor)
{
    rError.aProperties = comphelper::concatSequences(
        rError.aProperties, cpo::uno::Sequence<css::beans::PropertyValue>{
                                comphelper::makePropertyValue(u"LineColor"_ustr, aColor) });
}

void setFullCommentURL(SingleProofreadingError& rError, const OUString& rURL)
{
    rError.aProperties = comphelper::concatSequences(
        rError.aProperties, cpo::uno::Sequence<css::beans::PropertyValue>{
                                comphelper::makePropertyValue(u"FullCommentURL"_ustr, rURL) });
}

void takeSuggestions(SingleProofreadingError& rError, const boost::property_tree::ptree& rArray,
                     const char* pValueKey)
{
    std::vector<OUString> aSuggestions;
    for (const std::pair<const std::string, boost::property_tree::ptree>& rItem : rArray)
    {
        if (aSuggestions.size() == MAX_SUGGESTIONS)
            break;
        const std::string aValue
            = pValueKey ? rItem.second.get<std::string>(pValueKey, {}) : rItem.second.data();
        aSuggestions.push_back(OStringToOUString(aValue, RTL_TEXTENCODING_UTF8));
    }
    rError.aSuggestions = comphelper::containerToSequence(aSuggestions);
}

ResultCache::ResultCache(size_t nSize)
    : m_aMap(nSize)
{
}

std::optional<std::vector<SingleProofreadingError>> ResultCache::find(const OString& rKey)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    const o3tl::lru_map<OString, std::vector<SingleProofreadingError>>::const_iterator aFound
        = m_aMap.find(rKey);
    if (aFound == m_aMap.end())
        return std::nullopt;
    return aFound->second;
}

void ResultCache::insert(const OString& rKey, const std::vector<SingleProofreadingError>& rErrors)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    m_aMap.insert(std::make_pair(rKey, rErrors));
}

void ResultCache::clear()
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    m_aMap.clear();
}

// Watches the checker's configuration node. Holds the checker weakly, so a
// notification that arrives while the checker is going away finds nothing
// rather than a dangling pointer.
class ConfigurationListener final : public cppu::WeakImplHelper<css::util::XChangesListener>
{
public:
    explicit ConfigurationListener(CheckerBase* pOwner)
        : m_xOwner(rtl::Reference<CheckerBase>(pOwner))
    {
    }

    void changesOccurred(const css::util::ChangesEvent&) override
    {
        const rtl::Reference<CheckerBase> xOwner = m_xOwner.get();
        if (xOwner)
            xOwner->onConfigurationChanged();
    }

    void disposing(const css::lang::EventObject&) override { m_xOwner.clear(); }

private:
    unotools::WeakReference<CheckerBase> m_xOwner;
};

CheckerBase::CheckerBase(OUString aConfigNodePath)
    : m_aConfigNodePath(std::move(aConfigNodePath))
    , m_aCache(10)
{
}

CheckerBase::~CheckerBase()
{
    if (!m_xConfigListener)
        return;
    try
    {
        cpo::uno::Reference<css::util::XChangesNotifier> xNotifier(m_xConfigNode,
                                                                   cpo::uno::UNO_QUERY);
        if (xNotifier)
            xNotifier->removeChangesListener(m_xConfigListener);
    }
    catch (const cpo::uno::Exception&)
    {
    }
}

// Not from the constructor: the listener takes a weak reference to this
// object, which needs the object to be owned by then.
void CheckerBase::subscribe()
{
    if (m_bSubscribed)
        return;
    m_bSubscribed = true;
    try
    {
        cpo::uno::Reference<css::lang::XMultiServiceFactory> xProvider(
            css::configuration::theDefaultProvider::get(comphelper::getProcessComponentContext()));
        const css::beans::NamedValue aPath(u"nodepath"_ustr, cpo::uno::Any(m_aConfigNodePath));
        m_xConfigNode.set(xProvider->createInstanceWithArguments(
                              u"com.sun.star.configuration.ConfigurationAccess"_ustr,
                              { cpo::uno::Any(aPath) }),
                          cpo::uno::UNO_QUERY);
        cpo::uno::Reference<css::util::XChangesNotifier> xNotifier(m_xConfigNode,
                                                                   cpo::uno::UNO_QUERY);
        if (xNotifier)
        {
            m_xConfigListener.set(new ConfigurationListener(this));
            xNotifier->addChangesListener(m_xConfigListener);
        }
    }
    catch (const cpo::uno::Exception&)
    {
        SAL_WARN("lingucomponent.remote", "cannot watch " << m_aConfigNodePath);
    }
}

// The caches go under the mutex; the documents are told without it, because
// a listener may turn around and ask this checker for its locales from
// another thread while holding a lock of its own.
void CheckerBase::onConfigurationChanged()
{
    {
        osl::MutexGuard aGuard(linguistic::GetLinguMutex());
        m_aLocales = {};
        m_aLastFailedURL.clear();
        m_aRetryAfter = std::chrono::steady_clock::time_point();
        configurationChanged();
    }
    m_aCache.clear();
    broadcastProofreadAgain();
}

void CheckerBase::broadcastProofreadAgain()
{
    const LinguServiceEvent aEvent(static_cast<cppu::OWeakObject*>(this),
                                   LinguServiceEventFlags::PROOFREAD_AGAIN);
    std::vector<cpo::uno::Reference<XLinguServiceEventListener>> aListeners;
    {
        osl::MutexGuard aGuard(linguistic::GetLinguMutex());
        aListeners = m_aListeners;
    }
    for (const cpo::uno::Reference<XLinguServiceEventListener>& rListener : aListeners)
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

// The locales the checker is configured for, whether or not it is switched
// on: this is what keeps it on the service manager's lists while it is off.
cpo::uno::Sequence<css::lang::Locale> CheckerBase::getLocales()
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    subscribe();
    if (m_aLocales.hasElements())
        return m_aLocales;

    std::vector<css::lang::Locale> aLocales;
    for (const OUString& rTag : configuredLocales())
        aLocales.push_back(LanguageTag::convertToLocale(rTag, false));
    m_aLocales = comphelper::containerToSequence(aLocales);
    return m_aLocales;
}

// Whether the checker serves the locale now: the iterator asks this before
// it sends a paragraph, and moves on to the next checker on a no.
bool CheckerBase::hasLocale(const css::lang::Locale& rLocale)
{
    if (!isEnabled())
        return false;
    const cpo::uno::Sequence<css::lang::Locale> aLocales = getLocales();
    return std::find(aLocales.begin(), aLocales.end(), rLocale) != aLocales.end();
}

bool CheckerBase::isSpellChecker() { return false; }

Response CheckerBase::send(const Request& rRequest)
{
    {
        osl::MutexGuard aGuard(linguistic::GetLinguMutex());
        if (std::chrono::steady_clock::now() < m_aRetryAfter)
        {
            Response aSkipped;
            aSkipped.aError = "not asked: the server did not answer a moment ago"_ostr;
            return aSkipped;
        }
    }

    Response aResponse = post(rRequest);
    if (aResponse.bOk)
        return aResponse;

    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    if (m_aLastFailedURL != rRequest.aURL)
    {
        m_aLastFailedURL = rRequest.aURL;
        SAL_WARN("lingucomponent.remote", rRequest.aURL << " failed: " << aResponse.aError
                                                        << ", HTTP status " << aResponse.nStatus);
    }
    // A server that did not answer at all is left alone for a while. One
    // that answered with an error is asked again: a 4xx may be about this
    // one request.
    if (aResponse.nStatus == 0)
        m_aRetryAfter = std::chrono::steady_clock::now() + RETRY_DELAY;
    return aResponse;
}

ProofreadingResult CheckerBase::doProofreading(
    const OUString& aDocumentIdentifier, const OUString& aText, const css::lang::Locale& aLocale,
    sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition,
    const cpo::uno::Sequence<css::beans::PropertyValue>& /*aProperties*/)
{
    ProofreadingResult aResult;
    aResult.aDocumentIdentifier = aDocumentIdentifier;
    aResult.aText = aText;
    aResult.aLocale = aLocale;
    aResult.nStartOfSentencePosition = nStartOfSentencePosition;
    aResult.nBehindEndOfSentencePosition = nSuggestedBehindEndOfSentencePosition;
    aResult.nStartOfNextSentencePosition = nSuggestedBehindEndOfSentencePosition;
    aResult.xProofreader = this;

    // The whole paragraph goes to the server in one request, so the first
    // call answers for all of it and the later ones return nothing.
    if (aText.isEmpty() || nStartOfSentencePosition != 0)
        return aResult;

    aResult.nBehindEndOfSentencePosition = aText.getLength();
    aResult.nStartOfNextSentencePosition = aText.getLength();

    {
        osl::MutexGuard aGuard(linguistic::GetLinguMutex());
        subscribe();
    }
    if (!isEnabled())
        return aResult;

    const std::optional<Request> oRequest = buildRequest(aText, aLocale);
    if (!oRequest)
        return aResult;

    std::vector<SingleProofreadingError> aErrors;
    if (const std::optional<std::vector<SingleProofreadingError>> oCached
        = m_aCache.find(oRequest->aBody))
    {
        aErrors = *oCached;
    }
    else
    {
        const Response aResponse = send(*oRequest);
        if (!aResponse.bOk || aResponse.nStatus != 200 || aResponse.aBody.empty())
            return aResult;
        aErrors = parseResponse(aResponse.aBody, aText, aLocale);
        m_aCache.insert(oRequest->aBody, aErrors);
    }

    {
        osl::MutexGuard aGuard(linguistic::GetLinguMutex());
        if (!m_aIgnoredRules.empty())
        {
            std::erase_if(aErrors, [this](const SingleProofreadingError& rError) {
                return m_aIgnoredRules.count(rError.aRuleIdentifier) != 0;
            });
        }
    }
    aResult.aErrors = comphelper::containerToSequence(aErrors);
    return aResult;
}

void CheckerBase::ignoreRule(const OUString& aRuleIdentifier, const css::lang::Locale&)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    if (!aRuleIdentifier.isEmpty())
        m_aIgnoredRules.insert(aRuleIdentifier);
}

void CheckerBase::resetIgnoreRules()
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    m_aIgnoredRules.clear();
}

bool CheckerBase::addLinguServiceEventListener(
    const cpo::uno::Reference<XLinguServiceEventListener>& xListener)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    if (!xListener)
        return false;
    if (std::find(m_aListeners.begin(), m_aListeners.end(), xListener) != m_aListeners.end())
        return false;
    m_aListeners.push_back(xListener);
    return true;
}

bool CheckerBase::removeLinguServiceEventListener(
    const cpo::uno::Reference<XLinguServiceEventListener>& xListener)
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    const std::vector<cpo::uno::Reference<XLinguServiceEventListener>>::iterator aFound
        = std::find(m_aListeners.begin(), m_aListeners.end(), xListener);
    if (aFound == m_aListeners.end())
        return false;
    m_aListeners.erase(aFound);
    return true;
}

bool CheckerBase::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence<OUString> CheckerBase::getSupportedServiceNames()
{
    return { SN_GRAMMARCHECKER };
}

cpo::uno::Sequence<css::beans::PropertyValue> CheckerBase::getPropertyValues()
{
    if (!isEnabled())
        return { comphelper::makePropertyValue(u"IsReachable"_ustr, false) };
    return probe();
}

void CheckerBase::setPropertyValues(const cpo::uno::Sequence<css::beans::PropertyValue>&)
{
    // The settings are in the configuration, not here.
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
