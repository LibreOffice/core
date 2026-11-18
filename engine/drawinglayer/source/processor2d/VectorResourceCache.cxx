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

#include <drawinglayer/processor2d/VectorResourceCache.hxx>

#include <basegfx/vector/b2dvector.hxx>
#include <comphelper/random.hxx>
#include <o3tl/hash_combine.hxx>
#include <tools/json_writer.hxx>
#include <tools/stream.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/font/CharacterSubset.hxx>
#include <vcl/gfxlink.hxx>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <optional>

namespace drawinglayer
{
namespace
{
/// The graphic scaled down to rPixelSize, or nothing when it is not a still bitmap or not larger.
std::optional<Bitmap> fittedBitmap(const Graphic& rGraphic, const basegfx::B2DVector& rPixelSize)
{
    if (rGraphic.GetType() != GraphicType::Bitmap || rGraphic.IsAnimated()
        || rGraphic.getVectorGraphicData())
        return {};

    const Size aPixels = rGraphic.GetSizePixel();
    if (aPixels.IsEmpty())
        return {};

    // One scale for both sides keeps the proportions of the image. The side that needs more of its
    // pixels decides it.
    const double fScale = std::max(rPixelSize.getX() / aPixels.Width(),
                                   rPixelSize.getY() / aPixels.Height());
    if (fScale >= 1.0)
        return {};

    const Size aFitted(std::max<tools::Long>(1, std::lround(aPixels.Width() * fScale)),
                       std::max<tools::Long>(1, std::lround(aPixels.Height() * fScale)));
    Bitmap aBitmap(rGraphic.GetBitmap());
    if (!aBitmap.Scale(aFitted, BmpScaleFlag::BestQuality))
        return {};
    return aBitmap;
}
}

void VectorResourceCache::attachTo(Primitive2dJsonProcessor& rProcessor)
{
    rProcessor.setBitmapCache(maBitmapCache);
    rProcessor.setFontCache(maFontCache, maFaceByKey);
}

void VectorResourceCache::writeGraphic(tools::JsonWriter& rWriter, sal_Int64 nChecksum) const
{
    rWriter.put("type", "vectorrenderinggraphics");
    rWriter.put("checksum", nChecksum);
    auto aIterator = maBitmapCache.find(nChecksum);
    if (aIterator != maBitmapCache.end())
        Primitive2dJsonProcessor::writeGraphicBase64(rWriter, aIterator->second);
    else
        rWriter.put("error", "notfound");
}

std::string VectorResourceCache::graphicAnswer(sal_Int64 nChecksum) const
{
    tools::JsonWriter aWriter;
    aWriter.put("type", "vectorrenderinggraphics");
    aWriter.put("checksum", nChecksum);
    auto aIterator = maBitmapCache.find(nChecksum);
    if (aIterator == maBitmapCache.end())
    {
        aWriter.put("error", "notfound");
        return aWriter.finishAndGetAsStdString();
    }

    std::string aAnswer;
    const bool bEncoded = Primitive2dJsonProcessor::withGraphicBytes(
        aIterator->second,
        [&aWriter, &aAnswer](std::string_view rMimeType, const void* pData, std::size_t nSize)
        {
            aWriter.put("mime", rMimeType);
            aAnswer = aWriter.finishAndGetAsStdString();
            aAnswer.reserve(aAnswer.size() + 1 + nSize);
            aAnswer += '\n';
            aAnswer.append(static_cast<const char*>(pData), nSize);
        });
    if (!bEncoded)
    {
        aWriter.put("error", "conversionfailed");
        return aWriter.finishAndGetAsStdString();
    }
    return aAnswer;
}

void VectorResourceCache::writeFont(tools::JsonWriter& rWriter, sal_uInt64 nFontId) const
{
    rWriter.put("type", "vectorrenderingfont");
    rWriter.put("fontId", OString::number(nFontId, 16));

    auto aIterator = maFontCache.find(nFontId);
    if (aIterator == maFontCache.end())
        return;

    const BinaryDataContainer& rBytes = aIterator->second;
    rWriter.putBase64("data", "", rBytes.getData(), rBytes.getSize());
}

std::string VectorResourceCache::fontAnswer(sal_uInt64 nFontId) const
{
    tools::JsonWriter aWriter;
    aWriter.put("type", "vectorrenderingfont");
    aWriter.put("fontId", OString::number(nFontId, 16));
    std::string aAnswer = aWriter.finishAndGetAsStdString();

    auto aIterator = maFontCache.find(nFontId);
    if (aIterator == maFontCache.end())
        return aAnswer;

    const BinaryDataContainer& rBytes = aIterator->second;
    aAnswer.reserve(aAnswer.size() + 1 + rBytes.getSize());
    aAnswer += '\n';
    aAnswer.append(reinterpret_cast<const char*>(rBytes.getData()), rBytes.getSize());
    return aAnswer;
}

std::optional<FontSubset>
VectorResourceCache::subsetFont(sal_uInt64 nFontId, const std::set<sal_UCS4>& rCharacters) const
{
    auto aIterator = maFontCache.find(nFontId);
    if (aIterator == maFontCache.end())
        return {};

    FontSubset aSubset;
    aSubset.maBytes = font::subsetToCharacters(aIterator->second, rCharacters);
    if (aSubset.maBytes.isEmpty())
        return {};
    aSubset.mnFontId = aSubset.maBytes.calculateHash();
    aSubset.maRanges = font::characterRanges(rCharacters);
    return aSubset;
}

std::map<sal_uInt64, FontSubset> VectorResourceCache::subsetFonts(const ResourceUsage& rUsage) const
{
    std::map<sal_uInt64, FontSubset> aSubsets;
    for (const auto& [nFontId, rCharacters] : rUsage.maCharactersByFont)
    {
        if (std::optional<FontSubset> oSubset = subsetFont(nFontId, rCharacters))
            aSubsets.emplace(nFontId, std::move(*oSubset));
    }
    return aSubsets;
}

void VectorResourceCache::writeFontSubset(tools::JsonWriter& rWriter, const FontSubset& rSubset)
{
    rWriter.put("type", "vectorrenderingfont");
    rWriter.put("fontId", OString::number(rSubset.mnFontId, 16));
    {
        auto aRangeArray = rWriter.startArray("ranges");
        for (const auto& [nFirst, nLast] : rSubset.maRanges)
        {
            auto aRange = rWriter.startAnonArray();
            rWriter.putSimpleValue(sal_Int64(nFirst));
            rWriter.putSimpleValue(sal_Int64(nLast));
        }
    }
    rWriter.putBase64("data", "", rSubset.maBytes.getData(), rSubset.maBytes.getSize());
}

void VectorResourceCache::writeGraphicForPixelSize(tools::JsonWriter& rWriter,
                                                   sal_Int64 nChecksum,
                                                   const basegfx::B2DVector& rPixelSize) const
{
    auto aIterator = maBitmapCache.find(nChecksum);
    const std::optional<Bitmap> oFitted
        = aIterator != maBitmapCache.end() ? fittedBitmap(aIterator->second, rPixelSize)
                                           : std::nullopt;
    if (!oFitted)
    {
        writeGraphic(rWriter, nChecksum);
        return;
    }

    rWriter.put("type", "vectorrenderinggraphics");
    rWriter.put("checksum", nChecksum);

    // A photo stays a JPEG image, which is far smaller than PNG for one. JPEG holds no alpha, so an
    // image with transparent parts is a PNG image.
    const Graphic& rGraphic = aIterator->second;
    const bool bJpeg = rGraphic.IsGfxLink()
                       && rGraphic.GetGfxLink().GetType() == GfxLinkType::NativeJpg
                       && !oFitted->HasAlpha();
    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, Graphic(*oFitted),
                                 bJpeg ? ConvertDataFormat::JPG : ConvertDataFormat::PNG)
        != ERRCODE_NONE)
    {
        rWriter.put("error", "conversionfailed");
        return;
    }
    rWriter.putBase64("data", bJpeg ? "data:image/jpeg;base64," : "data:image/png;base64,",
              aStream.GetData(), aStream.Tell());
}

