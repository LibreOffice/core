/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef _WIN32

#include "ssl_schannel.hxx"
#include <sal/log.hxx>
#include <winsock2.h>
#include <vector>

namespace ucp {
namespace slack {
namespace https {

SChannelSSLContext::SChannelSSLContext()
    : m_certContext(nullptr)
    , m_socket(-1)
    , m_initialized(false)
    , m_handshakeComplete(false)
{
    memset(&m_credHandle, 0, sizeof(m_credHandle));
    memset(&m_ctxtHandle, 0, sizeof(m_ctxtHandle));
    memset(&m_streamSizes, 0, sizeof(m_streamSizes));
}

SChannelSSLContext::~SChannelSSLContext()
{
    cleanup();
}

bool SChannelSSLContext::initialize()
{
    SAL_WARN("ucb.ucp.slack", "Initializing SChannel SSL context");

    // Generate localhost certificate
    if (!generateLocalhostCertificate()) {
        SAL_WARN("ucb.ucp.slack", "Failed to generate localhost certificate");
        return false;
    }

    // Acquire credentials handle
    SCHANNEL_CRED credData;
    memset(&credData, 0, sizeof(credData));
    credData.dwVersion = SCHANNEL_CRED_VERSION;
    credData.cCreds = 1;
    credData.paCred = &m_certContext;
    credData.grbitEnabledProtocols = SP_PROT_TLS1_2 | SP_PROT_TLS1_3;
    credData.dwFlags = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION;

    TimeStamp tsExpiry;
    SECURITY_STATUS status = AcquireCredentialsHandle(
        nullptr,
        const_cast<LPWSTR>(UNISP_NAME),
        SECPKG_CRED_INBOUND,
        nullptr,
        &credData,
        nullptr,
        nullptr,
        &m_credHandle,
        &tsExpiry
    );

    if (status != SEC_E_OK) {
        SAL_WARN("ucb.ucp.slack", "Failed to acquire credentials handle: " << status);
        return false;
    }

    m_initialized = true;
    SAL_WARN("ucb.ucp.slack", "SChannel SSL context initialized successfully");
    return true;
}

bool SChannelSSLContext::createSSLSocket(int socket)
{
    if (!m_initialized) {
        SAL_WARN("ucb.ucp.slack", "SSL context not initialized");
        return false;
    }

    m_socket = socket;
    m_handshakeComplete = false;

    SAL_WARN("ucb.ucp.slack", "SSL socket created successfully");
    return true;
}

bool SChannelSSLContext::performHandshake()
{
    if (!m_initialized || m_socket == -1) {
        SAL_WARN("ucb.ucp.slack", "SSL context not ready for handshake");
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "Starting SSL handshake");

    // Perform handshake
    DWORD dwFlags = ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT |
                    ASC_REQ_CONFIDENTIALITY | ASC_REQ_EXTENDED_ERROR |
                    ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_STREAM;

    SecBufferDesc inBufferDesc, outBufferDesc;
    SecBuffer inBuffers[2], outBuffers[2];
    DWORD dwContextAttr;
    TimeStamp tsExpiry;

    // Initialize input buffers
    inBuffers[0].pvBuffer = nullptr;
    inBuffers[0].cbBuffer = 0;
    inBuffers[0].BufferType = SECBUFFER_TOKEN;

    inBuffers[1].pvBuffer = nullptr;
    inBuffers[1].cbBuffer = 0;
    inBuffers[1].BufferType = SECBUFFER_EMPTY;

    inBufferDesc.ulVersion = SECBUFFER_VERSION;
    inBufferDesc.cBuffers = 2;
    inBufferDesc.pBuffers = inBuffers;

    // Initialize output buffers
    outBuffers[0].pvBuffer = nullptr;
    outBuffers[0].cbBuffer = 0;
    outBuffers[0].BufferType = SECBUFFER_TOKEN;

    outBuffers[1].pvBuffer = nullptr;
    outBuffers[1].cbBuffer = 0;
    outBuffers[1].BufferType = SECBUFFER_ALERT;

    outBufferDesc.ulVersion = SECBUFFER_VERSION;
    outBufferDesc.cBuffers = 2;
    outBufferDesc.pBuffers = outBuffers;

    // Receive client hello
    std::vector<BYTE> receiveBuf(4096);
    int bytesReceived = recv(m_socket, reinterpret_cast<char*>(receiveBuf.data()), receiveBuf.size(), 0);

    if (bytesReceived <= 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to receive client hello");
        return false;
    }

    inBuffers[0].pvBuffer = receiveBuf.data();
    inBuffers[0].cbBuffer = bytesReceived;

    // Accept security context
    SECURITY_STATUS status = AcceptSecurityContext(
        &m_credHandle,
        nullptr,
        &inBufferDesc,
        dwFlags,
        SECURITY_NATIVE_DREP,
        &m_ctxtHandle,
        &outBufferDesc,
        &dwContextAttr,
        &tsExpiry
    );

    if (status == SEC_I_CONTINUE_NEEDED || status == SEC_E_OK) {
        // Send server response if there's output data
        if (outBuffers[0].cbBuffer > 0 && outBuffers[0].pvBuffer) {
            int bytesSent = send(m_socket,
                               reinterpret_cast<const char*>(outBuffers[0].pvBuffer),
                               outBuffers[0].cbBuffer, 0);

            FreeContextBuffer(outBuffers[0].pvBuffer);

            if (bytesSent != static_cast<int>(outBuffers[0].cbBuffer)) {
                SAL_WARN("ucb.ucp.slack", "Failed to send server response");
                return false;
            }
        }

        if (status == SEC_E_OK) {
            m_handshakeComplete = true;

            // Query stream sizes
            QueryContextAttributes(&m_ctxtHandle, SECPKG_ATTR_STREAM_SIZES, &m_streamSizes);

            SAL_WARN("ucb.ucp.slack", "SSL handshake completed successfully");
            return true;
        }
    }

    SAL_WARN("ucb.ucp.slack", "SSL handshake failed: " << status);
    return false;
}

int SChannelSSLContext::readSSL(void* buffer, size_t length)
{
    if (!m_handshakeComplete) {
        return -1;
    }

    // Simplified SSL read - in production this would need proper buffering
    std::vector<BYTE> encryptedBuf(m_streamSizes.cbMaximumMessage);
    int bytesReceived = recv(m_socket, reinterpret_cast<char*>(encryptedBuf.data()), encryptedBuf.size(), 0);

    if (bytesReceived <= 0) {
        return -1;
    }

    // Decrypt the data
    SecBufferDesc bufferDesc;
    SecBuffer buffers[4];

    buffers[0].pvBuffer = encryptedBuf.data();
    buffers[0].cbBuffer = bytesReceived;
    buffers[0].BufferType = SECBUFFER_DATA;

    buffers[1].pvBuffer = nullptr;
    buffers[1].cbBuffer = 0;
    buffers[1].BufferType = SECBUFFER_EMPTY;

    buffers[2].pvBuffer = nullptr;
    buffers[2].cbBuffer = 0;
    buffers[2].BufferType = SECBUFFER_EMPTY;

    buffers[3].pvBuffer = nullptr;
    buffers[3].cbBuffer = 0;
    buffers[3].BufferType = SECBUFFER_EMPTY;

    bufferDesc.ulVersion = SECBUFFER_VERSION;
    bufferDesc.cBuffers = 4;
    bufferDesc.pBuffers = buffers;

    SECURITY_STATUS status = DecryptMessage(&m_ctxtHandle, &bufferDesc, 0, nullptr);

    if (status == SEC_E_OK) {
        // Find the data buffer
        for (int i = 0; i < 4; i++) {
            if (buffers[i].BufferType == SECBUFFER_DATA) {
                size_t copyLength = std::min(static_cast<size_t>(buffers[i].cbBuffer), length);
                memcpy(buffer, buffers[i].pvBuffer, copyLength);
                return static_cast<int>(copyLength);
            }
        }
    }

    SAL_WARN("ucb.ucp.slack", "SSL read failed: " << status);
    return -1;
}

int SChannelSSLContext::writeSSL(const void* buffer, size_t length)
{
    if (!m_handshakeComplete) {
        return -1;
    }

    // Encrypt the data
    std::vector<BYTE> encryptBuf(m_streamSizes.cbHeader + length + m_streamSizes.cbTrailer);

    SecBufferDesc bufferDesc;
    SecBuffer buffers[4];

    buffers[0].pvBuffer = encryptBuf.data();
    buffers[0].cbBuffer = m_streamSizes.cbHeader;
    buffers[0].BufferType = SECBUFFER_STREAM_HEADER;

    buffers[1].pvBuffer = encryptBuf.data() + m_streamSizes.cbHeader;
    buffers[1].cbBuffer = static_cast<ULONG>(length);
    buffers[1].BufferType = SECBUFFER_DATA;
    memcpy(buffers[1].pvBuffer, buffer, length);

    buffers[2].pvBuffer = encryptBuf.data() + m_streamSizes.cbHeader + length;
    buffers[2].cbBuffer = m_streamSizes.cbTrailer;
    buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;

    buffers[3].pvBuffer = nullptr;
    buffers[3].cbBuffer = 0;
    buffers[3].BufferType = SECBUFFER_EMPTY;

    bufferDesc.ulVersion = SECBUFFER_VERSION;
    bufferDesc.cBuffers = 4;
    bufferDesc.pBuffers = buffers;

    SECURITY_STATUS status = EncryptMessage(&m_ctxtHandle, 0, &bufferDesc, 0);

    if (status == SEC_E_OK) {
        ULONG totalLength = buffers[0].cbBuffer + buffers[1].cbBuffer + buffers[2].cbBuffer;
        int bytesSent = send(m_socket, reinterpret_cast<const char*>(encryptBuf.data()), totalLength, 0);

        if (bytesSent == static_cast<int>(totalLength)) {
            return static_cast<int>(length);
        }
    }

    SAL_WARN("ucb.ucp.slack", "SSL write failed: " << status);
    return -1;
}

void SChannelSSLContext::cleanup()
{
    if (m_handshakeComplete) {
        DeleteSecurityContext(&m_ctxtHandle);
        m_handshakeComplete = false;
    }

    if (m_initialized) {
        FreeCredentialsHandle(&m_credHandle);
        m_initialized = false;
    }

    if (m_certContext) {
        CertFreeCertificateContext(m_certContext);
        m_certContext = nullptr;
    }

    m_socket = -1;
}

bool SChannelSSLContext::generateLocalhostCertificate()
{
    SAL_WARN("ucb.ucp.slack", "Generating localhost certificate using SChannel");

    // Create certificate store
    HCERTSTORE hStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, nullptr);
    if (!hStore) {
        SAL_WARN("ucb.ucp.slack", "Failed to create certificate store");
        return false;
    }

