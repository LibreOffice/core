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


#include "controller/SlsClipboard.hxx"

#include "SlideSorterViewShell.hxx"
#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsTheme.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsTransferableData.hxx"
#include "controller/SlsSelectionObserver.hxx"
#include "controller/SlsVisibleAreaManager.hxx"
#include "cache/SlsPageCache.hxx"

#include "ViewShellBase.hxx"
#include "View.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"
#include "fupoor.hxx"
#include "fuzoom.hxx"
#include "fucushow.hxx"
#include "fusldlg.hxx"
#include "fuexpand.hxx"
#include "fusumry.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "sdxfer.hxx"
#include "sdmod.hxx"
#include "ins_paste.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "sdtreelb.hxx"

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svxids.hrc>
#include <svx/svdstr.hrc>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <boost/bind.hpp>


namespace sd { namespace slidesorter { namespace controller {

namespace {
/** Temporarily deactivate slide tracking of the VisibleAreaManager.
    This is used as a workaround to avoid unwanted repositioning of
    the visible area when the selection of slides is copied to the
    clipboard (cloning of slides leads to model change notifications
    for the original model.)
*/
class TemporarySlideTrackingDeactivator
{
public:
    TemporarySlideTrackingDeactivator (SlideSorterController& rController)
        : mrController(rController),
          mbIsCurrentSlideTrackingActive (
              mrController.GetVisibleAreaManager().IsCurrentSlideTrackingActive())
    {
        if (mbIsCurrentSlideTrackingActive)
            mrController.GetVisibleAreaManager().DeactivateCurrentSlideTracking();
    }
    ~TemporarySlideTrackingDeactivator (void)
    {
        if (mbIsCurrentSlideTrackingActive)
            mrController.GetVisibleAreaManager().ActivateCurrentSlideTracking();
    }

private:
    SlideSorterController& mrController;
    const bool mbIsCurrentSlideTrackingActive;
};
} // end of anonymous namespace

class Clipboard::UndoContext
{
public:
    UndoContext (
        SdDrawDocument* pDocument,
        const ::boost::shared_ptr<ViewShell>& rpMainViewShell)
        : mpDocument(pDocument),
          mpMainViewShell(rpMainViewShell)
    {
        if (mpDocument!=NULL && mpDocument->IsUndoEnabled())
        {
            if (mpMainViewShell && mpMainViewShell->GetShellType() == ViewShell::ST_DRAW)
                mpDocument->BegUndo(SD_RESSTR(STRING_DRAG_AND_DROP_PAGES));
            else
                mpDocument->BegUndo(SD_RESSTR(STRING_DRAG_AND_DROP_SLIDES));
        }
    }

