/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapSepiaFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

#include <algorithm>

BitmapEx BitmapSepiaFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());
    Bitmap::ScopedReadAccess pReadAcc(aBitmap);
    bool bRet = false;

    if (pReadAcc)
    {
        const tools::Long nSepia
            = 10000 - 100 * std::clamp(mnSepiaPercent, sal_uInt16(0), sal_uInt16(100));
        BitmapPalette aSepiaPal(256);

        for (sal_uInt16 i = 0; i < 256; i++)
        {
            BitmapColor& rCol = aSepiaPal[i];
            const sal_uInt8 cSepiaValue = static_cast<sal_uInt8>(nSepia * i / 10000);

            rCol.SetRed(static_cast<sal_uInt8>(i));
            rCol.SetGreen(cSepiaValue);
            rCol.SetBlue(cSepiaValue);
        }

        Bitmap aNewBmp(aBitmap.GetSizePixel(), vcl::PixelFormat::N8_BPP, &aSepiaPal);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            BitmapColor aCol(sal_uInt8(0));
            const tools::Long nWidth = pWriteAcc->Width();
            const tools::Long nHeight = pWriteAcc->Height();

            if (pReadAcc->HasPalette())
            {
                const sal_uInt16 nPalCount = pReadAcc->GetPaletteEntryCount();
                std::unique_ptr<sal_uInt8[]> pIndexMap(new sal_uInt8[nPalCount]);
                for (sal_uInt16 i = 0; i < nPalCount; i++)
                {
                    pIndexMap[i] = pReadAcc->GetPaletteColor(i).GetLuminance();
                }

                for (tools::Long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for (tools::Long nX = 0; nX < nWidth; nX++)
                    {
                        aCol.SetIndex(pIndexMap[pReadAcc->GetIndexFromData(pScanlineRead, nX)]);
                        pWriteAcc->SetPixelOnData(pScanline, nX, aCol);
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
                        aCol.SetIndex(pReadAcc->GetPixelFromData(pScanlineRead, nX).GetLuminance());
                        pWriteAcc->SetPixelOnData(pScanline, nX, aCol);
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
            const Size aPrefSize(aBitmap.GetPrefSize());

            aBitmap = aNewBmp;

            aBitmap.SetPrefMapMode(aMap);
            aBitmap.SetPrefSize(aPrefSize);
        }
    }

    if (bRet)
        return rBitmapEx;

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
