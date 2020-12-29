/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <config_features.h>

#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <tools/debug.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <comphelper/lok.hxx>

#include <vcl/dibtools.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapFilterStackBlur.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bitmap/bmpfast.hxx>
#include <salgdi.hxx>
#include <salbmp.hxx>

#include <cassert>
#include <cstdlib>
#include <memory>

namespace
{
// Co = Cs + Cd*(1-As) premultiplied alpha -or-
// Co = (AsCs + AdCd*(1-As)) / Ao
sal_uInt8 CalcColor(const sal_uInt8 nSourceColor, const sal_uInt8 nSourceAlpha,
                    const sal_uInt8 nDstAlpha, const sal_uInt8 nResAlpha,
                    const sal_uInt8 nDestColor)
{
    int c = nResAlpha ? (static_cast<int>(nSourceAlpha) * nSourceColor
                         + static_cast<int>(nDstAlpha) * nDestColor
                         - static_cast<int>(nDstAlpha) * nDestColor * nSourceAlpha / 255)
                            / static_cast<int>(nResAlpha)
                      : 0;
    return sal_uInt8(c);
}

BitmapColor AlphaBlend(int nX, int nY, const tools::Long nMapX, const tools::Long nMapY,
                       BitmapReadAccess const* pP, BitmapReadAccess const* pA,
                       BitmapReadAccess const* pB, BitmapWriteAccess const* pAlphaW,
                       sal_uInt8& nResAlpha)
{
    BitmapColor aDstCol, aSrcCol;
    aSrcCol = pP->GetColor(nMapY, nMapX);
    aDstCol = pB->GetColor(nY, nX);

    // vcl stores transparency, not alpha - invert it
    const sal_uInt8 nSrcAlpha = 255 - pA->GetPixelIndex(nMapY, nMapX);
    const sal_uInt8 nDstAlpha = 255 - pAlphaW->GetPixelIndex(nY, nX);

    // Perform porter-duff compositing 'over' operation

    // Co = Cs + Cd*(1-As)
    // Ad = As + Ad*(1-As)
    nResAlpha = static_cast<int>(nSrcAlpha) + static_cast<int>(nDstAlpha)
                - static_cast<int>(nDstAlpha) * nSrcAlpha / 255;

    aDstCol.SetRed(CalcColor(aSrcCol.GetRed(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetRed()));
    aDstCol.SetBlue(
        CalcColor(aSrcCol.GetBlue(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetBlue()));
    aDstCol.SetGreen(
        CalcColor(aSrcCol.GetGreen(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetGreen()));

    return aDstCol;
}
}

void OutputDevice::BlendBitmap(const SalTwoRect& rPosAry, const Bitmap& rBmp)
{
    mpGraphics->BlendBitmap(rPosAry, *rBmp.ImplGetSalBitmap(), *this);
}

Bitmap OutputDevice::BlendBitmapWithAlpha(Bitmap& aBmp, BitmapReadAccess const* pP,
                                          BitmapReadAccess const* pA,
                                          const tools::Rectangle& aDstRect, const sal_Int32 nOffY,
                                          const sal_Int32 nDstHeight, const sal_Int32 nOffX,
                                          const sal_Int32 nDstWidth, const tools::Long* pMapX,
                                          const tools::Long* pMapY)

{
    BitmapColor aDstCol;
    Bitmap res;
    int nX, nY;
    sal_uInt8 nResAlpha;

    SAL_WARN_IF(!mpAlphaVDev, "vcl.gdi",
                "BlendBitmapWithAlpha(): call me only with valid alpha VirtualDevice!");

    bool bOldMapMode(mpAlphaVDev->IsMapModeEnabled());
    mpAlphaVDev->EnableMapMode(false);

    Bitmap aAlphaBitmap(mpAlphaVDev->GetBitmap(aDstRect.TopLeft(), aDstRect.GetSize()));
    BitmapScopedWriteAccess pAlphaW(aAlphaBitmap);

    if (GetBitCount() <= 8)
    {
        Bitmap aDither(aBmp.GetSizePixel(), 8);
        BitmapColor aIndex(0);
        Bitmap::ScopedReadAccess pB(aBmp);
        BitmapScopedWriteAccess pW(aDither);

        if (pB && pP && pA && pW && pAlphaW)
        {
            int nOutY;

            for (nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++)
            {
                const tools::Long nMapY = pMapY[nY];
                const tools::Long nModY = (nOutY & 0x0FL) << 4;
                int nOutX;

                Scanline pScanline = pW->GetScanline(nY);
                Scanline pScanlineAlpha = pAlphaW->GetScanline(nY);
                for (nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++)
                {
                    const tools::Long nMapX = pMapX[nX];
                    const sal_uLong nD = nVCLDitherLut[nModY | (nOutX & 0x0FL)];

                    aDstCol = AlphaBlend(nX, nY, nMapX, nMapY, pP, pA, pB.get(), pAlphaW.get(),
                                         nResAlpha);

                    aIndex.SetIndex(static_cast<sal_uInt8>(
                        nVCLRLut[(nVCLLut[aDstCol.GetRed()] + nD) >> 16]
                        + nVCLGLut[(nVCLLut[aDstCol.GetGreen()] + nD) >> 16]
                        + nVCLBLut[(nVCLLut[aDstCol.GetBlue()] + nD) >> 16]));
                    pW->SetPixelOnData(pScanline, nX, aIndex);

                    aIndex.SetIndex(
                        static_cast<sal_uInt8>(nVCLRLut[(nVCLLut[255 - nResAlpha] + nD) >> 16]
                                               + nVCLGLut[(nVCLLut[255 - nResAlpha] + nD) >> 16]
                                               + nVCLBLut[(nVCLLut[255 - nResAlpha] + nD) >> 16]));
                    pAlphaW->SetPixelOnData(pScanlineAlpha, nX, aIndex);
                }
            }
        }
        pB.reset();
        pW.reset();
        res = aDither;
    }
    else
    {
        BitmapScopedWriteAccess pB(aBmp);
        if (pB && pP && pA && pAlphaW)
        {
            for (nY = 0; nY < nDstHeight; nY++)
            {
                const tools::Long nMapY = pMapY[nY];
                Scanline pScanlineB = pB->GetScanline(nY);
                Scanline pScanlineAlpha = pAlphaW->GetScanline(nY);

                for (nX = 0; nX < nDstWidth; nX++)
                {
                    const tools::Long nMapX = pMapX[nX];
                    aDstCol = AlphaBlend(nX, nY, nMapX, nMapY, pP, pA, pB.get(), pAlphaW.get(),
                                         nResAlpha);

                    pB->SetPixelOnData(pScanlineB, nX, pB->GetBestMatchingColor(aDstCol));
                    pAlphaW->SetPixelOnData(
                        pScanlineAlpha, nX,
                        pB->GetBestMatchingColor(
                            Color(255L - nResAlpha, 255L - nResAlpha, 255L - nResAlpha)));
                }
            }
        }
        pB.reset();
        res = aBmp;
    }

    pAlphaW.reset();
    mpAlphaVDev->DrawBitmap(aDstRect.TopLeft(), aAlphaBitmap);
    mpAlphaVDev->EnableMapMode(bOldMapMode);

    return res;
}

Bitmap OutputDevice::BlendBitmap(Bitmap& aBmp, BitmapReadAccess const* pP,
                                 BitmapReadAccess const* pA, const sal_Int32 nOffY,
                                 const sal_Int32 nDstHeight, const sal_Int32 nOffX,
                                 const sal_Int32 nDstWidth, const tools::Rectangle& aBmpRect,
                                 const Size& aOutSz, const bool bHMirr, const bool bVMirr,
                                 const tools::Long* pMapX, const tools::Long* pMapY)
{
    BitmapColor aDstCol;
    Bitmap res;
    int nX, nY;

    if (GetBitCount() <= 8)
    {
        Bitmap aDither(aBmp.GetSizePixel(), 8);
        BitmapColor aIndex(0);
        Bitmap::ScopedReadAccess pB(aBmp);
        BitmapScopedWriteAccess pW(aDither);

        if (pB && pP && pA && pW)
        {
            int nOutY;

            for (nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++)
            {
                tools::Long nMapY = pMapY[nY];
                if (bVMirr)
                {
                    nMapY = aBmpRect.Bottom() - nMapY;
                }
                const tools::Long nModY = (nOutY & 0x0FL) << 4;
                int nOutX;

                Scanline pScanline = pW->GetScanline(nY);
                Scanline pScanlineAlpha = pA->GetScanline(nMapY);
                for (nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++)
                {
                    tools::Long nMapX = pMapX[nX];
                    if (bHMirr)
                    {
                        nMapX = aBmpRect.Right() - nMapX;
                    }
                    const sal_uLong nD = nVCLDitherLut[nModY | (nOutX & 0x0FL)];

                    aDstCol = pB->GetColor(nY, nX);
                    aDstCol.Merge(pP->GetColor(nMapY, nMapX),
                                  pA->GetIndexFromData(pScanlineAlpha, nMapX));
                    aIndex.SetIndex(static_cast<sal_uInt8>(
                        nVCLRLut[(nVCLLut[aDstCol.GetRed()] + nD) >> 16]
                        + nVCLGLut[(nVCLLut[aDstCol.GetGreen()] + nD) >> 16]
                        + nVCLBLut[(nVCLLut[aDstCol.GetBlue()] + nD) >> 16]));
                    pW->SetPixelOnData(pScanline, nX, aIndex);
                }
            }
        }

        pB.reset();
        pW.reset();
        res = aDither;
    }
    else
    {
        BitmapScopedWriteAccess pB(aBmp);

        bool bFastBlend = false;
        if (pP && pA && pB && !bHMirr && !bVMirr)
        {
            SalTwoRect aTR(aBmpRect.Left(), aBmpRect.Top(), aBmpRect.GetWidth(),
                           aBmpRect.GetHeight(), nOffX, nOffY, aOutSz.Width(), aOutSz.Height());

            bFastBlend = ImplFastBitmapBlending(*pB, *pP, *pA, aTR);
        }

        if (pP && pA && pB && !bFastBlend)
        {
            switch (pP->GetScanlineFormat())
            {
                case ScanlineFormat::N8BitPal:
                {
                    for (nY = 0; nY < nDstHeight; nY++)
                    {
                        tools::Long nMapY = pMapY[nY];
                        if (bVMirr)
                        {
                            nMapY = aBmpRect.Bottom() - nMapY;
                        }
                        Scanline pPScan = pP->GetScanline(nMapY);
                        Scanline pAScan = pA->GetScanline(nMapY);
                        Scanline pBScan = pB->GetScanline(nY);

                        for (nX = 0; nX < nDstWidth; nX++)
                        {
                            tools::Long nMapX = pMapX[nX];

                            if (bHMirr)
                            {
                                nMapX = aBmpRect.Right() - nMapX;
                            }
                            aDstCol = pB->GetPixelFromData(pBScan, nX);
                            aDstCol.Merge(pP->GetPaletteColor(pPScan[nMapX]), pAScan[nMapX]);
                            pB->SetPixelOnData(pBScan, nX, aDstCol);
                        }
                    }
                }
                break;

                default:
                {
                    for (nY = 0; nY < nDstHeight; nY++)
                    {
                        tools::Long nMapY = pMapY[nY];

                        if (bVMirr)
                        {
                            nMapY = aBmpRect.Bottom() - nMapY;
                        }
                        Scanline pAScan = pA->GetScanline(nMapY);
                        Scanline pBScan = pB->GetScanline(nY);
                        for (nX = 0; nX < nDstWidth; nX++)
                        {
                            tools::Long nMapX = pMapX[nX];

                            if (bHMirr)
                            {
                                nMapX = aBmpRect.Right() - nMapX;
                            }
                            aDstCol = pB->GetPixelFromData(pBScan, nX);
                            aDstCol.Merge(pP->GetColor(nMapY, nMapX), pAScan[nMapX]);
                            pB->SetPixelOnData(pBScan, nX, aDstCol);
                        }
                    }
                }
                break;
            }
        }

        pB.reset();
        res = aBmp;
    }

    return res;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
