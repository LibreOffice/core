/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// OpenSSL implementation now available on all platforms for consistent behavior
#include "ssl_openssl.hxx"
#include <sal/log.hxx>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <cstring>

namespace ucp {
namespace slack {
namespace https {

bool SharedOpenSSLContext::s_openSSLInitialized = false;

SharedOpenSSLContext& SharedOpenSSLContext::getInstance()
{
    static SharedOpenSSLContext instance;
    return instance;
}

SharedOpenSSLContext::SharedOpenSSLContext()
    : m_sslCtx(nullptr)
    , m_cert(nullptr)
    , m_pkey(nullptr)
    , m_initialized(false)
{
    m_initialized = initialize();
}

SharedOpenSSLContext::~SharedOpenSSLContext()
{
    if (m_sslCtx) {
        SSL_CTX_free(m_sslCtx);
        m_sslCtx = nullptr;
    }

    if (m_cert) {
        X509_free(m_cert);
        m_cert = nullptr;
    }

    if (m_pkey) {
        EVP_PKEY_free(m_pkey);
        m_pkey = nullptr;
    }
}

bool SharedOpenSSLContext::initialize()
{
    SAL_WARN("ucb.ucp.slack", "Initializing shared OpenSSL SSL context");

    // Initialize OpenSSL (only once)
    if (!s_openSSLInitialized) {
        SSL_load_error_strings();
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        s_openSSLInitialized = true;
    }

    // Create SSL context
    m_sslCtx = SSL_CTX_new(TLS_server_method());
    if (!m_sslCtx) {
        SAL_WARN("ucb.ucp.slack", "Failed to create SSL context");
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Set minimum protocol version
    SSL_CTX_set_min_proto_version(m_sslCtx, TLS1_2_VERSION);

    // Generate localhost certificate
    if (!generateLocalhostCertificate()) {
        SAL_WARN("ucb.ucp.slack", "Failed to generate localhost certificate");
        return false;
    }

    // Set certificate and private key
    if (SSL_CTX_use_certificate(m_sslCtx, m_cert) != 1) {
        SAL_WARN("ucb.ucp.slack", "Failed to set certificate");
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (SSL_CTX_use_PrivateKey(m_sslCtx, m_pkey) != 1) {
        SAL_WARN("ucb.ucp.slack", "Failed to set private key");
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Verify that the certificate and private key match
    if (SSL_CTX_check_private_key(m_sslCtx) != 1) {
        SAL_WARN("ucb.ucp.slack", "Certificate and private key do not match");
        ERR_print_errors_fp(stderr);
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "Shared OpenSSL SSL context initialized successfully");
    return true;
}

OpenSSLContext::OpenSSLContext()
    : m_ssl(nullptr)
    , m_socket(-1)
{
}

OpenSSLContext::~OpenSSLContext()
{
    cleanup();
}

bool OpenSSLContext::initialize()
{
    // No initialization needed - we use the shared context
    return true;
}

bool OpenSSLContext::createSSLSocket(int socket)
{
    auto& sharedContext = SharedOpenSSLContext::getInstance();
    if (!sharedContext.isInitialized()) {
        SAL_WARN("ucb.ucp.slack", "Shared SSL context not initialized");
        return false;
    }

    m_socket = socket;

    // Create SSL structure from shared context
    m_ssl = SSL_new(sharedContext.getSSLContext());
    if (!m_ssl) {
        SAL_WARN("ucb.ucp.slack", "Failed to create SSL structure");
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Set the socket file descriptor
    if (SSL_set_fd(m_ssl, m_socket) != 1) {
        SAL_WARN("ucb.ucp.slack", "Failed to set SSL file descriptor");
        ERR_print_errors_fp(stderr);
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "SSL socket created successfully");
    return true;
}

bool OpenSSLContext::performHandshake()
{
    if (!m_ssl) {
        SAL_WARN("ucb.ucp.slack", "No SSL structure for handshake");
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "Starting SSL handshake");

    int result = SSL_accept(m_ssl);
    if (result != 1) {
        int error = SSL_get_error(m_ssl, result);
        SAL_WARN("ucb.ucp.slack", "SSL handshake failed with error: " << error);
        ERR_print_errors_fp(stderr);
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "SSL handshake completed successfully");
    return true;
}

int OpenSSLContext::readSSL(void* buffer, size_t length)
{
    if (!m_ssl) {
        return -1;
    }

    int bytesRead = SSL_read(m_ssl, buffer, static_cast<int>(length));

    if (bytesRead <= 0) {
        int error = SSL_get_error(m_ssl, bytesRead);
        if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE) {
            SAL_WARN("ucb.ucp.slack", "SSL read error: " << error);
            ERR_print_errors_fp(stderr);
        }
        return -1;
    }

    return bytesRead;
}

int OpenSSLContext::writeSSL(const void* buffer, size_t length)
{
    if (!m_ssl) {
        return -1;
    }

    int bytesWritten = SSL_write(m_ssl, buffer, static_cast<int>(length));

    if (bytesWritten <= 0) {
        int error = SSL_get_error(m_ssl, bytesWritten);
        if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE) {
            SAL_WARN("ucb.ucp.slack", "SSL write error: " << error);
            ERR_print_errors_fp(stderr);
        }
        return -1;
    }

    return bytesWritten;
}

void OpenSSLContext::cleanup()
{
    if (m_ssl) {
        SSL_shutdown(m_ssl);
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }

    m_socket = -1;
}

bool SharedOpenSSLContext::generateLocalhostCertificate()
{
    SAL_WARN("ucb.ucp.slack", "Generating localhost certificate using OpenSSL");

    // Generate private key
    m_pkey = createPrivateKey();
    if (!m_pkey) {
        SAL_WARN("ucb.ucp.slack", "Failed to create private key");
        return false;
    }

    // Create certificate
    m_cert = createCertificate(m_pkey);
    if (!m_cert) {
        SAL_WARN("ucb.ucp.slack", "Failed to create certificate");
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "Localhost certificate generated successfully");
    return true;
}

EVP_PKEY* SharedOpenSSLContext::createPrivateKey()
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to initialize key generation");
        ERR_print_errors_fp(stderr);
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to set key size");
        ERR_print_errors_fp(stderr);
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to generate key");
        ERR_print_errors_fp(stderr);
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

X509* SharedOpenSSLContext::createCertificate(EVP_PKEY* pkey)
{
    X509* cert = X509_new();
    if (!cert) {
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    // Set version (X.509 v3)
    X509_set_version(cert, 2);

    // Set serial number
    ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);

    // Set validity period (1 year)
    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), 365 * 24 * 3600); // 1 year

    // Set public key
    X509_set_pubkey(cert, pkey);

    // Set subject name (CN=localhost)
    X509_NAME* name = X509_get_subject_name(cert);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                              reinterpret_cast<const unsigned char*>("localhost"), -1, -1, 0);