    ~UndoContext (void)
    {
        if (mpDocument!=NULL && mpDocument->IsUndoEnabled())
            mpDocument->EndUndo();
        if (mpMainViewShell && mpMainViewShell->GetViewFrame()!=NULL)
        {
            SfxBindings& rBindings = mpMainViewShell->GetViewFrame()->GetBindings();
            rBindings.Invalidate(SID_UNDO);
            rBindings.Invalidate(SID_REDO);
        }
    }
private:
    SdDrawDocument* mpDocument;
    ::boost::shared_ptr<ViewShell> mpMainViewShell;
};




Clipboard::Clipboard (SlideSorter& rSlideSorter)
    : ViewClipboard(rSlideSorter.GetView()),
      mrSlideSorter(rSlideSorter),
      mrController(mrSlideSorter.GetController()),
      maPagesToRemove(),
      maPagesToSelect(),
      mbUpdateSelectionPending(false),
      mpUndoContext(),
      mpSelectionObserverContext(),
      mnDragFinishedUserEventId(0)
{
}




Clipboard::~Clipboard (void)
{
    if (mnDragFinishedUserEventId != 0)
        Application::RemoveUserEvent(mnDragFinishedUserEventId);
}




/** With the current implementation the forwarded calls to the current
    function will come back eventually to call the local Do(Cut|Copy|Paste)
    methods.  A shortcut is possible but would be an unclean hack.
*/
void Clipboard::HandleSlotCall (SfxRequest& rRequest)
{
    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    FunctionReference xFunc;
    if (pViewShell != NULL)
        xFunc = pViewShell->GetCurrentFunction();
    switch (rRequest.GetSlot())
    {
        case SID_CUT:
            if (mrSlideSorter.GetModel().GetEditMode() != EM_MASTERPAGE)
            {
                if(xFunc.is())
                    xFunc->DoCut();
                else
                    DoCut();
            }
            rRequest.Done();
            break;

        case SID_COPY:
            if (mrSlideSorter.GetModel().GetEditMode() != EM_MASTERPAGE)
            {
                if(xFunc.is())
                    xFunc->DoCopy();
                else
                    DoCopy();
            }
            rRequest.Done();
            break;

        case SID_PASTE:
            // Prevent redraws while inserting pages from the clipboard
            // because the intermediate inconsistent state might lead to
            // a crash.
            if (mrSlideSorter.GetModel().GetEditMode() != EM_MASTERPAGE)
            {
                view::SlideSorterView::DrawLock aLock (mrSlideSorter);
                SelectionObserver::Context aContext (mrSlideSorter);
                if(xFunc.is())
                    xFunc->DoPaste();
                else
                    DoPaste();
            }
            rRequest.Done();
            break;

        case SID_DELETE:
            DoDelete();
            rRequest.Done();
            break;
    }
}




void Clipboard::DoCut (::Window* pWindow)
{
    if (mrSlideSorter.GetModel().GetPageCount() > 1)
    {
        DoCopy(pWindow);
        DoDelete(pWindow);
    }
}




void Clipboard::DoDelete (::Window* )
{
    if (mrSlideSorter.GetModel().GetPageCount() > 1)
    {
        mrController.GetSelectionManager()->DeleteSelectedPages();
    }
}




void Clipboard::DoCopy (::Window* pWindow )
{
    CreateSlideTransferable( pWindow, sal_False );
}




void Clipboard::DoPaste (::Window* pWindow)
{
    SdTransferable* pClipTransferable = SD_MOD()->pTransferClip;

    if (pClipTransferable!=NULL && pClipTransferable->IsPageTransferable())
    {
        sal_Int32 nInsertPosition = GetInsertionPosition(pWindow);

        if (nInsertPosition >= 0)
        {
            // Paste the pages from the clipboard.
            sal_Int32 nInsertPageCount = PasteTransferable(nInsertPosition);
            // Select the pasted pages and make the first of them the
            // current page.
            mrSlideSorter.GetContentWindow()->GrabFocus();
            SelectPageRange(nInsertPosition, nInsertPageCount);
        }
    }
}




sal_Int32 Clipboard::GetInsertionPosition (::Window* pWindow)
{
    sal_Int32 nInsertPosition = -1;

    // Determine the insertion position:
    // a) When the insertion indicator is visible, then at that position.
    // b) When the focus indicator is visible, then before or after the
    // focused page, depending on user input to a dialog.
    // c) When there is a selection but no focus, then after the
    // selection.
    // d) After the last page when there is no selection and no focus.

    ::boost::shared_ptr<controller::InsertionIndicatorHandler> pInsertionIndicatorHandler (
        mrController.GetInsertionIndicatorHandler());
    if (pInsertionIndicatorHandler->IsActive())
    {
        // Use the insertion index of an active insertion indicator.
        nInsertPosition = pInsertionIndicatorHandler->GetInsertionPageIndex();
    }
    else if (mrController.GetSelectionManager()->GetInsertionPosition() >= 0)
    {
        // Use the insertion index of an insertion indicator that has been
        // deactivated a short while ago.
        nInsertPosition = mrController.GetSelectionManager()->GetInsertionPosition();
    }
    else if (mrController.GetFocusManager().IsFocusShowing())
    {
        // Use the focus to determine the insertion position.
        SdInsertPasteDlg aDialog (pWindow);
        if (aDialog.Execute() == RET_OK)
        {
            nInsertPosition = mrController.GetFocusManager().GetFocusedPageIndex();
            if ( ! aDialog.IsInsertBefore())
                nInsertPosition ++;
        }
    }

    return nInsertPosition;
}




sal_Int32 Clipboard::PasteTransferable (sal_Int32 nInsertPosition)
{
    SdTransferable* pClipTransferable = SD_MOD()->pTransferClip;
    model::SlideSorterModel& rModel (mrSlideSorter.GetModel());
    bool bMergeMasterPages = !pClipTransferable->HasSourceDoc (rModel.GetDocument());
    sal_uInt16 nInsertIndex (rModel.GetCoreIndex(nInsertPosition));
    sal_Int32 nInsertPageCount (0);
    if (pClipTransferable->HasPageBookmarks())
    {
        const std::vector<OUString> &rBookmarkList = pClipTransferable->GetPageBookmarks();
        const SolarMutexGuard aGuard;

        nInsertPageCount = (sal_uInt16) rBookmarkList.size();
        rModel.GetDocument()->InsertBookmarkAsPage(
            rBookmarkList,
            NULL,
            sal_False,
            sal_False,
            nInsertIndex,
            sal_False,
            pClipTransferable->GetPageDocShell(),
            sal_True,
            bMergeMasterPages,
            sal_False);
    }
    else
    {
        SfxObjectShell* pShell = pClipTransferable->GetDocShell();
        DrawDocShell* pDataDocSh = (DrawDocShell*)pShell;
        SdDrawDocument* pDataDoc = pDataDocSh->GetDoc();

        if (pDataDoc!=NULL
            && pDataDoc->GetSdPageCount(PK_STANDARD))
        {
            const SolarMutexGuard aGuard;

            bMergeMasterPages = (pDataDoc != rModel.GetDocument());
            nInsertPageCount = pDataDoc->GetSdPageCount( PK_STANDARD );
            rModel.GetDocument()->InsertBookmarkAsPage(
                std::vector<OUString>(),
                NULL,
                sal_False,
                sal_False,
                nInsertIndex,
                sal_False,
                pDataDocSh,
                sal_True,
                bMergeMasterPages,
                sal_False);
        }
    }
    mrController.HandleModelChange();
    return nInsertPageCount;
}




void Clipboard::SelectPageRange (sal_Int32 nFirstIndex, sal_Int32 nPageCount)
{
    // Select the newly inserted pages.  That are the nInsertPageCount pages
    // after the nInsertIndex position.
    PageSelector& rSelector (mrController.GetPageSelector());
    rSelector.DeselectAllPages();
    for (sal_uInt16 i=0; i<nPageCount; i++)
    {
        model::SharedPageDescriptor pDescriptor (
            mrSlideSorter.GetModel().GetPageDescriptor(nFirstIndex + i));
        if (pDescriptor.get() != NULL)
        {
            rSelector.SelectPage(pDescriptor);
            // The first page of the new selection is made the current page.
            if (i == 0)
            {
                mrController.GetCurrentSlideManager()->SwitchCurrentSlide(pDescriptor);
            }
        }
    }
}




void Clipboard::CreateSlideTransferable (
    ::Window* pWindow,
    bool bDrag)
{
    std::vector<OUString> aBookmarkList;

    // Insert all selected pages into a bookmark list and remember them in
    // maPagesToRemove for possible later removal.
    model::PageEnumeration aSelectedPages
        (model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mrSlideSorter.GetModel()));
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        aBookmarkList.push_back(pDescriptor->GetPage()->GetName());
        maPagesToRemove.push_back (pDescriptor->GetPage());
    }

