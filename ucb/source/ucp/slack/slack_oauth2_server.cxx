/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "slack_oauth2_server.hxx"
#include "native_https/NativeHTTPSServer.hxx"
#include <config_oauth2.h>
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>

#include <cstring>
#include <chrono>
#include <thread>

namespace ucp {
namespace slack {

SlackOAuth2Server::SlackOAuth2Server()
    : m_bRunning(false)
    , m_bCodeReceived(false)
{
}

SlackOAuth2Server::~SlackOAuth2Server()
{
    stop();
}

bool SlackOAuth2Server::start()
{
    if (m_bRunning.load()) {
        return true; // Already running
    }

    SAL_WARN("ucb.ucp.slack", "Starting Slack OAuth2 server using native HTTPS");

    // Create native HTTPS server
    m_pHttpsServer = std::make_unique<https::NativeHTTPSServer>();

    // Set custom request handler for OAuth callback
    m_pHttpsServer->setRequestHandler([this](const https::HTTPRequest& request) -> https::HTTPResponse {
        SAL_WARN("ucb.ucp.slack", "OAuth server received request - Method: " << request.method << ", Path: " << request.path << ", Query: " << request.queryString);

        if (request.path == "/callback") {
            rtl::OUString authCode = request.getAuthorizationCode();
            if (!authCode.isEmpty()) {
                handleOAuthCallback(authCode);

                // Return success page
                https::HTTPResponse response;
                rtl::OUStringBuffer htmlBuffer;
                htmlBuffer.append("<!DOCTYPE html>\n");
                htmlBuffer.append("<html>\n");
                htmlBuffer.append("<head>\n");
                htmlBuffer.append("    <meta charset=\"UTF-8\">\n");
                htmlBuffer.append("    <title>Slack Authorization Successful</title>\n");
                htmlBuffer.append("    <style>\n");
                htmlBuffer.append("        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n");
                htmlBuffer.append("        .success { color: #28a745; font-size: 24px; }\n");
                htmlBuffer.append("    </style>\n");
                htmlBuffer.append("</head>\n");
                htmlBuffer.append("<body>\n");
                htmlBuffer.append("    <h1 class=\"success\">Slack Authorization Successful!</h1>\n");
                htmlBuffer.append("    <p>You can now close this window and return to LibreOffice.</p>\n");
                htmlBuffer.append("    <script>setTimeout(function(){ window.close(); }, 3000);</script>\n");
                htmlBuffer.append("</body>\n");
                htmlBuffer.append("</html>\n");
                response.body = htmlBuffer.makeStringAndClear();
                return response;
            } else {
                // Return error page
                https::HTTPResponse response;
                response.statusCode = 400;
                response.statusText = "Bad Request";
                rtl::OUStringBuffer errorBuffer;
                errorBuffer.append("<!DOCTYPE html>\n");
                errorBuffer.append("<html>\n");
                errorBuffer.append("<head>\n");
                errorBuffer.append("    <meta charset=\"UTF-8\">\n");
                errorBuffer.append("    <title>Authorization Error</title>\n");
                errorBuffer.append("    <style>\n");
                errorBuffer.append("        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n");
                errorBuffer.append("        .error { color: #dc3545; }\n");
                errorBuffer.append("    </style>\n");
                errorBuffer.append("</head>\n");
                errorBuffer.append("<body>\n");
                errorBuffer.append("    <h1 class=\"error\">Authorization Failed</h1>\n");
                errorBuffer.append("    <p>Error: No authorization code found in request</p>\n");
                errorBuffer.append("    <p>Please try again or contact support.</p>\n");
                errorBuffer.append("</body>\n");
                errorBuffer.append("</html>\n");
                response.body = errorBuffer.makeStringAndClear();
                return response;
            }
        }

        // Default 404 response
        SAL_WARN("ucb.ucp.slack", "Request did not match /callback path. Returning 404.");
        https::HTTPResponse response;
        response.statusCode = 404;
        response.statusText = "Not Found";
        response.body = "<html><body><h1>404 Not Found</h1><p>Expected path: /callback</p><p>Received path: " + request.path + "</p></body></html>";
        return response;
    });

    // Start the HTTPS server
    if (!m_pHttpsServer->start(8080)) {
        SAL_WARN("ucb.ucp.slack", "Failed to start native HTTPS server");
        m_pHttpsServer.reset();
        return false;
    }

    m_bRunning = true;
    m_bCodeReceived = false;
    m_sAuthCode.clear();

    SAL_WARN("ucb.ucp.slack", "Native HTTPS OAuth2 server started successfully");
    return true;
}

void SlackOAuth2Server::stop()
{
    if (!m_bRunning.load()) {
        return;
    }

    SAL_WARN("ucb.ucp.slack", "Stopping OAuth2 server");

    m_bRunning = false;

    // Stop native HTTPS server
    if (m_pHttpsServer) {
        m_pHttpsServer->stop();
        m_pHttpsServer.reset();
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

sal_Int32 SlackOAuth2Server::getPort() const
{
    if (m_pHttpsServer) {
        return m_pHttpsServer->getPort();
    }
    return 8080; // Default port
}

rtl::OUString SlackOAuth2Server::getCallbackURL() const
{
    if (m_pHttpsServer) {
        return m_pHttpsServer->getCallbackURL();
    }
    return rtl::OUString("https://localhost:8080/callback");
}

void SlackOAuth2Server::handleOAuthCallback(const rtl::OUString& authCode)
{
    SAL_WARN("ucb.ucp.slack", "OAuth callback received with authorization code");

    m_sAuthCode = authCode;
    m_bCodeReceived = true;
}



} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
