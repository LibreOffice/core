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

#include <controller/SlsPageSelector.hxx>

#include <SlideSorter.hxx>
#include <SlideSorterViewShell.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsSelectionManager.hxx>
#include <controller/SlsAnimator.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <controller/SlsVisibleAreaManager.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <model/SlideSorterModel.hxx>
#include <view/SlideSorterView.hxx>

#include <sdpage.hxx>
#include <ViewShell.hxx>
#include <DrawViewShell.hxx>
#include <ViewShellBase.hxx>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <memory>

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
      mnUpdateLockCount(0),
      mbIsUpdateCurrentPagePending(true)
{
    CountSelectedPages ();
}

void PageSelector::SelectAllPages()
{
    VisibleAreaManager::TemporaryDisabler aDisabler (mrSlideSorter);
    PageSelector::UpdateLock aLock (*this);

    int nPageCount = mrModel.GetPageCount();
    for (int nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
        SelectPage(nPageIndex);
}

void PageSelector::DeselectAllPages()
{
    VisibleAreaManager::TemporaryDisabler aDisabler (mrSlideSorter);
    PageSelector::UpdateLock aLock (*this);

    int nPageCount = mrModel.GetPageCount();
    for (int nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
        DeselectPage(nPageIndex);

    DBG_ASSERT (mnSelectedPageCount==0,
        "PageSelector::DeselectAllPages: the selected pages counter is not 0");
    mnSelectedPageCount = 0;
    mpSelectionAnchor.reset();
}

void PageSelector::GetCoreSelection()
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
            mrSlideSorter.GetController().GetVisibleAreaManager().RequestVisible(pDescriptor);
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

void PageSelector::SetCoreSelection()
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
    if (pDescriptor.get() != nullptr)
        SelectPage(pDescriptor);
}

void PageSelector::SelectPage (const SdPage* pPage)
{
    const sal_Int32 nPageIndex (mrModel.GetIndex(pPage));
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get()!=nullptr && pDescriptor->GetPage()==pPage)
        SelectPage(pDescriptor);
}

void PageSelector::SelectPage (const SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get()==nullptr
        || !mrSlideSorter.GetView().SetState(rpDescriptor, PageDescriptor::ST_Selected, true))
        return;

    ++mnSelectedPageCount;
    mrSlideSorter.GetController().GetVisibleAreaManager().RequestVisible(rpDescriptor,true);
    mrSlideSorter.GetView().RequestRepaint(rpDescriptor);

    mpMostRecentlySelectedPage = rpDescriptor;
    if (mpSelectionAnchor == nullptr)
        mpSelectionAnchor = rpDescriptor;

    if (mnBroadcastDisableLevel > 0)
        mbSelectionChangeBroadcastPending = true;
    else
        mrController.GetSelectionManager()->SelectionHasChanged();
    UpdateCurrentPage();

    CheckConsistency();
}

void PageSelector::DeselectPage (int nPageIndex)
{
    model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != nullptr)
        DeselectPage(pDescriptor);
}

void PageSelector::DeselectPage (
    const SharedPageDescriptor& rpDescriptor,
    const bool bUpdateCurrentPage)
{
    if (rpDescriptor.get()==nullptr
        || mrSlideSorter.GetView().SetState(rpDescriptor, PageDescriptor::ST_Selected, false))
        return;

    --mnSelectedPageCount;
    mrSlideSorter.GetController().GetVisibleAreaManager().RequestVisible(rpDescriptor);
    mrSlideSorter.GetView().RequestRepaint(rpDescriptor);
    if (mpMostRecentlySelectedPage == rpDescriptor)
        mpMostRecentlySelectedPage.reset();
    if (mnBroadcastDisableLevel > 0)
        mbSelectionChangeBroadcastPending = true;
    else
        mrController.GetSelectionManager()->SelectionHasChanged();
    if (bUpdateCurrentPage)
        UpdateCurrentPage();

    CheckConsistency();
}

void PageSelector::CheckConsistency() const
{
    int nSelectionCount (0);
    for (int nPageIndex=0,nPageCount=mrModel.GetPageCount(); nPageIndex<nPageCount; nPageIndex++)
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
        assert(pDescriptor);
        if (pDescriptor->HasState(PageDescriptor::ST_Selected))
            ++nSelectionCount;
    }
    if (nSelectionCount!=mnSelectedPageCount)
    {
        // #i120020# The former call to assert(..) internally calls
        // SlideSorterModel::GetPageDescriptor which will crash in this situation
        // (only in non-pro code). All what is wanted there is to assert it (the
        // error is already detected), so do this directly.
        OSL_ENSURE(false, "PageSelector: Consistency error (!)");
    }
}

