/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "slack_oauth2_server.hxx"
#include <config_oauth2.h>
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#include <cstring>
#include <chrono>
#include <fstream>
#include <vector>

namespace ucp {
namespace slack {

SlackOAuth2Server::SlackOAuth2Server()
    : m_bRunning(false)
    , m_bCodeReceived(false)
    , m_nPort(8080) // Changed to 8080 for direct HTTPS
    , m_nSocketFd(-1)
    , m_pSSLCtx(nullptr)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // Initialize OpenSSL
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

SlackOAuth2Server::~SlackOAuth2Server()
{
    stop();

    // Cleanup SSL context
    if (m_pSSLCtx) {
        SSL_CTX_free(static_cast<SSL_CTX*>(m_pSSLCtx));
        m_pSSLCtx = nullptr;
    }

    // Cleanup OpenSSL
    EVP_cleanup();

#ifdef _WIN32
    WSACleanup();
#endif
}

bool SlackOAuth2Server::start()
{
    if (m_bRunning.load()) {
        return true; // Already running
    }

    SAL_WARN("ucb.ucp.slack", "Starting native HTTPS server on port " + OUString::number(m_nPort));

    // Load SSL certificates first
    if (!loadSSLCertificates()) {
        SAL_WARN("ucb.ucp.slack", "Failed to load SSL certificates");
        return false;
    }

    // Create socket
#ifdef _WIN32
    m_nSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nSocketFd == INVALID_SOCKET) {
#else
    m_nSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nSocketFd < 0) {
#endif
        SAL_WARN("ucb.ucp.slack", "Failed to create socket");
        return false;
    }

