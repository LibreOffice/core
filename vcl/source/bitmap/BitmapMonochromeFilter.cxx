/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmap/BitmapMonochromeFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

Bitmap BitmapMonochromeFilter::execute(Bitmap const& rBitmap) const
{
    BitmapScopedReadAccess pReadAcc(rBitmap);
    if (!pReadAcc)
        return Bitmap();

    Bitmap aNewBmp(rBitmap.GetSizePixel(), vcl::PixelFormat::N8_BPP, &Bitmap::GetGreyPalette(256));
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return Bitmap();

    const BitmapColor aBlack(pWriteAcc->GetBestMatchingColor(COL_BLACK));
    const BitmapColor aWhite(pWriteAcc->GetBestMatchingColor(COL_WHITE));
    const sal_Int32 nWidth = pWriteAcc->Width();
    const sal_Int32 nHeight = pWriteAcc->Height();

    if (pReadAcc->HasPalette())
    {
        for (sal_Int32 nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (sal_Int32 nX = 0; nX < nWidth; nX++)
            {
                const sal_uInt8 cIndex = pReadAcc->GetIndexFromData(pScanlineRead, nX);
                if (pReadAcc->GetPaletteColor(cIndex).GetLuminance() >= mcThreshold)
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aWhite);
                }
                else
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aBlack);
                }
            }
        }
    }
    else
    {
        for (sal_Int32 nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (sal_Int32 nX = 0; nX < nWidth; nX++)
            {
                if (pReadAcc->GetPixelFromData(pScanlineRead, nX).GetLuminance() >= mcThreshold)
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aWhite);
                }
                else
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aBlack);
                }
            }
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    aNewBmp.SetPrefMapMode(rBitmap.GetPrefMapMode());
    aNewBmp.SetPrefSize(rBitmap.GetPrefSize());

    return aNewBmp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
