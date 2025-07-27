/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oauth2_http_server.hxx"
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

#include <cstring>
#include <chrono>

namespace ucp {
namespace dropbox {

OAuth2HttpServer::OAuth2HttpServer()
    : m_bRunning(false)
    , m_bCodeReceived(false)
    , m_nPort(8080)
    , m_nSocketFd(-1)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

OAuth2HttpServer::~OAuth2HttpServer()
{
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool OAuth2HttpServer::start()
{
    if (m_bRunning.load()) {
        return true; // Already running
    }

    SAL_WARN("ucb.ucp.dropbox", "Starting OAuth2 HTTP server on port " + OUString::number(m_nPort));

    // Create socket
#ifdef _WIN32
    m_nSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nSocketFd == INVALID_SOCKET) {
#else
    m_nSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nSocketFd < 0) {
#endif
        SAL_WARN("ucb.ucp.dropbox", "Failed to create socket");
        return false;
    }

    // Allow socket reuse
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(m_nSocketFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
#else
    if (setsockopt(m_nSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
#endif
        SAL_WARN("ucb.ucp.dropbox", "Failed to set socket options");
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
        SAL_WARN("ucb.ucp.dropbox", "Failed to bind socket to port " + OUString::number(m_nPort));
#ifdef _WIN32
        closesocket(m_nSocketFd);
#else
        close(m_nSocketFd);
#endif
        return false;
    }

    // Listen for connections
    if (listen(m_nSocketFd, 3) < 0) {
        SAL_WARN("ucb.ucp.dropbox", "Failed to listen on socket");
#ifdef _WIN32
        closesocket(m_nSocketFd);
#else
        close(m_nSocketFd);
#endif
        return false;
    }

    // Start server thread
    m_bRunning = true;
    m_bCodeReceived = false;
    m_sAuthCode = rtl::OUString();

    try {
        m_pServerThread = std::make_unique<std::thread>(&OAuth2HttpServer::serverLoop, this);
        SAL_WARN("ucb.ucp.dropbox", "OAuth2 HTTP server started successfully on port " + OUString::number(m_nPort));
        return true;
    } catch (...) {
        SAL_WARN("ucb.ucp.dropbox", "Failed to start server thread");
        m_bRunning = false;
#ifdef _WIN32
        closesocket(m_nSocketFd);
#else
        close(m_nSocketFd);
#endif
        return false;
    }
}

void OAuth2HttpServer::stop()
{
    if (!m_bRunning.load()) {
        return;
    }

    SAL_WARN("ucb.ucp.dropbox", "Stopping OAuth2 HTTP server");

    m_bRunning = false;

    // Close socket to break out of accept()
    if (m_nSocketFd >= 0) {
#ifdef _WIN32
        closesocket(m_nSocketFd);
#else
        close(m_nSocketFd);
#endif
        m_nSocketFd = -1;
    }

    // Wait for server thread to finish
    if (m_pServerThread && m_pServerThread->joinable()) {
        m_pServerThread->join();
    }
    m_pServerThread.reset();

    SAL_WARN("ucb.ucp.dropbox", "OAuth2 HTTP server stopped");
}

rtl::OUString OAuth2HttpServer::waitForAuthCode(sal_Int32 timeoutSeconds)
{
    SAL_WARN("ucb.ucp.dropbox", "Waiting for OAuth2 authorization code (timeout: " + OUString::number(timeoutSeconds) + " seconds)");

    auto startTime = std::chrono::steady_clock::now();

    while (!m_bCodeReceived.load() && m_bRunning.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();

        if (elapsed >= timeoutSeconds) {
            SAL_WARN("ucb.ucp.dropbox", "Timeout waiting for authorization code");
            return rtl::OUString();
        }
    }

    if (m_bCodeReceived.load()) {
        SAL_WARN("ucb.ucp.dropbox", "Authorization code received: " + m_sAuthCode.copy(0, 10) + "...");
        return m_sAuthCode;
    }

    SAL_WARN("ucb.ucp.dropbox", "Server stopped before receiving authorization code");
    return rtl::OUString();
}

void OAuth2HttpServer::serverLoop()
{
    SAL_WARN("ucb.ucp.dropbox", "OAuth2 server thread started");

    while (m_bRunning.load()) {
        struct sockaddr_in client_addr;
#ifdef _WIN32
        int client_len = sizeof(client_addr);
        int client_socket = accept(m_nSocketFd, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
#else
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(m_nSocketFd, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
#endif
            if (m_bRunning.load()) {
                SAL_WARN("ucb.ucp.dropbox", "Accept failed");
            }
            break;
        }

        // Read the HTTP request
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));

#ifdef _WIN32
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
#else
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
#endif

        if (bytes_read > 0) {
            rtl::OUString request = rtl::OUString::createFromAscii(buffer);
            SAL_WARN("ucb.ucp.dropbox", "Received HTTP request: " + request.copy(0, 200));

            // Parse authorization code from request
            rtl::OUString authCode = parseAuthCodeFromRequest(request);

            // Send HTTP response
            std::string response;
            if (!authCode.isEmpty()) {
                m_sAuthCode = authCode;
                m_bCodeReceived = true;

                response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "Connection: close\r\n"
                          "\r\n"
                          "<!DOCTYPE html><html><head><title>Authorization Successful</title></head>"
                          "<body><h1>Authorization Successful!</h1>"
                          "<p>You can now close this window and return to LibreOffice.</p>"
                          "<script>setTimeout(function(){window.close();}, 3000);</script>"
                          "</body></html>";

                SAL_WARN("ucb.ucp.dropbox", "Authorization code extracted successfully");
            } else {
                response = "HTTP/1.1 400 Bad Request\r\n"
                          "Content-Type: text/html\r\n"
                          "Connection: close\r\n"
                          "\r\n"
                          "<!DOCTYPE html><html><head><title>Authorization Failed</title></head>"
                          "<body><h1>Authorization Failed</h1>"
                          "<p>No authorization code found in the request.</p>"
                          "</body></html>";

                SAL_WARN("ucb.ucp.dropbox", "No authorization code found in request");
            }

#ifdef _WIN32
            send(client_socket, response.c_str(), response.length(), 0);
            closesocket(client_socket);
#else
            write(client_socket, response.c_str(), response.length());
            close(client_socket);
#endif

            // If we got the code, we can stop the server
            if (m_bCodeReceived.load()) {
                SAL_WARN("ucb.ucp.dropbox", "Authorization code received, stopping server");
                break;
            }
        } else {
#ifdef _WIN32
            closesocket(client_socket);
#else
            close(client_socket);
#endif
        }
    }

    SAL_WARN("ucb.ucp.dropbox", "OAuth2 server thread exiting");
}

rtl::OUString OAuth2HttpServer::parseAuthCodeFromRequest(const rtl::OUString& request)
{
    // Look for: GET /callback?code=AUTHORIZATION_CODE&...
    sal_Int32 codePos = request.indexOf(u"code=");
    if (codePos == -1) {
        return rtl::OUString();
    }

    sal_Int32 codeStart = codePos + 5; // Length of "code="
    sal_Int32 codeEnd = request.indexOf(u"&", codeStart);
    if (codeEnd == -1) {
        codeEnd = request.indexOf(u" ", codeStart); // End of URL in HTTP request
    }
    if (codeEnd == -1) {
        codeEnd = request.getLength();
    }

    if (codeEnd > codeStart) {
        rtl::OUString authCode = request.copy(codeStart, codeEnd - codeStart);
        SAL_WARN("ucb.ucp.dropbox", "Extracted authorization code: " + authCode.copy(0, 10) + "...");
        return authCode;
    }

    return rtl::OUString();
}

} // namespace dropbox
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */