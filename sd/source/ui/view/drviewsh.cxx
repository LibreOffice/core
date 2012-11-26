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

bool DrawViewShell::GotoBookmark(const String& rBookmark)
{
    bool bRet = false;
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

void DrawViewShell::MakeVisibleAtView(const basegfx::B2DRange& rToMakeVisibleRange, ::Window& rWin)
{
    if(!rToMakeVisibleRange.isEmpty() && !SlideShow::IsRunning(GetViewShellBase()))
    {
        const basegfx::B2DRange aVisibleLogic(rWin.GetLogicRange());

        if(!aVisibleLogic.isInside(rToMakeVisibleRange))
        {
            // object is not completely inside. Calc target area with border
            const double fPercentBorder(0.15); // 15%

            // default new top-left is current top-left
            basegfx::B2DPoint aNewPos(aVisibleLogic.getMinimum());

            if(rToMakeVisibleRange.getWidth() > aVisibleLogic.getWidth())
            {
                // object is wider than target range
                if(rToMakeVisibleRange.getMaxX() < aVisibleLogic.getMinX())
                {
                    // object is outside left, move view to show right side of object
                    aNewPos.setX(rToMakeVisibleRange.getMaxX() - (aVisibleLogic.getWidth() * (1.0 - fPercentBorder)));
                }
                else if(rToMakeVisibleRange.getMinX() > aVisibleLogic.getMaxX())
                {
                    // object is outside right
                    aNewPos.setX(rToMakeVisibleRange.getMinX() - (aVisibleLogic.getWidth() * fPercentBorder));
                }
                else
                {
                    // object is partially in visible range, nothing to do
                }
            }
            else
            {
                // object is equal or taller than target range
                if(rToMakeVisibleRange.getMinX() < aVisibleLogic.getMinX())
                {
                    // left side of object not completely visible
                    aNewPos.setX(rToMakeVisibleRange.getMinX() - (aVisibleLogic.getWidth() * fPercentBorder));
                }
                else if(rToMakeVisibleRange.getMaxX() > aVisibleLogic.getMaxX())
                {
                    // right side of object not completely visible
                    aNewPos.setX(rToMakeVisibleRange.getMaxX() - (aVisibleLogic.getWidth() * (1.0 - fPercentBorder)));
                }
                else
                {
                    // both sides visible, should not happen since already checked by isInside
                }
            }

            if(rToMakeVisibleRange.getHeight() > aVisibleLogic.getHeight())
            {
                // object is higher than target range
                if(rToMakeVisibleRange.getMaxY() < aVisibleLogic.getMinY())
                {
                    // object is above, move view to show lower side of object
                    aNewPos.setY(rToMakeVisibleRange.getMaxY() - (aVisibleLogic.getHeight() * (1.0 - fPercentBorder)));
                }
                else if(rToMakeVisibleRange.getMinY() > aVisibleLogic.getMaxY())
                {
                    // object is below
                    aNewPos.setY(rToMakeVisibleRange.getMinY() - (aVisibleLogic.getHeight() * fPercentBorder));
                }
                else
                {
                    // object is partially in visible range, nothing to do
                }
            }
            else
            {
                // object is equal or taller than target range
                if(rToMakeVisibleRange.getMinY() < aVisibleLogic.getMinY())
                {
                    // upper side of object not completely visible
                    aNewPos.setY(rToMakeVisibleRange.getMinY() - (aVisibleLogic.getHeight() * fPercentBorder));
                }
                else if(rToMakeVisibleRange.getMaxY() > aVisibleLogic.getMaxY())
                {
                    // bottom of object not completely visible
                    aNewPos.setY(rToMakeVisibleRange.getMaxY() - (aVisibleLogic.getHeight() * (1.0 - fPercentBorder)));
                }
                else
                {
                    // both sides visible, should not happen since already checked by isInside
                }
            }

            if(!aNewPos.equal(aVisibleLogic.getMinimum()))
            {
                // set new zoom if top-left has to be changed
                SetZoomRange(
                    basegfx::B2DRange(
                        aNewPos,
                        aNewPos + aVisibleLogic.getRange()));
            }
        }
    }
}

}
