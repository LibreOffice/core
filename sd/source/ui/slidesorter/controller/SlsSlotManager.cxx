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

#include <com/sun/star/beans/PropertyValue.hpp>

#include <editeng/outlobj.hxx>

#include <controller/SlsSlotManager.hxx>
#include <SlideSorter.hxx>
#include <SlideSorterViewShell.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <controller/SlsFocusManager.hxx>
#include <controller/SlsInsertionIndicatorHandler.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsSelectionFunction.hxx>
#include <controller/SlsSelectionManager.hxx>
#include <controller/SlsSelectionObserver.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <view/SlideSorterView.hxx>
#include <view/SlsLayouter.hxx>
#include <framework/FrameworkHelper.hxx>
#include <Window.hxx>
#include <fupoor.hxx>
#include <fuzoom.hxx>
#include <fucushow.hxx>
#include <fusldlg.hxx>
#include <fuexpand.hxx>
#include <fusumry.hxx>
#include <fuscale.hxx>
#include <slideshow.hxx>
#include <app.hrc>
#include <strings.hrc>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include <ViewShellImplementation.hxx>
#include <sdattr.hxx>
#include <FrameView.hxx>
#include <zoomlist.hxx>
#include <sdpage.hxx>
#include <sdxfer.hxx>
#include <helpids.h>
#include <unmodpg.hxx>
#include <DrawViewShell.hxx>
#include <sdabstdlg.hxx>
#include <sdmod.hxx>

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svx/svxids.hrc>
#include <sfx2/zoomitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svl/intitem.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svl/aeitem.hxx>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <vcl/svapp.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
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

} // end of anonymous namespace

SlotManager::SlotManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter)
{
}

SlotManager::~SlotManager()
{
}

void SlotManager::FuTemporary (SfxRequest& rRequest)
{
    SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();

    SlideSorterViewShell* pShell
        = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
    if (pShell == nullptr)
        return;

    switch (rRequest.GetSlot())
    {
        case SID_PRESENTATION:
        case SID_PRESENTATION_CURRENT_SLIDE:
        case SID_REHEARSE_TIMINGS:
            slideshowhelp::ShowSlideShow(rRequest, *mrSlideSorter.GetModel().GetDocument());
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
            if (rRequest.GetArgs() != nullptr)
            {
                const SfxUInt16Item* pPagesPerRow = rRequest.GetArg<SfxUInt16Item>(SID_PAGES_PER_ROW);
                if (pPagesPerRow != nullptr)
                {
                    sal_Int32 nColumnCount = pPagesPerRow->GetValue();
                    // Force the given number of columns by setting
                    // the minimal and maximal number of columns to
                    // the same value.
                    mrSlideSorter.GetView().GetLayouter().SetColumnCount (
                        nColumnCount, nColumnCount);
                    // Force a repaint and re-layout.
                    pShell->ArrangeGUIElements ();
                    // Rearrange the UI-elements controlled by the
                    // controller and force a rearrangement of the
                    // view.
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
            // First make sure that the sidebar is visible
            pShell->GetViewFrame()->ShowChildWindow(SID_SIDEBAR);
            ::sfx2::sidebar::Sidebar::ShowPanel(
                "SdSlideTransitionPanel",
                pShell->GetViewFrame()->GetFrame().GetFrameInterface());
            rRequest.Ignore ();
            break;
        }

        case SID_PRESENTATION_DLG:
            FuSlideShowDlg::Create (
                pShell,
                mrSlideSorter.GetContentWindow(),
                &mrSlideSorter.GetView(),
                pDocument,
                rRequest);
            break;

        case SID_CUSTOMSHOW_DLG:
            FuCustomShowDlg::Create (
                pShell,
                mrSlideSorter.GetContentWindow(),
                &mrSlideSorter.GetView(),
                pDocument,
                rRequest);
                break;

        case SID_EXPAND_PAGE:
            FuExpandPage::Create (
                pShell,
                mrSlideSorter.GetContentWindow(),
                &mrSlideSorter.GetView(),
                pDocument,
                rRequest);
            break;

        case SID_SUMMARY_PAGE:
            FuSummaryPage::Create (
                pShell,
                mrSlideSorter.GetContentWindow(),
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
        case SID_DELETE: // we need SID_CUT to handle the delete key
            // (DEL -> accelerator -> SID_CUT).
            if (mrSlideSorter.GetModel().GetPageCount() > 1)
            {
                mrSlideSorter.GetController().GetSelectionManager()->DeleteSelectedPages();
            }

            rRequest.Done();
            break;

        case SID_RENAMEPAGE:
        case SID_RENAME_MASTER_PAGE:
            RenameSlide (rRequest);
            rRequest.Done ();
            break;

        case SID_ASSIGN_LAYOUT:
        {
            pShell->mpImpl->AssignLayout( rRequest, PageKind::Standard );
            rRequest.Done ();
        }
        break;

        case SID_PHOTOALBUM:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            vcl::Window* pWin = mrSlideSorter.GetContentWindow();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSdPhotoAlbumDialog(
                pWin ? pWin->GetFrameWeld() : nullptr,
                pDocument));
            pDlg->Execute();
            rRequest.Done ();
        }
        break;

        case SID_REMOTE_DLG:
        {
#ifdef ENABLE_SDREMOTE
             SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
             ScopedVclPtr<VclAbstractDialog> pDlg( pFact->CreateRemoteDialog( mrSlideSorter.GetContentWindow() ) );
             pDlg->Execute();
#endif
        }
        break;

        default:
            break;
    }
}

