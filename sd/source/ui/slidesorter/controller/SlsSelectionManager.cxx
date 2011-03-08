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

#include "controller/SlsSelectionManager.hxx"

#include "SlideSorter.hxx"
#include "SlsSelectionCommand.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsAnimator.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsSlotManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
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


namespace {
    class VerticalVisibleAreaScroller
    {
    public:
        VerticalVisibleAreaScroller (SlideSorter& rSlideSorter,
            const double nStart, const double nEnd);
        void operator() (const double nValue);
    private:
        SlideSorter& mrSlideSorter;
        double mnStart;
        double mnEnd;
    };
    class HorizontalVisibleAreaScroller
    {
    public:
        HorizontalVisibleAreaScroller (SlideSorter& rSlideSorter,
            const double nStart, const double nEnd);
        void operator() (const double nValue);
    private:
        SlideSorter& mrSlideSorter;
        double mnStart;
        double mnEnd;
    };
}




SelectionManager::SelectionManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mrController(rSlideSorter.GetController()),
      maSelectionBeforeSwitch(),
      mbIsMakeSelectionVisiblePending(true)
{
}




SelectionManager::~SelectionManager (void)
{
}




void SelectionManager::DeleteSelectedPages (void)
{
    SlideSorterController::ModelChangeLock aLock (mrController);

    // Hide focus.
    bool bIsFocusShowing = mrController.GetFocusManager().IsFocusShowing();
    if (bIsFocusShowing)
        mrController.GetFocusManager().ToggleFocus();

    // Store pointers to all selected page descriptors.  This is necessary
    // because the pages get deselected when the first one is deleted.
    model::PageEnumeration aPageEnumeration (
        PageEnumerationProvider::CreateSelectedPagesEnumeration(mrSlideSorter.GetModel()));
    ::std::vector<SdPage*> aSelectedPages;
    while (aPageEnumeration.HasMoreElements())
        aSelectedPages.push_back (aPageEnumeration.GetNextElement()->GetPage());

    // The actual deletion of the selected pages is done in one of two
    // helper functions.  They are specialized for normal respectively for
    // master pages.
    mrSlideSorter.GetView().BegUndo (SdResId(STR_UNDO_DELETEPAGES));
    if (mrSlideSorter.GetModel().GetEditMode() == EM_PAGE)
        DeleteSelectedNormalPages(aSelectedPages);
    else
        DeleteSelectedMasterPages(aSelectedPages);
    mrSlideSorter.GetView().EndUndo ();

    mrController.HandleModelChange();
    aLock.Release();

    // Show focus and move it to next valid location.
    if (bIsFocusShowing)
        mrController.GetFocusManager().ToggleFocus ();
    mrController.GetFocusManager().MoveFocus (FocusManager::FMD_NONE);
}




void SelectionManager::DeleteSelectedNormalPages (const ::std::vector<SdPage*>& rSelectedPages)
{
    // Prepare the deletion via the UNO API.
    OSL_ASSERT(mrSlideSorter.GetModel().GetEditMode() == EM_PAGE);

    try
    {
        Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier( mrSlideSorter.GetModel().GetDocument()->getUnoModel(), UNO_QUERY_THROW );
        Reference<drawing::XDrawPages> xPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );

        // Iterate over all pages that where seleted when this method was called
        // and delete the draw page the notes page.  The iteration is done in
        // reverse order so that when one slide is not deleted (to avoid an
        // empty document) the remaining slide is the first one.
        ::std::vector<SdPage*>::const_reverse_iterator aI;
        for (aI=rSelectedPages.rbegin(); aI!=rSelectedPages.rend(); aI++)
        {
            // Do not delete the last slide in the document.
            if (xPages->getCount() <= 1)
                break;

            USHORT nPage = ((*aI)->GetPageNum()-1) / 2;

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
    // Prepare the deletion via the UNO API.
    OSL_ASSERT(mrSlideSorter.GetModel().GetEditMode() == EM_MASTERPAGE);

    try
    {
        Reference<drawing::XMasterPagesSupplier> xDrawPagesSupplier( mrSlideSorter.GetModel().GetDocument()->getUnoModel(), UNO_QUERY_THROW );
        Reference<drawing::XDrawPages> xPages( xDrawPagesSupplier->getMasterPages(), UNO_QUERY_THROW );

        // Iterate over all pages that where seleted when this method was called
        // and delete the draw page the notes page.  The iteration is done in
        // reverse order so that when one slide is not deleted (to avoid an
        // empty document) the remaining slide is the first one.
        ::std::vector<SdPage*>::const_reverse_iterator aI;
        for (aI=rSelectedPages.rbegin(); aI!=rSelectedPages.rend(); aI++)
        {
            // Do not delete the last slide in the document.
            if (xPages->getCount() <= 1)
                break;

            USHORT nPage = ((*aI)->GetPageNum()-1) / 2;

            Reference< XDrawPage > xPage( xPages->getByIndex( nPage ), UNO_QUERY_THROW );
            xPages->remove(xPage);
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("SelectionManager::DeleteSelectedMasterPages(), exception caught!");
    }
}




bool SelectionManager::MoveSelectedPages (const sal_Int32 nTargetPageIndex)
{
    bool bMoved (false);
    PageSelector& rSelector (mrController.GetPageSelector());

    mrSlideSorter.GetView().LockRedraw (TRUE);
    SlideSorterController::ModelChangeLock aLock (mrController);

    // Transfer selection of the slide sorter to the document.
    mrSlideSorter.GetModel().SynchronizeDocumentSelection ();

    // Detect how many pages lie between document start and insertion
    // position.
    sal_Int32 nPageCountBeforeTarget (0);
    ::boost::shared_ptr<PageSelector::PageSelection> pSelection (rSelector.GetPageSelection());
    PageSelector::PageSelection::const_iterator iSelectedPage (pSelection->begin());
    PageSelector::PageSelection::const_iterator iSelectionEnd (pSelection->end());
    for ( ; iSelectedPage!=iSelectionEnd; ++iSelectedPage)
    {
        if (*iSelectedPage==NULL)
            continue;
        if (((*iSelectedPage)->GetPageNum()-1)/2 <= nTargetPageIndex)
            ++nPageCountBeforeTarget;
        else
            break;
    }

    // Prepare to select the moved pages.
    SelectionCommand* pCommand = new SelectionCommand(
        rSelector,mrController.GetCurrentSlideManager(),mrSlideSorter.GetModel());
    sal_Int32 nSelectedPageCount (rSelector.GetSelectedPageCount());
    for (sal_Int32 nOffset=0; nOffset<nSelectedPageCount; ++nOffset)
        pCommand->AddSlide(sal::static_int_cast<USHORT>(
            nTargetPageIndex + nOffset - nPageCountBeforeTarget + 1));

    // At the moment we can not move master pages.
    if (nTargetPageIndex>=0)
    {
        if (mrSlideSorter.GetModel().GetEditMode() == EM_PAGE)
            bMoved = mrSlideSorter.GetModel().GetDocument()->MovePages(
                sal::static_int_cast<sal_uInt16>(nTargetPageIndex));
    }
    if (bMoved)
        mrController.GetSlotManager()->ExecuteCommandAsynchronously(
            ::std::auto_ptr<controller::Command>(pCommand));

    mrSlideSorter.GetView().LockRedraw (FALSE);

    return bMoved;
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

        // StatusBar
        pViewShell->Invalidate (SID_STATUS_PAGE);
        pViewShell->Invalidate (SID_STATUS_LAYOUT);

        OSL_ASSERT(mrController.GetCurrentSlideManager());
        SharedPageDescriptor pDescriptor(mrController.GetCurrentSlideManager()->GetCurrentSlide());
        if (pDescriptor.get() != NULL)
            pViewShell->UpdatePreview(pDescriptor->GetPage());

        // Tell the slection change listeners that the selection has changed.
        ::std::vector<Link>::iterator iListener (maSelectionChangeListeners.begin());
        ::std::vector<Link>::iterator iEnd (maSelectionChangeListeners.end());
        for (; iListener!=iEnd; ++iListener)
        {
            iListener->Call(NULL);
        }

        // Reset the insertion position: until set again it is calculated from
        // the current selection.
        mnInsertionPosition = -1;
    }
}




bool SelectionManager::IsMakeSelectionVisiblePending (void) const
{
    return mbIsMakeSelectionVisiblePending;
}




/** We have to distinguish three cases: 1) the selection is empty, 2) the
    selection fits completely into the visible area, 3) it does not.
    1) The visible area is not modified.
    2) When the selection fits completely into the visible area we have to
    decide how to align it.  It is done by scrolling it there and thus when
    we scoll up the (towards the end of the document) the bottom of the
    selection is aligned with the bottom of the window.  When we scroll
    down (towards the beginning of the document) the top of the selection is
    aligned with the top of the window.
    3) We have to decide what part of the selection to make visible.  Here
    we use the eSelectionHint and concentrate on either the first, the last,
    or the most recently selected page.  We then again apply the algorithm
    of a).

*/
Size SelectionManager::MakeSelectionVisible (const SelectionHint eSelectionHint)
{
    ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
    if (pWindow == NULL)
        return Size(0,0);

    mbIsMakeSelectionVisiblePending = false;

    // Determine the descriptors of the first and last selected page and the
    // bounding box that encloses their page objects.
    model::SharedPageDescriptor pFirst;
    model::SharedPageDescriptor pLast;
    Rectangle aSelectionBox;
    model::PageEnumeration aSelectedPages (
        PageEnumerationProvider::CreateSelectedPagesEnumeration(mrSlideSorter.GetModel()));
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());

        if (pFirst.get() == NULL)
            pFirst = pDescriptor;
        pLast = pDescriptor;

        aSelectionBox.Union (mrSlideSorter.GetView().GetPageBoundingBox (
            pDescriptor,
            view::SlideSorterView::CS_MODEL,
            view::SlideSorterView::BBT_INFO));
    }

    if (pFirst != NULL)
    {
        // Determine scroll direction and the position in model coordinates
        // that will be aligned with the top/left or bottom/right window
        // border.
        if (DoesSelectionExceedVisibleArea(aSelectionBox))
        {
            // We can show only a part of the selection.
            aSelectionBox = ResolveLargeSelection(pFirst,pLast, eSelectionHint);
        }

        return MakeRectangleVisible(aSelectionBox);
    }

    return Size(0,0);
}




Size SelectionManager::MakeRectangleVisible (const Rectangle& rBox)
{
    ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
    if (pWindow == NULL)
        return Size(0,0);

    Rectangle aVisibleArea (pWindow->PixelToLogic(
        Rectangle(
            Point(0,0),
            pWindow->GetOutputSizePixel())));

    if (mrSlideSorter.GetView().GetOrientation() == SlideSorterView::VERTICAL)
    {
        // Scroll the visible area to make aSelectionBox visible.
        sal_Int32 nNewTop (aVisibleArea.Top());
        if (mrSlideSorter.GetController().GetProperties()->IsCenterSelection())
        {
            nNewTop = rBox.Top() - (aVisibleArea.GetHeight() - rBox.GetHeight()) / 2;
        }
        else
        {
            if (rBox.Top() < aVisibleArea.Top())
                nNewTop = rBox.Top();
            else if (rBox.Bottom() > aVisibleArea.Bottom())
                nNewTop = rBox.Bottom() - aVisibleArea.GetHeight();
            // otherwise we do not modify the visible area.
        }

        // Make some corrections of the new visible area.
        Rectangle aModelArea (mrSlideSorter.GetView().GetModelArea());
        if (nNewTop + aVisibleArea.GetHeight() > aModelArea.Bottom())
            nNewTop = aModelArea.GetHeight() - aVisibleArea.GetHeight();
        if (nNewTop < aModelArea.Top())
            nNewTop = aModelArea.Top();

        // Scroll.
        if (nNewTop != aVisibleArea.Top())
        {
            mrController.GetAnimator()->AddAnimation(
                VerticalVisibleAreaScroller(mrSlideSorter, aVisibleArea.Top(), nNewTop),
                mrSlideSorter.GetController().GetProperties()->IsSmoothSelectionScrolling() ?
                    1000 : 0
                );
        }

        return Size(0,aVisibleArea.Top() - nNewTop);
    }
    else
    {
        // Scroll the visible area to make aSelectionBox visible.
        sal_Int32 nNewLeft (aVisibleArea.Left());
        if (mrSlideSorter.GetController().GetProperties()->IsCenterSelection())
        {
            nNewLeft = rBox.Left() - (aVisibleArea.GetWidth() - rBox.GetWidth()) / 2;
        }
        else
        {
            if (rBox.Left() < aVisibleArea.Left())
                nNewLeft = rBox.Left();
            else if (rBox.Right() > aVisibleArea.Right())
                nNewLeft = rBox.Right() - aVisibleArea.GetWidth();
            // otherwise we do not modify the visible area.
        }

        // Make some corrections of the new visible area.
        Rectangle aModelArea (mrSlideSorter.GetView().GetModelArea());
        if (nNewLeft + aVisibleArea.GetWidth() > aModelArea.Right())
            nNewLeft = aModelArea.GetWidth() - aVisibleArea.GetWidth();
        if (nNewLeft < aModelArea.Left())
            nNewLeft = aModelArea.Left();

        // Scroll.
        if (nNewLeft != aVisibleArea.Left())
        {
            mrController.GetAnimator()->AddAnimation(
                HorizontalVisibleAreaScroller(mrSlideSorter, aVisibleArea.Left(), nNewLeft),
                mrSlideSorter.GetController().GetProperties()->IsSmoothSelectionScrolling() ?
                    1000 : 0
                );
        }

        return Size(aVisibleArea.Left() - nNewLeft, 0);
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




bool SelectionManager::DoesSelectionExceedVisibleArea (const Rectangle& rSelectionBox) const
{
    ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
    if (pWindow == NULL)
        return true;

    Rectangle aVisibleArea (pWindow->PixelToLogic(
        Rectangle(
            Point(0,0),
            pWindow->GetOutputSizePixel())));
    if (mrSlideSorter.GetView().GetOrientation() == SlideSorterView::VERTICAL)
        return rSelectionBox.GetHeight() > aVisibleArea.GetHeight();
    else
        return rSelectionBox.GetWidth() > aVisibleArea.GetWidth();
}




Rectangle SelectionManager::ResolveLargeSelection (
    const SharedPageDescriptor& rpFirst,
    const SharedPageDescriptor& rpLast,
    const SelectionHint eSelectionHint)
{
    OSL_ASSERT(rpFirst.get()!=NULL);
    OSL_ASSERT(rpLast.get()!=NULL);

    // The mose recently selected page is assumed to lie in the range
    // between first and last selected page.  Therefore the bounding box is
    // not modified.
    model::SharedPageDescriptor pRecent (
        mrController.GetPageSelector().GetMostRecentlySelectedPage());

    // Get the bounding box of the page object on which to concentrate.
    model::SharedPageDescriptor pRepresentative;
    switch (eSelectionHint)
    {
        case SH_FIRST:
            pRepresentative = rpFirst;
            break;

        case SH_LAST:
            pRepresentative = rpLast;
            break;

        case SH_RECENT:
        default:
            if (pRecent.get() == NULL)
                pRepresentative = rpFirst;
            else
                pRepresentative = pRecent;
            break;
    }
    OSL_ASSERT(pRepresentative.get() != NULL);

    return mrSlideSorter.GetView().GetPageBoundingBox (
        pRepresentative,
        view::SlideSorterView::CS_MODEL,
        view::SlideSorterView::BBT_INFO);
}




sal_Int32 SelectionManager::GetInsertionPosition (void) const
{
    sal_Int32 nInsertionPosition (mnInsertionPosition);
    if (nInsertionPosition < 0)
    {
        model::PageEnumeration aSelectedPages
            (model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        // Initialize (for the case of an empty selection) with the position
        // at the end of the document.
        nInsertionPosition = mrSlideSorter.GetModel().GetPageCount();
        while (aSelectedPages.HasMoreElements())
        {
            const sal_Int32 nPosition (aSelectedPages.GetNextElement()->GetPage()->GetPageNum());
            // Convert *2+1 index to straight index (n-1)/2 after the page
            // (+1).
            nInsertionPosition = (nPosition-1)/2 + 1;
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
        // Assert but then ignore invalid values.
        OSL_ASSERT(nInsertionPosition<=mrSlideSorter.GetModel().GetPageCount());
        return;
    }
    else
        mnInsertionPosition = nInsertionPosition;
}




//===== VerticalVisibleAreaScroller ===========================================

namespace {

VerticalVisibleAreaScroller::VerticalVisibleAreaScroller (
    SlideSorter& rSlideSorter,
    const double nStart,
    const double nEnd)
    : mrSlideSorter(rSlideSorter),
      mnStart(nStart),
      mnEnd(nEnd)
{
}



void VerticalVisibleAreaScroller::operator() (const double nValue)
{
    mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
    mrSlideSorter.GetController().GetScrollBarManager().SetTop(
        int(mnStart * (1.0 - nValue) + mnEnd * nValue));
}




HorizontalVisibleAreaScroller::HorizontalVisibleAreaScroller (
    SlideSorter& rSlideSorter,
    const double nStart,
    const double nEnd)
    : mrSlideSorter(rSlideSorter),
      mnStart(nStart),
      mnEnd(nEnd)
{
}




void HorizontalVisibleAreaScroller::operator() (const double nValue)
{
    mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
    mrSlideSorter.GetController().GetScrollBarManager().SetLeft(
        int(mnStart * (1.0 - nValue) + mnEnd * nValue));
}

} // end of anonymous namespace

} } } // end of namespace ::sd::slidesorter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
