/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IdleDetection.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 12:26:43 $
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

#include "tools/IdleDetection.hxx"

#include "ViewShell.hxx"
#include "slideshow.hxx"
#include "ViewShellBase.hxx"

#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XFrame.hdl>

using namespace ::com::sun::star;

namespace sd { namespace tools {


sal_Int32 IdleDetection::GetIdleState (void)
{
    return CheckInputPending() | CheckSlideShowRunning();
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
        // Check whether the frame is the active one.
        uno::Reference<frame::XFrame> xFrame (pViewFrame->GetFrame()->GetFrameInterface());
        if (xFrame.is() && ! xFrame->isActive())
        {
            // Frames that are not active are ignored.
            continue;
        }

        // Get sd::ViewShell from active frame.
        ViewShellBase* pBase = ViewShellBase::GetViewShellBase(pViewFrame);
        if (pBase != NULL)
        {
            ViewShell* pViewShell = pBase->GetMainViewShell();
            // Test whether the view shell has a running full screen
            // show.
            Slideshow* pSlideShow = pViewShell->GetSlideShow();
            if (pSlideShow != NULL)
            {
                if (pSlideShow->isFullScreen())
                    eResult |= IDET_FULL_SCREEN_SHOW_ACTIVE;
                else
                    eResult |= IDET_WINDOW_SHOW_ACTIVE;
            }
        }
    }

    return eResult;
}


} } // end of namespace ::sd::tools
