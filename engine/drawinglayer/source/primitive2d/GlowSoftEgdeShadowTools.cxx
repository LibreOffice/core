/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <EuclideanDistanceTransform.hxx>
#include <GaussianGridBlur.hxx>
#include <vcl/bitmap/BitmapBasicMorphologyFilter.hxx>
#include <vcl/bitmap/BitmapFilterStackBlur.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/Scanline.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <algorithm>
#include <cmath>
#include <vector>

namespace drawinglayer::primitive2d
{
/* Returns 8-bit alpha mask created from passed mask.

   Negative fErodeDilateRadius values mean erode, positive - dilate.
   nTransparency defines minimal transparency level.
*/
AlphaMask ProcessAndBlurAlphaMask(const AlphaMask& rMask, double fErodeDilateRadius,
                                  double fBlurRadius, sal_uInt8 nTransparency, bool bConvertTo1Bit)
{
    // Invert it to operate in the transparency domain. Trying to update this method to
    // work in the alpha domain is fraught with hazards.
    AlphaMask tmpMask = rMask;
    tmpMask.Invert();

    // Only completely white pixels on the initial mask must be considered for transparency. Any
    // other color must be treated as black. This creates 1-bit B&W bitmap.
    Bitmap mask = bConvertTo1Bit ? tmpMask.GetBitmap().CreateMask(COL_WHITE) : tmpMask.GetBitmap();

    // Scaling down increases performance without noticeable quality loss. Additionally,
    // current blur implementation can only handle blur radius between 2 and 254.
    Size aSize = mask.GetSizePixel();
    double fScale = 1.0;
    while (fBlurRadius > 254 || aSize.Height() > 1000 || aSize.Width() > 1000)
    {
        fScale /= 2;
        fBlurRadius /= 2;
        fErodeDilateRadius /= 2;
        aSize /= 2;
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

    // And switch to the alpha domain.
    mask.Invert();

    return AlphaMask(mask);
}

namespace
{
// A mask that lets nothing through, so no halo is painted.
AlphaMask createTransparentMask(const Size& rSizePixel)
{
    const sal_uInt8 nFullyTransparent = 255;

    return AlphaMask(rSizePixel, &nFullyTransparent);
}

} // anonymous namespace

AlphaMask CreateGlowAlphaMask(const AlphaMask& rMask, double fGlowRadius, sal_uInt8 nTransparency)
{
    if (fGlowRadius <= 0.0)
        return createTransparentMask(rMask.GetSizePixel());

    Bitmap aMask = rMask.GetBitmap();
    aMask.Convert(BmpConversion::N8BitGreys);

    const tools::Long nWidth = aMask.GetSizePixel().Width();
    const tools::Long nHeight = aMask.GetSizePixel().Height();

    if (nWidth <= 0 || nHeight <= 0)
        return createTransparentMask(rMask.GetSizePixel());

    // Coverage of the object per pixel, and the most any pixel holds, which is its solid opacity.
    std::vector<float> aField(nWidth * nHeight);
    float* pField = aField.data();
    sal_uInt8 nPeakCoverage = 0;

    {
        BitmapScopedReadAccess pRead(aMask);

        if (!pRead)
            return createTransparentMask(rMask.GetSizePixel());

        // Eight bit greys are one byte a pixel, so read them straight off the scanline.
        const bool bBytePerPixel = pRead->GetScanlineFormat() == ScanlineFormat::N8BitPal;
        float* pTarget = pField;

        for (tools::Long nY = 0; nY < nHeight; ++nY)
        {
            Scanline pScanline = pRead->GetScanline(nY);

            for (tools::Long nX = 0; nX < nWidth; ++nX)
            {
                const sal_uInt8 nCoverage
                    = bBytePerPixel ? pScanline[nX] : pRead->GetIndexFromData(pScanline, nX);
                nPeakCoverage = std::max(nPeakCoverage, nCoverage);
                *pTarget++ = float(nCoverage);
            }
        }
    }

    if (nPeakCoverage == 0)
        return createTransparentMask(rMask.GetSizePixel());

    // Half the peak opacity is the object's outline to within a fraction of a pixel, however wide
    // the anti-aliased fringe, and it holds when the whole object is transparent.
    const float fContour = float(nPeakCoverage) * 0.5f;
    const float fFarAway = float(nWidth + nHeight) * float(nWidth + nHeight);

    for (tools::Long nIndex = 0; nIndex < nWidth * nHeight; ++nIndex)
        pField[nIndex] = pField[nIndex] >= fContour ? 0.0f : fFarAway;

    EuclideanDistanceTransform(nWidth, nHeight).execute(pField);

    // Growing by half the radius and softening over the other half is the blurred outline the
    // effect is. The blur shapes the corners: a convex one thins, a concave one fills in.
    const float fSpread = float(fGlowRadius) * 0.5f;
    const double fSigma = fGlowRadius / 6.0;

    // A halo is as opaque as the object's most opaque part. A half transparent shape halves it.
    const float fAmplitude = float(nPeakCoverage) * (1.0f / 255.0f);

    // Half a pixel of ramp each side anti-aliases the grown outline. Only that band needs a square
    // root, the value is flat on either side of it.
    const float fOuterEdge = fSpread + 0.5f;
    const float fOuterSquared = fOuterEdge * fOuterEdge;
    const float fInnerEdge = std::max(fSpread - 0.5f, 0.0f);
    const float fInnerSquared = fInnerEdge * fInnerEdge;

    for (tools::Long nIndex = 0; nIndex < nWidth * nHeight; ++nIndex)
    {
        const float fSquaredDistance = pField[nIndex];

        if (fSquaredDistance >= fOuterSquared)
            pField[nIndex] = 0.0f;
        else if (fSquaredDistance > fInnerSquared)
            pField[nIndex] = fAmplitude * (fOuterEdge - std::sqrt(fSquaredDistance));
        else
            pField[nIndex] = fAmplitude;
    }

    GaussianGridBlur(nWidth, nHeight, fSigma).execute(pField);

    {
        BitmapScopedWriteAccess pWrite(aMask);

        if (!pWrite)
            return createTransparentMask(rMask.GetSizePixel());

        const bool bBytePerPixel = pWrite->GetScanlineFormat() == ScanlineFormat::N8BitPal;
        const float fMaximumAlpha = 255.0f - float(nTransparency);
        const float* pSource = pField;

        for (tools::Long nY = 0; nY < nHeight; ++nY)
        {
            Scanline pScanline = pWrite->GetScanline(nY);

            for (tools::Long nX = 0; nX < nWidth; ++nX)
            {
                const sal_uInt8 nAlpha
                    = static_cast<sal_uInt8>(std::clamp(fMaximumAlpha * *pSource++, 0.0f, 255.0f));

                if (bBytePerPixel)
                    pScanline[nX] = nAlpha;
                else
                    pWrite->SetPixelOnData(pScanline, nX, BitmapColor(nAlpha));
            }
        }
    }

    return AlphaMask(aMask);
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
