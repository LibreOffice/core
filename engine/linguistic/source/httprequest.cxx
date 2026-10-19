/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <linguistic/httprequest.hxx>

#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <systools/curlinit.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <curl/curl.h>

#include <array>
#include <memory>
#include <sstream>

namespace linguistic::http
{
namespace
{
size_t writeCallback(const char* pData, size_t nSize, size_t nCount, void* pUserData)
{
    std::string* pBody = static_cast<std::string*>(pUserData);
    pBody->append(pData, nSize * nCount);
    return nSize * nCount;
}

// Deleters for std::unique_ptr, so a curl handle and a header list are freed
// on every way out of perform().
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
        SAL_WARN("linguistic.http", "curl initialization failed");
        aResponse.aError = "CURL initialization failed"_ostr;
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
    (void)curl_easy_setopt(pCurl.get(), CURLOPT_TIMEOUT,
                           static_cast<long>(rRequest.aTimeout.count()));
    (void)curl_easy_setopt(pCurl.get(), CURLOPT_CONNECTTIMEOUT,
                           static_cast<long>(rRequest.aConnectTimeout.count()));
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

    // The body of an error answer is read too: it tends to say why.
    const CURLcode eCode = curl_easy_perform(pCurl.get());
    curl_easy_getinfo(pCurl.get(), CURLINFO_RESPONSE_CODE, &aResponse.nStatus);
    // The callers say when a failure is worth a warning: a checker reports
    // a server once, not on every sentence.
    if (eCode != CURLE_OK)
    {
        aResponse.aError = OString(curl_easy_strerror(eCode));
        SAL_INFO("linguistic.http", aURL << " failed: (" << static_cast<sal_Int32>(eCode) << ") "
                                         << aResponse.aError << ", HTTP status "
                                         << aResponse.nStatus);
    }
    else if (aResponse.nStatus >= 400)
    {
        aResponse.aError = "HTTP " + OString::number(aResponse.nStatus);
        SAL_INFO("linguistic.http", aURL << " failed: " << aResponse.aError);
    }
    else
        aResponse.bOk = true;
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

OString percentEncode(std::string_view rUtf8)
{
    return percentEncode(OUString(rUtf8.data(), rUtf8.size(), RTL_TEXTENCODING_UTF8));
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
        SAL_WARN("linguistic.http", "cannot parse the answer: " << rError.what());
        return std::nullopt;
    }
    return aRoot;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
