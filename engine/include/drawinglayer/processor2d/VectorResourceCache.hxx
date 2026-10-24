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
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/graph.hxx>

#include <unordered_map>

namespace tools
{
class JsonWriter;
}

namespace drawinglayer
{
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

    /// Write the answer to a graphic request: type, checksum, and the image or an "error" reason.
    void writeGraphic(tools::JsonWriter& rWriter, sal_Int64 nChecksum) const;

    /// Write the answer to a font request: type, font id in hex, and the bytes in base64 if known.
    void writeFont(tools::JsonWriter& rWriter, sal_uInt64 nFontId) const;

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
