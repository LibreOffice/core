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
 * URL construction and manipulation for server endpoints.
 * Classes: ServerURL
 */

#pragma once

#include <string>
#include <RequestDetails.hpp>
#include <COOLWSD.hpp>

/** This class helps us to build a URL that will reliably point back
 * at our service. It does very simple splitting of proxy URL
 * and handles the proxy prefix feature.
 */
class ServerURL
{
    std::string _schemeAuthority;
    std::string _pathPlus;
    bool        _ssl;
    bool        _websocket;
public:
    ServerURL(const RequestDetails &requestDetails)
    {
        init(requestDetails.getHostUntrusted(),
             requestDetails.getProxyPrefix());
    }

    explicit ServerURL()
    {
        init("nohostname", "");
    }

    void init(const std::string &host, const std::string &proxyPrefix)
    {
        // The user can override the ServerRoot with a new prefix.
        _pathPlus = COOLWSD::ServiceRoot;

        _ssl = (ConfigUtil::isSslEnabled() || ConfigUtil::isSSLTermination());
        _websocket = true;
        _schemeAuthority = COOLWSD::ServerName.empty() ? host : COOLWSD::ServerName;

        // A well formed ProxyPrefix will override it.
        const std::string& url = proxyPrefix;
        if (url.empty())
            return;

        std::size_t pos = url.find("://");
        if (pos != std::string::npos) {
            pos += 3;
            auto hostEndPos = url.find('/', pos);
            if (hostEndPos != std::string::npos)
            {
                _websocket = false;
                std::string schemeProtocol = url.substr(0, pos);
                _ssl = (schemeProtocol != "http://");
                _schemeAuthority = url.substr(pos, hostEndPos - pos);
                _pathPlus = url.substr(hostEndPos);
                return;
            }
            else
                LOG_ERR("Unusual proxy prefix '" << url << '\'');
        } else
            LOG_ERR("No http[s]:// in unusual proxy prefix '" << url << '\'');
    }

    const std::string& getResponseRoot() const
    {
        return _pathPlus;
    }

    std::string getWebSocketUrl() const
    {
        std::string schemeProtocol = (_websocket ? "ws" : "http");
        if (_ssl)
            schemeProtocol += 's';
        return schemeProtocol + "://" + _schemeAuthority;
    }

    std::string getWebServerUrl() const
    {
        std::string schemeProtocol = "http";
        if (_ssl)
            schemeProtocol += 's';
        return schemeProtocol + "://" + _schemeAuthority;
    }

    std::string getSubURLForEndpoint(const std::string &path) const
    {
#if MOBILEAPP
        return std::string("cool:") + _pathPlus + path;
#else
        return std::string("http") + (_ssl ? "s" : "") + "://" + _schemeAuthority + _pathPlus + path;
#endif
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
