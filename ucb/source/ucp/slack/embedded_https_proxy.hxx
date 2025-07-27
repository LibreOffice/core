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
#include <string>

namespace ucp {
namespace slack {

/**
 * Embedded HTTPS proxy for OAuth2 callback
 * Acts as HTTPS server on port 8080, forwards to HTTP server on port 8081
 * This eliminates the need for external Python proxy script
 */
class EmbeddedHttpsProxy
{
public:
    EmbeddedHttpsProxy();
    ~EmbeddedHttpsProxy();

    // Start the proxy and return true if successful
    bool start();

    // Stop the proxy
    void stop();

    // Get the port the proxy is listening on
    sal_Int32 getPort() const { return m_nPort; }

    // Set the backend HTTP server port
    void setBackendPort(sal_Int32 port) { m_nBackendPort = port; }

private:
    void proxyLoop();
    std::string forwardToBackend(const std::string& request);

    std::atomic<bool> m_bRunning;
    sal_Int32 m_nPort;          // HTTPS proxy port (8080)
    sal_Int32 m_nBackendPort;   // HTTP backend port (8081)
    sal_Int32 m_nSocketFd;
    std::unique_ptr<std::thread> m_pProxyThread;
};

} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
