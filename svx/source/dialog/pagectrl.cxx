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
#include <vcl/bitmap.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>
#include <tools/fract.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/pageitem.hxx>
#include <svx/pagectrl.hxx>
#include <editeng/boxitem.hxx>
#include <algorithm>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#define CELL_WIDTH      1600L
#define CELL_HEIGHT      800L

SvxPageWindow::SvxPageWindow() :
    aWinSize(),
    aSize(),
    nTop(0),
    nBottom(0),
    nLeft(0),
    nRight(0),
    bResetBackground(false),
    bFrameDirection(false),
    nFrameDirection(SvxFrameDirection::Horizontal_LR_TB),
    nHdLeft(0),
    nHdRight(0),
    nHdDist(0),
    nHdHeight(0),
    nFtLeft(0),
    nFtRight(0),
    nFtDist(0),
    nFtHeight(0),
    maHeaderFillAttributes(),
    maFooterFillAttributes(),
    maPageFillAttributes(),
    bFooter(false),
    bHeader(false),
    bTable(false),
    bHorz(false),
    bVert(false),
    eUsage(SvxPageUsage::All)
{
}

SvxPageWindow::~SvxPageWindow()
{
}

void SvxPageWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push(PushFlags::MAPMODE);
    rRenderContext.SetMapMode(MapMode(MapUnit::MapTwip));

    Fraction aXScale(aWinSize.Width(), std::max(long(aSize.Width() * 2 + aSize.Width() / 8), 1L));
    Fraction aYScale(aWinSize.Height(), std::max(aSize.Height(), 1L));
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
    long nYPos = (aSz.Height() - aSize.Height()) / 2;

    if (eUsage == SvxPageUsage::All)
    {
        // all pages are equal -> draw one page
        if (aSize.Width() > aSize.Height())
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
            nYPos = (aSz.Height() - aSize.Height()) / 2;
            long nXPos = (aSz.Width() - aSize.Width()) / 2;
            DrawPage(rRenderContext, Point(nXPos,nYPos),true,true);
        }
        else
            // Portrait
            DrawPage(rRenderContext, Point((aSz.Width() - aSize.Width()) / 2,nYPos),true,true);
    }
    else
    {
        // Left and right page are different -> draw two pages if possible
        DrawPage(rRenderContext, Point(0, nYPos), false,
                 eUsage == SvxPageUsage::Left || eUsage == SvxPageUsage::All || eUsage == SvxPageUsage::Mirror);
        DrawPage(rRenderContext, Point(aSize.Width() + aSize.Width() / 8, nYPos), true,
                 eUsage == SvxPageUsage::Right || eUsage == SvxPageUsage::All || eUsage == SvxPageUsage::Mirror);
    }
    rRenderContext.Pop();
}

