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

#include <tools/LintMeasureCache.hxx>

#include <algorithm>
#include <utility>

#include <o3tl/hash_combine.hxx>

namespace sd::lint
{
namespace
{
/** How many bytes of encodings the cache keeps at most. An encoding is one image at the target
    resolution, a few hundred kilobytes for a photograph, so this room holds the measurements of
    several decks at once and still leaves the memory of a long session bounded. */
constexpr sal_uInt64 gnMaximumStoredBytes = 32 * 1024 * 1024;

} // end of anonymous namespace

std::size_t LintMeasureKeyHash::operator()(const LintMeasureKey& rKey) const
{
    std::size_t nSeed = 0;
    o3tl::hash_combine(nSeed, rKey.mnChecksum);
    o3tl::hash_combine(nSeed, rKey.mnLogicWidth);
    o3tl::hash_combine(nSeed, rKey.mnLogicHeight);
    o3tl::hash_combine(nSeed, rKey.mnTargetDPI);
    o3tl::hash_combine(nSeed, rKey.mnJPEGQuality);
    o3tl::hash_combine(nSeed, rKey.mnCropLeft);
    o3tl::hash_combine(nSeed, rKey.mnCropTop);
    o3tl::hash_combine(nSeed, rKey.mnCropRight);
    o3tl::hash_combine(nSeed, rKey.mnCropBottom);
    return nSeed;
}

std::optional<LintCompressedImage> LintMeasureCache::find(const LintMeasureKey& rKey,
                                                          sal_uInt64 nSourceBytes) const
{
    const auto aFound = maEntries.find(rKey);
    if (aFound == maEntries.end())
        return std::nullopt;

    LintCompressedImage aCompressed;
    aCompressed.maData = aFound->second;

    // The bytes are those of the image alone, and whether they are fewer than the document spends
    // today is about the one link that was asked after, so that answer is worked out here.
    aCompressed.mbSmaller = nSourceBytes > 0 && aCompressed.getByteCount() < nSourceBytes;

    return aCompressed;
}

void LintMeasureCache::store(const LintMeasureKey& rKey, const LintCompressedImage& rCompressed)
{
    if (rCompressed.maData.isEmpty())
        return;

    if (!maEntries.emplace(rKey, rCompressed.maData).second)
        return;

    maKeyOrder.push_back(rKey);
    mnStoredBytes += rCompressed.getByteCount();

    // The encodings that went in first go first once the kept bytes are over the limit. The one
    // that has just gone in stays whatever its size, so the measurement that asked for it reads it
    // back.
    while (mnStoredBytes > gnMaximumStoredBytes && maKeyOrder.size() > 1)
    {
        const auto aOldest = maEntries.find(maKeyOrder.front());
        if (aOldest != maEntries.end())
        {
            mnStoredBytes -= std::min(mnStoredBytes, sal_uInt64(aOldest->second.getSize()));
            maEntries.erase(aOldest);
        }

        maKeyOrder.pop_front();
    }
}

void LintMeasureCache::clear()
{
    maEntries.clear();
    maKeyOrder.clear();
    mnStoredBytes = 0;
}

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
