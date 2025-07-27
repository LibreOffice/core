/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <memory>
#include <atomic>
#include <thread>
#include <functional>

namespace ucp {
namespace slack {
namespace https {

/**
 * Abstract base class for platform-specific SSL implementations
 */
class SSLContext
{
public:
    virtual ~SSLContext() = default;

    // Initialize SSL context and generate localhost certificate
    virtual bool initialize() = 0;

    // Create SSL socket from regular socket
    virtual bool createSSLSocket(int socket) = 0;

    // Perform SSL handshake
    virtual bool performHandshake() = 0;

    // Read encrypted data
    virtual int readSSL(void* buffer, size_t length) = 0;

    // Write encrypted data
    virtual int writeSSL(const void* buffer, size_t length) = 0;

    // Cleanup SSL connection
    virtual void cleanup() = 0;
};

/**
 * HTTP request structure
 */
struct HTTPRequest
{
    rtl::OUString method;
    rtl::OUString path;
    rtl::OUString queryString;
    rtl::OUString headers;
    rtl::OUString body;

    // Parse authorization code from OAuth callback
    rtl::OUString getAuthorizationCode() const;
};

/**
 * HTTP response structure
 */
struct HTTPResponse
{
    int statusCode = 200;
    rtl::OUString statusText = "OK";
    rtl::OUString contentType = "text/html; charset=UTF-8";
    rtl::OUString body;

    // Generate full HTTP response
    rtl::OUString toHTTPResponse() const;
};

/**
 * Native HTTPS server for OAuth callbacks
 * Cross-platform implementation using platform-specific SSL APIs
 */
class NativeHTTPSServer
{
public:
    using RequestHandler = std::function<HTTPResponse(const HTTPRequest&)>;

    NativeHTTPSServer();
    ~NativeHTTPSServer();

    // Start the HTTPS server on specified port
    bool start(sal_Int32 port = 8080);

    // Stop the server
    void stop();

    // Check if server is running
    bool isRunning() const { return m_bRunning.load(); }

    // Get the port the server is listening on
    sal_Int32 getPort() const { return m_nPort; }

    // Set custom request handler
    void setRequestHandler(RequestHandler handler) { m_requestHandler = handler; }

    // Get callback URL
    rtl::OUString getCallbackURL() const;

private:
    // Create platform-specific SSL context
    std::unique_ptr<SSLContext> createSSLContext();

    // Server main loop
    void serverLoop();

    // Handle individual client connection
    void handleConnection(int clientSocket);

    // Parse HTTP request from raw data
    HTTPRequest parseHTTPRequest(const std::string& rawData);

    // Default request handler for OAuth callback
    HTTPResponse defaultRequestHandler(const HTTPRequest& request);

    // Generate success page for OAuth completion
    HTTPResponse generateSuccessPage() const;

    // Generate error page
    HTTPResponse generateErrorPage(const rtl::OUString& error) const;

private:
    std::atomic<bool> m_bRunning;
    sal_Int32 m_nPort;
    int m_nServerSocket;
    std::unique_ptr<std::thread> m_pServerThread;
    std::unique_ptr<SSLContext> m_pSSLContext;
    RequestHandler m_requestHandler;

    // OAuth state
    std::atomic<bool> m_bAuthCodeReceived;
    rtl::OUString m_sAuthorizationCode;
};

} // namespace https
} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