    // Create a small set of representatives of the selection for which
    // previews are included into the transferable so that an insertion
    // indicator can be rendered.
    aSelectedPages.Rewind();
    ::std::vector<TransferableData::Representative> aRepresentatives;
    aRepresentatives.reserve(3);
    ::boost::shared_ptr<cache::PageCache> pPreviewCache (
        mrSlideSorter.GetView().GetPreviewCache());
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        if ( ! pDescriptor || pDescriptor->GetPage()==NULL)
            continue;
        Bitmap aPreview (pPreviewCache->GetPreviewBitmap(pDescriptor->GetPage(), false));
        aRepresentatives.push_back(TransferableData::Representative(
            aPreview,
            pDescriptor->HasState(model::PageDescriptor::ST_Excluded)));
        if (aRepresentatives.size() >= 3)
            break;
    }

    if (!aBookmarkList.empty())
    {
        mrSlideSorter.GetView().BrkAction();
        SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
        SdTransferable* pTransferable = TransferableData::CreateTransferable (
            pDocument,
            NULL,
            sal_False,
            dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell()),
            aRepresentatives);

        if (bDrag)
            SD_MOD()->pTransferDrag = pTransferable;
        else
            SD_MOD()->pTransferClip = pTransferable;

        pDocument->CreatingDataObj (pTransferable);
        pTransferable->SetWorkDocument( dynamic_cast<SdDrawDocument*>(pDocument->AllocModel()) );
        pDocument->CreatingDataObj (NULL);
        TransferableObjectDescriptor aObjDesc;
        pTransferable->GetWorkDocument()->GetDocSh()
            ->FillTransferableObjectDescriptor (aObjDesc);

