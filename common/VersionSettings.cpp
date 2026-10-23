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

#include <common/Prefs.hpp>
#include <common/VersionSettings.hpp>

#include <algorithm>
#include <cstdint>
#include <utility>

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
} // namespace versioning

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
