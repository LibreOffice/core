/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmap/BitmapSimpleColorQuantizationFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/Octree.hxx>

Bitmap BitmapSimpleColorQuantizationFilter::execute(Bitmap const& rBitmap) const
{
    Bitmap aBitmap = rBitmap;

    if (vcl::numberOfColors(aBitmap.getPixelFormat()) <= sal_Int64(mnNewColorCount))
        return aBitmap;

    Bitmap aNewBmp;
    BitmapScopedReadAccess pRAcc(aBitmap);
    if (!pRAcc)
        return Bitmap();

    const sal_uInt16 nColorCount = std::min(mnNewColorCount, sal_uInt16(256));
    auto ePixelFormat = vcl::PixelFormat::N8_BPP;

    Octree aOct(*pRAcc, nColorCount);
    const BitmapPalette& rPal = aOct.GetPalette();

    aNewBmp = Bitmap(aBitmap.GetSizePixel(), ePixelFormat, &rPal);
    BitmapScopedWriteAccess pWAcc(aNewBmp);
    if (!pWAcc)
        return Bitmap();

    const sal_Int32 nWidth = pRAcc->Width();
    const sal_Int32 nHeight = pRAcc->Height();

    if (pRAcc->HasPalette())
    {
        for (sal_Int32 nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWAcc->GetScanline(nY);
            Scanline pScanlineRead = pRAcc->GetScanline(nY);
            for (sal_Int32 nX = 0; nX < nWidth; nX++)
            {
                auto c = pRAcc->GetPaletteColor(pRAcc->GetIndexFromData(pScanlineRead, nX));
                pWAcc->SetPixelOnData(
                    pScanline, nX,
                    BitmapColor(static_cast<sal_uInt8>(aOct.GetBestPaletteIndex(c))));
            }
        }
    }
    else
    {
        for (sal_Int32 nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWAcc->GetScanline(nY);
            Scanline pScanlineRead = pRAcc->GetScanline(nY);
            for (sal_Int32 nX = 0; nX < nWidth; nX++)
            {
                auto c = pRAcc->GetPixelFromData(pScanlineRead, nX);
                pWAcc->SetPixelOnData(
                    pScanline, nX,
                    BitmapColor(static_cast<sal_uInt8>(aOct.GetBestPaletteIndex(c))));
            }
        }
    }

    pWAcc.reset();
    pRAcc.reset();

    const MapMode aMap(aBitmap.GetPrefMapMode());
    const Size aSize(aBitmap.GetPrefSize());

    aBitmap = std::move(aNewBmp);

    aBitmap.SetPrefMapMode(aMap);
    aBitmap.SetPrefSize(aSize);

    return aBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