        if (pDocument->GetDocSh() != NULL)
            aObjDesc.maDisplayName = pDocument->GetDocSh()
                ->GetMedium()->GetURLObject().GetURLNoPass();

        ::Window* pActionWindow = pWindow;
        if (pActionWindow == NULL)
        {
            ViewShell* pViewShell = mrSlideSorter.GetViewShell();
            if (pViewShell != NULL)
                pActionWindow = pViewShell->GetActiveWindow();
        }

        pTransferable->SetStartPos (pActionWindow->PixelToLogic(
            pActionWindow->GetPointerPosPixel()));
        pTransferable->SetObjectDescriptor (aObjDesc);

        {
            TemporarySlideTrackingDeactivator aDeactivator (mrController);
            pTransferable->SetPageBookmarks (aBookmarkList, !bDrag);
        }

        if (bDrag)
        {
            pTransferable->SetView (&mrSlideSorter.GetView());
            sal_Int8 nDragSourceActions (DND_ACTION_COPY);
            // The move action is available only when not all pages would be
            // moved.  Otherwise an empty document would remain.  Crash.
            sal_Int32 nRemainingPages = mrSlideSorter.GetModel().GetPageCount() - aBookmarkList.size();
            if (nRemainingPages > 0)
                nDragSourceActions |= DND_ACTION_MOVE;
            pTransferable->StartDrag (pActionWindow, nDragSourceActions);
        }
        else
            pTransferable->CopyToClipboard (pActionWindow);
    }
}




