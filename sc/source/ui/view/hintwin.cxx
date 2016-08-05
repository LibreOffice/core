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

void ScHintWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /* rRect */)
{
    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> xProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));

    const Rectangle aRect(Rectangle(Point(0, 0), rRenderContext.PixelToLogic(GetSizePixel())));
    basegfx::B2DPolygon aPoly(basegfx::tools::createPolygonFromRect(basegfx::B2DRectangle(aRect.Left(), aRect.Top(),
                                                                                          aRect.Right(), aRect.Bottom())));

    const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
    Color aCommentColor = rColorCfg.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor;
    const drawinglayer::primitive2d::Primitive2DReference aBg(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPoly), aCommentColor.getBColor()));

    basegfx::BColor aBorder(0.5, 0.5, 0.5);
    const drawinglayer::primitive2d::Primitive2DReference aReference(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
            aPoly, aBorder));

    // Create the text primitive for the title
    basegfx::B2DVector aFontSize;
    drawinglayer::attribute::FontAttribute aFontAttr =
        drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, m_aHeadFont, false, false);

    FontMetric aFontMetric = rRenderContext.GetFontMetric(m_aHeadFont);
    double nTextOffsetY = aFontMetric.GetAscent() + HINT_MARGIN;
    Point aTextPos(HINT_MARGIN, nTextOffsetY);

    basegfx::B2DHomMatrix aTextMatrix(basegfx::tools::createScaleTranslateB2DHomMatrix(
                                            aFontSize.getX(), aFontSize.getY(),
                                            double(aTextPos.X()), double(aTextPos.Y())));

    const drawinglayer::primitive2d::Primitive2DReference aTitle(
                    new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                        aTextMatrix, m_aTitle, 0, m_aTitle.getLength(),
                        std::vector<double>(), aFontAttr, css::lang::Locale(),
                        GetLineColor().getBColor()));

    drawinglayer::primitive2d::Primitive2DContainer aSeq { aBg, aReference, aTitle };

    aFontMetric = rRenderContext.GetFontMetric(m_aTextFont);
    nTextOffsetY = aFontMetric.GetAscent();

    aFontAttr = drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, m_aTextFont, false, false);

    sal_Int32 nIndex = 0;
    Point aLineStart = m_aTextStart;
    while ( nIndex != -1 )
    {
        OUString aLine = m_aMessage.getToken( 0, '\r', nIndex );

        aTextMatrix = basegfx::tools::createScaleTranslateB2DHomMatrix(
                                aFontSize.getX(), aFontSize.getY(),
                                aLineStart.X(), aLineStart.Y() + nTextOffsetY);

        // Create the text primitive for each line of text
        const drawinglayer::primitive2d::Primitive2DReference aMessage(
                                        new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                                aTextMatrix, aLine, 0, aLine.getLength(),
                                                std::vector<double>(), aFontAttr, css::lang::Locale(),
                                                GetLineColor().getBColor()));

        aSeq.push_back(aMessage);

        aLineStart.Y() += m_nTextHeight;
    }

    xProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
