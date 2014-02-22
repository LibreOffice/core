/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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
        PageSelector::BroadcastLock aBroadcastLock (mrSlideSorter.GetController().GetPageSelector());

        mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();

        ReleaseCurrentSlide();
        AcquireCurrentSlide(nSlideIndex);

        
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
            
            FrameView* pFrameView = pViewShell->GetFrameView();
            if (pFrameView != NULL)
                pFrameView->SetSelectedPage(sal::static_int_cast<sal_uInt16>(mnCurrentSlideIndex));
            mrSlideSorter.GetController().GetPageSelector().SetCoreSelection();
        }

        
        
        
        
        
        
        
        
        maSwitchPageDelayTimer.Start();

        
        
        
        
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
                OUString("CurrentPage"),
                aPage);
        }
    }
    catch (const Exception&)
    {
        
        
        
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




IMPL_LINK_NOARG(CurrentSlideManager, SwitchPageCallback)
{
    if (mpCurrentSlide)
    {
        
        
        
        
        
        ViewShell* pViewShell = mrSlideSorter.GetViewShell();
        if (pViewShell==NULL || ! pViewShell->IsMainViewShell())
            SetCurrentSlideAtViewShellBase(mpCurrentSlide);
        SetCurrentSlideAtXController(mpCurrentSlide);
    }

    return 1;
}

} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
