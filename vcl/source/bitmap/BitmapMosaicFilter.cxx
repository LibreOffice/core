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
#include <vcl/BitmapMosaicFilter.hxx>

#include <bitmapwriteaccess.hxx>

BitmapEx BitmapMosaicFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    bool bRet = false;

    if (mnTileWidth > 1 || mnTileHeight > 1)
    {
        std::unique_ptr<Bitmap> pNewBmp;
        BitmapReadAccess* pReadAcc;
        BitmapWriteAccess* pWriteAcc;

        if (aBitmap.GetBitCount() > 8)
        {
            pReadAcc = pWriteAcc = aBitmap.AcquireWriteAccess();
        }
        else
        {
            pNewBmp.reset(new Bitmap(aBitmap.GetSizePixel(), 24));
            pReadAcc = aBitmap.AcquireReadAccess();
            pWriteAcc = pNewBmp->AcquireWriteAccess();
        }

        bool bConditionsMet = false;
        tools::Long nWidth(0);
        tools::Long nHeight(0);
        if (pReadAcc && pWriteAcc)
        {
            nWidth = pReadAcc->Width();
            nHeight = pReadAcc->Height();
            bConditionsMet = (nWidth > 0 && nHeight > 0);
        }

        if (bConditionsMet)
        {
            BitmapColor aCol;
            tools::Long nX, nY, nX1, nX2, nY1, nY2, nSumR, nSumG, nSumB;
            double fArea_1;

            nY1 = 0;
            nY2 = mnTileHeight - 1;

            if (nY2 >= nHeight)
                nY2 = nHeight - 1;

            do
            {
                nX1 = 0;
                nX2 = mnTileWidth - 1;

                if (nX2 >= nWidth)
                    nX2 = nWidth - 1;

                fArea_1 = 1.0 / ((nX2 - nX1 + 1) * (nY2 - nY1 + 1));

                if (!pNewBmp)
                {
                    do
                    {
                        for (nY = nY1, nSumR = nSumG = nSumB = 0; nY <= nY2; nY++)
                        {
                            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                            for (nX = nX1; nX <= nX2; nX++)
                            {
                                aCol = pReadAcc->GetPixelFromData(pScanlineRead, nX);
                                nSumR += aCol.GetRed();
                                nSumG += aCol.GetGreen();
                                nSumB += aCol.GetBlue();
                            }
                        }

                        aCol.SetRed(static_cast<sal_uInt8>(nSumR * fArea_1));
                        aCol.SetGreen(static_cast<sal_uInt8>(nSumG * fArea_1));
                        aCol.SetBlue(static_cast<sal_uInt8>(nSumB * fArea_1));

                        for (nY = nY1; nY <= nY2; nY++)
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nY);
                            for (nX = nX1; nX <= nX2; nX++)
                                pWriteAcc->SetPixelOnData(pScanline, nX, aCol);
                        }

                        nX1 += mnTileWidth;
                        nX2 += mnTileWidth;

                        if (nX2 >= nWidth)
                        {
                            nX2 = nWidth - 1;
                            fArea_1 = 1.0 / ((nX2 - nX1 + 1) * (nY2 - nY1 + 1));
                        }
                    } while (nX1 < nWidth);
                }
                else
                {
                    do
                    {
                        for (nY = nY1, nSumR = nSumG = nSumB = 0; nY <= nY2; nY++)
                        {
                            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                            for (nX = nX1; nX <= nX2; nX++)
                            {
                                const BitmapColor& rCol = pReadAcc->GetPaletteColor(
                                    pReadAcc->GetIndexFromData(pScanlineRead, nX));
                                nSumR += rCol.GetRed();
                                nSumG += rCol.GetGreen();
                                nSumB += rCol.GetBlue();
                            }
                        }

                        aCol.SetRed(static_cast<sal_uInt8>(nSumR * fArea_1));
                        aCol.SetGreen(static_cast<sal_uInt8>(nSumG * fArea_1));
                        aCol.SetBlue(static_cast<sal_uInt8>(nSumB * fArea_1));

                        for (nY = nY1; nY <= nY2; nY++)
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nY);
                            for (nX = nX1; nX <= nX2; nX++)
                                pWriteAcc->SetPixelOnData(pScanline, nX, aCol);
                        }

                        nX1 += mnTileWidth;
                        nX2 += mnTileWidth;

                        if (nX2 >= nWidth)
                        {
                            nX2 = nWidth - 1;
                            fArea_1 = 1.0 / ((nX2 - nX1 + 1) * (nY2 - nY1 + 1));
                        }
                    } while (nX1 < nWidth);
                }

                nY1 += mnTileHeight;
                nY2 += mnTileHeight;

                if (nY2 >= nHeight)
                    nY2 = nHeight - 1;

            } while (nY1 < nHeight);

            bRet = true;
        }

        Bitmap::ReleaseAccess(pReadAcc);

        if (pNewBmp)
        {
            Bitmap::ReleaseAccess(pWriteAcc);

            if (bRet)
            {
                const MapMode aMap(aBitmap.GetPrefMapMode());
                const Size aPrefSize(aBitmap.GetPrefSize());

                aBitmap = *pNewBmp;

                aBitmap.SetPrefMapMode(aMap);
                aBitmap.SetPrefSize(aPrefSize);
            }
        }
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
