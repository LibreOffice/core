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

#include <vcl/bitmap/BitmapSobelGreyFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <algorithm>

Bitmap BitmapSobelGreyFilter::execute(Bitmap const& rBitmap) const
{
    Bitmap aBitmap(rBitmap);

    if (!aBitmap.ImplMakeGreyscales())
        return Bitmap();

    BitmapScopedReadAccess pReadAcc(aBitmap);
    if (!pReadAcc)
        return Bitmap();

    Bitmap aNewBmp(aBitmap.GetSizePixel(), vcl::PixelFormat::N8_BPP, &pReadAcc->GetPalette());
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return Bitmap();

    BitmapColor aGrey(sal_uInt8(0));
    const sal_Int32 nWidth = pWriteAcc->Width();
    const sal_Int32 nHeight = pWriteAcc->Height();
    const sal_Int32 nMask111 = -1, nMask121 = 0, nMask131 = 1;
    const sal_Int32 nMask211 = -2, nMask221 = 0, nMask231 = 2;
    const sal_Int32 nMask311 = -1, nMask321 = 0, nMask331 = 1;
    const sal_Int32 nMask112 = 1, nMask122 = 2, nMask132 = 1;
    const sal_Int32 nMask212 = 0, nMask222 = 0, nMask232 = 0;
    const sal_Int32 nMask312 = -1, nMask322 = -2, nMask332 = -1;
    sal_Int32 nGrey11, nGrey12, nGrey13;
    sal_Int32 nGrey21, nGrey22, nGrey23;
    sal_Int32 nGrey31, nGrey32, nGrey33;
    std::unique_ptr<long[]> pHMap(new long[nWidth + 2]);
    std::unique_ptr<long[]> pVMap(new long[nHeight + 2]);
    sal_Int32 nX, nY, nSum1, nSum2;

    // fill mapping tables
    pHMap[0] = 0;

    for (nX = 1; nX <= nWidth; nX++)
    {
        pHMap[nX] = nX - 1;
    }

    pHMap[nWidth + 1] = nWidth - 1;

    pVMap[0] = 0;

    for (nY = 1; nY <= nHeight; nY++)
    {
        pVMap[nY] = nY - 1;
    }

    pVMap[nHeight + 1] = nHeight - 1;

    for (nY = 0; nY < nHeight; nY++)
    {
        nGrey11 = pReadAcc->GetPixel(pVMap[nY], pHMap[0]).GetIndex();
        nGrey12 = pReadAcc->GetPixel(pVMap[nY], pHMap[1]).GetIndex();
        nGrey13 = pReadAcc->GetPixel(pVMap[nY], pHMap[2]).GetIndex();
        nGrey21 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[0]).GetIndex();
        nGrey22 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[1]).GetIndex();
        nGrey23 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[2]).GetIndex();
        nGrey31 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[0]).GetIndex();
        nGrey32 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[1]).GetIndex();
        nGrey33 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[2]).GetIndex();

        Scanline pScanline = pWriteAcc->GetScanline(nY);
        for (nX = 0; nX < nWidth; nX++)
        {
            nSum1 = nSum2 = 0;

            nSum1 += nMask111 * nGrey11;
            nSum2 += nMask112 * nGrey11;

            nSum1 += nMask121 * nGrey12;
            nSum2 += nMask122 * nGrey12;

            nSum1 += nMask131 * nGrey13;
            nSum2 += nMask132 * nGrey13;

            nSum1 += nMask211 * nGrey21;
            nSum2 += nMask212 * nGrey21;

            nSum1 += nMask221 * nGrey22;
            nSum2 += nMask222 * nGrey22;

            nSum1 += nMask231 * nGrey23;
            nSum2 += nMask232 * nGrey23;

            nSum1 += nMask311 * nGrey31;
            nSum2 += nMask312 * nGrey31;

            nSum1 += nMask321 * nGrey32;
            nSum2 += nMask322 * nGrey32;

            nSum1 += nMask331 * nGrey33;
            nSum2 += nMask332 * nGrey33;

            nSum1 = static_cast<sal_Int32>(std::hypot(nSum1, nSum2));

            aGrey.SetIndex(
                ~static_cast<sal_uInt8>(std::clamp(nSum1, sal_Int32(0), sal_Int32(255))));
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

    pHMap.reset();
    pVMap.reset();
    pWriteAcc.reset();
    pReadAcc.reset();

    aNewBmp.SetPrefMapMode(aBitmap.GetPrefMapMode());
    aNewBmp.SetPrefSize(aBitmap.GetPrefSize());

    return aNewBmp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
