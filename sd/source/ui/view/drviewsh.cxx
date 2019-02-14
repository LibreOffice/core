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

#include <DrawViewShell.hxx>
#include <svl/aeitem.hxx>
#include <svl/itemset.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>

#include <sal/log.hxx>
#include <svx/fmshell.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/lok.hxx>

#include <app.hrc>
#include <sdpage.hxx>
#include <FrameView.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <Window.hxx>
#include <GraphicViewShell.hxx>
#include <drawview.hxx>

#include <slideshow.hxx>

namespace sd {

void DrawViewShell::GotoBookmark(const OUString& rBookmark)
{
    ::sd::DrawDocShell* pDocSh = GetDocSh();
    if( pDocSh )
    {
        if( !pDocSh->GetViewShell() ) //#i26016# this case occurs if the jump-target-document was opened already with file open dialog before triggering the jump via hyperlink
            pDocSh->Connect(this);
        pDocSh->GotoBookmark(rBookmark);
    }
}

/**
 * Make area visible (scroll part of picture)
|*
\************************************************************************/

void DrawViewShell::MakeVisible(const ::tools::Rectangle& rRect, vcl::Window& rWin)
{
    if ( (IsMouseButtonDown() && !IsMouseSelecting()) || SlideShow::IsRunning( GetViewShellBase() ) )
        return;

    // tdf#98646 check if Rectangle which contains the bounds of the region to
    // be shown eventually contains values that cause overflows when processing
    // e.g. when calling GetWidth()
    const bool bOverflowInX(!rtl::math::approxEqual(static_cast<double>(rRect.getWidth()), static_cast<double>(rRect.Right()) - static_cast<double>(rRect.Left())));
    const bool bOverflowInY(!rtl::math::approxEqual(static_cast<double>(rRect.getHeight()), static_cast<double>(rRect.Bottom()) - static_cast<double>(rRect.Top())));

    if(bOverflowInX || bOverflowInY)
    {
        SAL_WARN("sd", "The given Rectangle contains values that lead to numerical overflows (!)");
        return;
    }

    // In older versions, if in X or Y the size of the object was
    // smaller than the visible area, the user-defined zoom was
    // changed. This was decided to be a bug for
    // StarOffice 6.x (Apr 2002), thus I developed a
    // version which instead handles X/Y bigger/smaller and visibility
    // questions separately
    const Size aLogicSize(rRect.GetSize());

    // visible area
    Size aVisSizePixel(rWin.GetOutputSizePixel());
    bool bTiledRendering = comphelper::LibreOfficeKit::isActive() && !rWin.IsMapModeEnabled();
    if (bTiledRendering)
    {
        rWin.Push(PushFlags::MAPMODE);
        rWin.EnableMapMode();
    }
    ::tools::Rectangle aVisArea(rWin.PixelToLogic(::tools::Rectangle(Point(0,0), aVisSizePixel)));
    if (bTiledRendering)
        rWin.Pop();
    Size aVisAreaSize(aVisArea.GetSize());

    if ( aVisArea.IsInside(rRect) )
        return;

    // object is not entirely in visible area
    sal_Int32 nFreeSpaceX(aVisAreaSize.Width() - aLogicSize.Width());
    sal_Int32 nFreeSpaceY(aVisAreaSize.Height() - aLogicSize.Height());

    // allow a mode for move-only visibility without zooming.
    const sal_Int32 nPercentBorder(30);
    const ::tools::Rectangle aInnerRectangle(
        aVisArea.Left() + ((aVisAreaSize.Width() * nPercentBorder) / 200),
        aVisArea.Top() + ((aVisAreaSize.Height() * nPercentBorder) / 200),
        aVisArea.Right() - ((aVisAreaSize.Width() * nPercentBorder) / 200),
        aVisArea.Bottom() - ((aVisAreaSize.Height() * nPercentBorder) / 200)
        );
    Point aNewPos(aVisArea.TopLeft());

    if(nFreeSpaceX < 0)
    {
        if(aInnerRectangle.Left() > rRect.Right())
        {
            // object moves out to the left
            aNewPos.AdjustX( -(aVisAreaSize.Width() / 2) );
        }

        if(aInnerRectangle.Right() < rRect.Left())
        {
            // object moves out to the right
            aNewPos.AdjustX(aVisAreaSize.Width() / 2 );
        }
    }
    else
    {
        if(nFreeSpaceX > rRect.GetWidth())
        {
            nFreeSpaceX = rRect.GetWidth();
        }

        if(nFreeSpaceX <= 0)
        {
            SAL_WARN("sd", "The given Rectangle contains values that lead to numerical overflows (!)");
        }
        else
        {
            const long distRight(rRect.Right() - aNewPos.X() - aVisAreaSize.Width());

            if(distRight > 0)
            {
                long mult = (distRight / nFreeSpaceX) + 1;
                aNewPos.AdjustX(mult * nFreeSpaceX );
            }

            const long distLeft(aNewPos.X() - rRect.Left());

            if(distLeft > 0)
            {
                long mult = (distLeft / nFreeSpaceX) + 1;
                aNewPos.AdjustX( -(mult * nFreeSpaceX) );
            }
        }
    }

    if(nFreeSpaceY < 0)
    {
        if(aInnerRectangle.Top() > rRect.Bottom())
        {
            // object moves out to the top
            aNewPos.AdjustY( -(aVisAreaSize.Height() / 2) );
        }

        if(aInnerRectangle.Bottom() < rRect.Top())
        {
            // object moves out to the right
            aNewPos.AdjustY(aVisAreaSize.Height() / 2 );
        }
    }
    else
    {
        if(nFreeSpaceY > rRect.GetHeight())
        {
            nFreeSpaceY = rRect.GetHeight();
        }

        if(nFreeSpaceY <= 0)
        {
            SAL_WARN("sd", "The given Rectangle contains values that lead to numerical overflows (!)");
        }
        else
        {
            const long distBottom(rRect.Bottom() - aNewPos.Y() - aVisAreaSize.Height());

            if(distBottom > 0)
            {
                long mult = (distBottom / nFreeSpaceY) + 1;
                aNewPos.AdjustY(mult * nFreeSpaceY );
            }

            const long distTop(aNewPos.Y() - rRect.Top());

            if(distTop > 0)
            {
                long mult = (distTop / nFreeSpaceY) + 1;
                aNewPos.AdjustY( -(mult * nFreeSpaceY) );
            }
        }
    }

    // did position change? Does it need to be set?
    if(aNewPos != aVisArea.TopLeft())
    {
        aVisArea.SetPos(aNewPos);
        SetZoomRect(aVisArea);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