    // Allow socket reuse
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(m_nSocketFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
#else
    if (setsockopt(m_nSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
#endif
        SAL_WARN("ucb.ucp.slack", "Failed to set socket options");
#ifdef _WIN32
        closesocket(m_nSocketFd);
#else
        close(m_nSocketFd);
#endif
        return false;
    }

    // Bind to localhost
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(m_nPort);

    if (bind(m_nSocketFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to bind socket to port " + OUString::number(m_nPort));
#ifdef _WIN32
        closesocket(m_nSocketFd);
#else
        close(m_nSocketFd);
#endif
        return false;
    }

    // Listen for connections
    if (listen(m_nSocketFd, 1) < 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to listen on socket");
#ifdef _WIN32
        closesocket(m_nSocketFd);
#else
        close(m_nSocketFd);
#endif
        return false;
    }

    m_bRunning = true;
    m_bCodeReceived = false;
    m_sAuthCode.clear();

    // Start HTTPS server thread
    m_pServerThread = std::make_unique<std::thread>(&SlackOAuth2Server::serverLoopHTTPS, this);

    SAL_WARN("ucb.ucp.slack", "HTTPS OAuth2 server started successfully");
    return true;
}

void SlackOAuth2Server::stop()
{
    if (!m_bRunning.load()) {
        return;
    }

    SAL_WARN("ucb.ucp.slack", "Stopping OAuth2 server");

    m_bRunning = false;

    // Close socket to interrupt accept()
    if (m_nSocketFd >= 0) {
#ifdef _WIN32
        closesocket(m_nSocketFd);
#else
        close(m_nSocketFd);
#endif
        m_nSocketFd = -1;
    }

    // Wait for server thread to finish
    if (m_pServerThread) {
        try {
            if (m_pServerThread->joinable()) {
                m_pServerThread->join();
            }
        } catch (const std::exception& e) {
            SAL_WARN("ucb.ucp.slack", "Exception joining OAuth server thread: " << e.what());
        } catch (...) {
            SAL_WARN("ucb.ucp.slack", "Unknown exception joining OAuth server thread");
        }
        m_pServerThread.reset();
    }

    SAL_WARN("ucb.ucp.slack", "OAuth2 server stopped");
}

rtl::OUString SlackOAuth2Server::waitForAuthCode(sal_Int32 timeoutSeconds)
{
    SAL_WARN("ucb.ucp.slack", "Waiting for authorization code (timeout: " + OUString::number(timeoutSeconds) + "s)");

    auto startTime = std::chrono::steady_clock::now();
    auto timeoutDuration = std::chrono::seconds(timeoutSeconds);

    while (m_bRunning.load() && !m_bCodeReceived.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - startTime > timeoutDuration) {
            SAL_WARN("ucb.ucp.slack", "Timeout waiting for authorization code");
            return rtl::OUString();
        }
    }

    if (m_bCodeReceived.load()) {
        SAL_WARN("ucb.ucp.slack", "Authorization code received successfully");
        return m_sAuthCode;
    }

    SAL_WARN("ucb.ucp.slack", "No authorization code received");
    return rtl::OUString();
}

rtl::OUString SlackOAuth2Server::getCallbackURL() const
{
    return rtl::OUString("https://localhost:8080/callback");
}

void SlackOAuth2Server::serverLoop()
{
    SAL_WARN("ucb.ucp.slack", "OAuth2 server loop started");

    while (m_bRunning.load()) {
        struct sockaddr_in clientAddress;
#ifdef _WIN32
        int clientAddrLen = sizeof(clientAddress);
        SOCKET clientSocket = accept(m_nSocketFd, (struct sockaddr*)&clientAddress, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
#else
        socklen_t clientAddrLen = sizeof(clientAddress);
        int clientSocket = accept(m_nSocketFd, (struct sockaddr*)&clientAddress, &clientAddrLen);
        if (clientSocket < 0) {
#endif
            if (m_bRunning.load()) {
                SAL_WARN("ucb.ucp.slack", "Failed to accept connection");
            }
            break;
        }

        // Read HTTP request
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
#ifdef _WIN32
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
#else
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
#endif

        if (bytesRead > 0) {
            rtl::OUString request = rtl::OUString::fromUtf8(buffer);
            SAL_WARN("ucb.ucp.slack", "Received HTTP request");

            // Parse authorization code from request
            rtl::OUString authCode = parseAuthCodeFromRequest(request);

            if (!authCode.isEmpty()) {
                m_sAuthCode = authCode;
                m_bCodeReceived = true;
                SAL_WARN("ucb.ucp.slack", "Authorization code extracted from request");

                // Send success response
                rtl::OUString response = generateSuccessPage();
                std::string responseStr = response.toUtf8().getStr();

#ifdef _WIN32
                send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
#else
                write(clientSocket, responseStr.c_str(), responseStr.length());
#endif
            } else {
                SAL_WARN("ucb.ucp.slack", "No authorization code found in request");

                // Send error response
                std::string errorResponse = "HTTP/1.1 400 Bad Request\r\n"
                                          "Content-Type: text/html\r\n"
                                          "Connection: close\r\n\r\n"
                                          "<html><body><h1>Error</h1><p>No authorization code received.</p></body></html>";

#ifdef _WIN32
                send(clientSocket, errorResponse.c_str(), errorResponse.length(), 0);
#else
                write(clientSocket, errorResponse.c_str(), errorResponse.length());
#endif
            }
        }

        // Close client connection
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif

        if (m_bCodeReceived.load()) {
            break; // Mission accomplished
        }
    }

    SAL_WARN("ucb.ucp.slack", "OAuth2 server loop ended");
}

rtl::OUString SlackOAuth2Server::parseAuthCodeFromRequest(const rtl::OUString& request)
{
    // Convert to std::string for thread-safe parsing
    std::string requestStr = request.toUtf8().getStr();

    // Look for "code=" parameter in the HTTP request
    size_t codePos = requestStr.find("code=");
    if (codePos == std::string::npos) {
        return rtl::OUString();
    }

    size_t startPos = codePos + 5; // Length of "code="
    size_t endPos = requestStr.find("&", startPos);
    if (endPos == std::string::npos) {
        endPos = requestStr.find(" ", startPos); // Space before HTTP/1.1
    }

    if (endPos == std::string::npos) {
        endPos = requestStr.length();
    }

    std::string authCode = requestStr.substr(startPos, endPos - startPos);
    return rtl::OUString::fromUtf8(authCode.c_str());
}

rtl::OUString SlackOAuth2Server::generateSuccessPage()
{
    // Use std::string to avoid thread-safety issues with OUStringBuffer
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Connection: close\r\n\r\n"
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <title>Slack Authorization Successful</title>\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n"
        "        .success { color: #28a745; }\n"
        "        .info { color: #6c757d; margin-top: 20px; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1 class=\"success\">&#x2705; Authorization Successful!</h1>\n"
        "    <p>You have successfully authorized LibreOffice to access your Slack workspace.</p>\n"
        "    <p class=\"info\">You can now close this browser window and return to LibreOffice.</p>\n"
        "    <script>setTimeout(function(){ window.close(); }, 3000);</script>\n"
        "</body>\n"
        "</html>\n";

    return rtl::OUString::fromUtf8(response.c_str());
}

bool SlackOAuth2Server::loadSSLCertificates()
{
    // Create SSL context
    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        SAL_WARN("ucb.ucp.slack", "Failed to create SSL context");
        return false;
    }

    // Set SSL options for better security
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

    // Look for certificates in standard locations
    std::vector<std::string> certPaths = {
        "localhost+1.pem",  // Current directory (development)
        "ucb/source/ucp/slack/certs/localhost+1.pem",  // Relative to project root
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/.local/share/mkcert/localhost+1.pem",  // User directory
        "/usr/local/share/ca-certificates/localhost+1.pem"  // System directory
    };

    std::vector<std::string> keyPaths = {
        "localhost+1-key.pem",
        "ucb/source/ucp/slack/certs/localhost+1-key.pem",
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/.local/share/mkcert/localhost+1-key.pem",
        "/usr/local/share/ca-certificates/localhost+1-key.pem"
    };

    bool certLoaded = false;
    bool keyLoaded = false;

    // Try to load certificate
    for (const auto& certPath : certPaths) {
        if (certPath.empty()) continue;
        std::ifstream certFile(certPath);
        if (certFile.good()) {
            if (SSL_CTX_use_certificate_file(ctx, certPath.c_str(), SSL_FILETYPE_PEM) == 1) {
                SAL_WARN("ucb.ucp.slack", "Loaded SSL certificate from: " + OUString::fromUtf8(certPath.c_str()));
                certLoaded = true;
                break;
            }
        }
    }

    // Try to load private key
    for (const auto& keyPath : keyPaths) {
        if (keyPath.empty()) continue;
        std::ifstream keyFile(keyPath);
        if (keyFile.good()) {
            if (SSL_CTX_use_PrivateKey_file(ctx, keyPath.c_str(), SSL_FILETYPE_PEM) == 1) {
                SAL_WARN("ucb.ucp.slack", "Loaded SSL private key from: " + OUString::fromUtf8(keyPath.c_str()));
                keyLoaded = true;
                break;
            }
        }
    }

    if (!certLoaded || !keyLoaded) {
        SAL_WARN("ucb.ucp.slack", "Failed to load SSL certificate or key. Please run: mkcert localhost 127.0.0.1");
        SSL_CTX_free(ctx);
        return false;
    }

    // Verify that the private key matches the certificate
    if (SSL_CTX_check_private_key(ctx) != 1) {
        SAL_WARN("ucb.ucp.slack", "SSL private key does not match certificate");
        SSL_CTX_free(ctx);
        return false;
    }

    m_pSSLCtx = static_cast<void*>(ctx);
    SAL_WARN("ucb.ucp.slack", "SSL certificates loaded successfully");
    return true;
}

void SlackOAuth2Server::serverLoopHTTPS()
{
    SAL_WARN("ucb.ucp.slack", "HTTPS OAuth2 server loop started");

    SSL_CTX* ctx = static_cast<SSL_CTX*>(m_pSSLCtx);
    if (!ctx) {
        SAL_WARN("ucb.ucp.slack", "SSL context is null");
        return;
    }

    while (m_bRunning.load()) {
        struct sockaddr_in clientAddress;
#ifdef _WIN32
        int clientAddrLen = sizeof(clientAddress);
        SOCKET clientSocket = accept(m_nSocketFd, (struct sockaddr*)&clientAddress, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
#else
        socklen_t clientAddrLen = sizeof(clientAddress);
        int clientSocket = accept(m_nSocketFd, (struct sockaddr*)&clientAddress, &clientAddrLen);
        if (clientSocket < 0) {
#endif
            if (m_bRunning.load()) {
                SAL_WARN("ucb.ucp.slack", "Failed to accept HTTPS connection");
            }
            break;
        }

        // Create SSL connection
        SSL* ssl = SSL_new(ctx);
        if (!ssl) {
            SAL_WARN("ucb.ucp.slack", "Failed to create SSL connection");
#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif
            continue;
        }

        SSL_set_fd(ssl, clientSocket);

        // Perform SSL handshake
        if (SSL_accept(ssl) <= 0) {
            SAL_WARN("ucb.ucp.slack", "SSL handshake failed");
            SSL_free(ssl);
#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif
            continue;
        }

        SAL_WARN("ucb.ucp.slack", "SSL connection established");

        // Read HTTPS request
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);

        if (bytesRead > 0) {
            rtl::OUString request = rtl::OUString::fromUtf8(buffer);
            SAL_WARN("ucb.ucp.slack", "Received HTTPS request");

            // Parse authorization code from request
            rtl::OUString authCode = parseAuthCodeFromRequest(request);

            if (!authCode.isEmpty()) {
                m_sAuthCode = authCode;
                m_bCodeReceived = true;
                SAL_WARN("ucb.ucp.slack", "Authorization code extracted from HTTPS request");

                // Send success response over SSL
                rtl::OUString response = generateSuccessPage();
                std::string responseStr = response.toUtf8().getStr();
                SSL_write(ssl, responseStr.c_str(), responseStr.length());
            } else {
                SAL_WARN("ucb.ucp.slack", "No authorization code found in HTTPS request");

                // Send error response over SSL
                std::string errorResponse = "HTTP/1.1 400 Bad Request\r\n"
                                          "Content-Type: text/html\r\n"
                                          "Connection: close\r\n\r\n"
                                          "<html><body><h1>Error</h1><p>No authorization code received.</p></body></html>";
                SSL_write(ssl, errorResponse.c_str(), errorResponse.length());
            }
        }

        // Clean up SSL connection
        SSL_shutdown(ssl);
        SSL_free(ssl);

        // Close client connection
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif

        if (m_bCodeReceived.load()) {
            break; // Mission accomplished
        }
    }

    SAL_WARN("ucb.ucp.slack", "HTTPS OAuth2 server loop ended");
}

} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
