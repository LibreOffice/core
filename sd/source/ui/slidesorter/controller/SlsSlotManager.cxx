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


#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <editeng/outlobj.hxx>

#include "controller/SlsSlotManager.hxx"
#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsSelectionObserver.hxx"
#include "SlsCommand.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "framework/FrameworkHelper.hxx"
#include "Window.hxx"
#include "fupoor.hxx"
#include "fuzoom.hxx"
#include "fucushow.hxx"
#include "fusldlg.hxx"
#include "fuexpand.hxx"
#include "fusumry.hxx"
#include "fuscale.hxx"
#include "slideshow.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellImplementation.hxx"
#include "sdattr.hxx"
#include "FrameView.hxx"
#include "zoomlist.hxx"
#include "sdpage.hxx"
#include "sdxfer.hxx"
#include "helpids.h"
#include "glob.hrc"
#include "unmodpg.hxx"
#include "DrawViewShell.hxx"
#include "sdabstdlg.hxx"

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svx/svxids.hrc>
#include <sfx2/zoomitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svl/aeitem.hxx>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <vcl/svapp.hxx>

#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::beans;

namespace sd { namespace slidesorter { namespace controller {

namespace {

/** The state of a set of slides with respect to being excluded from the
    slide show.
*/
enum SlideExclusionState {UNDEFINED, EXCLUDED, INCLUDED, MIXED};

/** Return for the given set of slides whether they included are
    excluded from the slide show.
*/
SlideExclusionState GetSlideExclusionState (model::PageEnumeration& rPageSet);

} 



SlotManager::SlotManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maCommandQueue()
{
}




SlotManager::~SlotManager (void)
{
}




void SlotManager::FuTemporary (SfxRequest& rRequest)
{
    SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();

    SlideSorterViewShell* pShell
        = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
    if (pShell == NULL)
        return;

    switch (rRequest.GetSlot())
    {
        case SID_PRESENTATION:
        case SID_PRESENTATION_CURRENT_SLIDE:
        case SID_REHEARSE_TIMINGS:
            ShowSlideShow (rRequest);
            pShell->Cancel();
            rRequest.Done();
            break;

        case SID_HIDE_SLIDE:
            ChangeSlideExclusionState(model::SharedPageDescriptor(), true);
            break;

        case SID_SHOW_SLIDE:
            ChangeSlideExclusionState(model::SharedPageDescriptor(), false);
            break;

        case SID_PAGES_PER_ROW:
            if (rRequest.GetArgs() != NULL)
            {
                SFX_REQUEST_ARG(rRequest, pPagesPerRow, SfxUInt16Item,
                    SID_PAGES_PER_ROW, false);
                if (pPagesPerRow != NULL)
                {
                    sal_Int32 nColumnCount = pPagesPerRow->GetValue();
                    
                    
                    
                    mrSlideSorter.GetView().GetLayouter().SetColumnCount (
                        nColumnCount, nColumnCount);
                    
                    pShell->ArrangeGUIElements ();
                    
                    
                    
                    mrSlideSorter.GetController().Rearrange(true);
                }
            }
            rRequest.Done();
            break;

        case SID_SELECTALL:
            mrSlideSorter.GetController().GetPageSelector().SelectAllPages();
            rRequest.Done();
            break;

        case SID_SLIDE_TRANSITIONS_PANEL:
        {
            
            ::sfx2::sidebar::Sidebar::ShowPanel(
                OUString("SlideTransitionPanel"),
                pShell->GetViewFrame()->GetFrame().GetFrameInterface());
            rRequest.Ignore ();
            break;
        }

        case SID_PRESENTATION_DLG:
            FuSlideShowDlg::Create (
                pShell,
                mrSlideSorter.GetContentWindow().get(),
                &mrSlideSorter.GetView(),
                pDocument,
                rRequest);
            break;

        case SID_CUSTOMSHOW_DLG:
            FuCustomShowDlg::Create (
                pShell,
                mrSlideSorter.GetContentWindow().get(),
                &mrSlideSorter.GetView(),
                pDocument,
                rRequest);
                break;

        case SID_EXPAND_PAGE:
            FuExpandPage::Create (
                pShell,
                mrSlideSorter.GetContentWindow().get(),
                &mrSlideSorter.GetView(),
                pDocument,
                rRequest);
            break;

        case SID_SUMMARY_PAGE:
            FuSummaryPage::Create (
                pShell,
                mrSlideSorter.GetContentWindow().get(),
                &mrSlideSorter.GetView(),
                pDocument,
                rRequest);
            break;

        case SID_INSERTPAGE:
        case SID_INSERT_MASTER_PAGE:
            InsertSlide(rRequest);
            rRequest.Done();
            break;

        case SID_DUPLICATE_PAGE:
            DuplicateSelectedSlides(rRequest);
            rRequest.Done();
            break;

        case SID_DELETE_PAGE:
        case SID_DELETE_MASTER_PAGE:
        case SID_DELETE: 
            
            if (mrSlideSorter.GetModel().GetPageCount() > 1)
            {
                mrSlideSorter.GetController().GetSelectionManager()->DeleteSelectedPages();
            }

            rRequest.Done();
            break;

        case SID_RENAMEPAGE:
        case SID_RENAME_MASTER_PAGE:
            RenameSlide ();
            rRequest.Done ();
            break;

        case SID_ASSIGN_LAYOUT:
        {
            pShell->mpImpl->AssignLayout( rRequest, mrSlideSorter.GetModel().GetPageType() );
            rRequest.Done ();
        }
        break;

        case SID_PHOTOALBUM:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            VclAbstractDialog* pDlg = pFact ? pFact->CreateSdPhotoAlbumDialog(
                mrSlideSorter.GetContentWindow().get(),
                pDocument) : 0;

            if (pDlg)
            {
                pDlg->Execute();
                delete pDlg;
            }
            rRequest.Done ();
        }
        break;

        default:
            break;
    }
}




void SlotManager::FuPermanent (SfxRequest& rRequest)
{
    ViewShell* pShell = mrSlideSorter.GetViewShell();
    if (pShell == NULL)
        return;

    if(pShell->GetCurrentFunction().is())
    {
        rtl::Reference<FuPoor> xEmpty;
        if (pShell->GetOldFunction() == pShell->GetCurrentFunction())
            pShell->SetOldFunction(xEmpty);

        pShell->GetCurrentFunction()->Deactivate();
        pShell->SetCurrentFunction(xEmpty);
    }

    switch(rRequest.GetSlot())
    {
        case SID_OBJECT_SELECT:
            pShell->SetCurrentFunction( SelectionFunction::Create(mrSlideSorter, rRequest) );
            rRequest.Done();
            break;

        default:
                break;
    }

    if(pShell->GetOldFunction().is())
    {
        pShell->GetOldFunction()->Deactivate();
        rtl::Reference<FuPoor> xEmpty;
        pShell->SetOldFunction(xEmpty);
    }

    if(pShell->GetCurrentFunction().is())
    {
        pShell->GetCurrentFunction()->Activate();
        pShell->SetOldFunction(pShell->GetCurrentFunction());
    }

    
    
}

void SlotManager::FuSupport (SfxRequest& rRequest)
{
    switch (rRequest.GetSlot())
    {
        case SID_STYLE_FAMILY:
            if (rRequest.GetArgs() != NULL)
            {
                SdDrawDocument* pDocument
                    = mrSlideSorter.GetModel().GetDocument();
                if (pDocument != NULL)
                {
                    const SfxPoolItem& rItem (
                        rRequest.GetArgs()->Get(SID_STYLE_FAMILY));
                    pDocument->GetDocSh()->SetStyleFamily(
                        static_cast<const SfxUInt16Item&>(rItem).GetValue());
                }
            }
            break;

        case SID_PASTE:
        {
            SdTransferable* pTransferClip = SD_MOD()->pTransferClip;
            if( pTransferClip )
            {
                SfxObjectShell* pTransferDocShell = pTransferClip->GetDocShell();

                DrawDocShell* pDocShell = dynamic_cast<DrawDocShell*>(pTransferDocShell);
                if (pDocShell && pDocShell->GetDoc()->GetPageCount() > 1)
                {
                    mrSlideSorter.GetController().GetClipboard().HandleSlotCall(rRequest);
                    break;
                }
            }
            ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
            if (pBase != NULL)
            {
                ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
                    ::boost::dynamic_pointer_cast<DrawViewShell>(pBase->GetMainViewShell()));
                if (pDrawViewShell.get() != NULL)
                    pDrawViewShell->FuSupport(rRequest);
            }
        }
        break;

        case SID_CUT:
        case SID_COPY:
        case SID_DELETE:
            mrSlideSorter.GetController().GetClipboard().HandleSlotCall(rRequest);
            break;

        case SID_DRAWINGMODE:
        case SID_NOTESMODE:
        case SID_HANDOUTMODE:
        case SID_DIAMODE:
        case SID_OUTLINEMODE:
        {
            ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
            if (pBase != NULL)
            {
                framework::FrameworkHelper::Instance(*pBase)->HandleModeChangeSlot(
                    rRequest.GetSlot(), rRequest);
                rRequest.Done();
            }
            break;
        }

        case SID_UNDO:
        {
            SlideSorterViewShell* pViewShell
                = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
            if (pViewShell != NULL)
            {
                view::SlideSorterView::DrawLock aDrawLock (mrSlideSorter);
                SlideSorterController::ModelChangeLock aModelLock (mrSlideSorter.GetController());
                PageSelector::UpdateLock aUpdateLock (mrSlideSorter);
                SelectionObserver::Context aContext (mrSlideSorter);
                pViewShell->ImpSidUndo (sal_False, rRequest);
            }
            break;
        }

        case SID_REDO:
        {
            SlideSorterViewShell* pViewShell
                = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
            if (pViewShell != NULL)
            {
                view::SlideSorterView::DrawLock aDrawLock (mrSlideSorter);
                SlideSorterController::ModelChangeLock aModelLock (mrSlideSorter.GetController());
                PageSelector::UpdateLock aUpdateLock (mrSlideSorter);
                SelectionObserver::Context aContext (mrSlideSorter);
                pViewShell->ImpSidRedo (sal_False, rRequest);
            }
            break;
        }

        default:
            break;
    }
}




void SlotManager::ExecCtrl (SfxRequest& rRequest)
{
    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    sal_uInt16 nSlot = rRequest.GetSlot();
    switch (nSlot)
    {
        case SID_RELOAD:
        {
            
            mrSlideSorter.GetModel().GetDocument()->GetDocSh()->ClearUndoBuffer();

            
            if (pViewShell != NULL)
                pViewShell->GetViewFrame()->ExecuteSlot(rRequest);

            
            return;
        }

        case SID_OUTPUT_QUALITY_COLOR:
        case SID_OUTPUT_QUALITY_GRAYSCALE:
        case SID_OUTPUT_QUALITY_BLACKWHITE:
        case SID_OUTPUT_QUALITY_CONTRAST:
        {
            
            if (pViewShell != NULL)
                pViewShell->ExecReq (rRequest);
            break;
        }

        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            if (pViewShell != NULL)
                pViewShell->ExecReq (rRequest);
            break;
        }

        case SID_OPT_LOCALE_CHANGED:
        {
            mrSlideSorter.GetController().UpdateAllPages();
            if (pViewShell != NULL)
                pViewShell->UpdatePreview (pViewShell->GetActualPage());
            rRequest.Done();
            break;
        }

        case SID_SEARCH_DLG:
            
            
            
            
            
            
            if (pViewShell != NULL)
                pViewShell->GetViewFrame()->ExecuteSlot(rRequest);
            break;

        default:
            break;
    }
}




