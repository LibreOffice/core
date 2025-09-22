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
               Expired ///< The server is rejecting the current authorization key.
    );

private:
    std::string _data;
    Type _type;
    duration _expiryEpoch; ///< Milliseconds from the epoch when the access_token will expire.
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

    /// Expire the Authorization data.
    void expire() { _type = Type::Expired; }

    void setExpiryEpoch(duration epochMs) { _expiryEpoch = epochMs; }

    /// Returns true iff the Authorization data is invalid.
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
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
