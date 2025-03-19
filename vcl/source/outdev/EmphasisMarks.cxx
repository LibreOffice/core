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
#include <vcl/outdev.hxx>
#include <font/EmphasisMark.hxx>
#include <impglyphitem.hxx>
#include <vcl/vcllayout.hxx>

void OutputDevice::createEmphasisMarks(
    FontEmphasisMark nEmphasisMark, tools::Long nEmphasisHeight, const SalLayout& rSalLayout,
    const std::function<void(const basegfx::B2DPoint&, const basegfx::B2DPolyPolygon&, bool,
                             const tools::Rectangle&, const tools::Rectangle&)>& rCallback) const
{
    // tooling method to create geometry data for EmphasisMarks. It does the same
    // as OutputDevice::ImplDrawEmphasisMarks, but feeds the data into the
    // callback for further usage
    vcl::font::EmphasisMark aEmphasisMark(nEmphasisMark, nEmphasisHeight, GetDPIY());

    Point aOffset(0, 0);
    Point aOffsetVert(0, 0);

    if (nEmphasisMark & FontEmphasisMark::PosBelow)
    {
        aOffset.AdjustY(mpFontInstance->mxFontMetric->GetDescent() + aEmphasisMark.GetYOffset());
        aOffsetVert = aOffset;
    }
    else
    {
        aOffset.AdjustY(-(mpFontInstance->mxFontMetric->GetAscent() + aEmphasisMark.GetYOffset()));
        // Todo: use ideographic em-box or ideographic character face information.
        aOffsetVert.AdjustY(-(mpFontInstance->mxFontMetric->GetAscent()
                              + mpFontInstance->mxFontMetric->GetDescent()
                              + aEmphasisMark.GetYOffset()));
    }

    tools::Long nEmphasisWidth2 = aEmphasisMark.GetWidth() / 2;
    tools::Long nEmphasisHeight2 = nEmphasisHeight / 2;
    aOffset += Point(nEmphasisWidth2, nEmphasisHeight2);

    basegfx::B2DPolyPolygon aShape(aEmphasisMark.GetShape().getB2DPolyPolygon());

    basegfx::B2DPoint aOutPoint;
    basegfx::B2DRectangle aRectangle;
    const GlyphItem* pGlyph;
    const LogicalFontInstance* pGlyphFont;
    int nStart = 0;
    while (rSalLayout.GetNextGlyph(&pGlyph, aOutPoint, nStart, &pGlyphFont))
    {
        if (!pGlyph->GetGlyphBoundRect(pGlyphFont, aRectangle))
            continue;

        if (!pGlyph->IsSpacing())
        {
            Point aAdjPoint;
            if (pGlyph->IsVertical())
            {
                aAdjPoint = aOffsetVert;
                aAdjPoint.AdjustX((-pGlyph->origWidth() + aEmphasisMark.GetWidth()) / 2);
            }
            else
            {
                aAdjPoint = aOffset;
                aAdjPoint.AdjustX(aRectangle.getMinX()
                                  + (aRectangle.getWidth() - aEmphasisMark.GetWidth()) / 2);
            }

            if (mpFontInstance->mnOrientation)
            {
                Point aOriginPt(0, 0);
                aOriginPt.RotateAround(aAdjPoint, mpFontInstance->mnOrientation);
            }
            aOutPoint.adjustX(aAdjPoint.X() - nEmphasisWidth2);
            aOutPoint.adjustY(aAdjPoint.Y() - nEmphasisHeight2);

            // use callback to propagate the data to where it was requested from
            rCallback(aOutPoint, aShape, aEmphasisMark.IsShapePolyLine(), aEmphasisMark.GetRect1(),
                      aEmphasisMark.GetRect2());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
