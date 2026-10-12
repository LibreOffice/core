/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// What the remote sentence checkers share: the HTTP round trip, the JSON
// parsing, the result cache, and the XProofreader scaffolding around them.
// A concrete checker says how to build its request and how to read the
// answer; everything else is here.

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/linguistic2/SingleProofreadingError.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <o3tl/lru_map.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>

#include <curl/curl.h>
#include <boost/property_tree/ptree_fwd.hpp>

#include <chrono>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace lingucomponent::remote
{
struct Request
{
    OUString aURL;
    // The body, already encoded the way aContentType says.
    OString aBody;
    OString aContentType;
    std::vector<OString> aHeaders;
    // "user:password" for HTTP Basic authentication, empty for none.
    OString aUserPassword;
    bool bVerifySSL = true;
};

struct Response
{
    CURLcode eCode = CURLE_OK;
    // The HTTP status, in the type curl writes it in.
    long nStatus = 0;
    std::string aBody;
};

Response post(const Request& rRequest);
Response get(const Request& rRequest);

// Keeps RFC 3986's unreserved characters and percent-encodes the UTF-8 bytes
// of everything else.
OString percentEncode(std::u16string_view rText);

std::optional<boost::property_tree::ptree> parseJson(const std::string& rJson);

// The underline colours the remote checkers share: red for a misspelling,
// blue for a matter of style, orange for everything else.
constexpr Color COL_REMOTE_SPELLING = COL_LIGHTRED;
constexpr Color COL_REMOTE_STYLE = COL_LIGHTBLUE;
constexpr Color COL_REMOTE_GRAMMAR(0xD1, 0x68, 0x20);

void setLineColor(css::linguistic2::SingleProofreadingError& rError, Color aColor);
void setFullCommentURL(css::linguistic2::SingleProofreadingError& rError, const OUString& rURL);

// Takes the suggestions out of a JSON array. With a key, each element is an
// object and the suggestion is its member of that name; without one, each
// element is the suggestion itself. At most ten are kept: a context menu
// with more has broken the display on some platforms.
void takeSuggestions(css::linguistic2::SingleProofreadingError& rError,
                     const boost::property_tree::ptree& rArray, const char* pValueKey);

// The last few answers, by request body. Reached from the grammar checking
// thread and from the main thread, so every access takes the linguistic
// mutex.
class ResultCache
{
public:
    explicit ResultCache(size_t nSize);
    std::optional<std::vector<css::linguistic2::SingleProofreadingError>>
    find(const OString& rKey);
    void insert(const OString& rKey,
                const std::vector<css::linguistic2::SingleProofreadingError>& rErrors);
    void clear();

private:
    o3tl::lru_map<OString, std::vector<css::linguistic2::SingleProofreadingError>> m_aMap;
};

class ConfigurationListener;

// The part of a remote checker that does not depend on the service it
// talks to. XProofreader already derives from XSupportedLocales. The kit
// asks a checker about its server through XPropertyAccess; see probe().
class CheckerBase
    : public cppu::WeakImplHelper<css::linguistic2::XProofreader,
                                  css::linguistic2::XLinguServiceEventBroadcaster,
                                  css::lang::XServiceDisplayName, css::lang::XServiceInfo,
                                  css::beans::XPropertyAccess>
{
public:
    // XSupportedLocales
    cpo::uno::Sequence<css::lang::Locale> getLocales() override;
    bool hasLocale(const css::lang::Locale& rLocale) override;

    // XProofreader
    bool isSpellChecker() override;
    css::linguistic2::ProofreadingResult
    doProofreading(const OUString& aDocumentIdentifier, const OUString& aText,
                   const css::lang::Locale& aLocale, sal_Int32 nStartOfSentencePosition,
                   sal_Int32 nSuggestedBehindEndOfSentencePosition,
                   const cpo::uno::Sequence<css::beans::PropertyValue>& aProperties) override;
    void ignoreRule(const OUString& aRuleIdentifier, const css::lang::Locale& aLocale) override;
    void resetIgnoreRules() override;

    // XLinguServiceEventBroadcaster
    bool addLinguServiceEventListener(
        const cpo::uno::Reference<css::linguistic2::XLinguServiceEventListener>& xListener)
        override;
    bool removeLinguServiceEventListener(
        const cpo::uno::Reference<css::linguistic2::XLinguServiceEventListener>& xListener)
        override;

    // XServiceInfo
    bool supportsService(const OUString& rServiceName) override;
    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    // XPropertyAccess: what the server says about itself, see probe().
    cpo::uno::Sequence<css::beans::PropertyValue> getPropertyValues() override;
    void setPropertyValues(const cpo::uno::Sequence<css::beans::PropertyValue>& aProps) override;

protected:
    // The configuration node the checker reads its settings from. A change
    // under it empties the caches and asks the documents to check again.
    explicit CheckerBase(OUString aConfigNodePath);
    ~CheckerBase() override;

    virtual bool isEnabled() = 0;
    // BCP 47 tags of the locales the checker is configured to serve.
    virtual std::vector<OUString> configuredLocales() = 0;
    // The request for one paragraph, or nothing when the configuration is
    // not complete enough to send one.
    virtual std::optional<Request> buildRequest(const OUString& rText,
                                                const css::lang::Locale& rLocale)
        = 0;
    virtual std::vector<css::linguistic2::SingleProofreadingError>
    parseResponse(const std::string& rBody, const OUString& rText,
                  const css::lang::Locale& rLocale)
        = 0;
    // Asks the server whether it is there, and what it offers. Returns at
    // least IsReachable; a checker may add lists the settings dialog can
    // choose from. A request may be sent, so it is not called casually.
    virtual cpo::uno::Sequence<css::beans::PropertyValue> probe() = 0;
    // Called with the linguistic mutex held when the configuration changed,
    // before the documents are asked to check again.
    virtual void configurationChanged() {}

    // Sends a request and keeps track of a server that is not answering, so
    // one that is down is not asked again for every paragraph.
    Response send(const Request& rRequest);

private:
    friend class ConfigurationListener;
    void subscribe();
    void onConfigurationChanged();
    void broadcastProofreadAgain();

    const OUString m_aConfigNodePath;
    bool m_bSubscribed = false;
    cpo::uno::Reference<css::container::XNameAccess> m_xConfigNode;
    cpo::uno::Reference<css::util::XChangesListener> m_xConfigListener;
    cpo::uno::Sequence<css::lang::Locale> m_aLocales;
    ResultCache m_aCache;
    std::set<OUString> m_aIgnoredRules;
    std::vector<cpo::uno::Reference<css::linguistic2::XLinguServiceEventListener>>
        m_aListeners;
    // The server the last failure was logged for, so it is logged once.
    OUString m_aLastFailedURL;
    // No request before this moment, after a server failed to answer.
    std::chrono::steady_clock::time_point m_aRetryAfter;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
