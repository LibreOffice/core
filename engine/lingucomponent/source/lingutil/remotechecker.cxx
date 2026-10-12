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
#include <array>
#include <memory>
#include <sstream>

#include <boost/property_tree/json_parser.hpp>
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
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <systools/curlinit.hxx>
#include <unotools/weakref.hxx>

using namespace css::linguistic2;

namespace lingucomponent::remote
{
namespace
{
constexpr size_t MAX_SUGGESTIONS = 10;
constexpr std::chrono::seconds RETRY_DELAY(30);

size_t writeCallback(void* pData, size_t nSize, size_t nMembers, void* pUser)
{
    if (!pUser)
        return 0;
    std::string* pBody = static_cast<std::string*>(pUser);
    const size_t nBytes = nSize * nMembers;
    pBody->append(static_cast<char*>(pData), nBytes);
    return nBytes;
}

struct CurlCleanup
{
    void operator()(CURL* p) const { curl_easy_cleanup(p); }
};

struct SlistCleanup
{
    void operator()(curl_slist* p) const { curl_slist_free_all(p); }
};

Response perform(const Request& rRequest, bool bPost)
{
    Response aResponse;
    std::unique_ptr<CURL, CurlCleanup> pCurl(curl_easy_init());
    if (!pCurl)
    {
        SAL_WARN("lingucomponent.remote", "curl initialization failed");
        aResponse.eCode = CURLE_FAILED_INIT;
        return aResponse;
    }

    ::InitCurl_easy(pCurl.get());

    std::unique_ptr<curl_slist, SlistCleanup> pHeaders;
    if (!rRequest.aContentType.isEmpty())
    {
        const OString aHeader = "Content-Type: " + rRequest.aContentType;
        pHeaders.reset(curl_slist_append(pHeaders.release(), aHeader.getStr()));
    }
    for (const OString& rHeader : rRequest.aHeaders)
        pHeaders.reset(curl_slist_append(pHeaders.release(), rHeader.getStr()));

    const OString aURL = OUStringToOString(rRequest.aURL, RTL_TEXTENCODING_UTF8);
    (void)curl_easy_setopt(pCurl.get(), CURLOPT_URL, aURL.getStr());
    (void)curl_easy_setopt(pCurl.get(), CURLOPT_HTTPHEADER, pHeaders.get());
    (void)curl_easy_setopt(pCurl.get(), CURLOPT_FAILONERROR, 1L);
    (void)curl_easy_setopt(pCurl.get(), CURLOPT_TIMEOUT, 10L);
    (void)curl_easy_setopt(pCurl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    (void)curl_easy_setopt(pCurl.get(), CURLOPT_WRITEDATA, &aResponse.aBody);

    if (!rRequest.aUserPassword.isEmpty())
    {
        (void)curl_easy_setopt(pCurl.get(), CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        (void)curl_easy_setopt(pCurl.get(), CURLOPT_USERPWD, rRequest.aUserPassword.getStr());
    }

    // A self-signed or otherwise unknown certificate is let through only
    // when the configuration says so.
    if (!rRequest.bVerifySSL)
    {
        (void)curl_easy_setopt(pCurl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
        (void)curl_easy_setopt(pCurl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
    }

    if (bPost)
    {
        (void)curl_easy_setopt(pCurl.get(), CURLOPT_POST, 1L);
        (void)curl_easy_setopt(pCurl.get(), CURLOPT_POSTFIELDSIZE,
                               static_cast<long>(rRequest.aBody.getLength()));
        (void)curl_easy_setopt(pCurl.get(), CURLOPT_POSTFIELDS, rRequest.aBody.getStr());
    }

    aResponse.eCode = curl_easy_perform(pCurl.get());
    curl_easy_getinfo(pCurl.get(), CURLINFO_RESPONSE_CODE, &aResponse.nStatus);
    return aResponse;
}
}

Response post(const Request& rRequest) { return perform(rRequest, true); }

Response get(const Request& rRequest) { return perform(rRequest, false); }

OString percentEncode(std::u16string_view rText)
{
    static constexpr std::array<bool, rtl::UriCharClassSize> aUnreserved = rtl::createUriCharClass(
        u8"-._~0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    return OUStringToOString(rtl::Uri::encode(OUString(rText), aUnreserved.data(),
                                              rtl_UriEncodeStrict, RTL_TEXTENCODING_UTF8),
                             RTL_TEXTENCODING_ASCII_US);
}

std::optional<boost::property_tree::ptree> parseJson(const std::string& rJson)
{
    std::stringstream aStream(rJson);
    boost::property_tree::ptree aRoot;
    try
    {
        boost::property_tree::read_json(aStream, aRoot);
    }
    catch (const std::runtime_error& rError)
    {
        SAL_WARN("lingucomponent.remote", "cannot parse the answer: " << rError.what());
        return std::nullopt;
    }
    return aRoot;
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

cpo::uno::Sequence<css::lang::Locale> CheckerBase::getLocales()
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    subscribe();
    if (m_aLocales.hasElements())
        return m_aLocales;
    if (!isEnabled())
        return m_aLocales;

    std::vector<css::lang::Locale> aLocales;
    for (const OUString& rTag : configuredLocales())
        aLocales.push_back(LanguageTag::convertToLocale(rTag, false));
    m_aLocales = comphelper::containerToSequence(aLocales);
    return m_aLocales;
}

bool CheckerBase::hasLocale(const css::lang::Locale& rLocale)
{
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
            aSkipped.eCode = CURLE_COULDNT_CONNECT;
            return aSkipped;
        }
    }

    Response aResponse = post(rRequest);
    if (aResponse.eCode == CURLE_OK)
        return aResponse;

    osl::MutexGuard aGuard(linguistic::GetLinguMutex());
    if (m_aLastFailedURL != rRequest.aURL)
    {
        m_aLastFailedURL = rRequest.aURL;
        SAL_WARN("lingucomponent.remote",
                 rRequest.aURL << " failed: (" << static_cast<sal_Int32>(aResponse.eCode) << ") "
                               << curl_easy_strerror(aResponse.eCode)
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
        if (aResponse.eCode != CURLE_OK || aResponse.nStatus != 200 || aResponse.aBody.empty())
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