void SlotManager::GetAttrState (SfxItemSet& rSet)
{
    
    SfxWhichIter aIter (rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        sal_uInt16 nSlotId (nWhich);
        if (SfxItemPool::IsWhich(nWhich) && mrSlideSorter.GetViewShell()!=NULL)
            nSlotId = mrSlideSorter.GetViewShell()->GetPool().GetSlotId(nWhich);
        switch (nSlotId)
        {
            case SID_PAGES_PER_ROW:
                rSet.Put (
                    SfxUInt16Item (
                        nSlotId,
                        (sal_uInt16)mrSlideSorter.GetView().GetLayouter().GetColumnCount()
                        )
                    );
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void SlotManager::GetMenuState (SfxItemSet& rSet)
{
    EditMode eEditMode = mrSlideSorter.GetModel().GetEditMode();
    ViewShell* pShell = mrSlideSorter.GetViewShell();
    DrawDocShell* pDocShell = mrSlideSorter.GetModel().GetDocument()->GetDocSh();

    if (pShell!=NULL && pShell->GetCurrentFunction().is())
    {
        sal_uInt16 nSId = pShell->GetCurrentFunction()->GetSlotID();

        rSet.Put( SfxBoolItem( nSId, true ) );
    }
    rSet.Put( SfxBoolItem( SID_DRAWINGMODE, false ) );
    rSet.Put( SfxBoolItem( SID_DIAMODE, true ) );
    rSet.Put( SfxBoolItem( SID_OUTLINEMODE, false ) );
    rSet.Put( SfxBoolItem( SID_NOTESMODE, false ) );
    rSet.Put( SfxBoolItem( SID_HANDOUTMODE, false ) );

    if (pShell!=NULL && pShell->IsMainViewShell())
    {
        rSet.DisableItem(SID_SPELL_DIALOG);
        rSet.DisableItem(SID_SEARCH_DLG);
    }

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_EXPAND_PAGE))
    {
        bool bDisable = true;
        if (eEditMode == EM_PAGE)
        {
            
            
            
            model::PageEnumeration aSelectedPages (
                model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                    mrSlideSorter.GetModel()));
            while (aSelectedPages.HasMoreElements())
            {
                SdPage* pPage = aSelectedPages.GetNextElement()->GetPage();
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_OUTLINE);
                if (pObj!=NULL )
                {
                    if( !pObj->IsEmptyPresObj() )
                    {
                        bDisable = false;
                    }
                    else
                    {
                        
                        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                        if( pTextObj )
                        {
                            OutlinerParaObject* pParaObj = pTextObj->GetEditOutlinerParaObject();
                            if( pParaObj )
                            {
                                delete pParaObj;
                                bDisable = false;
                            }
                        }
                    }
                }
            }
        }

        if (bDisable)
            rSet.DisableItem (SID_EXPAND_PAGE);
    }

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_SUMMARY_PAGE))
    {
        bool bDisable = true;
        if (eEditMode == EM_PAGE)
        {
            
            
            
            model::PageEnumeration aSelectedPages (
                model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                    mrSlideSorter.GetModel()));
            while (aSelectedPages.HasMoreElements())
            {
                SdPage* pPage = aSelectedPages.GetNextElement()->GetPage();
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_TITLE);

                if (pObj!=NULL && !pObj->IsEmptyPresObj())
                    bDisable = false;
            }
        }
        if (bDisable)
            rSet.DisableItem (SID_SUMMARY_PAGE);
    }

    
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_REHEARSE_TIMINGS ) )
    {
        sal_Bool bDisable = sal_True;
        model::PageEnumeration aAllPages (
            model::PageEnumerationProvider::CreateAllPagesEnumeration(mrSlideSorter.GetModel()));
        while (aAllPages.HasMoreElements())
        {
            SdPage* pPage = aAllPages.GetNextElement()->GetPage();

            if( !pPage->IsExcluded() )
                bDisable = sal_False;
        }
        if( bDisable || pDocShell->IsPreview())
        {
            rSet.DisableItem( SID_PRESENTATION );
            rSet.DisableItem( SID_REHEARSE_TIMINGS );
        }
    }


    
    
    
    if (rSet.GetItemState(SID_RENAMEPAGE) == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_RENAME_MASTER_PAGE) == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_DUPLICATE_PAGE) == SFX_ITEM_AVAILABLE)
    {
        int n = mrSlideSorter.GetController().GetPageSelector()
            .GetSelectedPageCount();
        if (n != 1)
        {
            rSet.DisableItem(SID_RENAMEPAGE);
            rSet.DisableItem(SID_RENAME_MASTER_PAGE);
        }
        if (n == 0)
        {
            rSet.DisableItem(SID_DUPLICATE_PAGE);
        }
    }

    if (rSet.GetItemState(SID_HIDE_SLIDE) == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_SHOW_SLIDE)  == SFX_ITEM_AVAILABLE)
    {
        model::PageEnumeration aSelectedPages (
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        const SlideExclusionState eState (GetSlideExclusionState(aSelectedPages));
        switch (eState)
        {
            case MIXED:
                
                break;

            case EXCLUDED:
                rSet.DisableItem(SID_HIDE_SLIDE);
                break;

            case INCLUDED:
                rSet.DisableItem(SID_SHOW_SLIDE);
                break;

            case UNDEFINED:
                rSet.DisableItem(SID_HIDE_SLIDE);
                rSet.DisableItem(SID_SHOW_SLIDE);
                break;
        }
    }


    PageKind ePageKind = mrSlideSorter.GetModel().GetPageType();
    if ((eEditMode == EM_MASTERPAGE) && (ePageKind != PK_HANDOUT))
    {
        rSet.DisableItem(SID_ASSIGN_LAYOUT);
    }

    if ((eEditMode == EM_MASTERPAGE) || (ePageKind==PK_NOTES))
    {
        rSet.DisableItem(SID_INSERTPAGE);
    }

    
    if (eEditMode == EM_MASTERPAGE)
    {
        if (rSet.GetItemState(SID_INSERTPAGE) == SFX_ITEM_AVAILABLE)
            rSet.DisableItem(SID_INSERTPAGE);
        if (rSet.GetItemState(SID_DUPLICATE_PAGE) == SFX_ITEM_AVAILABLE)
            rSet.DisableItem(SID_DUPLICATE_PAGE);
    }
}




