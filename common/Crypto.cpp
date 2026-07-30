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
 * Cryptographic support key validation using RSA signatures.
 * Classes: SupportKey - Key verification and expiry checking
 */

#include <config.h>

#include "Crypto.hpp"

#include <common/Log.hpp>

#include <common/base64.hpp>

#include <Poco/Crypto/RSADigestEngine.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DigestStream.h>

#include <fstream>
#include <iostream>
#include <sstream>

#if ENABLE_SUPPORT_KEY
#include <support-public-key.hpp>
#endif

using namespace Poco;
using namespace Poco::Crypto;

std::string getSupportPublicKey()
{
#if ENABLE_SUPPORT_KEY
    return SUPPORT_PUBLIC_KEY;
#else
    return std::string();
#endif
}

struct SupportKeyImpl
{
    std::string _key;
    std::string _data;
    std::string _signature;
    DateTime _expiry;
    bool _invalid;
    // Key format: iso-expiry-date:field1:field2:field:...:<signature>
    SupportKeyImpl(const std::string &key)
        : _key(key), _invalid(true)
    {
        LOG_INF("Support key '" << key << "' provided");
        std::size_t firstColon = key.find(':');
        if (firstColon != std::string::npos)
        {
            std::string expiry(key.substr(0, firstColon));
            LOG_INF("Support key with expiry '" << expiry << '\'');

            try
            {
                int timeZoneDifferential = 0;
                Poco::DateTimeParser::parse(expiry, _expiry, timeZoneDifferential);

                std::size_t lastColon = key.rfind(':');
                if (lastColon != std::string::npos)
                {
                    _signature = key.substr(lastColon + 1,
                                            key.length() - lastColon);
                    _data = key.substr(0, lastColon);
                    LOG_INF("Support key signature '" << _signature << "' data '" << _data << '\'');

                    _invalid = false;
                }
            } catch (SyntaxException &) {
                LOG_ERR("Invalid support key expiry '" << expiry << '\'');
            }
        }
    }
};

SupportKey::SupportKey(const std::string& key)
    : _impl(std::make_unique<SupportKeyImpl>(key))
{
}

SupportKey::~SupportKey()
{
}

bool SupportKey::verify()
{
    if (_impl->_invalid)
    {
        LOG_ERR("Basic key structure is invalid.");
        return false;
    }

    std::istringstream pubStream(getSupportPublicKey());

    try {
        RSAKey keyPub(&pubStream);
        RSADigestEngine rsaEngine(keyPub, RSADigestEngine::DigestType::DIGEST_SHA1);
        rsaEngine.update(_impl->_data);

        std::string rawSig;
        macaron::Base64::Decode(_impl->_signature, rawSig);
        std::vector<unsigned char> rawSignature(rawSig.begin(), rawSig.end());
        LOG_INF("Signature of length " << rawSignature.size()
                << " data size: " << _impl->_data.length());
        if (!rsaEngine.verify(rawSignature))
        {
            LOG_ERR("Support key is not correctly signed.");
            return false;
        }
    } catch (...) {
        LOG_ERR("Exception validating support key.");
        return false;
    }
    LOG_INF("Support key correctly signed.");
    return true;
}

int SupportKey::validDaysRemaining()
{
    if (!verify())
    {
        LOG_ERR("Support key signature is invalid.");
        return 0;
    }
    Timespan remaining = _impl->_expiry - DateTime();
    int days = remaining.days();
    if (days > 0)
        LOG_INF("Support key has " << days << " remaining");
    else
        LOG_ERR("Support key has expired for " << -days << " days");

    return days;
}

DateTime SupportKey::expiry() const
{
    return _impl->_expiry;
}

std::string SupportKey::data() const
{
    return _impl->_data;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
