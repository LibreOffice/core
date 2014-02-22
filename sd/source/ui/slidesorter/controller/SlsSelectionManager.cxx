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


#include "controller/SlsSelectionManager.hxx"

#include "SlideSorter.hxx"
#include "SlsCommand.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsAnimator.hxx"
#include "controller/SlsAnimationFunction.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsSlotManager.hxx"
#include "controller/SlsSelectionObserver.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
#include <svx/svxids.hrc>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "strings.hrc"
#include "app.hrc"
#include "glob.hrc"


using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::sd::slidesorter::model;
using namespace ::sd::slidesorter::view;
using namespace ::sd::slidesorter::controller;

namespace sd { namespace slidesorter { namespace controller {


class SelectionManager::PageInsertionListener
    : public SfxListener
{
public:

};


SelectionManager::SelectionManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mrController(rSlideSorter.GetController()),
      maSelectionBeforeSwitch(),
      mbIsMakeSelectionVisiblePending(true),
      mnInsertionPosition(-1),
      mnAnimationId(Animator::NotAnAnimationId),
      maRequestedTopLeft(),
      mpPageInsertionListener(),
      mpSelectionObserver(new SelectionObserver(rSlideSorter))
{
}




SelectionManager::~SelectionManager (void)
{
    if (mnAnimationId != Animator::NotAnAnimationId)
        mrController.GetAnimator()->RemoveAnimation(mnAnimationId);
}




void SelectionManager::DeleteSelectedPages (const bool bSelectFollowingPage)
{
    
    
    SlideSorterController::ModelChangeLock aLock (mrController);
    SlideSorterView::DrawLock aDrawLock (mrSlideSorter);
    PageSelector::UpdateLock aSelectionLock (mrSlideSorter);

    
    bool bIsFocusShowing = mrController.GetFocusManager().IsFocusShowing();
    if (bIsFocusShowing)
        mrController.GetFocusManager().ToggleFocus();

    
    
    model::PageEnumeration aPageEnumeration (
        PageEnumerationProvider::CreateSelectedPagesEnumeration(mrSlideSorter.GetModel()));
    ::std::vector<SdPage*> aSelectedPages;
    sal_Int32 nNewCurrentSlide (-1);
    while (aPageEnumeration.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aPageEnumeration.GetNextElement());
        aSelectedPages.push_back(pDescriptor->GetPage());
        if (bSelectFollowingPage || nNewCurrentSlide<0)
            nNewCurrentSlide = pDescriptor->GetPageIndex();
    }
    if (aSelectedPages.empty())
        return;

    
    
    if (bSelectFollowingPage)
        nNewCurrentSlide -= aSelectedPages.size() - 1;
    else
        --nNewCurrentSlide;

    
    
    
    mrSlideSorter.GetView().BegUndo (SdResId(STR_UNDO_DELETEPAGES));
    if (mrSlideSorter.GetModel().GetEditMode() == EM_PAGE)
        DeleteSelectedNormalPages(aSelectedPages);
    else
        DeleteSelectedMasterPages(aSelectedPages);
    mrSlideSorter.GetView().EndUndo ();

