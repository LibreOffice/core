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
 * Implementation of JWT authentication and authorization.
 * Classes: JWTAuth
 */

#include <config.h>

#include "Auth.hpp"

#include <common/ConfigUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/Util.hpp>

#include <Poco/Crypto/RSADigestEngine.h>
#include <Poco/Crypto/RSAKey.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NetException.h>
#include <Poco/URI.h>

#include <cstdlib>
#include <string>

std::unique_ptr<Poco::Crypto::RSAKey> JWTAuth::_key(
    new Poco::Crypto::RSAKey(Poco::Crypto::RSAKey(Poco::Crypto::RSAKey::KL_2048, Poco::Crypto::RSAKey::EXP_LARGE)));

// avoid obscure double frees on exit.
void JWTAuth::cleanup()
{
    _key.reset();
}

std::string JWTAuth::getAccessToken()
{
    std::string encodedHeader = createHeader();
    std::string encodedPayload = createPayload();

    // trim '=' from end of encoded header
    encodedHeader.erase(std::find_if(encodedHeader.rbegin(), encodedHeader.rend(),
                                     [](char& ch)->bool {return ch != '='; }).base(), encodedHeader.end());
    // trim '=' from end of encoded payload
    encodedPayload.erase(std::find_if(encodedPayload.rbegin(), encodedPayload.rend(),
                                      [](char& ch)->bool { return ch != '='; }).base(), encodedPayload.end());
    LOG_INF("Encoded JWT header: " << encodedHeader);
    LOG_INF("Encoded JWT payload: " << encodedPayload);

    // Convert to a URL and filename safe variant:
    // Replace '+' with '-' && '/' with '_'
    std::replace(encodedHeader.begin(), encodedHeader.end(), '+', '-');
    std::replace(encodedHeader.begin(), encodedHeader.end(), '/', '_');

    std::replace(encodedPayload.begin(), encodedPayload.end(), '+', '-');
    std::replace(encodedPayload.begin(), encodedPayload.end(), '/', '_');

    const std::string encodedBody = encodedHeader + '.' +  encodedPayload;

    // sign the encoded body
    _digestEngine.update(encodedBody.c_str(), static_cast<unsigned>(encodedBody.length()));
    Poco::Crypto::DigestEngine::Digest digest = _digestEngine.signature();

    // The signature generated contains CRLF line endings.
    std::string encodedSig = Util::base64Encode(digest);

    // trim '=' from end of encoded signature
    encodedSig.erase(std::find_if(encodedSig.rbegin(), encodedSig.rend(),
                                  [](char& ch)->bool { return ch != '='; }).base(), encodedSig.end());

    // Be URL and filename safe
    std::replace(encodedSig.begin(), encodedSig.end(), '+', '-');
    std::replace(encodedSig.begin(), encodedSig.end(), '/', '_');

    LOG_INF("Sig generated is : " << encodedSig);

    std::string jwtToken = encodedBody + '.' + encodedSig;
    LOG_INF("JWT token generated: " << jwtToken);

    return jwtToken;
}

bool JWTAuth::verify(const std::string& accessToken)
{
    StringVector tokens(StringVector::tokenize(accessToken, '.'));

    try
    {
        if (tokens.size() < 3)
        {
            LOG_ERR("JWTAuth: verification failed; Not enough tokens");
            return false;
        }

        const std::string encodedBody = tokens[0] + '.' + tokens[1];
        _digestEngine.update(encodedBody.c_str(), static_cast<unsigned>(encodedBody.length()));
        Poco::Crypto::DigestEngine::Digest digest = _digestEngine.signature();

        // The signature generated contains CRLF line endings.
        std::string encodedSig = Util::base64Encode(digest);

        // trim '=' from end of encoded signature.
        encodedSig.erase(std::find_if(encodedSig.rbegin(), encodedSig.rend(),
                                      [](char& ch)->bool { return ch != '='; }).base(), encodedSig.end());

        // Make the encoded sig URL and filename safe
        std::replace(encodedSig.begin(), encodedSig.end(), '+', '-');
        std::replace(encodedSig.begin(), encodedSig.end(), '/', '_');

        if (encodedSig != tokens[2])
        {
            LOG_ERR("JWTAuth: verification failed; Expected: " << encodedSig << ", Received: " << tokens[2]);
            if constexpr (!Util::isFuzzing())
            {
                return false;
            }
        }

        const std::string decodedPayload = Util::base64Decode(tokens[1]);
        LOG_INF("JWTAuth:verify: decoded payload: " << decodedPayload);

        // Verify if the token is not already expired
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(decodedPayload);
        const Poco::JSON::Object::Ptr& object = result.extract<Poco::JSON::Object::Ptr>();
        std::time_t decodedExptime = 0;
        object->get("exp").convert(decodedExptime);

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t curtime = std::chrono::system_clock::to_time_t(now);

        LOG_TRC("JWT: cur time " << curtime << " vs. " << decodedExptime);
        if (curtime > decodedExptime)
        {
            LOG_INF("JWTAuth:verify: JWT expired; curtime:" << curtime << ", exp:" << decodedExptime);
            if constexpr (!Util::isFuzzing())
            {
                return false;
            }
        }
    }
    catch(Poco::Exception& exc)
    {
        LOG_ERR("JWTAuth:verify: Exception: " << exc.displayText());
        return false;
    }

    return true;
}

std::string JWTAuth::createHeader()
{
    // TODO: Some sane code to represent JSON objects
    const std::string header = R"({"alg":")" + _alg + R"(","typ":")" + _typ + "\"}";

    LOG_INF("JWT Header: " << header);
    return Util::base64Encode(header);
}

std::string JWTAuth::createPayload()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t curtime = std::chrono::system_clock::to_time_t(now);
    int expirySeconds = ConfigUtil::getConfigValue<int>("security.jwt_expiry_secs", 1800);
    const std::string exptime = std::to_string(curtime + expirySeconds);

    // TODO: Some sane code to represent JSON objects
    const std::string payload = R"({"iss":")" + _iss + R"(","sub":")" + _sub + R"(","aud":")" +
                                _aud + R"(","nme":")" + _name + R"(","exp":")" + exptime + "\"}";

    LOG_INF("JWT Payload: " << payload << " expires in " << expirySeconds << "seconds");
    return Util::base64Encode(payload);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
