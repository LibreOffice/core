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

#include <cstdlib>

#include <vcl/bitmap.hxx>

#include <bmpfast.hxx>
#include <bitmapwriteaccess.hxx>

void BitmapWriteAccess::SetLineColor(const Color& rColor)
{
    if (rColor.GetTransparency() == 255)
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
    if (rColor.GetTransparency() == 255)
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
