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
 * Ranking of tiles by how soon the view they belong to needs them.
 * Classes: TilePrioritizer
 */

#pragma once

#include <common/Rectangle.hpp>
#include <wsd/TileDesc.hpp>

#include <cstdint>
#include <utility>
#include <vector>

class TilePrioritizer
{
public:
    virtual ~TilePrioritizer() = default;

    enum class Priority : std::int8_t {
        NONE = -1,  // an error
        LOWEST,
        LOW,
        NORMAL,
        HIGH,
        VERYHIGH,
        ULTRAHIGH
    };
    virtual Priority getTilePriority(const TileDesc &) const { return Priority::NORMAL; }

    using ViewIdInactivity = std::pair<CanonicalViewId, float>;
    virtual std::vector<ViewIdInactivity> getViewIdsByInactivity() const { return {}; }

    /// Maps a canonical view id a session has already left onto the one it holds
    /// now. Returns the id unchanged when no session ever retired it.
    virtual CanonicalViewId resolveCanonicalViewId(CanonicalViewId id) const { return id; }

    /// Rank one tile by where it sits in a view: the caret comes first, then what the view shows,
    /// then what it is about to show, with another part and previews last. The preload margins say
    /// how far past the visible area, in twips, the view fills in ahead of itself. An empty
    /// rectangle stands for an area that is not known, and ranks nothing. This runs for every tile
    /// on both sides of the wire, so it stays with comparisons and additions.
    static Priority rankTile(const TileDesc& tile, bool isVisiblePart,
                             const Util::Rectangle& cursorArea,
                             const Util::Rectangle& visibleArea, int preloadMarginX,
                             int preloadMarginY)
    {
        // A preview is drawn for a slide panel or a thumbnail, so it belongs to no visible area.
        if (tile.isPreview())
            return Priority::LOWEST;

        if (!isVisiblePart)
            return Priority::LOW;

        if (tile.intersects(cursorArea))
            return Priority::ULTRAHIGH;

        if (tile.intersects(visibleArea))
            return Priority::VERYHIGH;

        const Util::Rectangle preloadArea = Util::Rectangle::create(
            visibleArea.getLeft() - preloadMarginX, visibleArea.getTop() - preloadMarginY,
            visibleArea.getRight() + preloadMarginX, visibleArea.getBottom() + preloadMarginY);
        if (tile.intersects(preloadArea))
            return Priority::HIGH;

        return Priority::NORMAL;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
