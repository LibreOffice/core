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

#include <sal/config.h>

#include <config_version.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include "languagetoolimp.hxx"

#include <i18nlangtag/languagetag.hxx>
#include <svtools/strings.hrc>
#include <unotools/resmgr.hxx>

#include <vector>
#include <set>
#include <string.h>

#include <officecfg/Office/Linguistic.hxx>

#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <algorithm>
#include <string_view>

#include <systools/curlinit.hxx>

#include <sal/log.hxx>
#include <tools/color.hxx>
#include <tools/long.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <comphelper/propertyvalue.hxx>
#include <unotools/lingucfg.hxx>
#include <osl/mutex.hxx>
#include <rtl/uri.hxx>

using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::linguistic2;

constexpr OUStringLiteral sDuden = u"duden";

namespace
{
constexpr size_t MAX_SUGGESTIONS_SIZE = 10;
using LanguageToolCfg = officecfg::Office::Linguistic::GrammarChecking::LanguageTool;

PropertyValue lcl_GetLineColorPropertyFromErrorId(const std::string& rErrorId)
{
    Color aColor;
    if (rErrorId == "TYPOS" || rErrorId == "orth")
    {
        aColor = COL_LIGHTRED;
    }
    else if (rErrorId == "STYLE")
    {
        aColor = COL_LIGHTBLUE;
    }
    else
    {
        // Same color is used for other errorId's such as GRAMMAR, TYPOGRAPHY..
        constexpr Color COL_ORANGE(0xD1, 0x68, 0x20);
        aColor = COL_ORANGE;
    }
    return comphelper::makePropertyValue(u"LineColor"_ustr, aColor);
}

OString encodeTextForLT(const OUString& text)
{
    // Let's be a bit conservative. I don't find a good description what needs encoding (and in
    // which way) at https://languagetool.org/http-api/; the "Try it out!" function shows that
    // different cases are handled differently by the demo; some percent-encode the UTF-8
    // representation, like %D0%90 (for cyrillic А); some turn into entities like &#33; (for
    // exclamation mark !); some other to things like \u0027 (for apostrophe '). So only keep
    // RFC 3986's "Unreserved Characters" set unencoded, use UTF-8 percent-encoding for the rest.
    static constexpr auto myCharClass = rtl::createUriCharClass(
        u8"-._~0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    return OUStringToOString(
        rtl::Uri::encode(text, myCharClass.data(), rtl_UriEncodeStrict, RTL_TEXTENCODING_UTF8),
        RTL_TEXTENCODING_ASCII_US);
}

// Callback to get the response data from server.
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userp)
{
    if (!userp)
        return 0;

    std::string* response = static_cast<std::string*>(userp);
    size_t real_size = size * nmemb;
    response->append(static_cast<char*>(ptr), real_size);
    return real_size;
}

enum class HTTP_METHOD
{
    HTTP_GET,
    HTTP_POST
};

std::string makeHttpRequest_impl(std::u16string_view aURL, HTTP_METHOD method,
                                 const OString& aPostData, curl_slist* pHttpHeader,
                                 tools::Long& nStatusCode)
{
    struct curl_cleanup_t
    {
        void operator()(CURL* p) const { curl_easy_cleanup(p); }
    };
    std::unique_ptr<CURL, curl_cleanup_t> curl(curl_easy_init());
    if (!curl)
    {
        SAL_WARN("languagetool", "CURL initialization failed");
        return {}; // empty string
    }

    ::InitCurl_easy(curl.get());

    OString aURL8 = OUStringToOString(aURL, RTL_TEXTENCODING_UTF8);
    (void)curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, pHttpHeader);
    (void)curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
    (void)curl_easy_setopt(curl.get(), CURLOPT_URL, aURL8.getStr());
    (void)curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 10L);
    // (void)curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);

    std::string response_body;
    (void)curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    (void)curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_body);

    // allow unknown or self-signed certificates
    if (!LanguageToolCfg::SSLCertVerify::get())
    {
        (void)curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, false);
        (void)curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, false);
    }

    if (method == HTTP_METHOD::HTTP_POST)
    {
        (void)curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        (void)curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, aPostData.getStr());
    }

    CURLcode cc = curl_easy_perform(curl.get());
    if (cc != CURLE_OK)
    {
        SAL_WARN("languagetool",
                 "CURL request returned with error: " << static_cast<sal_Int32>(cc));
    }

    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &nStatusCode);
    return response_body;
}

