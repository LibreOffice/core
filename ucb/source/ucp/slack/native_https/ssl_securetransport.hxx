/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifdef __APPLE__

#include "NativeHTTPSServer.hxx"
#include <Security/Security.h>
#include <Security/SecureTransport.h>

namespace ucp {
namespace slack {
namespace https {

/**
 * macOS SecureTransport SSL implementation
 */
class SecureTransportSSLContext : public SSLContext
{
public:
    SecureTransportSSLContext();
    ~SecureTransportSSLContext() override;

    bool initialize() override;
    bool createSSLSocket(int socket) override;
    bool performHandshake() override;
    int readSSL(void* buffer, size_t length) override;
    int writeSSL(const void* buffer, size_t length) override;
    void cleanup() override;

private:
    // Generate self-signed certificate for localhost
    bool generateLocalhostCertificate();

    // SecureTransport I/O callbacks
    static OSStatus readCallback(SSLConnectionRef connection, void* data, size_t* dataLength);
    static OSStatus writeCallback(SSLConnectionRef connection, const void* data, size_t* dataLength);

private:
    SSLContextRef m_sslContext;
    SecIdentityRef m_identity;
    int m_socket;
    bool m_initialized;
};

} // namespace https
} // namespace slack
} // namespace ucp

#endif // __APPLE__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
