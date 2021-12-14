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

#include <overlayobject.hxx>

#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <tools/lineend.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/metric.hxx>
#include <sal/log.hxx>

#define HINT_LINESPACE  2
#define HINT_INDENT     3
#define HINT_MARGIN     4

ScOverlayHint::ScOverlayHint(const OUString& rTit, const OUString& rMsg, const Color& rColor, const vcl::Font& rFont)
    : OverlayObject(rColor)
    , m_aTitle(rTit)
    , m_aMessage(convertLineEnd(rMsg, LINEEND_CR))
    , m_aTextFont(rFont)
    , m_aMapMode(MapUnit::MapPixel)
    , m_nLeft(0)
    , m_nTop(0)
{
}

drawinglayer::primitive2d::Primitive2DContainer ScOverlayHint::createOverlaySequence(sal_Int32 nLeft, sal_Int32 nTop,
                                                                                     const MapMode &rMapMode,
                                                                                     basegfx::B2DRange &rRange) const
{
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    MapMode aOld = pDefaultDev->GetMapMode();
    pDefaultDev->SetMapMode(rMapMode);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color& rColor = rStyleSettings.GetLabelTextColor();
    vcl::Font aTextFont = m_aTextFont;
    aTextFont.SetFontSize(pDefaultDev->PixelToLogic(aTextFont.GetFontSize(), rMapMode));
    vcl::Font aHeadFont = aTextFont;
    aHeadFont.SetWeight(WEIGHT_BOLD);

    // Create the text primitive for the title
    basegfx::B2DVector aFontSize;
    drawinglayer::attribute::FontAttribute aFontAttr =
        drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, aHeadFont, false, false);

    FontMetric aFontMetric = pDefaultDev->GetFontMetric(aHeadFont);
    Size aHintMargin = pDefaultDev->PixelToLogic(Size(HINT_MARGIN, HINT_MARGIN), rMapMode);
    Size aIndent = pDefaultDev->PixelToLogic(Size(HINT_INDENT, HINT_LINESPACE), rMapMode);
    double nTextOffsetY = nTop + aHintMargin.Height() + aFontMetric.GetAscent();
    Point aTextPos(nLeft + aHintMargin.Width() , nTextOffsetY);
    rRange = basegfx::B2DRange(nLeft, nTop, nLeft + aHintMargin.Width(), nTop + aHintMargin.Height());

    basegfx::B2DHomMatrix aTextMatrix(basegfx::utils::createScaleTranslateB2DHomMatrix(
                                            aFontSize.getX(), aFontSize.getY(),
                                            aTextPos.X(), aTextPos.Y()));

    rtl::Reference<drawinglayer::primitive2d::TextSimplePortionPrimitive2D> pTitle =
        new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                        aTextMatrix, m_aTitle, 0, m_aTitle.getLength(),
                        std::vector<double>(), aFontAttr, css::lang::Locale(),
                        rColor.getBColor());

    Point aTextStart(nLeft + aHintMargin.Width() + aIndent.Width(),
                     nTop + aHintMargin.Height() + aFontMetric.GetLineHeight() + aIndent.Height());

    drawinglayer::geometry::ViewInformation2D aDummy;
    rRange.expand(pTitle->getB2DRange(aDummy));

    drawinglayer::primitive2d::Primitive2DContainer aSeq { pTitle };

    aFontMetric = pDefaultDev->GetFontMetric(aTextFont);
    pDefaultDev->SetMapMode(aOld);

    nTextOffsetY = aFontMetric.GetAscent();
    sal_Int32 nLineHeight = aFontMetric.GetLineHeight();

    aFontAttr = drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, aTextFont, false, false);

    sal_Int32 nIndex = 0;
    Point aLineStart = aTextStart;
    sal_Int32 nLineCount = 0;
    while (nIndex != -1)
    {
        OUString aLine = m_aMessage.getToken( 0, '\r', nIndex );
        if (aLine.getLength() > 255)
        {
            // prevent silliness higher up from hanging up the program
            SAL_WARN("sc", "ridiculously long line, truncating, len=" << aLine.getLength());
            aLine = aLine.copy(0,255);
        }

        aTextMatrix = basegfx::utils::createScaleTranslateB2DHomMatrix(
                                aFontSize.getX(), aFontSize.getY(),
                                aLineStart.X(), aLineStart.Y() + nTextOffsetY);

        // Create the text primitive for each line of text
        rtl::Reference<drawinglayer::primitive2d::TextSimplePortionPrimitive2D> pMessage =
                                        new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                                aTextMatrix, aLine, 0, aLine.getLength(),
                                                std::vector<double>(), aFontAttr, css::lang::Locale(),
                                                rColor.getBColor());

        rRange.expand(pMessage->getB2DRange(aDummy));

        aSeq.push_back(pMessage);

        aLineStart.AdjustY(nLineHeight );
        nLineCount++;
        if (nLineCount > 50)
        {
            // prevent silliness higher up from hanging up the program
            SAL_WARN("sc", "ridiculously long message, bailing out");
            break;
        }
    }

    rRange.expand(basegfx::B2DTuple(rRange.getMaxX() + aHintMargin.Width(),
                                    rRange.getMaxY() + aHintMargin.Height()));

    basegfx::B2DPolygon aPoly(basegfx::utils::createPolygonFromRect(rRange));

    const drawinglayer::primitive2d::Primitive2DReference aBg(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPoly), getBaseColor().getBColor()));

    basegfx::BColor aBorderColor(0.5, 0.5, 0.5);
    const drawinglayer::primitive2d::Primitive2DReference aBorder(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
            aPoly, aBorderColor));

    aSeq.insert(aSeq.begin(), aBorder);
    aSeq.insert(aSeq.begin(), aBg);

    return aSeq;
}

drawinglayer::primitive2d::Primitive2DContainer ScOverlayHint::createOverlayObjectPrimitive2DSequence()
{
    basegfx::B2DRange aRange;
    return createOverlaySequence(m_nLeft, m_nTop, m_aMapMode, aRange);
}

Size ScOverlayHint::GetSizePixel() const
{
    basegfx::B2DRange aRange;
    createOverlaySequence(0, 0, MapMode(MapUnit::MapPixel), aRange);
    return Size(aRange.getWidth(), aRange.getHeight());
}

void ScOverlayHint::SetPos(const Point& rPos, const MapMode& rMode)
{
    m_nLeft = rPos.X();
    m_nTop = rPos.Y();
    m_aMapMode = rMode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
