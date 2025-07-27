/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "embedded_https_proxy.hxx"
#include <sal/log.hxx>
#include <curl/curl.h>
#include <thread>
#include <chrono>

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
#include <string>
#include <sstream>

namespace ucp {
namespace slack {

struct WriteCallbackData {
    std::string response;
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, WriteCallbackData* data) {
    size_t totalSize = size * nmemb;
    data->response.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

EmbeddedHttpsProxy::EmbeddedHttpsProxy()
    : m_bRunning(false)
    , m_nPort(8080)
    , m_nSocketFd(-1)
    , m_nBackendPort(8081)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

EmbeddedHttpsProxy::~EmbeddedHttpsProxy()
{
    stop();
    curl_global_cleanup();

#ifdef _WIN32
    WSACleanup();
#endif
}

bool EmbeddedHttpsProxy::start()
{
    if (m_bRunning.load()) {
        return true; // Already running
    }

    SAL_WARN("ucb.ucp.slack", "Starting embedded HTTPS proxy on port " + OUString::number(m_nPort));
    SAL_WARN("ucb.ucp.slack", "Forwarding to HTTP backend on port " + OUString::number(m_nBackendPort));

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

    // Start proxy thread
    m_pProxyThread = std::make_unique<std::thread>(&EmbeddedHttpsProxy::proxyLoop, this);

    SAL_WARN("ucb.ucp.slack", "Embedded HTTPS proxy started successfully");
    return true;
}

void EmbeddedHttpsProxy::stop()
{
    if (!m_bRunning.load()) {
        return;
    }

    SAL_WARN("ucb.ucp.slack", "Stopping embedded HTTPS proxy");

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

    // Wait for proxy thread to finish
    if (m_pProxyThread) {
        try {
            if (m_pProxyThread->joinable()) {
                m_pProxyThread->join();
            }
        } catch (const std::exception& e) {
            SAL_WARN("ucb.ucp.slack", "Exception joining proxy thread: " << e.what());
        } catch (...) {
            SAL_WARN("ucb.ucp.slack", "Unknown exception joining proxy thread");
        }
        m_pProxyThread.reset();
    }

    SAL_WARN("ucb.ucp.slack", "Embedded HTTPS proxy stopped");
}

void EmbeddedHttpsProxy::proxyLoop()
{
    SAL_WARN("ucb.ucp.slack", "HTTPS proxy loop started");

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
            std::string request(buffer);
            SAL_WARN("ucb.ucp.slack", "Received HTTPS request, forwarding to backend");

            // Forward request to backend HTTP server using curl
            std::string response = forwardToBackend(request);

            if (!response.empty()) {
                SAL_WARN("ucb.ucp.slack", "Forwarding response back to client");
                // Send response back to client
#ifdef _WIN32
                send(clientSocket, response.c_str(), response.length(), 0);
#else
                write(clientSocket, response.c_str(), response.length());
#endif
            } else {
                SAL_WARN("ucb.ucp.slack", "No response from backend, sending error");
                // Send error response
                std::string errorResponse = "HTTP/1.1 502 Bad Gateway\r\n"
                                          "Content-Type: text/html\r\n"
                                          "Connection: close\r\n\r\n"
                                          "<html><body><h1>Proxy Error</h1><p>Backend server unavailable.</p></body></html>";
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
    }

    SAL_WARN("ucb.ucp.slack", "HTTPS proxy loop ended");
}

std::string EmbeddedHttpsProxy::forwardToBackend(const std::string& request)
{
    // Extract path from HTTP request
    std::string path = "/callback";
    size_t getPos = request.find("GET ");
    if (getPos != std::string::npos) {
        size_t pathStart = getPos + 4;
        size_t pathEnd = request.find(" HTTP/", pathStart);
        if (pathEnd != std::string::npos) {
            path = request.substr(pathStart, pathEnd - pathStart);
        }
    }

    // Construct backend URL
    std::string backendUrl = "http://localhost:" + std::to_string(m_nBackendPort) + path;

    CURL* curl = curl_easy_init();
    if (!curl) {
        SAL_WARN("ucb.ucp.slack", "Failed to initialize curl");
        return "";
    }

    WriteCallbackData responseData;

    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, backendUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.slack", "Curl request failed: " << curl_easy_strerror(res));
        return "";
    }

    return responseData.response;
}

} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
