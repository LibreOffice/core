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

#include <sfx2/dialoghelper.hxx>
#include <svx/paraprev.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>

SvxParaPrevWindow::SvxParaPrevWindow() :
    nLeftMargin     ( 0 ),
    nRightMargin    ( 0 ),
    nFirstLineOfst  ( 0 ),
    nUpper          ( 0 ),
    nLower          ( 0 ),
    eAdjust         ( SvxAdjust::Left ),
    eLastLine       ( SvxAdjust::Left ),
    eLine           ( SvxPrevLineSpace::N1 )
{
    aSize = Size(11905, 16837);
}

void SvxParaPrevWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aOptimalSize(getParagraphPreviewOptimalSize(pDrawingArea->get_ref_device()));
    pDrawingArea->set_size_request(aOptimalSize.Width(), aOptimalSize.Height());
}

void SvxParaPrevWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    DrawParagraph(rRenderContext);
}

#define DEF_MARGIN  120

void SvxParaPrevWindow::DrawParagraph(vcl::RenderContext& rRenderContext)
{
    // Count in Twips by default
    rRenderContext.Push(PushFlags::MAPMODE);
    rRenderContext.SetMapMode(MapMode(MapUnit::MapTwip));

    Size aWinSize(GetOutputSizePixel());
    aWinSize = rRenderContext.PixelToLogic(aWinSize);
    Size aTmp(1, 1);
    aTmp = rRenderContext.PixelToLogic(aTmp);
    aWinSize.AdjustWidth( -(aTmp.Width() /2) );
    aWinSize.AdjustHeight( -(aTmp.Height() /2) );

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    const Color& rWinColor = rStyleSettings.GetWindowColor();
    Color aGrayColor(COL_LIGHTGRAY);

    rRenderContext.SetFillColor(rWinColor);
    rRenderContext.DrawRect(tools::Rectangle(Point(), aWinSize));

    rRenderContext.SetLineColor();

    long nH = aWinSize.Height() / 19;
    Size aLineSiz(aWinSize.Width() - DEF_MARGIN, nH);
    Size aSiz = aLineSiz;
    Point aPnt;
    aPnt.setX( DEF_MARGIN / 2 );
    rRenderContext.SetFillColor(aGrayColor);

    for (sal_uInt16 i = 0; i < 9; ++i)
    {
        if (i == 3)
        {
            rRenderContext.SetFillColor(COL_GRAY);
            auto nTop = nUpper * aLineSiz.Height() / aSize.Height();
            aPnt.AdjustY(nTop * 2 );
        }

        if (i == 6 )
            rRenderContext.SetFillColor(aGrayColor);

        if (3 <= i && 6 > i)
        {
            long nLeft = nLeftMargin * aLineSiz.Width() / aSize.Width();
            long nFirst = nFirstLineOfst * aLineSiz.Width() / aSize.Width();
            long nTmp = nLeft + nFirst;

            if (i == 3)
            {
                aPnt.AdjustX(nTmp );
                aSiz.AdjustWidth( -nTmp );
            }
            else
            {
                aPnt.AdjustX(nLeft );
                aSiz.AdjustWidth( -nLeft );
            }
            long nRight = nRightMargin * aLineSiz.Width() / aSize.Width();
            aSiz.AdjustWidth( -nRight );
        }

        if (4 == i || 5 == i || 6 == i)
        {
            switch (eLine)
            {
                case SvxPrevLineSpace::N1:
                    break;
                case SvxPrevLineSpace::N115:
                    aPnt.AdjustY(nH / 6.67 ); // 1/.15 = 6.(6)
                    break;
                case SvxPrevLineSpace::N15:
                    aPnt.AdjustY(nH / 2 );
                    break;
                case SvxPrevLineSpace::N2:
                    aPnt.AdjustY(nH );
                    break;
                case SvxPrevLineSpace::Prop:
                case SvxPrevLineSpace::Min:
                case SvxPrevLineSpace::Leading:
                    break;
            }
        }

        aPnt.AdjustY(nH );

        if (3 <= i && 5 >= i)
        {
            long nLW = long();
            switch (i)
            {
                case 3:
                    nLW = aLineSiz.Width() * 8 / 10;
                    break;
                case 4:
                    nLW = aLineSiz.Width() * 9 / 10;
                    break;
                case 5:
                    nLW = aLineSiz.Width() / 2;
                    break;
            }

            if (nLW > aSiz.Width())
                nLW = aSiz.Width();

            switch (eAdjust)
            {
                case SvxAdjust::Left:
                    break;
                case SvxAdjust::Right:
                    aPnt.AdjustX( aSiz.Width() - nLW );
                    break;
                case SvxAdjust::Center:
                    aPnt.AdjustX(( aSiz.Width() - nLW ) / 2 );
                    break;
                default: ; //prevent warning
            }
            if (SvxAdjust::Block == eAdjust)
            {
                if(5 == i)
                {
                    switch( eLastLine )
                    {
                        case SvxAdjust::Left:
                            break;
                        case SvxAdjust::Right:
                            aPnt.AdjustX( aSiz.Width() - nLW );
                            break;
                        case SvxAdjust::Center:
                            aPnt.AdjustX(( aSiz.Width() - nLW ) / 2 );
                            break;
                        case SvxAdjust::Block:
                            nLW = aSiz.Width();
                            break;
                        default: ; //prevent warning
                    }
                }
                else
                    nLW = aSiz.Width();
            }
            aSiz.setWidth( nLW );
        }

        tools::Rectangle aRect(aPnt, aSiz);

        rRenderContext.DrawRect( aRect );

        if (5 == i)
        {
            auto nBottom = nLower * aLineSiz.Height() / aSize.Height();
            aPnt.AdjustY(nBottom * 2 );
        }

        aPnt.AdjustY(nH );
        // Reset, recalculate for each line
        aPnt.setX( DEF_MARGIN / 2 );
        aSiz = aLineSiz;
    }
    rRenderContext.Pop();
}

#undef DEF_MARGIN

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
