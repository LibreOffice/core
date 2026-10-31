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

#pragma once

#include <drawinglayer/processor2d/VectorResourceCache.hxx>
#include <rtl/string.hxx>
#include <svx/VectorPart.hxx>
#include <swdllapi.h>
#include <swrect.hxx>

#include "VectorCollector.hxx"

#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

class SwPageFrame;
class SwRootFrame;

namespace tools
{
class JsonWriter;
}

namespace sw::vector
{
/// The page list a Writer part sits in: the pages of the print layout.
constexpr sal_Int32 constModePrintLayout = 0;

/// The id of the part that stands for rPage: the frame id of the page in decimal. It stays the
/// same while the page exists, wherever pages are added or removed before it.
SW_DLLPUBLIC OString partIdOf(const SwPageFrame& rPage);

/// The page of rLayout that the part id names, or nullptr when there is none.
SW_DLLPUBLIC const SwPageFrame* pageOf(const SwRootFrame& rLayout, std::string_view rPartId);

/// The part ids of the pages of rLayout, in page order.
SW_DLLPUBLIC std::vector<OString> partIdsOf(const SwRootFrame& rLayout);

/** What one document served of its pages as vector primitives, and the versions they count in.

    Every page is a part. A part counts its version up when something a client draws changed
    since the last write, and every entry remembers the version at which what it draws changed
    last and the version at which where it lies changed last. A delta carries the entries that
    changed after the version a client holds: an entry whose drawing changed comes with its
    primitives, and an entry that only moved comes with its box alone.

    What was written last for an entry is kept by its id, the frame id, which stays the same on
    every page the frame lies on.
 */
class SW_DLLPUBLIC PartStore
{
public:
    /** Writes the page that rPartId names. A non-negative nSinceVersion asks for a delta against
        that version. A push writes the delta since the version last pushed, and writes nothing
        when nothing changed after it, or when no client holds the part. For a page the layout
        does not hold, a pull and a push of a part that was served write the header alone, and
        the part is dropped.
     */
    void write(tools::JsonWriter& rWriter, const SwRootFrame& rLayout, std::string_view rPartId,
               sal_Int64 nSinceVersion, bool bPush);

    /// Marks the served pages that overlap one of rAreas as changed, and returns their part ids.
    /// A served page that is gone is returned too, and the state of a gone page that no client
    /// holds is dropped.
    std::vector<OString> invalidate(const SwRootFrame& rLayout, const std::vector<SwRect>& rAreas);

    /// Whether a client holds the part: it was served, and not forgotten since.
    bool isServed(std::string_view rPartId) const;

    /// Drops what was written for the entries of the part, and marks the part as held by no
    /// client. Its versions go on counting, and the next request for the part serves it whole.
    void forget(std::string_view rPartId);

    /// Records the part ids of the pages of rLayout in page order, and returns true when they
    /// differ from the ones recorded before.
    bool recordPageIds(const SwRootFrame& rLayout);

    drawinglayer::VectorResourceCache& getResources() { return maResources; }
    sal_Int32 getEpoch() const { return maResources.getEpoch(); }

private:
    /// What was written last for one entry, in page twips, and the versions of its changes.
    struct RecordedEntry
    {
        Entry maEntry;
        /// The part the entry lies on now. An entry lies on one page at a time.
        OString maPartId;
        /// The box relative to the page the entry lies on.
        SwRect maPageBox;
        /// The version at which what the entry draws changed last.
        sal_uInt64 mnDrawnVersion = 0;
        /// The version at which where the entry lies, its parent or its flags changed last.
        sal_uInt64 mnPlacedVersion = 0;
    };

    /// The state of one part.
    struct PartState
    {
        /// The version of the part, the version it was last pushed at, and the version at which
        /// the set of entries or their paint order changed last.
        svx::vector::PartVersions maVersions;
        /// True while the page may differ from what was compared last.
        bool mbDirty = true;
        /// While mbDirty is true, the areas in document twips where the page may differ from what
        /// was compared last. Empty for the whole page.
        std::vector<SwRect> maDirtyAreas;
        /// The ids of the entries in paint order, the page entry first.
        std::vector<sal_uInt64> maOrder;
        /// The ids of the entries the part holds.
        std::vector<sal_uInt64> maEntryIds;
    };

    /// Compares the page with its record and counts the part's version up when something
    /// differs. Where each entry lies is always compared. What an entry draws is compared when
    /// bWhole is true, when the entry moved, or when its box overlaps the dirty areas of the part.
    void refresh(PartState& rState, const SwPageFrame& rPage, bool bWhole);

    static void writeEntry(tools::JsonWriter& rWriter, const RecordedEntry& rRecord,
                           bool bDrawing,
                           drawinglayer::Primitive2dJsonProcessor& rProcessor);

    std::unordered_map<OString, PartState> maParts;
    std::unordered_map<sal_uInt64, RecordedEntry> maEntries;
    /// The part ids of the pages in page order, as they were last recorded.
    std::vector<OString> maPageIds;
    drawinglayer::VectorResourceCache maResources;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
