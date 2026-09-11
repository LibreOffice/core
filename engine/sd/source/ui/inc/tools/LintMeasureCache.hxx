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

#include <cstddef>
#include <deque>
#include <optional>
#include <unordered_map>

#include <sal/types.h>
#include <tools/long.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/checksum.hxx>

#include <sddllapi.h>

#include <tools/LintImageCompressor.hxx>

namespace sd::lint
{
/** What one measurement was worked out from: the bitmap the encoding started from, the size that
    bitmap is drawn at, and the settings the encoding ran with. Two measurements that agree on all
    of these produce the same encoded bytes.

    The settings are part of the key, so a measurement taken for one target resolution or one
    quality is never read for another one, and a reader who changes either setting simply asks
    under a different key. */
struct LintMeasureKey
{
    /** Checksum of the bitmap, which is what tells one bitmap from another. */
    BitmapChecksum mnChecksum = 0;

    /** The size the bitmap is drawn at, in hundredths of a millimetre. */
    tools::Long mnLogicWidth = 0;
    tools::Long mnLogicHeight = 0;

    /** Wanted resolution in dots per inch. */
    sal_Int32 mnTargetDPI = 0;

    /** JPEG quality from 1 to 100. */
    sal_Int32 mnJPEGQuality = 0;

    bool operator==(const LintMeasureKey& rOther) const
    {
        return mnChecksum == rOther.mnChecksum && mnLogicWidth == rOther.mnLogicWidth
               && mnLogicHeight == rOther.mnLogicHeight && mnTargetDPI == rOther.mnTargetDPI
               && mnJPEGQuality == rOther.mnJPEGQuality;
    }
};

/** Hash of a measure key, so that keys can be looked up in an unordered container. */
struct LintMeasureKeyHash
{
    std::size_t operator()(const LintMeasureKey& rKey) const;
};

/** Holds on to what the measurements of images worked out, so that the same bitmap drawn at the
    same size and measured at the same settings is encoded once however often it is met.

    Only the encoded bytes are kept. Whether an encoding is smaller than what the document holds
    today rests on the byte count of the graphic that was measured, and one bitmap may sit behind
    several links of different sizes, so that answer is worked out afresh for every count the cache
    is asked about.

    What it holds is bounded. Once the kept bytes are over the limit the encodings that went in
    first are dropped, so a session that scans one deck after another keeps a ceiling on what the
    cache costs. */
class SD_DLLPUBLIC LintMeasureCache
{
public:
    /** The encoding kept under the key, with the answer to whether it is smaller worked out against
        the bytes the document spends on the image today. Nothing comes back when no encoding is
        kept under the key. */
    std::optional<LintCompressedImage> find(const LintMeasureKey& rKey,
                                            sal_uInt64 nSourceBytes) const;

    /** Keeps the bytes of the encoding under the key. An encoding that came out empty is not kept,
        and a key that is already there keeps the bytes it has. */
    void store(const LintMeasureKey& rKey, const LintCompressedImage& rCompressed);

    /** Lets go of every encoding the cache holds. */
    void clear();

    /** How many encodings the cache holds. */
    std::size_t getCount() const { return maEntries.size(); }

private:
    std::unordered_map<LintMeasureKey, BinaryDataContainer, LintMeasureKeyHash> maEntries;

    /** The keys in the order their encodings went in, oldest first. */
    std::deque<LintMeasureKey> maKeyOrder;

    /** How many bytes the kept encodings take altogether. */
    sal_uInt64 mnStoredBytes = 0;
};

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
