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

#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <algorithm>
#include <string_view>
#include <sal/log.hxx>
#include <svtools/languagetoolcfg.hxx>
#include <tools/color.hxx>
#include <tools/long.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

#define COL_ORANGE Color(0xD1, 0x68, 0x20)

namespace
{
Sequence<PropertyValue> lcl_GetLineColorPropertyFromErrorId(const std::string& rErrorId)
{
    Color aColor;
    if (rErrorId == "TYPOS")
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
        aColor = COL_ORANGE;
    }
    Sequence<PropertyValue> aProperties{ comphelper::makePropertyValue("LineColor", aColor) };
    return aProperties;
}
}

LanguageToolGrammarChecker::LanguageToolGrammarChecker()
    : mCachedResults(MAX_CACHE_SIZE)
{
}

LanguageToolGrammarChecker::~LanguageToolGrammarChecker() {}

sal_Bool SAL_CALL LanguageToolGrammarChecker::isSpellChecker() { return false; }

sal_Bool SAL_CALL LanguageToolGrammarChecker::hasLocale(const Locale& rLocale)
{
    bool bRes = false;
    if (!m_aSuppLocales.hasElements())
        getLocales();

    for (auto const& suppLocale : std::as_const(m_aSuppLocales))
    {
        if (rLocale == suppLocale)
        {
            bRes = true;
            break;
        }
    }

    return bRes;
}

Sequence<Locale> SAL_CALL LanguageToolGrammarChecker::getLocales()
{
    if (m_aSuppLocales.hasElements())
        return m_aSuppLocales;
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    OString localeUrl = OUStringToOString(rLanguageOpts.getLocaleListURL(), RTL_TEXTENCODING_UTF8);
    if (localeUrl.isEmpty())
    {
        return m_aSuppLocales;
    }
    tools::Long statusCode = 0;
    std::string response = makeHttpRequest(localeUrl, HTTP_METHOD::HTTP_GET, OString(), statusCode);
    if (statusCode != 200)
    {
        return m_aSuppLocales;
    }
    if (response.empty())
    {
        return m_aSuppLocales;
    }
    boost::property_tree::ptree root;
    std::stringstream aStream(response);
    boost::property_tree::read_json(aStream, root);

    size_t length = root.size();
    m_aSuppLocales.realloc(length);
    auto pArray = m_aSuppLocales.getArray();
    int i = 0;
    for (auto it = root.begin(); it != root.end(); it++, i++)
    {
        boost::property_tree::ptree& localeItem = it->second;
        const std::string longCode = localeItem.get<std::string>("longCode");
        Locale aLocale = LanguageTag::convertToLocale(
            OUString(longCode.c_str(), longCode.length(), RTL_TEXTENCODING_UTF8));
        pArray[i] = aLocale;
    }
    return m_aSuppLocales;
}

// Callback to get the response data from server.
static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userp)
{
    if (!userp)
        return 0;

    std::string* response = static_cast<std::string*>(userp);
    size_t real_size = size * nmemb;
    response->append(static_cast<char*>(ptr), real_size);
    return real_size;
}

ProofreadingResult SAL_CALL LanguageToolGrammarChecker::doProofreading(
    const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale,
    sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition,
    const Sequence<PropertyValue>& aProperties)
{
    // ProofreadingResult declared here instead of parseHttpJSONResponse because of the early exists.
    ProofreadingResult xRes;
    xRes.aDocumentIdentifier = aDocumentIdentifier;
    xRes.aText = aText;
    xRes.aLocale = aLocale;
    xRes.nStartOfSentencePosition = nStartOfSentencePosition;
    xRes.nBehindEndOfSentencePosition = nSuggestedBehindEndOfSentencePosition;
    xRes.aProperties = Sequence<PropertyValue>();
    xRes.xProofreader = this;
    xRes.aErrors = Sequence<SingleProofreadingError>();

    if (aText.isEmpty())
    {
        return xRes;
    }

    if (nStartOfSentencePosition != 0)
    {
        return xRes;
    }

    xRes.nStartOfNextSentencePosition = aText.getLength();

    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    if (rLanguageOpts.getEnabled() == false)
    {
        return xRes;
    }

    OString checkerURL = OUStringToOString(rLanguageOpts.getCheckerURL(), RTL_TEXTENCODING_UTF8);
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

    auto cachedResult = mCachedResults.find(aText);
    if (cachedResult != mCachedResults.end())
    {
        xRes.aErrors = cachedResult->second;
        return xRes;
    }

    tools::Long http_code = 0;
    OUString langTag(aLocale.Language + "-" + aLocale.Country);
    OString postData(OUStringToOString(
        OUStringConcatenation("text=" + aText + "&language=" + langTag), RTL_TEXTENCODING_UTF8));
    const std::string response_body
        = makeHttpRequest(checkerURL, HTTP_METHOD::HTTP_POST, postData, http_code);

    if (http_code != 200)
    {
        return xRes;
    }

    if (response_body.length() <= 0)
    {
        return xRes;
    }

    parseProofreadingJSONResponse(xRes, response_body);
    // cache the result
    mCachedResults.insert(
        std::pair<OUString, Sequence<SingleProofreadingError>>(aText, xRes.aErrors));
    return xRes;
}

