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
} 

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
    rtl::Reference<FuPoor> xFunc;
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
    CreateSlideTransferable( pWindow, false );
}




void Clipboard::DoPaste (::Window* pWindow)
{
    SdTransferable* pClipTransferable = SD_MOD()->pTransferClip;

    if (pClipTransferable!=NULL && pClipTransferable->IsPageTransferable())
    {
        sal_Int32 nInsertPosition = GetInsertionPosition(pWindow);

        if (nInsertPosition >= 0)
        {
            
            sal_Int32 nInsertPageCount = PasteTransferable(nInsertPosition);
            
            
            mrSlideSorter.GetContentWindow()->GrabFocus();
            SelectPageRange(nInsertPosition, nInsertPageCount);
        }
    }
}




sal_Int32 Clipboard::GetInsertionPosition (::Window* pWindow)
{
    sal_Int32 nInsertPosition = -1;

    
    
    
    
    
    
    

    ::boost::shared_ptr<controller::InsertionIndicatorHandler> pInsertionIndicatorHandler (
        mrController.GetInsertionIndicatorHandler());
    if (pInsertionIndicatorHandler->IsActive())
    {
        
        nInsertPosition = pInsertionIndicatorHandler->GetInsertionPageIndex();
    }
    else if (mrController.GetSelectionManager()->GetInsertionPosition() >= 0)
    {
        
        
        nInsertPosition = mrController.GetSelectionManager()->GetInsertionPosition();
    }
    else if (mrController.GetFocusManager().IsFocusShowing())
    {
        
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
    
    
    PageSelector& rSelector (mrController.GetPageSelector());
    rSelector.DeselectAllPages();
    for (sal_uInt16 i=0; i<nPageCount; i++)
    {
        model::SharedPageDescriptor pDescriptor (
            mrSlideSorter.GetModel().GetPageDescriptor(nFirstIndex + i));
        if (pDescriptor.get() != NULL)
        {
            rSelector.SelectPage(pDescriptor);
            
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

    
    
    model::PageEnumeration aSelectedPages
        (model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mrSlideSorter.GetModel()));
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        aBookmarkList.push_back(pDescriptor->GetPage()->GetName());
        maPagesToRemove.push_back (pDescriptor->GetPage());
    }

    
    
    
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
            pTransferable->StartDrag (pActionWindow, DND_ACTION_COPY | DND_ACTION_MOVE);
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

        
        ::sd::ViewShell* pViewShell
              = SdPageObjsTLB::GetViewShellForDocShell(pTreeListBoxTransferable->GetDocShell());
        if (pViewShell == NULL)
            break;

        
        SlideSorterViewShell* pSlideSorterViewShell
            = SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
        if (pSlideSorterViewShell == NULL)
            break;
        SlideSorter& rSlideSorter (pSlideSorterViewShell->GetSlideSorter());

        
        TransferableDataHelper  aDataHelper (pTransferable);
        INetBookmark aINetBookmark;
        if ( ! aDataHelper.GetINetBookmark(SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark))
            break;
        const OUString sURL (aINetBookmark.GetURL());
        const sal_Int32 nIndex (sURL.indexOf((sal_Unicode)'#'));
        if (nIndex == -1)
            break;
        OUString sBookmark (sURL.copy(nIndex+1));

        
        SdDrawDocument* pTransferableDocument = rSlideSorter.GetModel().GetDocument();
        if (pTransferableDocument == NULL)
            break;
        sal_Bool bIsMasterPage = sal_False;
        const sal_uInt16 nPageIndex (pTransferableDocument->GetPageByName(sBookmark, bIsMasterPage));
        if (nPageIndex == SDRPAGE_NOTFOUND)
            break;

        
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

        
        
        Clipboard& rOtherClipboard (pSlideSorterViewShell->GetSlideSorter().GetController().GetClipboard());
        rOtherClipboard.maPagesToRemove.clear();
        rOtherClipboard.maPagesToRemove.push_back(pDescriptor->GetPage());

        
        ::boost::shared_ptr<SdTransferable::UserData> pNewTransferable (
            new TransferableData(
                pSlideSorterViewShell,
                aRepresentatives));
        pTransferable->SetWorkDocument( dynamic_cast<SdDrawDocument*>(
                pTreeListBoxTransferable->GetSourceDoc()->AllocModel()));
        

        
        std::vector<OUString> aPageBookmarks;
        aPageBookmarks.push_back(sBookmark);
        pTransferable->SetPageBookmarks(aPageBookmarks, false);

        
        
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
    CreateSlideTransferable(pWindow, true);

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

    
    ::rtl::Reference<SelectionFunction> pFunction (mrController.GetCurrentSelectionFunction());
    if (pFunction.is())
        pFunction->NotifyDragFinished();

    PageSelector& rSelector (mrController.GetPageSelector());
    if ((nDropAction & DND_ACTION_MOVE) != 0
        && ! maPagesToRemove.empty())
    {
        
        
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
            
            nAction = rEvent.mnAction;

            
            
            
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

            
            SelectionFunction* pSelectionFunction = dynamic_cast<SelectionFunction*>(
                mrSlideSorter.GetViewShell()->GetCurrentFunction().get());
            if (pSelectionFunction != NULL)
                pSelectionFunction->MouseDragged(rEvent, nAction);

            
            
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
            
            pInsertionIndicatorHandler->UpdatePosition(aEventModelPosition, rEvent.mnAction);
            

            
            
            if (IsInsertionTrivial(pDragTransferable, rEvent.mnAction))
                bContinue = false;

            
            
            
            
            pInsertionIndicatorHandler->End(Animator::AM_Immediate);

            if (bContinue)
            {
                SlideSorterController::ModelChangeLock aModelChangeLock (mrController);

                
                mpUndoContext.reset(new UndoContext (
                    mrSlideSorter.GetModel().GetDocument(),
                    mrSlideSorter.GetViewShell()->GetViewShellBase().GetMainViewShell()));
                mpSelectionObserverContext.reset(new SelectionObserver::Context(mrSlideSorter));

                HandlePageDrop(*pDragTransferable);
                nResult = rEvent.mnAction;

                
                
                
                
            }

            
            
            
            
            ::boost::shared_ptr<TransferableData> pSlideSorterTransferable (
                TransferableData::GetFromTransferable(pDragTransferable));
            BOOST_ASSERT(pSlideSorterTransferable);
            if (pSlideSorterTransferable
                && pSlideSorterTransferable->GetSourceViewShell() != mrSlideSorter.GetViewShell())
            {
                DragFinished(nResult);
            }

            
            
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
    
    
    
    const sal_Int32 nInsertionIndex (
        mrController.GetInsertionIndicatorHandler()->GetInsertionPageIndex());

    
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

    
    
    
    
    
    ::boost::shared_ptr<DrawViewShell> pDrawViewShell;
    if (mrSlideSorter.GetViewShell() != NULL)
        pDrawViewShell = ::boost::dynamic_pointer_cast<DrawViewShell>(
            mrSlideSorter.GetViewShell()->GetViewShellBase().GetMainViewShell());
    if (pDrawViewShell.get() != NULL
        && (pDrawViewShell->GetShellType() == ViewShell::ST_IMPRESS
            || pDrawViewShell->GetShellType() == ViewShell::ST_DRAW))
    {
        
        
        
        if (nPage == SDRPAGE_NOTFOUND)
        {
            model::SharedPageDescriptor pDescriptor (
                mrSlideSorter.GetModel().GetPageDescriptor(
                    mrSlideSorter.GetView().GetPageIndexAtPoint(rPosition)));
            if (pDescriptor)
                nPage = pDescriptor->GetPageIndex();
        }

        
        
        
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



} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