void SlotManager::FuPermanent (SfxRequest& rRequest)
{
    ViewShell* pShell = mrSlideSorter.GetViewShell();
    if (pShell == nullptr)
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

    //! that's only until ENUM-Slots ?are
    //  Invalidate( SID_OBJECT_SELECT );
}

void SlotManager::FuSupport (SfxRequest& rRequest)
{
    switch (rRequest.GetSlot())
    {
        case SID_STYLE_FAMILY:
            if (rRequest.GetArgs() != nullptr)
            {
                SdDrawDocument* pDocument
                    = mrSlideSorter.GetModel().GetDocument();
                if (pDocument != nullptr)
                {
                    const SfxPoolItem& rItem (
                        rRequest.GetArgs()->Get(SID_STYLE_FAMILY));
                    pDocument->GetDocSh()->SetStyleFamily(
                        static_cast<SfxStyleFamily>(static_cast<const SfxUInt16Item&>(rItem).GetValue()));
                }
            }
            break;

        case SID_PASTE:
        {
            SdTransferable* pTransferClip = SD_MOD()->pTransferClip;
            if( pTransferClip )
            {
                SfxObjectShell* pTransferDocShell = pTransferClip->GetDocShell().get();

                DrawDocShell* pDocShell = dynamic_cast<DrawDocShell*>(pTransferDocShell);
                if (pDocShell && pDocShell->GetDoc()->GetPageCount() > 1)
                {
                    mrSlideSorter.GetController().GetClipboard().HandleSlotCall(rRequest);
                    break;
                }
            }
            ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
            if (pBase != nullptr)
            {
                std::shared_ptr<DrawViewShell> pDrawViewShell (
                    std::dynamic_pointer_cast<DrawViewShell>(pBase->GetMainViewShell()));
                if (pDrawViewShell != nullptr)
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
        case SID_NOTES_MODE:
        case SID_HANDOUT_MASTER_MODE:
        case SID_SLIDE_SORTER_MODE:
        case SID_OUTLINE_MODE:
        {
            ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
            if (pBase != nullptr)
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
            if (pViewShell != nullptr)
            {
                pViewShell->ImpSidUndo (rRequest);
            }
            break;
        }

        case SID_REDO:
        {
            SlideSorterViewShell* pViewShell
                = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
            if (pViewShell != nullptr)
            {
                pViewShell->ImpSidRedo (rRequest);
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
            // empty Undo-Manager
            mrSlideSorter.GetModel().GetDocument()->GetDocSh()->ClearUndoBuffer();

            // normal forwarding to ViewFrame for execution
            if (pViewShell != nullptr)
                pViewShell->GetViewFrame()->ExecuteSlot(rRequest);

            // has to be finished right away
            return;
        }

        case SID_OUTPUT_QUALITY_COLOR:
        case SID_OUTPUT_QUALITY_GRAYSCALE:
        case SID_OUTPUT_QUALITY_BLACKWHITE:
        case SID_OUTPUT_QUALITY_CONTRAST:
        {
            // flush page cache
            if (pViewShell != nullptr)
                pViewShell->ExecReq (rRequest);
            break;
        }

        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            if (pViewShell != nullptr)
                pViewShell->ExecReq (rRequest);
            break;
        }

        case SID_OPT_LOCALE_CHANGED:
        {
            mrSlideSorter.GetController().UpdateAllPages();
            if (pViewShell != nullptr)
                pViewShell->UpdatePreview (pViewShell->GetActualPage());
            rRequest.Done();
            break;
        }

        case SID_SEARCH_DLG:
            // We have to handle the SID_SEARCH_DLG slot explicitly because
            // in some cases (when the slide sorter is displayed in the
            // center pane) we want to disable the search dialog.  Therefore
            // we have to handle the execution of that slot as well.
            // We try to do that by forwarding the request to the view frame
            // of the view shell.
            if (pViewShell != nullptr)
                pViewShell->GetViewFrame()->ExecuteSlot(rRequest);
            break;

        default:
            break;
    }
}

void SlotManager::GetAttrState (SfxItemSet& rSet)
{
    // Iterate over all items.
    SfxWhichIter aIter (rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        sal_uInt16 nSlotId (nWhich);
        if (SfxItemPool::IsWhich(nWhich) && mrSlideSorter.GetViewShell()!=nullptr)
            nSlotId = mrSlideSorter.GetViewShell()->GetPool().GetSlotId(nWhich);
        switch (nSlotId)
        {
            case SID_PAGES_PER_ROW:
                rSet.Put (
                    SfxUInt16Item (
                        nSlotId,
                        static_cast<sal_uInt16>(mrSlideSorter.GetView().GetLayouter().GetColumnCount())
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

    if (pShell!=nullptr && pShell->GetCurrentFunction().is())
    {
        sal_uInt16 nSId = pShell->GetCurrentFunction()->GetSlotID();

        rSet.Put( SfxBoolItem( nSId, true ) );
    }
    rSet.Put( SfxBoolItem( SID_DRAWINGMODE, false ) );
    rSet.Put( SfxBoolItem( SID_SLIDE_SORTER_MODE, true ) );
    rSet.Put( SfxBoolItem( SID_OUTLINE_MODE, false ) );
    rSet.Put( SfxBoolItem( SID_NOTES_MODE, false ) );
    rSet.Put( SfxBoolItem( SID_HANDOUT_MASTER_MODE, false ) );

    if (pShell!=nullptr && pShell->IsMainViewShell())
    {
        rSet.DisableItem(SID_SPELL_DIALOG);
        rSet.DisableItem(SID_SEARCH_DLG);
    }

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_EXPAND_PAGE))
    {
        bool bDisable = true;
        if (eEditMode == EditMode::Page)
        {
            // At least one of the selected pages has to contain an outline
            // presentation objects in order to enable the expand page menu
            // entry.
            model::PageEnumeration aSelectedPages (
                model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                    mrSlideSorter.GetModel()));
            while (aSelectedPages.HasMoreElements())
            {
                SdPage* pPage = aSelectedPages.GetNextElement()->GetPage();
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_OUTLINE);
                if (pObj!=nullptr )
                {
                    if( !pObj->IsEmptyPresObj() )
                    {
                        bDisable = false;
                    }
                    else
                    {
                        // check if the object is in edit, than its temporarily not empty
                        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                        if( pTextObj )
                        {
                            std::unique_ptr<OutlinerParaObject> pParaObj = pTextObj->GetEditOutlinerParaObject();
                            if( pParaObj )
                            {
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

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_SUMMARY_PAGE))
    {
        bool bDisable = true;
        if (eEditMode == EditMode::Page)
        {
            // At least one of the selected pages has to contain a title
            // presentation objects in order to enable the summary page menu
            // entry.
            model::PageEnumeration aSelectedPages (
                model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                    mrSlideSorter.GetModel()));
            while (aSelectedPages.HasMoreElements())
            {
                SdPage* pPage = aSelectedPages.GetNextElement()->GetPage();
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_TITLE);

                if (pObj!=nullptr && !pObj->IsEmptyPresObj())
                    bDisable = false;
            }
        }
        if (bDisable)
            rSet.DisableItem (SID_SUMMARY_PAGE);
    }

    // starting of presentation possible?
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_PRESENTATION ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_REHEARSE_TIMINGS ) )
    {
        bool bDisable = true;
        model::PageEnumeration aAllPages (
            model::PageEnumerationProvider::CreateAllPagesEnumeration(mrSlideSorter.GetModel()));
        while (aAllPages.HasMoreElements())
        {
            SdPage* pPage = aAllPages.GetNextElement()->GetPage();

            if( !pPage->IsExcluded() )
                bDisable = false;
        }
        if( bDisable || pDocShell->IsPreview())
        {
            rSet.DisableItem( SID_PRESENTATION );
            rSet.DisableItem( SID_REHEARSE_TIMINGS );
        }
    }

    // Disable the rename slots when there are no or more than one slides/master
    // pages selected; disable the duplicate slot when there are no slides
    // selected:
    if (rSet.GetItemState(SID_RENAMEPAGE) == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_RENAME_MASTER_PAGE) == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_DUPLICATE_PAGE) == SfxItemState::DEFAULT)
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

    if (rSet.GetItemState(SID_HIDE_SLIDE) == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_SHOW_SLIDE)  == SfxItemState::DEFAULT)
    {
        model::PageEnumeration aSelectedPages (
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        const SlideExclusionState eState (GetSlideExclusionState(aSelectedPages));
        switch (eState)
        {
            case MIXED:
                // Show both entries.
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

    if (eEditMode == EditMode::MasterPage)
    {
        rSet.DisableItem(SID_ASSIGN_LAYOUT);
        rSet.DisableItem(SID_INSERTPAGE);
    }

    // Disable some slots when in master page mode.
    if (eEditMode == EditMode::MasterPage)
    {
        if (rSet.GetItemState(SID_INSERTPAGE) == SfxItemState::DEFAULT)
            rSet.DisableItem(SID_INSERTPAGE);
        if (rSet.GetItemState(SID_DUPLICATE_PAGE) == SfxItemState::DEFAULT)
            rSet.DisableItem(SID_DUPLICATE_PAGE);
    }
}

void SlotManager::GetClipboardState ( SfxItemSet& rSet)
{
    SdTransferable* pTransferClip = SD_MOD()->pTransferClip;

    if (rSet.GetItemState(SID_PASTE)  == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_PASTE_SPECIAL)  == SfxItemState::DEFAULT)
    {
        // no own clipboard data?
        if ( !pTransferClip || !pTransferClip->GetDocShell().is() )
        {
            rSet.DisableItem(SID_PASTE);
            rSet.DisableItem(SID_PASTE_SPECIAL);
        }
        else
        {
            SfxObjectShell* pTransferDocShell = pTransferClip->GetDocShell().get();

            if( !pTransferDocShell || static_cast<DrawDocShell*>(pTransferDocShell)->GetDoc()->GetPageCount() <= 1 )
            {
                bool bIsPastingSupported (false);

                // No or just one page.  Check if there is anything that can be
                // pasted via a DrawViewShell.
                ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
                if (pBase != nullptr)
                {
                    std::shared_ptr<DrawViewShell> pDrawViewShell (
                        std::dynamic_pointer_cast<DrawViewShell>(pBase->GetMainViewShell()));
                    if (pDrawViewShell != nullptr)
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

    // Cut, copy and paste of master pages is not yet implemented properly
    if (rSet.GetItemState(SID_COPY) == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_PASTE)  == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_PASTE_SPECIAL)  == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_CUT)  == SfxItemState::DEFAULT)
    {
        if (mrSlideSorter.GetModel().GetEditMode() == EditMode::MasterPage)
        {
            if (rSet.GetItemState(SID_CUT) == SfxItemState::DEFAULT)
                rSet.DisableItem(SID_CUT);
            if (rSet.GetItemState(SID_COPY) == SfxItemState::DEFAULT)
                rSet.DisableItem(SID_COPY);
            if (rSet.GetItemState(SID_PASTE) == SfxItemState::DEFAULT)
                rSet.DisableItem(SID_PASTE);
            if (rSet.GetItemState(SID_PASTE_SPECIAL) == SfxItemState::DEFAULT)
                rSet.DisableItem(SID_PASTE_SPECIAL);
        }
    }

    // Cut, copy, and delete page are disabled when there is no selection.
    if (!(rSet.GetItemState(SID_CUT) == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_COPY)  == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_DELETE) == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_DELETE_PAGE) == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_DELETE_MASTER_PAGE) == SfxItemState::DEFAULT))
        return;

    model::PageEnumeration aSelectedPages (
        model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mrSlideSorter.GetModel()));

    // For copy to work we have to have at least one selected page.
    if ( ! aSelectedPages.HasMoreElements())
        rSet.DisableItem(SID_COPY);

    bool bDisable = false;
    // The operations that lead to the deletion of a page are valid if
    // a) there is at least one selected page
    // b) deleting the selected pages leaves at least one page in the
    // document
    // c) selected master pages must not be used by slides.

    // Test a).
    if ( ! aSelectedPages.HasMoreElements())
        bDisable = true;
    // Test b): Count the number of selected pages.  It has to be less
    // than the number of all pages.
    else if (mrSlideSorter.GetController().GetPageSelector().GetSelectedPageCount()
        >= mrSlideSorter.GetController().GetPageSelector().GetPageCount())
        bDisable = true;
    // Test c): Iterate over the selected pages and look for a master
    // page that is used by at least one page.
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

