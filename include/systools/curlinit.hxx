/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <curl/curl.h>

#include <officecfg/Office/Security.hxx>

// curl is built with --with-secure-transport on macOS and iOS so doesn't need these
// certs. Windows doesn't need them either, but lets assume everything else does
#if !defined(SYSTEM_OPENSSL) && !defined(_WIN32) && !defined(MACOSX) && !defined(IOS)
#include <com/sun/star/uno/RuntimeException.hpp>

#define LO_CURL_NEEDS_CA_BUNDLE
#include "opensslinit.hxx"
#endif

#include <rtl/string.hxx>
#include <sal/log.hxx>

#include <config_version.h>

static void InitCurl_easy(CURL* const pCURL)
{
    CURLcode rc;
    (void)rc;

#if defined(LO_CURL_NEEDS_CA_BUNDLE)
    char const* const path = GetCABundleFile();
    rc = curl_easy_setopt(pCURL, CURLOPT_CAINFO, path);
    if (rc != CURLE_OK) // only if OOM?
    {
        throw css::uno::RuntimeException("CURLOPT_CAINFO failed");
    }
#endif

    if (!officecfg::Office::Security::Net::AllowInsecureProtocols::get())
    {
        rc = curl_easy_setopt(pCURL, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        assert(rc == CURLE_OK);
        rc = curl_easy_setopt(pCURL, CURLOPT_PROXY_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        assert(rc == CURLE_OK);
#if (LIBCURL_VERSION_MAJOR > 7) || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 85)
        rc = curl_easy_setopt(pCURL, CURLOPT_PROTOCOLS_STR, "https");
        assert(rc == CURLE_OK);
        rc = curl_easy_setopt(pCURL, CURLOPT_REDIR_PROTOCOLS_STR, "https");
        assert(rc == CURLE_OK);
#else
        rc = curl_easy_setopt(pCURL, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
        assert(rc == CURLE_OK);
        rc = curl_easy_setopt(pCURL, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
        assert(rc == CURLE_OK);
#endif
    }

    curl_version_info_data const* const pVersion(curl_version_info(CURLVERSION_NOW));
    assert(pVersion);
    SAL_INFO("ucb.ucp.webdav.curl",
             "curl version: " << pVersion->version << " " << pVersion->host
                              << " features: " << ::std::hex << pVersion->features << " ssl: "
                              << pVersion->ssl_version << " libz: " << pVersion->libz_version);
    // Make sure a User-Agent header is always included, as at least
    // en.wikipedia.org:80 forces back 403 "Scripts should use an informative
    // User-Agent string with contact information, or they may be IP-blocked
    // without notice" otherwise:
    OString const useragent(
        OString::Concat("LibreOffice " LIBO_VERSION_DOTTED " denylistedbackend/")
        + pVersion->version + " " + pVersion->ssl_version);
    // looks like an explicit "User-Agent" header in CURLOPT_HTTPHEADER
    // will override CURLOPT_USERAGENT, see Curl_http_useragent(), so no need
    // to check anything here
    rc = curl_easy_setopt(pCURL, CURLOPT_USERAGENT, useragent.getStr());
    assert(rc == CURLE_OK);
}

#undef LO_CURL_NEEDS_CA_BUNDLE

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