/*
    rResult is both input and output
    aJSONBody is the response body from the HTTP Request to LanguageTool API
*/
void LanguageToolGrammarChecker::parseProofreadingJSONResponse(ProofreadingResult& rResult,
                                                               std::string_view aJSONBody)
{
    boost::property_tree::ptree root;
    std::stringstream aStream(aJSONBody.data());
    boost::property_tree::read_json(aStream, root);
    boost::property_tree::ptree& matches = root.get_child("matches");
    size_t matchSize = matches.size();

    if (matchSize <= 0)
    {
        return;
    }
    Sequence<SingleProofreadingError> aErrors(matchSize);
    auto pErrors = aErrors.getArray();
    size_t i = 0;
    for (auto it1 = matches.begin(); it1 != matches.end(); it1++, i++)
    {
        const boost::property_tree::ptree& match = it1->second;
        int offset = match.get<int>("offset");
        int length = match.get<int>("length");
        const std::string shortMessage = match.get<std::string>("message");
        const std::string message = match.get<std::string>("shortMessage");

        // Parse the error category for Line Color
        const boost::property_tree::ptree& rule = match.get_child("rule");
        const boost::property_tree::ptree& ruleCategory = rule.get_child("category");
        const std::string errorCategoryId = ruleCategory.get<std::string>("id");

        OUString aShortComment(shortMessage.c_str(), shortMessage.length(), RTL_TEXTENCODING_UTF8);
        OUString aFullComment(message.c_str(), message.length(), RTL_TEXTENCODING_UTF8);

        pErrors[i].nErrorStart = offset;
        pErrors[i].nErrorLength = length;
        pErrors[i].nErrorType = PROOFREADING_ERROR;
        pErrors[i].aShortComment = aShortComment;
        pErrors[i].aFullComment = aFullComment;
        pErrors[i].aProperties = lcl_GetLineColorPropertyFromErrorId(errorCategoryId);
        ;
        const boost::property_tree::ptree& replacements = match.get_child("replacements");
        int suggestionSize = replacements.size();

        if (suggestionSize <= 0)
        {
            continue;
        }
        pErrors[i].aSuggestions.realloc(std::min(suggestionSize, MAX_SUGGESTIONS_SIZE));
        auto pSuggestions = pErrors[i].aSuggestions.getArray();
        // Limit suggestions to avoid crash on context menu popup:
        // (soffice:17251): Gdk-CRITICAL **: 17:00:21.277: ../../../../../gdk/wayland/gdkdisplay-wayland.c:1399: Unable to create Cairo image
        // surface: invalid value (typically too big) for the size of the input (surface, pattern, etc.)
        int j = 0;
        for (auto it2 = replacements.begin(); it2 != replacements.end() && j < MAX_SUGGESTIONS_SIZE;
             it2++, j++)
        {
            const boost::property_tree::ptree& replacement = it2->second;
            std::string replacementStr = replacement.get<std::string>("value");
            pSuggestions[j]
                = OUString(replacementStr.c_str(), replacementStr.length(), RTL_TEXTENCODING_UTF8);
        }
    }
    rResult.aErrors = aErrors;
}

std::string LanguageToolGrammarChecker::makeHttpRequest(std::string_view aURL, HTTP_METHOD method,
                                                        const OString& aPostData,
                                                        tools::Long& nStatusCode)
{
    std::unique_ptr<CURL, std::function<void(CURL*)>> curl(curl_easy_init(),
                                                           [](CURL* p) { curl_easy_cleanup(p); });
    if (!curl)
        return {}; // empty string

    bool isPremium = false;
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    OString apiKey = OUStringToOString(rLanguageOpts.getApiKey(), RTL_TEXTENCODING_UTF8);
    OString username = OUStringToOString(rLanguageOpts.getUsername(), RTL_TEXTENCODING_UTF8);
    OString premiumPostData;
    if (!apiKey.isEmpty() && !username.isEmpty())
    {
        isPremium = true;
    }

    std::string response_body;
    (void)curl_easy_setopt(curl.get(), CURLOPT_URL, aURL.data());

    (void)curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
    // (void)curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);

    (void)curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    (void)curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, static_cast<void*>(&response_body));
    (void)curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, false);
    (void)curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, false);
    (void)curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, CURL_TIMEOUT);

    if (method == HTTP_METHOD::HTTP_POST)
    {
        (void)curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        if (isPremium == false)
        {
            (void)curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, aPostData.getStr());
        }
        else
        {
            premiumPostData = aPostData + "&username=" + username + "&apiKey=" + apiKey;
            (void)curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, premiumPostData.getStr());
        }
    }

    /*CURLcode cc = */
    (void)curl_easy_perform(curl.get());
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &nStatusCode);
    return response_body;
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
    return "org.openoffice.lingu.LanguageToolGrammarChecker";
}

sal_Bool SAL_CALL LanguageToolGrammarChecker::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL LanguageToolGrammarChecker::getSupportedServiceNames()
{
    return { SN_GRAMMARCHECKER };
}

void SAL_CALL LanguageToolGrammarChecker::initialize(const Sequence<Any>& /*rArguments*/) {}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
lingucomponent_LanguageToolGrammarChecker_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new LanguageToolGrammarChecker()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
