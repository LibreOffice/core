/*************************************************************************
 *
 *  $RCSfile: IdleDetection.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-17 09:45:40 $
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
    if (GetpApp()->AnyInput())
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
