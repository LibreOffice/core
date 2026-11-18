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

#include <sal/config.h>
#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/graph.hxx>
#include <vcl/vclenum.hxx>

#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace basegfx
{
class B2DVector;
}
namespace tools
{
class JsonWriter;
}

namespace drawinglayer
{
/// A font cut down to the characters of maRanges, each range the first and the last code point of a
/// run. The id is the hash of maBytes, so it differs from the id of the whole font.
struct FontSubset
{
    sal_uInt64 mnFontId = 0;
    std::vector<std::pair<sal_UCS4, sal_UCS4>> maRanges;
    BinaryDataContainer maBytes;
};

/** The graphics and fonts one document's vector primitives name, and the epoch of its versions.

    A primitive names a graphic by its checksum and a font by the hash of the font file. The client
    asks for the bytes separately, so each one is held from the time a primitive names it. Versions
    start at zero in every document, so a client compares versions only within one epoch.
 */
class DRAWINGLAYER_DLLPUBLIC VectorResourceCache
{
public:
    /// Let the processor store the graphics and fonts it writes into this cache.
    void attachTo(Primitive2dJsonProcessor& rProcessor);

    /// Write a graphic: type, checksum, and the image as a base64 data URL or an "error" reason.
    void writeGraphic(tools::JsonWriter& rWriter, sal_Int64 nChecksum) const;

    /// The answer to a graphic request: a line of JSON with type, checksum and the MIME type of
    /// the image as "mime", a newline, then the bytes of the image. A graphic that is not held or
    /// cannot be encoded is answered with the JSON alone, which has an "error" reason instead.
    std::string graphicAnswer(sal_Int64 nChecksum) const;

    /// Write a font: type, font id in hex, and the bytes of the font file in base64 if known.
    void writeFont(tools::JsonWriter& rWriter, sal_uInt64 nFontId) const;

    /// The answer to a font request: a line of JSON with type and font id in hex, a newline, then
    /// the bytes of the font file. A font that is not held is answered with the JSON alone.
    std::string fontAnswer(sal_uInt64 nFontId) const;

    /// The font with nFontId cut down to rCharacters, or nothing when the font is not held or
    /// cannot be cut down.
    std::optional<FontSubset> subsetFont(sal_uInt64 nFontId,
                                         const std::set<sal_UCS4>& rCharacters) const;

    /// The fonts of rUsage cut down to the characters drawn in each, by the id of the whole font.
    std::map<sal_uInt64, FontSubset> subsetFonts(const ResourceUsage& rUsage) const;

    /// Like writeFont for rSubset, with its code points as "ranges".
    static void writeFontSubset(tools::JsonWriter& rWriter, const FontSubset& rSubset);

    /// Like writeGraphic, with a still bitmap larger than rPixelSize scaled down to it, keeping its
    /// proportions.
    void writeGraphicForPixelSize(tools::JsonWriter& rWriter, sal_Int64 nChecksum,
                                  const basegfx::B2DVector& rPixelSize) const;

    /// Write the arrays "graphics" and "fonts" of rUsage, each graphic sized at fPixelsPerUnit. A
    /// font with an entry in rSubsets is written as that subset, any other font whole.
    void writeResources(tools::JsonWriter& rWriter, const ResourceUsage& rUsage,
                        double fPixelsPerUnit,
                        const std::map<sal_uInt64, FontSubset>& rSubsets) const;

    std::unordered_map<sal_Int64, Graphic>& getBitmapCache() { return maBitmapCache; }
    std::unordered_map<sal_uInt64, BinaryDataContainer>& getFontCache() { return maFontCache; }
    std::unordered_map<OUString, Primitive2dJsonProcessor::ResolvedFace>& getFaceByKey()
    {
        return maFaceByKey;
    }

    /// The epoch, drawn on first use. It differs from the epoch of every other document.
    sal_Int32 getEpoch() const;

    /// Draw a new epoch. Clients then hold versions of another epoch and ask for the whole content.
    void renewEpoch();

private:
    std::unordered_map<sal_Int64, Graphic> maBitmapCache;
    std::unordered_map<sal_uInt64, BinaryDataContainer> maFontCache;
    std::unordered_map<OUString, Primitive2dJsonProcessor::ResolvedFace> maFaceByKey;

    /// 0 until the epoch is first asked for.
    mutable sal_Int32 mnEpoch = 0;
};

} // namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
