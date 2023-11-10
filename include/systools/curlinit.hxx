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

#if defined(LINUX) && !defined(SYSTEM_CURL)
#include <com/sun/star/uno/RuntimeException.hpp>

#include "opensslinit.hxx"
#endif

#include <rtl/string.hxx>
#include <sal/log.hxx>

#include <config_version.h>

static void InitCurl_easy(CURL* const pCURL)
{
    CURLcode rc;
    (void)rc;

#if defined(LINUX) && !defined(SYSTEM_CURL)
    char const* const path = GetCABundleFile();
    rc = curl_easy_setopt(pCURL, CURLOPT_CAINFO, path);
    if (rc != CURLE_OK) // only if OOM?
    {
        throw css::uno::RuntimeException("CURLOPT_CAINFO failed");
    }
#endif

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
