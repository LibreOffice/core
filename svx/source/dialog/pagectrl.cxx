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

#include <memory>
#include <editeng/frmdir.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/themecolors.hxx>
#include <tools/color.hxx>
#include <tools/fract.hxx>
#include <tools/mapunit.hxx>
#include <svx/pageitem.hxx>
#include <svx/pagectrl.hxx>
#include <algorithm>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#define CELL_WIDTH      1600L
#define CELL_HEIGHT      800L

SvxPageWindow::SvxPageWindow() :
    m_nTop(0),
    m_nBottom(0),
    m_nLeft(0),
    m_nRight(0),
    m_bResetBackground(false),
    m_bFrameDirection(false),
    m_nFrameDirection(SvxFrameDirection::Horizontal_LR_TB),
    m_nHdLeft(0),
    m_nHdRight(0),
    m_nHdDist(0),
    m_nHdHeight(0),
    m_nFtLeft(0),
    m_nFtRight(0),
    m_nFtDist(0),
    m_nFtHeight(0),
    m_bFooter(false),
    m_bHeader(false),
    m_bTable(false),
    m_bHorz(false),
    m_bVert(false),
    m_eUsage(SvxPageUsage::All)
{
}

SvxPageWindow::~SvxPageWindow()
{
}

void SvxPageWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    auto popIt = rRenderContext.ScopedPush(vcl::PushFlags::MAPMODE);
    rRenderContext.SetMapMode(MapMode(MapUnit::MapTwip));

    Fraction aXScale(m_aWinSize.Width(), std::max(m_aSize.Width() * 2 + m_aSize.Width() / 8, tools::Long(1)));
    Fraction aYScale(m_aWinSize.Height(), std::max(m_aSize.Height(), tools::Long(1)));
    MapMode aMapMode(rRenderContext.GetMapMode());

    if(aYScale < aXScale)
    {
        aMapMode.SetScaleX(aYScale);
        aMapMode.SetScaleY(aYScale);
    }
    else
    {
        aMapMode.SetScaleX(aXScale);
        aMapMode.SetScaleY(aXScale);
    }
    rRenderContext.SetMapMode(aMapMode);
    Size aSz(rRenderContext.PixelToLogic(GetOutputSizePixel()));
    tools::Long nYPos = (aSz.Height() - m_aSize.Height()) / 2;

    if (m_eUsage == SvxPageUsage::All)
    {
        // all pages are equal -> draw one page
        if (m_aSize.Width() > m_aSize.Height())
        {
            // Draw Landscape page of the same size
            Fraction aX = aMapMode.GetScaleX();
            Fraction aY = aMapMode.GetScaleY();
            Fraction a2(1.5);
            aX *= a2;
            aY *= a2;
            aMapMode.SetScaleX(aX);
            aMapMode.SetScaleY(aY);
            rRenderContext.SetMapMode(aMapMode);
            aSz = rRenderContext.PixelToLogic(GetOutputSizePixel());
            nYPos = (aSz.Height() - m_aSize.Height()) / 2;
            tools::Long nXPos = (aSz.Width() - m_aSize.Width()) / 2;
            DrawPage(rRenderContext, Point(nXPos,nYPos),false,true);
        }
        else
            // Portrait
            DrawPage(rRenderContext, Point((aSz.Width() - m_aSize.Width()) / 2,nYPos),false,true);
    }
    else
    {
        // Left and right page are different -> draw two pages if possible
        DrawPage(rRenderContext, Point(0, nYPos), false,
                 m_eUsage == SvxPageUsage::Left || m_eUsage == SvxPageUsage::All || m_eUsage == SvxPageUsage::Mirror);
        DrawPage(rRenderContext, Point(m_aSize.Width() + m_aSize.Width() / 8, nYPos), true,
                 m_eUsage == SvxPageUsage::Right || m_eUsage == SvxPageUsage::All || m_eUsage == SvxPageUsage::Mirror);
    }
}

