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

#include "GlowSoftEgdeShadowTools.hxx"
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapFilter.hxx>
#include <vcl/BitmapBasicMorphologyFilter.hxx>
#include <vcl/BitmapFilterStackBlur.hxx>

namespace drawinglayer::primitive2d
{
/* Returns 8-bit alpha mask created from passed mask.

   Negative fErodeDilateRadius values mean erode, positive - dilate.
   nTransparency defines minimal transparency level.
*/
AlphaMask ProcessAndBlurAlphaMask(const Bitmap& rMask, double fErodeDilateRadius,
                                  double fBlurRadius, sal_uInt8 nTransparency, bool bConvertTo1Bit)
{
    // Only completely white pixels on the initial mask must be considered for transparency. Any
    // other color must be treated as black. This creates 1-bit B&W bitmap.
    BitmapEx mask(bConvertTo1Bit ? rMask.CreateMask(COL_WHITE) : rMask);

    // Scaling down increases performance without noticeable quality loss. Additionally,
    // current blur implementation can only handle blur radius between 2 and 254.
    Size aSize = mask.GetSizePixel();
    double fScale = 1.0;
    while (fBlurRadius > 254 || aSize.Height() > 1000 || aSize.Width() > 1000)
    {
        fScale /= 2;
        fBlurRadius /= 2;
        fErodeDilateRadius /= 2;
        aSize.setHeight(aSize.Height() / 2);
        aSize.setWidth(aSize.Width() / 2);
    }

    // BmpScaleFlag::NearestNeighbor is important for following color replacement
    mask.Scale(fScale, fScale, BmpScaleFlag::NearestNeighbor);

    if (fErodeDilateRadius > 0)
        BitmapFilter::Filter(mask, BitmapDilateFilter(fErodeDilateRadius));
    else if (fErodeDilateRadius < 0)
        BitmapFilter::Filter(mask, BitmapErodeFilter(-fErodeDilateRadius, 0xFF));

    if (nTransparency)
    {
        const Color aTransparency(nTransparency, nTransparency, nTransparency);
        mask.Replace(COL_BLACK, aTransparency);
    }

    // We need 8-bit grey mask for blurring
    mask.Convert(BmpConversion::N8BitGreys);

    // calculate blurry effect
    BitmapFilter::Filter(mask, BitmapFilterStackBlur(fBlurRadius));

    mask.Scale(rMask.GetSizePixel());

    return AlphaMask(mask.GetBitmap());
}

drawinglayer::geometry::ViewInformation2D
expandB2DRangeAtViewInformation2D(const drawinglayer::geometry::ViewInformation2D& rViewInfo,
                                  double nAmount)
{
    drawinglayer::geometry::ViewInformation2D aRetval(rViewInfo);
    basegfx::B2DRange viewport(rViewInfo.getViewport());
    viewport.grow(nAmount);
    aRetval.setViewport(viewport);
    return aRetval;
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
