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

#if defined(LINUX) && !defined(SYSTEM_CURL)
#include <com/sun/star/uno/RuntimeException.hpp>

#include "opensslinit.hxx"
#endif

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

    if (!officecfg::Office::Security::Net::AllowInsecureProtocols::get())
    {
        rc = curl_easy_setopt(pCURL, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        assert(rc == CURLE_OK);
        rc = curl_easy_setopt(pCURL, CURLOPT_PROXY_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        assert(rc == CURLE_OK);
        rc = curl_easy_setopt(pCURL, CURLOPT_PROTOCOLS_STR, "https");
        assert(rc == CURLE_OK);
        rc = curl_easy_setopt(pCURL, CURLOPT_REDIR_PROTOCOLS_STR, "https");
        assert(rc == CURLE_OK);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
