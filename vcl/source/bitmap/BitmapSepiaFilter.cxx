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

#include <vcl/bitmap/BitmapSepiaFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <algorithm>

Bitmap BitmapSepiaFilter::execute(Bitmap const& rBitmap) const
{
    Bitmap aBitmap(rBitmap);
    BitmapScopedReadAccess pReadAcc(aBitmap);
    if (!pReadAcc)
        return Bitmap();

    const sal_Int32 nSepia
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
    if (!pWriteAcc)
        return Bitmap();

    BitmapColor aCol(sal_uInt8(0));
    const sal_Int32 nWidth = pWriteAcc->Width();
    const sal_Int32 nHeight = pWriteAcc->Height();

    if (pReadAcc->HasPalette())
    {
        const sal_uInt16 nPalCount = pReadAcc->GetPaletteEntryCount();
        std::unique_ptr<sal_uInt8[]> pIndexMap(new sal_uInt8[nPalCount]);
        for (sal_uInt16 i = 0; i < nPalCount; i++)
        {
            pIndexMap[i] = pReadAcc->GetPaletteColor(i).GetLuminance();
        }

        for (sal_Int32 nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (sal_Int32 nX = 0; nX < nWidth; nX++)
            {
                aCol.SetIndex(pIndexMap[pReadAcc->GetIndexFromData(pScanlineRead, nX)]);
                pWriteAcc->SetPixelOnData(pScanline, nX, aCol);
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
                aCol.SetIndex(pReadAcc->GetPixelFromData(pScanlineRead, nX).GetLuminance());
                pWriteAcc->SetPixelOnData(pScanline, nX, aCol);
            }
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    const MapMode aMap(aBitmap.GetPrefMapMode());
    const Size aPrefSize(aBitmap.GetPrefSize());

    aBitmap = std::move(aNewBmp);

    aBitmap.SetPrefMapMode(aMap);
    aBitmap.SetPrefSize(aPrefSize);

    return aBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
