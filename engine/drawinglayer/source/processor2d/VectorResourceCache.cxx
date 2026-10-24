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

#include <comphelper/base64.hxx>
#include <comphelper/random.hxx>
#include <o3tl/hash_combine.hxx>
#include <rtl/strbuf.hxx>
#include <tools/json_writer.hxx>

#include <chrono>

namespace drawinglayer
{
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

void VectorResourceCache::writeFont(tools::JsonWriter& rWriter, sal_uInt64 nFontId) const
{
    rWriter.put("type", "vectorrenderingfont");
    rWriter.put("fontId", OString::number(nFontId, 16));

    auto aIterator = maFontCache.find(nFontId);
    if (aIterator == maFontCache.end())
        return;

    const BinaryDataContainer& rBytes = aIterator->second;
    const cpo::uno::Sequence<sal_Int8> aSequence(
        reinterpret_cast<const sal_Int8*>(rBytes.getData()),
        static_cast<sal_Int32>(rBytes.getSize()));
    OStringBuffer aBase64;
    comphelper::Base64::encode(aBase64, aSequence);
    rWriter.put("data", aBase64.makeStringAndClear());
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
