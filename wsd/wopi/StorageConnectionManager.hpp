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

#pragma once

#include <common/Authorization.hpp>
#include <common/ConfigUtil.hpp>
#include <net/HttpRequest.hpp>

#include <chrono>
#include <memory>
#include <string>

#include <Poco/URI.h>
#include <Poco/Util/Application.h>

/// A Storage Manager is responsible for the settings
/// of Storage and the creation of http::Session and
/// related objects.
class StorageConnectionManager final
{
public:
    static std::shared_ptr<StorageConnectionManager> create()
    {
        static std::weak_ptr<StorageConnectionManager> instance;
        std::shared_ptr<StorageConnectionManager> scm = instance.lock();
        if (!scm)
        {
            scm = std::shared_ptr<StorageConnectionManager>(new StorageConnectionManager());
            instance = scm;
        }

        return scm;
    }

    /// Create an http::Session from a URI.
    /// The configured timeout (net.connection_timeout_secs) is used when 0 is given.
    static std::shared_ptr<http::Session>
    getHttpSession(const Poco::URI& uri,
                   std::chrono::seconds timeout = std::chrono::seconds::zero());

    /// Create an http::Request with the common headers.
    static http::Request createHttpRequest(const Poco::URI& uri, const Authorization& auth);

    static void initialize();

    /// True when the storage client SSL context does no certificate
    /// verification: either storage SSL is off altogether, or it is on but
    /// ssl.ssl_verification is false.
    static bool isStorageVerificationDisabled();

private:
    StorageConnectionManager() = default;

    /// Sanitize a URI by removing authorization tokens.
    static Poco::URI sanitizeUri(Poco::URI uri)
    {
        constexpr std::string_view access_token("access_token");

        Poco::URI::QueryParameters queryParams = uri.getQueryParameters();
        for (auto& param : queryParams)
        {
            // Sanitize more params as needed.
            if (param.first == access_token)
            {
                // If access_token exists, clear it. But don't add it if not provided.
                param.second.clear();
                uri.setQueryParameters(queryParams);
                break;
            }
        }

        return uri;
    }

    /// If true, use only the WOPI URL for whether to use SSL to talk to storage server
    static bool SSLAsScheme;
    /// If true, force SSL communication with storage server
    static bool SSLEnabled;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