void SlotManager::GetStatusBarState (SfxItemSet& rSet)
{
    // page view and layout
    SdPage* pPage      = nullptr;
    SdPage* pFirstPage = nullptr;
    sal_uInt16 nFirstPage;
    sal_Int32 nPageCount;
    sal_Int32 nActivePageCount;
    sal_uInt16 nSelectedPages = mrSlideSorter.GetController().GetPageSelector().GetSelectedPageCount();
    OUString aPageStr;
    OUString aLayoutStr;

    //Set number of slides
    if (nSelectedPages > 0)
    {
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

            aPageStr = (nPageCount == nActivePageCount) ? SdResId(STR_SD_PAGE_COUNT) : SdResId(STR_SD_PAGE_COUNT_CUSTOM);

            aPageStr = aPageStr.replaceFirst("%1", OUString::number(nFirstPage));
            aPageStr = aPageStr.replaceFirst("%2", OUString::number(nPageCount));
            if(nPageCount != nActivePageCount)
                aPageStr = aPageStr.replaceFirst("%3", OUString::number(nActivePageCount));
        }
        rSet.Put( SfxStringItem( SID_STATUS_PAGE, aPageStr ) );
    }
    //Set layout
    if (nSelectedPages == 1 && pPage != nullptr)
    {
        pFirstPage = pPage;
        aLayoutStr = pFirstPage->GetLayoutName();
        sal_Int32 nIndex = aLayoutStr.indexOf( SD_LT_SEPARATOR );
        if( nIndex != -1 )
            aLayoutStr = aLayoutStr.copy(0, nIndex);
        rSet.Put( SfxStringItem( SID_STATUS_LAYOUT, aLayoutStr ) );
    }
}

