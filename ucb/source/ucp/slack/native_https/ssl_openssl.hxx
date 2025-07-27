/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// OpenSSL implementation now available on all platforms for consistent behavior
#include "NativeHTTPSServer.hxx"

// Use system OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace ucp {
namespace slack {
namespace https {

/**
 * Shared OpenSSL context for certificate and SSL_CTX reuse
 */
class SharedOpenSSLContext
{
public:
    static SharedOpenSSLContext& getInstance();

    SSL_CTX* getSSLContext() { return m_sslCtx; }
    bool isInitialized() const { return m_initialized; }

private:
    SharedOpenSSLContext();
    ~SharedOpenSSLContext();
    bool initialize();
    bool generateLocalhostCertificate();
    EVP_PKEY* createPrivateKey();
    X509* createCertificate(EVP_PKEY* pkey);
    bool addCertificateExtensions(X509* cert, X509* issuer, const char* name, const char* value);

    SSL_CTX* m_sslCtx;
    X509* m_cert;
    EVP_PKEY* m_pkey;
    bool m_initialized;
    static bool s_openSSLInitialized;
};

/**
 * Linux OpenSSL SSL implementation
 */
class OpenSSLContext : public SSLContext
{
public:
    OpenSSLContext();
    ~OpenSSLContext() override;

    bool initialize() override;
    bool createSSLSocket(int socket) override;
    bool performHandshake() override;
    int readSSL(void* buffer, size_t length) override;
    int writeSSL(const void* buffer, size_t length) override;
    void cleanup() override;

private:
    SSL* m_ssl;
    int m_socket;
};

} // namespace https
} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
