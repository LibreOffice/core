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

#include <config_features.h>
#include <config_fuzzers.h>
#include <config_wasm_strip.h>

#include <sal/config.h>

#include <hintids.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/imapdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <svl/visitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/clipfmtitem.hxx>
#include <svx/contdlg.hxx>
#include <vcl/graph.hxx>
#include <vcl/inputctx.hxx>
#include <svl/slstitm.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/flagsdef.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/objface.hxx>
#include <fmturl.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <swundo.hxx>
#include <fldbas.hxx>
#include <uitool.hxx>
#include <basesh.hxx>
#include <viewopt.hxx>
#include <fontcfg.hxx>
#include <fmtsrnd.hxx>
#include <fldmgr.hxx>
#include <frmmgr.hxx>
#include <tablemgr.hxx>
#include <mdiexp.hxx>
#include <swdtflvr.hxx>
#include <pagedesc.hxx>
#include <fmtcol.hxx>
#include <edtwin.hxx>
#include <tblafmt.hxx>
#include <swwait.hxx>
#include <cmdid.h>
#include <strings.hrc>
#include <unotxdoc.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swabstdlg.hxx>
#include <modcfg.hxx>
#include <svx/fmshell.hxx>
#include <SwRewriter.hxx>
#include <svx/galleryitem.hxx>
#include <sfx2/devtools/DevelopmentToolChildWindow.hxx>
#include <com/sun/star/gallery/GalleryItemType.hpp>
#include <memory>

#include <svx/unobrushitemhelper.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/lok.hxx>
#include <osl/diagnose.h>

#include <svx/svxdlg.hxx>

#include <shellres.hxx>
#include <UndoTable.hxx>

#include <ndtxt.hxx>
#include <UndoManager.hxx>

FlyMode SwBaseShell::eFrameMode = FLY_DRAG_END;

// These variables keep the state of Gallery (slot SID_GALLERY_BG_BRUSH)
// detected by GetGalleryState() for the subsequent ExecuteGallery() call.

static sal_uInt8 nParagraphPos;
static sal_uInt8 nGraphicPos;
static sal_uInt8 nOlePos;
static sal_uInt8 nFramePos;
static sal_uInt8 nTablePos;
static sal_uInt8 nTableRowPos;
static sal_uInt8 nTableCellPos;
static sal_uInt8 nPagePos;
static sal_uInt8 nHeaderPos;
static sal_uInt8 nFooterPos;

#define ShellClass_SwBaseShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>

#include <AccessibilityCheck.hxx>
#include <svx/AccessibilityCheckDialog.hxx>

namespace
{
    SvxContourDlg* GetContourDlg(SwView const &rView)
    {
        SfxChildWindow *pWnd = rView.GetViewFrame()->GetChildWindow(SvxContourDlgChildWindow::GetChildWindowId());
        return pWnd ? static_cast<SvxContourDlg*>(pWnd->GetController().get()) : nullptr;
    }

    SvxIMapDlg* GetIMapDlg(SwView const &rView)
    {
        SfxChildWindow* pWnd = rView.GetViewFrame()->GetChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
        return pWnd ? static_cast<SvxIMapDlg*>(pWnd->GetController().get()) : nullptr;
    }
}

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

SFX_IMPL_SUPERCLASS_INTERFACE(SwBaseShell, SfxShell)

void SwBaseShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxContourDlgChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(DevelopmentToolChildWindow::GetChildWindowId());
}

static void lcl_UpdateIMapDlg( SwWrtShell& rSh )
{
    Graphic aGrf( rSh.GetIMapGraphic() );
    GraphicType nGrfType = aGrf.GetType();
    void* pEditObj = GraphicType::NONE != nGrfType && GraphicType::Default != nGrfType
                        ? rSh.GetIMapInventor() : nullptr;
    TargetList aList;
    SfxFrame::GetDefaultTargetList(aList);

    SfxItemSetFixed<RES_URL, RES_URL> aSet( rSh.GetAttrPool() );
    rSh.GetFlyFrameAttr( aSet );
    const SwFormatURL &rURL = aSet.Get( RES_URL );
    SvxIMapDlgChildWindow::UpdateIMapDlg(
            aGrf, rURL.GetMap(), &aList, pEditObj );
}

static bool lcl_UpdateContourDlg( SwWrtShell &rSh, SelectionType nSel )
{
    Graphic aGraf( rSh.GetIMapGraphic() );
    GraphicType nGrfType = aGraf.GetType();
    bool bRet = GraphicType::NONE != nGrfType && GraphicType::Default != nGrfType;
    if( bRet )
    {
        OUString aGrfName;
        if ( nSel & SelectionType::Graphic )
            rSh.GetGrfNms( &aGrfName, nullptr );

        SvxContourDlg *pDlg = GetContourDlg(rSh.GetView());
        if (pDlg)
        {
            pDlg->Update(aGraf, !aGrfName.isEmpty(),
                         rSh.GetGraphicPolygon(), rSh.GetIMapInventor());
        }
    }
    return bRet;
}

void SwBaseShell::ExecDelete(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    SwEditWin& rTmpEditWin = GetView().GetEditWin();
    switch(rReq.GetSlot())
    {
        case SID_DELETE:
            if (rSh.GetViewOptions()->IsShowOutlineContentVisibilityButton())
            {
                // Disallow if the cursor is at the end of a paragraph and the document model
                // node at this position is an outline node with folded content or the next node
                // is an outline node with folded content.
                if (rSh.IsEndPara())
                {
                    SwNodeIndex aIdx(rSh.GetCursor()->GetNode());
                    if (aIdx.GetNode().IsTextNode())
                    {
                        bool bVisible = true;
                        aIdx.GetNode().GetTextNode()->GetAttrOutlineContentVisible(bVisible);
                        if (!bVisible)
                            break;
                        ++aIdx;
                        if (aIdx.GetNode().IsTextNode())
                        {
                            bVisible = true;
                            aIdx.GetNode().GetTextNode()->GetAttrOutlineContentVisible(bVisible);
                            if (!bVisible)
                                break;
                        }
                    }
                }
            }
            rSh.DelRight();
            break;

        case FN_BACKSPACE:
            if (rSh.GetViewOptions()->IsShowOutlineContentVisibilityButton())
            {
                // Disallow if the cursor is at the start of a paragraph and the document model
                // node at this position is an outline node with folded content or the previous
                // node is a content node without a layout frame.
                if (rSh.IsSttPara())
                {
                    SwNodeIndex aIdx(rSh.GetCursor()->GetNode());
                    if (aIdx.GetNode().IsTextNode())
                    {
                        bool bVisible = true;
                        aIdx.GetNode().GetTextNode()->GetAttrOutlineContentVisible(bVisible);
                        if (!bVisible)
                            break;
                        --aIdx;
                        if (aIdx.GetNode().IsContentNode() &&
                                !aIdx.GetNode().GetContentNode()->getLayoutFrame(rSh.GetLayout()))
                            break;
                    }
                }
            }
            if( rSh.IsNoNum() )
            {
                rSh.SttCursorMove();
                bool bLeft = rSh.Left( CRSR_SKIP_CHARS, true, 1, false  );
                if( bLeft )
                {
                    rSh.DelLeft();
                }
                else
                    // JP 15.07.96: If it no longer goes forward, cancel
                    //              the numbering. For example at the beginning
                    //              of a doc, frame, table or an area.
                    rSh.DelNumRules();

                rSh.EndCursorMove();
                break;
            }

            [[fallthrough]]; // otherwise call DelLeft
        case FN_SHIFT_BACKSPACE:
            rSh.DelLeft();
            break;
        default:
            OSL_FAIL("wrong Dispatcher");
            return;
    }
    rReq.Done();

    //#i42732# - notify the edit window that from now on we do not use the input language
    rTmpEditWin.SetUseInputLanguage( false );
}

void SwBaseShell::ExecClpbrd(SfxRequest &rReq)
{
    // Attention: At risk of suicide!
    // After paste, paste special the shell can be destroy.

    SwWrtShell &rSh = GetShell();
    sal_uInt16 nId = rReq.GetSlot();
    bool bIgnore = false;
    PasteTableType ePasteTable = PasteTableType::PASTE_DEFAULT;

    switch( nId )
    {
        case SID_CUT:
        case SID_COPY:
            m_rView.GetEditWin().FlushInBuffer();
            if ( rSh.HasSelection() )
            {
                rtl::Reference<SwTransferable> pTransfer = new SwTransferable( rSh );

                if ( nId == SID_CUT && FlyProtectFlags::NONE == rSh.IsSelObjProtected(FlyProtectFlags::Content|FlyProtectFlags::Parent) )
                    pTransfer->Cut();
                else
                {
                    const bool bLockedView = rSh.IsViewLocked();
                    rSh.LockView( true );    //lock visible section
                    pTransfer->Copy();
                    rSh.LockView( bLockedView );
                }
                break;
            }
            return;

        case FN_PASTE_NESTED_TABLE:
        case FN_TABLE_PASTE_ROW_BEFORE:
        case FN_TABLE_PASTE_COL_BEFORE:
            switch ( nId )
            {
                case FN_PASTE_NESTED_TABLE:
                     ePasteTable = PasteTableType::PASTE_TABLE;
                     break;
                case FN_TABLE_PASTE_ROW_BEFORE:
                     ePasteTable = PasteTableType::PASTE_ROW;
                     break;
                case FN_TABLE_PASTE_COL_BEFORE:
                     ePasteTable = PasteTableType::PASTE_COLUMN;
                     break;
                default:
                    ;
            }
            [[fallthrough]];
        case SID_PASTE:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard( &rSh.GetView().GetEditWin() ) );
                if( aDataHelper.GetXTransferable().is()
                    && SwTransferable::IsPaste( rSh, aDataHelper ) )
                {
                    // Temporary variables, because the shell could already be
                    // destroyed after the paste.
                    SwView* pView = &m_rView;

                    RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA;
                    const SfxUInt16Item* pAnchorType = rReq.GetArg<SfxUInt16Item>(FN_PARAM_1);
                    if (pAnchorType)
                        nAnchorType = static_cast<RndStdIds>(pAnchorType->GetValue());
                    bool bIgnoreComments = false;
                    const SfxBoolItem* pIgnoreComments = rReq.GetArg<SfxBoolItem>(FN_PARAM_2);
                    if (pIgnoreComments)
                        bIgnoreComments = pIgnoreComments->GetValue();
                    SwTransferable::Paste(rSh, aDataHelper, nAnchorType, bIgnoreComments, ePasteTable);

                    if( rSh.IsFrameSelected() || rSh.IsObjSelected() )
                        rSh.EnterSelFrameMode();
                    pView->AttrChangedNotify(nullptr);

                    // Fold pasted outlines that have outline content visible attribute false
                    MakeAllOutlineContentTemporarilyVisible a(rSh.GetDoc());
                }
                else
                    return;
            }
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                const SfxPoolItem* pFormat;
                if( pArgs && SfxItemState::SET == pArgs->GetItemState( nId, false, &pFormat ) )
                {
                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                            &rSh.GetView().GetEditWin()) );
                    if( aDataHelper.GetXTransferable().is()
                        /*&& SwTransferable::IsPaste( rSh, aDataHelper )*/ )
                    {
                        // Temporary variables, because the shell could already be
                        // destroyed after the paste.
                        SwView* pView = &m_rView;

                        SwTransferable::PasteFormat( rSh, aDataHelper,
                                        static_cast<SotClipboardFormatId>(static_cast<const SfxUInt32Item*>(pFormat)->GetValue()) );

                        //Done() has to be called before the shell has been removed
                        rReq.Done();
                        bIgnore = true;
                        if( rSh.IsFrameSelected() || rSh.IsObjSelected())
                            rSh.EnterSelFrameMode();
                        pView->AttrChangedNotify(nullptr);

                        // Fold pasted outlines that have outline content visible attribute false
                        MakeAllOutlineContentTemporarilyVisible a(rSh.GetDoc());
                    }
                }
            }
            break;

        case SID_PASTE_UNFORMATTED:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard( &rSh.GetView().GetEditWin()) );
                if( aDataHelper.GetXTransferable().is()
                    && SwTransferable::IsPaste( rSh, aDataHelper ) )
                {
                    // Temporary variables, because the shell could already be
                    // destroyed after the paste.
                    SwView* pView = &m_rView;
                    rReq.Ignore();
                    bIgnore = true;
                    if(SwTransferable::PasteUnformatted( rSh, aDataHelper ))
                    {
                        SfxViewFrame* pViewFrame = pView->GetViewFrame();
                        uno::Reference< frame::XDispatchRecorder > xRecorder =
                            pViewFrame->GetBindings().GetRecorder();
                        if(xRecorder.is()) {
                            SfxRequest aReq( pViewFrame, SID_CLIPBOARD_FORMAT_ITEMS );
                            aReq.AppendItem( SfxUInt32Item( SID_CLIPBOARD_FORMAT_ITEMS, static_cast<sal_uInt32>(SotClipboardFormatId::STRING) ) );
                            aReq.Done();
                        }
                    }

                    if (rSh.IsFrameSelected() || rSh.IsObjSelected())
                        rSh.EnterSelFrameMode();
                    pView->AttrChangedNotify(nullptr);

                    // Fold pasted outlines that have outline content visible attribute false
                    MakeAllOutlineContentTemporarilyVisible a(rSh.GetDoc());
                }
                else
                    return;
            }
            break;

        case SID_PASTE_SPECIAL:
            {
                std::shared_ptr<TransferableDataHelper> aDataHelper =
                    std::make_shared<TransferableDataHelper>(TransferableDataHelper::CreateFromSystemClipboard( &rSh.GetView().GetEditWin()));

                if( aDataHelper->GetXTransferable().is()
                    && SwTransferable::IsPaste( rSh, *aDataHelper )
                    && !rSh.CursorInsideInputField() )
                {
                    rReq.Ignore();
                    bIgnore = true;

                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    VclPtr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog( rReq.GetFrameWeld() ));

                    // Prepare the dialog
                    SwTransferable::PrePasteSpecial(rSh, *aDataHelper, pDlg);
                    pDlg->PreGetFormat(*aDataHelper);


                    pDlg->StartExecuteAsync([aDataHelper, pDlg, &rSh, this](sal_Int32 nResult){
                    if (nResult == RET_OK)
                    {
                        // Temporary variables, because the shell could already be
                        // destroyed after the paste.
                        SwView* pView = &m_rView;
                        bool bRet = false;
                        SotClipboardFormatId nFormatId = pDlg->GetFormatOnly();

                        if( nFormatId != SotClipboardFormatId::NONE )
                            bRet = SwTransferable::PasteFormat( rSh, *aDataHelper, nFormatId );

                        if (bRet)
                        {
                            SfxViewFrame* pViewFrame = pView->GetViewFrame();
                            uno::Reference< frame::XDispatchRecorder > xRecorder =
                                    pViewFrame->GetBindings().GetRecorder();
                            if(xRecorder.is()) {
                                SfxRequest aReq( pViewFrame, SID_CLIPBOARD_FORMAT_ITEMS );
                                aReq.AppendItem( SfxUInt32Item( SID_CLIPBOARD_FORMAT_ITEMS, static_cast<sal_uInt32>(nFormatId) ) );
                                aReq.Done();
                            }
                        }

                        if (rSh.IsFrameSelected() || rSh.IsObjSelected())
                            rSh.EnterSelFrameMode();
                        pView->AttrChangedNotify(nullptr);

                        // Fold pasted outlines that have outline content visible attribute false
                        MakeAllOutlineContentTemporarilyVisible a(rSh.GetDoc());
                    }

                    pDlg->disposeOnce();

                    });
                }
                else
                    return;
            }
            break;

        default:
            OSL_FAIL("wrong Dispatcher");
            return;
    }
    if(!bIgnore)
        rReq.Done();
}

