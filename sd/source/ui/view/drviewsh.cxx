/*************************************************************************
 *
 *  $RCSfile: drviewsh.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:57:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#pragma hdrstop

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

namespace sd {

#define TABCONTROL_INITIAL_SIZE     500

/*************************************************************************
|*
|* Sprung zu Bookmark
|*
\************************************************************************/

BOOL DrawViewShell::GotoBookmark(const String& rBookmark)
{
    return (GetDocSh()->GotoBookmark(rBookmark));
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

    if(!aVisArea.IsInside(rRect) && !pFuSlideShow)
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
