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
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapMedianColorQuantizationFilter.hxx>

#include <bitmapwriteaccess.hxx>
#include <impoctree.hxx>

#include <cstdlib>

BitmapEx BitmapMedianColorQuantizationFilter::execute(BitmapEx const& aBitmapEx)
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();

    bool bRet = false;

    if (aBitmap.GetColorCount() <= static_cast<sal_uLong>(mnNewColorCount))
    {
        bRet = true;
    }
    else
    {
        Bitmap::ScopedReadAccess pRAcc(aBitmap);
        sal_uInt16 nBitCount;

        if (mnNewColorCount < 17)
        {
            nBitCount = 4;
        }
        else if (mnNewColorCount < 257)
        {
            nBitCount = 8;
        }
        else
        {
            OSL_FAIL("Bitmap::ImplReduceMedian(): invalid color count!");
            nBitCount = 8;
            mnNewColorCount = 256;
        }

        if (pRAcc)
        {
            Bitmap aNewBmp(aBitmap.GetSizePixel(), nBitCount);
            BitmapScopedWriteAccess pWAcc(aNewBmp);

            if (pWAcc)
            {
                const sal_uLong nSize = 32768 * sizeof(sal_uLong);
                sal_uLong* pColBuf = static_cast<sal_uLong*>(rtl_allocateMemory(nSize));
                const long nWidth = pWAcc->Width();
                const long nHeight = pWAcc->Height();
                long nIndex = 0;

                memset(pColBuf, 0, nSize);

                // create Buffer
                if (pRAcc->HasPalette())
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanlineRead = pRAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            const BitmapColor& rCol = pRAcc->GetPaletteColor(
                                pRAcc->GetIndexFromData(pScanlineRead, nX));

                            pColBuf[RGB15(rCol.GetRed() >> 3, rCol.GetGreen() >> 3,
                                          rCol.GetBlue() >> 3)]++;
                        }
                    }
                }
                else
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanlineRead = pRAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            const BitmapColor aCol(pRAcc->GetPixelFromData(pScanlineRead, nX));
                            pColBuf[RGB15(aCol.GetRed() >> 3, aCol.GetGreen() >> 3,
                                          aCol.GetBlue() >> 3)]++;
                        }
                    }
                }

                // create palette via median cut
                BitmapPalette aPal(pWAcc->GetPaletteEntryCount());
                medianCut(aBitmap, pColBuf, aPal, 0, 31, 0, 31, 0, 31, mnNewColorCount,
                          nWidth * nHeight, nIndex);

                // do mapping of colors to palette
                InverseColorMap aMap(aPal);
                pWAcc->SetPalette(aPal);
                for (long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pWAcc->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; nX++)
                    {
                        pWAcc->SetPixelOnData(
                            pScanline, nX,
                            BitmapColor(static_cast<sal_uInt8>(
                                aMap.GetBestPaletteIndex(pRAcc->GetColor(nY, nX)))));
                    }
                }

                rtl_freeMemory(pColBuf);
                pWAcc.reset();
                bRet = true;
            }

            pRAcc.reset();
            if (bRet)
            {
                const MapMode aMap(aBitmap.GetPrefMapMode());
                const Size aSize(aBitmap.GetPrefSize());

                aBitmap = aNewBmp;

                aBitmap.SetPrefMapMode(aMap);
                aBitmap.SetPrefSize(aSize);
            }
        }
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