void SlotManager::RenameSlide(const SfxRequest& rRequest)
{
    View* pDrView = &mrSlideSorter.GetView();

    if ( pDrView->IsTextEdit() )
    {
        pDrView->SdrEndTextEdit();
    }

    SdPage* pSelectedPage = nullptr;
    model::PageEnumeration aSelectedPages (
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
    if (aSelectedPages.HasMoreElements())
        pSelectedPage = aSelectedPages.GetNextElement()->GetPage();
    if (pSelectedPage == nullptr)
        return;

    // tdf#107183 Set different dialog titles when renaming
    // master slides or normal ones
    OUString aTitle;
    if( rRequest.GetSlot() == SID_RENAME_MASTER_PAGE )
        aTitle = SdResId( STR_TITLE_RENAMEMASTER );
    else
        aTitle = SdResId( STR_TITLE_RENAMESLIDE );

    OUString aDescr( SdResId( STR_DESC_RENAMESLIDE ) );
    OUString aPageName = pSelectedPage->GetName();

    if(rRequest.GetArgs())
    {
       OUString aName;
       aName = rRequest.GetArgs()->GetItem<const SfxStringItem>(SID_RENAMEPAGE)->GetValue();

       bool bResult =  RenameSlideFromDrawViewShell(pSelectedPage->GetPageNum()/2, aName );
       DBG_ASSERT( bResult, "Couldn't rename slide" );
    }
    else
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        vcl::Window* pWin = mrSlideSorter.GetContentWindow();
        ScopedVclPtr<AbstractSvxNameDialog> aNameDlg(pFact->CreateSvxNameDialog(
                pWin ? pWin->GetFrameWeld() : nullptr,
                aPageName, aDescr));
        aNameDlg->SetText( aTitle );
        aNameDlg->SetCheckNameHdl( LINK( this, SlotManager, RenameSlideHdl ), true );
        aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

        if( aNameDlg->Execute() == RET_OK )
        {
            OUString aNewName;
            aNameDlg->GetName( aNewName );
            if (aNewName != aPageName)
            {
                bool bResult =
                        RenameSlideFromDrawViewShell(
                          pSelectedPage->GetPageNum()/2, aNewName );
                DBG_ASSERT( bResult, "Couldn't rename slide" );
            }
        }
        aNameDlg.disposeAndClear();
    }
    // Tell the slide sorter about the name change (necessary for
    // accessibility.)
    mrSlideSorter.GetController().PageNameHasChanged(
            (pSelectedPage->GetPageNum()-1)/2, aPageName);
}

