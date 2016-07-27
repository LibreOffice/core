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

#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <svl/languageoptions.hxx>
#include <svx/svxids.hrc>
#include <sfx2/linkmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/imapdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <svl/visitem.hxx>
#include <sfx2/objitem.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/gallery.hxx>
#include <editeng/langitem.hxx>
#include <svx/clipfmtitem.hxx>
#include <svx/contdlg.hxx>
#include <vcl/graph.hxx>
#include <svl/slstitm.hxx>
#include <vcl/msgbox.hxx>
#include <svl/ptitem.hxx>
#include <svl/itemiter.hxx>
#include <svl/stritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <svl/srchitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/flagsdef.hxx>
#include <svx/xflclit.hxx>
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
#include <docstat.hxx>
#include <usrfld.hxx>
#include <expfld.hxx>
#include <fldmgr.hxx>
#include <frmmgr.hxx>
#include <tablemgr.hxx>
#include <mdiexp.hxx>
#include <swdtflvr.hxx>
#include <pagedesc.hxx>
#include <convert.hxx>
#include <fmtcol.hxx>
#include <edtwin.hxx>
#include <tblafmt.hxx>
#include <caption.hxx>
#include <swwait.hxx>
#include <cmdid.h>
#include <globals.hrc>
#include <shells.hrc>
#include <statstr.hrc>
#include <globals.h>
#include <unotxdoc.hxx>
#include <fmtinfmt.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include "swabstdlg.hxx"
#include "dialog.hrc"
#include "fldui.hrc"
#include "table.hrc"
#include <modcfg.hxx>
#include <instable.hxx>
#include <svx/fmshell.hxx>
#include <SwRewriter.hxx>
#include <comcore.hrc>
#include <unomid.h>
#include <svx/galleryitem.hxx>
#include <com/sun/star/gallery/GalleryItemType.hpp>
#include <memory>

//UUUU
#include <svx/unobrushitemhelper.hxx>

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

#define SwBaseShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"

namespace
{
    SvxContourDlg* GetContourDlg(SwView &rView)
    {
        SfxChildWindow *pChildWindow = rView.GetViewFrame()->GetChildWindow(
            SvxContourDlgChildWindow::GetChildWindowId());

        return pChildWindow ? static_cast<SvxContourDlg*>(pChildWindow->GetWindow()) : nullptr;
    }

