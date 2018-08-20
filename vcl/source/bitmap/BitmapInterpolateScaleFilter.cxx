/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/helpers.hxx>

#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>

#include <vcl/bitmapwriteaccess.hxx>
#include <BitmapFastScaleFilter.hxx>
#include <BitmapInterpolateScaleFilter.hxx>

BitmapEx BitmapInterpolateScaleFilter::execute(BitmapEx const& rBitmapEx)
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const Size aSizePix(aBitmap.GetSizePixel());
    const long nNewWidth = FRound(aSizePix.Width() * mfScaleX);
    const long nNewHeight = FRound(aSizePix.Height() * mfScaleY);
    bool bRet = false;

    if ((nNewWidth > 1) && (nNewHeight > 1))
    {
        Bitmap::ScopedReadAccess pReadAcc(aBitmap);
        if (pReadAcc)
        {
            long nWidth = pReadAcc->Width();
            long nHeight = pReadAcc->Height();
            Bitmap aNewBmp(Size(nNewWidth, nHeight), 24);
            BitmapScopedWriteAccess pWriteAcc(aNewBmp);

            if (pWriteAcc)
            {
                const long nNewWidth1 = nNewWidth - 1;
                const long nWidth1 = pReadAcc->Width() - 1;
                const double fRevScaleX = static_cast<double>(nWidth1) / nNewWidth1;

                std::unique_ptr<long[]> pLutInt(new long[nNewWidth]);
                std::unique_ptr<long[]> pLutFrac(new long[nNewWidth]);

                for (long nX = 0, nTemp = nWidth - 2; nX < nNewWidth; nX++)
                {
                    double fTemp = nX * fRevScaleX;
                    pLutInt[nX] = MinMax(static_cast<long>(fTemp), 0, nTemp);
                    fTemp -= pLutInt[nX];
                    pLutFrac[nX] = static_cast<long>(fTemp * 1024.);
                }

                for (long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    if (1 == nWidth)
                    {
                        BitmapColor aCol0;
                        if (pReadAcc->HasPalette())
                        {
                            aCol0 = pReadAcc->GetPaletteColor(
                                pReadAcc->GetIndexFromData(pScanlineRead, 0));
                        }
                        else
                        {
                            aCol0 = pReadAcc->GetPixelFromData(pScanlineRead, 0);
                        }

                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        for (long nX = 0; nX < nNewWidth; nX++)
                        {
                            pWriteAcc->SetPixelOnData(pScanline, nX, aCol0);
                        }
                    }
                    else
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        for (long nX = 0; nX < nNewWidth; nX++)
                        {
                            long nTemp = pLutInt[nX];

                            BitmapColor aCol0, aCol1;
                            if (pReadAcc->HasPalette())
                            {
                                aCol0 = pReadAcc->GetPaletteColor(
                                    pReadAcc->GetIndexFromData(pScanlineRead, nTemp++));
                                aCol1 = pReadAcc->GetPaletteColor(
                                    pReadAcc->GetIndexFromData(pScanlineRead, nTemp));
                            }
                            else
                            {
                                aCol0 = pReadAcc->GetPixelFromData(pScanlineRead, nTemp++);
                                aCol1 = pReadAcc->GetPixelFromData(pScanlineRead, nTemp);
                            }

                            nTemp = pLutFrac[nX];

                            long lXR0 = aCol0.GetRed();
                            long lXG0 = aCol0.GetGreen();
                            long lXB0 = aCol0.GetBlue();
                            long lXR1 = aCol1.GetRed() - lXR0;
                            long lXG1 = aCol1.GetGreen() - lXG0;
                            long lXB1 = aCol1.GetBlue() - lXB0;

                            aCol0.SetRed(
                                static_cast<sal_uInt8>((lXR1 * nTemp + (lXR0 << 10)) >> 10));
                            aCol0.SetGreen(
                                static_cast<sal_uInt8>((lXG1 * nTemp + (lXG0 << 10)) >> 10));
                            aCol0.SetBlue(
                                static_cast<sal_uInt8>((lXB1 * nTemp + (lXB0 << 10)) >> 10));

                            pWriteAcc->SetPixelOnData(pScanline, nX, aCol0);
                        }
                    }
                }

                bRet = true;
            }

            pReadAcc.reset();
            pWriteAcc.reset();

            if (bRet)
            {
                bRet = false;
                const Bitmap aOriginal(aBitmap);
                aBitmap = aNewBmp;
                aNewBmp = Bitmap(Size(nNewWidth, nNewHeight), 24);
                pReadAcc = Bitmap::ScopedReadAccess(aBitmap);
                pWriteAcc = BitmapScopedWriteAccess(aNewBmp);

                if (pReadAcc && pWriteAcc)
                {
                    const long nNewHeight1 = nNewHeight - 1;
                    const long nHeight1 = pReadAcc->Height() - 1;
                    const double fRevScaleY = static_cast<double>(nHeight1) / nNewHeight1;

                    std::unique_ptr<long[]> pLutInt(new long[nNewHeight]);
                    std::unique_ptr<long[]> pLutFrac(new long[nNewHeight]);

                    for (long nY = 0, nTemp = nHeight - 2; nY < nNewHeight; nY++)
                    {
                        double fTemp = nY * fRevScaleY;
                        pLutInt[nY] = MinMax(static_cast<long>(fTemp), 0, nTemp);
                        fTemp -= pLutInt[nY];
                        pLutFrac[nY] = static_cast<long>(fTemp * 1024.);
                    }

                    // after 1st step, bitmap *is* 24bit format (see above)
                    OSL_ENSURE(!pReadAcc->HasPalette(), "OOps, somehow ImplScaleInterpolate "
                                                        "in-between format has palette, should not "
                                                        "happen (!)");

                    for (long nX = 0; nX < nNewWidth; nX++)
                    {
                        if (1 == nHeight)
                        {
                            BitmapColor aCol0 = pReadAcc->GetPixel(0, nX);

                            for (long nY = 0; nY < nNewHeight; nY++)
                            {
                                pWriteAcc->SetPixel(nY, nX, aCol0);
                            }
                        }
                        else
                        {
                            for (long nY = 0; nY < nNewHeight; nY++)
                            {
                                long nTemp = pLutInt[nY];

                                BitmapColor aCol0 = pReadAcc->GetPixel(nTemp++, nX);
                                BitmapColor aCol1 = pReadAcc->GetPixel(nTemp, nX);

                                nTemp = pLutFrac[nY];

                                long lXR0 = aCol0.GetRed();
                                long lXG0 = aCol0.GetGreen();
                                long lXB0 = aCol0.GetBlue();
                                long lXR1 = aCol1.GetRed() - lXR0;
                                long lXG1 = aCol1.GetGreen() - lXG0;
                                long lXB1 = aCol1.GetBlue() - lXB0;

                                aCol0.SetRed(
                                    static_cast<sal_uInt8>((lXR1 * nTemp + (lXR0 << 10)) >> 10));
                                aCol0.SetGreen(
                                    static_cast<sal_uInt8>((lXG1 * nTemp + (lXG0 << 10)) >> 10));
                                aCol0.SetBlue(
                                    static_cast<sal_uInt8>((lXB1 * nTemp + (lXB0 << 10)) >> 10));

                                pWriteAcc->SetPixel(nY, nX, aCol0);
                            }
                        }
                    }

                    bRet = true;
                }

                pReadAcc.reset();
                pWriteAcc.reset();

                if (bRet)
                {
                    aOriginal.AdaptBitCount(aNewBmp);
                    aBitmap = aNewBmp;
                }
            }
        }
    }

    if (!bRet)
    {
        // fallback to fast scale filter
        BitmapEx aBmpEx(aBitmap);
        bRet = BitmapFilter::Filter(aBmpEx, BitmapFastScaleFilter(mfScaleX, mfScaleY));
        aBitmap = aBmpEx.GetBitmap();
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
