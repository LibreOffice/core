/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawinglayer/primitive2d/BitmapAlphaPrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <utility>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
Primitive2DReference BitmapAlphaPrimitive2D::create2DDecomposition(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (basegfx::fTools::equal(getTransparency(), 1.0))
    {
        // completely transparent, done
        return nullptr;
    }

    if (getBitmap().IsEmpty())
    {
        // no geometry, done
        return nullptr;
    }

    if (basegfx::fTools::equalZero(getTransparency()))
    {
        // no transparency, use simple BitmapPrimitive2D
        return Primitive2DReference{ new BitmapPrimitive2D(getBitmap(), getTransform()) };
    }

    // default: embed to UnifiedTransparencePrimitive2D
    Primitive2DContainer aContent{ new BitmapPrimitive2D(getBitmap(), getTransform()) };
    return Primitive2DReference{ new UnifiedTransparencePrimitive2D(std::move(aContent),
                                                                    getTransparency()) };
}

BitmapAlphaPrimitive2D::BitmapAlphaPrimitive2D(BitmapEx xXBitmap, basegfx::B2DHomMatrix aTransform,
                                               double fTransparency)
    : maBitmap(std::move(xXBitmap))
    , maTransform(std::move(aTransform))
    , mfTransparency(std::max(0.0, std::min(1.0, fTransparency)))
{
}

bool BitmapAlphaPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const BitmapAlphaPrimitive2D& rCompare
            = static_cast<const BitmapAlphaPrimitive2D&>(rPrimitive);

        return (getBitmap() == rCompare.getBitmap() && getTransform() == rCompare.getTransform()
                && basegfx::fTools::equal(getTransparency(), rCompare.getTransparency()));
    }

    return false;
}

basegfx::B2DRange
BitmapAlphaPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
    aRetval.transform(maTransform);
    return aRetval;
}

sal_Int64 BitmapAlphaPrimitive2D::estimateUsage()
{
    if (getBitmap().IsEmpty())
    {
        return 0;
    }
    return getBitmap().GetSizeBytes();
}

// provide unique ID
sal_uInt32 BitmapAlphaPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_BITMAPALPHAPRIMITIVE2D;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
