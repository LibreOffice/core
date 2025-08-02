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

#include <vcl/bitmap/BitmapGaussianSeparableBlurFilter.hxx>
#include <vcl/bitmap/BitmapSeparableUnsharpenFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

Bitmap BitmapSeparableUnsharpenFilter::execute(Bitmap const& rBitmap) const
{
    Bitmap aBitmap(rBitmap);

    const sal_Int32 nWidth = aBitmap.GetSizePixel().Width();
    const sal_Int32 nHeight = aBitmap.GetSizePixel().Height();

    Bitmap aBlur(aBitmap);

    BitmapFilter::Filter(aBlur, BitmapGaussianSeparableBlurFilter(-mfRadius));

    // Amount of unsharpening effect on image - currently set to a fixed value
    double aAmount = 2.0;

    Bitmap aResultBitmap(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);

    BitmapScopedReadAccess pReadAccBlur(aBlur);
    BitmapScopedReadAccess pReadAcc(aBitmap);
    BitmapScopedWriteAccess pWriteAcc(aResultBitmap);

    BitmapColor aColor, aColorBlur;

    // For all pixels in original image subtract pixels values from blurred image
    for (sal_Int32 y = 0; y < nHeight; y++)
    {
        Scanline pScanline = pWriteAcc->GetScanline(y);
        for (sal_Int32 x = 0; x < nWidth; x++)
        {
            aColorBlur = pReadAccBlur->GetColor(y, x);
            aColor = pReadAcc->GetColor(y, x);

            BitmapColor aResultColor(
                static_cast<sal_uInt8>(
                    std::clamp(aColor.GetRed() + (aColor.GetRed() - aColorBlur.GetRed()) * aAmount,
                               0.0, 255.0)),
                static_cast<sal_uInt8>(std::clamp(
                    aColor.GetGreen() + (aColor.GetGreen() - aColorBlur.GetGreen()) * aAmount, 0.0,
                    255.0)),
                static_cast<sal_uInt8>(std::clamp(
                    aColor.GetBlue() + (aColor.GetBlue() - aColorBlur.GetBlue()) * aAmount, 0.0,
                    255.0)));

            pWriteAcc->SetPixelOnData(pScanline, x, aResultColor);
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();
    pReadAccBlur.reset();
    aBitmap.ReassignWithSize(aResultBitmap);

    return aBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