void VectorResourceCache::writeResources(tools::JsonWriter& rWriter, const ResourceUsage& rUsage,
                                         double fPixelsPerUnit,
                                         const std::map<sal_uInt64, FontSubset>& rSubsets) const
{
    {
        auto aGraphicArray = rWriter.startArray("graphics");
        for (const auto& [nChecksum, rDrawnSize] : rUsage.maDrawnSizeByGraphic)
        {
            auto aEntry = rWriter.startStruct();
            writeGraphicForPixelSize(rWriter, nChecksum,
                                     basegfx::B2DVector(rDrawnSize.getX() * fPixelsPerUnit,
                                                        rDrawnSize.getY() * fPixelsPerUnit));
        }
    }

    auto aFontArray = rWriter.startArray("fonts");
    for (const auto& rFont : rUsage.maCharactersByFont)
    {
        auto aEntry = rWriter.startStruct();
        if (auto aSubset = rSubsets.find(rFont.first); aSubset != rSubsets.end())
            writeFontSubset(rWriter, aSubset->second);
        else
            writeFont(rWriter, rFont.first);
    }
}

sal_Int32 VectorResourceCache::getEpoch() const
{
    // Only a document that serves vector content takes a number from the random sequence.
    if (mnEpoch == 0)
        const_cast<VectorResourceCache*>(this)->renewEpoch();
    return mnEpoch;
}

void VectorResourceCache::renewEpoch()
{
    // The clock is mixed in because the random seed stays the same across kit restarts. The loop
    // makes every renewal change the epoch.
    const sal_Int32 nOldEpoch = mnEpoch;
    do
    {
        sal_uInt32 nEpoch = sal_uInt32(comphelper::rng::uniform_int_distribution(1, SAL_MAX_INT32));
        o3tl::hash_combine(nEpoch, std::chrono::steady_clock::now().time_since_epoch().count());
        nEpoch &= SAL_MAX_INT32;
        mnEpoch = nEpoch == 0 ? 1 : sal_Int32(nEpoch);
    } while (mnEpoch == nOldEpoch);
}

} // namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
