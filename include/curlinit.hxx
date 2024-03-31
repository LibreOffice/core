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

// curl is built with --with-secure-transport on macOS and iOS so doesn't need these
// certs. Windows doesn't need them either, but lets assume everything else does
#if !defined(SYSTEM_OPENSSL) && !defined(_WIN32) && !defined(MACOSX) && !defined(IOS)
#include <com/sun/star/uno/RuntimeException.hpp>

#include "opensslinit.hxx"

static void InitCurl_easy(CURL* const pCURL)
{
    char const* const path = GetCABundleFile();
    auto rc = curl_easy_setopt(pCURL, CURLOPT_CAINFO, path);
    if (rc != CURLE_OK) // only if OOM?
    {
        throw css::uno::RuntimeException("CURLOPT_CAINFO failed");
    }
}

#else

static void InitCurl_easy(CURL* const)
{
    // these don't use OpenSSL so CAs work out of the box
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
