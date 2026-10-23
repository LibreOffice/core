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

#include <common/JsonUtil.hpp>
#include <wsd/CacheUtil.hpp>

#include <Poco/JSON/Object.h>

#include <string>

/// The shared settings a CheckFileInfo answer names: the URI the settings are fetched from
/// and the configuration id derived from it. The id is "shared-" followed by the cache id of
/// the URI, followed by "-" and the stamp when the answer carries one. Both are empty when the
/// answer names no SharedSettings.
class SharedSettings
{
public:
    explicit SharedSettings(const Poco::JSON::Object::Ptr& wopiInfo)
    {
        if (auto settingsJSON = wopiInfo->getObject("SharedSettings"))
        {
            JsonUtil::findJSONValue(settingsJSON, "uri", _uri);
            _configId = "shared-" + Cache::getConfigId(_uri);

            std::string stamp;
            JsonUtil::findJSONValue(settingsJSON, "stamp", stamp);
            if (!stamp.empty())
                _configId.append("-").append(stamp);
        }
    }

    const std::string& getConfigId() const { return _configId; }

    const std::string& getUri() const { return _uri; }

private:
    std::string _uri;
    std::string _configId;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