// ClipBoard state

void SwBaseShell::StateClpbrd(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter(rSet);

    const bool bCopy = rSh.HasSelection();

    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
        case SID_CUT:
            if( FlyProtectFlags::NONE != rSh.IsSelObjProtected(FlyProtectFlags::Content|FlyProtectFlags::Parent ) )
            {
                rSet.DisableItem( nWhich );
                break;
            }
            [[fallthrough]];
        case SID_COPY:
            if( !bCopy || GetObjectShell()->isContentExtractionLocked())
                rSet.DisableItem( nWhich );
            break;

        case FN_PASTE_NESTED_TABLE:
        case FN_TABLE_PASTE_ROW_BEFORE:
        case FN_TABLE_PASTE_COL_BEFORE:
            if( !rSh.IsCursorInTable()
                || !GetView().IsPasteSpecialAllowed()
                || rSh.CursorInsideInputField()
                // disable if not a native Writer table and not a spreadsheet format
                || !GetView().IsPasteSpreadsheet(rSh.GetTableCopied()) )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case SID_PASTE:
            if( !GetView().IsPasteAllowed() )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case SID_PASTE_SPECIAL:
            if( !GetView().IsPasteSpecialAllowed()
                || rSh.CursorInsideInputField() )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case SID_PASTE_UNFORMATTED:
            if( !GetView().IsPasteSpecialAllowed() )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard(
                                            &rSh.GetView().GetEditWin()) );

                SvxClipboardFormatItem aFormatItem( nWhich );
                SwTransferable::FillClipFormatItem( rSh, aDataHelper, aFormatItem );
                rSet.Put( aFormatItem );
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

// Perform undo

void SwBaseShell::ExecUndo(SfxRequest &rReq)
{
    MakeAllOutlineContentTemporarilyVisible a(GetShell().GetDoc());

    SwWrtShell &rWrtShell = GetShell();

    SwUndoId nUndoId(SwUndoId::EMPTY);
    sal_uInt16 nId = rReq.GetSlot(), nCnt = 1;
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( nId, false, &pItem ))
        nCnt = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

    // Repair mode: allow undo/redo of all undo actions, even if access would
    // be limited based on the view shell ID.
    bool bRepair = false;
    if (pArgs && pArgs->GetItemState(SID_REPAIRPACKAGE, false, &pItem) == SfxItemState::SET)
        bRepair = static_cast<const SfxBoolItem*>(pItem)->GetValue();

    // #i106349#: save pointer: undo/redo may delete the shell, i.e., this!
    SfxViewFrame *const pViewFrame( GetView().GetViewFrame() );

    IDocumentUndoRedo& rUndoRedo = rWrtShell.GetIDocumentUndoRedo();
    bool bWasRepair = rUndoRedo.DoesRepair();
    rUndoRedo.DoRepair(bRepair);
    comphelper::ScopeGuard aGuard([&rUndoRedo, bWasRepair]()
    {
        rUndoRedo.DoRepair(bWasRepair);
    });

    switch( nId )
    {
        case SID_UNDO:
            if (rUndoRedo.GetLastUndoInfo(nullptr, &nUndoId, &rWrtShell.GetView()))
            {
                for (SwViewShell& rShell : rWrtShell.GetRingContainer())
                    rShell.LockPaint();

                sal_uInt16 nUndoOffset = 0;
                if (comphelper::LibreOfficeKit::isActive() && !bRepair && nCnt == 1)
                {
                    sw::UndoManager& rManager = rWrtShell.GetDoc()->GetUndoManager();
                    const SfxUndoAction* pAction = rManager.GetUndoAction();
                    SwView& rView = rWrtShell.GetView();
                    ViewShellId nViewShellId = rView.GetViewShellId();
                    sal_uInt16 nOffset = 0;
                    if (pAction->GetViewShellId() != nViewShellId
                        && rManager.IsViewUndoActionIndependent(&rView, nOffset))
                    {
                        // Execute the undo with an offset: don't undo the top action, but an
                        // earlier one, since it's independent and that belongs to our view.
                        nUndoOffset += nOffset;
                    }
                }

                rWrtShell.Do(SwWrtShell::UNDO, nCnt, nUndoOffset);

                for (SwViewShell& rShell : rWrtShell.GetRingContainer())
                    rShell.UnlockPaint();

                // tdf#141613 FIXME: Disable redoing header/footer changes for now.
                // The proper solution would be to write a SwUndoHeaderFooter class
                // to represent the addition of a header or footer to the current page.
                if (nUndoId == SwUndoId::HEADER_FOOTER)
                    rUndoRedo.ClearRedo();
            }
            break;

        case SID_REDO:
            if (rUndoRedo.GetFirstRedoInfo(nullptr, &nUndoId, &rWrtShell.GetView()))
            {
                for (SwViewShell& rShell : rWrtShell.GetRingContainer())
                    rShell.LockPaint();
                rWrtShell.Do( SwWrtShell::REDO, nCnt );
                for (SwViewShell& rShell : rWrtShell.GetRingContainer())
                    rShell.UnlockPaint();
            }
            break;

        case SID_REPEAT:
            rWrtShell.Do( SwWrtShell::REPEAT );
            break;
        default:
            OSL_FAIL("wrong Dispatcher");
    }

    if (nUndoId == SwUndoId::CONFLICT)
    {
        rReq.SetReturnValue( SfxUInt32Item(nId, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)) );
    }
    else if (nUndoId == SwUndoId::INSFMTATTR)
    {
        rWrtShell.GetDoc()->GetDocShell()->GetStyleSheetPool()->Broadcast(SfxHint(SfxHintId::StyleSheetModified));
    }

    if (pViewFrame) { pViewFrame->GetBindings().InvalidateAll(false); }
}

// State of undo

