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

#include <sal/config.h>

#include <cassert>

#include <controller/SlsClipboard.hxx>

#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <utility>
#include <view/SlideSorterView.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsInsertionIndicatorHandler.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsSelectionFunction.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <controller/SlsFocusManager.hxx>
#include <controller/SlsSelectionManager.hxx>
#include <controller/SlsTransferableData.hxx>
#include <controller/SlsSelectionObserver.hxx>
#include <controller/SlsVisibleAreaManager.hxx>
#include <cache/SlsPageCache.hxx>

#include <ViewShellBase.hxx>
#include <DrawViewShell.hxx>
#include <Window.hxx>
#include <fupoor.hxx>
#include <strings.hrc>
#include <sdresid.hxx>
#include <sdxfer.hxx>
#include <sdmod.hxx>
#include <ins_paste.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <sdfilter.hxx>
#include <sdpage.hxx>
#include <sdtreelb.hxx>
#include <app.hrc>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XStorage.hpp>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svxids.hrc>
#include <tools/urlobj.hxx>
#include <rtl/ustring.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/classids.hxx>
#include <comphelper/storagehelper.hxx>

using namespace ::com::sun::star;

namespace sd::slidesorter::controller {

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
    explicit TemporarySlideTrackingDeactivator (SlideSorterController& rController)
        : mrController(rController),
          mbIsCurrentSlideTrackingActive (
              mrController.GetVisibleAreaManager().IsCurrentSlideTrackingActive())
    {
        if (mbIsCurrentSlideTrackingActive)
            mrController.GetVisibleAreaManager().DeactivateCurrentSlideTracking();
    }
    ~TemporarySlideTrackingDeactivator()
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
        std::shared_ptr<ViewShell> pMainViewShell)
        : mpDocument(pDocument),
          mpMainViewShell(std::move(pMainViewShell))
    {
        if (mpDocument!=nullptr && mpDocument->IsUndoEnabled())
        {
            if (mpMainViewShell && mpMainViewShell->GetShellType() == ViewShell::ST_DRAW)
                mpDocument->BegUndo(SdResId(STRING_DRAG_AND_DROP_PAGES));
            else
                mpDocument->BegUndo(SdResId(STRING_DRAG_AND_DROP_SLIDES));
        }
    }

    ~UndoContext()
    {
        if (mpDocument!=nullptr && mpDocument->IsUndoEnabled())
            mpDocument->EndUndo();
        if (mpMainViewShell && mpMainViewShell->GetViewFrame()!=nullptr)
        {
            SfxBindings& rBindings = mpMainViewShell->GetViewFrame()->GetBindings();
            rBindings.Invalidate(SID_UNDO);
            rBindings.Invalidate(SID_REDO);
        }
    }
private:
    SdDrawDocument* mpDocument;
    std::shared_ptr<ViewShell> mpMainViewShell;
};

Clipboard::Clipboard (SlideSorter& rSlideSorter)
    : ViewClipboard(rSlideSorter.GetView()),
      mrSlideSorter(rSlideSorter),
      mrController(mrSlideSorter.GetController()),
      mnDragFinishedUserEventId(nullptr)
{
}

Clipboard::~Clipboard()
{
    if (mnDragFinishedUserEventId != nullptr)
        Application::RemoveUserEvent(mnDragFinishedUserEventId);
}

