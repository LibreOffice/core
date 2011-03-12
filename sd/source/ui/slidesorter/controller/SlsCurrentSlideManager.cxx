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

#include "precompiled_sd.hxx"

#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsFocusManager.hxx"
#include "view/SlideSorterView.hxx"
#include "ViewShellBase.hxx"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "sdpage.hxx"
#include "FrameView.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using namespace ::sd::slidesorter::model;


namespace sd { namespace slidesorter { namespace controller {


CurrentSlideManager::CurrentSlideManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mnCurrentSlideIndex(-1),
      mpCurrentSlide(),
      maSwitchPageDelayTimer()
{
    maSwitchPageDelayTimer.SetTimeout(100);
    maSwitchPageDelayTimer.SetTimeoutHdl(LINK(this,CurrentSlideManager,SwitchPageCallback));
}




CurrentSlideManager::~CurrentSlideManager (void)
{
}




void CurrentSlideManager::NotifyCurrentSlideChange (const SdPage* pPage)
{
    if (pPage != NULL)
        NotifyCurrentSlideChange(
            mrSlideSorter.GetModel().GetIndex(
                Reference<drawing::XDrawPage>(
                    const_cast<SdPage*>(pPage)->getUnoPage(),
                    UNO_QUERY)));
    else
        NotifyCurrentSlideChange(-1);
}




void CurrentSlideManager::NotifyCurrentSlideChange (const sal_Int32 nSlideIndex)
{
    if (mnCurrentSlideIndex != nSlideIndex)
    {
        ReleaseCurrentSlide();
        AcquireCurrentSlide(nSlideIndex);

        // Update the selection.
        mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
        if (mpCurrentSlide)
        {
            mrSlideSorter.GetController().GetPageSelector().SelectPage(mpCurrentSlide);
            mrSlideSorter.GetController().GetFocusManager().SetFocusedPage(mpCurrentSlide);
        }
    }
}




void CurrentSlideManager::ReleaseCurrentSlide (void)
{
    if (mpCurrentSlide.get() != NULL)
        mrSlideSorter.GetView().SetState(mpCurrentSlide, PageDescriptor::ST_Current, false);

    mpCurrentSlide.reset();
    mnCurrentSlideIndex = -1;
}




bool CurrentSlideManager::IsCurrentSlideIsValid (void)
{
    return mnCurrentSlideIndex >= 0 && mnCurrentSlideIndex<mrSlideSorter.GetModel().GetPageCount();
}




void CurrentSlideManager::AcquireCurrentSlide (const sal_Int32 nSlideIndex)
{
    mnCurrentSlideIndex = nSlideIndex;

    if (IsCurrentSlideIsValid())
    {
        // Get a descriptor for the XDrawPage reference.  Note that the
        // given XDrawPage may or may not be member of the slide sorter
        // document.
        mpCurrentSlide = mrSlideSorter.GetModel().GetPageDescriptor(mnCurrentSlideIndex);
        if (mpCurrentSlide.get() != NULL)
            mrSlideSorter.GetView().SetState(mpCurrentSlide, PageDescriptor::ST_Current, true);
    }
}




void CurrentSlideManager::SwitchCurrentSlide (
    const sal_Int32 nSlideIndex,
    const bool bUpdateSelection)
{
    SwitchCurrentSlide(mrSlideSorter.GetModel().GetPageDescriptor(nSlideIndex), bUpdateSelection);
}




void CurrentSlideManager::SwitchCurrentSlide (
    const SharedPageDescriptor& rpDescriptor,
    const bool bUpdateSelection)
{
    if (rpDescriptor.get() != NULL && mpCurrentSlide!=rpDescriptor)
    {
        ReleaseCurrentSlide();
        AcquireCurrentSlide((rpDescriptor->GetPage()->GetPageNum()-1)/2);

        ViewShell* pViewShell = mrSlideSorter.GetViewShell();
        if (pViewShell != NULL && pViewShell->IsMainViewShell())
        {
            // The slide sorter is the main view.
            FrameView* pFrameView = pViewShell->GetFrameView();
            if (pFrameView != NULL)
                pFrameView->SetSelectedPage(sal::static_int_cast<sal_uInt16>(mnCurrentSlideIndex));
            mrSlideSorter.GetController().GetPageSelector().SetCoreSelection();
        }

        // We do not tell the XController/ViewShellBase about the new
        // slide right away.  This is done asynchronously after a short
        // delay to allow for more slide switches in the slide sorter.
        // This goes under the assumption that slide switching inside
        // the slide sorter is fast (no expensive redraw of the new page
        // (unless the preview of the new slide is not yet preset)) and
        // that slide switching in the edit view is slow (all shapes of
        // the new slide have to be repainted.)
        maSwitchPageDelayTimer.Start();

        // We have to store the (index of the) new current slide at
        // the tab control because there are other asynchronous
        // notifications of the slide switching that otherwise
        // overwrite the correct value.
        SetCurrentSlideAtTabControl(mpCurrentSlide);

        if (bUpdateSelection)
        {
            mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
            mrSlideSorter.GetController().GetPageSelector().SelectPage(rpDescriptor);
        }
        mrSlideSorter.GetController().GetFocusManager().SetFocusedPage(rpDescriptor);
    }
}




void CurrentSlideManager::SetCurrentSlideAtViewShellBase (const SharedPageDescriptor& rpDescriptor)
{
    OSL_ASSERT(rpDescriptor.get() != NULL);

    ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
    if (pBase != NULL)
    {
        DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(
            pBase->GetMainViewShell().get());
        if (pDrawViewShell != NULL)
        {
            sal_uInt16 nPageNumber = (rpDescriptor->GetPage()->GetPageNum()-1)/2;
            pDrawViewShell->SwitchPage(nPageNumber);
            pDrawViewShell->GetPageTabControl()->SetCurPageId(nPageNumber+1);
        }
    }
}




void CurrentSlideManager::SetCurrentSlideAtTabControl (const SharedPageDescriptor& rpDescriptor)
{
    OSL_ASSERT(rpDescriptor.get() != NULL);

    ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
    if (pBase != NULL)
    {
        ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
            ::boost::dynamic_pointer_cast<DrawViewShell>(pBase->GetMainViewShell()));
        if (pDrawViewShell)
        {
            sal_uInt16 nPageNumber = (rpDescriptor->GetPage()->GetPageNum()-1)/2;
            pDrawViewShell->GetPageTabControl()->SetCurPageId(nPageNumber+1);
        }
    }
}




void CurrentSlideManager::SetCurrentSlideAtXController (const SharedPageDescriptor& rpDescriptor)
{
    OSL_ASSERT(rpDescriptor.get() != NULL);

    try
    {
        Reference<beans::XPropertySet> xSet (mrSlideSorter.GetXController(), UNO_QUERY);
        if (xSet.is())
        {
            Any aPage;
            aPage <<= rpDescriptor->GetPage()->getUnoPage();
            xSet->setPropertyValue (
                String::CreateFromAscii("CurrentPage"),
                aPage);
        }
    }
    catch (Exception aException)
    {
        // We have not been able to set the current page at the main view.
        // This is sad but still leaves us in a valid state.  Therefore,
        // this exception is silently ignored.
    }
}




SharedPageDescriptor CurrentSlideManager::GetCurrentSlide (void)
{
    return mpCurrentSlide;
}




void CurrentSlideManager::PrepareModelChange (void)
{
    mpCurrentSlide.reset();
}




void CurrentSlideManager::HandleModelChange (void)
{
    if (mnCurrentSlideIndex >= 0)
    {
        mpCurrentSlide = mrSlideSorter.GetModel().GetPageDescriptor(mnCurrentSlideIndex);
        if (mpCurrentSlide.get() != NULL)
            mrSlideSorter.GetView().SetState(mpCurrentSlide, PageDescriptor::ST_Current, true);
    }
}




IMPL_LINK(CurrentSlideManager, SwitchPageCallback, void*, EMPTYARG)
{
    if (mpCurrentSlide)
    {
        // Set current page.  At the moment we have to do this in two
        // different ways.  The UNO way is the preferable one but, alas,
        // it does not work always correctly (after some kinds of model
        // changes).  Therefore, we call DrawViewShell::SwitchPage(),
        // too.
        ViewShell* pViewShell = mrSlideSorter.GetViewShell();
        if (pViewShell==NULL || ! pViewShell->IsMainViewShell())
            SetCurrentSlideAtViewShellBase(mpCurrentSlide);
        SetCurrentSlideAtXController(mpCurrentSlide);
    }

    return 1;
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