std::string makeDudenHttpRequest(std::u16string_view aURL, const OString& aPostData,
                                 tools::Long& nStatusCode)
{
    struct curl_slist* pList = nullptr;
    OString sAccessToken
        = OUStringToOString(LanguageToolCfg::ApiKey::get().value_or(""), RTL_TEXTENCODING_UTF8);

    pList = curl_slist_append(pList, "Cache-Control: no-cache");
    pList = curl_slist_append(pList, "Content-Type: application/json");
    if (!sAccessToken.isEmpty())
    {
        sAccessToken = "access_token: " + sAccessToken;
        pList = curl_slist_append(pList, sAccessToken.getStr());
    }

    return makeHttpRequest_impl(aURL, HTTP_METHOD::HTTP_POST, aPostData, pList, nStatusCode);
}

std::string makeHttpRequest(std::u16string_view aURL, HTTP_METHOD method, const OString& aPostData,
                            tools::Long& nStatusCode)
{
    OString realPostData(aPostData);
    if (method == HTTP_METHOD::HTTP_POST)
    {
        OString apiKey
            = OUStringToOString(LanguageToolCfg::ApiKey::get().value_or(""), RTL_TEXTENCODING_UTF8);
        OUString username = LanguageToolCfg::Username::get().value_or("");
        if (!apiKey.isEmpty() && !username.isEmpty())
            realPostData += "&username=" + encodeTextForLT(username) + "&apiKey=" + apiKey;
    }

    return makeHttpRequest_impl(aURL, method, realPostData, nullptr, nStatusCode);
}

template <typename Func>
uno::Sequence<SingleProofreadingError> parseJson(std::string&& json, std::string path, Func f)
{
    std::stringstream aStream(std::move(json)); // Optimized in C++20
    boost::property_tree::ptree aRoot;
    boost::property_tree::read_json(aStream, aRoot);

    if (auto tree = aRoot.get_child_optional(path))
    {
        uno::Sequence<SingleProofreadingError> aErrors(tree->size());
        auto it = tree->begin();
        for (auto& rError : asNonConstRange(aErrors))
        {
            f(it->second, rError);
            it++;
        }
        return aErrors;
    }
    return {};
}

void parseDudenResponse(ProofreadingResult& rResult, std::string&& aJSONBody)
{
    rResult.aErrors = parseJson(
        std::move(aJSONBody), "check-positions",
        [](const boost::property_tree::ptree& rPos, SingleProofreadingError& rError) {
            rError.nErrorStart = rPos.get<int>("offset", 0);
            rError.nErrorLength = rPos.get<int>("length", 0);
            rError.nErrorType = text::TextMarkupType::PROOFREADING;
            //rError.aShortComment = ??
            //rError.aFullComment = ??
            const std::string sType = rPos.get<std::string>("type", {});
            rError.aProperties = { lcl_GetLineColorPropertyFromErrorId(sType) };

            const auto proposals = rPos.get_child_optional("proposals");
            if (!proposals)
                return;
            rError.aSuggestions.realloc(std::min(proposals->size(), MAX_SUGGESTIONS_SIZE));
            auto itProp = proposals->begin();
            for (auto& rSuggestion : asNonConstRange(rError.aSuggestions))
            {
                rSuggestion = OStringToOUString(itProp->second.data(), RTL_TEXTENCODING_UTF8);
                itProp++;
            }
        });
}