IMPL_LINK(SlotManager, RenameSlideHdl, AbstractSvxNameDialog&, rDialog, bool)
{
    OUString aNewName;
    rDialog.GetName( aNewName );

    model::SharedPageDescriptor pDescriptor (
        mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
    SdPage* pCurrentPage = nullptr;
    if (pDescriptor.get() != nullptr)
        pCurrentPage = pDescriptor->GetPage();

    return (pCurrentPage!=nullptr && aNewName == pCurrentPage->GetName())
        || (mrSlideSorter.GetViewShell()
            && mrSlideSorter.GetViewShell()->GetDocSh()->IsNewPageNameValid( aNewName ) );
}

bool SlotManager::RenameSlideFromDrawViewShell( sal_uInt16 nPageId, const OUString & rName  )
{
    bool   bOutDummy;
    SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
    if( pDocument->GetPageByName( rName, bOutDummy ) != SDRPAGE_NOTFOUND )
        return false;

    SdPage* pPageToRename = nullptr;

    SfxUndoManager* pManager = pDocument->GetDocSh()->GetUndoManager();

    if( mrSlideSorter.GetModel().GetEditMode() == EditMode::Page )
    {
        model::SharedPageDescriptor pDescriptor (
            mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
        if (pDescriptor.get() != nullptr)
            pPageToRename = pDescriptor->GetPage();

        if (pPageToRename != nullptr)
        {
            // Undo
            SdPage* pUndoPage = pPageToRename;
            SdrLayerAdmin &  rLayerAdmin = pDocument->GetLayerAdmin();
            SdrLayerID nBackground = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
            SdrLayerID nBgObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
            SdrLayerIDSet aVisibleLayers = pPageToRename->TRG_GetMasterPageVisibleLayers();

            // (#67720#)
            pManager->AddUndoAction(
                std::make_unique<ModifyPageUndoAction>(
                    pDocument, pUndoPage, rName, pUndoPage->GetAutoLayout(),
                    aVisibleLayers.IsSet( nBackground ),
                    aVisibleLayers.IsSet( nBgObj )));

            // rename
            pPageToRename->SetName( rName );

            // also rename notes-page
            SdPage* pNotesPage = pDocument->GetSdPage( nPageId, PageKind::Notes );
            if (pNotesPage != nullptr)
                pNotesPage->SetName (rName);
        }
    }
    else
    {
        // rename MasterPage -> rename LayoutTemplate
        pPageToRename = pDocument->GetMasterSdPage( nPageId, PageKind::Standard );
        if (pPageToRename != nullptr)
        {
            const OUString aOldLayoutName( pPageToRename->GetLayoutName() );
            pManager->AddUndoAction( std::make_unique<RenameLayoutTemplateUndoAction>( pDocument, aOldLayoutName, rName ) );
            pDocument->RenameLayoutTemplate( aOldLayoutName, rName );
        }
    }

    bool bSuccess = pPageToRename!=nullptr && ( rName == pPageToRename->GetName() );

    if( bSuccess )
    {
        // user edited page names may be changed by the page so update control
        //        aTabControl.SetPageText( nPageId, rName );

        // set document to modified state
        pDocument->SetChanged();

        // inform navigator about change
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, true );
        if (mrSlideSorter.GetViewShell() != nullptr)
            mrSlideSorter.GetViewShell()->GetDispatcher()->ExecuteList(
                SID_NAVIGATOR_INIT,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                { &aItem });
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

    SdPage* pNewPage = nullptr;
    if (mrSlideSorter.GetModel().GetEditMode() == EditMode::Page)
    {
        SlideSorterViewShell* pShell = dynamic_cast<SlideSorterViewShell*>(
            mrSlideSorter.GetViewShell());
        if (pShell != nullptr)
        {
            pNewPage = pShell->CreateOrDuplicatePage (
                rRequest,
                PageKind::Standard,
                nInsertionIndex>=0
                    ? mrSlideSorter.GetModel().GetPageDescriptor(nInsertionIndex)->GetPage()
                        : nullptr);
        }
    }
    else
    {
        // Use the API to create a new page.
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

                // Create shapes for the default layout.
                pNewPage = pDocument->GetMasterSdPage(
                    static_cast<sal_uInt16>(nInsertionIndex+1), PageKind::Standard);
                pNewPage->CreateTitleAndLayout (true,true);
            }
        }
    }
    if (pNewPage == nullptr)
        return;

    // When a new page has been inserted then select it, make it the
    // current page, and focus it.
    view::SlideSorterView::DrawLock aDrawLock (mrSlideSorter);
    PageSelector::UpdateLock aUpdateLock (mrSlideSorter);
    mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
    mrSlideSorter.GetController().GetPageSelector().SelectPage(pNewPage);
}

