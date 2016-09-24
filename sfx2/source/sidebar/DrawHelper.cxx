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

#include <sfx2/sidebar/DrawHelper.hxx>
#include <sfx2/sidebar/Paint.hxx>

#include <vcl/lineinfo.hxx>

namespace sfx2 { namespace sidebar {

void DrawHelper::DrawBorder(vcl::RenderContext& rRenderContext, const Rectangle& rBox, const SvBorder& rBorderSize,
                            const Paint& rHorizontalPaint, const Paint& rVerticalPaint)
{
    // Draw top line.
    DrawHorizontalLine(rRenderContext, rBox.Left(), rBox.Right(),
                       rBox.Top(), rBorderSize.Top(), rHorizontalPaint);

    // Draw bottom line.
    DrawHorizontalLine(rRenderContext, rBox.Left() + rBorderSize.Left(), rBox.Right(),
                       rBox.Bottom() - rBorderSize.Bottom() + 1, rBorderSize.Bottom(),
                       rHorizontalPaint);
    // Draw left line.
    DrawVerticalLine(rRenderContext, rBox.Top() + rBorderSize.Top(), rBox.Bottom(),
                     rBox.Left(), rBorderSize.Left(), rVerticalPaint);
    // Draw right line.
    DrawVerticalLine(rRenderContext, rBox.Top() + rBorderSize.Top(), rBox.Bottom() - rBorderSize.Bottom(),
                     rBox.Right() - rBorderSize.Right() + 1, rBorderSize.Right(), rVerticalPaint);
}

void DrawHelper::DrawHorizontalLine(vcl::RenderContext& rRenderContext, const sal_Int32 nLeft, const sal_Int32 nRight,
                                    const sal_Int32 nY, const sal_Int32 nHeight, const Paint& rPaint)
{
    switch (rPaint.GetType())
    {
        case Paint::ColorPaint:
        {
            const Color aColor(rPaint.GetColor());
            rRenderContext.SetLineColor(aColor);
            for (sal_Int32 nYOffset = 0; nYOffset < nHeight; ++nYOffset)
            {
                rRenderContext.DrawLine(Point(nLeft, nY + nYOffset),
                                        Point(nRight, nY + nYOffset));
            }
            break;
        }
        case Paint::GradientPaint:
            rRenderContext.DrawGradient(Rectangle(nLeft, nY, nRight, nY + nHeight - 1),
                                        rPaint.GetGradient());
            break;

        case Paint::NoPaint:
        default:
            break;
    }
}

void DrawHelper::DrawVerticalLine(vcl::RenderContext& rRenderContext, const sal_Int32 nTop, const sal_Int32 nBottom,
                                  const sal_Int32 nX, const sal_Int32 nWidth, const Paint& rPaint)
{
    switch (rPaint.GetType())
    {
        case Paint::ColorPaint:
        {
            const Color aColor(rPaint.GetColor());
            rRenderContext.SetLineColor(aColor);
            for (sal_Int32 nXOffset = 0; nXOffset < nWidth; ++nXOffset)
            {
                rRenderContext.DrawLine(Point(nX + nXOffset, nTop),
                                        Point(nX + nXOffset, nBottom));
            }
            break;
        }
        case Paint::GradientPaint:
            rRenderContext.DrawGradient(Rectangle(nX, nTop, nX + nWidth - 1, nBottom),
                                        rPaint.GetGradient());
            break;

        case Paint::NoPaint:
        default:
            break;
    }
}

void DrawHelper::DrawRoundedRectangle(vcl::RenderContext& rRenderContext, const Rectangle& rBox, const sal_Int32 nCornerRadius,
                                      const Color& rBorderColor, const Paint& rFillPaint)
{
    rRenderContext.SetLineColor(rBorderColor);
    switch (rFillPaint.GetType())
    {
        case Paint::ColorPaint:
            rRenderContext.SetFillColor(rFillPaint.GetColor());
            rRenderContext.DrawRect(rBox, nCornerRadius, nCornerRadius);
            break;

        case Paint::GradientPaint:
            rRenderContext.DrawGradient(rBox, rFillPaint.GetGradient());
            rRenderContext.SetFillColor();
            rRenderContext.DrawRect(rBox, nCornerRadius, nCornerRadius);
            break;

        case Paint::NoPaint:
        default:
            rRenderContext.SetFillColor();
            rRenderContext.DrawRect(rBox, nCornerRadius, nCornerRadius);
            break;
    }
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