    SvxIMapDlg* GetIMapDlg(SwView &rView)
    {
        SfxChildWindow* pWnd = rView.GetViewFrame()->GetChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
        return pWnd ? static_cast<SvxIMapDlg*>(pWnd->GetWindow()) : nullptr;
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
}


static void lcl_UpdateIMapDlg( SwWrtShell& rSh )
{
    Graphic aGrf( rSh.GetIMapGraphic() );
    GraphicType nGrfType = aGrf.GetType();
    void* pEditObj = GraphicType::NONE != nGrfType && GraphicType::Default != nGrfType
                        ? rSh.GetIMapInventor() : nullptr;
    std::unique_ptr<TargetList> pList(new TargetList);
    rSh.GetView().GetViewFrame()->GetTopFrame().GetTargetList(*pList);

    SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
    rSh.GetFlyFrameAttr( aSet );
    const SwFormatURL &rURL = static_cast<const SwFormatURL&>(aSet.Get( RES_URL ));
    SvxIMapDlgChildWindow::UpdateIMapDlg(
            aGrf, rURL.GetMap(), pList.get(), pEditObj );
}

static bool lcl_UpdateContourDlg( SwWrtShell &rSh, int nSel )
{
    Graphic aGraf( rSh.GetIMapGraphic() );
    GraphicType nGrfType = aGraf.GetType();
    bool bRet = GraphicType::NONE != nGrfType && GraphicType::Default != nGrfType;
    if( bRet )
    {
        OUString aGrfName;
        if ( nSel & nsSelectionType::SEL_GRF )
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
            rSh.DelRight();
            break;

        case FN_BACKSPACE:

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

            SAL_FALLTHROUGH; // otherwise call DelLeft
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
    switch( nId )
    {
        case SID_CUT:
        case SID_COPY:
            rView.GetEditWin().FlushInBuffer();
            if ( rSh.HasSelection() )
            {
                SwTransferable* pTransfer = new SwTransferable( rSh );
/*??*/          uno::Reference< datatransfer::XTransferable > xRef( pTransfer );

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

        case SID_PASTE:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard( &rSh.GetView().GetEditWin() ) );
                if( aDataHelper.GetXTransferable().is()
                    && SwTransferable::IsPaste( rSh, aDataHelper ) )
                {
                    // Temporary variables, because the shell could already be
                    // destroyed after the paste.
                    SwView* pView = &rView;

                    const SfxUInt16Item* pAnchorType = rReq.GetArg<SfxUInt16Item>(FN_PARAM_1);
                    if (pAnchorType)
                        SwTransferable::Paste(rSh, aDataHelper, pAnchorType->GetValue());
                    else
                        SwTransferable::Paste(rSh, aDataHelper);

                    if( rSh.IsFrameSelected() || rSh.IsObjSelected() )
                        rSh.EnterSelFrameMode();
                    pView->AttrChangedNotify( &rSh );
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
                        SwView* pView = &rView;

                        SwTransferable::PasteFormat( rSh, aDataHelper,
                                        static_cast<SotClipboardFormatId>(static_cast<const SfxUInt32Item*>(pFormat)->GetValue()) );

                        //Done() has to be called before the shell has been removed
                        rReq.Done();
                        bIgnore = true;
                        if( rSh.IsFrameSelected() || rSh.IsObjSelected())
                            rSh.EnterSelFrameMode();
                        pView->AttrChangedNotify( &rSh );
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
                    SwView* pView = &rView;
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
                    pView->AttrChangedNotify( &rSh );
                }
                else
                    return;
            }
            break;

        case SID_PASTE_SPECIAL:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard( &rSh.GetView().GetEditWin()) );
                if( aDataHelper.GetXTransferable().is()
                    && SwTransferable::IsPaste( rSh, aDataHelper )
                    && !rSh.CursorInsideInputField() )
                {
                    // Temporary variables, because the shell could already be
                    // destroyed after the paste.
                    SwView* pView = &rView;
                    SotClipboardFormatId nFormatId = SotClipboardFormatId::NONE;
                    rReq.Ignore();
                    bIgnore = true;
                    if(SwTransferable::PasteSpecial( rSh, aDataHelper, nFormatId ))
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
                    pView->AttrChangedNotify( &rSh );
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
            SAL_FALLTHROUGH;
        case SID_COPY:
            if( !bCopy )
                rSet.DisableItem( nWhich );
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
    SwWrtShell &rSh = GetShell();

    sal_uInt16 nId = rReq.GetSlot(), nCnt = 1;
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( nId, false, &pItem ))
        nCnt = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

    // #i106349#: save pointer: undo/redo may delete the shell, i.e., this!
    SfxViewFrame *const pViewFrame( GetView().GetViewFrame() );

    switch( nId )
    {
        case SID_UNDO:
            rSh.LockPaint();
            rSh.Do( SwWrtShell::UNDO, nCnt );
            rSh.UnlockPaint();
            break;

        case SID_REDO:
            rSh.LockPaint();
            rSh.Do( SwWrtShell::REDO, nCnt );
            rSh.UnlockPaint();
            break;

        case SID_REPEAT:
            rSh.Do( SwWrtShell::REPEAT );
            break;
        default:
            OSL_FAIL("wrong Dispatcher");
    }

    if (pViewFrame) { pViewFrame->GetBindings().InvalidateAll(false); }
}

// State of undo

void SwBaseShell::StateUndo(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_UNDO:
            {
                if (rSh.GetLastUndoInfo(nullptr, nullptr))
                {
                    rSet.Put( SfxStringItem(nWhich,
                        rSh.GetDoString(SwWrtShell::UNDO)));
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }
            case SID_REDO:
            {
                if (rSh.GetFirstRedoInfo(nullptr))
                {
                    rSet.Put(SfxStringItem(nWhich,
                        rSh.GetDoString(SwWrtShell::REDO)));
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }
            case SID_REPEAT:
            {   // Repeat is only possible if no REDO is possible - UI-Restriction
                if ((!rSh.GetFirstRedoInfo(nullptr)) &&
                    !rSh.IsSelFrameMode() &&
                    (UNDO_EMPTY != rSh.GetRepeatInfo(nullptr)))
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
                if (rSh.GetFirstRedoInfo(nullptr))
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
                Reference < XUnoTunnel > xDocTunnel ( xModel, UNO_QUERY );
                SwXTextDocument *pDoc = reinterpret_cast < SwXTextDocument * > ( xDocTunnel->getSomething ( SwXTextDocument::getUnoTunnelId() ) );
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
                SwWait aWait( *rView.GetDocShell(), true );
                rSh.UpdateAllCharts();
            }
            break;

        case FN_UPDATE_ALL:
            {
                SwView&  rTempView = GetView();
                rSh.EnterStdMode();
                if( !rSh.GetLinkManager().GetLinks().empty() )
                {
                    rSh.StartAllAction();
                    rSh.GetLinkManager().UpdateAllLinks( false, true, true );
                    rSh.EndAllAction();
                }
                SfxDispatcher &rDis = *rTempView.GetViewFrame()->GetDispatcher();
                rDis.Execute( FN_UPDATE_FIELDS );
                rDis.Execute( FN_UPDATE_TOX );
                rDis.Execute( FN_UPDATE_CHARTS );
                rSh.CalcLayout();
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
            SwFieldType* pFieldType = aFieldMgr.GetFieldType(RES_JUMPEDITFLD);

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
                    rSh.ClickToField(*pCurField);
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
                    GetView().AttrChangedNotify( &rSh );
                }
        }
        break;
        case SID_GALLERY_FORMATS:
        {
            const SvxGalleryItem* pGalleryItem = SfxItemSet::GetItem<SvxGalleryItem>(pArgs, SID_GALLERY_FORMATS, false);
            if ( !pGalleryItem )
                break;

            const int nSelType = rSh.GetSelectionType();
            sal_Int8 nGalleryItemType( pGalleryItem->GetType() );

            if ( (!rSh.IsSelFrameMode() || nSelType & nsSelectionType::SEL_GRF) &&
                nGalleryItemType == css::gallery::GalleryItemType::GRAPHIC )
            {
                SwWait aWait( *rView.GetDocShell(), true );

                OUString aGrfName, aFltName;
                const Graphic aGrf( pGalleryItem->GetGraphic() );

                if ( nSelType & nsSelectionType::SEL_GRF )
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
                    nCount = static_cast<const SfxUInt16Item &>(pArgs->Get(SID_ATTR_COLUMNS)).GetValue();
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
            SwInsertTableOptions aInsTableOpts( tabopts::ALL_TBL_INS_ATTR, 1 );
            SwTableAutoFormat const* pTAFormat = nullptr;
            std::unique_ptr<SwTableAutoFormatTable> pAutoFormatTable;
            bool bDeleteFormat = true;
            if(pArgs && SfxItemState::SET == pArgs->GetItemState( FN_PARAM_1, true, &pItem))
            {
                aInsTableOpts.mnInsMode = 0;
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
                    aInsTableOpts.mnInsMode |= tabopts::HEADLINE;
                // RepeatHeaderLines
                if(SfxItemState::SET == pArgs->GetItemState( FN_PARAM_4, true, &pItem))
                   aInsTableOpts.mnRowsToRepeat =
                            (sal_uInt16)static_cast< const SfxInt16Item* >(pItem)->GetValue();
                //WithBorder
                if(SfxItemState::SET == pArgs->GetItemState( FN_PARAM_5, true, &pItem) &&
                    static_cast< const SfxBoolItem* >(pItem)->GetValue())
                    aInsTableOpts.mnInsMode |= tabopts::DEFAULT_BORDER;
                //DontSplitTable
                if(SfxItemState::SET == pArgs->GetItemState( FN_PARAM_6, true, &pItem) &&
                    !static_cast< const SfxBoolItem* >(pItem)->GetValue() )
                    aInsTableOpts.mnInsMode |= tabopts::SPLIT_LAYOUT;
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                std::unique_ptr<AbstractSwConvertTableDlg> pDlg(pFact->CreateSwConvertTableDlg(GetView(), bToTable));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
                if( RET_OK == pDlg->Execute() )
                {
                    pDlg->GetValues( cDelim, aInsTableOpts, pTAFormat );

                }
            }

            if( cDelim )
            {
                //Shell change!
                SwView& rSaveView = rView;
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
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_3, 0 != (aInsTableOpts.mnInsMode & tabopts::HEADLINE)));
                        aReq.AppendItem( SfxInt16Item( FN_PARAM_4, (short)aInsTableOpts.mnRowsToRepeat ));
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_5, 0 != (aInsTableOpts.mnInsMode & tabopts::DEFAULT_BORDER) ));
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_6, !(aInsTableOpts.mnInsMode & tabopts::SPLIT_LAYOUT)));
                    }
                    aReq.Done();
                }

                if( !bToTable )
                    rSh.TableToText( cDelim );
                else
                {
                    bInserted = rSh.TextToTable( aInsTableOpts, cDelim, text::HoriOrientation::FULL, pTAFormat );
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
            ShellModes eMode = GetView().GetShellMode();
            if ( SHELL_MODE_DRAW != eMode &&
                 SHELL_MODE_DRAW_CTRL != eMode &&
                 SHELL_MODE_DRAW_FORM != eMode &&
                 SHELL_MODE_DRAWTEXT != eMode &&
                 SHELL_MODE_BEZIER != eMode )
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
                    SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                    rSh.GetFlyFrameAttr( aSet );
                    SwFormatURL aURL( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );
                    aURL.SetMap( &pDlg->GetImageMap() );
                    aSet.Put( aURL );
                    rSh.SetFlyFrameAttr( aSet );
            }
        }
        break;
        case SID_CONTOUR_DLG:
        {
            sal_uInt16 nId = SvxContourDlgChildWindow::GetChildWindowId();

            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            pVFrame->ToggleChildWindow( nId );
            pVFrame->GetBindings().Invalidate( SID_CONTOUR_DLG );

            int nSel = rSh.GetSelectionType();
            if ( pVFrame->HasChildWindow( nId ) &&
                 (nSel & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_OLE)) )
            {
                lcl_UpdateContourDlg( rSh, nSel );
            }
        }
        break;
        case SID_CONTOUR_EXEC:
        {
            SvxContourDlg *pDlg = GetContourDlg(GetView());
            // Check, if the allocation is useful or allowed at all.
            int nSel = rSh.GetSelectionType();
            if ( nSel & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_OLE) )
            {
                if (pDlg && pDlg->GetEditingObject() == rSh.GetIMapInventor())
                {
                    rSh.StartAction();
                    SfxItemSet aSet( rSh.GetAttrPool(), RES_SURROUND, RES_SURROUND);
                    rSh.GetFlyFrameAttr( aSet );
                    SwFormatSurround aSur( static_cast<const SwFormatSurround&>(aSet.Get( RES_SURROUND )) );
                    if ( !aSur.IsContour() )
                    {
                        aSur.SetContour( true );
                        if ( aSur.GetSurround() == SURROUND_NONE )
                            aSur.SetSurround( SURROUND_PARALLEL );
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
        case FN_TOOL_ANCHOR:
            break;
        case FN_TOOL_ANCHOR_PAGE:
        case FN_TOOL_ANCHOR_PARAGRAPH:
        case FN_TOOL_ANCHOR_CHAR:
        case FN_TOOL_ANCHOR_AT_CHAR:
        case FN_TOOL_ANCHOR_FRAME:
        {
            RndStdIds eSet = nSlot == FN_TOOL_ANCHOR_PAGE
                                ? FLY_AT_PAGE
                                : nSlot == FN_TOOL_ANCHOR_PARAGRAPH
                                    ? FLY_AT_PARA
                                    : nSlot == FN_TOOL_ANCHOR_FRAME
                                        ? FLY_AT_FLY
                                        : nSlot == FN_TOOL_ANCHOR_CHAR
                                            ? FLY_AS_CHAR
                                            : FLY_AT_CHAR;
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
                    && FLY_AS_CHAR == eSet && rSh.GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT );
            if (bDoMathBaselineAlignment)
                rSh.AlignFormulaToBaseline( xObj );

            sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
            if( nHtmlMode )
            {
                SfxItemSet aSet(GetPool(), RES_SURROUND, RES_HORI_ORIENT);
                rSh.GetFlyFrameAttr(aSet);

                const SwFormatSurround& rSurround = static_cast<const SwFormatSurround&>(aSet.Get(RES_SURROUND));
                const SwFormatVertOrient& rVert = static_cast<const SwFormatVertOrient&>(aSet.Get(RES_VERT_ORIENT));
                const SwFormatHoriOrient& rHori = static_cast<const SwFormatHoriOrient&>(aSet.Get(RES_HORI_ORIENT));
                sal_Int16 eVOrient = rVert.GetVertOrient();
                sal_Int16 eHOrient = rHori.GetHoriOrient();
                SwSurround eSurround = rSurround.GetSurround();

                switch( eSet )
                {
                case FLY_AT_FLY:
                case FLY_AT_PAGE:
                    //Wrap through, left or from left, top, from top
                    if(eSurround != SURROUND_THROUGHT)
                        aSet.Put(SwFormatSurround(SURROUND_THROUGHT));

                    if( eVOrient != text::VertOrientation::TOP && eVOrient != text::VertOrientation::NONE)
                        aSet.Put(SwFormatVertOrient(0, text::VertOrientation::TOP));

                    if (eHOrient != text::HoriOrientation::NONE && eHOrient != text::HoriOrientation::LEFT)
                        aSet.Put(SwFormatHoriOrient(0, text::HoriOrientation::LEFT));
                    break;

                case FLY_AT_PARA:
                    // left, from left, right, top, no wrap, wrap left and right
                    if (eSurround != SURROUND_LEFT && eSurround != SURROUND_RIGHT)
                        aSet.Put(SwFormatSurround(SURROUND_LEFT));

                    if( eVOrient != text::VertOrientation::TOP)
                        aSet.Put(SwFormatVertOrient(0, text::VertOrientation::TOP));

                    if (eHOrient != text::HoriOrientation::NONE && eHOrient != text::HoriOrientation::LEFT && eHOrient != text::HoriOrientation::RIGHT)
                        aSet.Put(SwFormatHoriOrient(0, text::HoriOrientation::LEFT));
                    break;

                case FLY_AT_CHAR:
                    // left, from left, right, top, wrap through
                    if(eSurround != SURROUND_THROUGHT)
                        aSet.Put(SwFormatSurround(SURROUND_THROUGHT));

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

            GetView().GetViewFrame()->GetBindings().Invalidate( FN_TOOL_ANCHOR );
        }
        break;

        case FN_FRAME_NOWRAP:
        case FN_FRAME_WRAP:
        case FN_FRAME_WRAP_IDEAL:
        case FN_FRAME_WRAPTHRU:
        case FN_FRAME_WRAPTHRU_TRANSP:
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
                    rSh.GetLinkManager().UpdateAllLinks( false, false );
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

                    // set form design mode
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
    if(bMore && pArgs)
    {
        pItem = nullptr;
        pArgs->GetItemState(GetPool().GetWhich(nSlot), false, &pItem);
        if(pItem)
        switch(nSlot)
        {
        case SID_ATTR_BRUSH:
        case SID_ATTR_BORDER_SHADOW:
        case RES_SHADOW:
        {
            rSh.StartAllAction();
            SfxItemSet   aSet( rSh.GetAttrPool(),
                                RES_SHADOW, RES_SHADOW,
                                RES_BACKGROUND, RES_BACKGROUND, 0 );

            aSet.Put(*pItem);
            // Tabele cell(s) selected?
            if ( rSh.IsTableMode() )
            {
                SwFrameFormat *pFormat = rSh.GetTableFormat();
                pFormat->SetFormatAttr( *pItem );
            }
            else if ( rSh.IsFrameSelected() )
            {
                // Set border attributes via Frame-Manager.
                SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE );
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
            // Tabele cell(s) selected?
            if ( rSh.IsTableMode() )
            {
                // Set border attributes Get/SetTabBorders()
                rSh.SetTabBorders(*pArgs);
            }
            else if ( rSh.IsFrameSelected() )
            {
                // Set border attributes via Frame-Manager.
                SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE );
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
}

// Here the state fpr SID_IMAP / SID_CONTOUR will be handled
// until the swapping of the graphic is finished.

IMPL_LINK_NOARG_TYPED(SwBaseShell, GraphicArrivedHdl, SwCursorShell&, void)
{
    GraphicType nGrfType;
    SwWrtShell &rSh = GetShell();
    if( CNT_GRF == rSh.SwEditShell::GetCntType() &&
        GraphicType::NONE != ( nGrfType = rSh.GetGraphicType() ) &&
        !aGrfUpdateSlots.empty() )
    {
        bool bProtect = FlyProtectFlags::NONE != rSh.IsSelObjProtected(FlyProtectFlags::Content|FlyProtectFlags::Parent);
        SfxViewFrame* pVFrame = GetView().GetViewFrame();
        sal_uInt16 nSlot;
        std::set<sal_uInt16>::iterator it;
        for( it = aGrfUpdateSlots.begin(); it != aGrfUpdateSlots.end(); ++it )
        {
            bool bSetState = false;
            bool bState = false;
            switch( nSlot = *it )
            {
            case SID_IMAP:
            case SID_IMAP_EXEC:
                {
                    sal_uInt16 nId = SvxIMapDlgChildWindow::GetChildWindowId();
                    SfxChildWindow *pChildWindow = pVFrame->HasChildWindow(nId) ?
                        pVFrame->GetChildWindow(nId) : nullptr;
                    SvxIMapDlg *pDlg = pChildWindow ?
                        static_cast<SvxIMapDlg*>(pChildWindow->GetWindow()) : nullptr;

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
                        static_cast<SvxIMapDlg*>(pChildWindow->GetWindow()) : nullptr;
                    if( pDlg && pDlg->GetEditingObject() !=
                                rSh.GetIMapInventor() )
                        lcl_UpdateContourDlg( rSh, nsSelectionType::SEL_GRF );

                    bSetState = true;
                    bState = nullptr != pDlg;
                }
                break;

            case FN_FRAME_WRAP_CONTOUR:
                if( !bProtect )
                {
                    SfxItemSet aSet(GetPool(), RES_SURROUND, RES_SURROUND);
                    rSh.GetFlyFrameAttr(aSet);
                    const SwFormatSurround& rWrap = static_cast<const SwFormatSurround&>(aSet.Get(RES_SURROUND));
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
                if( pGetStateSet )
                    pGetStateSet->Put( aBool );
                else
                    pVFrame->GetBindings().SetState( aBool );
            }
        }
        aGrfUpdateSlots.clear();
    }
}

void SwBaseShell::GetState( SfxItemSet &rSet )
{
    SwWrtShell &rSh = GetShell();
    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    pGetStateSet = &rSet;
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_GALLERY_FORMATS:
                if ( rSh.IsObjSelected() ||
                     (rSh.IsSelFrameMode() &&
                      !(rSh.GetSelectionType() & nsSelectionType::SEL_GRF)) )
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
                SfxItemSet   aSet( rSh.GetAttrPool(),
                                    RES_SHADOW, RES_SHADOW );

                // Table cell(s) selected?
                if ( rSh.IsTableMode() )
                {
                    SwFrameFormat *pFormat = rSh.GetTableFormat();
                    aSet.Put(pFormat->GetFormatAttr( nWhich ));
                }
                else if( rSh.IsFrameSelected() )
                {
                    SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE );
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
                                rSh.GetSelectionType() == nsSelectionType::SEL_GRF;

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
                    if(rSh.GetSelectionType() == nsSelectionType::SEL_GRF
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
                    int nSel = rSh.GetSelectionType();
                    bool bOk = 0 != (nSel & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_OLE));

                    bool bDisable = false;
                    if( !bHas && !bOk )
                        bDisable = true;
                    // #i59688#
                    // Avoid unnecessary loading of selected graphic.
                    // The graphic is only needed, if the dialog is open.
                    // If the swapping of the graphic is finished, the status
                    // must be determined asynchronously, until this the slot
                    // will be disabled.
                    else if ( bHas && (nSel & nsSelectionType::SEL_GRF) &&
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
                        if ( nSel & nsSelectionType::SEL_GRF )
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
                int nSel = rSh.GetSelectionType();
                if( !(nSel & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_OLE)) )
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

            case FN_TOOL_ANCHOR:
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
                    SfxItemSet aSet(GetPool(), RES_ANCHOR, RES_ANCHOR);
                    if(bObj)
                        rSh.GetObjAttr(aSet);
                    else
                        rSh.GetFlyFrameAttr(aSet);
                    RndStdIds eSet = static_cast<const SwFormatAnchor&>(aSet.Get(RES_ANCHOR)).GetAnchorId();
                    const bool bSet =
                           ((nWhich == FN_TOOL_ANCHOR_PAGE) &&
                            (eSet == FLY_AT_PAGE))
                        || ((nWhich == FN_TOOL_ANCHOR_PARAGRAPH) &&
                            (eSet == FLY_AT_PARA))
                        || ((nWhich == FN_TOOL_ANCHOR_FRAME) &&
                            (eSet == FLY_AT_FLY))
                        || ((nWhich == FN_TOOL_ANCHOR_AT_CHAR) &&
                            (eSet == FLY_AT_CHAR))
                        || ((nWhich == FN_TOOL_ANCHOR_CHAR) &&
                            (eSet == FLY_AS_CHAR));
                    if(nWhich != FN_TOOL_ANCHOR)
                    {
                        if( nWhich == FN_TOOL_ANCHOR_FRAME && !rSh.IsFlyInFly() )
                            rSet.DisableItem(nWhich);
                        else
                            rSet.Put(SfxBoolItem(nWhich, bSet));
                    }
                    else
                    {
                        sal_uInt16 nSlotId = 0;

                        switch (eSet)
                        {
                            case FLY_AT_PAGE:
                                nSlotId = FN_TOOL_ANCHOR_PAGE;
                            break;
                            case FLY_AT_PARA:
                                nSlotId = FN_TOOL_ANCHOR_PARAGRAPH;
                            break;
                            case FLY_AS_CHAR:
                                nSlotId = FN_TOOL_ANCHOR_CHAR;
                            break;
                            case FLY_AT_CHAR:
                                nSlotId = FN_TOOL_ANCHOR_AT_CHAR;
                            break;
                            case FLY_AT_FLY:
                                nSlotId = FN_TOOL_ANCHOR_FRAME;
                            break;
                            default:
                                ;
                        }
                        rSet.Put(SfxUInt16Item(nWhich, nSlotId));
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
            case FN_FRAME_WRAP_CONTOUR:
            case FN_WRAP_ANCHOR_ONLY:
            case FN_FRAME_WRAP_LEFT:
            case FN_FRAME_WRAP_RIGHT:
            {
                bool bObj = 0 != rSh.IsObjSelected();
                bool bParentCntProt = rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;

                if( !bParentCntProt && (bObj || rSh.IsFrameSelected()))
                {
                    SfxItemSet aSet(GetPool(), RES_OPAQUE, RES_ANCHOR);
                    int nAnchorType;
                    if(bObj)
                    {
                        rSh.GetObjAttr(aSet);
                        nAnchorType = rSh.GetAnchorId();
                    }
                    else
                    {
                        rSh.GetFlyFrameAttr(aSet);
                        nAnchorType = static_cast<const SwFormatAnchor&>(aSet.Get(RES_ANCHOR)).GetAnchorId();
                    }
                    const SwFormatSurround& rWrap = static_cast<const SwFormatSurround&>(aSet.Get(RES_SURROUND));

                    const SvxOpaqueItem& rOpaque = static_cast<const SvxOpaqueItem&>(aSet.Get(RES_OPAQUE));
                    bool bOpaque = rOpaque.GetValue();
                    SwSurround nSurround = rWrap.GetSurround();
                    bool bSet = false;

                    bool bDisable =
                        (nAnchorType == - 1) || (nAnchorType == FLY_AS_CHAR);
                    const bool bHtmlMode =
                        0 != ::GetHtmlMode(GetView().GetDocShell());

                    switch( nWhich )
                    {
                        case FN_FRAME_NOWRAP:
                            bDisable |=
                                (   (nAnchorType != FLY_AT_PARA)
                                 && (nAnchorType != FLY_AT_CHAR)
                                 && (nAnchorType != FLY_AT_PAGE));
                            bSet = nSurround == SURROUND_NONE;
                        break;
                        case FN_FRAME_WRAP:
                            bDisable |= bHtmlMode;
                            bSet = nSurround == SURROUND_PARALLEL;
                        break;
                        case FN_FRAME_WRAP_IDEAL:
                            bDisable |= bHtmlMode;
                            bSet = nSurround == SURROUND_IDEAL;
                        break;
                        case FN_FRAME_WRAPTHRU:
                            bDisable |= (bHtmlMode ||
                                (   (nAnchorType != FLY_AT_PARA)
                                 && (nAnchorType != FLY_AT_CHAR)
                                 && (nAnchorType != FLY_AT_PAGE)));
                            if(bObj)
                                bSet = nSurround == SURROUND_THROUGHT && rSh.GetLayerId();
                            else
                                bSet = nSurround == SURROUND_THROUGHT && bOpaque;
                        break;
                        case FN_FRAME_WRAPTHRU_TRANSP:
                            bDisable |= bHtmlMode;
                            if(bObj)
                                bSet = nSurround == SURROUND_THROUGHT && !rSh.GetLayerId();
                            else
                                bSet = nSurround == SURROUND_THROUGHT && !bOpaque;
                        break;
                        case FN_FRAME_WRAP_CONTOUR:
                            bDisable |= bHtmlMode;
                            //no contour available whenn no wrap or wrap through is set
                            bDisable |= (nSurround == SURROUND_NONE || nSurround == SURROUND_THROUGHT);
                            if( !bDisable )
                            {
                                int nSel = rSh.GetSelectionType();
                                if( (nSel & nsSelectionType::SEL_GRF) &&
                                            rSh.IsLinkedGrfSwapOut())
                                {
                                    if( AddGrfUpdateSlot( nWhich ))
                                        rSh.GetGraphic(false);  // start the loading
                                }
                                else if( rSh.IsFrameSelected() )
                                {
                                    // #i102253# applied patch from OD (see task)
                                    bDisable =
                                        nSel & nsSelectionType::SEL_FRM ||
                                        GraphicType::NONE == rSh.GetIMapGraphic().GetType();
                                }
                            }
                            bSet = !bDisable && rWrap.IsContour();

                        break;
                        case FN_WRAP_ANCHOR_ONLY:
                            bDisable |= (bHtmlMode ||
                                (nAnchorType != FLY_AT_PARA));
                            bSet = rWrap.IsAnchorOnly();
                        break;
                        case FN_FRAME_WRAP_LEFT:
                            bSet = nSurround == SURROUND_LEFT;
                        break;
                        case FN_FRAME_WRAP_RIGHT:
                            bSet = nSurround == SURROUND_RIGHT;
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
    pGetStateSet = nullptr;
}

// Disable the slots with this status methode

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

// Disable the slots with this status methode

void SwBaseShell::StateStyle( SfxItemSet &rSet )
{
    bool bParentCntProt = GetShell().IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;
    ShellModes eMode = GetView().GetShellMode();

    if ( bParentCntProt ||
         SHELL_MODE_DRAW == eMode ||
         SHELL_MODE_DRAW_CTRL == eMode ||
         SHELL_MODE_DRAW_FORM == eMode ||
         SHELL_MODE_DRAWTEXT == eMode ||
         SHELL_MODE_BEZIER == eMode )
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
    if( bObj || rSh.IsFrameSelected())
    {
        SfxItemSet aSet(GetPool(), RES_OPAQUE, RES_SURROUND);
        if(bObj)
            rSh.GetObjAttr(aSet);
        else
            rSh.GetFlyFrameAttr(aSet);
        SwFormatSurround aWrap( static_cast<const SwFormatSurround&>(aSet.Get(RES_SURROUND)) );
        SwSurround nOldSurround(aWrap.GetSurround());
        SwSurround nSurround = SURROUND_PARALLEL;

        switch (nSlot)
        {
            case FN_FRAME_NOWRAP:
                nSurround = SURROUND_NONE;
                if (aWrap.IsContour())
                    aWrap.SetContour(false);
                break;
            case FN_FRAME_WRAP_IDEAL:
                nSurround = SURROUND_IDEAL;
                break;
            case FN_WRAP_ANCHOR_ONLY:
                aWrap.SetAnchorOnly(!aWrap.IsAnchorOnly());

                // keep previous wrapping

                // switch to wrap SURROUND_PARALLEL, if previous wrap is SURROUND_NONE
                if ( nOldSurround != SURROUND_NONE )
                {
                    nSurround = nOldSurround;
                }
                break;
            case FN_FRAME_WRAP_CONTOUR:
                aWrap.SetContour(!aWrap.IsContour());
                break;
            case FN_FRAME_WRAPTHRU_TRANSP:
                if (aWrap.IsContour())
                    aWrap.SetContour(false);
                SAL_FALLTHROUGH;
            case FN_FRAME_WRAPTHRU:
                nSurround = SURROUND_THROUGHT;
                break;

            case FN_FRAME_WRAP_LEFT:
                nSurround = SURROUND_LEFT;
                break;

            case FN_FRAME_WRAP_RIGHT:
                nSurround = SURROUND_RIGHT;
                break;

            default:
                break;
        }
        aWrap.SetSurround(nSurround);

        if (nSlot != FN_FRAME_WRAP_CONTOUR)
        {
            // Defaulting the contour wrap on draw objects.
            if (bObj && nOldSurround != nSurround &&
                (nOldSurround == SURROUND_NONE || nOldSurround == SURROUND_THROUGHT))
            {
                aWrap.SetContour(true);
            }
        }

        aSet.Put( aWrap );
        aSet.Put(SvxOpaqueItem(RES_OPAQUE, nSlot != FN_FRAME_WRAPTHRU_TRANSP));
        if(bObj)
        {
            rSh.SetObjAttr(aSet);
            if (nSlot != FN_FRAME_WRAPTHRU_TRANSP)
                rSh.SelectionToHeaven();
            else
                rSh.SelectionToHell();
        }
        else
            rSh.SetFlyFrameAttr(aSet);
    }
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
    rView(rVw),
    pGetStateSet(nullptr)
{
    SwWrtShell& rWrtSh = rView.GetWrtShell();

    SetPool(&rWrtSh.GetAttrPool());
    SetName("Base");
    rWrtSh.SetGrfArrivedLnk( LINK( this, SwBaseShell, GraphicArrivedHdl));
}

SwBaseShell::~SwBaseShell()
{
    if( rView.GetCurShell() == this )
        rView.ResetSubShell();

    Link<SwCursorShell&,void> aTmp( LINK( this, SwBaseShell, GraphicArrivedHdl));
    if( aTmp == rView.GetWrtShell().GetGrfArrivedLnk() )
        rView.GetWrtShell().SetGrfArrivedLnk( Link<SwCursorShell&,void>() );
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
        SfxItemSet aHeightSet( GetPool(),  RES_CHRATR_FONTSIZE, RES_CHRATR_FONTSIZE,
                                            RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_FONTSIZE,
                                            RES_CHRATR_CTL_FONTSIZE, RES_CHRATR_CTL_FONTSIZE,
                                        0L);

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
                SAL_FALLTHROUGH;
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

                    SfxItemSet aLangSet( GetPool(), RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
                                                    RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
                                                    RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
                                                    0L);
                    rSh.GetCurAttr( aLangSet );

                    sal_Int32 nWesternSize =
                            pStdFont->GetFontHeight(FONT_STANDARD, FONT_GROUP_DEFAULT,
                            static_cast<const SvxLanguageItem&>(aLangSet.Get( RES_CHRATR_LANGUAGE)).GetLanguage());
                    sal_Int32 nCJKSize =
                            pStdFont->GetFontHeight(FONT_STANDARD, FONT_GROUP_CJK,
                            static_cast<const SvxLanguageItem&>(aLangSet.Get( RES_CHRATR_CJK_LANGUAGE)).GetLanguage());
                    sal_Int32 nCTLSize =
                            pStdFont->GetFontHeight(FONT_STANDARD, FONT_GROUP_CTL,
                            static_cast<const SvxLanguageItem&>(aLangSet.Get( RES_CHRATR_CTL_LANGUAGE)).GetLanguage());

                    switch(nScripts)
                    {
                        case SvtScriptType::LATIN:
                            nCJKSize = nHeight * nCJKSize / nWesternSize;
                            nCTLSize = nHeight * nCTLSize / nWesternSize;
                            nWesternSize = (sal_Int32) nHeight;
                        break;
                        case SvtScriptType::ASIAN:
                            nCTLSize = nHeight* nCTLSize / nCJKSize;
                            nWesternSize = nHeight * nWesternSize / nCJKSize;
                            nCJKSize = (sal_Int32) nHeight;
                        break;
                        case SvtScriptType::COMPLEX:
                            nCJKSize = nHeight * nCJKSize / nCTLSize;
                            nWesternSize = nHeight * nWesternSize / nCTLSize;
                            nCTLSize = (sal_Int32) nHeight;
                        break;
                        default: break;
                    }
                    aHeightSet.Put( SvxFontHeightItem( (sal_uInt32)nWesternSize, 100, RES_CHRATR_FONTSIZE ));
                    aHeightSet.Put( SvxFontHeightItem( (sal_uInt32)nCJKSize, 100, RES_CHRATR_CJK_FONTSIZE ));
                    aHeightSet.Put( SvxFontHeightItem( (sal_uInt32)nCTLSize, 100, RES_CHRATR_CTL_FONTSIZE ));
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
    std::unique_ptr<SfxItemSet> pFntCoreSet;
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
                    pFntCoreSet.reset(new SfxItemSet( *rSet.GetPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END-1 ));
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
                    std::unique_ptr<SfxPoolItem> pNewItem(pI->CloneSetWhich(nWhich));
                    rSet.Put( *pNewItem );
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
                    aFont.SetOrientation(bVertical ? 2700 : 0);
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
    int nSelType(rSh.GetSelectionType());
    SvxBrushItem aBrushItem(RES_BACKGROUND);

    if( nsSelectionType::SEL_TBL_CELLS & nSelType )
    {
        rSh.GetBoxBackground( aBrushItem );
    }
    else
    {
        //UUUU Adapt to new DrawingLayer FillStyle; use a parent which has XFILL_NONE set
        SfxItemSet aCoreSet(GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

        aCoreSet.SetParent(&GetView().GetDocShell()->GetDoc()->GetDfltFrameFormat()->GetAttrSet());

        if(nSelType & nsSelectionType::SEL_GRF || nsSelectionType::SEL_FRM & nSelType)
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
            {
                SvxColorItem aColorItem(aBrushItem.GetColor(),SID_BACKGROUND_COLOR);
                rSet.Put(aColorItem);
                break;
            }
            case SID_ATTR_BRUSH:
            case RES_BACKGROUND:
            {
                std::unique_ptr<SfxPoolItem> pNewItem(aBrushItem.CloneSetWhich(GetPool().GetWhich(nWhich)));
                rSet.Put(*pNewItem);
                break;
            }
        }

        nWhich = aIter.NextWhich();
    }
}

void SwBaseShell::ExecBckCol(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    int nSelType(rSh.GetSelectionType());
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSlot(rReq.GetSlot());

    if(!pArgs  && nSlot != SID_BACKGROUND_COLOR)
    {
        return;
    }

    SvxBrushItem aBrushItem(RES_BACKGROUND);

    if( nsSelectionType::SEL_TBL_CELLS & nSelType )
    {
        rSh.GetBoxBackground( aBrushItem );
    }
    else
    {
        //UUUU Adapt to new DrawingLayer FillStyle; use a parent which has XFILL_NONE set
        SfxItemSet aCoreSet(GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

        aCoreSet.SetParent(&GetView().GetDocShell()->GetDoc()->GetDfltFrameFormat()->GetAttrSet());

        if((nsSelectionType::SEL_FRM & nSelType) || (nsSelectionType::SEL_GRF & nSelType))
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
        {
            aBrushItem.SetGraphicPos(GPOS_NONE);

            if(pArgs)
            {
                const SvxColorItem& rNewColorItem = static_cast<const SvxColorItem&>(pArgs->Get(SID_BACKGROUND_COLOR));
                const Color& rNewColor = rNewColorItem.GetValue();
                aBrushItem.SetColor(rNewColor);
                GetView().GetViewFrame()->GetBindings().SetState(rNewColorItem);
            }
            else
            {
                aBrushItem.SetColor(COL_TRANSPARENT);
                rReq.AppendItem(SvxColorItem(Color(COL_TRANSPARENT),nSlot));
            }
            break;
        }

        case SID_ATTR_BRUSH:
        case RES_BACKGROUND:
        {
            const SvxBrushItem& rNewBrushItem = static_cast<const SvxBrushItem&>(pArgs->Get(GetPool().GetWhich(nSlot)));
            aBrushItem = rNewBrushItem;
            break;
        }
        default:
        {
            rReq.Ignore();
            OSL_FAIL("unknown message in ExecuteAttr!" );
            return;
        }
    }

    if( nsSelectionType::SEL_TBL_CELLS & nSelType )
    {
        rSh.SetBoxBackground( aBrushItem );
    }
    else
    {
        //UUUU Adapt to new DrawingLayer FillStyle; use a parent which has XFILL_NONE set
        SfxItemSet aCoreSet(GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

        aCoreSet.SetParent(&GetView().GetDocShell()->GetDoc()->GetDfltFrameFormat()->GetAttrSet());
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, aCoreSet);

        if((nsSelectionType::SEL_FRM & nSelType) || (nsSelectionType::SEL_GRF & nSelType))
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
    // Tabele cell(s) selected?
    bool bPrepare = true;
    bool bTableMode = rSh.IsTableMode();
    if ( bTableMode )
    {
        SfxItemSet aCoreSet( GetPool(),
                             RES_BOX, RES_BOX,
                             SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0 );
        SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
        aCoreSet.Put( aBoxInfo );
        rSh.GetTabBorders( aCoreSet );
        rSet.Put( aCoreSet );
    }
    else if ( rSh.IsFrameSelected() )
    {
        SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE );
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
    vcl::Window *pMDI = &GetView().GetViewFrame()->GetWindow();
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
            std::unique_ptr<VclAbstractDialog> pDlg(pFact->CreateTitlePageDlg( pMDI ));
            pDlg->Execute();
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
                // Temporary view, because the shell does not need to be valid after the dialogue
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
                }

                rTempView.GetDocShell()->FormatPage(
                    rPageDesc.GetName(), sPageId, rSh);
                rTempView.InvalidateRulerPos();
            }
        }
        break;
        case FN_FORMAT_BORDER_DLG:
        {
            SfxItemSet   aSet( rSh.GetAttrPool(),
                               RES_BOX              , RES_SHADOW,
                               SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                               0 );
            std::unique_ptr<SfxAbstractDialog> pDlg;
            // Table cell(s) selected?
            if ( rSh.IsTableMode() )
            {
                // Set border attributes Get/SetTabBorders()
                ::PrepareBoxInfo( aSet, rSh );
                rSh.GetTabBorders( aSet );
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                pDlg.reset(pFact->CreateSwBorderDlg( pMDI, aSet, SwBorderModes::TABLE, RC_DLG_SWBORDERDLG ));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetTabBorders( *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else if ( rSh.IsFrameSelected() )
            {
                // Set border attributes via Frame-Manager
                SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE );
                aSet.Put( aMgr.GetAttrSet() );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                pDlg.reset(pFact->CreateSwBorderDlg( pMDI, aSet, SwBorderModes::FRAME, RC_DLG_SWBORDERDLG ));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
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
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                pDlg.reset(pFact->CreateSwBorderDlg( pMDI, aSet, SwBorderModes::PARA, RC_DLG_SWBORDERDLG ));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
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
            SfxItemSet aSet( rSh.GetAttrPool(),
                             RES_BACKGROUND, RES_BACKGROUND );

            std::unique_ptr<SfxAbstractDialog> pDlg;
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            // Table cell(s) selected?
            if ( rSh.IsTableMode() )
            {
                // Get background attributes of the table and put it in the set
                SvxBrushItem aBrush(RES_BACKGROUND);
                rSh.GetBoxBackground( aBrush );
                pDlg.reset(pFact->CreateSfxDialog( pMDI, aSet,
                    rView.GetViewFrame()->GetFrame().GetFrameInterface(),
                    RC_SWDLG_BACKGROUND ));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
                aSet.Put( aBrush );
                if ( pDlg->Execute() == RET_OK )
                {

                    rSh.SetBoxBackground( static_cast<const SvxBrushItem&>(
                        pDlg->GetOutputItemSet()->Get( RES_BACKGROUND )));
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else if ( rSh.IsFrameSelected() )
            {

                rSh.GetFlyFrameAttr( aSet );

                pDlg.reset(pFact->CreateSfxDialog( pMDI, aSet,
                    rView.GetViewFrame()->GetFrame().GetFrameInterface(),
                    RC_SWDLG_BACKGROUND ));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetFlyFrameAttr((SfxItemSet &) *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else
            {
                // Set border attributes Umrandungsattribute with the shell quite normal.
                rSh.GetCurAttr( aSet );

                pDlg.reset(pFact->CreateSfxDialog( pMDI, aSet,
                    rView.GetViewFrame()->GetFrame().GetFrameInterface(),
                    RC_SWDLG_BACKGROUND ));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
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
        default:OSL_FAIL("wrong Dispatcher (basesh.cxx)");
    }
    if(!bDone)
        rReq.Done();
}

SwWrtShell& SwBaseShell::GetShell()
{
    return rView.GetWrtShell();
}

SwWrtShell* SwBaseShell::GetShellPtr()
{
    return rView.GetWrtShellPtr();
}

void SwBaseShell::InsertTable( SfxRequest& _rRequest )
{
    const SfxItemSet* pArgs = _rRequest.GetArgs();
    SwWrtShell& rSh = GetShell();

    if ( !( rSh.GetFrameType( nullptr, true ) & FrameTypeFlags::FOOTNOTE ) )
    {
        SwView &rTempView = GetView(); // Because GetView() does not work after the shell exchange
        bool bHTMLMode = 0 != (::GetHtmlMode(rTempView.GetDocShell())&HTMLMODE_ON);
        bool bCallEndUndo = false;

        if( !pArgs && rSh.IsSelection() && !rSh.IsInClickToEdit() &&
            !rSh.IsTableMode() )
        {
            const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
            SwInsertTableOptions aInsTableOpts = pModOpt->GetInsTableFlags(bHTMLMode);

            rSh.StartUndo(UNDO_INSTABLE);
            bCallEndUndo = true;

            bool bInserted = rSh.TextToTable( aInsTableOpts, '\t' );
            rSh.EnterStdMode();
            if (bInserted)
                rTempView.AutoCaption(TABLE_CAP);
            _rRequest.Done();
        }
        else
        {
            sal_uInt16 nCols = 0;
            sal_uInt16 nRows = 0;
            SwInsertTableOptions aInsTableOpts( tabopts::ALL_TBL_INS_ATTR, 1 );
            OUString aTableName;
            OUString aAutoName;
            SwTableAutoFormat* pTAFormat = nullptr;

            if( pArgs && pArgs->Count() >= 2 )
            {
                const SfxStringItem* pName = _rRequest.GetArg<SfxStringItem>(FN_INSERT_TABLE);
                const SfxUInt16Item* pCols = _rRequest.GetArg<SfxUInt16Item>(SID_ATTR_TABLE_COLUMN);
                const SfxUInt16Item* pRows = _rRequest.GetArg<SfxUInt16Item>(SID_ATTR_TABLE_ROW);
                const SfxInt32Item* pFlags = _rRequest.GetArg<SfxInt32Item>(FN_PARAM_1);
                const SfxStringItem* pAuto = _rRequest.GetArg<SfxStringItem>(FN_PARAM_2);

                if ( pName )
                    aTableName = pName->GetValue();
                if ( pCols )
                    nCols = pCols->GetValue();
                if ( pRows )
                    nRows = pRows->GetValue();
                if ( pAuto )
                {
                    aAutoName = pAuto->GetValue();
                    if ( !aAutoName.isEmpty() )
                    {
                        SwTableAutoFormatTable aTableTable;
                        aTableTable.Load();
                        for ( size_t n=0; n<aTableTable.size(); n++ )
                        {
                            if ( aTableTable[n].GetName() == aAutoName )
                            {
                                pTAFormat = new SwTableAutoFormat( aTableTable[n] );
                                break;
                            }
                        }
                    }
                }

                if ( pFlags )
                    aInsTableOpts.mnInsMode = (sal_uInt16) pFlags->GetValue();
                else
                {
                    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                    aInsTableOpts = pModOpt->GetInsTableFlags(bHTMLMode);
                }
            }

            if( !nCols || !nRows )
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                std::unique_ptr<AbstractInsTableDlg> pDlg(pFact->CreateInsTableDlg(rTempView));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
                if( RET_OK == pDlg->Execute() )
                {
                    pDlg->GetValues( aTableName, nRows, nCols, aInsTableOpts, aAutoName, pTAFormat );
                }
                else
                    _rRequest.Ignore();
            }

            if( nCols && nRows )
            {
                // record before shell change
                _rRequest.AppendItem( SfxStringItem( FN_INSERT_TABLE, aTableName ) );
                if ( !aAutoName.isEmpty() )
                    _rRequest.AppendItem( SfxStringItem( FN_PARAM_2, aAutoName ) );
                _rRequest.AppendItem( SfxUInt16Item( SID_ATTR_TABLE_COLUMN, nCols ) );
                _rRequest.AppendItem( SfxUInt16Item( SID_ATTR_TABLE_ROW, nRows ) );
                _rRequest.AppendItem( SfxInt32Item( FN_PARAM_1, (sal_Int32) aInsTableOpts.mnInsMode ) );
                _rRequest.Done();

                rSh.StartUndo(UNDO_INSTABLE);
                bCallEndUndo = true;

                rSh.StartAllAction();
                if( rSh.HasSelection() )
                    rSh.DelRight();

                rSh.InsertTable( aInsTableOpts, nRows, nCols, text::HoriOrientation::FULL, pTAFormat );
                rSh.MoveTable( fnTablePrev, fnTableStart );

                if( !aTableName.isEmpty() && !rSh.GetTableStyle( aTableName ) )
                    rSh.GetTableFormat()->SetName( aTableName );

                rSh.EndAllAction();
                rTempView.AutoCaption(TABLE_CAP);
            }
            delete pTAFormat;
        }

        if( bCallEndUndo )
        {
            SwRewriter aRewriter;

            if (rSh.GetTableFormat())
            {
                aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_START_QUOTE));
                aRewriter.AddRule(UndoArg2, rSh.GetTableFormat()->GetName());
                aRewriter.AddRule(UndoArg3, SW_RESSTR(STR_END_QUOTE));

            }
            rSh.EndUndo(UNDO_INSTABLE, &aRewriter); // If possible change the Shell
        }
    }
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
            int nSel = rSh.GetSelectionType();
            SfxStringListItem aLst( nWhich );
            std::vector<OUString> &rLst = aLst.GetList();
            nParagraphPos = nGraphicPos = nOlePos = nFramePos = nTablePos =
            nTableRowPos  = nTableCellPos = nPagePos =
            nHeaderPos    = nFooterPos = 0;
            sal_uInt8 nPos = 1;
            rLst.push_back( SW_RESSTR( STR_SWBG_PAGE ) );
            nPagePos = nPos++;
            sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
            bool bHtmlMode = 0 != (nHtmlMode & HTMLMODE_ON);

            if ( (!bHtmlMode || (nHtmlMode & HTMLMODE_FULL_STYLES)) &&
                 (nSel & nsSelectionType::SEL_TXT) )
            {
                rLst.push_back( SW_RESSTR( STR_SWBG_PARAGRAPH ) );
                nParagraphPos = nPos++;
            }
            if ( (!bHtmlMode || (nHtmlMode & HTMLMODE_SOME_STYLES)) &&
                    nSel & (nsSelectionType::SEL_TBL|nsSelectionType::SEL_TBL_CELLS) )
            {
                rLst.push_back( SW_RESSTR( STR_SWBG_TABLE ) );
                nTablePos = nPos++;

                if(!bHtmlMode)
                {
                    rLst.push_back( SW_RESSTR( STR_SWBG_TABLE_ROW ) );
                    nTableRowPos = nPos++;
                }

                rLst.push_back( SW_RESSTR( STR_SWBG_TABLE_CELL) );
                nTableCellPos = nPos++;
            }
            if(!bHtmlMode)
            {
                if ( nSel & nsSelectionType::SEL_FRM )
                {
                    rLst.push_back( SW_RESSTR( STR_SWBG_FRAME ) );
                    nFramePos = nPos++;
                }
                if ( nSel & nsSelectionType::SEL_GRF )
                {
                    rLst.push_back( SW_RESSTR( STR_SWBG_GRAPHIC ) );
                    nGraphicPos = nPos++;
                }
                if ( nSel & nsSelectionType::SEL_OLE )
                {
                    rLst.push_back( SW_RESSTR( STR_SWBG_OLE ) );
                    nOlePos = nPos++;
                }
                const FrameTypeFlags nType = rSh.GetFrameType(nullptr,true);
                if ( nType & FrameTypeFlags::HEADER )
                {
                    rLst.push_back( SW_RESSTR( STR_SWBG_HEADER ) );
                    nHeaderPos = nPos++;
                }
                if ( nType & FrameTypeFlags::FOOTER )
                {
                    rLst.push_back( SW_RESSTR( STR_SWBG_FOOTER ) );
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

            int nSel = rSh.GetSelectionType();
            if ( nSel & nsSelectionType::SEL_DRW_TXT )
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
                SfxItemSet aCoreSet(GetPool(), RES_BACKGROUND, RES_BACKGROUND);
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

void SwBaseShell::ExecField( SfxRequest& rReq )
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch( nSlot )
    {
        case FN_CHANGE_DBFIELD:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            std::unique_ptr<VclAbstractDialog> pDlg(pFact->CreateSwChangeDBDlg(GetView()));
            OSL_ENSURE(pDlg, "Dialog creation failed!");
            pDlg->Execute();
        }
        break;
        default:
            OSL_FAIL("wrong dispatcher");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
