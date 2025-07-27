/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "NativeHTTPSServer.hxx"
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>

#include "ssl_openssl.hxx"
#include "ssl_securetransport.hxx"
#include "ssl_schannel.hxx"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <cstring>
#include <sstream>
#include <regex>

namespace ucp {
namespace slack {
namespace https {

// HTTPRequest implementation
rtl::OUString HTTPRequest::getAuthorizationCode() const
{
    std::string queryStr = queryString.toUtf8().getStr();

    SAL_WARN("ucb.ucp.slack", "Parsing auth code from query string: '" << queryStr.c_str() << "'");

    // Look for "code=" parameter (can be at start or after & or ?)
    std::regex codeRegex(R"((?:^|[&?])code=([^&\s]+))");
    std::smatch match;

    if (std::regex_search(queryStr, match, codeRegex) && match.size() > 1) {
        std::string authCode = match[1].str();
        SAL_WARN("ucb.ucp.slack", "Found authorization code: '" << authCode.c_str() << "'");
        return rtl::OUString::fromUtf8(authCode.c_str());
    }

    SAL_WARN("ucb.ucp.slack", "No authorization code found in query string");
    return rtl::OUString();
}

// HTTPResponse implementation
rtl::OUString HTTPResponse::toHTTPResponse() const
{
    rtl::OUStringBuffer response;
    response.append("HTTP/1.1 ");
    response.append(statusCode);
    response.append(" ");
    response.append(statusText);
    response.append("\r\n");
    response.append("Content-Type: ");
    response.append(contentType);
    response.append("\r\n");
    response.append("Content-Length: ");
    response.append(static_cast<sal_Int32>(body.getLength()));
    response.append("\r\n");
    response.append("Connection: close\r\n");
    response.append("Server: LibreOffice-OAuth\r\n");
    response.append("\r\n");
    response.append(body);

    return response.makeStringAndClear();
}

// NativeHTTPSServer implementation
NativeHTTPSServer::NativeHTTPSServer()
    : m_bRunning(false)
    , m_nPort(8080)
    , m_nServerSocket(-1)
    , m_bAuthCodeReceived(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // Set default request handler
    m_requestHandler = [this](const HTTPRequest& request) {
        return defaultRequestHandler(request);
    };
}

NativeHTTPSServer::~NativeHTTPSServer()
{
    stop();

#ifdef _WIN32
    WSACleanup();
#endif
}

bool NativeHTTPSServer::start(sal_Int32 port)
{
    if (m_bRunning.load()) {
        return true; // Already running
    }

    m_nPort = port;

    SAL_WARN("ucb.ucp.slack", "Starting native HTTPS server on port " << m_nPort);

    // Create SSL context
    m_pSSLContext = createSSLContext();
    if (!m_pSSLContext || !m_pSSLContext->initialize()) {
        SAL_WARN("ucb.ucp.slack", "Failed to initialize SSL context");
        return false;
    }

    // Create server socket
#ifdef _WIN32
    m_nServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nServerSocket == INVALID_SOCKET) {
#else
    m_nServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nServerSocket < 0) {
#endif
        SAL_WARN("ucb.ucp.slack", "Failed to create server socket");
        return false;
    }

