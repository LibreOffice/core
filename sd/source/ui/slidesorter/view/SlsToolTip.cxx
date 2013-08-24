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


#include "view/SlsToolTip.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include <vcl/help.hxx>


namespace sd { namespace slidesorter { namespace view {

ToolTip::ToolTip (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      msCurrentHelpText(),
      mnHelpWindowHandle(0),
      maShowTimer(),
      maHiddenTimer()
{
    SharedSdWindow window = rSlideSorter.GetContentWindow();
    const HelpSettings& rHelpSettings = window->GetSettings().GetHelpSettings();
    maShowTimer.SetTimeout(rHelpSettings.GetTipDelay());
    maShowTimer.SetTimeoutHdl(LINK(this, ToolTip, DelayTrigger));
    maHiddenTimer.SetTimeout(rHelpSettings.GetTipDelay());
}




ToolTip::~ToolTip (void)
{
    maShowTimer.Stop();
    maHiddenTimer.Stop();
    Hide();
}


void ToolTip::SetPage (const model::SharedPageDescriptor& rpDescriptor)
{
    if (mpDescriptor != rpDescriptor)
    {
        maShowTimer.Stop();
        bool bWasVisible = Hide();

        if (bWasVisible)
        {
            maHiddenTimer.Start();
        }

        mpDescriptor = rpDescriptor;

        if (mpDescriptor)
        {
            SdPage* pPage = mpDescriptor->GetPage();
            OUString sHelpText;
            if (pPage != NULL)
                sHelpText = pPage->GetName();
            else
            {
                OSL_ASSERT(mpDescriptor->GetPage() != NULL);
            }
            if (sHelpText.isEmpty())
            {
                sHelpText = SD_RESSTR(STR_PAGE);
                sHelpText += OUString::number(mpDescriptor->GetPageIndex()+1);
            }

            msCurrentHelpText = sHelpText;
            // show new tooltip immediately, if last one was recently hidden
            Show(maHiddenTimer.IsActive());
        }
        else
        {
            msCurrentHelpText = OUString();
        }
    }
}



void ToolTip::Show (const bool bNoDelay)
{
    if (bNoDelay)
        DoShow();
    else
        maShowTimer.Start();
}




void ToolTip::DoShow (void)
{
    if (maShowTimer.IsActive())
    {
        // The delay timer is active.  Wait for it to trigger the showing of
        // the tool tip.
        return;
    }

    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (!msCurrentHelpText.isEmpty() && pWindow)
    {
        Rectangle aBox (
            mrSlideSorter.GetView().GetLayouter().GetPageObjectLayouter()->GetBoundingBox(
                mpDescriptor,
                PageObjectLayouter::Preview,
                PageObjectLayouter::WindowCoordinateSystem));

        // Do not show the help text when the (lower edge of the ) preview
        // is not visible.  The tool tip itself may still be outside the
        // window.
        if (aBox.Bottom() >= pWindow->GetSizePixel().Height())
            return;

        ::Window* pParent (pWindow.get());
        while (pParent!=NULL && pParent->GetParent()!=NULL)
            pParent = pParent->GetParent();
        const Point aOffset (pWindow->GetWindowExtentsRelative(pParent).TopLeft());

        // We do not know how high the tool tip will be but want its top
        // edge not its bottom to be at a specific position (a little below
        // the preview).  Therefore we use a little trick and place the tool
        // tip at the top of a rectangle that is placed below the preview.
        aBox.Move(aOffset.X(), aOffset.Y() + aBox.GetHeight() + 3);
        mnHelpWindowHandle = Help::ShowTip(
            pWindow.get(),
            aBox,
            msCurrentHelpText,
            QUICKHELP_CENTER | QUICKHELP_TOP);
    }
}




bool ToolTip::Hide (void)
{
    if (mnHelpWindowHandle>0)
    {
        Help::HideTip(mnHelpWindowHandle);
        mnHelpWindowHandle = 0;
        return true;
    }
    else
        return false;
}




IMPL_LINK_NOARG(ToolTip, DelayTrigger)
{
    DoShow();

    return 0;
}

} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
