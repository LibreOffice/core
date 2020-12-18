/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/helpers.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapGaussianSeparableBlurFilter.hxx>
#include <vcl/BitmapSeparableUnsharpenFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

BitmapEx BitmapSeparableUnsharpenFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const tools::Long nWidth = aBitmap.GetSizePixel().Width();
    const tools::Long nHeight = aBitmap.GetSizePixel().Height();

    Bitmap aBlur(aBitmap);
    BitmapEx aBlurEx(aBlur);

    BitmapFilter::Filter(aBlurEx, BitmapGaussianSeparableBlurFilter(-mfRadius));
    aBlur = aBlurEx.GetBitmap();

    // Amount of unsharpening effect on image - currently set to a fixed value
    double aAmount = 2.0;

    Bitmap aResultBitmap(Size(nWidth, nHeight), 24);

    Bitmap::ScopedReadAccess pReadAccBlur(aBlur);
    Bitmap::ScopedReadAccess pReadAcc(aBitmap);
    BitmapScopedWriteAccess pWriteAcc(aResultBitmap);

    BitmapColor aColor, aColorBlur;

    // For all pixels in original image subtract pixels values from blurred image
    for (tools::Long y = 0; y < nHeight; y++)
    {
        Scanline pScanline = pWriteAcc->GetScanline(y);
        for (tools::Long x = 0; x < nWidth; x++)
        {
            aColorBlur = pReadAccBlur->GetColor(y, x);
            aColor = pReadAcc->GetColor(y, x);

            BitmapColor aResultColor(
                static_cast<sal_uInt8>(MinMax(
                    aColor.GetRed() + (aColor.GetRed() - aColorBlur.GetRed()) * aAmount, 0, 255)),
                static_cast<sal_uInt8>(MinMax(
                    aColor.GetGreen() + (aColor.GetGreen() - aColorBlur.GetGreen()) * aAmount, 0,
                    255)),
                static_cast<sal_uInt8>(
                    MinMax(aColor.GetBlue() + (aColor.GetBlue() - aColorBlur.GetBlue()) * aAmount,
                           0, 255)));

            pWriteAcc->SetPixelOnData(pScanline, x, aResultColor);
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();
    pReadAccBlur.reset();
    aBitmap.ReassignWithSize(aResultBitmap);

    return BitmapEx(aBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