void SwBaseShell::StateUndo(SfxItemSet &rSet)
{
    SwUndoId nUndoId(SwUndoId::EMPTY);
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_UNDO:
            {
                if (rSh.GetLastUndoInfo(nullptr, &nUndoId, &rSh.GetView()))
                {
                    rSet.Put( SfxStringItem(nWhich,
                        rSh.GetDoString(SwWrtShell::UNDO)));
                }
                else if (nUndoId == SwUndoId::CONFLICT)
                {
                    rSet.Put( SfxUInt32Item(nWhich, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)) );
                }
                else
                    rSet.DisableItem(nWhich);

                break;
            }
            case SID_REDO:
            {
                if (rSh.GetFirstRedoInfo(nullptr, &nUndoId, &rSh.GetView()))
                {
                    rSet.Put(SfxStringItem(nWhich,
                        rSh.GetDoString(SwWrtShell::REDO)));
                }
                else if (nUndoId == SwUndoId::CONFLICT)
                {
                     rSet.Put( SfxInt32Item(nWhich, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)) );
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }
            case SID_REPEAT:
            {   // Repeat is only possible if no REDO is possible - UI-Restriction
                if ((!rSh.GetFirstRedoInfo(nullptr, nullptr)) &&
                    !rSh.IsSelFrameMode() &&
                    (SwUndoId::EMPTY != rSh.GetRepeatInfo(nullptr)))
                {
                    rSet.Put(SfxStringItem(nWhich, rSh.GetRepeatString()));
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }

            case SID_GETUNDOSTRINGS:
                if (rSh.GetLastUndoInfo(nullptr, nullptr))
                {
                    SfxStringListItem aStrLst( nWhich );
                    rSh.GetDoStrings( SwWrtShell::UNDO, aStrLst );
                    rSet.Put( aStrLst );
                }
                else
                    rSet.DisableItem( nWhich );
                break;

            case SID_GETREDOSTRINGS:
                if (rSh.GetFirstRedoInfo(nullptr, nullptr))
                {
                    SfxStringListItem aStrLst( nWhich );
                    rSh.GetDoStrings( SwWrtShell::REDO, aStrLst );
                    rSet.Put( aStrLst );
                }
                else
                    rSet.DisableItem( nWhich );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

// Evaluate respectively dispatching the slot Id

void SwBaseShell::Execute(SfxRequest &rReq)
{
    const SfxPoolItem *pItem;
    SwWrtShell &rSh = GetShell();
    const SfxItemSet* pArgs = rReq.GetArgs();
    bool bMore = false;

    sal_uInt16 nSlot = rReq.GetSlot();
    switch(nSlot)
    {
        case FN_REPAGINATE:
            {
                Reference < XModel > xModel = GetView().GetDocShell()->GetModel();
                auto pDoc = comphelper::getFromUnoTunnel<SwXTextDocument>(xModel);
                pDoc->NotifyRefreshListeners();
                rSh.CalcLayout();
            }
            break;
        case FN_UPDATE_FIELDS:
            {
                rSh.UpdateDocStat();
                rSh.EndAllTableBoxEdit();
                rSh.SwViewShell::UpdateFields(true);

                if( rSh.IsCursorInTable() )
                {
                    if( !rSh.IsTableComplexForChart() )
                        SwTableFUNC( &rSh ).UpdateChart();
                    rSh.ClearTableBoxContent();
                    rSh.SaveTableBoxContent();
                }
            }
            break;
        case FN_UPDATE_CHARTS:
            {
                SwWait aWait( *m_rView.GetDocShell(), true );
                rSh.UpdateAllCharts();
            }
            break;

        case FN_UPDATE_ALL:
            {
                comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer
                    = GetObjectShell()->getEmbeddedObjectContainer();
                rEmbeddedObjectContainer.setUserAllowsLinkUpdate(true);

                SwView&  rTempView = GetView();
                rSh.EnterStdMode();
                if( !rSh.GetLinkManager().GetLinks().empty() )
                {
                    rSh.StartAllAction();
                    rSh.GetLinkManager().UpdateAllLinks( false, true, nullptr );
                    rSh.EndAllAction();
                }
                SfxDispatcher &rDis = *rTempView.GetViewFrame()->GetDispatcher();
                rDis.Execute( FN_UPDATE_FIELDS );
                rDis.Execute( FN_UPDATE_TOX );
                rDis.Execute( FN_UPDATE_CHARTS );
                rSh.Reformat();
                rSh.UpdateOleObjectPreviews();
            }
            break;

        case FN_UPDATE_INPUTFIELDS:
            rSh.UpdateInputFields();
            break;

        case FN_PREV_BOOKMARK:
            rReq.SetReturnValue(SfxBoolItem( nSlot, rSh.GoPrevBookmark()));
            break;
        case FN_NEXT_BOOKMARK:
            rReq.SetReturnValue(SfxBoolItem( nSlot, rSh.GoNextBookmark()));
            break;

        case FN_GOTO_NEXT_MARK:
        case FN_GOTO_PREV_MARK:
        {
            SwFieldMgr aFieldMgr;
            SwFieldType* pFieldType = aFieldMgr.GetFieldType(SwFieldIds::JumpEdit);

            if (pFieldType)
            {
                if (rSh.IsSelFrameMode())
                {
                    rSh.UnSelectFrame();
                    rSh.LeaveSelFrameMode();
                }

                if (rSh.HasMark())
                {
                    SwMvContext aMvContext(&rSh);
                    if (rSh.IsCursorPtAtEnd())
                        rSh.SwapPam();
                    rSh.ClearMark();
                    rSh.EndSelect();
                }
                bool bRet = rSh.MoveFieldType( pFieldType, nSlot == FN_GOTO_NEXT_MARK );
                SwField* pCurField = bRet ? rSh.GetCurField() : nullptr;
                if (pCurField)
                    rSh.ClickToField(*pCurField, /*bExecHyperlinks=*/false);
                rReq.SetReturnValue(SfxBoolItem( nSlot, bRet));
            }
        }
        break;

        case FN_START_DOC_DIRECT:
        case FN_END_DOC_DIRECT:
        {
            if (rSh.IsSelFrameMode())
            {
                rSh.UnSelectFrame();
                rSh.LeaveSelFrameMode();
            }
            rSh.EnterStdMode();
            nSlot == FN_START_DOC_DIRECT ?
                rSh.SttEndDoc(true) :
                    rSh.SttEndDoc(false);
        }
        break;
        case FN_GOTO_PREV_OBJ:
        case FN_GOTO_NEXT_OBJ:
        {
                bool bSuccess = rSh.GotoObj( nSlot == FN_GOTO_NEXT_OBJ );
                rReq.SetReturnValue(SfxBoolItem(nSlot, bSuccess));
                if (bSuccess && !rSh.IsSelFrameMode())
                {
                    rSh.HideCursor();
                    rSh.EnterSelFrameMode();
                    GetView().AttrChangedNotify(nullptr);
                }
        }
        break;
        case SID_GALLERY_FORMATS:
        {
            const SvxGalleryItem* pGalleryItem = SfxItemSet::GetItem<SvxGalleryItem>(pArgs, SID_GALLERY_FORMATS, false);
            if ( !pGalleryItem )
                break;

            const SelectionType nSelType = rSh.GetSelectionType();
            sal_Int8 nGalleryItemType( pGalleryItem->GetType() );

            if ( (!rSh.IsSelFrameMode() || nSelType & SelectionType::Graphic) &&
                nGalleryItemType == css::gallery::GalleryItemType::GRAPHIC )
            {
                SwWait aWait( *m_rView.GetDocShell(), true );

                OUString aGrfName, aFltName;
                const Graphic aGrf( pGalleryItem->GetGraphic() );

                if ( nSelType & SelectionType::Graphic )
                    rSh.ReRead( aGrfName, aFltName, &aGrf );
                else
                    rSh.Insert( aGrfName, aFltName, aGrf );

                GetView().GetEditWin().GrabFocus();
            }
            else if(!rSh.IsSelFrameMode() &&
                nGalleryItemType == css::gallery::GalleryItemType::MEDIA  )
            {
                const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, pGalleryItem->GetURL() );
                GetView().GetViewFrame()->GetDispatcher()->ExecuteList(
                        SID_INSERT_AVMEDIA, SfxCallMode::SYNCHRON,
                        { &aMediaURLItem });
            }
        }
        break;
        case FN_PAGE_STYLE_SET_COLS:
        {
            if (pArgs)
            {
                // Determine the current PageDescriptor and fill the set with that.
                const size_t nCurIdx = rSh.GetCurPageDesc();
                SwPageDesc aPageDesc(rSh.GetPageDesc(nCurIdx));

                SwFrameFormat &rFormat = aPageDesc.GetMaster();

                SwFormatCol aFormatCol = rFormat.GetCol();

                sal_uInt16 nCount;
                if(SfxItemState::SET == pArgs->GetItemState(nSlot))
                    nCount = static_cast<const SfxUInt16Item &>(pArgs->Get(nSlot)).GetValue();
                else
                    nCount = pArgs->Get(SID_ATTR_COLUMNS).GetValue();
                sal_uInt16 nGutterWidth = DEF_GUTTER_WIDTH;

                aFormatCol.Init(nCount ? nCount : 1, nGutterWidth, USHRT_MAX);
                aFormatCol.SetWishWidth(USHRT_MAX);
                aFormatCol.SetGutterWidth(nGutterWidth, USHRT_MAX);

                rFormat.SetFormatAttr(aFormatCol);

                rSh.ChgPageDesc(nCurIdx, aPageDesc);
            }
            else
                GetView().GetViewFrame()->GetDispatcher()->Execute(FN_FORMAT_PAGE_COLUMN_DLG);
        }
        break;
        case FN_CONVERT_TABLE_TO_TEXT:
        case FN_CONVERT_TEXT_TO_TABLE:
        case FN_CONVERT_TEXT_TABLE:
        {
            sal_Unicode cDelim = 0;
            bool bToTable = false;
            if( nSlot == FN_CONVERT_TEXT_TO_TABLE ||
                ( nSlot == FN_CONVERT_TEXT_TABLE && nullptr == rSh.GetTableFormat() ))
                bToTable = true;
            SwInsertTableOptions aInsTableOpts( SwInsertTableFlags::All, 1 );
            SwTableAutoFormat const* pTAFormat = nullptr;
            std::unique_ptr<SwTableAutoFormatTable> pAutoFormatTable;
            bool bDeleteFormat = true;
            if(pArgs && SfxItemState::SET == pArgs->GetItemState( FN_PARAM_1, true, &pItem))
            {
                aInsTableOpts.mnInsMode = SwInsertTableFlags::NONE;
                // Delimiter
                OUString sDelim = static_cast< const SfxStringItem* >(pItem)->GetValue();
                if(!sDelim.isEmpty())
                    cDelim = sDelim[0];
                // AutoFormat
                if(SfxItemState::SET == pArgs->GetItemState( FN_PARAM_2, true, &pItem))
                {
                    OUString sAutoFormat = static_cast< const SfxStringItem* >(pItem)->GetValue();

                    pAutoFormatTable.reset(new SwTableAutoFormatTable);
                    pAutoFormatTable->Load();

                    for( sal_uInt16 i = 0, nCount = pAutoFormatTable->size(); i < nCount; i++ )
                    {
                        SwTableAutoFormat const*const pFormat = &(*pAutoFormatTable)[ i ];
                        if( pFormat->GetName() == sAutoFormat )
                        {
                            pTAFormat = pFormat;
                            bDeleteFormat = false;
                            break;
                        }
                    }
                }
                //WithHeader
                if(SfxItemState::SET == pArgs->GetItemState( FN_PARAM_3, true, &pItem) &&
                            static_cast< const SfxBoolItem* >(pItem)->GetValue())
                    aInsTableOpts.mnInsMode |= SwInsertTableFlags::Headline;
                // RepeatHeaderLines
                if(SfxItemState::SET == pArgs->GetItemState( FN_PARAM_4, true, &pItem))
                   aInsTableOpts.mnRowsToRepeat =
                            o3tl::narrowing<sal_uInt16>(static_cast< const SfxInt16Item* >(pItem)->GetValue());
                //WithBorder
                if(SfxItemState::SET == pArgs->GetItemState( FN_PARAM_5, true, &pItem) &&
                    static_cast< const SfxBoolItem* >(pItem)->GetValue())
                    aInsTableOpts.mnInsMode |= SwInsertTableFlags::DefaultBorder;
                //DontSplitTable
                if(SfxItemState::SET == pArgs->GetItemState( FN_PARAM_6, true, &pItem) &&
                    !static_cast< const SfxBoolItem* >(pItem)->GetValue() )
                    aInsTableOpts.mnInsMode |= SwInsertTableFlags::SplitLayout;
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractSwConvertTableDlg> pDlg(pFact->CreateSwConvertTableDlg(GetView(), bToTable));
                if( RET_OK == pDlg->Execute() )
                {
                    pDlg->GetValues( cDelim, aInsTableOpts, pTAFormat );

                }
            }

            if( cDelim )
            {
                //Shell change!
                SwView& rSaveView = m_rView;
                bool bInserted = false;
                //recording:
                SfxViewFrame* pViewFrame = GetView().GetViewFrame();
                if( SfxRequest::HasMacroRecorder(pViewFrame) )
                {
                    SfxRequest aReq( pViewFrame, nSlot);
                    aReq.AppendItem( SfxStringItem( FN_PARAM_1, OUString(cDelim) ));
                    if(bToTable)
                    {
                        if(pTAFormat)
                            aReq.AppendItem( SfxStringItem( FN_PARAM_2, pTAFormat->GetName()));
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_3, bool(aInsTableOpts.mnInsMode & SwInsertTableFlags::Headline)));
                        aReq.AppendItem( SfxInt16Item( FN_PARAM_4, static_cast<short>(aInsTableOpts.mnRowsToRepeat) ));
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_5, bool(aInsTableOpts.mnInsMode & SwInsertTableFlags::DefaultBorder) ));
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_6, !(aInsTableOpts.mnInsMode & SwInsertTableFlags::SplitLayout)));
                    }
                    aReq.Done();
                }

                if( !bToTable )
                    rSh.TableToText( cDelim );
                else
                {
                    bInserted = rSh.TextToTable( aInsTableOpts, cDelim, pTAFormat );
                }
                rSh.EnterStdMode();

                if( bInserted )
                    rSaveView.AutoCaption( TABLE_CAP );
            }
            if(bDeleteFormat)
                delete pTAFormat;
        }
        break;
        case SID_STYLE_WATERCAN:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        case SID_STYLE_NEW_BY_EXAMPLE:
        case SID_STYLE_APPLY:
        {
            ShellMode eMode = GetView().GetShellMode();
            if ( ShellMode::Draw != eMode &&
                 ShellMode::DrawForm != eMode &&
                 ShellMode::DrawText != eMode &&
                 ShellMode::Bezier != eMode )
            {
                // oj #107754#
                if ( SID_STYLE_WATERCAN == nSlot )
                {
                    const bool bLockedView = rSh.IsViewLocked();
                    rSh.LockView( true );    //lock visible section

                    GetView().GetDocShell()->ExecStyleSheet(rReq);

                    rSh.LockView( bLockedView );
                }
                else
                // Will be recorded from the DocShell
                    GetView().GetDocShell()->ExecStyleSheet(rReq);
            }
        }
        break;
        case SID_CLASSIFICATION_APPLY:
        {
            GetView().GetDocShell()->Execute(rReq);
        }
        break;
        case SID_CLASSIFICATION_DIALOG:
        {
            GetView().GetDocShell()->Execute(rReq);
        }
        break;
        case SID_PARAGRAPH_SIGN_CLASSIFY_DLG:
        {
            GetView().GetDocShell()->Execute(rReq);
        }
        break;
        case SID_WATERMARK:
        {
            GetView().GetDocShell()->Execute(rReq);
        }
        break;
        case FN_ESCAPE:
            GetView().ExecuteSlot(rReq);
        break;
        case SID_IMAP:
        {
            sal_uInt16      nId = SvxIMapDlgChildWindow::GetChildWindowId();

            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            pVFrame->ToggleChildWindow( nId );
            pVFrame->GetBindings().Invalidate( SID_IMAP );

            if ( pVFrame->HasChildWindow( nId ) && rSh.IsFrameSelected() )
                lcl_UpdateIMapDlg( rSh );
        }
        break;
        case SID_IMAP_EXEC:
        {
            SvxIMapDlg* pDlg = GetIMapDlg(GetView());

            // Check, if the allocation is useful or allowed at all.
            if ( rSh.IsFrameSelected() &&
                 pDlg->GetEditingObject() == rSh.GetIMapInventor() )
            {
                    SfxItemSetFixed<RES_URL, RES_URL> aSet( rSh.GetAttrPool() );
                    rSh.GetFlyFrameAttr( aSet );
                    SwFormatURL aURL( aSet.Get( RES_URL ) );
                    aURL.SetMap( &pDlg->GetImageMap() );
                    aSet.Put( aURL );
                    rSh.SetFlyFrameAttr( aSet );
            }
        }
        break;
        case FN_SET_TRACKED_CHANGES_IN_TEXT:
        case FN_SET_TRACKED_DELETIONS_IN_MARGIN:
        case FN_SET_TRACKED_INSERTIONS_IN_MARGIN:
        {
            SwViewOption aViewOption = *rSh.GetViewOptions();
            bool bAllInText = FN_SET_TRACKED_CHANGES_IN_TEXT == nSlot;
            aViewOption.SetShowChangesInMargin( !bAllInText );
            if ( !bAllInText )
                aViewOption.SetShowChangesInMargin2( FN_SET_TRACKED_INSERTIONS_IN_MARGIN == nSlot );
            rSh.ApplyViewOptions( aViewOption );

            // tdf#140982 restore annotation ranges stored in temporary bookmarks
            // (only remove temporary bookmarks during file saving to avoid possible
            // conflict with lazy deletion of the bookmarks of the moved tracked deletions)
            if ( bAllInText || FN_SET_TRACKED_INSERTIONS_IN_MARGIN == nSlot )
                rSh.GetDoc()->getIDocumentMarkAccess()->restoreAnnotationMarks(false);
        }
        break;
        case SID_CONTOUR_DLG:
        {
            sal_uInt16 nId = SvxContourDlgChildWindow::GetChildWindowId();

            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            pVFrame->ToggleChildWindow( nId );
            pVFrame->GetBindings().Invalidate( SID_CONTOUR_DLG );

            SelectionType nSel = rSh.GetSelectionType();
            if ( pVFrame->HasChildWindow( nId ) &&
                 (nSel & (SelectionType::Graphic|SelectionType::Ole)) )
            {
                lcl_UpdateContourDlg( rSh, nSel );
            }
        }
        break;
        case SID_CONTOUR_EXEC:
        {
            SvxContourDlg *pDlg = GetContourDlg(GetView());
            // Check, if the allocation is useful or allowed at all.
            SelectionType nSel = rSh.GetSelectionType();
            if ( nSel & (SelectionType::Graphic|SelectionType::Ole) )
            {
                if (pDlg && pDlg->GetEditingObject() == rSh.GetIMapInventor())
                {
                    rSh.StartAction();
                    SfxItemSetFixed<RES_SURROUND, RES_SURROUND> aSet( rSh.GetAttrPool() );
                    rSh.GetFlyFrameAttr( aSet );
                    SwFormatSurround aSur( aSet.Get( RES_SURROUND ) );
                    if ( !aSur.IsContour() )
                    {
                        aSur.SetContour( true );
                        if ( aSur.GetSurround() == css::text::WrapTextMode_NONE )
                            aSur.SetSurround( css::text::WrapTextMode_PARALLEL );
                        aSet.Put( aSur );
                        rSh.SetFlyFrameAttr( aSet );
                    }
                    const tools::PolyPolygon aPoly( pDlg->GetPolyPolygon() );
                    rSh.SetGraphicPolygon( &aPoly );
                    if ( pDlg->IsGraphicChanged() )
                        rSh.ReRead( OUString(), OUString(), &pDlg->GetGraphic());
                    rSh.EndAction();
                }
            }
        }
        break;
        case FN_FRAME_TO_ANCHOR:
        {
            rSh.GotoFlyAnchor();
            rSh.EnterStdMode();
            rSh.CallChgLnk();
        }
        break;
        case FN_TOOL_ANCHOR_PAGE:
        case FN_TOOL_ANCHOR_PARAGRAPH:
        case FN_TOOL_ANCHOR_CHAR:
        case FN_TOOL_ANCHOR_AT_CHAR:
        case FN_TOOL_ANCHOR_FRAME:
        {
            RndStdIds eSet = nSlot == FN_TOOL_ANCHOR_PAGE
                                ? RndStdIds::FLY_AT_PAGE
                                : nSlot == FN_TOOL_ANCHOR_PARAGRAPH
                                    ? RndStdIds::FLY_AT_PARA
                                    : nSlot == FN_TOOL_ANCHOR_FRAME
                                        ? RndStdIds::FLY_AT_FLY
                                        : nSlot == FN_TOOL_ANCHOR_CHAR
                                            ? RndStdIds::FLY_AS_CHAR
                                            : RndStdIds::FLY_AT_CHAR;
            rSh.StartUndo();
            if (rSh.IsObjSelected())
                rSh.ChgAnchor(eSet);
            else if (rSh.IsFrameSelected())
            {
                SwFormatAnchor aAnc(eSet, rSh.GetPhyPageNum());
                SfxItemSet aSet(SwFEShell::makeItemSetFromFormatAnchor(GetPool(), aAnc));
                rSh.SetFlyFrameAttr(aSet);
            }
            // if new anchor is 'as char' and it is a Math object and the usual
            // pre-conditions are met then align the formula to the baseline of the text
            const uno::Reference < embed::XEmbeddedObject > xObj( rSh.GetOleRef() );
            const bool bDoMathBaselineAlignment = xObj.is() && SotExchange::IsMath( xObj->getClassID() )
                    && RndStdIds::FLY_AS_CHAR == eSet && rSh.GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT );
            if (bDoMathBaselineAlignment)
                rSh.AlignFormulaToBaseline( xObj );

            sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
            if( nHtmlMode )
            {
                SfxItemSetFixed<RES_SURROUND, RES_HORI_ORIENT> aSet(GetPool());
                rSh.GetFlyFrameAttr(aSet);

                const SwFormatSurround& rSurround = aSet.Get(RES_SURROUND);
                const SwFormatVertOrient& rVert = aSet.Get(RES_VERT_ORIENT);
                const SwFormatHoriOrient& rHori = aSet.Get(RES_HORI_ORIENT);
                sal_Int16 eVOrient = rVert.GetVertOrient();
                sal_Int16 eHOrient = rHori.GetHoriOrient();
                css::text::WrapTextMode eSurround = rSurround.GetSurround();

                switch( eSet )
                {
                case RndStdIds::FLY_AT_FLY:
                case RndStdIds::FLY_AT_PAGE:
                    //Wrap through, left or from left, top, from top
                    if(eSurround != css::text::WrapTextMode_THROUGH)
                        aSet.Put(SwFormatSurround(css::text::WrapTextMode_THROUGH));

                    if( eVOrient != text::VertOrientation::TOP && eVOrient != text::VertOrientation::NONE)
                        aSet.Put(SwFormatVertOrient(0, text::VertOrientation::TOP));

                    if (eHOrient != text::HoriOrientation::NONE && eHOrient != text::HoriOrientation::LEFT)
                        aSet.Put(SwFormatHoriOrient(0, text::HoriOrientation::LEFT));
                    break;

                case RndStdIds::FLY_AT_PARA:
                    // left, from left, right, top, no wrap, wrap left and right
                    if (eSurround != css::text::WrapTextMode_LEFT && eSurround != css::text::WrapTextMode_RIGHT)
                        aSet.Put(SwFormatSurround(css::text::WrapTextMode_LEFT));

                    if( eVOrient != text::VertOrientation::TOP)
                        aSet.Put(SwFormatVertOrient(0, text::VertOrientation::TOP));

                    if (eHOrient != text::HoriOrientation::NONE && eHOrient != text::HoriOrientation::LEFT && eHOrient != text::HoriOrientation::RIGHT)
                        aSet.Put(SwFormatHoriOrient(0, text::HoriOrientation::LEFT));
                    break;

                case RndStdIds::FLY_AT_CHAR:
                    // left, from left, right, top, wrap through
                    if(eSurround != css::text::WrapTextMode_THROUGH)
                        aSet.Put(SwFormatSurround(css::text::WrapTextMode_THROUGH));

                    if( eVOrient != text::VertOrientation::TOP)
                        aSet.Put(SwFormatVertOrient(0, text::VertOrientation::TOP));

                    if (eHOrient != text::HoriOrientation::NONE && eHOrient != text::HoriOrientation::LEFT && eHOrient != text::HoriOrientation::RIGHT)
                        aSet.Put(SwFormatHoriOrient(0, text::HoriOrientation::LEFT));
                    break;

                default:
                    ;
                }

                if( aSet.Count() )
                    rSh.SetFlyFrameAttr( aSet );
            }
            rSh.EndUndo();

            GetView().GetViewFrame()->GetBindings().Invalidate( SID_ANCHOR_MENU );
        }
        break;

        case FN_FRAME_NOWRAP:
        case FN_FRAME_WRAP:
        case FN_FRAME_WRAP_IDEAL:
        case FN_FRAME_WRAPTHRU:
        case FN_FRAME_WRAPTHRU_TRANSP:
        case FN_FRAME_WRAPTHRU_TOGGLE:
        case FN_FRAME_WRAP_CONTOUR:
        case FN_WRAP_ANCHOR_ONLY:
        case FN_FRAME_WRAP_LEFT:
        case FN_FRAME_WRAP_RIGHT:
            SetWrapMode( nSlot );
            break;

        case FN_UPDATE_ALL_LINKS:
            {
                if( !rSh.GetLinkManager().GetLinks().empty() )
                {
                    rSh.EnterStdMode();
                    rSh.StartAllAction();
                    rSh.GetLinkManager().UpdateAllLinks( false, false, nullptr );
                    rSh.EndAllAction();
                }
            }
            break;

        case FN_XFORMS_DESIGN_MODE:
            if (pArgs && pArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET)
            {
                if (const SfxBoolItem* pBoolItem = dynamic_cast<const SfxBoolItem*>(pItem))
                {
                    bool bDesignMode = pBoolItem->GetValue();

                    // set from design mode
                    OSL_ENSURE( GetView().GetFormShell() != nullptr, "form shell?" );
                    SfxRequest aReq( GetView().GetViewFrame(), SID_FM_DESIGN_MODE );
                    aReq.AppendItem( SfxBoolItem( SID_FM_DESIGN_MODE, bDesignMode ) );
                    GetView().GetFormShell()->Execute( aReq );
                    aReq.Done();

                    // also set suitable view options
                    SwViewOption aViewOption = *rSh.GetViewOptions();
                    aViewOption.SetFormView( ! bDesignMode );
                    rSh.ApplyViewOptions( aViewOption );
                }
            }
            break;

        default:
            bMore = true;
    }


    if(!bMore || !pArgs)
        return;

    pItem = nullptr;
    pArgs->GetItemState(GetPool().GetWhich(nSlot), false, &pItem);
    if(!pItem)
        return;

    switch(nSlot)
    {
    case SID_ATTR_BRUSH:
    case SID_ATTR_BORDER_SHADOW:
    case RES_SHADOW:
    {
        rSh.StartAllAction();
        // Table cell(s) selected?
        if ( rSh.IsTableMode() )
        {
            SwFrameFormat *pFormat = rSh.GetTableFormat();
            pFormat->SetFormatAttr( *pItem );
        }
        else if ( rSh.IsFrameSelected() )
        {
            // Set border attributes via Frame-Manager.
            SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE, nullptr );
            aMgr.SetAttrSet( *pArgs );
            aMgr.UpdateFlyFrame();
        }
        else
        {
            rSh.SetAttrSet( *pArgs );
        }
        rSh.EndAllAction();
    }
    break;
    case FN_PAGE_STYLE_SET_LR_MARGIN:
    case FN_PAGE_STYLE_SET_UL_MARGIN:
    case FN_PAGE_STYLE_SET_NUMBER_FORMAT:
    case FN_PAGE_STYLE_SET_PAPER_SIZE:
    case FN_PAGE_STYLE_SET_PAPER_BIN:
    {
        OSL_FAIL("not implemented");
    }
    break;

    case SID_ATTR_BORDER_OUTER:
    {
        // Table cell(s) selected?
        if ( rSh.IsTableMode() )
        {
            // Set border attributes Get/SetTabBorders()
            rSh.SetTabBorders(*pArgs);
        }
        else if ( rSh.IsFrameSelected() )
        {
            // Set border attributes via Frame-Manager.
            SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE, nullptr );
            aMgr.SetAttrSet(*pArgs);
            aMgr.UpdateFlyFrame();
        }
        else
        {
            // Set border attributes via shell quite normally.
            rSh.SetAttrItem( *pItem );
        }
    }
    break;
    default:
            OSL_FAIL("wrong Dispatcher");
    }
}

