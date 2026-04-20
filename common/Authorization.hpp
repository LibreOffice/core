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

// WOPI Authorization

#pragma once

#include <common/Anonymizer.hpp>
#include <common/Log.hpp>
#include <common/StateEnum.hpp>

#include <chrono>
#include <string>

namespace Poco
{
namespace Net
{
class HTTPRequest;
}

class URI;

} // namespace Poco

/// Class to keep the authorization data, which can be either access_token or access_header.
class Authorization
{
    using duration = std::chrono::milliseconds;

public:
    STATE_ENUM(Type,
               None, ///< Unlike Expired, this implies no Authorization needed.
               Token, ///< Valid access_token -> "Authorization: Bearer ..." header.
               Header, ///< Valid access_header -> Custom header(s).
               TokenRefresh, ///< Pending a Token refresh from integration.
               Expired ///< The server is rejecting the current authorization key.
    );

private:
    std::string _data;
    Type _type;
    duration _expiryEpoch; ///< Milliseconds from the epoch when the access_token will expire.
    std::chrono::steady_clock::time_point _tokenRefreshStartTime; ///< Only when refreshing.
    std::chrono::seconds _tokenRefreshTimeout; ///< Maximum time to wait for Token refresh.
    bool _noHeader;

    Authorization()
        : Authorization(Type::None, std::string(), false)
    {
    }

public:
    Authorization(Type type, std::string data, bool noHeader)
        : _data(std::move(data))
        , _type(type)
        , _expiryEpoch(duration::zero())
        , _tokenRefreshStartTime(duration::zero())
        , _tokenRefreshTimeout(std::chrono::seconds::zero())
        , _noHeader(noHeader)
    {
    }

    /// Create an Authorization instance from the URI query parameters.
    /// Expects access_token (preferred) or access_header.
    static Authorization create(const Poco::URI& uri);
    static Authorization create(const std::string& uri);

    void resetAccessToken(std::string accessToken, duration expiryEpoch)
    {
        _type = Type::Token;
        _data = std::move(accessToken);
        _expiryEpoch = expiryEpoch;
    }

    /// Returns true iff Type is Token and we passed the expiry-epoch, or
    /// we are refreshing already.
    bool needTokenRefresh() const
    {
        return _type == Type::TokenRefresh ||
               (_type == Type::Token && _expiryEpoch > duration::zero() &&
                std::chrono::system_clock::now().time_since_epoch() > _expiryEpoch);
    }

    /// Start waiting for a token refresh.
    void startTokenRefresh(const std::chrono::seconds timeout)
    {
        LOG_ASSERT_MSG(_type == Type::Token, "Token refresh is meaningful only for access_token");
        _type = Type::TokenRefresh;
        _tokenRefreshStartTime = std::chrono::steady_clock::now();
        _tokenRefreshTimeout = timeout;
    }

    /// Returns true iff we are refreshing the token.
    bool isRefreshingToken() const { return _type == Type::TokenRefresh; }

    /// Returns true if the timeout has elapsed without a refresh.
    bool isTokenRefreshTimedOut(
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) const
    {
        return isRefreshingToken() && (now - _tokenRefreshStartTime) >= _tokenRefreshTimeout;
    }

    /// Sets the Token's expiry time from the epoch.
    void setExpiryEpoch(duration epochMs)
    {
        LOG_ASSERT_MSG(epochMs == duration::zero() || _type == Type::Token,
                       "Token expiry is meaningful only for access_token");
        _expiryEpoch = epochMs;
    }

    /// Expire the Authorization data.
    void expire() { _type = Type::Expired; }

    /// Returns true if Type is Expired or we passed the expiry-epoch.
    bool isExpired() const
    {
        return _type == Type::Expired ||
               (_expiryEpoch > duration::zero() &&
                std::chrono::system_clock::now().time_since_epoch() > _expiryEpoch);
    }

    /// Set the access_token parameter to the given URI.
    void authorizeURI(Poco::URI& uri) const;

    /// Set the Authorization: header in request.
    void authorizeRequest(Poco::Net::HTTPRequest& request) const;

    void dumpState(std::ostream& os, const std::string& indent = "\n  ") const
    {
        const auto now = std::chrono::system_clock::now();

        os << indent << "Authorization: " << (Anonymizer::enabled() ? "<redacted>" : _data);
        os << indent << "\ttype: " << name(_type);
        os << indent << "\texpiryEpoch (TTL): " << _expiryEpoch
           << Util::getTimeForLog(
                  now, std::chrono::system_clock::time_point(
                           std::chrono::duration_cast<std::chrono::system_clock::duration>(
                               _expiryEpoch)));
        os << indent
           << "\ttokenRefreshStartTime: " << Util::getTimeForLog(now, _tokenRefreshStartTime);
        os << indent << "\ttokenRefreshTimeout: " << _tokenRefreshTimeout;
        os << indent << "\theader: " << (_noHeader ? "No" : "Yes");
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
