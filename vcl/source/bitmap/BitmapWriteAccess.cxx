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

#include <sal/config.h>
#include <sal/log.hxx>
#include <tools/debug.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bitmap/bmpfast.hxx>

BitmapWriteAccess::BitmapWriteAccess(Bitmap& rBitmap)
    : BitmapReadAccess(rBitmap, BitmapAccessMode::Write)
{
}

BitmapWriteAccess::~BitmapWriteAccess() {}

void BitmapWriteAccess::CopyScanline(tools::Long nY, const BitmapReadAccess& rReadAcc)
{
    assert(nY >= 0 && nY < mpBuffer->mnHeight && "y-coordinate in destination out of range!");
    SAL_WARN_IF(nY >= rReadAcc.Height(), "vcl", "y-coordinate in source out of range!");
    SAL_WARN_IF((!HasPalette() || !rReadAcc.HasPalette())
                    && (HasPalette() || rReadAcc.HasPalette()),
                "vcl", "No copying possible between palette bitmap and TC bitmap!");

    if ((GetScanlineFormat() == rReadAcc.GetScanlineFormat())
        && (GetScanlineSize() >= rReadAcc.GetScanlineSize()))
    {
        memcpy(GetScanline(nY), rReadAcc.GetScanline(nY), rReadAcc.GetScanlineSize());
    }
    else
    {
        tools::Long nWidth = std::min(mpBuffer->mnWidth, rReadAcc.Width());
        if (!ImplFastCopyScanline(nY, *ImplGetBitmapBuffer(), *rReadAcc.ImplGetBitmapBuffer()))
        {
            Scanline pScanline = GetScanline(nY);
            Scanline pScanlineRead = rReadAcc.GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
                SetPixelOnData(pScanline, nX, rReadAcc.GetPixelFromData(pScanlineRead, nX));
        }
    }
}

void BitmapWriteAccess::CopyScanline(tools::Long nY, ConstScanline aSrcScanline,
                                     ScanlineFormat nSrcScanlineFormat, sal_uInt32 nSrcScanlineSize)
{
    const ScanlineFormat nFormat = RemoveScanline(nSrcScanlineFormat);

    assert(nY >= 0 && nY < mpBuffer->mnHeight && "y-coordinate in destination out of range!");
    DBG_ASSERT((HasPalette() && nFormat <= ScanlineFormat::N8BitPal)
                   || (!HasPalette() && nFormat > ScanlineFormat::N8BitPal),
               "No copying possible between palette and non palette scanlines!");

    const sal_uLong nCount = std::min(GetScanlineSize(), nSrcScanlineSize);

    if (!nCount)
        return;

    if (GetScanlineFormat() == RemoveScanline(nSrcScanlineFormat))
        memcpy(GetScanline(nY), aSrcScanline, nCount);
    else
    {
        if (ImplFastCopyScanline(nY, *ImplGetBitmapBuffer(), aSrcScanline, nSrcScanlineFormat,
                                 nSrcScanlineSize))
            return;

        DBG_ASSERT(nFormat != ScanlineFormat::N32BitTcMask,
                   "No support for pixel formats with color masks yet!");
        FncGetPixel pFncGetPixel;
        switch (nFormat)
        {
            case ScanlineFormat::N1BitMsbPal:
                pFncGetPixel = GetPixelForN1BitMsbPal;
                break;
            case ScanlineFormat::N1BitLsbPal:
                pFncGetPixel = GetPixelForN1BitLsbPal;
                break;
            case ScanlineFormat::N4BitMsnPal:
                pFncGetPixel = GetPixelForN4BitMsnPal;
                break;
            case ScanlineFormat::N4BitLsnPal:
                pFncGetPixel = GetPixelForN4BitLsnPal;
                break;
            case ScanlineFormat::N8BitPal:
                pFncGetPixel = GetPixelForN8BitPal;
                break;
            case ScanlineFormat::N24BitTcBgr:
                pFncGetPixel = GetPixelForN24BitTcBgr;
                break;
            case ScanlineFormat::N24BitTcRgb:
                pFncGetPixel = GetPixelForN24BitTcRgb;
                break;
            case ScanlineFormat::N32BitTcAbgr:
                if (Bitmap32IsPreMultipled())
                    pFncGetPixel = GetPixelForN32BitTcAbgr;
                else
                    pFncGetPixel = GetPixelForN32BitTcXbgr;
                break;
            case ScanlineFormat::N32BitTcArgb:
                if (Bitmap32IsPreMultipled())
                    pFncGetPixel = GetPixelForN32BitTcArgb;
                else
                    pFncGetPixel = GetPixelForN32BitTcXrgb;
                break;
            case ScanlineFormat::N32BitTcBgra:
                if (Bitmap32IsPreMultipled())
                    pFncGetPixel = GetPixelForN32BitTcBgra;
                else
                    pFncGetPixel = GetPixelForN32BitTcBgrx;
                break;
            case ScanlineFormat::N32BitTcRgba:
                if (Bitmap32IsPreMultipled())
                    pFncGetPixel = GetPixelForN32BitTcRgba;
                else
                    pFncGetPixel = GetPixelForN32BitTcRgbx;
                break;
            case ScanlineFormat::N32BitTcMask:
                pFncGetPixel = GetPixelForN32BitTcMask;
                break;

            default:
                assert(false);
                pFncGetPixel = nullptr;
                break;
        }

        if (pFncGetPixel)
        {
            const ColorMask aDummyMask;
            Scanline pScanline = GetScanline(nY);
            for (tools::Long nX = 0, nWidth = mpBuffer->mnWidth; nX < nWidth; ++nX)
                SetPixelOnData(pScanline, nX, pFncGetPixel(aSrcScanline, nX, aDummyMask));
        }
    }
}

