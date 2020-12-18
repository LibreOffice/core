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
#include <vcl/BitmapSimpleColorQuantizationFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bitmap/Octree.hxx>

BitmapEx BitmapSimpleColorQuantizationFilter::execute(BitmapEx const& aBitmapEx) const
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();

    bool bRet = false;

    if (aBitmap.GetColorCount() <= sal_Int64(mnNewColorCount))
    {
        bRet = true;
    }
    else
    {
        Bitmap aNewBmp;
        Bitmap::ScopedReadAccess pRAcc(aBitmap);
        const sal_uInt16 nColorCount = std::min(mnNewColorCount, sal_uInt16(256));
        sal_uInt16 nBitCount = 0;

        if (nColorCount <= 2)
            nBitCount = 1;
        else if (nColorCount <= 16)
            nBitCount = 4;
        else
            nBitCount = 8;

        if (pRAcc)
        {
            Octree aOct(*pRAcc, nColorCount);
            const BitmapPalette& rPal = aOct.GetPalette();

            aNewBmp = Bitmap(aBitmap.GetSizePixel(), nBitCount, &rPal);
            BitmapScopedWriteAccess pWAcc(aNewBmp);

            if (pWAcc)
            {
                const tools::Long nWidth = pRAcc->Width();
                const tools::Long nHeight = pRAcc->Height();

                if (pRAcc->HasPalette())
                {
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWAcc->GetScanline(nY);
                        Scanline pScanlineRead = pRAcc->GetScanline(nY);
                        for (tools::Long nX = 0; nX < nWidth; nX++)
                        {
                            auto c = pRAcc->GetPaletteColor(
                                pRAcc->GetIndexFromData(pScanlineRead, nX));
                            pWAcc->SetPixelOnData(
                                pScanline, nX,
                                BitmapColor(static_cast<sal_uInt8>(aOct.GetBestPaletteIndex(c))));
                        }
                    }
                }
                else
                {
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWAcc->GetScanline(nY);
                        Scanline pScanlineRead = pRAcc->GetScanline(nY);
                        for (tools::Long nX = 0; nX < nWidth; nX++)
                        {
                            auto c = pRAcc->GetPixelFromData(pScanlineRead, nX);
                            pWAcc->SetPixelOnData(
                                pScanline, nX,
                                BitmapColor(static_cast<sal_uInt8>(aOct.GetBestPaletteIndex(c))));
                        }
                    }
                }

                pWAcc.reset();
                bRet = true;
            }

            pRAcc.reset();
        }

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