::boost::shared_ptr<SdTransferable::UserData> Clipboard::CreateTransferableUserData (SdTransferable* pTransferable)
{
    do
    {
        SdPageObjsTLB::SdPageObjsTransferable* pTreeListBoxTransferable
            = dynamic_cast<SdPageObjsTLB::SdPageObjsTransferable*>(pTransferable);
        if (pTreeListBoxTransferable == NULL)
            break;

        // Find view shell for the document of the transferable.
        ::sd::ViewShell* pViewShell
              = SdPageObjsTLB::GetViewShellForDocShell(pTreeListBoxTransferable->GetDocShell());
        if (pViewShell == NULL)
            break;

        // Find slide sorter for the document of the transferable.
        SlideSorterViewShell* pSlideSorterViewShell
            = SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
        if (pSlideSorterViewShell == NULL)
            break;
        SlideSorter& rSlideSorter (pSlideSorterViewShell->GetSlideSorter());

        // Get bookmark from transferable.
        TransferableDataHelper  aDataHelper (pTransferable);
        INetBookmark aINetBookmark;
        if ( ! aDataHelper.GetINetBookmark(SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark))
            break;
        const OUString sURL (aINetBookmark.GetURL());
        const sal_Int32 nIndex (sURL.indexOf((sal_Unicode)'#'));
        if (nIndex == -1)
            break;
        String sBookmark (sURL.copy(nIndex+1));

        // Make sure that the bookmark points to a page.
        SdDrawDocument* pTransferableDocument = rSlideSorter.GetModel().GetDocument();
        if (pTransferableDocument == NULL)
            break;
        sal_Bool bIsMasterPage = sal_False;
        const sal_uInt16 nPageIndex (pTransferableDocument->GetPageByName(sBookmark, bIsMasterPage));
        if (nPageIndex == SDRPAGE_NOTFOUND)
            break;

        // Create preview.
        ::std::vector<TransferableData::Representative> aRepresentatives;
        aRepresentatives.reserve(1);
        ::boost::shared_ptr<cache::PageCache> pPreviewCache (
            rSlideSorter.GetView().GetPreviewCache());
        model::SharedPageDescriptor pDescriptor (rSlideSorter.GetModel().GetPageDescriptor((nPageIndex-1)/2));
        if ( ! pDescriptor || pDescriptor->GetPage()==NULL)
            break;
        Bitmap aPreview (pPreviewCache->GetPreviewBitmap(pDescriptor->GetPage(), false));
        aRepresentatives.push_back(TransferableData::Representative(
                aPreview,
                pDescriptor->HasState(model::PageDescriptor::ST_Excluded)));

        // Remember the page in maPagesToRemove so that it can be removed
        // when drag and drop action is "move".
        Clipboard& rOtherClipboard (pSlideSorterViewShell->GetSlideSorter().GetController().GetClipboard());
        rOtherClipboard.maPagesToRemove.clear();
        rOtherClipboard.maPagesToRemove.push_back(pDescriptor->GetPage());

        // Create the new transferable.
        ::boost::shared_ptr<SdTransferable::UserData> pNewTransferable (
            new TransferableData(
                pSlideSorterViewShell,
                aRepresentatives));
        pTransferable->SetWorkDocument( dynamic_cast<SdDrawDocument*>(
                pTreeListBoxTransferable->GetSourceDoc()->AllocModel()));
        //        pTransferable->SetView(&mrSlideSorter.GetView());

        // Set page bookmark list.
        std::vector<OUString> aPageBookmarks;
        aPageBookmarks.push_back(sBookmark);
        pTransferable->SetPageBookmarks(aPageBookmarks, false);

        // Replace the view referenced by the transferable with the
        // corresponding slide sorter view.
        pTransferable->SetView(&pSlideSorterViewShell->GetSlideSorter().GetView());

        return pNewTransferable;
    }
    while (false);

    return ::boost::shared_ptr<SdTransferable::UserData>();
}




void Clipboard::StartDrag (
    const Point& rPosition,
    ::Window* pWindow)
{
    maPagesToRemove.clear();
    maPagesToSelect.clear();
    mbUpdateSelectionPending = false;
    CreateSlideTransferable(pWindow, sal_True);

    mrController.GetInsertionIndicatorHandler()->UpdatePosition(
        rPosition,
        InsertionIndicatorHandler::UnknownMode);
}




void Clipboard::DragFinished (sal_Int8 nDropAction)
{
    if (mnDragFinishedUserEventId == 0)
    {
        if ( ! Application::PostUserEvent(
            mnDragFinishedUserEventId,
            LINK(this, Clipboard, ProcessDragFinished),
            reinterpret_cast<void*>(nDropAction)))
        {
            mnDragFinishedUserEventId = 0;
        }
    }
}




