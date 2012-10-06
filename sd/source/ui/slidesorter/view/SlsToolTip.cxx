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


#include "view/SlsToolTip.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include <vcl/help.hxx>

using ::rtl::OUString;

namespace sd { namespace slidesorter { namespace view {

ToolTip::ToolTip (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      msDefaultHelpText(),
      msCurrentHelpText(),
      mnHelpWindowHandle(0),
      maTimer()
{
    maTimer.SetTimeout(Theme_ToolTipDelay);
    maTimer.SetTimeoutHdl(LINK(this, ToolTip, DelayTrigger));
}




ToolTip::~ToolTip (void)
{
    maTimer.Stop();
    Hide();
}




void ToolTip::SetPage (const model::SharedPageDescriptor& rpDescriptor)
{
    if (mpDescriptor != rpDescriptor)
    {
        maTimer.Stop();
        Hide();

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
                sHelpText = String(SdResId(STR_PAGE));
                sHelpText += String::CreateFromInt32(mpDescriptor->GetPageIndex()+1);
            }

            msDefaultHelpText = sHelpText;
            msCurrentHelpText = sHelpText;
            Show(false);
        }
        else
        {
            msDefaultHelpText = OUString();
            msCurrentHelpText = OUString();
        }
    }
}



void ToolTip::Show (const bool bNoDelay)
{
    if (bNoDelay)
        DoShow();
    else
        maTimer.Start();
}




void ToolTip::DoShow (void)
{
    if (maTimer.IsActive())
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