void BitmapWriteAccess::SetLineColor(const Color& rColor)
{
    if (rColor.GetAlpha() == 0)
    {
        mpLineColor.reset();
    }
    else
    {
        if (HasPalette())
        {
            mpLineColor = BitmapColor(static_cast<sal_uInt8>(GetBestPaletteIndex(rColor)));
        }
        else
        {
            mpLineColor = BitmapColor(rColor);
        }
    }
}

void BitmapWriteAccess::SetFillColor() { mpFillColor.reset(); }

void BitmapWriteAccess::SetFillColor(const Color& rColor)
{
    if (rColor.GetAlpha() == 0)
    {
        mpFillColor.reset();
    }
    else
    {
        if (HasPalette())
        {
            mpFillColor = BitmapColor(static_cast<sal_uInt8>(GetBestPaletteIndex(rColor)));
        }
        else
        {
            mpFillColor = BitmapColor(rColor);
        }
    }
}

void BitmapWriteAccess::Erase(const Color& rColor)
{
    // convert the color format from RGB to palette index if needed
    // TODO: provide and use Erase( BitmapColor& method)
    BitmapColor aColor = rColor;
    if (HasPalette())
    {
        aColor = BitmapColor(static_cast<sal_uInt8>(GetBestPaletteIndex(rColor)));
    }

    // try fast bitmap method first
    if (ImplFastEraseBitmap(*mpBuffer, aColor))
        return;

    tools::Rectangle aRect(Point(), maBitmap.GetSizePixel());
    if (aRect.IsEmpty())
        return;
    // clear the bitmap by filling the first line pixel by pixel,
    // then dup the first line over each other line
    Scanline pFirstScanline = GetScanline(0);
    const tools::Long nEndX = aRect.Right();
    for (tools::Long nX = 0; nX <= nEndX; ++nX)
        SetPixelOnData(pFirstScanline, nX, rColor);
    const auto nScanlineSize = GetScanlineSize();
    const tools::Long nEndY = aRect.Bottom();
    for (tools::Long nY = 1; nY <= nEndY; nY++)
    {
        Scanline pDestScanline = GetScanline(nY);
        memcpy(pDestScanline, pFirstScanline, nScanlineSize);
    }
}

