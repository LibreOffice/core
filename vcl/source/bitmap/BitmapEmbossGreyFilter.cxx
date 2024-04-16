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

#include <tools/helpers.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapEmbossGreyFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <algorithm>

BitmapEx BitmapEmbossGreyFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    if (!aBitmap.ImplMakeGreyscales())
        return BitmapEx();

    BitmapScopedReadAccess pReadAcc(aBitmap);
    if (!pReadAcc)
        return BitmapEx();

    Bitmap aNewBmp(aBitmap.GetSizePixel(), vcl::PixelFormat::N8_BPP, &pReadAcc->GetPalette());
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return BitmapEx();

    BitmapColor aGrey(sal_uInt8(0));
    const sal_Int32 nWidth = pWriteAcc->Width();
    const sal_Int32 nHeight = pWriteAcc->Height();
    const double fAzim = toRadians(mnAzimuthAngle);
    const double fElev = toRadians(mnElevationAngle);
    std::vector<sal_Int32> pHMap(nWidth + 2);
    std::vector<sal_Int32> pVMap(nHeight + 2);
    const double nLx = cos(fAzim) * cos(fElev) * 255.0;
    const double nLy = sin(fAzim) * cos(fElev) * 255.0;
    const double nLz = sin(fElev) * 255.0;
    const double nNz = 6 * 255.0 / 4;
    const double nNzLz = nNz * nLz;
    const sal_uInt8 cLz = basegfx::fround<sal_uInt8>(nLz);

    // fill mapping tables
    pHMap[0] = 0;

    for (sal_Int32 nX = 1; nX <= nWidth; nX++)
    {
        pHMap[nX] = nX - 1;
    }

    pHMap[nWidth + 1] = nWidth - 1;

    pVMap[0] = 0;

    for (sal_Int32 nY = 1; nY <= nHeight; nY++)
    {
        pVMap[nY] = nY - 1;
    }

    pVMap[nHeight + 1] = nHeight - 1;

    for (sal_Int32 nY = 0; nY < nHeight; nY++)
    {
        sal_Int32 nGrey11 = pReadAcc->GetPixel(pVMap[nY], pHMap[0]).GetIndex();
        sal_Int32 nGrey12 = pReadAcc->GetPixel(pVMap[nY], pHMap[1]).GetIndex();
        sal_Int32 nGrey13 = pReadAcc->GetPixel(pVMap[nY], pHMap[2]).GetIndex();
        sal_Int32 nGrey21 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[0]).GetIndex();
        sal_Int32 nGrey22 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[1]).GetIndex();
        sal_Int32 nGrey23 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[2]).GetIndex();
        sal_Int32 nGrey31 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[0]).GetIndex();
        sal_Int32 nGrey32 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[1]).GetIndex();
        sal_Int32 nGrey33 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[2]).GetIndex();

        Scanline pScanline = pWriteAcc->GetScanline(nY);
        for (sal_Int32 nX = 0; nX < nWidth; nX++)
        {
            const sal_Int32 nNx = nGrey11 + nGrey21 + nGrey31 - nGrey13 - nGrey23 - nGrey33;
            const sal_Int32 nNy = nGrey31 + nGrey32 + nGrey33 - nGrey11 - nGrey12 - nGrey13;

            if (!nNx && !nNy)
            {
                aGrey.SetIndex(cLz);
            }
            else if (double nDotL = nNx * nLx + nNy * nLy + nNzLz; nDotL < 0)
            {
                aGrey.SetIndex(0);
            }
            else
            {
                const double fGrey = nDotL / std::hypot(nNx, nNy, nNz);
                aGrey.SetIndex(basegfx::fround<sal_uInt8>(fGrey));
            }

            pWriteAcc->SetPixelOnData(pScanline, nX, aGrey);

            if (nX < (nWidth - 1))
            {
                const sal_Int32 nNextX = pHMap[nX + 3];

                nGrey11 = nGrey12;
                nGrey12 = nGrey13;
                nGrey13 = pReadAcc->GetPixel(pVMap[nY], nNextX).GetIndex();
                nGrey21 = nGrey22;
                nGrey22 = nGrey23;
                nGrey23 = pReadAcc->GetPixel(pVMap[nY + 1], nNextX).GetIndex();
                nGrey31 = nGrey32;
                nGrey32 = nGrey33;
                nGrey33 = pReadAcc->GetPixel(pVMap[nY + 2], nNextX).GetIndex();
            }
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    const MapMode aMap(aBitmap.GetPrefMapMode());
    const Size aPrefSize(aBitmap.GetPrefSize());

    aBitmap = aNewBmp;

    aBitmap.SetPrefMapMode(aMap);
    aBitmap.SetPrefSize(aPrefSize);

    return BitmapEx(aBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
