/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <translate/httprequest.hxx>

#include <sal/log.hxx>
#include <systools/curlinit.hxx>
#include <tools/long.hxx>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <sstream>

namespace linguistic::translate
{
HttpRequest::HttpRequest(const OString& rUrl, std::chrono::seconds aTimeout,
                         std::chrono::seconds aConnectTimeout)
    : m_aUrl(rUrl)
    , m_pCurl(curl_easy_init(), [](CURL* p) { curl_easy_cleanup(p); })
{
    if (!m_pCurl)
    {
        SAL_WARN("linguistic", "HttpRequest: CURL initialization failed");
        return;
    }

    ::InitCurl_easy(m_pCurl.get());

    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_URL, m_aUrl.getStr());
    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_TIMEOUT,
                           static_cast<tools::Long>(aTimeout.count()));
    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_CONNECTTIMEOUT,
                           static_cast<tools::Long>(aConnectTimeout.count()));

    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEFUNCTION,
                           +[](void* buffer, size_t size, size_t nmemb, void* userp) -> size_t {
                               if (!userp)
                                   return 0;
                               std::string* response = static_cast<std::string*>(userp);
                               size_t real_size = size * nmemb;
                               response->append(static_cast<char*>(buffer), real_size);
                               return real_size;
                           });
    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEDATA, static_cast<void*>(&m_aResponseBody));
}

void HttpRequest::addHeader(const OString& rHeader)
{
    if (!m_pCurl)
        return;
    m_pHeaders.reset(curl_slist_append(m_pHeaders.release(), rHeader.getStr()));
    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPHEADER, m_pHeaders.get());
}

void HttpRequest::setPostFields(const OString& rBody)
{
    if (!m_pCurl)
        return;
    m_aBody = rBody;
    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_POSTFIELDSIZE,
                           static_cast<tools::Long>(m_aBody.getLength()));
    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_POSTFIELDS, m_aBody.getStr());
}

void HttpRequest::setJsonBody(const OString& rJson)
{
    addHeader("Content-Type: application/json"_ostr);
    setPostFields(rJson);
}

void HttpRequest::setVerifySSL(bool bVerify)
{
    if (!m_pCurl || bVerify)
        return;
    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
    (void)curl_easy_setopt(m_pCurl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
}

OString HttpRequest::escape(const OString& rValue) const
{
    if (!m_pCurl)
        return {};
    std::unique_ptr<char, std::function<void(char*)>> pEscaped(
        curl_easy_escape(m_pCurl.get(), rValue.getStr(), rValue.getLength()),
        [](char* p) { curl_free(p); });
    return pEscaped ? OString(pEscaped.get()) : OString();
}

OString HttpRequest::perform()
{
    if (!m_pCurl)
        return "CURL initialization failed"_ostr;

    CURLcode cc = curl_easy_perform(m_pCurl.get());
    if (cc != CURLE_OK)
    {
        SAL_WARN("linguistic", "HttpRequest: CURL perform returned with error: "
                                   << static_cast<sal_Int32>(cc) << " " << curl_easy_strerror(cc));
        return OString(curl_easy_strerror(cc));
    }
    tools::Long nStatusCode = 0;
    curl_easy_getinfo(m_pCurl.get(), CURLINFO_RESPONSE_CODE, &nStatusCode);
    if (nStatusCode != 200)
    {
        // The error body says why, e.g. "Authorization failure, check auth_key".
        OString aError("HTTP " + OString::number(nStatusCode));
        if (m_pErrorMessageKey)
        {
            if (std::optional<boost::property_tree::ptree> oRoot
                = parseJson(m_aResponseBody, "error response"))
            {
                const std::string aMessage
                    = oRoot->get<std::string>(m_pErrorMessageKey, std::string());
                if (!aMessage.empty())
                    aError += OString::Concat(": ") + std::string_view(aMessage);
            }
        }
        SAL_WARN("linguistic", "HttpRequest: request to " << m_aUrl << " failed: " << aError);
        return aError;
    }
    return {};
}

std::optional<boost::property_tree::ptree> parseJson(const std::string& rJson, const char* pWhat)
{
    try
    {
        boost::property_tree::ptree aRoot;
        std::stringstream aStream(rJson);
        boost::property_tree::read_json(aStream, aRoot);
        return aRoot;
    }
    catch (const boost::property_tree::ptree_error& rException)
    {
        SAL_WARN("linguistic", "parseJson: unexpected " << pWhat << ": " << rException.what());
        return std::nullopt;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
