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
#include <vcl/BitmapMonochromeMatrixFilter.hxx>
#include <vcl/bitmapaccess.hxx>

#include <bitmapwriteaccess.hxx>

static void ImplCreateDitherMatrix(sal_uInt8 (*pDitherMatrix)[16][16])
{
    const double fVal = 3.125;
    const double fVal16 = fVal / 16.;
    const double fValScale = 254.;
    sal_uInt16 pMtx[16][16];
    sal_uInt16 nMax = 0;
    static const sal_uInt8 pMagic[4][4] = { {
                                                0,
                                                14,
                                                3,
                                                13,
                                            },
                                            {
                                                11,
                                                5,
                                                8,
                                                6,
                                            },
                                            {
                                                12,
                                                2,
                                                15,
                                                1,
                                            },
                                            { 7, 9, 4, 10 } };

    // Build MagicSquare
    for (long i = 0; i < 4; i++)
        for (long j = 0; j < 4; j++)
            for (long k = 0; k < 4; k++)
                for (long l = 0; l < 4; l++)
                {
                    pMtx[(k << 2) + i][(l << 2) + j] = static_cast<sal_uInt16>(
                        0.5 + pMagic[i][j] * fVal + pMagic[k][l] * fVal16);
                    nMax = std::max(pMtx[(k << 2) + i][(l << 2) + j], nMax);
                }

    // Scale to interval [0;254]
    double tmp = fValScale / nMax;
    for (long i = 0; i < 16; i++)
        for (long j = 0; j < 16; j++)
            (*pDitherMatrix)[i][j] = static_cast<sal_uInt8>(tmp * pMtx[i][j]);
}

BitmapEx BitmapMonochromeMatrixFilter::execute(BitmapEx const& aBitmapEx) const
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();
    Bitmap::ScopedReadAccess pReadAcc(aBitmap);
    bool bRet = false;

    if (pReadAcc)
    {
        Bitmap aNewBmp(aBitmap.GetSizePixel(), 1);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const BitmapColor aBlack(pWriteAcc->GetBestMatchingColor(COL_BLACK));
            const BitmapColor aWhite(pWriteAcc->GetBestMatchingColor(COL_WHITE));
            const long nWidth = pWriteAcc->Width();
            const long nHeight = pWriteAcc->Height();
            sal_uInt8 pDitherMatrix[16][16];

            ImplCreateDitherMatrix(&pDitherMatrix);

            if (pReadAcc->HasPalette())
            {
                for (long nY = 0; nY < nHeight; nY++)
                {
                    for (long nX = 0, nModY = nY % 16; nX < nWidth; nX++)
                    {
                        const sal_uInt8 cIndex = pReadAcc->GetPixelIndex(nY, nX);
                        if (pReadAcc->GetPaletteColor(cIndex).GetLuminance()
                            > pDitherMatrix[nModY][nX % 16])
                        {
                            pWriteAcc->SetPixel(nY, nX, aWhite);
                        }
                        else
                            pWriteAcc->SetPixel(nY, nX, aBlack);
                    }
                }
            }
            else
            {
                for (long nY = 0; nY < nHeight; nY++)
                {
                    for (long nX = 0, nModY = nY % 16; nX < nWidth; nX++)
                    {
                        if (pReadAcc->GetPixel(nY, nX).GetLuminance()
                            > pDitherMatrix[nModY][nX % 16])
                        {
                            pWriteAcc->SetPixel(nY, nX, aWhite);
                        }
                        else
                            pWriteAcc->SetPixel(nY, nX, aBlack);
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