// Here the state for SID_IMAP / SID_CONTOUR will be handled
// until the swapping of the graphic is finished.

IMPL_LINK_NOARG(SwBaseShell, GraphicArrivedHdl, SwCursorShell&, void)
{
    SwWrtShell &rSh = GetShell();
    if (CNT_GRF != rSh.SwEditShell::GetCntType())
        return;
    GraphicType const nGrfType(rSh.GetGraphicType());
    if (GraphicType::NONE == nGrfType || m_aGrfUpdateSlots.empty())
        return;

    bool bProtect = FlyProtectFlags::NONE != rSh.IsSelObjProtected(FlyProtectFlags::Content|FlyProtectFlags::Parent);
    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    for( const auto nSlot : m_aGrfUpdateSlots )
    {
        bool bSetState = false;
        bool bState = false;
        switch( nSlot )
        {
        case SID_IMAP:
        case SID_IMAP_EXEC:
            {
                sal_uInt16 nId = SvxIMapDlgChildWindow::GetChildWindowId();
                SfxChildWindow *pChildWindow = pVFrame->HasChildWindow(nId) ?
                    pVFrame->GetChildWindow(nId) : nullptr;
                SvxIMapDlg *pDlg = pChildWindow ?
                    static_cast<SvxIMapDlg*>(pChildWindow->GetController().get()) : nullptr;

                if( pDlg && ( SID_IMAP_EXEC == nSlot ||
                            ( SID_IMAP == nSlot && !bProtect)) &&
                    pDlg->GetEditingObject() != rSh.GetIMapInventor())
                        lcl_UpdateIMapDlg( rSh );

                if( !bProtect && SID_IMAP == nSlot )
                {
                    bSetState = true;
                    bState = nullptr != pDlg;
                }
            }
            break;

        case SID_CONTOUR_DLG:
            if( !bProtect )
            {
                sal_uInt16 nId = SvxContourDlgChildWindow::GetChildWindowId();
                SfxChildWindow *pChildWindow = pVFrame->HasChildWindow(nId) ?
                    pVFrame->GetChildWindow(nId) : nullptr;
                SvxIMapDlg *pDlg = pChildWindow ?
                    static_cast<SvxIMapDlg*>(pChildWindow->GetController().get()) : nullptr;
                if( pDlg && pDlg->GetEditingObject() !=
                            rSh.GetIMapInventor() )
                    lcl_UpdateContourDlg( rSh, SelectionType::Graphic );

                bSetState = true;
                bState = nullptr != pDlg;
            }
            break;

        case FN_FRAME_WRAP_CONTOUR:
            if( !bProtect )
            {
                SfxItemSetFixed<RES_SURROUND, RES_SURROUND> aSet(GetPool());
                rSh.GetFlyFrameAttr(aSet);
                const SwFormatSurround& rWrap = aSet.Get(RES_SURROUND);
                bSetState = true;
                bState = rWrap.IsContour();
            }
            break;

        case SID_GRFFILTER:
        case SID_GRFFILTER_INVERT:
        case SID_GRFFILTER_SMOOTH:
        case SID_GRFFILTER_SHARPEN:
        case SID_GRFFILTER_REMOVENOISE:
        case SID_GRFFILTER_SOBEL:
        case SID_GRFFILTER_MOSAIC:
        case SID_GRFFILTER_EMBOSS:
        case SID_GRFFILTER_POSTER:
        case SID_GRFFILTER_POPART:
        case SID_GRFFILTER_SEPIA:
        case SID_GRFFILTER_SOLARIZE:
            bSetState = bState = GraphicType::Bitmap == nGrfType;
            break;
        }

        if( bSetState )
        {
            SfxBoolItem aBool( nSlot, bState );
            if( m_pGetStateSet )
                m_pGetStateSet->Put( aBool );
            else
                pVFrame->GetBindings().SetState( aBool );
        }
    }
    m_aGrfUpdateSlots.clear();
}