void BitmapWriteAccess::DrawLine(const Point& rStart, const Point& rEnd)
{
    if (!mpLineColor)
        return;

    const BitmapColor& rLineColor = *mpLineColor;
    tools::Long nX, nY;

    if (rStart.X() == rEnd.X())
    {
        // Vertical Line
        const tools::Long nEndY = rEnd.Y();

        nX = rStart.X();
        nY = rStart.Y();

        if (nEndY > nY)
        {
            for (; nY <= nEndY; nY++)
                SetPixel(nY, nX, rLineColor);
        }
        else
        {
            for (; nY >= nEndY; nY--)
                SetPixel(nY, nX, rLineColor);
        }
    }
    else if (rStart.Y() == rEnd.Y())
    {
        // Horizontal Line
        const tools::Long nEndX = rEnd.X();

        nX = rStart.X();
        nY = rStart.Y();

        if (nEndX > nX)
        {
            for (; nX <= nEndX; nX++)
                SetPixel(nY, nX, rLineColor);
        }
        else
        {
            for (; nX >= nEndX; nX--)
                SetPixel(nY, nX, rLineColor);
        }
    }
    else
    {
        const tools::Long nDX = std::abs(rEnd.X() - rStart.X());
        const tools::Long nDY = std::abs(rEnd.Y() - rStart.Y());
        tools::Long nX1;
        tools::Long nY1;
        tools::Long nX2;
        tools::Long nY2;

        if (nDX >= nDY)
        {
            if (rStart.X() < rEnd.X())
            {
                nX1 = rStart.X();
                nY1 = rStart.Y();
                nX2 = rEnd.X();
                nY2 = rEnd.Y();
            }
            else
            {
                nX1 = rEnd.X();
                nY1 = rEnd.Y();
                nX2 = rStart.X();
                nY2 = rStart.Y();
            }

            const tools::Long nDYX = (nDY - nDX) << 1;
            const tools::Long nDY2 = nDY << 1;
            tools::Long nD = nDY2 - nDX;
            bool bPos = nY1 < nY2;

            for (nX = nX1, nY = nY1; nX <= nX2; nX++)
            {
                SetPixel(nY, nX, rLineColor);

                if (nD < 0)
                    nD += nDY2;
                else
                {
                    nD += nDYX;

                    if (bPos)
                        nY++;
                    else
                        nY--;
                }
            }
        }
        else
        {
            if (rStart.Y() < rEnd.Y())
            {
                nX1 = rStart.X();
                nY1 = rStart.Y();
                nX2 = rEnd.X();
                nY2 = rEnd.Y();
            }
            else
            {
                nX1 = rEnd.X();
                nY1 = rEnd.Y();
                nX2 = rStart.X();
                nY2 = rStart.Y();
            }

            const tools::Long nDYX = (nDX - nDY) << 1;
            const tools::Long nDY2 = nDX << 1;
            tools::Long nD = nDY2 - nDY;
            bool bPos = nX1 < nX2;

            for (nX = nX1, nY = nY1; nY <= nY2; nY++)
            {
                SetPixel(nY, nX, rLineColor);

                if (nD < 0)
                    nD += nDY2;
                else
                {
                    nD += nDYX;

                    if (bPos)
                        nX++;
                    else
                        nX--;
                }
            }
        }
    }
}

void BitmapWriteAccess::FillRect(const tools::Rectangle& rRect)
{
    if (!mpFillColor)
        return;

    const BitmapColor& rFillColor = *mpFillColor;
    tools::Rectangle aRect(Point(), maBitmap.GetSizePixel());

    aRect.Intersection(rRect);

    if (aRect.IsEmpty())
        return;

    const tools::Long nStartX = rRect.Left();
    const tools::Long nStartY = rRect.Top();
    const tools::Long nEndX = rRect.Right();
    const tools::Long nEndY = rRect.Bottom();

    for (tools::Long nY = nStartY; nY <= nEndY; nY++)
    {
        Scanline pScanline = GetScanline(nY);
        for (tools::Long nX = nStartX; nX <= nEndX; nX++)
        {
            SetPixelOnData(pScanline, nX, rFillColor);
        }
    }
}

void BitmapWriteAccess::DrawRect(const tools::Rectangle& rRect)
{
    if (mpFillColor)
        FillRect(rRect);

    if (mpLineColor && (!mpFillColor || (*mpFillColor != *mpLineColor)))
    {
        DrawLine(rRect.TopLeft(), rRect.TopRight());
        DrawLine(rRect.TopRight(), rRect.BottomRight());
        DrawLine(rRect.BottomRight(), rRect.BottomLeft());
        DrawLine(rRect.BottomLeft(), rRect.TopLeft());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
