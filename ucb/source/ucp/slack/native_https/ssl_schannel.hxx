/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifdef _WIN32

#include "NativeHTTPSServer.hxx"
#include <windows.h>
#include <wincrypt.h>
#include <schannel.h>
#include <sspi.h>

#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "crypt32.lib")

namespace ucp {
namespace slack {
namespace https {

/**
 * Windows SChannel SSL implementation
 */
class SChannelSSLContext : public SSLContext
{
public:
    SChannelSSLContext();
    ~SChannelSSLContext() override;

    bool initialize() override;
    bool createSSLSocket(int socket) override;
    bool performHandshake() override;
    int readSSL(void* buffer, size_t length) override;
    int writeSSL(const void* buffer, size_t length) override;
    void cleanup() override;

private:
    // Generate self-signed certificate for localhost
    bool generateLocalhostCertificate();

    // Perform SSL handshake steps
    bool doHandshakeStep();

    // Encrypt/decrypt data
    bool encryptData(const void* plainData, size_t plainLength, void* encryptedData, size_t* encryptedLength);
    bool decryptData(const void* encryptedData, size_t encryptedLength, void* plainData, size_t* plainLength);

private:
    CredHandle m_credHandle;
    CtxtHandle m_ctxtHandle;
    PCCERT_CONTEXT m_certContext;
    SecPkgContext_StreamSizes m_streamSizes;
    int m_socket;
    bool m_initialized;
    bool m_handshakeComplete;
    std::vector<BYTE> m_receiveBuf;
    std::vector<BYTE> m_decryptBuf;
};

} // namespace https
} // namespace slack
} // namespace ucp

#endif // _WIN32

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