void SvxPageWindow::DrawPage(vcl::RenderContext& rRenderContext, const Point& rOrg, const bool bSecond, const bool bEnabled)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    const Color& rFieldColor = rStyleSettings.GetFieldColor();
    const Color& rFieldTextColor = rStyleSettings.GetFieldTextColor();
    const Color& rDisableColor = rStyleSettings.GetDisableColor();
    const Color& rDlgColor = rStyleSettings.GetDialogColor();

    // background
    if (!bSecond || bResetBackground)
    {
        rRenderContext.SetLineColor(COL_TRANSPARENT);
        rRenderContext.SetFillColor(rDlgColor);
        Size winSize(rRenderContext.GetOutputSize());
        rRenderContext.DrawRect(tools::Rectangle(Point(0,0), winSize));

        if (bResetBackground)
            bResetBackground = false;
    }
    rRenderContext.SetLineColor(rFieldTextColor);

    // Shadow
    Size aTempSize = aSize;

    // Page
    if (!bEnabled)
    {
        rRenderContext.SetFillColor(rDisableColor);
        rRenderContext.DrawRect(tools::Rectangle(rOrg, aTempSize));
        return;
    }
    rRenderContext.SetFillColor(rFieldColor);
    rRenderContext.DrawRect(tools::Rectangle(rOrg, aTempSize));

    long nL = nLeft;
    long nR = nRight;

    if (eUsage == SvxPageUsage::Mirror && !bSecond)
    {
        // turn for mirrored
        nL = nRight;
        nR = nLeft;
    }

    tools::Rectangle aRect;

    aRect.SetLeft( rOrg.X() + nL );
    aRect.SetRight( rOrg.X() + aTempSize.Width() - nR );
    aRect.SetTop( rOrg.Y() + nTop );
    aRect.SetBottom( rOrg.Y() + aTempSize.Height() - nBottom );

    tools::Rectangle aHdRect(aRect);
    tools::Rectangle aFtRect(aRect);

    if (bHeader || bFooter)
    {
        // Header and/or footer used
        const Color aLineColor(rRenderContext.GetLineColor());

        // draw PageFill first and on the whole page, no outline
        rRenderContext.SetLineColor();
        drawFillAttributes(rRenderContext, maPageFillAttributes, aRect, aRect);
        rRenderContext.SetLineColor(aLineColor);

        if (bHeader)
        {
            // show headers if possible
            aHdRect.AdjustLeft(nHdLeft );
            aHdRect.AdjustRight( -nHdRight );
            aHdRect.SetBottom( aRect.Top() + nHdHeight );
            aRect.AdjustTop(nHdHeight + nHdDist );

            // draw header over PageFill, plus outline
            drawFillAttributes(rRenderContext, maHeaderFillAttributes, aHdRect, aHdRect);
        }

        if (bFooter)
        {
            // show footer if possible
            aFtRect.AdjustLeft(nFtLeft );
            aFtRect.AdjustRight( -nFtRight );
            aFtRect.SetTop( aRect.Bottom() - nFtHeight );
            aRect.AdjustBottom( -(nFtHeight + nFtDist) );

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

    if (bFrameDirection && !bTable)
    {
        Point aPos;
        vcl::Font aFont(rRenderContext.GetFont());
        const Size aSaveSize = aFont.GetFontSize();
        Size aDrawSize(0,aRect.GetHeight() / 6);
        aFont.SetFontSize(aDrawSize);
        rRenderContext.SetFont(aFont);
        OUString sText("ABC");
        Point aMove(1, rRenderContext.GetTextHeight());
        sal_Unicode cArrow = 0x2193;
        long nAWidth = rRenderContext.GetTextWidth(sText.copy(0,1));
        switch (nFrameDirection)
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
        sText += OUStringLiteral1(cArrow);
        for (sal_Int32 i = 0; i < sText.getLength(); i++)
        {
            OUString sDraw(sText.copy(i,1));
            long nHDiff = 0;
            long nCharWidth = rRenderContext.GetTextWidth(sDraw);
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
    if (bTable)
    {
        // Paint Table, if necessary center it
        rRenderContext.SetLineColor(COL_LIGHTGRAY);

        long nW = aRect.GetWidth();
        long nH = aRect.GetHeight();
        long const nTW = CELL_WIDTH * 3;
        long const nTH = CELL_HEIGHT * 3;
        long _nLeft = bHorz ? aRect.Left() + ((nW - nTW) / 2) : aRect.Left();
        long _nTop = bVert ? aRect.Top() + ((nH - nTH) / 2) : aRect.Top();
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
}

void SvxPageWindow::drawFillAttributes(vcl::RenderContext& rRenderContext,
                                       const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
                                       const tools::Rectangle& rPaintRange,
                                       const tools::Rectangle& rDefineRange)
{
    const basegfx::B2DRange aPaintRange(
        rPaintRange.Left(),
        rPaintRange.Top(),
        rPaintRange.Right(),
        rPaintRange.Bottom());

    if(!aPaintRange.isEmpty() &&
       !basegfx::fTools::equalZero(aPaintRange.getWidth()) &&
       !basegfx::fTools::equalZero(aPaintRange.getHeight()))
    {
        const basegfx::B2DRange aDefineRange(
            rDefineRange.Left(),
            rDefineRange.Top(),
            rDefineRange.Right(),
            rDefineRange.Bottom());

        // prepare primitive sequence
        drawinglayer::primitive2d::Primitive2DContainer aSequence;

        // create fill geometry if there is something to fill
        if (rFillAttributes.get() && rFillAttributes->isUsed())
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
        if (!aSequence.empty())
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                            basegfx::B2DHomMatrix(), rRenderContext.GetViewTransformation(), aPaintRange, nullptr,
                            0.0, css::uno::Sequence<css::beans::PropertyValue >());

            std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
                drawinglayer::processor2d::createProcessor2DFromOutputDevice(rRenderContext, aViewInformation2D));
            if (pProcessor)
            {
                pProcessor->process(aSequence);
            }
        }
    }
}


void SvxPageWindow::EnableFrameDirection(bool bEnable)
{
    bFrameDirection = bEnable;
}

void SvxPageWindow::SetFrameDirection(SvxFrameDirection nDirection)
{
    nFrameDirection = nDirection;
}

void SvxPageWindow::ResetBackground()
{
    bResetBackground = true;
}

void SvxPageWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);

    OutputDevice& rRefDevice = pDrawingArea->get_ref_device();
    // Count in Twips by default
    rRefDevice.Push(PushFlags::MAPMODE);
    rRefDevice.SetMapMode(MapMode(MapUnit::MapTwip));
    aWinSize = rRefDevice.LogicToPixel(Size(75, 46), MapMode(MapUnit::MapAppFont));
    pDrawingArea->set_size_request(aWinSize.Width(), aWinSize.Height());

    aWinSize.AdjustHeight( -4 );
    aWinSize.AdjustWidth( -4 );

    aWinSize = rRefDevice.PixelToLogic(aWinSize);
    rRefDevice.Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