IMPL_LINK(Clipboard, ProcessDragFinished, void*, pUserData)
{
    const sal_Int8 nDropAction (static_cast<sal_Int8>(reinterpret_cast<sal_IntPtr>(pUserData)));

    mnDragFinishedUserEventId = 0;

    // Hide the substitution display and insertion indicator.
    ::rtl::Reference<SelectionFunction> pFunction (mrController.GetCurrentSelectionFunction());
    if (pFunction.is())
        pFunction->NotifyDragFinished();

    PageSelector& rSelector (mrController.GetPageSelector());
    if ((nDropAction & DND_ACTION_MOVE) != 0
        && ! maPagesToRemove.empty())
    {
        // Remove the pages that have been moved to another place (possibly
        // in the same document.)
        rSelector.DeselectAllPages();
        PageList::iterator aDraggedPage;
        for (aDraggedPage=maPagesToRemove.begin();
             aDraggedPage!=maPagesToRemove.end();
             ++aDraggedPage)
        {
            rSelector.SelectPage(*aDraggedPage);
        }
        mrController.GetSelectionManager()->DeleteSelectedPages();
    }
    mpUndoContext.reset();
    mpSelectionObserverContext.reset();

    return 1;
}




sal_Int8 Clipboard::AcceptDrop (
    const AcceptDropEvent& rEvent,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    sal_Int8 nAction (DND_ACTION_NONE);

    const Clipboard::DropType eDropType (IsDropAccepted(rTargetHelper));

    switch (eDropType)
    {
        case DT_PAGE:
        case DT_PAGE_FROM_NAVIGATOR:
        {
            // Accept a drop.
            nAction = rEvent.mnAction;

            // Use the copy action when the drop action is the default, i.e. not
            // explicitly set to move or link, and when the source and
            // target models are not the same.
            SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
            if (pDragTransferable != NULL
                && pDragTransferable->IsPageTransferable()
                && ((rEvent.maDragEvent.DropAction
                        & ::com::sun::star::datatransfer::dnd::DNDConstants::ACTION_DEFAULT) != 0)
                && (mrSlideSorter.GetModel().GetDocument()->GetDocSh()
                    != pDragTransferable->GetPageDocShell()))
            {
                nAction = DND_ACTION_COPY;
            }
            else if (IsInsertionTrivial(pDragTransferable, nAction))
            {
                nAction = DND_ACTION_NONE;
            }

            // Show the insertion marker and the substitution for a drop.
            SelectionFunction* pSelectionFunction = dynamic_cast<SelectionFunction*>(
                mrSlideSorter.GetViewShell()->GetCurrentFunction().get());
            if (pSelectionFunction != NULL)
                pSelectionFunction->MouseDragged(rEvent, nAction);

            // Scroll the window when the mouse reaches the window border.
            //            mrController.GetScrollBarManager().AutoScroll (rEvent.maPosPixel);
        }
        break;

        case DT_SHAPE:
            nAction = ExecuteOrAcceptShapeDrop(
                DC_ACCEPT,
                rEvent.maPosPixel,
                &rEvent,
                rTargetHelper,
                pTargetWindow,
                nPage,
                nLayer);
            break;

        default:
        case DT_NONE:
            nAction = DND_ACTION_NONE;
            break;
    }

    return nAction;
}




