/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * Implementation of slide cache management.
 * Classes: SlideCache
 */

#include <config.h>

#include "SlideCache.hpp"

void SlideLayerCacheMap::insert(const std::string& key, std::shared_ptr<Message> cachedData)
{
    if (cache_map.contains(key))
    {
        if (insertion_order.back() != key)
        {
            auto pos = std::find(insertion_order.begin(), insertion_order.end(), key);
            std::rotate(pos, pos + 1, insertion_order.end());
        }
    }
    else
        insertion_order.push_back(key);

    cache_map[key].emplace_back(std::move(cachedData));
    reduceSizeTo(max_size);
}

std::size_t SlideLayerCacheMap::reduceSizeTo(std::size_t desiredSize)
{
    if (cache_map.size() <= desiredSize)
        return 0;

    std::size_t total_deleted_entries = 0;
    while (cache_map.size() > desiredSize)
    {
        cache_map.erase(insertion_order.front());
        insertion_order.erase(insertion_order.begin());
        total_deleted_entries++;
    }

    return total_deleted_entries;
}

void SlideLayerCacheMap::erase_all()
{
    cache_map.clear();
    insertion_order.clear();
}

SlideLayerCacheMap::Map::const_iterator
SlideLayerCacheMap::find(const std::string& key) const
{
    return cache_map.find(key);
}

SlideLayerCacheMap::Map::const_iterator
SlideLayerCacheMap::end() const
{
    return cache_map.end();
}

std::size_t SlideLayerCacheMap::size() const { return cache_map.size(); }
