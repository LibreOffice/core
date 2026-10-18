/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <linguistic/translate.hxx>
#include <sal/log.hxx>
#include <curl/curl.h>
#include <rtl/string.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <systools/curlinit.hxx>
#include <tools/long.hxx>

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace linguistic
{
namespace
{
/// Percent-encodes rValue for use as a form field value.
OString escape(CURL* pCurl, const OString& rValue)
{
    std::unique_ptr<char, std::function<void(char*)>> pEscaped(
        curl_easy_escape(pCurl, rValue.getStr(), rValue.getLength()),
        [](char* p) { curl_free(p); });
    return pEscaped ? OString(pEscaped.get()) : OString();
}

/// Picks the "message" out of a DeepL error response, if there is one.
OString getErrorMessage(const std::string& rResponseBody)
{
    try
    {
        boost::property_tree::ptree aRoot;
        std::stringstream aStream(rResponseBody);
        boost::property_tree::read_json(aStream, aRoot);
        return OString(aRoot.get<std::string>("message", std::string()));
    }
    catch (const boost::property_tree::ptree_error& rException)
    {
        SAL_WARN("linguistic", "Translate: unexpected error response: " << rException.what());
        return {};
    }
}

/// A curl handle set up for a request to a DeepL endpoint, authenticated with the given
/// key and collecting the response body. A GET request unless POST fields are set on get().
class Request
{
public:
    Request(const OString& rUrl, std::string_view rAuthKey, tools::Long nTimeout)
        : m_aUrl(rUrl)
        , m_pCurl(curl_easy_init(), [](CURL* p) { curl_easy_cleanup(p); })
    {
        if (!m_pCurl)
        {
            SAL_WARN("linguistic", "Request: CURL initialization failed");
            return;
        }

        ::InitCurl_easy(m_pCurl.get());

        (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_URL, m_aUrl.getStr());
        (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_TIMEOUT, nTimeout);

        // DeepL only accepts the key in the Authorization header; the former
        // auth_key form field is rejected with 403 since January 2026.
        const OString aAuthHeader(OString::Concat("Authorization: DeepL-Auth-Key ") + rAuthKey);
        m_pHeaders.reset(curl_slist_append(nullptr, aAuthHeader.getStr()));
        (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPHEADER, m_pHeaders.get());

        (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEFUNCTION,
                               +[](void* buffer, size_t size, size_t nmemb, void* userp) -> size_t {
                                   if (!userp)
                                       return 0;
                                   std::string* response = static_cast<std::string*>(userp);
                                   size_t real_size = size * nmemb;
                                   response->append(static_cast<char*>(buffer), real_size);
                                   return real_size;
                               });
        (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEDATA,
                               static_cast<void*>(&m_aResponseBody));
    }

    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;

    /// The curl handle; null when curl could not be initialized.
    CURL* get() const { return m_pCurl.get(); }

    /// Sends the request. Returns an empty string when the service answered with
    /// HTTP 200, otherwise describes the failure.
    OString perform()
    {
        CURLcode cc = curl_easy_perform(m_pCurl.get());
        if (cc != CURLE_OK)
        {
            SAL_WARN("linguistic", "Request: CURL perform returned with error: "
                                       << static_cast<sal_Int32>(cc) << " "
                                       << curl_easy_strerror(cc));
            return OString(curl_easy_strerror(cc));
        }
        tools::Long nStatusCode = 0;
        curl_easy_getinfo(m_pCurl.get(), CURLINFO_RESPONSE_CODE, &nStatusCode);
        if (nStatusCode != 200)
        {
            // The error body says why, e.g. "Authorization failure, check auth_key".
            OString aError("HTTP " + OString::number(nStatusCode));
            const OString aMessage = getErrorMessage(m_aResponseBody);
            if (!aMessage.isEmpty())
                aError += ": " + aMessage;
            SAL_WARN("linguistic", "Request: request to " << m_aUrl << " failed: " << aError);
            return aError;
        }
        return {};
    }

    const std::string& getResponseBody() const { return m_aResponseBody; }

private:
    const OString m_aUrl;
    std::unique_ptr<CURL, std::function<void(CURL*)>> m_pCurl;
    std::unique_ptr<curl_slist, std::function<void(curl_slist*)>> m_pHeaders{
        nullptr, [](curl_slist* p) { curl_slist_free_all(p); }
    };
    std::string m_aResponseBody;
};
}

TranslateResult Translate(const OString& rTargetLang, const OString& rAPIUrl,
                          std::string_view rAuthKey, const OString& rData)
{
    constexpr tools::Long CURL_TIMEOUT = 10L;

    Request aRequest(rAPIUrl, rAuthKey, CURL_TIMEOUT);
    if (!aRequest.get())
        return { {}, "CURL initialization failed"_ostr };

    // rData is HTML: tag_handling keeps the markup intact across the translation.
    const OString aPostData("target_lang=" + escape(aRequest.get(), rTargetLang)
                            + "&tag_handling=html&text=" + escape(aRequest.get(), rData));
    (void)curl_easy_setopt(aRequest.get(), CURLOPT_POSTFIELDS, aPostData.getStr());

    const OString aError = aRequest.perform();
    if (!aError.isEmpty())
        return { {}, aError };

    // parse the response
    try
    {
        boost::property_tree::ptree root;
        std::stringstream aStream(aRequest.getResponseBody());
        boost::property_tree::read_json(aStream, root);
        const boost::property_tree::ptree& translations = root.get_child("translations");
        if (translations.empty())
        {
            SAL_WARN("linguistic", "Translate: API did not return any translations");
            return { {}, "The service did not return any translations"_ostr };
        }
        // take the first one
        const boost::property_tree::ptree& translation = translations.begin()->second;
        return { OString(translation.get<std::string>("text")), {} };
    }
    catch (const boost::property_tree::ptree_error& rException)
    {
        SAL_WARN("linguistic", "Translate: unexpected response: " << rException.what());
        return { {}, "Unexpected response from the service"_ostr };
    }
}

std::vector<OString> GetTargetLanguages(const OString& rAPIUrl, std::string_view rAuthKey)
{
    constexpr tools::Long CURL_TIMEOUT = 5L;
    // The request blocks the main loop, so an unreachable host should fail fast.
    constexpr tools::Long CURL_CONNECT_TIMEOUT = 2L;
    static constexpr std::string_view aTranslatePath = "/v2/translate";

    std::string_view aBaseUrl(rAPIUrl);
    if (aBaseUrl.ends_with('/'))
        aBaseUrl.remove_suffix(1);
    if (!aBaseUrl.ends_with(aTranslatePath))
    {
        SAL_INFO("linguistic", "GetTargetLanguages: no language list for " << rAPIUrl);
        return {};
    }
    aBaseUrl.remove_suffix(aTranslatePath.size());

    Request aRequest(OString::Concat(aBaseUrl) + "/v3/languages?resource=translate_text", rAuthKey,
                     CURL_TIMEOUT);
    if (!aRequest.get())
        return {};
    (void)curl_easy_setopt(aRequest.get(), CURLOPT_CONNECTTIMEOUT, CURL_CONNECT_TIMEOUT);
    if (!aRequest.perform().isEmpty())
        return {};

    // The response is an array of objects like
    // {"lang": "en-GB", "name": "English (British)", "usable_as_target": true, ...}
    try
    {
        boost::property_tree::ptree aRoot;
        std::stringstream aStream(aRequest.getResponseBody());
        boost::property_tree::read_json(aStream, aRoot);
        std::vector<OString> aLanguages;
        for (const auto& rEntry : aRoot)
        {
            if (!rEntry.second.get<bool>("usable_as_target", false))
                continue;
            const std::string aLang = rEntry.second.get<std::string>("lang", std::string());
            if (!aLang.empty())
                aLanguages.emplace_back(aLang);
        }
        return aLanguages;
    }
    catch (const boost::property_tree::ptree_error& rException)
    {
        SAL_WARN("linguistic", "GetTargetLanguages: unexpected response: " << rException.what());
        return {};
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