sal_Int8 Clipboard::ExecuteDrop (
    const ExecuteDropEvent& rEvent,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    sal_Int8 nResult = DND_ACTION_NONE;
    mpUndoContext.reset();
    const Clipboard::DropType eDropType (IsDropAccepted(rTargetHelper));

    switch (eDropType)
    {
        case DT_PAGE:
        case DT_PAGE_FROM_NAVIGATOR:
        {
            SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
            const Point aEventModelPosition (
                pTargetWindow->PixelToLogic (rEvent.maPosPixel));
            const sal_Int32 nXOffset (labs (pDragTransferable->GetStartPos().X()
                - aEventModelPosition.X()));
            const sal_Int32 nYOffset (labs (pDragTransferable->GetStartPos().Y()
                - aEventModelPosition.Y()));
            bool bContinue =
                ( pDragTransferable->GetView() != &mrSlideSorter.GetView() )
                || ( nXOffset >= 2 && nYOffset >= 2 );

            ::boost::shared_ptr<InsertionIndicatorHandler> pInsertionIndicatorHandler(
                        mrController.GetInsertionIndicatorHandler());
            // Get insertion position and then turn off the insertion indicator.
            pInsertionIndicatorHandler->UpdatePosition(aEventModelPosition, rEvent.mnAction);
            //            sal_uInt16 nIndex = DetermineInsertPosition(*pDragTransferable);

            // Do not process the insertion when it is trivial,
            // i.e. would insert pages at their original place.
            if (IsInsertionTrivial(pDragTransferable, rEvent.mnAction))
                bContinue = false;

            // Tell the insertion indicator handler to hide before the model
            // is modified.  Doing it later may result in page objects whose
            // animation state is not properly reset because they are then
            // in another run then before the model change.
            pInsertionIndicatorHandler->End(Animator::AM_Immediate);

            if (bContinue)
            {
                SlideSorterController::ModelChangeLock aModelChangeLock (mrController);

                // Handle a general drop operation.
                mpUndoContext.reset(new UndoContext (
                    mrSlideSorter.GetModel().GetDocument(),
                    mrSlideSorter.GetViewShell()->GetViewShellBase().GetMainViewShell()));
                mpSelectionObserverContext.reset(new SelectionObserver::Context(mrSlideSorter));

                HandlePageDrop(*pDragTransferable);
                nResult = rEvent.mnAction;

                // We leave the undo context alive for when moving or
                // copying inside one view then the actions in
                // NotifyDragFinished should be covered as well as
                // well as the ones above.
            }

            // When the pages originated in another slide sorter then
            // only that is notified automatically about the drag
            // operation being finished.  Because the target slide sorter
            // has be notified, too, add a callback for that.
            ::boost::shared_ptr<TransferableData> pSlideSorterTransferable (
                TransferableData::GetFromTransferable(pDragTransferable));
            BOOST_ASSERT(pSlideSorterTransferable);
            if (pSlideSorterTransferable
                && pSlideSorterTransferable->GetSourceViewShell() != mrSlideSorter.GetViewShell())
            {
                DragFinished(nResult);
            }

            // Notify the receiving selection function that drag-and-drop is
            // finished and the substitution handler can be released.
            ::rtl::Reference<SelectionFunction> pFunction (
                mrController.GetCurrentSelectionFunction());
            if (pFunction.is())
                pFunction->NotifyDragFinished();
        }
        break;

        case DT_SHAPE:
            nResult = ExecuteOrAcceptShapeDrop(
                DC_EXECUTE,
                rEvent.maPosPixel,
                &rEvent,
                rTargetHelper,
                pTargetWindow,
                nPage,
                nLayer);
            break;

        default:
        case DT_NONE:
            break;
    }

    return nResult;
}




bool Clipboard::IsInsertionTrivial (
    SdTransferable* pTransferable,
    const sal_Int8 nDndAction) const
{
    ::boost::shared_ptr<TransferableData> pSlideSorterTransferable (
        TransferableData::GetFromTransferable(pTransferable));
    if (pSlideSorterTransferable
        && pSlideSorterTransferable->GetSourceViewShell() != mrSlideSorter.GetViewShell())
        return false;
    return mrController.GetInsertionIndicatorHandler()->IsInsertionTrivial(nDndAction);
}




void Clipboard::Abort (void)
{
    if (mpSelectionObserverContext)
    {
        mpSelectionObserverContext->Abort();
        mpSelectionObserverContext.reset();
    }
}




sal_uInt16 Clipboard::DetermineInsertPosition (const SdTransferable& )
{
    // Tell the model to move the dragged pages behind the one with the
    // index nInsertionIndex which first has to be transformed into an index
    // understandable by the document.
    const sal_Int32 nInsertionIndex (
        mrController.GetInsertionIndicatorHandler()->GetInsertionPageIndex());

    // Convert to insertion index to that of an SdModel.
    if (nInsertionIndex >= 0)
        return mrSlideSorter.GetModel().GetCoreIndex(nInsertionIndex);
    else
        return 0;
}