void SwBaseShell::GetState( SfxItemSet &rSet )
{
    SwWrtShell &rSh = GetShell();
    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    m_pGetStateSet = &rSet;
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_GALLERY_FORMATS:
                if ( rSh.IsObjSelected() ||
                     (rSh.IsSelFrameMode() &&
                      !(rSh.GetSelectionType() & SelectionType::Graphic)) )
                    rSet.DisableItem( nWhich );
                break;
            case SID_GALLERY_ENABLE_ADDCOPY:
                // #108230# allow copy from gallery in Writer AND Writer/Web!
                rSet.Put( SfxBoolItem( SID_GALLERY_ENABLE_ADDCOPY, true ) );
                break;
            case FN_EDIT_REGION:
                if( !rSh.IsAnySectionInDoc() )
                    rSet.DisableItem(nWhich);
                break;

            case FN_EDIT_CURRENT_REGION:
                //tdf#112808 if cursor is in an index, don't show the edit section.
                if( !rSh.GetCurrSection() ||
                    (rSh.GetCurrSection()->GetType() != SectionType::Content &&
                    rSh.GetCurrSection()->GetType() != SectionType::FileLink ))
                {
                    rSet.DisableItem(nWhich);
                }
                break;

            case FN_INSERT_REGION:
                if( rSh.CursorInsideInputField()
                    || rSh.IsSelFrameMode()
                    || !rSh.IsInsRegionAvailable() )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case FN_CONVERT_TABLE_TO_TEXT:
            {
                FrameTypeFlags eFrameType = rSh.GetFrameType(nullptr,true);
                if( (eFrameType & FrameTypeFlags::FOOTNOTE) ||
                    !rSh.GetTableFormat() )
                    rSet.DisableItem( nWhich );
            }
            break;
            case FN_CONVERT_TEXT_TO_TABLE:
            {
                FrameTypeFlags eFrameType = rSh.GetFrameType(nullptr,true);
                if( (eFrameType & FrameTypeFlags::FOOTNOTE) ||
                    !rSh.IsTextToTableAvailable()  )
                    rSet.DisableItem( nWhich );
            }
            break;
            case FN_CONVERT_TEXT_TABLE:
            {
                FrameTypeFlags eFrameType = rSh.GetFrameType(nullptr,true);
                if( (eFrameType & FrameTypeFlags::FOOTNOTE) ||
                    (!rSh.GetTableFormat() && !rSh.IsTextToTableAvailable() ) )
                    rSet.DisableItem( nWhich );
            }
            break;
            case RES_SHADOW:
            {
                SfxItemSetFixed<RES_SHADOW, RES_SHADOW> aSet( rSh.GetAttrPool());

                // Table cell(s) selected?
                if ( rSh.IsTableMode() )
                {
                    SwFrameFormat *pFormat = rSh.GetTableFormat();
                    aSet.Put(pFormat->GetFormatAttr( nWhich ));
                }
                else if( rSh.IsFrameSelected() )
                {
                    SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE, nullptr );
                    aSet.Put( aMgr.GetAttrSet() );
                }
                else
                    rSh.GetCurAttr( aSet );

                const SvxShadowItem& rShItem = static_cast<const SvxShadowItem&>(aSet.Get(nWhich));
                rSet.Put(rShItem);
            }
            break;
            case SID_IMAP:
            {
                // #i59688#
                // Improve efficiency:
                // If selected object is protected, item has to disabled.
                const bool bProtect = FlyProtectFlags::NONE != rSh.IsSelObjProtected(FlyProtectFlags::Content|FlyProtectFlags::Parent);
                if ( bProtect )
                {
                    rSet.DisableItem( nWhich );
                }
                else
                {
                    const sal_uInt16 nId = SvxIMapDlgChildWindow::GetChildWindowId();
                    const bool bHas = pVFrame->HasChildWindow( nId );
                    const bool bFrameSel = rSh.IsFrameSelected();
                    const bool bIsGraphicSelection =
                                rSh.GetSelectionType() == SelectionType::Graphic;

                    // #i59688#
                    // Avoid unnecessary loading of selected graphic.
                    // The graphic is only needed, if the dialog is open.
                    // If the swapping of the graphic is finished, the status
                    // must be determined asynchronously, until this the slot
                    // will be disabled.
                    if ( bHas && bIsGraphicSelection && rSh.IsLinkedGrfSwapOut() )
                    {
                        if( AddGrfUpdateSlot( nWhich ))
                            rSh.GetGraphic(false);  // start the loading
                    }
                    else
                    {
                        if ( !bHas &&
                             ( !bFrameSel ||
                               ( bIsGraphicSelection &&
                                 rSh.GetGraphicType() == GraphicType::NONE ) ) )
                        {
                            rSet.DisableItem( nWhich );
                        }
                        else
                        {
                            SfxBoolItem aBool(nWhich, bHas);
                            if ( bHas && bFrameSel )
                                lcl_UpdateIMapDlg( rSh );
                            rSet.Put(aBool);
                        }
                    }
                }
            }
            break;
            case SID_IMAP_EXEC:
            {
                bool bDisable = false;
                if( !rSh.IsFrameSelected())
                    bDisable = true;
                sal_uInt16 nId = SvxIMapDlgChildWindow::GetChildWindowId();
                if(!bDisable && pVFrame->HasChildWindow( nId ))
                {
                    if(rSh.GetSelectionType() == SelectionType::Graphic
                                    && rSh.IsLinkedGrfSwapOut())
                    {
                        if( AddGrfUpdateSlot( nWhich ))
                            rSh.GetGraphic(false);  // start the loading
                    }
                    else
                    {
                        SvxIMapDlg *pDlg = GetIMapDlg(GetView());
                        if (pDlg && pDlg->GetEditingObject() != rSh.GetIMapInventor())
                            lcl_UpdateIMapDlg( rSh );
                    }
                }
                rSet.Put(SfxBoolItem(nWhich, bDisable));
            }
            break;

            case FN_BACKSPACE:
            case SID_DELETE:
                if ( ( rSh.HasReadonlySel() && !rSh.CursorInsideInputField() )
                     || rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_CONTOUR_DLG:
            {
                bool bParentCntProt = FlyProtectFlags::NONE != rSh.IsSelObjProtected(FlyProtectFlags::Content|FlyProtectFlags::Parent );

                if( bParentCntProt || 0 != (HTMLMODE_ON & ::GetHtmlMode(
                                            GetView().GetDocShell() )) )
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_uInt16 nId = SvxContourDlgChildWindow::GetChildWindowId();
                    bool bHas = GetView().GetViewFrame()->HasChildWindow( nId );
                    SelectionType nSel = rSh.GetSelectionType();
                    bool bOk(nSel & (SelectionType::Graphic|SelectionType::Ole));

                    bool bDisable = false;
                    if( !bHas && !bOk )
                        bDisable = true;
                    // #i59688#
                    // Avoid unnecessary loading of selected graphic.
                    // The graphic is only needed, if the dialog is open.
                    // If the swapping of the graphic is finished, the status
                    // must be determined asynchronously, until this the slot
                    // will be disabled.
                    else if ( bHas && (nSel & SelectionType::Graphic) &&
                              rSh.IsLinkedGrfSwapOut() )
                    {
                        if( AddGrfUpdateSlot( nWhich ))
                            rSh.GetGraphic(false);  // start the loading
                        // #i75481#
                        bDisable = true;
                    }
                    else if( bHas && bOk )
                        bDisable = !lcl_UpdateContourDlg( rSh, nSel );
                    else if( bOk )
                    {
                        // #i75481#
                        // apply fix #i59688# only for selected graphics
                        if ( nSel & SelectionType::Graphic )
                            bDisable = GraphicType::NONE == rSh.GetGraphicType();
                        else
                            bDisable = GraphicType::NONE == rSh.GetIMapGraphic().GetType();
                    }

                    if( bDisable )
                        rSet.DisableItem( nWhich );
                    else
                        rSet.Put( SfxBoolItem(nWhich, bHas) );
                }
            }
            break;
            case SID_CONTOUR_EXEC:
            {
                bool bDisable = false;
                SelectionType nSel = rSh.GetSelectionType();
                if( !(nSel & (SelectionType::Graphic|SelectionType::Ole)) )
                    bDisable = true;
                sal_uInt16 nId = SvxContourDlgChildWindow::GetChildWindowId();
                if( !bDisable && GetView().GetViewFrame()->HasChildWindow( nId ))
                {
                    SvxContourDlg *pDlg = GetContourDlg(GetView());
                    if (pDlg && pDlg->GetEditingObject() != rSh.GetIMapInventor())
                        bDisable = true;
                }
                rSet.Put(SfxBoolItem(nWhich, bDisable));
            }
            break;

            case SID_ANCHOR_MENU:
            case FN_TOOL_ANCHOR_PAGE:
            case FN_TOOL_ANCHOR_PARAGRAPH:
            case FN_TOOL_ANCHOR_CHAR:
            case FN_TOOL_ANCHOR_AT_CHAR:
            case FN_TOOL_ANCHOR_FRAME:
            {
                bool bObj = 0 != rSh.IsObjSelected();
                bool bParentCntProt = rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;

                if( !bParentCntProt && (bObj || rSh.IsFrameSelected()))
                {
                    SfxItemSetFixed<RES_ANCHOR, RES_ANCHOR> aSet(GetPool());
                    if(bObj)
                        rSh.GetObjAttr(aSet);
                    else
                        rSh.GetFlyFrameAttr(aSet);
                    RndStdIds eSet = aSet.Get(RES_ANCHOR).GetAnchorId();
                    const bool bSet =
                           ((nWhich == FN_TOOL_ANCHOR_PAGE) &&
                            (eSet == RndStdIds::FLY_AT_PAGE))
                        || ((nWhich == FN_TOOL_ANCHOR_PARAGRAPH) &&
                            (eSet == RndStdIds::FLY_AT_PARA))
                        || ((nWhich == FN_TOOL_ANCHOR_FRAME) &&
                            (eSet == RndStdIds::FLY_AT_FLY))
                        || ((nWhich == FN_TOOL_ANCHOR_AT_CHAR) &&
                            (eSet == RndStdIds::FLY_AT_CHAR))
                        || ((nWhich == FN_TOOL_ANCHOR_CHAR) &&
                            (eSet == RndStdIds::FLY_AS_CHAR));

                    if( nWhich == FN_TOOL_ANCHOR_FRAME && !rSh.IsFlyInFly() )
                        rSet.DisableItem(nWhich);
                    else if(nWhich != SID_ANCHOR_MENU)
                        rSet.Put(SfxBoolItem(nWhich, bSet));

                    if (comphelper::LibreOfficeKit::isActive())
                    {
                        if (nWhich == FN_TOOL_ANCHOR_PAGE || nWhich == FN_TOOL_ANCHOR_FRAME)
                        {
                            rSet.DisableItem(nWhich);
                        }
                    }
                }
                else
                    rSet.DisableItem( nWhich );
            }
            break;
            case FN_FRAME_NOWRAP:
            case FN_FRAME_WRAP:
            case FN_FRAME_WRAP_IDEAL:
            case FN_FRAME_WRAPTHRU:
            case FN_FRAME_WRAPTHRU_TRANSP:
            case FN_FRAME_WRAPTHRU_TOGGLE:
            case FN_FRAME_WRAP_CONTOUR:
            case FN_WRAP_ANCHOR_ONLY:
            case FN_FRAME_WRAP_LEFT:
            case FN_FRAME_WRAP_RIGHT:
            {
                bool bObj = 0 != rSh.IsObjSelected();
                bool bParentCntProt = rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;

                if( !bParentCntProt && (bObj || rSh.IsFrameSelected()))
                {
                    SfxItemSetFixed<RES_OPAQUE, RES_ANCHOR> aSet(GetPool());
                    RndStdIds nAnchorType;
                    if(bObj)
                    {
                        rSh.GetObjAttr(aSet);
                        nAnchorType = rSh.GetAnchorId();
                    }
                    else
                    {
                        rSh.GetFlyFrameAttr(aSet);
                        nAnchorType = aSet.Get(RES_ANCHOR).GetAnchorId();
                    }
                    const SwFormatSurround& rWrap = aSet.Get(RES_SURROUND);

                    const SvxOpaqueItem& rOpaque = aSet.Get(RES_OPAQUE);
                    bool bOpaque = rOpaque.GetValue();
                    css::text::WrapTextMode nSurround = rWrap.GetSurround();
                    bool bSet = false;

                    bool bDisable =
                        (nAnchorType == RndStdIds::UNKNOWN) || (nAnchorType == RndStdIds::FLY_AS_CHAR);
                    const bool bHtmlMode =
                        0 != ::GetHtmlMode(GetView().GetDocShell());

                    switch( nWhich )
                    {
                        case FN_FRAME_NOWRAP:
                            bDisable |=
                                (   (nAnchorType != RndStdIds::FLY_AT_PARA)
                                 && (nAnchorType != RndStdIds::FLY_AT_CHAR)
                                 && (nAnchorType != RndStdIds::FLY_AT_PAGE));
                            bSet = nSurround == css::text::WrapTextMode_NONE;
                        break;
                        case FN_FRAME_WRAP:
                            bDisable |= bHtmlMode;
                            bSet = nSurround == css::text::WrapTextMode_PARALLEL;
                        break;
                        case FN_FRAME_WRAP_IDEAL:
                            bDisable |= bHtmlMode;
                            bSet = nSurround == css::text::WrapTextMode_DYNAMIC;
                        break;
                        case FN_FRAME_WRAPTHRU:
                            bDisable |= (bHtmlMode ||
                                (   (nAnchorType != RndStdIds::FLY_AT_PARA)
                                 && (nAnchorType != RndStdIds::FLY_AT_CHAR)
                                 && (nAnchorType != RndStdIds::FLY_AT_PAGE)));
                            if(bObj)
                                bSet = nSurround == css::text::WrapTextMode_THROUGH && rSh.GetLayerId();
                            else
                                bSet = nSurround == css::text::WrapTextMode_THROUGH && bOpaque;
                        break;
                        case FN_FRAME_WRAPTHRU_TRANSP:
                        case FN_FRAME_WRAPTHRU_TOGGLE:
                            bDisable |= bHtmlMode;
                            if(bObj)
                                bSet = nSurround == css::text::WrapTextMode_THROUGH && !rSh.GetLayerId();
                            else
                                bSet = nSurround == css::text::WrapTextMode_THROUGH && !bOpaque;
                        break;
                        case FN_FRAME_WRAP_CONTOUR:
                            bDisable |= bHtmlMode;
                            //no contour available whenn no wrap or wrap through is set
                            bDisable |= (nSurround == css::text::WrapTextMode_NONE || nSurround == css::text::WrapTextMode_THROUGH);
                            if( !bDisable )
                            {
                                SelectionType nSel = rSh.GetSelectionType();
                                if( (nSel & SelectionType::Graphic) &&
                                            rSh.IsLinkedGrfSwapOut())
                                {
                                    if( AddGrfUpdateSlot( nWhich ))
                                        rSh.GetGraphic(false);  // start the loading
                                }
                                else if( rSh.IsFrameSelected() )
                                {
                                    // #i102253# applied patch from OD (see task)
                                    bDisable =
                                        nSel & SelectionType::Frame ||
                                        GraphicType::NONE == rSh.GetIMapGraphic().GetType();
                                }
                            }
                            bSet = !bDisable && rWrap.IsContour();

                        break;
                        case FN_WRAP_ANCHOR_ONLY:
                            bDisable |= (bHtmlMode ||
                                (nAnchorType != RndStdIds::FLY_AT_PARA));
                            bSet = rWrap.IsAnchorOnly();
                        break;
                        case FN_FRAME_WRAP_LEFT:
                            bSet = nSurround == css::text::WrapTextMode_LEFT;
                        break;
                        case FN_FRAME_WRAP_RIGHT:
                            bSet = nSurround == css::text::WrapTextMode_RIGHT;
                        break;
                    }

                    if(bDisable)
                        rSet.DisableItem(nWhich);
                    else
                        rSet.Put(SfxBoolItem(nWhich, bSet));
                }
                else
                    rSet.DisableItem(nWhich);
            }
            break;
            case FN_UPDATE_CHARTS:
                if( !rSh.HasCharts() )
                    rSet.DisableItem( nWhich );
                break;
            case FN_UPDATE_ALL_LINKS:
                if ( rSh.GetLinkManager().GetLinks().empty() )
                    rSet.DisableItem(nWhich);
                break;
            case FN_XFORMS_DESIGN_MODE:
                // enable if in XForms document
                if( rSh.GetDoc()->isXForms() )
                {
                    // determine current state from view options
                    bool bValue = ! rSh.GetViewOptions()->IsFormView();
                    rSet.Put( SfxBoolItem( nWhich, bValue ) );
                }
                else
                    rSet.Put( SfxVisibilityItem( nWhich, false ) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
    m_pGetStateSet = nullptr;
}

