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



#include "precompiled_sd.hxx"

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
    maTimer.SetTimeout(rSlideSorter.GetTheme()->GetIntegerValue(Theme::Integer_ToolTipDelay));
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
            if (sHelpText.getLength() == 0)
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




void ToolTip::ShowDefaultHelpText (const ::rtl::OUString& rsHelpText)
{
    if (msDefaultHelpText != rsHelpText)
    {
        const bool bIsVisible (Hide());

        msDefaultHelpText = rsHelpText;
        msCurrentHelpText = rsHelpText;

        Show(bIsVisible);
    }
}




void ToolTip::ShowDefaultHelpText (void)
{
    if (msCurrentHelpText != msDefaultHelpText)
    {
        const bool bIsVisible (Hide());

        msCurrentHelpText = msDefaultHelpText;

        Show(bIsVisible);
    }
}




void ToolTip::ShowHelpText (const ::rtl::OUString& rsHelpText)
{
    if (msCurrentHelpText != rsHelpText)
    {
        const bool bIsVisible (Hide());

        msCurrentHelpText = rsHelpText;

        Show(bIsVisible);
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
    if (msCurrentHelpText.getLength()>0 && pWindow)
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




IMPL_LINK(ToolTip, DelayTrigger, void*, EMPTYARG)
{
    DoShow();

    return 0;
}

} } } // end of namespace ::sd::slidesorter::view