void BitmapMedianColorQuantizationFilter::medianCut(Bitmap& rBitmap, sal_uLong* pColBuf,
                                                    BitmapPalette& rPal, long nR1, long nR2,
                                                    long nG1, long nG2, long nB1, long nB2,
                                                    long nColors, long nPixels, long& rIndex)
{
    if (!nPixels)
        return;

    BitmapColor aCol;
    const long nRLen = nR2 - nR1;
    const long nGLen = nG2 - nG1;
    const long nBLen = nB2 - nB1;
    sal_uLong* pBuf = pColBuf;

    if (!nRLen && !nGLen && !nBLen)
    {
        if (pBuf[RGB15(nR1, nG1, nB1)])
        {
            aCol.SetRed(static_cast<sal_uInt8>(nR1 << 3));
            aCol.SetGreen(static_cast<sal_uInt8>(nG1 << 3));
            aCol.SetBlue(static_cast<sal_uInt8>(nB1 << 3));
            rPal[static_cast<sal_uInt16>(rIndex++)] = aCol;
        }
    }
    else
    {
        if (nColors == 1 || nPixels == 1)
        {
            long nPixSum = 0, nRSum = 0, nGSum = 0, nBSum = 0;

            for (long nR = nR1; nR <= nR2; nR++)
            {
                for (long nG = nG1; nG <= nG2; nG++)
                {
                    for (long nB = nB1; nB <= nB2; nB++)
                    {
                        nPixSum = pBuf[RGB15(nR, nG, nB)];

                        if (nPixSum)
                        {
                            nRSum += nR * nPixSum;
                            nGSum += nG * nPixSum;
                            nBSum += nB * nPixSum;
                        }
                    }
                }
            }

            aCol.SetRed(static_cast<sal_uInt8>((nRSum / nPixels) << 3));
            aCol.SetGreen(static_cast<sal_uInt8>((nGSum / nPixels) << 3));
            aCol.SetBlue(static_cast<sal_uInt8>((nBSum / nPixels) << 3));
            rPal[static_cast<sal_uInt16>(rIndex++)] = aCol;
        }
        else
        {
            const long nTest = (nPixels >> 1);
            long nPixOld = 0;
            long nPixNew = 0;

            if (nBLen > nGLen && nBLen > nRLen)
            {
                long nB = nB1 - 1;

                while (nPixNew < nTest)
                {
                    nB++;
                    nPixOld = nPixNew;
                    for (long nR = nR1; nR <= nR2; nR++)
                    {
                        for (long nG = nG1; nG <= nG2; nG++)
                        {
                            nPixNew += pBuf[RGB15(nR, nG, nB)];
                        }
                    }
                }

                if (nB < nB2)
                {
                    medianCut(rBitmap, pBuf, rPal, nR1, nR2, nG1, nG2, nB1, nB, nColors >> 1,
                              nPixNew, rIndex);
                    medianCut(rBitmap, pBuf, rPal, nR1, nR2, nG1, nG2, nB + 1, nB2, nColors >> 1,
                              nPixels - nPixNew, rIndex);
                }
                else
                {
                    medianCut(rBitmap, pBuf, rPal, nR1, nR2, nG1, nG2, nB1, nB - 1, nColors >> 1,
                              nPixOld, rIndex);
                    medianCut(rBitmap, pBuf, rPal, nR1, nR2, nG1, nG2, nB, nB2, nColors >> 1,
                              nPixels - nPixOld, rIndex);
                }
            }
            else if (nGLen > nRLen)
            {
                long nG = nG1 - 1;

                while (nPixNew < nTest)
                {
                    nG++;
                    nPixOld = nPixNew;
                    for (long nR = nR1; nR <= nR2; nR++)
                    {
                        for (long nB = nB1; nB <= nB2; nB++)
                        {
                            nPixNew += pBuf[RGB15(nR, nG, nB)];
                        }
                    }
                }

                if (nG < nG2)
                {
                    medianCut(rBitmap, pBuf, rPal, nR1, nR2, nG1, nG, nB1, nB2, nColors >> 1,
                              nPixNew, rIndex);
                    medianCut(rBitmap, pBuf, rPal, nR1, nR2, nG + 1, nG2, nB1, nB2, nColors >> 1,
                              nPixels - nPixNew, rIndex);
                }
                else
                {
                    medianCut(rBitmap, pBuf, rPal, nR1, nR2, nG1, nG - 1, nB1, nB2, nColors >> 1,
                              nPixOld, rIndex);
                    medianCut(rBitmap, pBuf, rPal, nR1, nR2, nG, nG2, nB1, nB2, nColors >> 1,
                              nPixels - nPixOld, rIndex);
                }
            }
            else
            {
                long nR = nR1 - 1;

                while (nPixNew < nTest)
                {
                    nR++;
                    nPixOld = nPixNew;
                    for (long nG = nG1; nG <= nG2; nG++)
                    {
                        for (long nB = nB1; nB <= nB2; nB++)
                        {
                            nPixNew += pBuf[RGB15(nR, nG, nB)];
                        }
                    }
                }

                if (nR < nR2)
                {
                    medianCut(rBitmap, pBuf, rPal, nR1, nR, nG1, nG2, nB1, nB2, nColors >> 1,
                              nPixNew, rIndex);
                    medianCut(rBitmap, pBuf, rPal, nR1 + 1, nR2, nG1, nG2, nB1, nB2, nColors >> 1,
                              nPixels - nPixNew, rIndex);
                }
                else
                {
                    medianCut(rBitmap, pBuf, rPal, nR1, nR - 1, nG1, nG2, nB1, nB2, nColors >> 1,
                              nPixOld, rIndex);
                    medianCut(rBitmap, pBuf, rPal, nR, nR2, nG1, nG2, nB1, nB2, nColors >> 1,
                              nPixels - nPixOld, rIndex);
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