/** With the current implementation the forwarded calls to the current
    function will come back eventually to call the local Do(Cut|Copy|Paste)
    methods.  A shortcut is possible but would be an unclean hack.
*/
void Clipboard::HandleSlotCall (SfxRequest& rRequest)
{
    ViewShell& rViewShell = mrSlideSorter.GetViewShell();
    rtl::Reference<FuPoor> xFunc;
    xFunc = rViewShell.GetCurrentFunction();
    switch (rRequest.GetSlot())
    {
        case SID_CUT:
            if (mrSlideSorter.GetModel().GetEditMode() != EditMode::MasterPage)
            {
                if(xFunc.is())
                    xFunc->DoCut();
                else
                    DoCut();
            }
            rRequest.Done();
            break;

        case SID_COPY:
        case SID_COPY_SLIDE:
            if (mrSlideSorter.GetModel().GetEditMode() != EditMode::MasterPage)
            {
                if(xFunc.is())
                    xFunc->DoCopy();
                else
                    DoCopy();
            }
            else
            {
                // Copying master pages only.
                if(xFunc.is())
                    xFunc->DoCopy(true);
                else
                    DoCopy(true);
            }
            rRequest.Done();
            break;

        case SID_PASTE:
        case SID_PASTE_SLIDE:
            // Prevent redraws while inserting pages from the clipboard
            // because the intermediate inconsistent state might lead to
            // a crash.
            if (mrSlideSorter.GetModel().GetEditMode() != EditMode::MasterPage)
            {
                view::SlideSorterView::DrawLock aLock (mrSlideSorter);
                SelectionObserver::Context aContext (mrSlideSorter);
                if(xFunc.is())
                    xFunc->DoPaste();
                else
                    DoPaste();
            }
            else
            {
                // Pasting master pages only.
                view::SlideSorterView::DrawLock aLock (mrSlideSorter);
                SelectionObserver::Context aContext (mrSlideSorter);
                if(xFunc.is())
                    xFunc->DoPaste(true);
                else
                    DoPaste(true);
            }
            rRequest.Done();
            break;

        case SID_DELETE:
            DoDelete();
            rRequest.Done();
            break;
    }
}

void Clipboard::DoCut ()
{
    if (mrSlideSorter.GetModel().GetPageCount() > 1)
    {
        DoCopy();
        DoDelete();
    }
}

void Clipboard::DoDelete()
{
    if (mrSlideSorter.GetModel().GetPageCount() > 1)
    {
        mrController.GetSelectionManager()->DeleteSelectedPages();
    }
}

void Clipboard::DoCopy (bool bMergeMasterPagesOnly )
{
    CreateSlideTransferable( nullptr, false, bMergeMasterPagesOnly);
}

void Clipboard::DoPaste (bool bMergeMasterPagesOnly )
{
    SdTransferable* pClipTransferable = SdModule::get()->pTransferClip;

    if (pClipTransferable==nullptr || !pClipTransferable->IsPageTransferable())
        return;

    sal_Int32 nInsertPosition = GetInsertionPosition();

    if (nInsertPosition >= 0)
    {
        // Paste the pages from the clipboard.
        sal_Int32 nInsertPageCount = PasteTransferable(nInsertPosition, bMergeMasterPagesOnly);
        // Select the pasted pages and make the first of them the
        // current page.
        mrSlideSorter.GetContentWindow()->GrabFocus();
        SelectPageRange(nInsertPosition, nInsertPageCount);
    }
}

sal_Int32 Clipboard::GetInsertionPosition ()
{
    sal_Int32 nInsertPosition = -1;

    // Determine the insertion position:
    // a) When the insertion indicator is visible, then at that position.
    // b) When the focus indicator is visible, then before or after the
    // focused page, depending on user input to a dialog.
    // c) When there is a selection but no focus, then after the
    // selection.
    // d) After the last page when there is no selection and no focus.

    std::shared_ptr<controller::InsertionIndicatorHandler> pInsertionIndicatorHandler (
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
        vcl::Window* pWin = mrSlideSorter.GetContentWindow();
        SdInsertPasteDlg aDialog(pWin ? pWin->GetFrameWeld() : nullptr);
        if (aDialog.run() == RET_OK)
        {
            nInsertPosition = mrController.GetFocusManager().GetFocusedPageIndex();
            if (!aDialog.IsInsertBefore())
                nInsertPosition ++;
        }
    }

    return nInsertPosition;
}

