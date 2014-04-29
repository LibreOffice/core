/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"
#include <svl/aeitem.hxx>
#include <svl/itemset.hxx>
#include <sfx2/request.hxx>
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif


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

sal_Bool DrawViewShell::GotoBookmark(const String& rBookmark)
{
    sal_Bool bRet = sal_False;
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
    // questions separately. The new behaviour is triggered with the
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
