/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_crypto.h>

// Also include/systools/curlinit.hxx needs GetCABundleFile() if
// !defined(SYSTEM_CURL) it defines LO_CURL_NEEDS_CA_BUNDLE.
#if !defined(_WIN32) && (!defined(SYSTEM_OPENSSL) || defined(LO_CURL_NEEDS_CA_BUNDLE))

#include <unistd.h>

static char const* GetCABundleFile()
{
    // try system ones first; inspired by:
    // https://www.happyassassin.net/posts/2015/01/12/a-note-about-ssltls-trusted-certificate-stores-and-platforms/
    auto const candidates = {
        "/etc/pki/tls/certs/ca-bundle.crt",
        "/etc/pki/tls/certs/ca-bundle.trust.crt",
        "/etc/ssl/certs/ca-certificates.crt",
        "/var/lib/ca-certificates/ca-bundle.pem",
        "/etc/ssl/cert.pem", // macOS has one at this location
    };
    for (char const* const candidate : candidates)
    {
        if (access(candidate, R_OK) == 0)
        {
            return candidate;
        }
    }

    throw css::uno::RuntimeException("no OpenSSL CA certificate bundle found");
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