void SlotManager::DuplicateSelectedSlides (SfxRequest& rRequest)
{
    // Create a list of the pages that are to be duplicated.  The process of
    // duplication alters the selection.
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

    // Duplicate the pages in aPagesToDuplicate and collect the newly
    // created pages in aPagesToSelect.
    const bool bUndo (aPagesToDuplicate.size()>1 && mrSlideSorter.GetView().IsUndoEnabled());
    if (bUndo)
        mrSlideSorter.GetView().BegUndo(SdResId(STR_INSERTPAGE));

    ::std::vector<SdPage*> aPagesToSelect;
    for(const auto& rpPage : aPagesToDuplicate)
    {
        aPagesToSelect.push_back(
            mrSlideSorter.GetViewShell()->CreateOrDuplicatePage(
                rRequest, PageKind::Standard, rpPage, nInsertPosition));
        nInsertPosition += 2;
    }
    aPagesToDuplicate.clear();

    if (bUndo)
        mrSlideSorter.GetView().EndUndo();

    // Set the selection to the pages in aPagesToSelect.
    PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());
    rSelector.DeselectAllPages();
    for (auto const& it: aPagesToSelect)
    {
        rSelector.SelectPage(it);
    }
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

sal_Int32 SlotManager::GetInsertionPosition()
{
    PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());

    // The insertion indicator is preferred.  After all the user explicitly
    // used it to define the insertion position.
    if (mrSlideSorter.GetController().GetInsertionIndicatorHandler()->IsActive())
    {
        // Select the page before the insertion indicator.
        return mrSlideSorter.GetController().GetInsertionIndicatorHandler()->GetInsertionPageIndex()
            - 1;
    }

    // Is there a stored insertion position?
    else if (mrSlideSorter.GetController().GetSelectionManager()->GetInsertionPosition() >= 0)
    {
        return mrSlideSorter.GetController().GetSelectionManager()->GetInsertionPosition() - 1;
    }

    // Use the index of the last selected slide.
    else if (rSelector.GetSelectedPageCount() > 0)
    {
        for (int nIndex=rSelector.GetPageCount()-1; nIndex>=0; --nIndex)
            if (rSelector.IsPageSelected(nIndex))
                return nIndex;

        // We should never get here.
        OSL_ASSERT(false);
        return rSelector.GetPageCount() - 1;
    }

    // Select the last page when there is at least one page.
    else if (rSelector.GetPageCount() > 0)
    {
        return rSelector.GetPageCount() - 1;
    }

    // Hope for the best that CreateOrDuplicatePage() can cope with an empty
    // selection.
    else
    {
        // We should never get here because there has to be at least one page.
        OSL_ASSERT(false);
        return -1;
    }
}

void SlotManager::NotifyEditModeChange()
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

    // Get toggle state of the selected pages.
    while (rPageSet.HasMoreElements() && eState!=MIXED)
    {
        const bool bState = rPageSet.GetNextElement()->GetPage()->IsExcluded();
        switch (eState)
        {
            case UNDEFINED:
                // Use the first selected page to set the initial value.
                eState = bState ? EXCLUDED : INCLUDED;
                break;

            case EXCLUDED:
                // The pages before where all not part of the show,
                // this one is.
                if ( ! bState)
                    eState = MIXED;
                break;

            case INCLUDED:
                // The pages before where all part of the show,
                // this one is not.
                if (bState)
                    eState = MIXED;
                break;

            default:
                // No need to change anything.
                break;
        }
    }

    return eState;
}

} // end of anonymous namespace

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
