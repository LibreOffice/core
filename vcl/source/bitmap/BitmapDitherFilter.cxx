/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/helpers.hxx>
#include <tools/long.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapDitherFilter.hxx>

#include <bitmapwriteaccess.hxx>

#include "floyd.hxx"

static BitmapColor getColor(BitmapReadAccess* pReadAcc, tools::Long nZ)
{
    Scanline pScanlineRead = pReadAcc->GetScanline(0);

    if (pReadAcc->HasPalette())
        return pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nZ));
    else
        return pReadAcc->GetPixelFromData(pScanlineRead, nZ);
}

static tools::Long* shiftColor(tools::Long* pColorArray, BitmapColor const& rColor)
{
    *pColorArray++ = static_cast<tools::Long>(rColor.GetBlue()) << 12;
    *pColorArray++ = static_cast<tools::Long>(rColor.GetGreen()) << 12;
    *pColorArray++ = static_cast<tools::Long>(rColor.GetRed()) << 12;

    return pColorArray;
}

static tools::Long* shiftScanlineColors(BitmapReadAccess* pReadAcc, tools::Long nWidth,
                                        tools::Long* pColorArray)
{
    for (tools::Long nZ = 0; nZ < nWidth; nZ++)
    {
        pColorArray = shiftColor(pColorArray, getColor(pReadAcc, nZ));
    }

    return pColorArray;
}

BitmapEx BitmapDitherFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const Size aSize(aBitmap.GetSizePixel());

    if (aSize.Width() == 1 || aSize.Height() == 1)
        return rBitmapEx;

    if (aSize.Width() > 3 && aSize.Height() > 2)
    {
        Bitmap::ScopedReadAccess pReadAcc(aBitmap);
        Bitmap aNewBmp(aBitmap.GetSizePixel(), 8);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pReadAcc && pWriteAcc)
        {
            BitmapColor aColor;
            tools::Long nWidth = pReadAcc->Width();
            tools::Long nWidth1 = nWidth - 1;
            tools::Long nHeight = pReadAcc->Height();
            tools::Long nW = nWidth * 3;
            tools::Long nW2 = nW - 3;
            std::unique_ptr<tools::Long[]> p1(new tools::Long[nW]);
            std::unique_ptr<tools::Long[]> p2(new tools::Long[nW]);
            tools::Long* p1T = p1.get();
            tools::Long* p2T = p2.get();
            tools::Long* pTmp;

            pTmp = p2T;
            pTmp = shiftScanlineColors(pReadAcc.get(), nWidth, pTmp);

            tools::Long nRErr, nGErr, nBErr;
            tools::Long nRC, nGC, nBC;

            for (tools::Long nY = 1, nYAcc = 0; nY <= nHeight; nY++, nYAcc++)
            {
                pTmp = p1T;
                p1T = p2T;
                p2T = pTmp;

                if (nY < nHeight)
                    pTmp = shiftScanlineColors(pReadAcc.get(), nWidth, pTmp);

                // Examine first Pixel separately
                tools::Long nX = 0;
                tools::Long nTemp;
                CALC_ERRORS;
                CALC_TABLES7;
                nX -= 5;
                CALC_TABLES5;
                Scanline pScanline = pWriteAcc->GetScanline(nYAcc);
                pWriteAcc->SetPixelOnData(pScanline, 0,
                                          BitmapColor(static_cast<sal_uInt8>(
                                              nVCLBLut[nBC] + nVCLGLut[nGC] + nVCLRLut[nRC])));

                // Get middle Pixels using a loop
                tools::Long nXAcc;
                for (nX = 3, nXAcc = 1; nX < nW2; nXAcc++)
                {
                    CALC_ERRORS;
                    CALC_TABLES7;
                    nX -= 8;
                    CALC_TABLES3;
                    CALC_TABLES5;
                    pWriteAcc->SetPixelOnData(pScanline, nXAcc,
                                              BitmapColor(static_cast<sal_uInt8>(
                                                  nVCLBLut[nBC] + nVCLGLut[nGC] + nVCLRLut[nRC])));
                }

                // Treat last Pixel separately
                CALC_ERRORS;
                nX -= 5;
                CALC_TABLES3;
                CALC_TABLES5;
                pWriteAcc->SetPixelOnData(pScanline, nWidth1,
                                          BitmapColor(static_cast<sal_uInt8>(
                                              nVCLBLut[nBC] + nVCLGLut[nGC] + nVCLRLut[nRC])));
            }

            pReadAcc.reset();
            pWriteAcc.reset();

            const MapMode aMap(aBitmap.GetPrefMapMode());
            const Size aPrefSize(aBitmap.GetPrefSize());

            aBitmap = aNewBmp;

            aBitmap.SetPrefMapMode(aMap);
            aBitmap.SetPrefSize(aPrefSize);

            return BitmapEx(aBitmap);
        }
    }

    return rBitmapEx;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
