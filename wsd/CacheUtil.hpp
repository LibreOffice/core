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
 * Utilities for cache directory management and cleanup.
 * Functions: getCacheDir(), clearCacheDir()
 */

#pragma once

#include <string>
#include <vector>

struct CacheQuery
{
    std::string _uri;
    std::string _stamp;
    std::string _dest;

    CacheQuery(const std::string& uri, const std::string& stamp, const std::string& dest)
        : _uri(uri)
        , _stamp(stamp)
        , _dest(dest)
    {
    }
};

class Cache
{
public:
    /// The directory in the cache that holds the slide previews. It is not a config cache
    /// entry, and it keeps its own expiry.
    static constexpr const char* PreviewsDirName = "previews";

    static void initialize(const std::string& path);

    static std::string getConfigId(const std::string& uri);

    static void cacheConfigFile(const std::string& configId, const std::string& uri,
                                const std::string& stamp, const std::string& filename);

    static void supplyConfigFiles(const std::string& configId, std::vector<CacheQuery>& queries);

    static void clearOutdatedConfigs();

private:
    static void updateLastUsed(const std::string& path);

    static bool supplyConfigFile(const std::string& cacheDir, const std::string& stamp,
                                 const std::string& dest);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
