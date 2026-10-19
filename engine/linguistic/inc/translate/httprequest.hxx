/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/string.hxx>
#include <boost/property_tree/ptree_fwd.hpp>
#include <curl/curl.h>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace linguistic::translate
{
/// One HTTP request to a translation service: a curl handle with the response
/// body collected. A GET request unless a body is set. The request blocks the
/// thread it is sent from.
class HttpRequest
{
public:
    HttpRequest(const OString& rUrl, std::chrono::seconds aTimeout,
                std::chrono::seconds aConnectTimeout);

    // curl keeps pointers into the object: it stays where it is.
    HttpRequest(const HttpRequest&) = delete;
    HttpRequest& operator=(const HttpRequest&) = delete;

    /// False when curl could not be initialized; nothing else works then.
    bool isValid() const { return m_pCurl != nullptr; }

    /// Adds a request header, e.g. "Authorization: Bearer x".
    void addHeader(const OString& rHeader);

    /// Makes it a POST request with rBody as form fields, percent-encoded by
    /// the caller with escape().
    void setPostFields(const OString& rBody);

    /// Makes it a POST request with the JSON document rJson as its body.
    void setJsonBody(const OString& rJson);

    /// Whether the server's certificate is checked; it is unless told otherwise.
    void setVerifySSL(bool bVerify);

    /// The JSON member of a failure response that says what went wrong, e.g.
    /// "message" for DeepL or "error" for LibreTranslate; its value is added
    /// to the failure description of perform().
    void setErrorMessageKey(const char* pKey) { m_pErrorMessageKey = pKey; }

    /// Percent-encodes rValue for a form field.
    OString escape(const OString& rValue) const;

    /// Sends the request. Returns an empty string when the service answered
    /// with HTTP 200, otherwise describes the failure: the curl error, or
    /// the HTTP status with the service's own message.
    OString perform();

    /// What the service answered.
    const std::string& getResponseBody() const { return m_aResponseBody; }

private:
    const OString m_aUrl;
    std::unique_ptr<CURL, std::function<void(CURL*)>> m_pCurl;
    std::unique_ptr<curl_slist, std::function<void(curl_slist*)>> m_pHeaders{
        nullptr, [](curl_slist* p) { curl_slist_free_all(p); }
    };
    /// The POST body; curl keeps a pointer to it, so it lives as long as the request.
    OString m_aBody;
    std::string m_aResponseBody;
    const char* m_pErrorMessageKey = nullptr;
};

/// Parses a JSON document; nothing when it is not one, with a warning that
/// names pWhat.
std::optional<boost::property_tree::ptree> parseJson(const std::string& rJson, const char* pWhat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
