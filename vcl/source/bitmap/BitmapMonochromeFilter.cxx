/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

BitmapEx BitmapMonochromeFilter::execute(BitmapEx const& aBitmapEx) const
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();
    Bitmap::ScopedReadAccess pReadAcc(aBitmap);
    bool bRet = false;

    if (pReadAcc)
    {
        Bitmap aNewBmp(aBitmap.GetSizePixel(), vcl::PixelFormat::N1_BPP);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const BitmapColor aBlack(pWriteAcc->GetBestMatchingColor(COL_BLACK));
            const BitmapColor aWhite(pWriteAcc->GetBestMatchingColor(COL_WHITE));
            const tools::Long nWidth = pWriteAcc->Width();
            const tools::Long nHeight = pWriteAcc->Height();

            if (pReadAcc->HasPalette())
            {
                for (tools::Long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for (tools::Long nX = 0; nX < nWidth; nX++)
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
                for (tools::Long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for (tools::Long nX = 0; nX < nWidth; nX++)
                    {
                        if (pReadAcc->GetPixelFromData(pScanlineRead, nX).GetLuminance()
                            >= mcThreshold)
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
            bRet = true;
        }

        pReadAcc.reset();

        if (bRet)
        {
            const MapMode aMap(aBitmap.GetPrefMapMode());
            const Size aSize(aBitmap.GetPrefSize());

            aBitmap = aNewBmp;

            aBitmap.SetPrefMapMode(aMap);
            aBitmap.SetPrefSize(aSize);
        }
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