/*
    rResult is both input and output
    aJSONBody is the response body from the HTTP Request to LanguageTool API
*/
void parseProofreadingJSONResponse(ProofreadingResult& rResult, std::string&& aJSONBody)
{
    rResult.aErrors = parseJson(
        std::move(aJSONBody), "matches",
        [](const boost::property_tree::ptree& match, SingleProofreadingError& rError) {
            rError.nErrorStart = match.get<int>("offset", 0);
            rError.nErrorLength = match.get<int>("length", 0);
            rError.nErrorType = text::TextMarkupType::PROOFREADING;
            const std::string shortMessage = match.get<std::string>("message", {});
            const std::string message = match.get<std::string>("shortMessage", {});

            rError.aShortComment = OStringToOUString(shortMessage, RTL_TEXTENCODING_UTF8);
            rError.aFullComment = OStringToOUString(message, RTL_TEXTENCODING_UTF8);

            // Parse the error category for Line Color
            std::string errorCategoryId;
            if (auto rule = match.get_child_optional("rule"))
                if (auto ruleCategory = rule->get_child_optional("category"))
                    errorCategoryId = ruleCategory->get<std::string>("id", {});
            rError.aProperties = { lcl_GetLineColorPropertyFromErrorId(errorCategoryId) };

            const auto replacements = match.get_child_optional("replacements");
            if (!replacements)
                return;
            // Limit suggestions to avoid crash on context menu popup:
            // (soffice:17251): Gdk-CRITICAL **: 17:00:21.277: ../../../../../gdk/wayland/gdkdisplay-wayland.c:1399: Unable to create Cairo image
            // surface: invalid value (typically too big) for the size of the input (surface, pattern, etc.)
            rError.aSuggestions.realloc(std::min(replacements->size(), MAX_SUGGESTIONS_SIZE));
            auto itRep = replacements->begin();
            for (auto& rSuggestion : asNonConstRange(rError.aSuggestions))
            {
                std::string replacementStr = itRep->second.get<std::string>("value", {});
                rSuggestion = OStringToOUString(replacementStr, RTL_TEXTENCODING_UTF8);
                itRep++;
            }
        });
}

OUString getCheckerURL()
{
    if (auto oURL = LanguageToolCfg::BaseURL::get())
        if (!oURL->isEmpty())
            return *oURL + "/check";
    return {};
}
}

LanguageToolGrammarChecker::LanguageToolGrammarChecker()
    : mCachedResults(10)
{
}

LanguageToolGrammarChecker::~LanguageToolGrammarChecker() {}

sal_Bool SAL_CALL LanguageToolGrammarChecker::isSpellChecker() { return false; }

sal_Bool SAL_CALL LanguageToolGrammarChecker::hasLocale(const Locale& rLocale)
{
    if (!m_aSuppLocales.hasElements())
        getLocales();

    for (auto const& suppLocale : m_aSuppLocales)
        if (rLocale == suppLocale)
            return true;

    SAL_INFO("languagetool", "No locale \"" << LanguageTag::convertToBcp47(rLocale, false) << "\"");
    return false;
}

uno::Sequence<Locale> SAL_CALL LanguageToolGrammarChecker::getLocales()
{
    osl::MutexGuard aGuard(linguistic::GetLinguMutex());

    if (m_aSuppLocales.hasElements())
        return m_aSuppLocales;

    if (!LanguageToolCfg::IsEnabled::get())
        return m_aSuppLocales;

    SvtLinguConfig aLinguCfg;
    uno::Sequence<OUString> aLocaleList;

    if (LanguageToolCfg::RestProtocol::get().value_or("") == sDuden)
    {
        aLocaleList.realloc(3);
        aLocaleList.getArray()[0] = "de-DE";
        aLocaleList.getArray()[1] = "en-US";
        aLocaleList.getArray()[2] = "en-GB";
    }
    else
        aLinguCfg.GetLocaleListFor(u"GrammarCheckers"_ustr,
                                   u"org.openoffice.lingu.LanguageToolGrammarChecker"_ustr,
                                   aLocaleList);

    auto nLength = aLocaleList.getLength();
    m_aSuppLocales.realloc(nLength);
    auto pArray = m_aSuppLocales.getArray();
    auto pLocaleList = aLocaleList.getArray();

    for (auto i = 0; i < nLength; i++)
    {
        pArray[i] = LanguageTag::convertToLocale(pLocaleList[i]);
    }

    return m_aSuppLocales;
}

