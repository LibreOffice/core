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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
