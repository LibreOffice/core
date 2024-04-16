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

#include <sal/log.hxx>
#include <tools/helpers.hxx>

#include <vcl/bitmapex.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <bitmap/BitmapFastScaleFilter.hxx>

BitmapEx BitmapFastScaleFilter::execute(BitmapEx const& rBitmapEx) const
{
    SAL_INFO("vcl.gdi", "BitmapFastScaleFilter::execute()");

    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const Size aSizePix(aBitmap.GetSizePixel());
    const sal_Int32 nNewWidth = basegfx::fround(aSizePix.Width() * mfScaleX);
    const sal_Int32 nNewHeight = basegfx::fround(aSizePix.Height() * mfScaleY);
    bool bRet = false;

    SAL_INFO("vcl.gdi", "New width: " << nNewWidth << "\nNew height: " << nNewHeight);

    if (nNewWidth > 0 && nNewHeight > 0)
    {
        BitmapScopedReadAccess pReadAcc(aBitmap);

        if (pReadAcc)
        {
            Bitmap aNewBmp(Size(nNewWidth, nNewHeight), aBitmap.getPixelFormat(),
                           &pReadAcc->GetPalette());
            BitmapScopedWriteAccess pWriteAcc(aNewBmp);

            if (pWriteAcc)
            {
                const sal_Int32 nScanlineSize = pWriteAcc->GetScanlineSize();
                const sal_Int32 nNewHeight1 = nNewHeight - 1;

                if (nNewWidth && nNewHeight)
                {
                    const double nWidth = pReadAcc->Width();
                    const double nHeight = pReadAcc->Height();
                    std::unique_ptr<sal_Int32[]> pLutX(new sal_Int32[nNewWidth]);
                    std::unique_ptr<sal_Int32[]> pLutY(new sal_Int32[nNewHeight]);

                    for (sal_Int32 nX = 0; nX < nNewWidth; nX++)
                    {
                        pLutX[nX] = sal_Int32(nX * nWidth / nNewWidth);
                    }

                    for (sal_Int32 nY = 0; nY < nNewHeight; nY++)
                    {
                        pLutY[nY] = sal_Int32(nY * nHeight / nNewHeight);
                    }

                    sal_Int32 nActY = 0;
                    while (nActY < nNewHeight)
                    {
                        sal_Int32 nMapY = pLutY[nActY];
                        Scanline pScanline = pWriteAcc->GetScanline(nActY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nMapY);

                        for (sal_Int32 nX = 0; nX < nNewWidth; nX++)
                        {
                            pWriteAcc->SetPixelOnData(
                                pScanline, nX,
                                pReadAcc->GetPixelFromData(pScanlineRead, pLutX[nX]));
                        }

                        while ((nActY < nNewHeight1) && (pLutY[nActY + 1] == nMapY))
                        {
                            memcpy(pWriteAcc->GetScanline(nActY + 1), pWriteAcc->GetScanline(nActY),
                                   nScanlineSize);
                            nActY++;
                        }
                        nActY++;
                    }

                    bRet = true;
                }

                pWriteAcc.reset();
            }
            pReadAcc.reset();

            if (bRet)
            {
                aBitmap.ReassignWithSize(aNewBmp);
                SAL_INFO("vcl.gdi", "Bitmap size: " << aBitmap.GetSizePixel());
            }
            else
            {
                SAL_WARN("vcl.gdi", "no resize");
            }
        }
    }

    AlphaMask aMask(rBitmapEx.GetAlphaMask());

    if (bRet && !aMask.IsEmpty())
        bRet = aMask.Scale(maSize, BmpScaleFlag::Fast);

    SAL_WARN_IF(!aMask.IsEmpty() && aBitmap.GetSizePixel() != aMask.GetSizePixel(), "vcl",
                "BitmapEx::Scale(): size mismatch for bitmap and alpha mask.");

    if (bRet)
        return BitmapEx(aBitmap, aMask);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