    // Set issuer name (self-signed)
    X509_set_issuer_name(cert, name);

    // Add extensions for browser compatibility
    addCertificateExtensions(cert, cert, "basicConstraints", "CA:FALSE");
    addCertificateExtensions(cert, cert, "keyUsage", "critical,digitalSignature,keyEncipherment");
    addCertificateExtensions(cert, cert, "extendedKeyUsage", "serverAuth");
    addCertificateExtensions(cert, cert, "subjectAltName", "DNS:localhost,DNS:*.localhost,IP:127.0.0.1,IP:::1");

    // Sign the certificate
    if (X509_sign(cert, pkey, EVP_sha256()) == 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to sign certificate");
        ERR_print_errors_fp(stderr);
        X509_free(cert);
        return nullptr;
    }

    return cert;
}

bool SharedOpenSSLContext::addCertificateExtensions(X509* cert, X509* issuer, const char* name, const char* value)
{
    X509_EXTENSION* ext = nullptr;
    X509V3_CTX ctx;

    X509V3_set_ctx(&ctx, issuer, cert, nullptr, nullptr, 0);
    ext = X509V3_EXT_conf(nullptr, &ctx, name, value);

    if (!ext) {
        SAL_WARN("ucb.ucp.slack", "Failed to create extension: " << name);
        ERR_print_errors_fp(stderr);
        return false;
    }

    X509_add_ext(cert, ext, -1);
    X509_EXTENSION_free(ext);

    return true;
}

} // namespace https
} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
