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

#include "SlsDragAndDropContext.hxx"

#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsTransferable.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include <sfx2/bindings.hxx>
#include <boost/bind.hpp>

namespace sd { namespace slidesorter { namespace controller {

DragAndDropContext::DragAndDropContext (SlideSorter& rSlideSorter)
    : mpTargetSlideSorter(&rSlideSorter),
      mnInsertionIndex(-1)
{
    ::std::vector<const SdPage*> aPages;

    // No Drag-and-Drop for master pages.
    if (rSlideSorter.GetModel().GetEditMode() != EM_PAGE)
        return;

    rSlideSorter.GetController().GetInsertionIndicatorHandler()->UpdateIndicatorIcon(
        dynamic_cast<Transferable*>(SD_MOD()->pTransferDrag));
}




DragAndDropContext::~DragAndDropContext (void)
{
    SetTargetSlideSorter (NULL, Point(0,0), InsertionIndicatorHandler::UnknownMode, false);
}




void DragAndDropContext::GetPagesFromBookmarks (
    ::std::vector<const SdPage*>& rPages,
    sal_Int32& rnSelectionCount,
    DrawDocShell* pDocShell,
    const List& rBookmarks) const
{
    if (pDocShell == NULL)
        return;

    const SdDrawDocument* pDocument = pDocShell->GetDoc();
    if (pDocument == NULL)
        return;

    for (sal_uLong nIndex=0,nCount=rBookmarks.Count(); nIndex<nCount; ++nIndex)
    {
        const String sPageName (*static_cast<String*>(rBookmarks.GetObject(nIndex)));
        sal_Bool bIsMasterPage (sal_False);
        const sal_uInt16 nPageIndex (pDocument->GetPageByName(sPageName, bIsMasterPage));
        if (nPageIndex == SDRPAGE_NOTFOUND)
            continue;

        const SdPage* pPage = dynamic_cast<const SdPage*>(pDocument->GetPage(nPageIndex));
        if (pPage != NULL)
            rPages.push_back(pPage);
    }
    rnSelectionCount = rBookmarks.Count();
}




void DragAndDropContext::GetPagesFromSelection (
    ::std::vector<const SdPage*>& rPages,
    sal_Int32& rnSelectionCount,
    model::PageEnumeration& rSelection) const
{
    // Show a new substitution for the selected page objects.
    rnSelectionCount = 0;

    while (rSelection.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (rSelection.GetNextElement());
        if (rPages.size() < 3)
            rPages.push_back(pDescriptor->GetPage());
        ++rnSelectionCount;
    }
}




void DragAndDropContext::Dispose (void)
{
    mnInsertionIndex = -1;
}




void DragAndDropContext::UpdatePosition (
    const Point& rMousePosition,
    const InsertionIndicatorHandler::Mode eMode,
    const bool bAllowAutoScroll)
{
    if (mpTargetSlideSorter == NULL)
        return;

    if (mpTargetSlideSorter->GetProperties()->IsUIReadOnly())
        return;

    // Convert window coordinates into model coordinates (we need the
    // window coordinates for auto-scrolling because that remains
    // constant while scrolling.)
    SharedSdWindow pWindow (mpTargetSlideSorter->GetContentWindow());
    const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));
    ::boost::shared_ptr<InsertionIndicatorHandler> pInsertionIndicatorHandler (
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler());

    if ( ! (bAllowAutoScroll
            && mpTargetSlideSorter->GetController().GetScrollBarManager().AutoScroll(
                rMousePosition,
                ::boost::bind(
                    &DragAndDropContext::UpdatePosition, this, rMousePosition, eMode, false))))
    {
        pInsertionIndicatorHandler->UpdatePosition(aMouseModelPosition, eMode);

        // Remember the new insertion index.
        mnInsertionIndex = pInsertionIndicatorHandler->GetInsertionPageIndex();
        if (pInsertionIndicatorHandler->IsInsertionTrivial(mnInsertionIndex, eMode))
            mnInsertionIndex = -1;
    }
}




void DragAndDropContext::SetTargetSlideSorter (
    SlideSorter* pSlideSorter,
    const Point aMousePosition,
    const InsertionIndicatorHandler::Mode eMode,
    const bool bIsOverSourceView)
{
    if (mpTargetSlideSorter != NULL)
    {
        mpTargetSlideSorter->GetController().GetScrollBarManager().StopAutoScroll();
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->End(
            Animator::AM_Animated);
    }

    mpTargetSlideSorter = pSlideSorter;

    if (mpTargetSlideSorter != NULL)
    {
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->Start(
            bIsOverSourceView);
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->UpdatePosition(
            aMousePosition,
            eMode);

    }
}


} } } // end of namespace ::sd::slidesorter::controller
