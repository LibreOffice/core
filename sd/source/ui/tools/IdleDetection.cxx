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

#include "tools/IdleDetection.hxx"

#include "ViewShell.hxx"
#include "slideshow.hxx"
#include "ViewShellBase.hxx"

#include <vcl/window.hxx>
#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XFrame.hdl>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

namespace sd { namespace tools {


sal_Int32 IdleDetection::GetIdleState (const ::Window* pWindow)
{
    sal_Int32 nResult (CheckInputPending() | CheckSlideShowRunning());
    if (pWindow != NULL)
        nResult |= CheckWindowPainting(*pWindow);
    return nResult;
}




sal_Int32 IdleDetection::CheckInputPending (void)
{
    if (GetpApp()->AnyInput(INPUT_MOUSE | INPUT_KEYBOARD | INPUT_PAINT))
        return IDET_SYSTEM_EVENT_PENDING;
    else
        return IDET_IDLE;
}




sal_Int32 IdleDetection::CheckSlideShowRunning (void)
{
    sal_Int32 eResult (IDET_IDLE);

    bool bIsSlideShowShowing = false;

    // Iterate over all view frames.
    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    for (pViewFrame = SfxViewFrame::GetFirst();
         pViewFrame!=NULL && !bIsSlideShowShowing;
         pViewFrame = SfxViewFrame::GetNext(*pViewFrame))
    {
        // Ignore the current frame when it does not exist, is not valid, or
        // is not active.
        bool bIgnoreFrame (true);
        uno::Reference<frame::XFrame> xFrame (pViewFrame->GetFrame().GetFrameInterface());
        try
        {
            if (xFrame.is() && xFrame->isActive())
                bIgnoreFrame = false;
        }
        catch (uno::RuntimeException e)
        {
            (void) e;
        }
        if (bIgnoreFrame)
            continue;

        // Get sd::ViewShell from active frame.
        ViewShellBase* pBase = ViewShellBase::GetViewShellBase(pViewFrame);
        if (pBase != NULL)
        {
            rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( *pBase ) );
            if( xSlideShow.is() && xSlideShow->isRunning() )
            {
                if (xSlideShow->isFullScreen())
                    eResult |= IDET_FULL_SCREEN_SHOW_ACTIVE;
                else
                    eResult |= IDET_WINDOW_SHOW_ACTIVE;
            }
        }
    }

    return eResult;
}




sal_Int32 IdleDetection::CheckWindowPainting (const ::Window& rWindow)
{
    if (rWindow.IsInPaint())
        return IDET_WINDOW_PAINTING;
    else
        return IDET_IDLE;
}

} } // end of namespace ::sd::tools
