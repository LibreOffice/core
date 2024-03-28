/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/IdleDetection.hxx>

#include <slideshow.hxx>
#include <ViewShellBase.hxx>

#include <vcl/window.hxx>
#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

namespace sd::tools {

IdleState IdleDetection::GetIdleState (const vcl::Window* pWindow)
{
    IdleState nResult (CheckInputPending() | CheckSlideShowRunning());
    if (pWindow != nullptr)
        nResult |= CheckWindowPainting(*pWindow);
    return nResult;
}

IdleState IdleDetection::CheckInputPending()
{
    if (Application::AnyInput(VclInputFlags::MOUSE | VclInputFlags::KEYBOARD | VclInputFlags::PAINT))
        return IdleState::SystemEventPending;
    else
        return IdleState::Idle;
}

IdleState IdleDetection::CheckSlideShowRunning()
{
    IdleState eResult (IdleState::Idle);

    // Iterate over all view frames.
    for (SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
         pViewFrame!=nullptr;
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
        catch (const uno::RuntimeException&)
        {
        }
        if (bIgnoreFrame)
            continue;

        // Get sd::ViewShell from active frame.
        ViewShellBase* pBase = ViewShellBase::GetViewShellBase(pViewFrame);
        if (pBase != nullptr)
        {
            rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( *pBase ) );
            if( xSlideShow.is() && xSlideShow->isRunning() && !xSlideShow->IsInteractiveSlideshow()) // IASS
            {
                if (xSlideShow->isFullScreen())
                    eResult |= IdleState::FullScreenShowActive;
                else
                    eResult |= IdleState::WindowShowActive;
            }
        }
    }

    return eResult;
}

IdleState IdleDetection::CheckWindowPainting (const vcl::Window& rWindow)
{
    if (rWindow.IsInPaint())
        return IdleState::WindowPainting;
    else
        return IdleState::Idle;
}

} // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
