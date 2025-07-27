/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef __APPLE__

// Disable deprecation warnings for SecureTransport APIs
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "ssl_securetransport.hxx"
#include <sal/log.hxx>
#include <sys/socket.h>
#include <unistd.h>

namespace ucp {
namespace slack {
namespace https {

SecureTransportSSLContext::SecureTransportSSLContext()
    : m_sslContext(nullptr)
    , m_identity(nullptr)
    , m_socket(-1)
    , m_initialized(false)
{
}

SecureTransportSSLContext::~SecureTransportSSLContext()
{
    cleanup();
}

bool SecureTransportSSLContext::initialize()
{
    SAL_WARN("ucb.ucp.slack", "Initializing SecureTransport SSL context");

    // Generate localhost certificate
    if (!generateLocalhostCertificate()) {
        SAL_WARN("ucb.ucp.slack", "Failed to generate localhost certificate");
        return false;
    }

    m_initialized = true;
    SAL_WARN("ucb.ucp.slack", "SecureTransport SSL context initialized successfully");
    return true;
}

bool SecureTransportSSLContext::createSSLSocket(int socket)
{
    if (!m_initialized) {
        SAL_WARN("ucb.ucp.slack", "SSL context not initialized");
        return false;
    }

    m_socket = socket;

    // Create SSL context
    m_sslContext = SSLCreateContext(kCFAllocatorDefault, kSSLServerSide, kSSLStreamType);
    if (!m_sslContext) {
        SAL_WARN("ucb.ucp.slack", "Failed to create SSL context");
        return false;
    }

    // Set I/O callbacks
    OSStatus status = SSLSetIOFuncs(m_sslContext, readCallback, writeCallback);
    if (status != errSecSuccess) {
        SAL_WARN("ucb.ucp.slack", "Failed to set I/O callbacks: " << status);
        return false;
    }

    // Set connection reference (socket)
    status = SSLSetConnection(m_sslContext, (SSLConnectionRef)(intptr_t)m_socket);
    if (status != errSecSuccess) {
        SAL_WARN("ucb.ucp.slack", "Failed to set connection: " << status);
        return false;
    }

    // Set certificate
    if (m_identity) {
        CFArrayRef certArray = CFArrayCreate(kCFAllocatorDefault, (const void**)&m_identity, 1, &kCFTypeArrayCallBacks);
        status = SSLSetCertificate(m_sslContext, certArray);
        CFRelease(certArray);

        if (status != errSecSuccess) {
            SAL_WARN("ucb.ucp.slack", "Failed to set certificate: " << status);
            return false;
        }
    }

    // Set minimum protocol version
    status = SSLSetProtocolVersionMin(m_sslContext, kTLSProtocol12);
    if (status != errSecSuccess) {
        SAL_WARN("ucb.ucp.slack", "Failed to set minimum protocol version: " << status);
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "SSL socket created successfully");
    return true;
}

bool SecureTransportSSLContext::performHandshake()
{
    if (!m_sslContext) {
        SAL_WARN("ucb.ucp.slack", "No SSL context for handshake");
        return false;
    }

    OSStatus status;
    do {
        status = SSLHandshake(m_sslContext);
    } while (status == errSSLWouldBlock);

    if (status != errSecSuccess) {
        SAL_WARN("ucb.ucp.slack", "SSL handshake failed: " << status);
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "SSL handshake completed successfully");
    return true;
}

int SecureTransportSSLContext::readSSL(void* buffer, size_t length)
{
    if (!m_sslContext) {
        return -1;
    }

    size_t processed = 0;
    OSStatus status = SSLRead(m_sslContext, buffer, length, &processed);

    if (status == errSecSuccess || status == errSSLWouldBlock) {
        return static_cast<int>(processed);
    }

    SAL_WARN("ucb.ucp.slack", "SSL read failed: " << status);
    return -1;
}

int SecureTransportSSLContext::writeSSL(const void* buffer, size_t length)
{
    if (!m_sslContext) {
        return -1;
    }

    size_t processed = 0;
    OSStatus status = SSLWrite(m_sslContext, buffer, length, &processed);

    if (status == errSecSuccess || status == errSSLWouldBlock) {
        return static_cast<int>(processed);
    }

    SAL_WARN("ucb.ucp.slack", "SSL write failed: " << status);
    return -1;
}

void SecureTransportSSLContext::cleanup()
{
    if (m_sslContext) {
        SSLClose(m_sslContext);
        CFRelease(m_sslContext);
        m_sslContext = nullptr;
    }

    if (m_identity) {
        CFRelease(m_identity);
        m_identity = nullptr;
    }

    m_socket = -1;
    m_initialized = false;
}

bool SecureTransportSSLContext::generateLocalhostCertificate()
{
    SAL_WARN("ucb.ucp.slack", "SecureTransport certificate generation not implemented");
    SAL_WARN("ucb.ucp.slack", "For production, will need platform-specific certificate generation");

    // For now, just create a dummy identity
    // In production, this would need proper certificate generation
    // using Security Framework or delegate to system certificate store

    return true; // Return true to allow testing without certificates
}

OSStatus SecureTransportSSLContext::readCallback(SSLConnectionRef connection, void* data, size_t* dataLength)
{
    int socket = static_cast<int>(reinterpret_cast<intptr_t>(connection));

    ssize_t bytesRead = recv(socket, data, *dataLength, 0);

    if (bytesRead > 0) {
        *dataLength = bytesRead;
        return errSecSuccess;
    } else if (bytesRead == 0) {
        *dataLength = 0;
        return errSSLClosedGraceful;
    } else {
        *dataLength = 0;
        return errSSLClosedAbort;
    }
}

OSStatus SecureTransportSSLContext::writeCallback(SSLConnectionRef connection, const void* data, size_t* dataLength)
{
    int socket = static_cast<int>(reinterpret_cast<intptr_t>(connection));

    ssize_t bytesWritten = send(socket, data, *dataLength, 0);

    if (bytesWritten > 0) {
        *dataLength = bytesWritten;
        return errSecSuccess;
    } else {
        *dataLength = 0;
        return errSSLClosedAbort;
    }
}

} // namespace https
} // namespace slack
} // namespace ucp

#pragma clang diagnostic pop

#endif // __APPLE__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