    // Create subject name
    CERT_NAME_BLOB subjectBlob;
    LPCWSTR subjectName = L"CN=localhost";

    if (!CertStrToName(X509_ASN_ENCODING, subjectName, CERT_X500_NAME_STR, nullptr,
                       nullptr, &subjectBlob.cbData, nullptr)) {
        SAL_WARN("ucb.ucp.slack", "Failed to get subject name size");
        CertCloseStore(hStore, 0);
        return false;
    }

    std::vector<BYTE> subjectEncoded(subjectBlob.cbData);
    subjectBlob.pbData = subjectEncoded.data();

    if (!CertStrToName(X509_ASN_ENCODING, subjectName, CERT_X500_NAME_STR, nullptr,
                       subjectBlob.pbData, &subjectBlob.cbData, nullptr)) {
        SAL_WARN("ucb.ucp.slack", "Failed to encode subject name");
        CertCloseStore(hStore, 0);
        return false;
    }

    // Create key provider
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        SAL_WARN("ucb.ucp.slack", "Failed to acquire crypto context");
        CertCloseStore(hStore, 0);
        return false;
    }

    // Generate key pair
    HCRYPTKEY hKey = 0;
    if (!CryptGenKey(hProv, AT_SIGNATURE, 2048 << 16 | CRYPT_EXPORTABLE, &hKey)) {
        SAL_WARN("ucb.ucp.slack", "Failed to generate key pair");
        CryptReleaseContext(hProv, 0);
        CertCloseStore(hStore, 0);
        return false;
    }

    // Create certificate
    SYSTEMTIME notBefore, notAfter;
    GetSystemTime(&notBefore);
    notAfter = notBefore;
    notAfter.wYear += 1; // Valid for 1 year

    FILETIME ftNotBefore, ftNotAfter;
    SystemTimeToFileTime(&notBefore, &ftNotBefore);
    SystemTimeToFileTime(&notAfter, &ftNotAfter);

    m_certContext = CertCreateSelfSignCertificate(
        hProv,
        &subjectBlob,
        0,
        nullptr,
        nullptr,
        &ftNotBefore,
        &ftNotAfter,
        nullptr
    );

    CryptDestroyKey(hKey);
    CryptReleaseContext(hProv, 0);
    CertCloseStore(hStore, 0);

    if (!m_certContext) {
        SAL_WARN("ucb.ucp.slack", "Failed to create self-signed certificate");
        return false;
    }

    SAL_WARN("ucb.ucp.slack", "Localhost certificate generated successfully");
    return true;
}

} // namespace https
} // namespace slack
} // namespace ucp

#endif // _WIN32

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