sal_uInt16 Clipboard::InsertSlides (
    const SdTransferable& rTransferable,
    sal_uInt16 nInsertPosition)
{
    sal_uInt16 nInsertedPageCount = ViewClipboard::InsertSlides (
        rTransferable,
        nInsertPosition);

    // Remember the inserted pages so that they can be selected when the
    // operation is finished.
    maPagesToSelect.clear();
    SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
    if (pDocument != NULL)
        for (sal_Int32 i=0; i<=nInsertedPageCount; i+=2)
            maPagesToSelect.push_back(
                dynamic_cast<SdPage*>(pDocument->GetPage(nInsertPosition+i)));

    mbUpdateSelectionPending |= (nInsertedPageCount>0);

    return nInsertedPageCount;
}




Clipboard::DropType Clipboard::IsDropAccepted (DropTargetHelper&) const
{
    const SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
    if (pDragTransferable == NULL)
        return DT_NONE;

    if (pDragTransferable->IsPageTransferable())
    {
        if (mrSlideSorter.GetModel().GetEditMode() != EM_MASTERPAGE)
            return DT_PAGE;
        else
            return DT_NONE;
    }

    const SdPageObjsTLB::SdPageObjsTransferable* pPageObjsTransferable
        = dynamic_cast<const SdPageObjsTLB::SdPageObjsTransferable*>(pDragTransferable);
    if (pPageObjsTransferable != NULL)
        return DT_PAGE_FROM_NAVIGATOR;

    return DT_SHAPE;
}




sal_Int8 Clipboard::ExecuteOrAcceptShapeDrop (
    DropCommand eCommand,
    const Point& rPosition,
    const void* pDropEvent,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    sal_Int8 nResult = 0;

    // The dropping of a shape is accepted or executed only when there is
    // DrawViewShell available to which we can forward this call.  This has
    // technical reasons:  The actual code to accept or execute a shape drop
    // is implemented in the ViewShell class and uses the page view of the
    // main edit view.  This is not possible without a DrawViewShell.
    ::boost::shared_ptr<DrawViewShell> pDrawViewShell;
    if (mrSlideSorter.GetViewShell() != NULL)
        pDrawViewShell = ::boost::dynamic_pointer_cast<DrawViewShell>(
            mrSlideSorter.GetViewShell()->GetViewShellBase().GetMainViewShell());
    if (pDrawViewShell.get() != NULL
        && (pDrawViewShell->GetShellType() == ViewShell::ST_IMPRESS
            || pDrawViewShell->GetShellType() == ViewShell::ST_DRAW))
    {
        // The drop is only accepted or executed when it takes place over a
        // page object.  Therefore we replace a missing page number by the
        // number of the page under the mouse.
        if (nPage == SDRPAGE_NOTFOUND)
        {
            model::SharedPageDescriptor pDescriptor (
                mrSlideSorter.GetModel().GetPageDescriptor(
                    mrSlideSorter.GetView().GetPageIndexAtPoint(rPosition)));
            if (pDescriptor)
                nPage = pDescriptor->GetPageIndex();
        }

        // Now comes the code that is different for the Execute and Accept:
        // We simply forward the call to the AcceptDrop() or ExecuteDrop()
        // methods of the DrawViewShell in the center pane.
        if (nPage != SDRPAGE_NOTFOUND)
            switch (eCommand)
            {
                case DC_ACCEPT:
                    nResult = pDrawViewShell->AcceptDrop(
                        *reinterpret_cast<const AcceptDropEvent*>(pDropEvent),
                        rTargetHelper,
                        pTargetWindow,
                        nPage,
                        nLayer);
                    break;

                case DC_EXECUTE:
                    nResult = pDrawViewShell->ExecuteDrop(
                        *reinterpret_cast<const ExecuteDropEvent*>(pDropEvent),
                        rTargetHelper,
                        pTargetWindow,
                        nPage,
                        nLayer);
                    break;
            }
    }

    return nResult;
}



} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