bool PageSelector::IsPageSelected (int nPageIndex)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != nullptr)
        return pDescriptor->HasState(PageDescriptor::ST_Selected);
    else
        return false;
}

int PageSelector::GetPageCount() const
{
    return mrModel.GetPageCount();
}

void PageSelector::CountSelectedPages()
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

void PageSelector::EnableBroadcasting()
{
    if (mnBroadcastDisableLevel > 0)
        mnBroadcastDisableLevel --;
    if (mnBroadcastDisableLevel==0 && mbSelectionChangeBroadcastPending)
    {
        mrController.GetSelectionManager()->SelectionHasChanged();
        mbSelectionChangeBroadcastPending = false;
    }
}

void PageSelector::DisableBroadcasting()
{
    mnBroadcastDisableLevel ++;
}

std::shared_ptr<PageSelector::PageSelection> PageSelector::GetPageSelection() const
{
    std::shared_ptr<PageSelection> pSelection (new PageSelection);
    pSelection->reserve(GetSelectedPageCount());

    int nPageCount = GetPageCount();
    for (int nIndex=0; nIndex<nPageCount; nIndex++)
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nIndex));
        if (pDescriptor.get()!=nullptr && pDescriptor->HasState(PageDescriptor::ST_Selected))
            pSelection->push_back(pDescriptor->GetPage());
    }

    return pSelection;
}

void PageSelector::SetPageSelection (
    const std::shared_ptr<PageSelection>& rpSelection,
    const bool bUpdateCurrentPage)
{
    for (const auto& rpPage : *rpSelection)
        SelectPage(rpPage);
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
    SharedPageDescriptor pCurrentPageDescriptor;
    const sal_Int32 nPageCount (GetPageCount());
    for (sal_Int32 nIndex=0; nIndex<nPageCount; ++nIndex)
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nIndex));
        if ( ! pDescriptor)
            continue;
        if (pDescriptor->HasState(PageDescriptor::ST_Selected))
        {
            pCurrentPageDescriptor = pDescriptor;
            break;
        }
    }

    if (!pCurrentPageDescriptor)
        return;

    // Switching the current slide normally sets also the
    // selection to just the new current slide.  To prevent that,
    // we store (and at the end of this scope restore) the current
    // selection.
    std::shared_ptr<PageSelection> pSelection (GetPageSelection());

    mrController.GetCurrentSlideManager()->SwitchCurrentSlide(pCurrentPageDescriptor);

    // Restore the selection and prevent a recursive call to
    // UpdateCurrentPage().
    SetPageSelection(pSelection, false);
}

//===== PageSelector::UpdateLock ==============================================

PageSelector::UpdateLock::UpdateLock (SlideSorter const & rSlideSorter)
    : mpSelector(&rSlideSorter.GetController().GetPageSelector())
{
    ++mpSelector->mnUpdateLockCount;
}

PageSelector::UpdateLock::UpdateLock (PageSelector& rSelector)
    : mpSelector(&rSelector)
{
    ++mpSelector->mnUpdateLockCount;
}

PageSelector::UpdateLock::~UpdateLock()
{
    Release();
}

void PageSelector::UpdateLock::Release()
{
    if (mpSelector != nullptr)
    {
        --mpSelector->mnUpdateLockCount;
        OSL_ASSERT(mpSelector->mnUpdateLockCount >= 0);
        if (mpSelector->mnUpdateLockCount == 0)
            mpSelector->UpdateCurrentPage(true);

        mpSelector = nullptr;
    }
}

//===== PageSelector::BroadcastLock ==============================================

PageSelector::BroadcastLock::BroadcastLock (SlideSorter const & rSlideSorter)
    : mrSelector(rSlideSorter.GetController().GetPageSelector())
{
    mrSelector.DisableBroadcasting();
}

PageSelector::BroadcastLock::BroadcastLock (PageSelector& rSelector)
    : mrSelector(rSelector)
{
    mrSelector.DisableBroadcasting();
}

PageSelector::BroadcastLock::~BroadcastLock()
{
    mrSelector.EnableBroadcasting();
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