    mrController.HandleModelChange();
    aLock.Release();

    
    if (bIsFocusShowing)
        mrController.GetFocusManager().ToggleFocus();

    
    if (nNewCurrentSlide < 0)
        nNewCurrentSlide = 0;
    else if (nNewCurrentSlide >= mrSlideSorter.GetModel().GetPageCount())
        nNewCurrentSlide = mrSlideSorter.GetModel().GetPageCount()-1;
    mrController.GetPageSelector().CountSelectedPages();
    mrController.GetPageSelector().SelectPage(nNewCurrentSlide);
    mrController.GetFocusManager().SetFocusedPage(nNewCurrentSlide);
}




void SelectionManager::DeleteSelectedNormalPages (const ::std::vector<SdPage*>& rSelectedPages)
{
    
    OSL_ASSERT(mrSlideSorter.GetModel().GetEditMode() == EM_PAGE);

    try
    {
        Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier( mrSlideSorter.GetModel().GetDocument()->getUnoModel(), UNO_QUERY_THROW );
        Reference<drawing::XDrawPages> xPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );

        
        
        
        
        ::std::vector<SdPage*>::const_reverse_iterator aI;
        for (aI=rSelectedPages.rbegin(); aI!=rSelectedPages.rend(); ++aI)
        {
            
            if (xPages->getCount() <= 1)
                break;

            const sal_uInt16 nPage (model::FromCoreIndex((*aI)->GetPageNum()));

            Reference< XDrawPage > xPage( xPages->getByIndex( nPage ), UNO_QUERY_THROW );
            xPages->remove(xPage);
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("SelectionManager::DeleteSelectedNormalPages(), exception caught!");
    }
}




void SelectionManager::DeleteSelectedMasterPages (const ::std::vector<SdPage*>& rSelectedPages)
{
    
    OSL_ASSERT(mrSlideSorter.GetModel().GetEditMode() == EM_MASTERPAGE);

    try
    {
        Reference<drawing::XMasterPagesSupplier> xDrawPagesSupplier( mrSlideSorter.GetModel().GetDocument()->getUnoModel(), UNO_QUERY_THROW );
        Reference<drawing::XDrawPages> xPages( xDrawPagesSupplier->getMasterPages(), UNO_QUERY_THROW );

        
        
        
        
        ::std::vector<SdPage*>::const_reverse_iterator aI;
        for (aI=rSelectedPages.rbegin(); aI!=rSelectedPages.rend(); ++aI)
        {
            
            if (xPages->getCount() <= 1)
                break;

            const sal_uInt16 nPage (model::FromCoreIndex((*aI)->GetPageNum()));

            Reference< XDrawPage > xPage( xPages->getByIndex( nPage ), UNO_QUERY_THROW );
            xPages->remove(xPage);
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("SelectionManager::DeleteSelectedMasterPages(), exception caught!");
    }
}




void SelectionManager::SelectionHasChanged (const bool bMakeSelectionVisible)
{
    if (bMakeSelectionVisible)
        mbIsMakeSelectionVisiblePending = true;

    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell != NULL)
    {
        pViewShell->Invalidate (SID_EXPAND_PAGE);
        pViewShell->Invalidate (SID_SUMMARY_PAGE);
        pViewShell->Invalidate(SID_SHOW_SLIDE);
        pViewShell->Invalidate(SID_HIDE_SLIDE);
        pViewShell->Invalidate(SID_DELETE_PAGE);
        pViewShell->Invalidate(SID_DELETE_MASTER_PAGE);
        pViewShell->Invalidate(SID_ASSIGN_LAYOUT);

        
        pViewShell->Invalidate (SID_STATUS_PAGE);
        pViewShell->Invalidate (SID_STATUS_LAYOUT);

        OSL_ASSERT(mrController.GetCurrentSlideManager());
        SharedPageDescriptor pDescriptor(mrController.GetCurrentSlideManager()->GetCurrentSlide());
        if (pDescriptor.get() != NULL)
            pViewShell->UpdatePreview(pDescriptor->GetPage());

        
        ::std::vector<Link>::iterator iListener (maSelectionChangeListeners.begin());
        ::std::vector<Link>::iterator iEnd (maSelectionChangeListeners.end());
        for (; iListener!=iEnd; ++iListener)
        {
            iListener->Call(NULL);
        }

        
        
        mnInsertionPosition = -1;
    }
}




void SelectionManager::AddSelectionChangeListener (const Link& rListener)
{
    if (::std::find (
        maSelectionChangeListeners.begin(),
        maSelectionChangeListeners.end(),
        rListener) == maSelectionChangeListeners.end())
    {
        maSelectionChangeListeners.push_back (rListener);
    }
}




void SelectionManager::RemoveSelectionChangeListener(const Link&rListener)
{
    maSelectionChangeListeners.erase (
        ::std::find (
            maSelectionChangeListeners.begin(),
            maSelectionChangeListeners.end(),
            rListener));
}




sal_Int32 SelectionManager::GetInsertionPosition (void) const
{
    sal_Int32 nInsertionPosition (mnInsertionPosition);
    if (nInsertionPosition < 0)
    {
        model::PageEnumeration aSelectedPages
            (model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        
        
        nInsertionPosition = mrSlideSorter.GetModel().GetPageCount();
        while (aSelectedPages.HasMoreElements())
        {
            const sal_Int32 nPosition (aSelectedPages.GetNextElement()->GetPage()->GetPageNum());
            
            
            nInsertionPosition = model::FromCoreIndex(nPosition) + 1;
        }

    }
    return nInsertionPosition;
}




void SelectionManager::SetInsertionPosition (const sal_Int32 nInsertionPosition)
{
    if (nInsertionPosition < 0)
        mnInsertionPosition = -1;
    else if (nInsertionPosition > mrSlideSorter.GetModel().GetPageCount())
    {
        
        OSL_ASSERT(nInsertionPosition<=mrSlideSorter.GetModel().GetPageCount());
        return;
    }
    else
        mnInsertionPosition = nInsertionPosition;
}




::boost::shared_ptr<SelectionObserver> SelectionManager::GetSelectionObserver (void) const
{
    return mpSelectionObserver;
}

} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
