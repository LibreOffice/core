/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * OpenSSL context management for SSL/TLS connections.
 * Classes: SslContext, ssl::CertificateVerification
 */

#pragma once

#include <common/Util.hpp>

#include <cassert>
#include <memory>
#include <string>

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/conf.h>

namespace ssl
{
/// The certificate verification requirements.
enum class CertificateVerification: std::uint8_t
{
    Disabled, ///< No verification is performed or results ignored.
    Required ///< Certificate must be provided and will be verified.
};
} // namespace ssl

class SslContext final
{
public:
    SslContext(const std::string& certFilePath, const std::string& keyFilePath,
               const std::string& caFilePath, const std::string& cipherList,
               ssl::CertificateVerification verification);

    /// Returns a new SSL Context to be used with raw API.
    SSL* newSsl() { return SSL_new(_ctx); }

    ~SslContext();

    ssl::CertificateVerification verification() const { return _verification; }

private:
    void initDH();
    void initECDH();
    void shutdown();

    std::string getLastErrorMsg() const;

private:
    SSL_CTX* _ctx;
    const ssl::CertificateVerification _verification;
};

namespace ssl
{
class Manager
{
public:
    static void initializeServerContext(const std::string& certFilePath,
                                        const std::string& keyFilePath,
                                        const std::string& caFilePath,
                                        const std::string& cipherList,
                                        ssl::CertificateVerification verification)
    {
        assert(!isServerContextInitialized() &&
               "Cannot initialize the server context more than once");
        ServerInstance = std::make_unique<SslContext>(certFilePath, keyFilePath, caFilePath,
                                                      cipherList, verification);
    }

    static void uninitializeServerContext() { ServerInstance.reset(); }

    /// Returns true iff the Server SslContext has been initialized.
    static bool isServerContextInitialized() { return !!ServerInstance; }

    static SSL* newServerSsl(ssl::CertificateVerification& verification)
    {
        assert(isServerContextInitialized() && "Server SslContext is not initialized");
        verification = ServerInstance->verification();
        return ServerInstance->newSsl();
    }

    static void initializeClientContext(const std::string& certFilePath,
                                        const std::string& keyFilePath,
                                        const std::string& caFilePath,
                                        const std::string& cipherList,
                                        ssl::CertificateVerification verification)
    {
        assert(!isClientContextInitialized() &&
               "Cannot initialize the client context more than once");
        ClientInstance = std::make_unique<SslContext>(certFilePath, keyFilePath, caFilePath,
                                                      cipherList, verification);
    }

    static ssl::CertificateVerification getClientVerification()
    {
        assert(isClientContextInitialized() && "client context must be initialized");
        return ClientInstance->verification();
    }

    static void uninitializeClientContext() { ClientInstance.reset(); }

    /// Returns true iff the SslContext has been initialized.
    static bool isClientContextInitialized() { return !!ClientInstance; }

    static SSL* newClientSsl(ssl::CertificateVerification& verification)
    {
        assert(isClientContextInitialized() && "Client SslContext is not initialized");
        verification = ClientInstance->verification();
        return ClientInstance->newSsl();
    }

private:
    static std::unique_ptr<SslContext> ServerInstance;
    static std::unique_ptr<SslContext> ClientInstance;
};

/// Generate a self-signed certificate and matching private key under @dir
/// (created if necessary), using @commonName as the certificate's CN. Writes
/// privkey.pem and cert.pem; the certificate is its own CA. Returns true on
/// success. This replaces the openssl command-line invocation that the
/// container start script used to run before launching the server.
bool generateSelfSignedCert(const std::string& dir, const std::string& commonName);

} // namespace ssl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
