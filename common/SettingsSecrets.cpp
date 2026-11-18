/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <span>
#include <string>
#include <string_view>

#include <Poco/JSON/Object.h>

#include <common/JsonUtil.hpp>
#include <common/SettingsSecrets.hpp>

std::string SettingsSecrets::redactSettingSecrets(
    const std::string& body, std::span<const std::string_view> secretFields)
{
    Poco::JSON::Object::Ptr json;
    if (!JsonUtil::parseJSON(body, json) || !json)
        return "";

    bool changed = false;
    for (const std::string_view& field : secretFields)
    {
        const std::string name(field);
        if (!json->has(name))
            continue;
        std::string value;
        JsonUtil::findJSONValue(json, name, value);
        if (value.empty())
            continue;
        json->set(name, std::string());
        json->set(name + std::string(SettingsSecrets::StoredFlagSuffix), true);
        changed = true;
    }

    return changed ? JsonUtil::jsonToString(json) : body;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