// Disable the slots with this status method

void SwBaseShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

// Disable the slots with this status method

void SwBaseShell::StateStyle( SfxItemSet &rSet )
{
    bool bParentCntProt = GetShell().IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;
    ShellMode eMode = GetView().GetShellMode();

    if ( bParentCntProt ||
         ShellMode::Draw == eMode ||
         ShellMode::DrawForm == eMode ||
         ShellMode::DrawText == eMode ||
         ShellMode::Bezier == eMode )
    {
        SfxWhichIter aIter( rSet );
        sal_uInt16 nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            rSet.DisableItem( nWhich );
            nWhich = aIter.NextWhich();
        }
    }
    else
        GetView().GetDocShell()->StateStyleSheet(rSet, &GetShell());
}

void SwBaseShell::SetWrapMode( sal_uInt16 nSlot )
{
    SwWrtShell &rSh = GetShell();
    bool bObj = 0 != rSh.IsObjSelected();
    if( !bObj && !rSh.IsFrameSelected())
        return;

    SfxItemSetFixed<RES_OPAQUE, RES_SURROUND> aSet(GetPool());
    if(bObj)
        rSh.GetObjAttr(aSet);
    else
        rSh.GetFlyFrameAttr(aSet);
    SwFormatSurround aWrap( aSet.Get(RES_SURROUND) );
    css::text::WrapTextMode nOldSurround(aWrap.GetSurround());
    css::text::WrapTextMode nSurround = css::text::WrapTextMode_PARALLEL;

    switch (nSlot)
    {
        case FN_FRAME_NOWRAP:
            nSurround = css::text::WrapTextMode_NONE;
            if (aWrap.IsContour())
                aWrap.SetContour(false);
            break;
        case FN_FRAME_WRAP_IDEAL:
            nSurround = css::text::WrapTextMode_DYNAMIC;
            break;
        case FN_WRAP_ANCHOR_ONLY:
            aWrap.SetAnchorOnly(!aWrap.IsAnchorOnly());

            // keep previous wrapping

            // switch to wrap css::text::WrapTextMode_PARALLEL, if previous wrap is css::text::WrapTextMode_NONE
            if ( nOldSurround != css::text::WrapTextMode_NONE )
            {
                nSurround = nOldSurround;
            }
            break;
        case FN_FRAME_WRAP_CONTOUR:
            aWrap.SetContour(!aWrap.IsContour());
            // Contour is meaningless in no-wrap or wrap-through context. Otherwise keep existing wrap.
            if ( !aWrap.IsContour() ||
                 ( nOldSurround != css::text::WrapTextMode_NONE &&
                   nOldSurround != css::text::WrapTextMode_THROUGH ) )
            {
                nSurround = nOldSurround;
            }
            break;
        case FN_FRAME_WRAPTHRU_TRANSP:
        case FN_FRAME_WRAPTHRU_TOGGLE:
            if (aWrap.IsContour())
                aWrap.SetContour(false);
            [[fallthrough]];
        case FN_FRAME_WRAPTHRU:
            nSurround = css::text::WrapTextMode_THROUGH;
            break;

        case FN_FRAME_WRAP_LEFT:
            nSurround = css::text::WrapTextMode_LEFT;
            break;

        case FN_FRAME_WRAP_RIGHT:
            nSurround = css::text::WrapTextMode_RIGHT;
            break;

        default:
            break;
    }
    aWrap.SetSurround(nSurround);

    if (nSlot != FN_FRAME_WRAP_CONTOUR)
    {
        // Defaulting the contour wrap on draw objects.
        if (bObj && nOldSurround != nSurround &&
            (nOldSurround == css::text::WrapTextMode_NONE || nOldSurround == css::text::WrapTextMode_THROUGH))
        {
            aWrap.SetContour(true);
        }
    }

    aSet.Put( aWrap );

    bool bOpaque = nSlot != FN_FRAME_WRAPTHRU_TRANSP && nSlot != FN_FRAME_WRAPTHRU_TOGGLE;
    if( nSlot == FN_FRAME_WRAPTHRU_TOGGLE )
    {
        if( bObj )
            bOpaque = !rSh.GetLayerId();
        else
        {
            const SvxOpaqueItem& aOpaque( aSet.Get(RES_OPAQUE) );
            bOpaque = !aOpaque.GetValue();
        }
    }
    aSet.Put(SvxOpaqueItem(RES_OPAQUE, bOpaque ));

    if(bObj)
    {
        rSh.SetObjAttr(aSet);
        if ( bOpaque )
            rSh.SelectionToHeaven();
        else
            rSh.SelectionToHell();
    }
    else
        rSh.SetFlyFrameAttr(aSet);

}

//Force update of the status line

void SwBaseShell::SetFrameMode(FlyMode eMode, SwWrtShell *pSh )
{
    eFrameMode = eMode;
    SfxBindings &rBnd = pSh->GetView().GetViewFrame()->GetBindings();

    if( eMode == FLY_DRAG || pSh->IsFrameSelected() || pSh->IsObjSelected() )
    {
        const SfxPointItem aTmp1( SID_ATTR_POSITION, pSh->GetAnchorObjDiff());
        const SvxSizeItem  aTmp2( SID_ATTR_SIZE,     pSh->GetObjSize());
        rBnd.SetState( aTmp1 );
        rBnd.SetState( aTmp2 );
    }
    else if( eMode == FLY_DRAG_END )
    {
        static sal_uInt16 aInval[] =
        {
            SID_ATTR_POSITION, SID_ATTR_SIZE, 0
        };
        rBnd.Invalidate(aInval);
    }
}

SwBaseShell::SwBaseShell(SwView& rVw) :
    SfxShell( &rVw ),
    m_rView(rVw),
    m_pGetStateSet(nullptr)
{
    SwWrtShell& rWrtSh = m_rView.GetWrtShell();

    SetPool(&rWrtSh.GetAttrPool());
    SetName("Base");
    rWrtSh.SetGrfArrivedLnk( LINK( this, SwBaseShell, GraphicArrivedHdl));
}

SwBaseShell::~SwBaseShell()
{
    if( m_rView.GetCurShell() == this )
        m_rView.ResetSubShell();

    Link<SwCursorShell&,void> aTmp( LINK( this, SwBaseShell, GraphicArrivedHdl));
    if( aTmp == m_rView.GetWrtShell().GetGrfArrivedLnk() )
        m_rView.GetWrtShell().SetGrfArrivedLnk( Link<SwCursorShell&,void>() );
}

void SwBaseShell::ExecTextCtrl( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();

    if( pArgs)
    {
        SwWrtShell &rSh = GetShell();
        std::unique_ptr<SvxScriptSetItem> pSSetItem;
        sal_uInt16 nSlot = rReq.GetSlot();
        SfxItemPool& rPool = rSh.GetAttrPool();
        sal_uInt16 nWhich = rPool.GetWhich( nSlot );
        SvtScriptType nScripts = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;
        SfxItemSetFixed<RES_CHRATR_FONTSIZE, RES_CHRATR_FONTSIZE,
                        RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_FONTSIZE,
                        RES_CHRATR_CTL_FONTSIZE, RES_CHRATR_CTL_FONTSIZE>
            aHeightSet( GetPool() );

        switch( nSlot )
        {
            case SID_ATTR_CHAR_FONT:
            {
                nScripts = rSh.GetScriptType();
                // #i42732# input language should be preferred over
                // current cursor position to detect script type
                if(!rSh.HasSelection())
                {
                    LanguageType nInputLang = GetView().GetEditWin().GetInputLanguage();
                    if(nInputLang != LANGUAGE_DONTKNOW && nInputLang != LANGUAGE_SYSTEM)
                        nScripts = SvtLanguageOptions::GetScriptTypeOfLanguage( nInputLang );
                }
                [[fallthrough]];
            }
            case SID_ATTR_CHAR_POSTURE:
            case SID_ATTR_CHAR_WEIGHT:
            {
                pSSetItem.reset(new SvxScriptSetItem( nSlot, rPool ));
                pSSetItem->PutItemForScriptType( nScripts, pArgs->Get( nWhich ));
                pArgs = &pSSetItem->GetItemSet();
            }
            break;
            case SID_ATTR_CHAR_FONTHEIGHT:
            {
                if(rSh.HasSelection())
                {
                    pSSetItem.reset(new SvxScriptSetItem( nSlot, rPool ));
                    pSSetItem->PutItemForScriptType( nScripts, pArgs->Get( nWhich ));
                    pArgs = &pSSetItem->GetItemSet();
                }
                else
                {
                    nScripts = rSh.GetScriptType();
                    LanguageType nInputLang = GetView().GetEditWin().GetInputLanguage();
                    if(nInputLang != LANGUAGE_DONTKNOW && nInputLang != LANGUAGE_SYSTEM)
                        nScripts = SvtLanguageOptions::GetScriptTypeOfLanguage( nInputLang );
                    sal_uInt32 nHeight = static_cast< const SvxFontHeightItem& >(pArgs->Get( nWhich )).GetHeight();
                    SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();

                    SfxItemSetFixed<RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
                                    RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
                                    RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE>
                            aLangSet( GetPool() );
                    rSh.GetCurAttr( aLangSet );

                    sal_Int32 nWesternSize =
                            pStdFont->GetFontHeight(FONT_STANDARD, FONT_GROUP_DEFAULT,
                            aLangSet.Get( RES_CHRATR_LANGUAGE).GetLanguage());
                    sal_Int32 nCJKSize =
                            pStdFont->GetFontHeight(FONT_STANDARD, FONT_GROUP_CJK,
                            aLangSet.Get( RES_CHRATR_CJK_LANGUAGE).GetLanguage());
                    sal_Int32 nCTLSize =
                            pStdFont->GetFontHeight(FONT_STANDARD, FONT_GROUP_CTL,
                            aLangSet.Get( RES_CHRATR_CTL_LANGUAGE).GetLanguage());

                    switch(nScripts)
                    {
                        case SvtScriptType::LATIN:
                            nCJKSize = nHeight * nCJKSize / nWesternSize;
                            nCTLSize = nHeight * nCTLSize / nWesternSize;
                            nWesternSize = static_cast<sal_Int32>(nHeight);
                        break;
                        case SvtScriptType::ASIAN:
                            nCTLSize = nHeight* nCTLSize / nCJKSize;
                            nWesternSize = nHeight * nWesternSize / nCJKSize;
                            nCJKSize = static_cast<sal_Int32>(nHeight);
                        break;
                        case SvtScriptType::COMPLEX:
                            nCJKSize = nHeight * nCJKSize / nCTLSize;
                            nWesternSize = nHeight * nWesternSize / nCTLSize;
                            nCTLSize = static_cast<sal_Int32>(nHeight);
                        break;
                        default: break;
                    }
                    aHeightSet.Put( SvxFontHeightItem( static_cast<sal_uInt32>(nWesternSize), 100, RES_CHRATR_FONTSIZE ));
                    aHeightSet.Put( SvxFontHeightItem( static_cast<sal_uInt32>(nCJKSize), 100, RES_CHRATR_CJK_FONTSIZE ));
                    aHeightSet.Put( SvxFontHeightItem( static_cast<sal_uInt32>(nCTLSize), 100, RES_CHRATR_CTL_FONTSIZE ));
                    pArgs = &aHeightSet;
                }
            }
            break;
        }

        if( pArgs )
        {
            bool bAuto = false;
            if ( !isCHRATR(nWhich) ||
                 ( rSh.HasSelection() && rSh.IsSelFullPara() ) )
            {
                SwTextFormatColl * pColl = rSh.GetCurTextFormatColl();
                if ( pColl && pColl->IsAutoUpdateFormat() )
                {
                    rSh.AutoUpdatePara( pColl, *pArgs );
                    bAuto = true;
                }
            }

            if (!bAuto)
            {
                rSh.SetAttrSet( *pArgs );
            }
        }
    }
    else
        GetView().GetViewFrame()->GetDispatcher()->Execute( SID_CHAR_DLG );
    rReq.Done();
}

void SwBaseShell::GetTextCtrlState( SfxItemSet& rSet )
{
    SwWrtShell &rSh = GetShell();
    rSh.GetCurAttr( rSet );
}

void SwBaseShell::GetTextFontCtrlState( SfxItemSet& rSet )
{
    SwWrtShell &rSh = GetShell();
    bool bFirst = true;
    std::optional<SfxItemSet> pFntCoreSet;
    SvtScriptType nScriptType = SvtScriptType::LATIN;
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        switch( nWhich )
        {
        case RES_CHRATR_FONT:
        case RES_CHRATR_FONTSIZE:
        case RES_CHRATR_WEIGHT:
        case RES_CHRATR_POSTURE:
            {
                if( !pFntCoreSet )
                {
                    pFntCoreSet.emplace( *rSet.GetPool(),
                                    svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END-1> );
                    rSh.GetCurAttr( *pFntCoreSet );
                    nScriptType = rSh.GetScriptType();
                    // #i42732# input language should be preferred over
                    // current cursor position to detect script type
                    SwEditWin& rEditWin = GetView().GetEditWin();
                    if( rEditWin.IsUseInputLanguage() )
                    {
                        if(!rSh.HasSelection() && (
                            nWhich == RES_CHRATR_FONT ||
                            nWhich == RES_CHRATR_FONTSIZE ))
                        {
                            LanguageType nInputLang = rEditWin.GetInputLanguage();
                            if(nInputLang != LANGUAGE_DONTKNOW && nInputLang != LANGUAGE_SYSTEM)
                                nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nInputLang );
                        }
                    }
                }
                SfxItemPool& rPool = *rSet.GetPool();
                SvxScriptSetItem aSetItem( rPool.GetSlotId( nWhich ), rPool );
                aSetItem.GetItemSet().Put( *pFntCoreSet, false );
                const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                if( pI )
                {
                    rSet.Put( pI->CloneSetWhich(nWhich) );
                }
                else
                    rSet.InvalidateItem( nWhich );
                // Set input context of the SwEditWin according to the selected font and script type
                if(RES_CHRATR_FONT == nWhich)
                {
                    vcl::Font aFont;
                    if (const SvxFontItem* pFontItem = dynamic_cast<const SvxFontItem*>(pI))
                    {
                        aFont.SetFamilyName(pFontItem->GetFamilyName());
                        aFont.SetStyleName(pFontItem->GetStyleName());
                        aFont.SetFamily(pFontItem->GetFamily());
                        aFont.SetPitch(pFontItem->GetPitch());
                        aFont.SetCharSet(pFontItem->GetCharSet());
                    }

                    bool bVertical = rSh.IsInVerticalText();
                    aFont.SetOrientation(Degree10(bVertical ? 2700 : 0));
                    aFont.SetVertical(bVertical);
                    GetView().GetEditWin().SetInputContext( InputContext( aFont, InputContextFlags::Text |
                                                        InputContextFlags::ExtText ) );
                }
            }
            break;

        default:
            if( bFirst )
            {
                rSh.GetCurAttr( rSet );
                bFirst = false;
            }
        }
        nWhich = aIter.NextWhich();
    }
}

