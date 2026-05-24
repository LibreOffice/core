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
 * Caching for slide previews and thumbnails.
 * Classes: SlideCache
 */

#pragma once

#include <common/ContainerUtil.hpp>
#include <common/Message.hpp>

#include <string>
#include <vector>
#include <memory>

/**
 Used for caching rendered slide layers for slideshow
 example entry may look like following
 key: hash=108777063986320 part=0 width=1919 height=1080 renderBackground=1 renderMasterPage=1 devicePixelRatio=1 compressedLayers=0
 value vector:[SlideLayer1 Binary message, SlideLayer2 Binary message..., sliderenderingcomplete]
 key consists of all the parameters browser sends us for particular slide rendering
 value vector will be in order layers should be rendered and displayed and last message will be sliderenderingcomplete
*/
class SlideLayerCacheMap final
{
    using Map = Util::UnorderedStringMap<std::vector<std::shared_ptr<Message>>>;

    Map cache_map;

    // Helps if we want to clean older cache
    std::vector<std::string> insertion_order;

    // Number of slides to cache layers for
    std::size_t max_size;

public:
    SlideLayerCacheMap(std::size_t maxSize)
        : max_size(maxSize)
    {
    }

    void insert(const std::string& key, std::shared_ptr<Message> cachedData);

    std::size_t reduceSizeTo(std::size_t desiredSize);

    void erase_all();

    Map::const_iterator find(const std::string& key) const;

    Map::const_iterator end() const;

    std::size_t size() const;
};