sal_Int32 Clipboard::PasteTransferable (sal_Int32 nInsertPosition, bool bMergeMasterPagesOnly)
{
    SdTransferable* pClipTransferable = SdModule::get()->pTransferClip;
    model::SlideSorterModel& rModel (mrSlideSorter.GetModel());
    bool bMergeMasterPages = !pClipTransferable->HasSourceDoc (rModel.GetDocument());
    sal_uInt16 nInsertIndex (rModel.GetCoreIndex(nInsertPosition));
    sal_Int32 nInsertPageCount (0);
    if (pClipTransferable->HasPageBookmarks())
    {
        const std::vector<OUString> &rBookmarkList = pClipTransferable->GetPageBookmarks();
        const SolarMutexGuard aGuard;

        nInsertPageCount = static_cast<sal_uInt16>(rBookmarkList.size());
        rModel.GetDocument()->PasteBookmarkAsPage(
            rBookmarkList,
            nInsertIndex,
            pClipTransferable->GetPageDocShell(),
            bMergeMasterPages,
            bMergeMasterPagesOnly);
    }
    else
    {
        SfxObjectShell* pShell = pClipTransferable->GetDocShell().get();
        DrawDocShell* pDataDocSh = static_cast<DrawDocShell*>(pShell);
        SdDrawDocument* pDataDoc = pDataDocSh->GetDoc();

        if (pDataDoc!=nullptr
            && pDataDoc->GetSdPageCount(PageKind::Standard))
        {
            const SolarMutexGuard aGuard;

            bMergeMasterPages = (pDataDoc != rModel.GetDocument());
            nInsertPageCount = pDataDoc->GetSdPageCount( PageKind::Standard );
            rModel.GetDocument()->PasteBookmarkAsPage(
                std::vector<OUString>(),
                nInsertIndex,
                pDataDocSh,
                bMergeMasterPages,
                bMergeMasterPagesOnly);
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
    for (sal_Int32 i=0; i<nPageCount; i++)
    {
        model::SharedPageDescriptor pDescriptor (
            mrSlideSorter.GetModel().GetPageDescriptor(nFirstIndex + i));
        if (pDescriptor)
        {
            rSelector.SelectPage(pDescriptor);
            // The first page of the new selection is made the current page.
            if (i == 0)
            {
                mrController.GetCurrentSlideManager().SwitchCurrentSlide(pDescriptor);
            }
        }
    }
}

void Clipboard::CreateSlideTransferable (
    vcl::Window* pWindow,
    bool bDrag,
    bool bMergeMasterPagesOnly)
{
    std::vector<OUString> aBookmarkList;

    // Insert all selected pages into a bookmark list and remember them in
    // maPagesToRemove for possible later removal.
    model::PageEnumeration aSelectedPages
        (model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mrSlideSorter.GetModel()));
    SdDrawDocument* const pDocument = mrSlideSorter.GetModel().GetDocument();
    DrawDocShell* const pDataDocSh = pDocument->GetDocSh();

    sal_Int32 nUniqueID = 0;
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());

        //ensure that the slides have unique names
        const OUString sOrigName = pDescriptor->GetPage()->GetName();
        if ( pDataDocSh && !pDataDocSh->IsPageNameUnique( sOrigName ) )
        {
            OUString sUniqueName;
            bool bUnique = false;
            while ( !bUnique )
            {
                sUniqueName = sOrigName + "_clipboard" + OUString::number(nUniqueID++);
                bUnique = pDataDocSh->IsNewPageNameValid( sUniqueName );
                if ( bUnique )
                    pDescriptor->GetPage()->SetName(sUniqueName);
            }
        }

        aBookmarkList.push_back(pDescriptor->GetPage()->GetName());
        maPagesToRemove.push_back (pDescriptor->GetPage());
    }

    // Create a small set of representatives of the selection for which
    // previews are included into the transferable so that an insertion
    // indicator can be rendered.
    aSelectedPages.Rewind();
    ::std::vector<TransferableData::Representative> aRepresentatives;
    aRepresentatives.reserve(3);
    std::shared_ptr<cache::PageCache> pPreviewCache (
        mrSlideSorter.GetView().GetPreviewCache());
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        if ( ! pDescriptor || pDescriptor->GetPage()==nullptr)
            continue;
        BitmapEx aPreview (pPreviewCache->GetPreviewBitmap(pDescriptor->GetPage(), false));
        aRepresentatives.emplace_back(
            aPreview,
            pDescriptor->HasState(model::PageDescriptor::ST_Excluded));
        if (aRepresentatives.size() >= 3)
            break;
    }

    if (aBookmarkList.empty())
        return;

    mrSlideSorter.GetView().BrkAction();
    rtl::Reference<SdTransferable> pTransferable = TransferableData::CreateTransferable (
        pDocument,
        dynamic_cast<SlideSorterViewShell*>(&mrSlideSorter.GetViewShell()),
        std::move(aRepresentatives));

    if (bDrag)
        SdModule::get()->pTransferDrag = pTransferable.get();
    else
        SdModule::get()->pTransferClip = pTransferable.get();

    pDocument->CreatingDataObj (pTransferable.get());
    pTransferable->SetWorkDocument(pDocument->AllocSdDrawDocument());
    std::unique_ptr<TransferableObjectDescriptor> pObjDesc(new TransferableObjectDescriptor);
    pTransferable->GetWorkDocument()->GetDocSh()
        ->FillTransferableObjectDescriptor (*pObjDesc);

    // Makes it possible at paste site to determine that the origin is the slide sorter
    uno::Reference<document::XDocumentProperties> xDestination = pTransferable->GetWorkDocument()->GetDocSh()->getDocProperties();
    uno::Reference<beans::XPropertyContainer> xDestinationPropertyContainer = xDestination->getUserDefinedProperties();
    xDestinationPropertyContainer->addProperty("slidesorter", beans::PropertyAttribute::REMOVABLE, uno::Any(true));

    if (pDataDocSh != nullptr)
        pObjDesc->maDisplayName = pDataDocSh->GetMedium()->GetURLObject().GetURLNoPass();

    vcl::Window* pActionWindow = pWindow;
    if (pActionWindow == nullptr)
    {
        ViewShell& rViewShell = mrSlideSorter.GetViewShell();
        pActionWindow = rViewShell.GetActiveWindow();
    }

    assert(pActionWindow);

    pTransferable->SetStartPos (pActionWindow->PixelToLogic(
        pActionWindow->GetPointerPosPixel()));
    pTransferable->SetObjectDescriptor (std::move(pObjDesc));

    {
        TemporarySlideTrackingDeactivator aDeactivator (mrController);
        pTransferable->SetPageBookmarks (std::move(aBookmarkList), !bDrag, bMergeMasterPagesOnly);
    }

    if (bDrag)
    {
        pTransferable->SetView (&mrSlideSorter.GetView());
        pTransferable->StartDrag (pActionWindow, DND_ACTION_COPY | DND_ACTION_MOVE);
    }
    else
        pTransferable->CopyToClipboard (pActionWindow);

    pDocument->CreatingDataObj(nullptr);
}