void SwBaseShell::GetBckColState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich(aIter.FirstWhich());
    SelectionType nSelType(rSh.GetSelectionType());
    std::unique_ptr<SvxBrushItem> aBrushItem(std::make_unique<SvxBrushItem>(RES_BACKGROUND));

    if( nWhich == SID_TABLE_CELL_BACKGROUND_COLOR )
    {
        rSh.GetBoxBackground( aBrushItem );
    }
    else
    {
        // Adapt to new DrawingLayer FillStyle; use a parent which has XFILL_NONE set
        SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aCoreSet(GetPool());

        aCoreSet.SetParent(&GetView().GetDocShell()->GetDoc()->GetDfltFrameFormat()->GetAttrSet());

        if(nSelType & SelectionType::Graphic || SelectionType::Frame & nSelType)
        {
            rSh.GetFlyFrameAttr(aCoreSet);
        }
        else
        {
            rSh.GetCurAttr(aCoreSet);
        }

        aBrushItem = getSvxBrushItemFromSourceSet(aCoreSet, RES_BACKGROUND);
    }

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_BACKGROUND_COLOR:
            case SID_TABLE_CELL_BACKGROUND_COLOR:
            {
                SvxColorItem aColorItem(aBrushItem->GetColor(), nWhich);
                rSet.Put(aColorItem);
                break;
            }
            case SID_ATTR_BRUSH:
            case RES_BACKGROUND:
            {
                // if this was intended to have a independent copy of the Item to be set
                // this is not needed due to the ItemSet/Pool cloning Items which get set anyways.
                // Keeping code as reference - it may have had other reasons I do notz see (?!?)
                // std::unique_ptr<SfxPoolItem> pNewItem(aBrushItem.CloneSetWhich(GetPool().GetWhich(nWhich)));
                rSet.Put(*aBrushItem);
                break;
            }
        }

        nWhich = aIter.NextWhich();
    }
}

void SwBaseShell::ExecBckCol(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    SelectionType nSelType(rSh.GetSelectionType());
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSlot(rReq.GetSlot());

    if (!pArgs && nSlot != SID_BACKGROUND_COLOR && nSlot != SID_TABLE_CELL_BACKGROUND_COLOR)
    {
        return;
    }

    std::unique_ptr<SvxBrushItem> aBrushItem(std::make_unique<SvxBrushItem>(RES_BACKGROUND));

    if ( nSlot == SID_TABLE_CELL_BACKGROUND_COLOR )
    {
        rSh.GetBoxBackground( aBrushItem );
    }
    else
    {
        // Adapt to new DrawingLayer FillStyle; use a parent which has XFILL_NONE set
        SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aCoreSet(GetPool());

        aCoreSet.SetParent(&GetView().GetDocShell()->GetDoc()->GetDfltFrameFormat()->GetAttrSet());

        if((SelectionType::Frame & nSelType) || (SelectionType::Graphic & nSelType))
        {
            rSh.GetFlyFrameAttr(aCoreSet);
        }
        else
        {
            rSh.GetCurAttr(aCoreSet);
        }

        aBrushItem = getSvxBrushItemFromSourceSet(aCoreSet, RES_BACKGROUND);
    }

    switch(nSlot)
    {
        case SID_BACKGROUND_COLOR:
        case SID_TABLE_CELL_BACKGROUND_COLOR:
        {
            const SfxPoolItem* pColorStringItem = nullptr;
            bool bIsTransparent = false;

            aBrushItem->SetGraphicPos(GPOS_NONE);

            sal_uInt16 nSlotId = (nSlot == SID_BACKGROUND_COLOR) ? SID_BACKGROUND_COLOR : SID_TABLE_CELL_BACKGROUND_COLOR;
            if (pArgs && SfxItemState::SET == pArgs->GetItemState(SID_ATTR_COLOR_STR, false, &pColorStringItem))
            {
                OUString sColor = static_cast<const SfxStringItem*>(pColorStringItem)->GetValue();
                if (sColor == "transparent")
                {
                    bIsTransparent = true;
                }
                else
                {
                    Color aColor(ColorTransparency, sColor.toInt32(16));

                    aBrushItem->SetColor(aColor);

                    SvxColorItem aNewColorItem(nSlotId);
                    aNewColorItem.SetValue(aColor);

                    GetView().GetViewFrame()->GetBindings().SetState(aNewColorItem);
                }
            }
            else if (pArgs)
            {
                const SvxColorItem& rNewColorItem = static_cast<const SvxColorItem&>(pArgs->Get(nSlotId));
                const Color& rNewColor = rNewColorItem.GetValue();
                aBrushItem->SetColor(rNewColor);
                GetView().GetViewFrame()->GetBindings().SetState(rNewColorItem);
            }
            else
            {
                bIsTransparent = true;
            }

            if (bIsTransparent)
            {
                aBrushItem->SetColor(COL_TRANSPARENT);
                rReq.AppendItem(SvxColorItem(COL_TRANSPARENT,nSlot));
            }
            break;
        }

        case SID_ATTR_BRUSH:
        case RES_BACKGROUND:
        {
            assert(pArgs && "only SID_BACKGROUND_COLOR can have !pArgs, checked at entry");
            aBrushItem.reset(static_cast<SvxBrushItem*>(pArgs->Get(GetPool().GetWhich(nSlot)).Clone()));
            break;
        }
        default:
        {
            rReq.Ignore();
            OSL_FAIL("unknown message in ExecuteAttr!" );
            return;
        }
    }

    if ( nSlot == SID_TABLE_CELL_BACKGROUND_COLOR )
    {
        rSh.SetBoxBackground( *aBrushItem );
    }
    else
    {
        // Adapt to new DrawingLayer FillStyle; use a parent which has XFILL_NONE set
        SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aCoreSet(GetPool());

        aCoreSet.SetParent(&GetView().GetDocShell()->GetDoc()->GetDfltFrameFormat()->GetAttrSet());
        setSvxBrushItemAsFillAttributesToTargetSet(*aBrushItem, aCoreSet);

        if((SelectionType::Frame & nSelType) || (SelectionType::Graphic & nSelType))
        {
            // Template autoupdate
            SwFrameFormat* pFormat = rSh.GetSelectedFrameFormat();

            if(pFormat && pFormat->IsAutoUpdateFormat())
            {
                rSh.AutoUpdateFrame(pFormat, aCoreSet);
            }
            else
            {
                rSh.SetFlyFrameAttr(aCoreSet);
            }
        }
        else
        {
            SwTextFormatColl* pColl = rSh.GetCurTextFormatColl();

            if(pColl && pColl->IsAutoUpdateFormat())
            {
                rSh.AutoUpdatePara(pColl, aCoreSet);
            }
            else
            {
                rSh.SetAttrSet(aCoreSet);
            }
        }
    }

    rReq.Done();
}

void SwBaseShell::GetBorderState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    // Table cell(s) selected?
    bool bPrepare = true;
    bool bTableMode = rSh.IsTableMode();
    if ( bTableMode )
    {
        SfxItemSetFixed<RES_BOX, RES_BOX,
                         SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER>  aCoreSet( GetPool() );
        SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
        aCoreSet.Put( aBoxInfo );
        rSh.GetTabBorders( aCoreSet );
        rSet.Put( aCoreSet );
    }
    else if ( rSh.IsFrameSelected() )
    {
        SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE, nullptr );
        rSet.Put( aMgr.GetAttrSet() );
        bPrepare = false;
    }
    else
        // Get border attributes via shell quite normal
        rSh.GetCurAttr( rSet );
    if ( bPrepare )
        ::PrepareBoxInfo( rSet, rSh );
    // Switch the border toolbox controller mode
    rSet.Put( SfxBoolItem( SID_BORDER_REDUCED_MODE, !bTableMode ));
}

