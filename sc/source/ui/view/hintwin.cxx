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

#include "hintwin.hxx"
#include "global.hxx"
#include "overlayobject.hxx"
#include "scmod.hxx"

#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svtools/colorcfg.hxx>

#define HINT_LINESPACE  2
#define HINT_INDENT     3
#define HINT_MARGIN     4

ScHintWindow::ScHintWindow( vcl::Window* pParent, const OUString& rTit, const OUString& rMsg ) :
    Window( pParent, 0 ),
    m_aTitle( rTit ),
    m_aMessage( convertLineEnd(rMsg, LINEEND_CR) )
{
    m_aTextFont = GetFont();
    m_aTextFont.SetTransparent( true );
    m_aTextFont.SetWeight( WEIGHT_NORMAL );
    m_aHeadFont = m_aTextFont;
    m_aHeadFont.SetWeight( WEIGHT_BOLD );

    SetFont( m_aHeadFont );
    Size aHeadSize( GetTextWidth( m_aTitle ), GetTextHeight() );
    SetFont( m_aTextFont );

    Size aTextSize;
    sal_Int32 nIndex = 0;
    while ( nIndex != -1 )
    {
        OUString aLine = m_aMessage.getToken( 0, '\r', nIndex );
        Size aLineSize( GetTextWidth( aLine ), GetTextHeight() );
        m_nTextHeight = aLineSize.Height();
        aTextSize.Height() += m_nTextHeight;
        if ( aLineSize.Width() > aTextSize.Width() )
            aTextSize.Width() = aLineSize.Width();
    }
    aTextSize.Width() += HINT_INDENT;

    m_aTextStart = Point( HINT_MARGIN + HINT_INDENT,
                        aHeadSize.Height() + HINT_MARGIN + HINT_LINESPACE );

    Size aWinSize( std::max( aHeadSize.Width(), aTextSize.Width() ) + 2 * HINT_MARGIN + 1,
                    aHeadSize.Height() + aTextSize.Height() + HINT_LINESPACE + 2 * HINT_MARGIN + 1 );
    SetOutputSizePixel( aWinSize );
}

ScOverlayHint::ScOverlayHint(const OUString& rTit, const OUString& rMsg, const Color& rColor, const vcl::Font& rFont)
    : OverlayObject(rColor)
    , m_aTitle(rTit)
    , m_aMessage(convertLineEnd(rMsg, LINEEND_CR))
    , m_rTextFont(rFont)
{
}

drawinglayer::primitive2d::Primitive2DContainer ScOverlayHint::createOverlayObjectPrimitive2DSequence()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color& rColor = rStyleSettings.GetWindowTextColor();
    vcl::Font aHeadFont = m_rTextFont;
    aHeadFont.SetWeight(WEIGHT_BOLD);

    // Create the text primitive for the title
    basegfx::B2DVector aFontSize;
    drawinglayer::attribute::FontAttribute aFontAttr =
        drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, aHeadFont, false, false);

    OutputDevice* pDefaultDev = Application::GetDefaultDevice();

    FontMetric aFontMetric = pDefaultDev->GetFontMetric(aHeadFont);
    double nTextOffsetY = HINT_MARGIN + aFontMetric.GetAscent();
    Point aTextPos(HINT_MARGIN, nTextOffsetY);
    basegfx::B2DRange aRange(0, 0, HINT_MARGIN, nTextOffsetY);

    basegfx::B2DHomMatrix aTextMatrix(basegfx::tools::createScaleTranslateB2DHomMatrix(
                                            aFontSize.getX(), aFontSize.getY(),
                                            double(aTextPos.X()), double(aTextPos.Y())));

    drawinglayer::primitive2d::TextSimplePortionPrimitive2D* pTitle =
        new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                        aTextMatrix, m_aTitle, 0, m_aTitle.getLength(),
                        std::vector<double>(), aFontAttr, css::lang::Locale(),
                        rColor.getBColor());

    const drawinglayer::primitive2d::Primitive2DReference aTitle(pTitle);

    Point m_aTextStart(HINT_MARGIN + HINT_INDENT, HINT_MARGIN + aFontMetric.GetLineHeight() + HINT_LINESPACE);

    drawinglayer::geometry::ViewInformation2D aDummy;
    aRange.expand(pTitle->getB2DRange(aDummy));

    drawinglayer::primitive2d::Primitive2DContainer aSeq { aTitle };

    aFontMetric = pDefaultDev->GetFontMetric(m_rTextFont);

    nTextOffsetY = aFontMetric.GetAscent();

    aFontAttr = drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, m_rTextFont, false, false);

    sal_Int32 nIndex = 0;
    Point aLineStart = m_aTextStart;
    while (nIndex != -1)
    {
        OUString aLine = m_aMessage.getToken( 0, '\r', nIndex );

        aTextMatrix = basegfx::tools::createScaleTranslateB2DHomMatrix(
                                aFontSize.getX(), aFontSize.getY(),
                                aLineStart.X(), aLineStart.Y() + nTextOffsetY);

        // Create the text primitive for each line of text
        drawinglayer::primitive2d::TextSimplePortionPrimitive2D* pMessage =
                                        new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                                aTextMatrix, aLine, 0, aLine.getLength(),
                                                std::vector<double>(), aFontAttr, css::lang::Locale(),
                                                rColor.getBColor());

        aRange.expand(pMessage->getB2DRange(aDummy));

        const drawinglayer::primitive2d::Primitive2DReference aMessage(pMessage);
        aSeq.push_back(aMessage);

        aLineStart.Y() += aFontMetric.GetLineHeight();
    }

    aRange.grow(HINT_MARGIN);

    basegfx::B2DPolygon aPoly(basegfx::tools::createPolygonFromRect(aRange));

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

void ScHintWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /* rRect */)
{
    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> xProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));

    const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
    Color aCommentColor = rColorCfg.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor;

    ScOverlayHint* pHint = new ScOverlayHint(m_aTitle, m_aMessage, aCommentColor, GetFont());

    drawinglayer::primitive2d::Primitive2DContainer aSeq = pHint->createOverlayObjectPrimitive2DSequence();

    delete pHint;

    xProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