    // Set socket options
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(m_nServerSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
#else
    if (setsockopt(m_nServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
#endif
        SAL_WARN("ucb.ucp.slack", "Failed to set socket options");
#ifdef _WIN32
        closesocket(m_nServerSocket);
#else
        close(m_nServerSocket);
#endif
        return false;
    }

    // Bind to localhost
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(m_nPort);

    if (bind(m_nServerSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to bind to port " << m_nPort);
#ifdef _WIN32
        closesocket(m_nServerSocket);
#else
        close(m_nServerSocket);
#endif
        return false;
    }

    // Listen for connections
    if (listen(m_nServerSocket, 5) < 0) {
        SAL_WARN("ucb.ucp.slack", "Failed to listen on socket");
#ifdef _WIN32
        closesocket(m_nServerSocket);
#else
        close(m_nServerSocket);
#endif
        return false;
    }

    m_bRunning = true;
    m_bAuthCodeReceived = false;
    m_sAuthorizationCode.clear();

    // Start server thread
    m_pServerThread = std::make_unique<std::thread>(&NativeHTTPSServer::serverLoop, this);

    SAL_WARN("ucb.ucp.slack", "Native HTTPS server started successfully");
    return true;
}

void NativeHTTPSServer::stop()
{
    if (!m_bRunning.load()) {
        return;
    }

    SAL_WARN("ucb.ucp.slack", "Stopping native HTTPS server");

    m_bRunning = false;

    // Close server socket
    if (m_nServerSocket >= 0) {
#ifdef _WIN32
        closesocket(m_nServerSocket);
#else
        close(m_nServerSocket);
#endif
        m_nServerSocket = -1;
    }

    // Wait for server thread
    if (m_pServerThread && m_pServerThread->joinable()) {
        m_pServerThread->join();
    }
    m_pServerThread.reset();

    // Cleanup SSL context
    m_pSSLContext.reset();

    SAL_WARN("ucb.ucp.slack", "Native HTTPS server stopped");
}

rtl::OUString NativeHTTPSServer::getCallbackURL() const
{
    return rtl::OUString("https://localhost:") + rtl::OUString::number(m_nPort) + "/callback";
}

std::unique_ptr<SSLContext> NativeHTTPSServer::createSSLContext()
{
    // For initial implementation, use OpenSSL on all platforms
    // This provides consistent behavior and avoids platform-specific SSL issues
    // Platform-specific implementations can be enabled later after testing

#ifdef _WIN32
    // TODO: Enable SChannel after testing
    return std::make_unique<OpenSSLContext>();
#elif __APPLE__
    // TODO: Enable SecureTransport after fixing certificate generation
    return std::make_unique<OpenSSLContext>();
#else
    return std::make_unique<OpenSSLContext>();
#endif
}

void NativeHTTPSServer::serverLoop()
{
    SAL_WARN("ucb.ucp.slack", "HTTPS server loop started");

    while (m_bRunning.load()) {
        struct sockaddr_in clientAddress;
#ifdef _WIN32
        int clientAddrLen = sizeof(clientAddress);
        SOCKET clientSocket = accept(m_nServerSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
#else
        socklen_t clientAddrLen = sizeof(clientAddress);
        int clientSocket = accept(m_nServerSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);
        if (clientSocket < 0) {
#endif
            if (m_bRunning.load()) {
                SAL_WARN("ucb.ucp.slack", "Failed to accept connection");
            }
            break;
        }

        // Handle connection in separate thread or inline
        std::thread connectionThread(&NativeHTTPSServer::handleConnection, this, clientSocket);
        connectionThread.detach(); // Let it run independently
    }

    SAL_WARN("ucb.ucp.slack", "HTTPS server loop ended");
}

void NativeHTTPSServer::handleConnection(int clientSocket)
{
    // Create SSL session from existing SSL context
    auto sslContext = createSSLContext();
    if (!sslContext || !sslContext->initialize() || !sslContext->createSSLSocket(clientSocket)) {
        SAL_WARN("ucb.ucp.slack", "Failed to create SSL socket");
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
        return;
    }

    // Perform SSL handshake
    if (!sslContext->performHandshake()) {
        SAL_WARN("ucb.ucp.slack", "SSL handshake failed");
        sslContext->cleanup();
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
        return;
    }

    // Read HTTP request
    char buffer[4096];
    int bytesRead = sslContext->readSSL(buffer, sizeof(buffer) - 1);

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::string rawRequest(buffer);

        // Parse HTTP request
        HTTPRequest request = parseHTTPRequest(rawRequest);

        // Handle request
        HTTPResponse response = m_requestHandler(request);

        // Send response
        rtl::OUString responseStr = response.toHTTPResponse();
        std::string responseUtf8 = responseStr.toUtf8().getStr();

        sslContext->writeSSL(responseUtf8.c_str(), responseUtf8.length());
    }

    // Cleanup
    sslContext->cleanup();
#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif
}

HTTPRequest NativeHTTPSServer::parseHTTPRequest(const std::string& rawData)
{
    HTTPRequest request;

    std::istringstream stream(rawData);
    std::string line;

    // Parse request line
    if (std::getline(stream, line)) {
        std::istringstream requestLine(line);
        std::string method, uri, version;
        requestLine >> method >> uri >> version;

        request.method = rtl::OUString::fromUtf8(method.c_str());

        // Split URI into path and query
        size_t queryPos = uri.find('?');
        if (queryPos != std::string::npos) {
            request.path = rtl::OUString::fromUtf8(uri.substr(0, queryPos).c_str());
            request.queryString = rtl::OUString::fromUtf8(uri.substr(queryPos + 1).c_str());
        } else {
            request.path = rtl::OUString::fromUtf8(uri.c_str());
        }
    }

    // Parse headers (simplified)
    rtl::OUStringBuffer headers;
    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        headers.append(rtl::OUString::fromUtf8(line.c_str()));
        headers.append("\n");
    }
    request.headers = headers.makeStringAndClear();

    // Parse body (if any)
    std::string body;
    while (std::getline(stream, line)) {
        body += line + "\n";
    }
    request.body = rtl::OUString::fromUtf8(body.c_str());

    return request;
}

HTTPResponse NativeHTTPSServer::defaultRequestHandler(const HTTPRequest& request)
{
    SAL_WARN("ucb.ucp.slack", "Handling request for path: " << request.path);
    SAL_WARN("ucb.ucp.slack", "Query string: " << request.queryString);
    SAL_WARN("ucb.ucp.slack", "Method: " << request.method);

    if (request.path == "/callback" || request.path == "/auth" || request.path == "/") {
        // OAuth callback - accept multiple paths
        rtl::OUString authCode = request.getAuthorizationCode();

        if (!authCode.isEmpty()) {
            m_sAuthorizationCode = authCode;
            m_bAuthCodeReceived = true;
            SAL_WARN("ucb.ucp.slack", "Authorization code received: " << authCode);
            return generateSuccessPage();
        } else {
            SAL_WARN("ucb.ucp.slack", "No authorization code found. Path: " << request.path << ", Query: " << request.queryString);
            return generateErrorPage("No authorization code found in request");
        }
    }

    // Default response for other paths
    HTTPResponse response;
    response.statusCode = 404;
    response.statusText = "Not Found";
    response.body = "<html><body><h1>404 Not Found</h1></body></html>";
    return response;
}

HTTPResponse NativeHTTPSServer::generateSuccessPage() const
{
    HTTPResponse response;
    response.body =
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

    return response;
}

HTTPResponse NativeHTTPSServer::generateErrorPage(const rtl::OUString& error) const
{
    HTTPResponse response;
    response.statusCode = 400;
    response.statusText = "Bad Request";
    response.body =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <title>Authorization Error</title>\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n"
        "        .error { color: #dc3545; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1 class=\"error\">&#x274C; Authorization Failed</h1>\n"
        "    <p>Error: " + error + "</p>\n"
        "    <p>Please try again or contact support.</p>\n"
        "</body>\n"
        "</html>\n";

    return response;
}

} // namespace https
} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
