/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IdleDetection.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:52:30 $
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

#include "tools/IdleDetection.hxx"

#include "ViewShell.hxx"
#include "slideshow.hxx"
#include "ViewShellBase.hxx"

#include <vcl/window.hxx>
#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XFrame.hdl>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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
        if (pViewFrame->GetFrame() != NULL)
        {
            uno::Reference<frame::XFrame> xFrame (pViewFrame->GetFrame()->GetFrameInterface());
            try
            {
                if (xFrame.is() && xFrame->isActive())
                    bIgnoreFrame = false;
            }
            catch (uno::RuntimeException e)
            {
                (void) e;
            }
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
