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

#include <SlideSorter.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <controller/SlsFocusManager.hxx>
#include <view/SlideSorterView.hxx>
#include <ViewShellBase.hxx>
#include <ViewShell.hxx>
#include <DrawViewShell.hxx>
#include <sdpage.hxx>
#include <FrameView.hxx>
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
    maSwitchPageDelayTimer.SetInvokeHandler(LINK(this,CurrentSlideManager,SwitchPageCallback));
}

CurrentSlideManager::~CurrentSlideManager()
{
}

void CurrentSlideManager::NotifyCurrentSlideChange (const SdPage* pPage)
{
    if (pPage != nullptr)
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
    if (mnCurrentSlideIndex == nSlideIndex)
        return;

    PageSelector::BroadcastLock aBroadcastLock (mrSlideSorter.GetController().GetPageSelector());

    mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();

    ReleaseCurrentSlide();
    AcquireCurrentSlide(nSlideIndex);

    // Update the selection.
    if (mpCurrentSlide)
    {
        mrSlideSorter.GetController().GetPageSelector().SelectPage(mpCurrentSlide);
        mrSlideSorter.GetController().GetFocusManager().SetFocusedPage(mpCurrentSlide);
    }
}

void CurrentSlideManager::ReleaseCurrentSlide()
{
    if (mpCurrentSlide.get() != nullptr)
        mrSlideSorter.GetView().SetState(mpCurrentSlide, PageDescriptor::ST_Current, false);

    mpCurrentSlide.reset();
    mnCurrentSlideIndex = -1;
}

void CurrentSlideManager::AcquireCurrentSlide (const sal_Int32 nSlideIndex)
{
    mnCurrentSlideIndex = nSlideIndex;

    // if current slide valid
    if (mnCurrentSlideIndex >= 0 && mnCurrentSlideIndex<mrSlideSorter.GetModel().GetPageCount())
    {
        // Get a descriptor for the XDrawPage reference.  Note that the
        // given XDrawPage may or may not be member of the slide sorter
        // document.
        mpCurrentSlide = mrSlideSorter.GetModel().GetPageDescriptor(mnCurrentSlideIndex);
        if (mpCurrentSlide.get() != nullptr)
            mrSlideSorter.GetView().SetState(mpCurrentSlide, PageDescriptor::ST_Current, true);
    }
}

void CurrentSlideManager::SwitchCurrentSlide (
    const sal_Int32 nSlideIndex)
{
    SwitchCurrentSlide(mrSlideSorter.GetModel().GetPageDescriptor(nSlideIndex), true/*bUpdateSelection*/);
}

void CurrentSlideManager::SwitchCurrentSlide (
    const SharedPageDescriptor& rpDescriptor,
    const bool bUpdateSelection)
{
    if (rpDescriptor.get() == nullptr || mpCurrentSlide==rpDescriptor)
        return;

    ReleaseCurrentSlide();
    AcquireCurrentSlide((rpDescriptor->GetPage()->GetPageNum()-1)/2);

    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell != nullptr && pViewShell->IsMainViewShell())
    {
        // The slide sorter is the main view.
        FrameView* pFrameView = pViewShell->GetFrameView();
        if (pFrameView != nullptr)
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

void CurrentSlideManager::SetCurrentSlideAtViewShellBase (const SharedPageDescriptor& rpDescriptor)
{
    OSL_ASSERT(rpDescriptor.get() != nullptr);

    ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
    if (pBase != nullptr)
    {
        DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(
            pBase->GetMainViewShell().get());
        if (pDrawViewShell != nullptr)
        {
            sal_uInt16 nPageNumber = (rpDescriptor->GetPage()->GetPageNum()-1)/2;
            pDrawViewShell->SwitchPage(nPageNumber);
            TabControl& rPageTabControl = pDrawViewShell->GetPageTabControl();
            rPageTabControl.SetCurPageId(rPageTabControl.GetPageId(nPageNumber));
        }
    }
}

void CurrentSlideManager::SetCurrentSlideAtTabControl (const SharedPageDescriptor& rpDescriptor)
{
    OSL_ASSERT(rpDescriptor.get() != nullptr);

    ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
    if (pBase != nullptr)
    {
        std::shared_ptr<DrawViewShell> pDrawViewShell (
            std::dynamic_pointer_cast<DrawViewShell>(pBase->GetMainViewShell()));
        if (pDrawViewShell)
        {
            sal_uInt16 nPageNumber = (rpDescriptor->GetPage()->GetPageNum()-1)/2;
            TabControl& rPageTabControl = pDrawViewShell->GetPageTabControl();
            rPageTabControl.SetCurPageId(rPageTabControl.GetPageId(nPageNumber));
        }
    }
}

void CurrentSlideManager::SetCurrentSlideAtXController (const SharedPageDescriptor& rpDescriptor)
{
    OSL_ASSERT(rpDescriptor.get() != nullptr);

    try
    {
        Reference<beans::XPropertySet> xSet (mrSlideSorter.GetXController(), UNO_QUERY);
        if (xSet.is())
        {
            Any aPage;
            aPage <<= rpDescriptor->GetPage()->getUnoPage();
            xSet->setPropertyValue( "CurrentPage", aPage );
        }
    }
    catch (const Exception&)
    {
        // We have not been able to set the current page at the main view.
        // This is sad but still leaves us in a valid state.  Therefore,
        // this exception is silently ignored.
    }
}

void CurrentSlideManager::PrepareModelChange()
{
    mpCurrentSlide.reset();
}

void CurrentSlideManager::HandleModelChange()
{
    if (mnCurrentSlideIndex >= 0)
    {
        mpCurrentSlide = mrSlideSorter.GetModel().GetPageDescriptor(mnCurrentSlideIndex);
        if (mpCurrentSlide.get() != nullptr)
            mrSlideSorter.GetView().SetState(mpCurrentSlide, PageDescriptor::ST_Current, true);
    }
}

IMPL_LINK_NOARG(CurrentSlideManager, SwitchPageCallback, Timer *, void)
{
    if (mpCurrentSlide)
    {
        // Set current page.  At the moment we have to do this in two
        // different ways.  The UNO way is the preferable one but, alas,
        // it does not work always correctly (after some kinds of model
        // changes).  Therefore, we call DrawViewShell::SwitchPage(),
        // too.
        ViewShell* pViewShell = mrSlideSorter.GetViewShell();
        if (pViewShell==nullptr || ! pViewShell->IsMainViewShell())
            SetCurrentSlideAtViewShellBase(mpCurrentSlide);
        SetCurrentSlideAtXController(mpCurrentSlide);
    }
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