std::shared_ptr<SdTransferable::UserData> Clipboard::CreateTransferableUserData (SdTransferable* pTransferable)
{
    do
    {
        SdPageObjsTLV::SdPageObjsTransferable* pTreeListBoxTransferable
            = dynamic_cast<SdPageObjsTLV::SdPageObjsTransferable*>(pTransferable);
        if (pTreeListBoxTransferable == nullptr)
            break;

        // Find view shell for the document of the transferable.
        ::sd::ViewShell* pViewShell
              = SdPageObjsTLV::GetViewShellForDocShell(pTreeListBoxTransferable->GetDocShell());
        if (pViewShell == nullptr)
            break;

        // Find slide sorter for the document of the transferable.
        SlideSorterViewShell* pSlideSorterViewShell
            = SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
        if (pSlideSorterViewShell == nullptr)
            break;
        SlideSorter& rSlideSorter (pSlideSorterViewShell->GetSlideSorter());

        // Get bookmark from transferable.
        TransferableDataHelper  aDataHelper (pTransferable);
        INetBookmark aINetBookmark;
        if ( ! aDataHelper.GetINetBookmark(SotClipboardFormatId::NETSCAPE_BOOKMARK, aINetBookmark))
            break;
        const OUString sURL (aINetBookmark.GetURL());
        const sal_Int32 nIndex (sURL.indexOf('#'));
        if (nIndex == -1)
            break;
        OUString sBookmark (sURL.copy(nIndex+1));

        // Make sure that the bookmark points to a page.
        SdDrawDocument* pTransferableDocument = rSlideSorter.GetModel().GetDocument();
        if (pTransferableDocument == nullptr)
            break;
        bool bIsMasterPage = false;
        const sal_uInt16 nPageIndex (pTransferableDocument->GetPageByName(sBookmark, bIsMasterPage));
        if (nPageIndex == SDRPAGE_NOTFOUND)
            break;

        // Create preview.
        ::std::vector<TransferableData::Representative> aRepresentatives;
        aRepresentatives.reserve(1);
        std::shared_ptr<cache::PageCache> pPreviewCache (
            rSlideSorter.GetView().GetPreviewCache());
        model::SharedPageDescriptor pDescriptor (rSlideSorter.GetModel().GetPageDescriptor((nPageIndex-1)/2));
        if ( ! pDescriptor || pDescriptor->GetPage()==nullptr)
            break;
        BitmapEx aPreview (pPreviewCache->GetPreviewBitmap(pDescriptor->GetPage(), false));
        aRepresentatives.emplace_back(
                aPreview,
                pDescriptor->HasState(model::PageDescriptor::ST_Excluded));

        // Remember the page in maPagesToRemove so that it can be removed
        // when drag and drop action is "move".
        Clipboard& rOtherClipboard (pSlideSorterViewShell->GetSlideSorter().GetController().GetClipboard());
        rOtherClipboard.maPagesToRemove.clear();
        rOtherClipboard.maPagesToRemove.push_back(pDescriptor->GetPage());

        // Create the new transferable.
        std::shared_ptr<SdTransferable::UserData> pNewTransferable =
            std::make_shared<TransferableData>(
                pSlideSorterViewShell,
                std::move(aRepresentatives));
        pTransferable->SetWorkDocument(pTreeListBoxTransferable->GetSourceDoc()->AllocSdDrawDocument());
        //        pTransferable->SetView(&mrSlideSorter.GetView());

        // Set page bookmark list.
        std::vector<OUString> aPageBookmarks { sBookmark };
        pTransferable->SetPageBookmarks(std::move(aPageBookmarks), false);

        // Replace the view referenced by the transferable with the
        // corresponding slide sorter view.
        pTransferable->SetView(&pSlideSorterViewShell->GetSlideSorter().GetView());

        return pNewTransferable;
    }
    while (false);

    return std::shared_ptr<SdTransferable::UserData>();
}

