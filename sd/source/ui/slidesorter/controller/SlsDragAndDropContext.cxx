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
#include "controller/SlsClipboard.hxx"
#include "controller/SlsTransferableData.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "sdtreelb.hxx"
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

    // For poperly handling transferables created by the navigator we
    // need additional information.  For this a user data object is
    // created that contains the necessary information.
    SdTransferable* pTransferable = SD_MOD()->pTransferDrag;
    SdPageObjsTLB::SdPageObjsTransferable* pTreeListBoxTransferable
        = dynamic_cast<SdPageObjsTLB::SdPageObjsTransferable*>(pTransferable);
    if (pTreeListBoxTransferable!=NULL && !TransferableData::GetFromTransferable(pTransferable))
    {
        pTransferable->AddUserData(
            rSlideSorter.GetController().GetClipboard().CreateTransferableUserData(pTransferable));
    }

    rSlideSorter.GetController().GetInsertionIndicatorHandler()->UpdateIndicatorIcon(pTransferable);
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
        bool bIsMasterPage (false);
        const sal_uInt32 nPageIndex (pDocument->GetPageByName(sPageName, bIsMasterPage));
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
