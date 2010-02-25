/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsPageSelector.cxx,v $
 * $Revision: 1.11 $
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

#include "controller/SlsPageSelector.hxx"

#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsAnimator.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlideSorterModel.hxx"
#include "view/SlideSorterView.hxx"

#include "sdpage.hxx"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellBase.hxx"
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <boost/bind.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::slidesorter::model;
using namespace ::sd::slidesorter::view;


namespace sd { namespace slidesorter { namespace controller {

PageSelector::PageSelector (SlideSorter& rSlideSorter)
    : mrModel(rSlideSorter.GetModel()),
      mrSlideSorter(rSlideSorter),
      mrController(mrSlideSorter.GetController()),
      mnSelectedPageCount(0),
      mnBroadcastDisableLevel(0),
      mbSelectionChangeBroadcastPending(false),
      mpMostRecentlySelectedPage(),
      mpSelectionAnchor(),
      mpCurrentPage(),
      mnUpdateLockCount(0),
      mbIsUpdateCurrentPagePending(false)
{
    CountSelectedPages ();
}




void PageSelector::SelectAllPages (void)
{
    PageSelector::UpdateLock aLock (*this);

    int nPageCount = mrModel.GetPageCount();
    for (int nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
        SelectPage (nPageIndex);
}




void PageSelector::DeselectAllPages (void)
{
    PageSelector::UpdateLock aLock (*this);

    int nPageCount = mrModel.GetPageCount();
    for (int nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
        DeselectPage (nPageIndex);
    DBG_ASSERT (mnSelectedPageCount==0,
        "PageSelector::DeselectAllPages: the selected pages counter is not 0");
    mnSelectedPageCount = 0;
    mpMostRecentlySelectedPage.reset();
    mpSelectionAnchor.reset();
}




void PageSelector::GetCoreSelection (void)
{
    PageSelector::UpdateLock aLock (*this);

    bool bSelectionHasChanged (true);
    mnSelectedPageCount = 0;
    model::PageEnumeration aAllPages (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
    while (aAllPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        if (pDescriptor->GetCoreSelection())
        {
            mrSlideSorter.GetView().RequestRepaint(pDescriptor);
            bSelectionHasChanged = true;
        }

        if (pDescriptor->HasState(PageDescriptor::ST_Selected))
            mnSelectedPageCount++;
    }

    if (bSelectionHasChanged)
    {
        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.GetSelectionManager()->SelectionHasChanged();
    }
}




void PageSelector::SetCoreSelection (void)
{
    model::PageEnumeration aAllPages (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
    while (aAllPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        pDescriptor->SetCoreSelection();
    }
}




void PageSelector::SelectPage (int nPageIndex)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        SelectPage(pDescriptor);
}




void PageSelector::SelectPage (const SdPage* pPage)
{
    int nPageIndex = (pPage->GetPageNum()-1) / 2;
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get()!=NULL && pDescriptor->GetPage()==pPage)
        SelectPage(pDescriptor);
}




void PageSelector::SelectPage (const SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get()!=NULL
        && mrSlideSorter.GetView().SetState(rpDescriptor, PageDescriptor::ST_Selected, true))
    {
        mnSelectedPageCount ++;
        mrSlideSorter.GetView().RequestRepaint(rpDescriptor);

        mpMostRecentlySelectedPage = rpDescriptor;
        if (mpSelectionAnchor == NULL)
            mpSelectionAnchor = rpDescriptor;

        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.GetSelectionManager()->SelectionHasChanged();
        UpdateCurrentPage();
    }
}




void PageSelector::DeselectPage (int nPageIndex)
{
    model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        DeselectPage(pDescriptor);
}




void PageSelector::DeselectPage (const SdPage* pPage)
{
    int nPageIndex = (pPage->GetPageNum()-1) / 2;
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get()!=NULL && pDescriptor->GetPage()==pPage)
        DeselectPage(pDescriptor);
}




void PageSelector::DeselectPage (const SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get()!=NULL
        && mrSlideSorter.GetView().SetState(rpDescriptor, PageDescriptor::ST_Selected, false))
    {
        mnSelectedPageCount --;
        mrSlideSorter.GetView().RequestRepaint(rpDescriptor);
        if (mpMostRecentlySelectedPage == rpDescriptor)
            mpMostRecentlySelectedPage.reset();
        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.GetSelectionManager()->SelectionHasChanged();
        UpdateCurrentPage();
    }
}




bool PageSelector::IsPageSelected (int nPageIndex)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        return pDescriptor->HasState(PageDescriptor::ST_Selected);
    else
        return false;
}




int PageSelector::GetPageCount (void) const
{
    return mrModel.GetPageCount();
}




int PageSelector::GetSelectedPageCount (void) const
{
    return mnSelectedPageCount;
}




void PageSelector::PrepareModelChange (void)
{
    DeselectAllPages ();
}




void PageSelector::HandleModelChange (void)
{
    GetCoreSelection();
}




SharedPageDescriptor PageSelector::GetMostRecentlySelectedPage (void) const
{
    return mpMostRecentlySelectedPage;
}




void PageSelector::SetMostRecentlySelectedPage (const model::SharedPageDescriptor& rpDescriptor)
{
    mpMostRecentlySelectedPage = rpDescriptor;
}




SharedPageDescriptor PageSelector::GetSelectionAnchor (void) const
{
    return mpSelectionAnchor;
}




void PageSelector::CountSelectedPages (void)
{
    mnSelectedPageCount = 0;
    model::PageEnumeration aSelectedPages (
        model::PageEnumerationProvider::CreateSelectedPagesEnumeration(mrModel));
    while (aSelectedPages.HasMoreElements())
    {
        mnSelectedPageCount++;
        aSelectedPages.GetNextElement();
    }
}




void PageSelector::EnableBroadcasting (bool bMakeSelectionVisible)
{
    if (mnBroadcastDisableLevel > 0)
        mnBroadcastDisableLevel --;
    if (mnBroadcastDisableLevel==0 && mbSelectionChangeBroadcastPending)
    {
        mrController.GetSelectionManager()->SelectionHasChanged(bMakeSelectionVisible);
        mbSelectionChangeBroadcastPending = false;
    }
}




void PageSelector::DisableBroadcasting (void)
{
    mnBroadcastDisableLevel ++;
}




::boost::shared_ptr<PageSelector::PageSelection> PageSelector::GetPageSelection (void) const
{
    ::boost::shared_ptr<PageSelection> pSelection (new PageSelection());
    pSelection->reserve(GetSelectedPageCount());

    int nPageCount = GetPageCount();
    for (int nIndex=0; nIndex<nPageCount; nIndex++)
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nIndex));
        if (pDescriptor.get()!=NULL && pDescriptor->HasState(PageDescriptor::ST_Selected))
            pSelection->push_back(pDescriptor->GetPage());
    }

    return pSelection;
}