void SlotManager::GetClipboardState ( SfxItemSet& rSet)
{
    SdTransferable* pTransferClip = SD_MOD()->pTransferClip;

    if (rSet.GetItemState(SID_PASTE)  == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_PASTE_SPECIAL)  == SFX_ITEM_AVAILABLE)
    {
        
        if ( !pTransferClip || !pTransferClip->GetDocShell() )
        {
            rSet.DisableItem(SID_PASTE);
            rSet.DisableItem(SID_PASTE_SPECIAL);
        }
        else
        {
            SfxObjectShell* pTransferDocShell = pTransferClip->GetDocShell();

            if( !pTransferDocShell || ( (DrawDocShell*) pTransferDocShell)->GetDoc()->GetPageCount() <= 1 )
            {
                bool bIsPastingSupported (false);

                
                
                ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
                if (pBase != NULL)
                {
                    ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
                        ::boost::dynamic_pointer_cast<DrawViewShell>(pBase->GetMainViewShell()));
                    if (pDrawViewShell.get() != NULL)
                    {
                        TransferableDataHelper aDataHelper (
                            TransferableDataHelper::CreateFromSystemClipboard(
                                pDrawViewShell->GetActiveWindow()));
                        if (aDataHelper.GetFormatCount() > 0)
                            bIsPastingSupported = true;
                    }
                }

                if ( ! bIsPastingSupported)
                {
                    rSet.DisableItem(SID_PASTE);
                    rSet.DisableItem(SID_PASTE_SPECIAL);
                }
            }
        }
    }

    
    if (rSet.GetItemState(SID_COPY) == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_PASTE)  == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_PASTE_SPECIAL)  == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_CUT)  == SFX_ITEM_AVAILABLE)
    {
        if (mrSlideSorter.GetModel().GetEditMode() == EM_MASTERPAGE)
        {
            if (rSet.GetItemState(SID_CUT) == SFX_ITEM_AVAILABLE)
                rSet.DisableItem(SID_CUT);
            if (rSet.GetItemState(SID_COPY) == SFX_ITEM_AVAILABLE)
                rSet.DisableItem(SID_COPY);
            if (rSet.GetItemState(SID_PASTE) == SFX_ITEM_AVAILABLE)
                rSet.DisableItem(SID_PASTE);
            if (rSet.GetItemState(SID_PASTE_SPECIAL) == SFX_ITEM_AVAILABLE)
                rSet.DisableItem(SID_PASTE_SPECIAL);
        }
    }

    
    if (rSet.GetItemState(SID_CUT) == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_COPY)  == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_DELETE) == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_DELETE_PAGE) == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_DELETE_MASTER_PAGE) == SFX_ITEM_AVAILABLE)
    {
        model::PageEnumeration aSelectedPages (
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));

        
        if ( ! aSelectedPages.HasMoreElements())
            rSet.DisableItem(SID_COPY);

        bool bDisable = false;
        
        
        
        
        

        
        if ( ! aSelectedPages.HasMoreElements())
            bDisable = true;
        
        
        else if (mrSlideSorter.GetController().GetPageSelector().GetSelectedPageCount()
            >= mrSlideSorter.GetController().GetPageSelector().GetPageCount())
            bDisable = true;
        
        
        else while (aSelectedPages.HasMoreElements())
        {
            SdPage* pPage = aSelectedPages.GetNextElement()->GetPage();
            int nUseCount (mrSlideSorter.GetModel().GetDocument()
                ->GetMasterPageUserCount(pPage));
            if (nUseCount > 0)
            {
                bDisable = true;
                break;
            }
        }

        if (bDisable)
        {
            rSet.DisableItem(SID_CUT);
            rSet.DisableItem(SID_DELETE_PAGE);
            rSet.DisableItem(SID_DELETE_MASTER_PAGE);
        }
    }
}




