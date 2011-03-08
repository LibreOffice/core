/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"
#include <svl/aeitem.hxx>
#include <svl/itemset.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>


#include <svx/fmshell.hxx>
#include <sfx2/dispatch.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "sdpage.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "Window.hxx"
#include "GraphicViewShell.hxx"
#include "drawview.hxx"

#include "slideshow.hxx"

namespace sd {

#define TABCONTROL_INITIAL_SIZE     500

/*************************************************************************
|*
|* Sprung zu Bookmark
|*
\************************************************************************/

BOOL DrawViewShell::GotoBookmark(const String& rBookmark)
{
    BOOL bRet = FALSE;
    ::sd::DrawDocShell* pDocSh = GetDocSh();
    if( pDocSh )
    {
        if( !pDocSh->GetViewShell() ) //#i26016# this case occurs if the jump-target-document was opened already with file open dialog before triggering the jump via hyperlink
            pDocSh->Connect(this);
        bRet = (pDocSh->GotoBookmark(rBookmark));
    }
    return bRet;
}

/*************************************************************************
|*
|* Bereich sichtbar machen (Bildausschnitt scrollen)
|*
\************************************************************************/

void DrawViewShell::MakeVisible(const Rectangle& rRect, ::Window& rWin)
{
    // In older versions, if in X or Y the size of the object was
    // smaller than the visible area, the user-defined zoom was
    // changed. This was decided to be a bug for 6.x, thus I developed a
    // version which instead handles X/Y bigger/smaller and visibility
    // questions seperately. The new behaviour is triggered with the
    // bZoomAllowed parameter which for old behaviour should be set to
    // sal_True. I looked at all uses of MakeVisible() in the application
    // and found no valid reason for really changing the zoom factor, thus I
    // decided to NOT expand (incompatible) this virtual method to get one
    // more parameter. If this is wanted in later versions, feel free to add
    // that bool to the parameter list.
    sal_Bool bZoomAllowed(sal_False);
    Size aLogicSize(rRect.GetSize());

    // Sichtbarer Bereich
    Size aVisSizePixel(rWin.GetOutputSizePixel());
    Rectangle aVisArea(rWin.PixelToLogic(Rectangle(Point(0,0), aVisSizePixel)));
    Size aVisAreaSize(aVisArea.GetSize());

    if(!aVisArea.IsInside(rRect) && !SlideShow::IsRunning( GetViewShellBase() ) )
    {
        // Objekt liegt nicht komplett im sichtbaren Bereich
        sal_Int32 nFreeSpaceX(aVisAreaSize.Width() - aLogicSize.Width());
        sal_Int32 nFreeSpaceY(aVisAreaSize.Height() - aLogicSize.Height());

        if(bZoomAllowed && (nFreeSpaceX < 0 || nFreeSpaceY < 0))
        {
            // Objekt passt nicht in sichtbaren Bereich -> auf Objektgroesse zoomen
            SetZoomRect(rRect);
        }
        else
        {
            // allow a mode for move-only visibility without zooming.
            const sal_Int32 nPercentBorder(30);
            const Rectangle aInnerRectangle(
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
                    aNewPos.X() -= aVisAreaSize.Width() / 2;
                }

                if(aInnerRectangle.Right() < rRect.Left())
                {
                    // object moves out to the right
                    aNewPos.X() += aVisAreaSize.Width() / 2;
                }
            }
            else
            {
                if(nFreeSpaceX > rRect.GetWidth())
                    nFreeSpaceX = rRect.GetWidth();

                while(rRect.Right() > aNewPos.X() + aVisAreaSize.Width())
                    aNewPos.X() += nFreeSpaceX;

                while(rRect.Left() < aNewPos.X())
                    aNewPos.X() -= nFreeSpaceX;
            }

            if(nFreeSpaceY < 0)
            {
                if(aInnerRectangle.Top() > rRect.Bottom())
                {
                    // object moves out to the top
                    aNewPos.Y() -= aVisAreaSize.Height() / 2;
                }

                if(aInnerRectangle.Bottom() < rRect.Top())
                {
                    // object moves out to the right
                    aNewPos.Y() += aVisAreaSize.Height() / 2;
                }
            }
            else
            {
                if(nFreeSpaceY > rRect.GetHeight())
                    nFreeSpaceY = rRect.GetHeight();

                while(rRect.Bottom() > aNewPos.Y() + aVisAreaSize.Height())
                    aNewPos.Y() += nFreeSpaceY;

                while(rRect.Top() < aNewPos.Y())
                    aNewPos.Y() -= nFreeSpaceY;
            }

            // did position change? Does it need to be set?
            if(aNewPos != aVisArea.TopLeft())
            {
                aVisArea.SetPos(aNewPos);
                SetZoomRect(aVisArea);
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