void Clipboard::StartDrag (
    const Point& rPosition,
    vcl::Window* pWindow)
{
    maPagesToRemove.clear();
    CreateSlideTransferable(pWindow, true);

    mrController.GetInsertionIndicatorHandler()->UpdatePosition(
        rPosition,
        InsertionIndicatorHandler::UnknownMode);
}

void Clipboard::DragFinished (sal_Int8 nDropAction)
{
    if (mnDragFinishedUserEventId == nullptr)
    {
        mnDragFinishedUserEventId = Application::PostUserEvent(
            LINK(this, Clipboard, ProcessDragFinished),
            reinterpret_cast<void*>(nDropAction));
    }
}

IMPL_LINK(Clipboard, ProcessDragFinished, void*, pUserData, void)
{
    const sal_Int8 nDropAction (static_cast<sal_Int8>(reinterpret_cast<sal_IntPtr>(pUserData)));

    mnDragFinishedUserEventId = nullptr;

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
        for (const auto& rpDraggedPage : maPagesToRemove)
        {
            rSelector.SelectPage(rpDraggedPage);
        }
        mrController.GetSelectionManager()->DeleteSelectedPages();
    }
    mxUndoContext.reset();
    mxSelectionObserverContext.reset();
}

sal_Int8 Clipboard::AcceptDrop (
    const AcceptDropEvent& rEvent,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    SdrLayerID nLayer)
{
    sal_Int8 nAction (DND_ACTION_NONE);

    const Clipboard::DropType eDropType (IsDropAccepted());

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
            SdTransferable* pDragTransferable = SdModule::get()->pTransferDrag;
            if (pDragTransferable != nullptr
                && pDragTransferable->IsPageTransferable()
                && ((rEvent.maDragEvent.DropAction
                        & css::datatransfer::dnd::DNDConstants::ACTION_DEFAULT) != 0)
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
                mrSlideSorter.GetViewShell().GetCurrentFunction().get());
            if (pSelectionFunction != nullptr)
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
    SdrLayerID nLayer)
{
    sal_Int8 nResult = DND_ACTION_NONE;
    mxUndoContext.reset();
    const Clipboard::DropType eDropType (IsDropAccepted());

    switch (eDropType)
    {
        case DT_PAGE:
        case DT_PAGE_FROM_NAVIGATOR:
        {
            SdTransferable* pDragTransferable = SdModule::get()->pTransferDrag;
            const Point aEventModelPosition (
                pTargetWindow->PixelToLogic (rEvent.maPosPixel));
            const sal_Int32 nXOffset (std::abs (pDragTransferable->GetStartPos().X()
                - aEventModelPosition.X()));
            const sal_Int32 nYOffset (std::abs (pDragTransferable->GetStartPos().Y()
                - aEventModelPosition.Y()));
            bool bContinue =
                ( pDragTransferable->GetView() != &mrSlideSorter.GetView() )
                || ( nXOffset >= 2 && nYOffset >= 2 );

            std::shared_ptr<InsertionIndicatorHandler> pInsertionIndicatorHandler(
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
                mxUndoContext.reset(new UndoContext (
                    mrSlideSorter.GetModel().GetDocument(),
                    mrSlideSorter.GetViewShell().GetViewShellBase().GetMainViewShell()));
                mxSelectionObserverContext.reset(new SelectionObserver::Context(mrSlideSorter));

                if (rEvent.mnAction == DND_ACTION_MOVE)
                {
                    SdDrawDocument* pDoc = mrSlideSorter.GetModel().GetDocument();
                    const bool bDoesMakePageObjectsNamesUnique = pDoc->DoesMakePageObjectsNamesUnique();
                    pDoc->DoMakePageObjectsNamesUnique(false);
                    HandlePageDrop(*pDragTransferable);
                    pDoc->DoMakePageObjectsNamesUnique(bDoesMakePageObjectsNamesUnique);
                }
                else
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
            std::shared_ptr<TransferableData> pSlideSorterTransferable (
                TransferableData::GetFromTransferable(pDragTransferable));
            assert(pSlideSorterTransferable);
            if (pSlideSorterTransferable
                && pSlideSorterTransferable->GetSourceViewShell() != &mrSlideSorter.GetViewShell())
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
    SdTransferable const * pTransferable,
    const sal_Int8 nDndAction) const
{
    std::shared_ptr<TransferableData> pSlideSorterTransferable (
        TransferableData::GetFromTransferable(pTransferable));
    if (pSlideSorterTransferable
        && pSlideSorterTransferable->GetSourceViewShell() != &mrSlideSorter.GetViewShell())
        return false;
    return mrController.GetInsertionIndicatorHandler()->IsInsertionTrivial(nDndAction);
}

void Clipboard::Abort()
{
    if (mxSelectionObserverContext)
    {
        mxSelectionObserverContext->Abort();
        mxSelectionObserverContext.reset();
    }
}

sal_uInt16 Clipboard::DetermineInsertPosition ()
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

Clipboard::DropType Clipboard::IsDropAccepted() const
{
    const SdTransferable* pDragTransferable = SdModule::get()->pTransferDrag;
    if (pDragTransferable == nullptr)
        return DT_NONE;

    if (pDragTransferable->IsPageTransferable())
    {
        if (mrSlideSorter.GetModel().GetEditMode() != EditMode::MasterPage)
            return DT_PAGE;
        else
            return DT_NONE;
    }

    const SdPageObjsTLV::SdPageObjsTransferable* pPageObjsTransferable
        = dynamic_cast<const SdPageObjsTLV::SdPageObjsTransferable*>(pDragTransferable);
    if (pPageObjsTransferable != nullptr)
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
    SdrLayerID nLayer)
{
    sal_Int8 nResult = 0;

    // The dropping of a shape is accepted or executed only when there is
    // DrawViewShell available to which we can forward this call.  This has
    // technical reasons:  The actual code to accept or execute a shape drop
    // is implemented in the ViewShell class and uses the page view of the
    // main edit view.  This is not possible without a DrawViewShell.
    std::shared_ptr<DrawViewShell> pDrawViewShell;
    pDrawViewShell = std::dynamic_pointer_cast<DrawViewShell>(
        mrSlideSorter.GetViewShell().GetViewShellBase().GetMainViewShell());
    if (pDrawViewShell != nullptr
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
                        *static_cast<const AcceptDropEvent*>(pDropEvent),
                        rTargetHelper,
                        pTargetWindow,
                        nPage,
                        nLayer);
                    break;

                case DC_EXECUTE:
                    nResult = pDrawViewShell->ExecuteDrop(
                        *static_cast<const ExecuteDropEvent*>(pDropEvent),
                        rTargetHelper,
                        pTargetWindow,
                        nPage,
                        nLayer);
                    break;
            }
    }

    return nResult;
}

bool Clipboard::PasteSlidesFromSystemClipboard()
{
    ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
    std::shared_ptr<DrawViewShell> pDrawViewShell(
        std::dynamic_pointer_cast<DrawViewShell>(pBase->GetMainViewShell()));
    if (!pDrawViewShell)
        return false;
    TransferableDataHelper aDataHelper(
        TransferableDataHelper::CreateFromSystemClipboard(pDrawViewShell->GetActiveWindow()));

    {
        // Only attempt to load EMBED_SOURCE, if its descriptor is correct
        if (!aDataHelper.HasFormat(SotClipboardFormatId::OBJECTDESCRIPTOR))
            return false;

        TransferableObjectDescriptor aObjDesc;
        if (!aDataHelper.GetTransferableObjectDescriptor(SotClipboardFormatId::OBJECTDESCRIPTOR,
                                                         aObjDesc))
            return false;

        if (aObjDesc.maClassName != SvGlobalName(SO3_SIMPRESS_CLASSID))
            return false;
    }

    SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
    assert(pDocument);
    OUString aDocShellID = SfxObjectShell::CreateShellID(pDocument->GetDocSh());
    auto xStm = aDataHelper.GetInputStream(SotClipboardFormatId::EMBED_SOURCE, aDocShellID);

    if (xStm.is())
    {
        uno::Reference<embed::XStorage> xStore(
            ::comphelper::OStorageHelper::GetStorageFromInputStream(xStm));
        ::sd::DrawDocShellRef xDocShRef(new ::sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, true,
                                                               pDocument->GetDocumentType()));
        SfxMedium* pMedium = new SfxMedium(xStore, OUString());
        xDocShRef->DoLoad(pMedium);

        // Only accept pastes that originated in a slide sorter here, so we
        // don't create a new page for other types of pastes
        if (!IsSlideSorterPaste(*xDocShRef))
            return false;

        std::vector<OUString> aBookmarkList;
        std::vector<OUString> aExchangeList;

        auto insertPos = mrSlideSorter.GetModel().GetCoreIndex(
            mrSlideSorter.GetController().GetClipboard().GetInsertionPosition());
        pDocument->InsertFileAsPage(aBookmarkList, &aExchangeList, false /*bLink*/,
                                    insertPos /*nPos*/, xDocShRef.get());

        std::vector<OUString> aObjectBookmarkList;
        pDocument->InsertBookmarkAsObject(aObjectBookmarkList, aExchangeList, xDocShRef.get(),
                                          nullptr, false);

        return true;
    }
    return false;
}

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
