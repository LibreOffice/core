/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drviewsh.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:18:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif


#ifndef _SVX_FMSHELL_HXX            // XXX nur temp (dg)
#include <svx/fmshell.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "app.hrc"
#include "strings.hrc"
#include "sdpage.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

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
    // #98568# In older versions, if in X or Y the size of the object was
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
            // #98568# allow a mode for move-only visibility without zooming.
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