void PageSelector::SetPageSelection (
    const ::boost::shared_ptr<PageSelection>& rpSelection,
    const bool bUpdateCurrentPage)
{
    PageSelection::const_iterator iPage;
    for (iPage=rpSelection->begin(); iPage!=rpSelection->end(); ++iPage)
        SelectPage(*iPage);
    if (bUpdateCurrentPage)
        UpdateCurrentPage();
}




void PageSelector::UpdateCurrentPage (const bool bUpdateOnlyWhenPending)
{
    if (mnUpdateLockCount > 0)
    {
        mbIsUpdateCurrentPagePending = true;
        return;
    }

    if ( ! mbIsUpdateCurrentPagePending && bUpdateOnlyWhenPending)
        return;

    mbIsUpdateCurrentPagePending = false;

    // Make the first selected page the current page.
    const sal_Int32 nPageCount (GetPageCount());
    for (sal_Int32 nIndex=0; nIndex<nPageCount; ++nIndex)
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nIndex));
        if (pDescriptor && pDescriptor->HasState(PageDescriptor::ST_Selected))
        {
            // Switching the current slide normally sets also the selection
            // to just the new current slide.  To prevent that here we store
            // and at the end of this scope restore the current selection.
            ::boost::shared_ptr<PageSelection> pSelection (GetPageSelection());
            SharedPageDescriptor pRecentSelection (GetMostRecentlySelectedPage());

            mrController.GetCurrentSlideManager()->SwitchCurrentSlide(pDescriptor);

            // Restore the selection and prevent a recursive call to
            // UpdateCurrentPage().
            SetPageSelection(pSelection, false);
            // Restore the most recently selected page.  Important for
            // making the right part of the selection visible.
            mpMostRecentlySelectedPage = pRecentSelection;
            return;
        }
    }

    // No page is selected.  Do not change the current slide.
}




//===== PageSelector::UpdateLock ==============================================

PageSelector::UpdateLock::UpdateLock (SlideSorter& rSlideSorter)
    : mrSelector(rSlideSorter.GetController().GetPageSelector())
{
    ++mrSelector.mnUpdateLockCount;
}




PageSelector::UpdateLock::UpdateLock (PageSelector& rSelector)
    : mrSelector(rSelector)
{
    ++mrSelector.mnUpdateLockCount;
}




PageSelector::UpdateLock::~UpdateLock (void)
{
    --mrSelector.mnUpdateLockCount;
    OSL_ASSERT(mrSelector.mnUpdateLockCount >= 0);
    if (mrSelector.mnUpdateLockCount == 0)
        mrSelector.UpdateCurrentPage(true);
}




} } } // end of namespace ::sd::slidesorter::controller
