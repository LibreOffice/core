/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "DrawHelper.hxx"
#include "Paint.hxx"

#include <vcl/lineinfo.hxx>

namespace sfx2 { namespace sidebar {

void DrawHelper::DrawBorder (
    OutputDevice& rDevice,
    const Rectangle rBox,
    const SvBorder aBorderSize,
    const Paint& rHorizontalPaint,
    const Paint& rVerticalPaint)
{
    
    DrawHorizontalLine(
        rDevice,
        rBox.Left(),
        rBox.Right(),
        rBox.Top(),
        aBorderSize.Top(),
        rHorizontalPaint);
    
    DrawHorizontalLine(
        rDevice,
        rBox.Left()+aBorderSize.Left(),
        rBox.Right(),
        rBox.Bottom()-aBorderSize.Bottom()+1,
        aBorderSize.Bottom(),
        rHorizontalPaint);
    
    DrawVerticalLine(
        rDevice,
        rBox.Top()+aBorderSize.Top(),
        rBox.Bottom(),
        rBox.Left(),
        aBorderSize.Left(),
        rVerticalPaint);
    
    DrawVerticalLine(
        rDevice,
        rBox.Top()+aBorderSize.Top(),
        rBox.Bottom()-aBorderSize.Bottom(),
        rBox.Right()-aBorderSize.Right()+1,
        aBorderSize.Right(),
        rVerticalPaint);
}

void DrawHelper::DrawHorizontalLine(
    OutputDevice& rDevice,
    const sal_Int32 nLeft,
    const sal_Int32 nRight,
    const sal_Int32 nY,
    const sal_Int32 nHeight,
    const Paint& rPaint)
{
    switch (rPaint.GetType())
    {
        case Paint::NoPaint:
        default:
            break;

        case Paint::ColorPaint:
        {
            const Color aColor (rPaint.GetColor());
            rDevice.SetLineColor(aColor);
            for (sal_Int32 nYOffset=0; nYOffset<nHeight; ++nYOffset)
                rDevice.DrawLine(
                    Point(nLeft,nY+nYOffset),
                    Point(nRight,nY+nYOffset));
            break;
        }
        case Paint::GradientPaint:
            rDevice.DrawGradient(
                Rectangle(
                    nLeft,
                    nY,
                    nRight,
                    nY+nHeight-1),
                rPaint.GetGradient());
            break;
    }
}

void DrawHelper::DrawVerticalLine(
    OutputDevice& rDevice,
    const sal_Int32 nTop,
    const sal_Int32 nBottom,
    const sal_Int32 nX,
    const sal_Int32 nWidth,
    const Paint& rPaint)
{
    switch (rPaint.GetType())
    {
        case Paint::NoPaint:
        default:
            break;

        case Paint::ColorPaint:
        {
            const Color aColor (rPaint.GetColor());
            rDevice.SetLineColor(aColor);
            for (sal_Int32 nXOffset=0; nXOffset<nWidth; ++nXOffset)
                rDevice.DrawLine(
                    Point(nX+nXOffset, nTop),
                    Point(nX+nXOffset, nBottom));
            break;
        }
        case Paint::GradientPaint:
            rDevice.DrawGradient(
                Rectangle(
                    nX,
                    nTop,
                    nX+nWidth-1,
                    nBottom),
                rPaint.GetGradient());
            break;
    }
}

void DrawHelper::DrawRoundedRectangle (
    OutputDevice& rDevice,
    const Rectangle& rBox,
    const sal_Int32 nCornerRadius,
    const Color& rBorderColor,
    const Paint& rFillPaint)
{
    rDevice.SetLineColor(rBorderColor);
    switch(rFillPaint.GetType())
    {
        case Paint::NoPaint:
        default:
            rDevice.SetFillColor();
            rDevice.DrawRect(rBox, nCornerRadius, nCornerRadius);
            break;

        case Paint::ColorPaint:
            rDevice.SetFillColor(rFillPaint.GetColor());
            rDevice.DrawRect(rBox, nCornerRadius, nCornerRadius);
            break;

        case Paint::GradientPaint:
            rDevice.DrawGradient(
                rBox,
                rFillPaint.GetGradient());
            rDevice.SetFillColor();
            rDevice.DrawRect(rBox, nCornerRadius, nCornerRadius);
            break;
    }
}

} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
