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

// Authentication and Authorization support.

#pragma once

#include <string>
#include <memory>

#if !MOBILEAPP
#include <Poco/Crypto/RSADigestEngine.h>
#include <Poco/Crypto/RSAKey.h>
#endif

/// Base class of all Authentication/Authorization implementations.
class AuthBase
{
public:
    virtual ~AuthBase() = default;
    /// Called to acquire an access token.
    virtual std::string getAccessToken() = 0;

    /// Used to verify the validity of an access token.
    virtual bool verify(const std::string& token) = 0;
};

#if !MOBILEAPP

/// JWT Authorization.
class JWTAuth : public AuthBase
{
public:
    JWTAuth(const std::string& name, const std::string& sub, const std::string& aud)
        : _name(name),
          _sub(sub),
          _aud(aud),
          _digestEngine(*_key, "SHA256")
    {
    }

    std::string getAccessToken() override;

    bool verify(const std::string& accessToken) override;

    static void cleanup();

private:
    std::string createHeader();

    std::string createPayload();

private:
    const std::string _alg = "RS256";
    const std::string _typ = "JWT";

    const std::string _iss = "cool";
    const std::string _name;
    const std::string _sub;
    const std::string _aud;

    static std::unique_ptr<Poco::Crypto::RSAKey> _key;
    Poco::Crypto::RSADigestEngine _digestEngine;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