void SvxPageWindow::DrawPage(vcl::RenderContext& rRenderContext, const Point& rOrg, const bool bSecond, const bool bEnabled)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    const Color& rFieldColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    const Color& rFieldTextColor = rStyleSettings.GetFieldTextColor();
    const Color& rDisableColor = rStyleSettings.GetDisableColor();
    const Color& rDlgColor = rStyleSettings.GetDialogColor();

    // background
    if (!bSecond || m_bResetBackground)
    {
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rDlgColor);
        Size winSize(rRenderContext.GetOutputSize());
        rRenderContext.DrawRect(tools::Rectangle(Point(0,0), winSize));

        if (m_bResetBackground)
            m_bResetBackground = false;
    }
    rRenderContext.SetLineColor(rFieldTextColor);
    rRenderContext.SetTextColor(COL_GRAY);

    // Shadow
    Size aTempSize = m_aSize;

    // Page
    if (!bEnabled)
    {
        rRenderContext.SetFillColor(rDisableColor);
        rRenderContext.DrawRect(tools::Rectangle(rOrg, aTempSize));
        return;
    }
    rRenderContext.SetFillColor(rFieldColor);
    rRenderContext.DrawRect(tools::Rectangle(rOrg, aTempSize));

    tools::Long nL = m_nLeft;
    tools::Long nR = m_nRight;

    if (m_eUsage == SvxPageUsage::Mirror && !bSecond)
    {
        // turn for mirrored
        nL = m_nRight;
        nR = m_nLeft;
    }

    tools::Rectangle aRect;

    aRect.SetLeft( rOrg.X() + nL );
    aRect.SetRight( rOrg.X() + aTempSize.Width() - nR );
    aRect.SetTop( rOrg.Y() + m_nTop );
    aRect.SetBottom( rOrg.Y() + aTempSize.Height() - m_nBottom );

    tools::Rectangle aHdRect(aRect);
    tools::Rectangle aFtRect(aRect);

    if (m_bHeader || m_bFooter)
    {
        // draw PageFill first and on the whole page, no outline
        rRenderContext.SetLineColor();
        drawFillAttributes(rRenderContext, maPageFillAttributes, aRect, aRect);
        rRenderContext.SetLineColor(COL_GRAY);

        if (m_bHeader)
        {
            // show headers if possible
            aHdRect.AdjustLeft(m_nHdLeft );
            aHdRect.AdjustRight( -m_nHdRight );
            aHdRect.SetBottom( aRect.Top() + m_nHdHeight );
            aRect.AdjustTop(m_nHdHeight + m_nHdDist );

            // draw header over PageFill, plus outline
            drawFillAttributes(rRenderContext, maHeaderFillAttributes, aHdRect, aHdRect);
        }

        if (m_bFooter)
        {
            // show footer if possible
            aFtRect.AdjustLeft(m_nFtLeft );
            aFtRect.AdjustRight( -m_nFtRight );
            aFtRect.SetTop( aRect.Bottom() - m_nFtHeight );
            aRect.AdjustBottom( -(m_nFtHeight + m_nFtDist) );

            // draw footer over PageFill, plus outline
            drawFillAttributes(rRenderContext, maFooterFillAttributes, aFtRect, aFtRect);
        }

        // draw page's reduced outline, only outline
        drawFillAttributes(rRenderContext, drawinglayer::attribute::SdrAllFillAttributesHelperPtr(), aRect, aRect);
    }
    else
    {
        // draw PageFill and outline
        drawFillAttributes(rRenderContext, maPageFillAttributes, aRect, aRect);
    }

    if (m_bFrameDirection && !m_bTable)
    {
        Point aPos;
        vcl::Font aFont(rRenderContext.GetFont());
        const Size aSaveSize = aFont.GetFontSize();
        Size aDrawSize(0,aRect.GetHeight() / 6);
        aFont.SetFontSize(aDrawSize);
        rRenderContext.SetFont(aFont);
        OUString sText(u"ABC"_ustr);
        Point aMove(1, rRenderContext.GetTextHeight());
        sal_Unicode cArrow = 0x2193;
        tools::Long nAWidth = rRenderContext.GetTextWidth(sText.copy(0,1));
        switch (m_nFrameDirection)
        {
        case SvxFrameDirection::Horizontal_LR_TB:
            aPos = aRect.TopLeft();
            aPos.AdjustX(rRenderContext.PixelToLogic(Point(1,1)).X() );
            aMove.setY( 0 );
            cArrow = 0x2192;
            break;
        case SvxFrameDirection::Horizontal_RL_TB:
            aPos = aRect.TopRight();
            aPos.AdjustX( -nAWidth );
            aMove.setY( 0 );
            aMove.setX( aMove.X() * -1 );
            cArrow = 0x2190;
            break;
        case SvxFrameDirection::Vertical_LR_TB:
            aPos = aRect.TopLeft();
            aPos.AdjustX(rRenderContext.PixelToLogic(Point(1,1)).X() );
            aMove.setX( 0 );
            break;
        case SvxFrameDirection::Vertical_RL_TB:
            aPos = aRect.TopRight();
            aPos.AdjustX( -nAWidth );
            aMove.setX( 0 );
            break;
        default: break;
        }
        sText += OUStringChar(cArrow);
        for (sal_Int32 i = 0; i < sText.getLength(); i++)
        {
            OUString sDraw(sText.copy(i,1));
            tools::Long nHDiff = 0;
            tools::Long nCharWidth = rRenderContext.GetTextWidth(sDraw);
            bool bHorizontal = 0 == aMove.Y();
            if (!bHorizontal)
            {
                nHDiff = (nAWidth - nCharWidth) / 2;
                aPos.AdjustX(nHDiff );
            }
            rRenderContext.DrawText(aPos,sDraw);
            if (bHorizontal)
            {
                aPos.AdjustX(aMove.X() < 0 ? -nCharWidth : nCharWidth );
            }
            else
            {
                aPos.AdjustX( -nHDiff );
                aPos.AdjustY(aMove.Y() );
            }
        }
        aFont.SetFontSize(aSaveSize);
        rRenderContext.SetFont(aFont);

    }
    if (!m_bTable)
        return;

    // Paint Table, if necessary center it
    rRenderContext.SetLineColor(COL_LIGHTGRAY);

    tools::Long nW = aRect.GetWidth();
    tools::Long nH = aRect.GetHeight();
    tools::Long const nTW = CELL_WIDTH * 3;
    tools::Long const nTH = CELL_HEIGHT * 3;
    tools::Long _nLeft = m_bHorz ? aRect.Left() + ((nW - nTW) / 2) : aRect.Left();
    tools::Long _nTop = m_bVert ? aRect.Top() + ((nH - nTH) / 2) : aRect.Top();
    tools::Rectangle aCellRect(Point(_nLeft, _nTop),Size(CELL_WIDTH, CELL_HEIGHT));

    for (sal_uInt16 i = 0; i < 3; ++i)
    {
        aCellRect.SetLeft( _nLeft );
        aCellRect.SetRight( _nLeft + CELL_WIDTH );
        if(i > 0)
            aCellRect.Move(0,CELL_HEIGHT);

        for (sal_uInt16 j = 0; j < 3; ++j)
        {
            if (j > 0)
                aCellRect.Move(CELL_WIDTH,0);
            rRenderContext.DrawRect(aCellRect);
        }
    }
}