ProofreadingResult SAL_CALL LanguageToolGrammarChecker::doProofreading(
    const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale,
    sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition,
    const uno::Sequence<PropertyValue>& aProperties)
{
    // ProofreadingResult declared here instead of parseHttpJSONResponse because of the early exists.
    ProofreadingResult xRes;
    xRes.aDocumentIdentifier = aDocumentIdentifier;
    xRes.aText = aText;
    xRes.aLocale = aLocale;
    xRes.nStartOfSentencePosition = nStartOfSentencePosition;
    xRes.nBehindEndOfSentencePosition = nSuggestedBehindEndOfSentencePosition;
    xRes.aProperties = {};
    xRes.xProofreader = this;
    xRes.aErrors = {};

    if (aText.isEmpty())
    {
        return xRes;
    }

    if (nStartOfSentencePosition != 0)
    {
        return xRes;
    }

    xRes.nStartOfNextSentencePosition = aText.getLength();

    if (!LanguageToolCfg::IsEnabled::get())
    {
        return xRes;
    }

    OUString checkerURL = getCheckerURL();
    if (checkerURL.isEmpty())
    {
        return xRes;
    }

    if (aProperties.getLength() > 0 && aProperties[0].Name == "Update")
    {
        // locale changed
        xRes.aText = "";
        return xRes;
    }

    sal_Int32 spaceIndex = std::min(xRes.nStartOfNextSentencePosition, aText.getLength() - 1);
    while (spaceIndex < aText.getLength() && aText[spaceIndex] == ' ')
    {
        xRes.nStartOfNextSentencePosition += 1;
        spaceIndex = xRes.nStartOfNextSentencePosition;
    }
    if (xRes.nStartOfNextSentencePosition == nSuggestedBehindEndOfSentencePosition
        && spaceIndex < aText.getLength())
    {
        xRes.nStartOfNextSentencePosition
            = std::min(nSuggestedBehindEndOfSentencePosition + 1, aText.getLength());
    }
    xRes.nBehindEndOfSentencePosition
        = std::min(xRes.nStartOfNextSentencePosition, aText.getLength());

    OString langTag(LanguageTag::convertToBcp47(aLocale, false).toUtf8());
    OString postData;
    const bool bDudenProtocol = LanguageToolCfg::RestProtocol::get().value_or("") == "duden";
    if (bDudenProtocol)
    {
        std::stringstream aStream;
        boost::property_tree::ptree aTree;
        aTree.put("text-language", langTag.getStr());
        aTree.put("text", aText.toUtf8()); // We don't encode the text in Duden Corrector tool case.
        aTree.put("hyphenation", false);
        aTree.put("spellchecking-level", 3);
        aTree.put("correction-proposals", true);
        boost::property_tree::write_json(aStream, aTree);
        postData = OString(aStream.str());
    }
    else
    {
        postData = "text=" + encodeTextForLT(aText) + "&language=" + langTag;
    }

    if (auto cachedResult = mCachedResults.find(postData); cachedResult != mCachedResults.end())
    {
        xRes.aErrors = cachedResult->second;
        return xRes;
    }

    tools::Long http_code = 0;
    std::string response_body;
    if (bDudenProtocol)
        response_body = makeDudenHttpRequest(checkerURL, postData, http_code);
    else
        response_body = makeHttpRequest(checkerURL, HTTP_METHOD::HTTP_POST, postData, http_code);

    if (http_code != 200)
    {
        return xRes;
    }

    if (response_body.length() <= 0)
    {
        return xRes;
    }

    if (bDudenProtocol)
    {
        parseDudenResponse(xRes, std::move(response_body));
    }
    else
    {
        parseProofreadingJSONResponse(xRes, std::move(response_body));
    }
    // cache the result
    mCachedResults.insert(std::make_pair(postData, xRes.aErrors));
    return xRes;
}

void SAL_CALL LanguageToolGrammarChecker::ignoreRule(const OUString& /*aRuleIdentifier*/,
                                                     const Locale& /*aLocale*/
)
{
}
void SAL_CALL LanguageToolGrammarChecker::resetIgnoreRules() {}

OUString SAL_CALL LanguageToolGrammarChecker::getServiceDisplayName(const Locale& rLocale)
{
    std::locale loc(Translate::Create("svt", LanguageTag(rLocale)));
    return Translate::get(STR_DESCRIPTION_LANGUAGETOOL, loc);
}

OUString SAL_CALL LanguageToolGrammarChecker::getImplementationName()
{
    return u"org.openoffice.lingu.LanguageToolGrammarChecker"_ustr;
}

sal_Bool SAL_CALL LanguageToolGrammarChecker::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL LanguageToolGrammarChecker::getSupportedServiceNames()
{
    return { SN_GRAMMARCHECKER };
}

void SAL_CALL LanguageToolGrammarChecker::initialize(const uno::Sequence<uno::Any>&) {}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
lingucomponent_LanguageToolGrammarChecker_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new LanguageToolGrammarChecker());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
