/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Network-related utility functions and helper classes.
 * Classes: net::HostEntry, net::DefaultValues
 * Functions: connect(), resolve(), localhostIPv4/IPv6()
 */

#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <memory>
#include <string_view>
#include <vector>

// This file hosts network related common functionality
// and helper/utility functions and classes.
// HTTP-specific helpers are in HttpHelper.hpp.

class StreamSocket;
class ProtocolHandlerInterface;
struct addrinfo;
struct sockaddr;

namespace net
{

/// Initial values for DefaultValues. Tests that mutate net::Defaults
/// should reset to these rather than hard-code a literal.
constexpr std::chrono::seconds DefaultInactivityTimeout = std::chrono::seconds(3600);
constexpr size_t DefaultMaxExtConnections = 200000;

class DefaultValues
{
public:
    /// StreamSocket inactivity timeout in us (DefaultInactivityTimeout default). Zero disables instrument.
    std::chrono::microseconds inactivityTimeout;

    /// Maximum number of concurrent external TCP connections. Zero disables instrument,
    /// limiting the maximum number of connections by the available sockets to the system.
    size_t maxExtConnections;
};
extern DefaultValues Defaults;

class HostEntry
{
    std::string _requestName;
    std::string _canonicalName;
    std::vector<std::string> _ipAddresses;
    std::shared_ptr<addrinfo> _ainfo;
    int _saved_errno;
    int _eaino;

    void setEAI(int eaino);

    std::string makeIPAddress(const sockaddr* ai_addr);

public:
    explicit HostEntry(std::string desc);
    HostEntry(const HostEntry&) = default;
    HostEntry(HostEntry&&) = default;
    HostEntry& operator=(const HostEntry&) = default;
    HostEntry& operator=(HostEntry&&) = default;
    ~HostEntry();

    bool good() const { return _saved_errno == 0 && _eaino == 0; }
    std::string errorMessage() const;

    const std::string& getCanonicalName() const { return  _canonicalName; }
    const std::vector<std::string>& getAddresses() const { return  _ipAddresses; }
    const addrinfo* getAddrInfo() const { return _ainfo.get(); }

    std::string resolveHostAddress() const;
    bool isLocalhost() const;
};

#if !MOBILEAPP

/// Resolves the IP of the given hostname. On failure, returns @targetHost.
std::string resolveHostAddress(const std::string& targetHost);

/// Returns true if @targetHost is on the same host.
bool isLocalhost(const std::string& targetHost);

/// Returns the canonical host name of the given IP address or host name.
std::string canonicalHostName(const std::string& addressToCheck);

/// Returns a vector containing the IPAddresses for the host.
std::vector<std::string> resolveAddresses(const std::string& addressToCheck);

#endif

/// Connect to an end-point at the given host and port and return StreamSocket.
std::shared_ptr<StreamSocket>
connect(const std::string& host, const std::string& port, bool isSSL,
        const std::shared_ptr<ProtocolHandlerInterface>& protocolHandler);

enum class AsyncConnectResult : std::uint8_t {
    Ok = 0,
    SocketError,
    ConnectionError,
    HostNameError,
    UnknownHostError,
    SSLHandShakeFailure,
    MissingSSLError
};

using asyncConnectCB =
    std::function<void(std::shared_ptr<StreamSocket>, AsyncConnectResult result)>;

void asyncConnect(std::string host, const std::string& port, bool isSSL,
                  const std::shared_ptr<ProtocolHandlerInterface>& protocolHandler,
                  const asyncConnectCB& asyncCb);

/// Connect to an end-point at the given @uri and return StreamSocket.
std::shared_ptr<StreamSocket>
connect(std::string uri, const std::shared_ptr<ProtocolHandlerInterface>& protocolHandler);

inline std::string_view getDefaultPortForScheme(const std::string_view scheme)
{
    if (scheme == "https://" || scheme == "wss://")
        return "443";
    if (scheme == "http://" || scheme == "ws://")
        return "80";
    return std::string_view();
}

// Returns true if both URIs are equivalent for an origin check. Implicit
// default port numbers are considered equivalent if explicitly included in the
// compared against peer.
bool sameOrigin(const std::string& expectedOrigin, const std::string& actualOrigin);

} // namespace net

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