void SwBaseShell::ExecDlg(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    weld::Window* pMDI = GetView().GetFrameWeld();
    // So that from the basic no dialogues for the background views are called:
    bool bBackground = (&GetView() != GetActiveView());
    const SfxPoolItem* pItem = nullptr;
    const SfxItemSet* pArgs = rReq.GetArgs();

    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pOutSet = nullptr;
    bool bDone = false;
    if(pArgs)
        pArgs->GetItemState( GetPool().GetWhich(nSlot), false, &pItem );

    switch ( nSlot )
    {
        case FN_FORMAT_TITLEPAGE_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateTitlePageDlg(pMDI));
            VclAbstractDialog::AsyncContext aContext;
            aContext.maEndDialogFn = [](sal_Int32){};
            pDlg->StartExecuteAsync(aContext);
        }
        break;
        case FN_FORMAT_PAGE_DLG:
        case FN_FORMAT_PAGE_COLUMN_DLG:
        case FN_FORMAT_PAGE_SETTING_DLG:
        {
            if( !bBackground )
            {
                const size_t nCurIdx = rSh.GetCurPageDesc();
                const SwPageDesc& rPageDesc = rSh.GetPageDesc( nCurIdx );
                // Temporary view, because the shell does not need to be valid after the dialog
                // for example disable header
                SwView& rTempView = GetView();

                OString sPageId;
                switch (nSlot)
                {
                    case FN_FORMAT_PAGE_COLUMN_DLG:
                        sPageId = "columns";
                        break;
                    case FN_FORMAT_PAGE_SETTING_DLG:
                        sPageId = "page";
                        break;
                    case FN_FORMAT_PAGE_DLG:
                        if (pItem)
                          sPageId = OUStringToOString(static_cast<const SfxStringItem*>(pItem)->GetValue(), RTL_TEXTENCODING_UTF8);
                        break;
                }
                rTempView.GetDocShell()->FormatPage(rReq.GetFrameWeld(), rPageDesc.GetName(), sPageId, rSh, &rReq);
                rTempView.InvalidateRulerPos();

                bDone = true; // FormatPage() takes care of calling Done()
            }
        }
        break;
        case FN_FORMAT_BORDER_DLG:
        {
            SfxItemSetFixed<RES_BOX             , RES_SHADOW,
                           SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER> aSet( rSh.GetAttrPool() );
            ScopedVclPtr<SfxAbstractDialog> pDlg;
            // Table cell(s) selected?
            if ( rSh.IsTableMode() )
            {
                // Set border attributes Get/SetTabBorders()
                ::PrepareBoxInfo( aSet, rSh );
                rSh.GetTabBorders( aSet );
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                pDlg.disposeAndReset(pFact->CreateSwBorderDlg(pMDI, aSet, SwBorderModes::TABLE));
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetTabBorders( *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else if ( rSh.IsFrameSelected() )
            {
                // Set border attributes via Frame-Manager
                SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE, nullptr );
                aSet.Put( aMgr.GetAttrSet() );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                pDlg.disposeAndReset(pFact->CreateSwBorderDlg(pMDI, aSet, SwBorderModes::FRAME));
                if ( pDlg->Execute() == RET_OK )
                {
                    aMgr.SetAttrSet( *pDlg->GetOutputItemSet() );
                    aMgr.UpdateFlyFrame();
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else
            {
                // Set border attributes via Shell quite normal
                rSh.GetCurAttr( aSet );
                ::PrepareBoxInfo( aSet, rSh );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                pDlg.disposeAndReset(pFact->CreateSwBorderDlg(pMDI, aSet, SwBorderModes::PARA));
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetAttrSet( *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            if(pOutSet)
            {
                rReq.Done(*pOutSet);
                bDone = true;
            }
        }
        break;
        case FN_FORMAT_BACKGROUND_DLG:
        {
            SfxItemSetFixed<RES_BACKGROUND, RES_BACKGROUND,
                            XATTR_FILL_FIRST, XATTR_FILL_LAST>  aSet( rSh.GetAttrPool() );

            ScopedVclPtr<SfxAbstractDialog> pDlg;
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();

            // Table cell(s) selected?
            if ( rSh.IsTableMode() )
            {
                // Get background attributes of the table and put it in the set
                // tdf#144843 calling GetBoxBackground *requires* an incarnation to be handed over
                std::unique_ptr<SvxBrushItem> aBrush(std::make_unique<SvxBrushItem>(RES_BACKGROUND));
                rSh.GetBoxBackground( aBrush );
                pDlg.disposeAndReset(pFact->CreateSwBackgroundDialog(pMDI, aSet));
                aSet.Put( *aBrush );
                if ( pDlg->Execute() == RET_OK )
                {

                    rSh.SetBoxBackground( pDlg->GetOutputItemSet()->Get( RES_BACKGROUND ) );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else if ( rSh.IsFrameSelected() )
            {

                rSh.GetFlyFrameAttr( aSet );

                pDlg.disposeAndReset(pFact->CreateSwBackgroundDialog(pMDI, aSet));
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetFlyFrameAttr(const_cast<SfxItemSet &>(*pDlg->GetOutputItemSet()) );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else
            {
                // Set border attributes Umrandungsattribute with the shell quite normal.
                rSh.GetCurAttr( aSet );

                pDlg.disposeAndReset(pFact->CreateSwBackgroundDialog(pMDI, aSet));
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetAttrSet( *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            if(pOutSet)
            {
                rReq.Done(*pOutSet);
                bDone = true;
            }
        }
        break;
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
        case SID_ACCESSIBILITY_CHECK:
        {
            sw::AccessibilityCheck aCheck(rSh.GetDoc());
            aCheck.check();
            svx::AccessibilityCheckDialog aDialog(pMDI, aCheck.getIssueCollection());
            aDialog.run();
        }
        break;
#endif
        default:OSL_FAIL("wrong Dispatcher (basesh.cxx)");
    }
    if(!bDone)
        rReq.Done();
}

SwWrtShell& SwBaseShell::GetShell()
{
    return m_rView.GetWrtShell();
}

SwWrtShell* SwBaseShell::GetShellPtr()
{
    return m_rView.GetWrtShellPtr();
}

static void EndUndo(SwWrtShell& rSh)
{
    SwRewriter aRewriter;

    if (rSh.GetTableFormat())
    {
        aRewriter.AddRule(UndoArg1, SwResId(STR_START_QUOTE));
        aRewriter.AddRule(UndoArg2, rSh.GetTableFormat()->GetName());
        aRewriter.AddRule(UndoArg3, SwResId(STR_END_QUOTE));

    }
    rSh.EndUndo(SwUndoId::INSTABLE, &aRewriter); // If possible change the Shell
}

static void InsertTableImpl(SwWrtShell& rSh,
                    SwView &rTempView,
                    const OUString& aTableName,
                    sal_uInt16 nRows,
                    sal_uInt16 nCols,
                    SwInsertTableOptions aInsTableOpts,
                    const OUString& aAutoName,
                    const std::unique_ptr<SwTableAutoFormat>& pTAFormat)
{
    rSh.StartUndo(SwUndoId::INSTABLE);

    rSh.StartAllAction();
    if( rSh.HasSelection() )
        rSh.DelRight();

    rSh.InsertTable( aInsTableOpts, nRows, nCols, pTAFormat.get() );
    rSh.MoveTable( GotoPrevTable, fnTableStart );

    if( !aTableName.isEmpty() && !rSh.GetTableStyle( aTableName ) )
        rSh.GetTableFormat()->SetName( aTableName );

    if( pTAFormat != nullptr && !aAutoName.isEmpty()
                        && aAutoName != SwViewShell::GetShellRes()->aStrNone )
    {
        SwTableNode* pTableNode = const_cast<SwTableNode*>( rSh.IsCursorInTable() );
        if ( pTableNode )
        {
            pTableNode->GetTable().SetTableStyleName( aAutoName );
            SwUndoTableAutoFormat* pUndo = new SwUndoTableAutoFormat( *pTableNode, *pTAFormat );
            if ( pUndo )
                rSh.GetIDocumentUndoRedo().AppendUndo( std::unique_ptr<SwUndo>(pUndo) );
        }
    }

    rSh.EndAllAction();
    rTempView.AutoCaption(TABLE_CAP);
}

void SwBaseShell::InsertTable( SfxRequest& _rRequest )
{
    const SfxItemSet* pArgs = _rRequest.GetArgs();
    SwWrtShell& rSh = GetShell();

    if ( rSh.GetFrameType( nullptr, true ) & FrameTypeFlags::FOOTNOTE )
        return;

    SwView &rTempView = GetView(); // Because GetView() does not work after the shell exchange
    bool bHTMLMode = 0 != (::GetHtmlMode(rTempView.GetDocShell())&HTMLMODE_ON);
    bool bCallEndUndo = false;

    if( !pArgs && rSh.IsSelection() && !rSh.IsInClickToEdit() &&
        !rSh.IsTableMode() )
    {
        const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
        SwInsertTableOptions aInsTableOpts = pModOpt->GetInsTableFlags(bHTMLMode);

        rSh.StartUndo(SwUndoId::INSTABLE);
        bCallEndUndo = true;

        bool bInserted = rSh.TextToTable( aInsTableOpts, '\t' );
        rSh.EnterStdMode();
        if (bInserted)
            rTempView.AutoCaption(TABLE_CAP);
        _rRequest.Done();
    }
    else
    {
        sal_uInt16 nColsIn = 0;
        sal_uInt16 nRowsIn = 0;
        SwInsertTableOptions aInsTableOptsIn( SwInsertTableFlags::All, 1 );
        OUString aTableNameIn;
        OUString aAutoNameIn;
        std::unique_ptr<SwTableAutoFormat> pTAFormatIn;

        if( pArgs && pArgs->Count() >= 2 )
        {
            const SfxStringItem* pName = _rRequest.GetArg<SfxStringItem>(FN_INSERT_TABLE);
            const SfxUInt16Item* pCols = _rRequest.GetArg<SfxUInt16Item>(SID_ATTR_TABLE_COLUMN);
            const SfxUInt16Item* pRows = _rRequest.GetArg<SfxUInt16Item>(SID_ATTR_TABLE_ROW);
            const SfxInt32Item* pFlags = _rRequest.GetArg<SfxInt32Item>(FN_PARAM_1);
            const SfxStringItem* pAuto = _rRequest.GetArg<SfxStringItem>(FN_PARAM_2);

            if ( pName )
                aTableNameIn = pName->GetValue();
            if ( pCols )
                nColsIn = pCols->GetValue();
            if ( pRows )
                nRowsIn = pRows->GetValue();
            if ( pAuto )
            {
                aAutoNameIn = pAuto->GetValue();
                if ( !aAutoNameIn.isEmpty() )
                {
                    SwTableAutoFormatTable aTableTable;
                    aTableTable.Load();
                    for ( size_t n=0; n<aTableTable.size(); n++ )
                    {
                        if ( aTableTable[n].GetName() == aAutoNameIn )
                        {
                            pTAFormatIn.reset(new SwTableAutoFormat( aTableTable[n] ));
                            break;
                        }
                    }
                }
            }

            if ( pFlags )
                aInsTableOptsIn.mnInsMode = static_cast<SwInsertTableFlags>(pFlags->GetValue());
            else
            {
                const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                aInsTableOptsIn = pModOpt->GetInsTableFlags(bHTMLMode);
            }
        }

        if( !nColsIn || !nRowsIn )
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            std::shared_ptr<AbstractInsTableDlg> pAbstractDialog(pFact->CreateInsTableDlg(rTempView));
            std::shared_ptr<weld::DialogController> pDialogController(pAbstractDialog->getDialogController());

            weld::DialogController::runAsync(pDialogController,
                [pAbstractDialog, &rSh, &rTempView, aTableNameIn, nRowsIn, nColsIn, aInsTableOptsIn, aAutoNameIn] (sal_Int32 nResult) {
                    if( RET_OK == nResult )
                    {
                        sal_uInt16 nCols = nColsIn;
                        sal_uInt16 nRows = nRowsIn;
                        SwInsertTableOptions aInsTableOpts = aInsTableOptsIn;
                        OUString aTableName = aTableNameIn;
                        OUString aAutoName = aAutoNameIn;
                        std::unique_ptr<SwTableAutoFormat> pTAFormat;

                        pAbstractDialog->GetValues( aTableName, nRows, nCols, aInsTableOpts, aAutoName, pTAFormat );

                        if( nCols && nRows )
                        {
                            InsertTableImpl( rSh, rTempView, aTableName, nRows, nCols, aInsTableOpts, aAutoName, pTAFormat );
                            EndUndo(rSh);
                        }
                    }
                }
            );
        }
        else
        {
            // record before shell change
            _rRequest.AppendItem( SfxStringItem( FN_INSERT_TABLE, aTableNameIn ) );
            if ( !aAutoNameIn.isEmpty() )
                _rRequest.AppendItem( SfxStringItem( FN_PARAM_2, aAutoNameIn ) );
            _rRequest.AppendItem( SfxUInt16Item( SID_ATTR_TABLE_COLUMN, nColsIn ) );
            _rRequest.AppendItem( SfxUInt16Item( SID_ATTR_TABLE_ROW, nRowsIn ) );
            _rRequest.AppendItem( SfxInt32Item( FN_PARAM_1, static_cast<sal_Int32>(aInsTableOptsIn.mnInsMode) ) );
            _rRequest.Done();

            InsertTableImpl( rSh, rTempView, aTableNameIn, nRowsIn, nColsIn, aInsTableOptsIn, aAutoNameIn, pTAFormatIn );

            bCallEndUndo = true;
        }
    }

    if( bCallEndUndo )
        EndUndo(rSh);
}

void SwBaseShell::GetGalleryState( SfxItemSet &rSet )
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    switch ( nWhich )
    {
        case SID_GALLERY_BG_BRUSH:
        {
            SelectionType nSel = rSh.GetSelectionType();
            SfxStringListItem aLst( nWhich );
            std::vector<OUString> &rLst = aLst.GetList();
            nParagraphPos = nGraphicPos = nOlePos = nFramePos = nTablePos =
            nTableRowPos  = nTableCellPos = nPagePos =
            nHeaderPos    = nFooterPos = 0;
            sal_uInt8 nPos = 1;
            rLst.push_back( SwResId( STR_SWBG_PAGE ) );
            nPagePos = nPos++;
            sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
            bool bHtmlMode = 0 != (nHtmlMode & HTMLMODE_ON);

            if ( (!bHtmlMode || (nHtmlMode & HTMLMODE_FULL_STYLES)) &&
                 (nSel & SelectionType::Text) )
            {
                rLst.push_back( SwResId( STR_SWBG_PARAGRAPH ) );
                nParagraphPos = nPos++;
            }
            if ( (!bHtmlMode || (nHtmlMode & HTMLMODE_SOME_STYLES)) &&
                    nSel & (SelectionType::Table|SelectionType::TableCell) )
            {
                rLst.push_back( SwResId( STR_SWBG_TABLE ) );
                nTablePos = nPos++;

                if(!bHtmlMode)
                {
                    rLst.push_back( SwResId( STR_SWBG_TABLE_ROW ) );
                    nTableRowPos = nPos++;
                }

                rLst.push_back( SwResId( STR_SWBG_TABLE_CELL) );
                nTableCellPos = nPos++;
            }
            if(!bHtmlMode)
            {
                if ( nSel & SelectionType::Frame )
                {
                    rLst.push_back( SwResId( STR_SWBG_FRAME ) );
                    nFramePos = nPos++;
                }
                if ( nSel & SelectionType::Graphic )
                {
                    rLst.push_back( SwResId( STR_SWBG_GRAPHIC ) );
                    nGraphicPos = nPos++;
                }
                if ( nSel & SelectionType::Ole )
                {
                    rLst.push_back( SwResId( STR_SWBG_OLE ) );
                    nOlePos = nPos++;
                }
                const FrameTypeFlags nType = rSh.GetFrameType(nullptr,true);
                if ( nType & FrameTypeFlags::HEADER )
                {
                    rLst.push_back( SwResId( STR_SWBG_HEADER ) );
                    nHeaderPos = nPos++;
                }
                if ( nType & FrameTypeFlags::FOOTER )
                {
                    rLst.push_back( SwResId( STR_SWBG_FOOTER ) );
                    nFooterPos = nPos;
                }
            }
            if ( rLst.empty() )
                rSet.DisableItem( nWhich );
            else
                rSet.Put( aLst );
            break;
        }
    }
}

void SwBaseShell::ExecuteGallery(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    rSh.StartAction();
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch(nSlot)
    {
        case SID_GALLERY_BG_BRUSH:
        {
            if ( !pArgs )
                break;

            SelectionType nSel = rSh.GetSelectionType();
            if ( nSel & SelectionType::DrawObjectEditMode )
                break;

            const SfxUInt16Item* pPos = rReq.GetArg<SfxUInt16Item>(SID_GALLERY_BG_POS);
            const SvxBrushItem* pBrush = rReq.GetArg<SvxBrushItem>(SID_GALLERY_BG_BRUSH);
            if ( !pPos || !pBrush )
                break;

            sal_uInt8 nPos = pPos->GetValue();
            ++nPos;

            SvxBrushItem aBrush( *pBrush );
            aBrush.SetWhich( RES_BACKGROUND );
            if ( nPos == nParagraphPos )
                rSh.SetAttrItem( aBrush );
            else if ( nPos == nTablePos )
                rSh.SetTabBackground( aBrush );
            else if ( nPos == nTableRowPos )
                rSh.SetRowBackground( aBrush );
            else if ( nPos == nTableCellPos )
                rSh.SetBoxBackground( aBrush );
            else if ( nPos == nFramePos || nPos == nGraphicPos || nPos == nOlePos )
            {
                SfxItemSetFixed<RES_BACKGROUND, RES_BACKGROUND> aCoreSet(GetPool());
                aCoreSet.Put( aBrush );
                rSh.SetFlyFrameAttr( aCoreSet );
            }
            else if ( nPos == nPagePos || nPos == nHeaderPos || nPos == nFooterPos )
            {
                sal_uInt16 nDesc = rSh.GetCurPageDesc();
                SwPageDesc aDesc( rSh.GetPageDesc( nDesc ) );
                if ( nPos == nPagePos )
                    aDesc.GetMaster().SetFormatAttr( aBrush );
                else if ( nPos == nHeaderPos )
                {
                    SwFormatHeader aHead( aDesc.GetMaster().GetHeader() );
                    aHead.GetHeaderFormat()->SetFormatAttr( aBrush );
                    aDesc.GetMaster().SetFormatAttr( aHead );
                }
                else if ( nPos == nFooterPos )
                {
                    SwFormatFooter aFoot( aDesc.GetMaster().GetFooter() );
                    aFoot.GetFooterFormat()->SetFormatAttr( aBrush );
                    aDesc.GetMaster().SetFormatAttr( aFoot );
                }
                rSh.ChgPageDesc( nDesc, aDesc );
            }
            break;
        }
    }
    rSh.EndAction();
    rReq.Done();
}

void SwBaseShell::ExecField( SfxRequest const & rReq )
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch( nSlot )
    {
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
        case FN_CHANGE_DBFIELD:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSwChangeDBDlg(GetView()));
            pDlg->Execute();
        }
        break;
#endif
        default:
            OSL_FAIL("wrong dispatcher");
    }
}

std::shared_ptr<std::vector<std::unique_ptr<SwPaM>>> SwBaseShell::CopyPaMRing(SwPaM& rOrig)
{
    auto vCursors = std::make_shared<std::vector<std::unique_ptr<SwPaM>>>();
    vCursors->emplace_back(std::make_unique<SwPaM>(rOrig, nullptr));
    for (auto& rCursor : rOrig.GetRingContainer())
    {
        if (&rCursor != &rOrig)
            vCursors->emplace_back(std::make_unique<SwPaM>(rCursor, vCursors->front().get()));
    }
    return vCursors;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
