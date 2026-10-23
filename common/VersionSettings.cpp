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

#include <config.h>

#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Prefs.hpp>
#include <common/VersionSettings.hpp>

#include <algorithm>
#include <cstdint>
#include <utility>

#include <Poco/JSON/Object.h>

namespace versioning
{
Limits limitsFromPrefs(const Prefs& prefs)
{
    const auto number = [&prefs](const char* key, std::uint64_t fallback) -> std::uint64_t
    {
        const std::string value = prefs.get(key);
        if (value.empty())
            return fallback;
        try
        {
            return std::stoull(value);
        }
        catch (const std::exception&)
        {
            return fallback;
        }
    };

    // A zero cap would silently delete everything kept so far.
    const auto cap = [&number](const char* key, std::uint64_t fallback)
    { return std::max<std::uint64_t>(1, number(key, fallback)); };

    Limits limits;
    limits.enabled = prefs.get("versions.enabled") != "false";
    limits.mergeWindow = std::chrono::seconds(number("versions.mergeWindowSecs", 600));
    limits.maxVersionsPerDoc = cap("versions.maxPerDoc", 120);
    limits.maxFileSizeBytes = cap("versions.maxFileSizeMB", 128) * 1024 * 1024;
    // The store keeps no version of a document above the storage limit.
    limits.maxStoreSizeBytes = std::max(cap("versions.maxStoreSizeMB", 4096) * 1024 * 1024,
                                        limits.maxFileSizeBytes);
    return limits;
}

void initializeForApp(const std::string& appDataDir, const Prefs& prefs,
                      std::vector<std::string> extraExcludedPrefixes,
                      const std::string& exportDirectory)
{
    extraExcludedPrefixes.push_back(appDataDir);
    initialize(appDataDir + "/versions", limitsFromPrefs(prefs), std::move(extraExcludedPrefixes),
               exportDirectory);
}

std::string statusJson()
{
    const Limits limits = currentLimits();
    Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
    json->set("enabled", limits.enabled);
    json->set("mergeWindowSecs", static_cast<Poco::UInt64>(limits.mergeWindow.count()));
    json->set("maxPerDoc", static_cast<Poco::UInt64>(limits.maxVersionsPerDoc));
    json->set("maxFileSizeMB", static_cast<Poco::UInt64>(limits.maxFileSizeBytes / (1024 * 1024)));
    json->set("maxStoreSizeMB",
              static_cast<Poco::UInt64>(limits.maxStoreSizeBytes / (1024 * 1024)));
    json->set("storeSizeBytes", static_cast<Poco::UInt64>(storeSizeBytes()));
    return JsonUtil::jsonToString(json);
}

void applyLimitsJson(const std::string& payload, Prefs& prefs)
{
    Poco::JSON::Object::Ptr object;
    if (!JsonUtil::parseJSON(payload, object))
    {
        LOG_ERR("SETVERSIONSTORELIMITS: malformed payload");
        return;
    }

    std::vector<std::pair<std::string, std::string>> values;
    if (object->has("enabled"))
        values.emplace_back("versions.enabled",
                            object->optValue<bool>("enabled", true) ? "true" : "false");
    for (const char* key : { "mergeWindowSecs", "maxPerDoc", "maxFileSizeMB", "maxStoreSizeMB" })
    {
        if (!object->has(key))
            continue;
        try
        {
            values.emplace_back(std::string("versions.") + key,
                                std::to_string(object->getValue<std::uint64_t>(key)));
        }
        catch (const std::exception&)
        {
            LOG_ERR("SETVERSIONSTORELIMITS: bad value for " << key);
        }
    }

    for (const auto& [key, value] : values)
        prefs.set(key, value);
    setLimits(limitsFromPrefs(prefs));
}

std::string clearAllJson()
{
    clearAll();
    return std::to_string(storeSizeBytes());
}
} // namespace versioning

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
