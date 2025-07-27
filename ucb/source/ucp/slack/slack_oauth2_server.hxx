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
#include <thread>
#include <atomic>
#include <memory>

namespace ucp {
namespace slack {

/**
 * Simple HTTP server to listen for OAuth2 callback from Slack
 * Listens on localhost:8080/callback for the authorization code
 */
class SlackOAuth2Server
{
public:
    SlackOAuth2Server();
    ~SlackOAuth2Server();

    // Start the server and return true if successful
    bool start();

    // Stop the server
    void stop();

    // Wait for authorization code (blocks until received or timeout)
    // Returns the authorization code or empty string on timeout/error
    rtl::OUString waitForAuthCode(sal_Int32 timeoutSeconds = 120);

    // Get the port the server is listening on
    sal_Int32 getPort() const { return m_nPort; }

    // Get the full callback URL
    rtl::OUString getCallbackURL() const;

private:
    void serverLoop();
    rtl::OUString parseAuthCodeFromRequest(const rtl::OUString& request);
    rtl::OUString generateSuccessPage();

    std::atomic<bool> m_bRunning;
    std::atomic<bool> m_bCodeReceived;
    sal_Int32 m_nPort;
    sal_Int32 m_nSocketFd;
    rtl::OUString m_sAuthCode;
    std::unique_ptr<std::thread> m_pServerThread;
};

} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
