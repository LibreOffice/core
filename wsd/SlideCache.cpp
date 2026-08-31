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

#include <algorithm>

namespace
{
// The layer messages of one rendering arrive one at a time, so an entry grows message by
// message and is whole once its last message is the closing sliderenderingcomplete.
bool isComplete(const std::vector<std::shared_ptr<Message>>& entry)
{
    return !entry.empty() && entry.back()->firstTokenMatches("sliderenderingcomplete:");
}
}

void SlideLayerCacheMap::insert(const std::string& key, std::shared_ptr<Message> cachedData)
{
    auto it = cache_map.find(key);
    if (it != cache_map.end())
    {
        // The key encodes every render parameter, so a second rendering for the same key
        // produces the same layers. Once a whole rendering is stored, keep it and drop the
        // messages of any further rendering for this key.
        if (isComplete(it->second))
            return;

        if (insertion_order.back() != key)
        {
            auto pos = std::find(insertion_order.begin(), insertion_order.end(), key);
            std::rotate(pos, pos + 1, insertion_order.end());
        }

        it->second.emplace_back(std::move(cachedData));
    }
    else
    {
        insertion_order.push_back(key);
        cache_map[key].emplace_back(std::move(cachedData));
    }

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

void SlideLayerCacheMap::erase(const std::string& key)
{
    if (cache_map.erase(key) > 0)
    {
        auto pos = std::find(insertion_order.begin(), insertion_order.end(), key);
        if (pos != insertion_order.end())
            insertion_order.erase(pos);
    }
}

void SlideLayerCacheMap::erase_all()
{
    cache_map.clear();
    insertion_order.clear();
}

SlideLayerCacheMap::Map::const_iterator
SlideLayerCacheMap::find(const std::string& key) const
{
    auto it = cache_map.find(key);
    // Only a whole rendering is found. An entry that still lacks its closing
    // sliderenderingcomplete message holds a rendering that is still streaming in.
    if (it != cache_map.end() && !isComplete(it->second))
        return cache_map.end();
    return it;
}

SlideLayerCacheMap::Map::const_iterator
SlideLayerCacheMap::end() const
{
    return cache_map.end();
}

std::size_t SlideLayerCacheMap::size() const { return cache_map.size(); }