void SlotManager::GetStatusBarState (SfxItemSet& rSet)
{
    
    SdPage* pPage      = NULL;
    SdPage* pFirstPage = NULL;
    sal_uInt16 nFirstPage;
    sal_Int32 nPageCount;
    sal_Int32 nActivePageCount;
    sal_uInt16 nSelectedPages = mrSlideSorter.GetController().GetPageSelector().GetSelectedPageCount();
    OUStringBuffer aPageStr;
    OUString aLayoutStr;

    
    if (nSelectedPages > 0)
    {
        aPageStr = SD_RESSTR(STR_SLIDE_SINGULAR);
        model::PageEnumeration aSelectedPages (
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        if (pDescriptor)
        {
            pPage = pDescriptor->GetPage();
            nFirstPage = (pPage->GetPageNum()/2) + 1;
            nPageCount = mrSlideSorter.GetModel().GetPageCount();
            nActivePageCount = static_cast<sal_Int32>(mrSlideSorter.GetModel().GetDocument()->GetActiveSdPageCount());

            aPageStr.append(" ").append(static_cast<sal_Int32>(nFirstPage), 10).append(" / ").append(nPageCount, 10);
            if (nPageCount != nActivePageCount)
            {
                aPageStr.append(" (").append(nActivePageCount, 10).append(")");
            }
        }
      rSet.Put( SfxStringItem( SID_STATUS_PAGE, aPageStr.makeStringAndClear() ) );
    }

    
    if (nSelectedPages == 1 && pPage != NULL)
    {
        pFirstPage = pPage;
        aLayoutStr = pFirstPage->GetLayoutName();
        sal_Int32 nIndex = aLayoutStr.indexOf( SD_LT_SEPARATOR );
        if( nIndex != -1 )
            aLayoutStr = aLayoutStr.copy(0, nIndex);
        rSet.Put( SfxStringItem( SID_STATUS_LAYOUT, aLayoutStr ) );
    }
}

void SlotManager::ShowSlideShow( SfxRequest& rReq)
{
    slideshowhelp::ShowSlideShow(rReq, *mrSlideSorter.GetModel().GetDocument());
}

void SlotManager::RenameSlide (void)
{
    PageKind ePageKind = mrSlideSorter.GetModel().GetPageType();
    View* pDrView = &mrSlideSorter.GetView();

    if (ePageKind==PK_STANDARD || ePageKind==PK_NOTES)
    {
        if ( pDrView->IsTextEdit() )
        {
            pDrView->SdrEndTextEdit();
        }

        SdPage* pSelectedPage = NULL;
        model::PageEnumeration aSelectedPages (
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        if (aSelectedPages.HasMoreElements())
            pSelectedPage = aSelectedPages.GetNextElement()->GetPage();
        if (pSelectedPage != NULL)
        {
            OUString aTitle( SdResId( STR_TITLE_RENAMESLIDE ) );
            OUString aDescr( SdResId( STR_DESC_RENAMESLIDE ) );
            OUString aPageName = pSelectedPage->GetName();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog(
                mrSlideSorter.GetContentWindow().get(),
                aPageName, aDescr);
            DBG_ASSERT(aNameDlg, "Dialogdiet fail!");
            aNameDlg->SetText( aTitle );
            aNameDlg->SetCheckNameHdl( LINK( this, SlotManager, RenameSlideHdl ), true );
            aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

            if( aNameDlg->Execute() == RET_OK )
            {
                OUString aNewName;
                aNameDlg->GetName( aNewName );
                if (aNewName != aPageName)
                {
#ifdef DBG_UTIL
                    bool bResult =
#endif
                        RenameSlideFromDrawViewShell(
                          pSelectedPage->GetPageNum()/2, aNewName );
                    DBG_ASSERT( bResult, "Couldn't rename slide" );
                }
            }
            delete aNameDlg;

            
            
            mrSlideSorter.GetController().PageNameHasChanged(
                (pSelectedPage->GetPageNum()-1)/2, aPageName);
        }
    }
}

IMPL_LINK(SlotManager, RenameSlideHdl, AbstractSvxNameDialog*, pDialog)
{
    if( ! pDialog )
        return 0;

    OUString aNewName;
    pDialog->GetName( aNewName );

    model::SharedPageDescriptor pDescriptor (
        mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
    SdPage* pCurrentPage = NULL;
    if (pDescriptor.get() != NULL)
        pCurrentPage = pDescriptor->GetPage();

    return long( (pCurrentPage!=NULL && aNewName == pCurrentPage->GetName())
        || (mrSlideSorter.GetViewShell()
            && mrSlideSorter.GetViewShell()->GetDocSh()->IsNewPageNameValid( aNewName ) ));
}

bool SlotManager::RenameSlideFromDrawViewShell( sal_uInt16 nPageId, const OUString & rName  )
{
    sal_Bool   bOutDummy;
    SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
    if( pDocument->GetPageByName( rName, bOutDummy ) != SDRPAGE_NOTFOUND )
        return false;

    SdPage* pPageToRename = NULL;
    PageKind ePageKind = mrSlideSorter.GetModel().GetPageType();

    ::svl::IUndoManager* pManager = pDocument->GetDocSh()->GetUndoManager();

    if( mrSlideSorter.GetModel().GetEditMode() == EM_PAGE )
    {
        model::SharedPageDescriptor pDescriptor (
            mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
        if (pDescriptor.get() != NULL)
            pPageToRename = pDescriptor->GetPage();

        if (pPageToRename != NULL)
        {
            
            SdPage* pUndoPage = pPageToRename;
            SdrLayerAdmin &  rLayerAdmin = pDocument->GetLayerAdmin();
            sal_uInt8 nBackground = rLayerAdmin.GetLayerID( SD_RESSTR( STR_LAYER_BCKGRND ), false );
            sal_uInt8 nBgObj = rLayerAdmin.GetLayerID( SD_RESSTR( STR_LAYER_BCKGRNDOBJ ), false );
            SetOfByte aVisibleLayers = pPageToRename->TRG_GetMasterPageVisibleLayers();

            
            ModifyPageUndoAction* pAction = new ModifyPageUndoAction(
                pDocument, pUndoPage, rName, pUndoPage->GetAutoLayout(),
                aVisibleLayers.IsSet( nBackground ),
                aVisibleLayers.IsSet( nBgObj ));
            pManager->AddUndoAction( pAction );

            
            pPageToRename->SetName( rName );

            if( ePageKind == PK_STANDARD )
            {
                
                SdPage* pNotesPage = pDocument->GetSdPage( nPageId, PK_NOTES );
                if (pNotesPage != NULL)
                    pNotesPage->SetName (rName);
            }
        }
    }
    else
    {
        
        pPageToRename = pDocument->GetMasterSdPage( nPageId, ePageKind );
        if (pPageToRename != NULL)
        {
            const OUString aOldLayoutName( pPageToRename->GetLayoutName() );
            pManager->AddUndoAction( new RenameLayoutTemplateUndoAction( pDocument, aOldLayoutName, rName ) );
            pDocument->RenameLayoutTemplate( aOldLayoutName, rName );
        }
    }

    bool bSuccess = pPageToRename!=NULL && ( rName == pPageToRename->GetName() );

    if( bSuccess )
    {
        
        

        
        pDocument->SetChanged( true );

        
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, true );
        if (mrSlideSorter.GetViewShell() != NULL)
            mrSlideSorter.GetViewShell()->GetDispatcher()->Execute(
                SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
    }

    return bSuccess;
}




/** Insert a slide.  The insertion position depends on a) the selection and
    b) the mouse position when there is no selection.

    When there is a selection then insertion takes place after the last
    slide of the selection.  For this to work all but the last selected
    slide are deselected first.

    Otherwise, when there is no selection but the insertion marker is visible
    the slide is inserted at that position.  The slide before that marker is
    selected first.

    When both the selection and the insertion marker are not visible--can
    that happen?--the new slide is inserted after the last slide.
*/
void SlotManager::InsertSlide (SfxRequest& rRequest)
{
    const sal_Int32 nInsertionIndex (GetInsertionPosition());

    PageSelector::BroadcastLock aBroadcastLock (mrSlideSorter);

    SdPage* pNewPage = NULL;
    if (mrSlideSorter.GetModel().GetEditMode() == EM_PAGE)
    {
        SlideSorterViewShell* pShell = dynamic_cast<SlideSorterViewShell*>(
            mrSlideSorter.GetViewShell());
        if (pShell != NULL)
        {
            pNewPage = pShell->CreateOrDuplicatePage (
                rRequest,
                mrSlideSorter.GetModel().GetPageType(),
                nInsertionIndex>=0
                    ? mrSlideSorter.GetModel().GetPageDescriptor(nInsertionIndex)->GetPage()
                        : NULL);
        }
    }
    else
    {
        
        SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
        Reference<drawing::XMasterPagesSupplier> xMasterPagesSupplier (
            pDocument->getUnoModel(), UNO_QUERY);
        if (xMasterPagesSupplier.is())
        {
            Reference<drawing::XDrawPages> xMasterPages (
                xMasterPagesSupplier->getMasterPages());
            if (xMasterPages.is())
            {
                xMasterPages->insertNewByIndex (nInsertionIndex+1);

                
                pNewPage = pDocument->GetMasterSdPage(
                    (sal_uInt16)(nInsertionIndex+1), PK_STANDARD);
                pNewPage->CreateTitleAndLayout (sal_True,sal_True);
            }
        }
    }
    if (pNewPage == NULL)
        return;

    
    
    view::SlideSorterView::DrawLock aDrawLock (mrSlideSorter);
    PageSelector::UpdateLock aUpdateLock (mrSlideSorter);
    mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
    mrSlideSorter.GetController().GetPageSelector().SelectPage(pNewPage);
}




void SlotManager::DuplicateSelectedSlides (SfxRequest& rRequest)
{
    
    
    sal_Int32 nInsertPosition (0);
    ::std::vector<SdPage*> aPagesToDuplicate;
    model::PageEnumeration aSelectedPages (
        model::PageEnumerationProvider::CreateSelectedPagesEnumeration(mrSlideSorter.GetModel()));
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        if (pDescriptor && pDescriptor->GetPage())
        {
            aPagesToDuplicate.push_back(pDescriptor->GetPage());
            nInsertPosition = pDescriptor->GetPage()->GetPageNum()+2;
        }
    }

    
    
    const bool bUndo (aPagesToDuplicate.size()>1 && mrSlideSorter.GetView().IsUndoEnabled());
    if (bUndo)
        mrSlideSorter.GetView().BegUndo(SD_RESSTR(STR_INSERTPAGE));

    ::std::vector<SdPage*> aPagesToSelect;
    for(::std::vector<SdPage*>::const_iterator
            iPage(aPagesToDuplicate.begin()),
            iEnd(aPagesToDuplicate.end());
        iPage!=iEnd;
        ++iPage, nInsertPosition+=2)
    {
        aPagesToSelect.push_back(
            mrSlideSorter.GetViewShell()->CreateOrDuplicatePage(
                rRequest, PK_STANDARD, *iPage, nInsertPosition));
    }
    aPagesToDuplicate.clear();

    if (bUndo)
        mrSlideSorter.GetView().EndUndo();

    
    PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());
    rSelector.DeselectAllPages();
    ::std::for_each (
        aPagesToSelect.begin(),
        aPagesToSelect.end(),
        ::boost::bind(
            static_cast<void (PageSelector::*)(const SdPage*)>(&PageSelector::SelectPage),
            ::boost::ref(rSelector),
            _1));
}

void SlotManager::ChangeSlideExclusionState (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bExcludeSlide)
{
    if (rpDescriptor)
    {
        mrSlideSorter.GetView().SetState(
            rpDescriptor,
            model::PageDescriptor::ST_Excluded,
            bExcludeSlide);
    }
    else
    {
        model::PageEnumeration aSelectedPages (
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        while (aSelectedPages.HasMoreElements())
        {
            model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
            mrSlideSorter.GetView().SetState(
                pDescriptor,
                model::PageDescriptor::ST_Excluded,
                bExcludeSlide);
        }
    }

    SfxBindings& rBindings (mrSlideSorter.GetViewShell()->GetViewFrame()->GetBindings());
    rBindings.Invalidate(SID_PRESENTATION);
    rBindings.Invalidate(SID_REHEARSE_TIMINGS);
    rBindings.Invalidate(SID_HIDE_SLIDE);
    rBindings.Invalidate(SID_SHOW_SLIDE);
    mrSlideSorter.GetModel().GetDocument()->SetChanged();
}




sal_Int32 SlotManager::GetInsertionPosition (void)
{
    PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());

    
    
    if (mrSlideSorter.GetController().GetInsertionIndicatorHandler()->IsActive())
    {
        
        return mrSlideSorter.GetController().GetInsertionIndicatorHandler()->GetInsertionPageIndex()
            - 1;
    }

    
    else if (mrSlideSorter.GetController().GetSelectionManager()->GetInsertionPosition() >= 0)
    {
        return mrSlideSorter.GetController().GetSelectionManager()->GetInsertionPosition() - 1;
    }

    
    else if (rSelector.GetSelectedPageCount() > 0)
    {
        for (int nIndex=rSelector.GetPageCount()-1; nIndex>=0; --nIndex)
            if (rSelector.IsPageSelected(nIndex))
                return nIndex;

        
        OSL_ASSERT(false);
        return rSelector.GetPageCount() - 1;
    }

    
    else if (rSelector.GetPageCount() > 0)
    {
        return rSelector.GetPageCount() - 1;
    }

    
    
    else
    {
        
        OSL_ASSERT(false);
        return -1;
    }
}




void SlotManager::NotifyEditModeChange (void)
{
    SfxBindings& rBindings (mrSlideSorter.GetViewShell()->GetViewFrame()->GetBindings());
    rBindings.Invalidate(SID_PRESENTATION);
    rBindings.Invalidate(SID_INSERTPAGE);
    rBindings.Invalidate(SID_DUPLICATE_PAGE);
}






namespace {



SlideExclusionState GetSlideExclusionState (model::PageEnumeration& rPageSet)
{
    SlideExclusionState eState (UNDEFINED);
    sal_Bool bState;

    
    while (rPageSet.HasMoreElements() && eState!=MIXED)
    {
        bState = rPageSet.GetNextElement()->GetPage()->IsExcluded();
        switch (eState)
        {
            case UNDEFINED:
                
                eState = bState ? EXCLUDED : INCLUDED;
                break;

            case EXCLUDED:
                
                
                if ( ! bState)
                    eState = MIXED;
                break;

            case INCLUDED:
                
                
                if (bState)
                    eState = MIXED;
                break;

            case MIXED:
            default:
                
                break;
        }
    }

    return eState;
}

} 

} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