void SvxPageWindow::drawFillAttributes(vcl::RenderContext& rRenderContext,
                                       const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
                                       const tools::Rectangle& rPaintRange,
                                       const tools::Rectangle& rDefineRange)
{
    const basegfx::B2DRange aPaintRange = vcl::unotools::b2DRectangleFromRectangle(rPaintRange);

    if(aPaintRange.isEmpty() ||
       basegfx::fTools::equalZero(aPaintRange.getWidth()) ||
       basegfx::fTools::equalZero(aPaintRange.getHeight()))
        return;

    const basegfx::B2DRange aDefineRange = vcl::unotools::b2DRectangleFromRectangle(rDefineRange);

    // prepare primitive sequence
    drawinglayer::primitive2d::Primitive2DContainer aSequence;

    // create fill geometry if there is something to fill
    if (rFillAttributes && rFillAttributes->isUsed())
    {
        aSequence = rFillAttributes->getPrimitive2DSequence(aPaintRange, aDefineRange);
    }

    // create line geometry if a LineColor is set at the target device
    if (rRenderContext.IsLineColor())
    {
        const drawinglayer::primitive2d::Primitive2DReference xOutline(
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                basegfx::utils::createPolygonFromRect(aPaintRange), rRenderContext.GetLineColor().getBColor()));

        aSequence.push_back(xOutline);
    }

    // draw that if we have something to draw
    if (aSequence.empty())
        return;

    drawinglayer::geometry::ViewInformation2D aViewInformation2D;
    aViewInformation2D.setViewTransformation(rRenderContext.GetViewTransformation());
    aViewInformation2D.setViewport(aPaintRange);

    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createProcessor2DFromOutputDevice(rRenderContext, aViewInformation2D));
    pProcessor->process(aSequence);
}

void SvxPageWindow::EnableFrameDirection(bool bEnable)
{
    m_bFrameDirection = bEnable;
}

void SvxPageWindow::SetFrameDirection(SvxFrameDirection nDirection)
{
    m_nFrameDirection = nDirection;
}

void SvxPageWindow::ResetBackground()
{
    m_bResetBackground = true;
}

void SvxPageWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);

    OutputDevice& rRefDevice = pDrawingArea->get_ref_device();
    // Count in Twips by default
    auto popIt = rRefDevice.ScopedPush(vcl::PushFlags::MAPMODE);
    rRefDevice.SetMapMode(MapMode(MapUnit::MapTwip));
    m_aWinSize = rRefDevice.LogicToPixel(Size(75, 46), MapMode(MapUnit::MapAppFont));
    pDrawingArea->set_size_request(m_aWinSize.Width(), m_aWinSize.Height());

    m_aWinSize.AdjustHeight( -4 );
    m_aWinSize.AdjustWidth( -4 );

    m_aWinSize = rRefDevice.PixelToLogic(m_aWinSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
