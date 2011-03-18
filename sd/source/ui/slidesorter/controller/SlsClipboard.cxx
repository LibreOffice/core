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
#include "controller/SlsTransferable.hxx"
#include "controller/SlsSelectionObserver.hxx"
#include "cache/SlsPageCache.hxx"

#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"
#include "fupoor.hxx"
#include "fuslhide.hxx"
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
#include "sddll.hxx"
#include "ins_paste.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"

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

namespace sd { namespace slidesorter { namespace controller {

class Clipboard::UndoContext
{
public:
    UndoContext (
        SdDrawDocument* pDocument,
        const ::boost::shared_ptr<ViewShell>& rpMainViewShell,
        const ::boost::shared_ptr<view::Theme>& rpTheme)
        : mpDocument(pDocument),
          mpMainViewShell(rpMainViewShell)
    {
        if (mpDocument!=NULL && mpDocument->IsUndoEnabled())
        {
            if (mpMainViewShell && mpMainViewShell->GetShellType() == ViewShell::ST_DRAW)
                mpDocument->BegUndo(rpTheme->GetString(view::Theme::String_DragAndDropPages));
            else
                mpDocument->BegUndo(rpTheme->GetString(view::Theme::String_DragAndDropSlides));
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
        const List& rBookmarkList = pClipTransferable->GetPageBookmarks();
        const SolarMutexGuard aGuard;

        nInsertPageCount = (sal_uInt16) rBookmarkList.Count();
        rModel.GetDocument()->InsertBookmarkAsPage(
            const_cast<List*>(&rBookmarkList),
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
                NULL,
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
    List aBookmarkList;

    // Insert all selected pages into a bookmark list and remember them in
    // maPagesToRemove for possible later removal.
    model::PageEnumeration aSelectedPages
        (model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mrSlideSorter.GetModel()));
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        aBookmarkList.Insert (
            new String(pDescriptor->GetPage()->GetName()),
            LIST_APPEND);
        maPagesToRemove.push_back (pDescriptor->GetPage());
    }

    // Create a small set of representatives of the selection for which
    // previews are included into the transferable so that an insertion
    // indicator can be rendered.
    aSelectedPages.Rewind();
    ::std::vector<Transferable::Representative> aRepresentatives;
    aRepresentatives.reserve(3);
    ::boost::shared_ptr<cache::PageCache> pPreviewCache (
        mrSlideSorter.GetView().GetPreviewCache());
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        if ( ! pDescriptor || pDescriptor->GetPage()==NULL)
            continue;
        Bitmap aPreview (pPreviewCache->GetPreviewBitmap(pDescriptor->GetPage(), false));
        aRepresentatives.push_back(Transferable::Representative(
            aPreview,
            pDescriptor->HasState(model::PageDescriptor::ST_Excluded)));
        if (aRepresentatives.size() >= 3)
            break;
    }

    if (aBookmarkList.Count() > 0)
    {
        mrSlideSorter.GetView().BrkAction();
        SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
        SdTransferable* pTransferable = new Transferable (
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
        pTransferable->SetPageBookmarks (aBookmarkList, !bDrag);

        for (void* p=aBookmarkList.First(); p!=NULL; p=aBookmarkList.Next())
            delete static_cast<String*>(p);

        if (bDrag)
        {
            pTransferable->SetView (&mrSlideSorter.GetView());
            sal_Int8 nDragSourceActions (DND_ACTION_COPY);
            // The move action is available only when not all pages would be
            // moved.  Otherwise an empty document would remain.  Crash.
            sal_Int32 nRemainingPages = mrSlideSorter.GetModel().GetPageCount() - aBookmarkList.Count();
            if (nRemainingPages > 0)
                nDragSourceActions |= DND_ACTION_MOVE;
            pTransferable->StartDrag (pActionWindow, nDragSourceActions);
        }
        else
            pTransferable->CopyToClipboard (pActionWindow);
    }
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
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
    if (pDragTransferable != NULL)
        pDragTransferable->SetView (NULL);

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




void Clipboard::SelectPages (void)
{
    PageSelector& rSelector (mrController.GetPageSelector());

    // Select the dropped pages.
    PageList::iterator iPage;
    rSelector.DeselectAllPages();
    for (iPage=maPagesToSelect.begin(); iPage!=maPagesToSelect.end(); ++iPage)
    {
        rSelector.SelectPage(*iPage);
    }
}




sal_Int8 Clipboard::AcceptDrop (
    const AcceptDropEvent& rEvent,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    sal_Int8 nAction (DND_ACTION_NONE);

    const Clipboard::DropType eDropType (IsDropAccepted());

    switch (eDropType)
    {
        case DT_PAGE:
        {
            // Accept a drop.
            nAction = rEvent.mnAction;

            // Use the copy action when the drop action is the default, i.e. not
            // explicitly set to move or link, and when the source and
            // target models are not the same.
            const SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
            if (pDragTransferable != NULL
                && pDragTransferable->IsPageTransferable()
                && ((rEvent.maDragEvent.DropAction
                        & ::com::sun::star::datatransfer::dnd::DNDConstants::ACTION_DEFAULT) != 0)
                && (mrSlideSorter.GetModel().GetDocument()->GetDocSh()
                    != pDragTransferable->GetPageDocShell()))
            {
                nAction = DND_ACTION_COPY;
            }
            else if (mrController.GetInsertionIndicatorHandler()->IsInsertionTrivial(nAction))
            {
                nAction = DND_ACTION_NONE;
            }

            // Show the insertion marker and the substitution for a drop.
            Point aPosition = pTargetWindow->PixelToLogic (rEvent.maPosPixel);
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

    switch (IsDropAccepted())
    {
        case DT_PAGE:
        {
            const SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
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
            if (pInsertionIndicatorHandler->IsInsertionTrivial(rEvent.mnAction))
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
                    mrSlideSorter.GetViewShell()->GetViewShellBase().GetMainViewShell(),
                    mrSlideSorter.GetTheme()));
                mpSelectionObserverContext.reset(new SelectionObserver::Context(mrSlideSorter));

                HandlePageDrop(*pDragTransferable);
                nResult = rEvent.mnAction;

                // We leave the undo context alive for when moving or
                // copying inside one view then the actions in
                // NotifyDragFinished should be covered as well as
                // well as the ones above.
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
            break;
    }

    return nResult;
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




Clipboard::DropType Clipboard::IsDropAccepted (void) const
{
    DropType eResult (DT_NONE);

    const SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
    if (pDragTransferable != NULL)
    {
        if (pDragTransferable->IsPageTransferable())
        {
            if (mrSlideSorter.GetModel().GetEditMode() != EM_MASTERPAGE)
                eResult = DT_PAGE;
        }
        else
        {
            eResult = DT_SHAPE;
        }
    }

    return eResult;
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
