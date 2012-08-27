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


#include <sot/factory.hxx>
#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <svl/languageoptions.hxx>

#include <svx/svxids.hrc>
#include <sfx2/linkmgr.hxx>
#include <svx/htmlmode.hxx>
#include <svx/imapdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <svl/visitem.hxx>
#include <sfx2/objitem.hxx>
#include <svtools/filter.hxx>
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
#include <editeng/brshitem.hxx>
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
#include <crsskip.hxx>
#include <fmtinfmt.hxx>
#include <doc.hxx>

#include "swabstdlg.hxx"
#include "dialog.hrc"
#include "fldui.hrc"
#include "table.hrc"
#include <modcfg.hxx>
#include <instable.hxx>
#include <svx/fmshell.hxx> // for FN_XFORMS_DESIGN_MODE
#include <SwRewriter.hxx>
#include <comcore.hrc>

#include <unomid.h>

FlyMode SwBaseShell::eFrameMode = FLY_DRAG_END;

//Fuer die Erkennung der Id, die variable von Gallery mit SID_GALLERY_BG_BRUSH
//ankommt.
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
#define Shadow
#include <sfx2/msg.hxx>
#include "swslots.hxx"

#define SWCONTOURDLG(rView) ( (SvxContourDlg*) ( rView.GetViewFrame()->GetChildWindow(  \
                          SvxContourDlgChildWindow::GetChildWindowId() )->  \
                          GetWindow() ) )

#define SWIMAPDLG(rView) ( (SvxIMapDlg*) ( rView.GetViewFrame()->GetChildWindow(        \
                        SvxIMapDlgChildWindow::GetChildWindowId() )->   \
                        GetWindow() ) )


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

SFX_IMPL_INTERFACE(SwBaseShell, SfxShell, SW_RES(0))
{
    SFX_CHILDWINDOW_REGISTRATION(SvxIMapDlgChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SvxContourDlgChildWindow::GetChildWindowId());
}

TYPEINIT1(SwBaseShell,SfxShell)

/*--------------------------------------------------------------------
    Beschreibung:   statics
 --------------------------------------------------------------------*/
void lcl_UpdateIMapDlg( SwWrtShell& rSh )
{
    Graphic aGrf( rSh.GetIMapGraphic() );
    GraphicType nGrfType = aGrf.GetType();
    void* pEditObj = GRAPHIC_NONE != nGrfType && GRAPHIC_DEFAULT != nGrfType
                        ? rSh.GetIMapInventor() : 0;
    TargetList* pList = new TargetList;
    rSh.GetView().GetViewFrame()->GetTopFrame().GetTargetList(*pList);

    SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
    rSh.GetFlyFrmAttr( aSet );
    const SwFmtURL &rURL = (SwFmtURL&)aSet.Get( RES_URL );
    SvxIMapDlgChildWindow::UpdateIMapDlg(
            aGrf, rURL.GetMap(), pList, pEditObj );

    for ( size_t i = 0, n = pList->size(); i < n; ++i )
        delete pList->at( i );
    delete pList;
}

sal_Bool lcl_UpdateContourDlg( SwWrtShell &rSh, int nSel )
{
    Graphic aGraf( rSh.GetIMapGraphic() );
    GraphicType nGrfType = aGraf.GetType();
    sal_Bool bRet = GRAPHIC_NONE != nGrfType && GRAPHIC_DEFAULT != nGrfType;
    if( bRet )
    {
        String aGrfName;
        if ( nSel & nsSelectionType::SEL_GRF )
            rSh.GetGrfNms( &aGrfName, 0 );

        SvxContourDlg *pDlg = SWCONTOURDLG(rSh.GetView());
        pDlg->Update( aGraf, aGrfName.Len() > 0,
                  rSh.GetGraphicPolygon(), rSh.GetIMapInventor() );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   loeschen
 --------------------------------------------------------------------*/
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
                rSh.SttCrsrMove();
                sal_Bool bLeft = rSh.Left( CRSR_SKIP_CHARS, sal_True, 1, sal_False  );
                if( bLeft )
                {
                    rSh.DelLeft();
                }
                else
                    // JP 15.07.96: wenns nicht mehr nach vorne geht, die
                    //              Numerierung aufheben. Z.B. am Doc-/
                    //              Rahmen-/Tabellen-/Bereichs-Anfang
                    rSh.DelNumRules();

                rSh.EndCrsrMove();
                break;
            }

            // ansonsten DelLeft rufen
        case FN_SHIFT_BACKSPACE:
            rSh.DelLeft();
            break;
        default:
            OSL_FAIL("wrong Dispatcher");
            return;
    }
    rReq.Done();

    //#i42732# - notify the edit window that from now on we do not use the input language
    rTmpEditWin.SetUseInputLanguage( sal_False );
}

void SwBaseShell::ExecClpbrd(SfxRequest &rReq)
{
    //Achtung: Suizid gefaehrdet! Nach Paste, Paste special kann der die
    //Shell zerstoert sein.


    SwWrtShell &rSh = GetShell();
    sal_uInt16 nId = rReq.GetSlot();
    sal_Bool bIgnore = sal_False;
    switch( nId )
    {
        case SID_CUT:
        case SID_COPY:
            rView.GetEditWin().FlushInBuffer();
            if ( rSh.HasSelection() )
            {
                SwTransferable* pTransfer = new SwTransferable( rSh );
/*??*/          uno::Reference< datatransfer::XTransferable > xRef( pTransfer );

                if ( nId == SID_CUT && !rSh.IsSelObjProtected(FLYPROTECT_CONTENT|FLYPROTECT_PARENT) )
                    pTransfer->Cut();
                else
                {
                    const sal_Bool bLockedView = rSh.IsViewLocked();
                    rSh.LockView( sal_True );    //lock visible section
                    pTransfer->Copy();
                    rSh.LockView( bLockedView );
                }
                break;
            }
            return;

        case SID_PASTE:
            {
                TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                        &rSh.GetView().GetEditWin() ) );

                if( aDataHelper.GetXTransferable().is() &&
                    SwTransferable::IsPaste( rSh, aDataHelper ))
                {
                    // temp. Variablen, da die Shell nach dem Paste schon
                    // zerstoert sein kann
                    SwView* pView = &rView;
                    SwTransferable::Paste( rSh, aDataHelper );
                    if( rSh.IsFrmSelected() || rSh.IsObjSelected() )
                        rSh.EnterSelFrmMode();
                    pView->AttrChangedNotify( &rSh );
                }
                else
                    return;
            }
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                const SfxPoolItem* pFmt;
                if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nId, sal_False, &pFmt ) )
                {
                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                            &rSh.GetView().GetEditWin()) );
                    if( aDataHelper.GetXTransferable().is()
                        /*&& SwTransferable::IsPaste( rSh, aDataHelper )*/ )
                    {
                        // temp. Variablen, da die Shell nach dem Paste schon
                        // zerstoert sein kann
                        SwView* pView = &rView;

                        SwTransferable::PasteFormat( rSh, aDataHelper,
                                        ((SfxUInt32Item*)pFmt)->GetValue() );

                        //Done() has to be called before the shell has been removed
                        rReq.Done();
                        bIgnore = sal_True;
                        if( rSh.IsFrmSelected() || rSh.IsObjSelected())
                            rSh.EnterSelFrmMode();
                        pView->AttrChangedNotify( &rSh );
                    }
                }
            }
            break;

        case SID_PASTE_UNFORMATTED:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard(
                        &rSh.GetView().GetEditWin()) );
                if( aDataHelper.GetXTransferable().is() &&
                    SwTransferable::IsPaste( rSh, aDataHelper ))
                {
                    // temp. Variablen, da die Shell nach dem Paste schon
                    // zerstoert sein kann
                    SwView* pView = &rView;
                    rReq.Ignore();
                    bIgnore = sal_True;
                    int nRet = SwTransferable::PasteUnformatted( rSh, aDataHelper );
                    if(nRet)
                    {
                        SfxViewFrame* pViewFrame = pView->GetViewFrame();
                        uno::Reference< frame::XDispatchRecorder > xRecorder =
                            pViewFrame->GetBindings().GetRecorder();
                        if(xRecorder.is()) {
                            SfxRequest aReq( pViewFrame, SID_CLIPBOARD_FORMAT_ITEMS );
                            aReq.AppendItem( SfxUInt32Item( SID_CLIPBOARD_FORMAT_ITEMS, SOT_FORMAT_STRING ) );
                            aReq.Done();
                        }
                    }

                    if (rSh.IsFrmSelected() || rSh.IsObjSelected())
                        rSh.EnterSelFrmMode();
                    pView->AttrChangedNotify( &rSh );
                }
                else
                    return;
            }
            break;

        case SID_PASTE_SPECIAL:
            {
                TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                        &rSh.GetView().GetEditWin()) );
                if( aDataHelper.GetXTransferable().is() &&
                    SwTransferable::IsPaste( rSh, aDataHelper ))
                {
                    // temp. Variablen, da die Shell nach dem Paste schon
                    // zerstoert sein kann
                    SwView* pView = &rView;
                    sal_uLong nFormatId = 0;
                    rReq.Ignore();
                    bIgnore = sal_True;
                    int nRet = SwTransferable::PasteSpecial( rSh, aDataHelper, nFormatId );
                    if(nRet)
                    {
                        SfxViewFrame* pViewFrame = pView->GetViewFrame();
                        uno::Reference< frame::XDispatchRecorder > xRecorder =
                                pViewFrame->GetBindings().GetRecorder();
                        if(xRecorder.is()) {
                            SfxRequest aReq( pViewFrame, SID_CLIPBOARD_FORMAT_ITEMS );
                            aReq.AppendItem( SfxUInt32Item( SID_CLIPBOARD_FORMAT_ITEMS, nFormatId ) );
                            aReq.Done();
                        }
                    }

                    if (rSh.IsFrmSelected() || rSh.IsObjSelected())
                        rSh.EnterSelFrmMode();
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

/*--------------------------------------------------------------------
    Beschreibung:   ClipBoard-Status
 --------------------------------------------------------------------*/
void SwBaseShell::StateClpbrd(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter(rSet);

    const sal_Bool bCopy = rSh.HasSelection();

    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
        case SID_CUT:
            if( 0 != rSh.IsSelObjProtected(FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) )
            {
                rSet.DisableItem( nWhich );
                break;
            }
        case SID_COPY:
            if( !bCopy )
                rSet.DisableItem( nWhich );
            break;

        case SID_PASTE:
            if( !GetView().IsPasteAllowed() )
                rSet.DisableItem( SID_PASTE );
            break;

        case SID_PASTE_SPECIAL:
            if( !GetView().IsPasteSpecialAllowed() )
            {
                rSet.DisableItem( SID_PASTE_SPECIAL );
                rSet.DisableItem( SID_PASTE_UNFORMATTED );
            }
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard(
                                            &rSh.GetView().GetEditWin()) );

                SvxClipboardFmtItem aFmtItem( nWhich );
                SwTransferable::FillClipFmtItem( rSh, aDataHelper, aFmtItem );
                rSet.Put( aFmtItem );
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Undo ausfuehren
 --------------------------------------------------------------------*/
void SwBaseShell::ExecUndo(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();

    sal_uInt16 nId = rReq.GetSlot(), nCnt = 1;
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nId, sal_False, &pItem ))
        nCnt = ((SfxUInt16Item*)pItem)->GetValue();

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

    if (pViewFrame) { pViewFrame->GetBindings().InvalidateAll(sal_False); }
}

/*--------------------------------------------------------------------
    Beschreibung:   Zustand Undo
 --------------------------------------------------------------------*/
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
                if (rSh.GetLastUndoInfo(0, 0))
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
                if (rSh.GetFirstRedoInfo(0))
                {
                    rSet.Put(SfxStringItem(nWhich,
                        rSh.GetDoString(SwWrtShell::REDO)));
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }
            case SID_REPEAT:
            {   // Repeat nur moeglich wenn kein REDO moeglich - UI-Restriktion
                if ((!rSh.GetFirstRedoInfo(0)) &&
                    !rSh.IsSelFrmMode() &&
                    (UNDO_EMPTY != rSh.GetRepeatInfo(0)))
                {
                    rSet.Put(SfxStringItem(nWhich, rSh.GetRepeatString()));
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }

            case SID_GETUNDOSTRINGS:
                if (rSh.GetLastUndoInfo(0, 0))
                {
                    SfxStringListItem aStrLst( nWhich );
                    rSh.GetDoStrings( SwWrtShell::UNDO, aStrLst );
                    rSet.Put( aStrLst );
                }
                else
                    rSet.DisableItem( nWhich );
                break;

            case SID_GETREDOSTRINGS:
                if (rSh.GetFirstRedoInfo(0))
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

/*--------------------------------------------------------------------
    Beschreibung:   Slot-Id auswerten bzw. Dispatchen
 --------------------------------------------------------------------*/
void SwBaseShell::Execute(SfxRequest &rReq)
{
    const SfxPoolItem *pItem;
    SwWrtShell &rSh = GetShell();
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_Bool bMore = sal_False;

    sal_uInt16 nSlot = rReq.GetSlot();
    switch(nSlot)
    {
        case FN_REPAGINATE:
            {
                Reference < XModel > xModel = GetView().GetDocShell()->GetModel();
                Reference < XUnoTunnel > xDocTunnel ( xModel, UNO_QUERY );
                SwXTextDocument *pDoc = reinterpret_cast < SwXTextDocument * > ( xDocTunnel->getSomething ( SwXTextDocument::getUnoTunnelId() ) );
                pDoc->notifyRefreshListeners();
                rSh.CalcLayout();
            }
            break;
        case FN_UPDATE_FIELDS:
            {
                rSh.UpdateDocStat();
                rSh.EndAllTblBoxEdit();
                rSh.ViewShell::UpdateFlds(sal_True);

                if( rSh.IsCrsrInTbl() )
                {
                    if( !rSh.IsTblComplexForChart() )
                        SwTableFUNC( &rSh, sal_False).UpdateChart();
                    rSh.ClearTblBoxCntnt();
                    rSh.SaveTblBoxCntnt();
                }
            }
            break;
        case FN_UPDATE_CHARTS:
            {
                SwWait aWait( *rView.GetDocShell(), sal_True );
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
                    rSh.GetLinkManager().UpdateAllLinks( sal_False, sal_True, sal_True );
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
            rSh.UpdateInputFlds(NULL, sal_False);
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
            SwFldMgr aFldMgr;
            SwFieldType* pFldType = aFldMgr.GetFldType(RES_JUMPEDITFLD);

            if (pFldType)
            {
                if (rSh.IsSelFrmMode())
                {
                    rSh.UnSelectFrm();
                    rSh.LeaveSelFrmMode();
                }

                if (rSh.HasMark())
                {
                    MV_KONTEXT(&rSh);
                    if (rSh.IsCrsrPtAtEnd())
                        rSh.SwapPam();
                    rSh.ClearMark();
                    rSh.EndSelect();
                }
                sal_Bool bRet = rSh.MoveFldType(pFldType, nSlot == FN_GOTO_NEXT_MARK);
                SwField* pCurField = bRet ? rSh.GetCurFld() : 0;
                if (pCurField)
                    rSh.ClickToField(*pCurField);
                rReq.SetReturnValue(SfxBoolItem( nSlot, bRet));
            }
        }
        break;

        case FN_START_DOC_DIRECT:
        case FN_END_DOC_DIRECT:
        {
            if (rSh.IsSelFrmMode())
            {
                rSh.UnSelectFrm();
                rSh.LeaveSelFrmMode();
            }
            rSh.EnterStdMode();
            nSlot == FN_START_DOC_DIRECT ?
                rSh.SttEndDoc(sal_True) :
                    rSh.SttEndDoc(sal_False);
        }
        break;
        case FN_GOTO_PREV_OBJ:
        case FN_GOTO_NEXT_OBJ:
        {
                sal_Bool bSuccess = rSh.GotoObj(
                            nSlot == FN_GOTO_NEXT_OBJ ? sal_True : sal_False);
                rReq.SetReturnValue(SfxBoolItem(nSlot, bSuccess));
                if (bSuccess && !rSh.IsSelFrmMode())
                {
                    rSh.HideCrsr();
                    rSh.EnterSelFrmMode();
                    GetView().AttrChangedNotify( &rSh );
                }
        }
        break;
        case SID_GALLERY_FORMATS:
        {
            const int nSelType = rSh.GetSelectionType();
            if(SFX_ITEM_SET == pArgs->GetItemState( nSlot, sal_True, &pItem))
            {
                GalleryExplorer* pGal = 0;
                if ( (!rSh.IsSelFrmMode() || nSelType & nsSelectionType::SEL_GRF) &&
                    0!= (pGal = SVX_GALLERY())&&
                    0 != (SGA_FORMAT_GRAPHIC & ((SfxUInt32Item*)pItem)->GetValue()))
                {
                    SwWait aWait( *rView.GetDocShell(), sal_True );

                    String aGrfName, aFltName;
                    const Graphic aGrf( pGal->GetGraphic() );

                    if( pGal->IsLinkage() )
                    {
                        // Verknuepft
                        aGrfName = pGal->GetURL().GetMainURL(INetURLObject::NO_DECODE);
                        aFltName = pGal->GetFilterName();
                    }

                    if ( nSelType & nsSelectionType::SEL_GRF )
                        rSh.ReRead( aGrfName, aFltName, &aGrf );
                    else
                        rSh.Insert( aGrfName, aFltName, aGrf );

                    GetView().GetEditWin().GrabFocus();
                }
                else if(!rSh.IsSelFrmMode() && SGA_FORMAT_SOUND & ((SfxUInt32Item*)pItem)->GetValue())
                {
                    const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, pGal->GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
                       GetView().GetViewFrame()->GetDispatcher()->Execute( SID_INSERT_AVMEDIA, SFX_CALLMODE_SYNCHRON, &aMediaURLItem, 0L );
                }
            }
        }
        break;
        case FN_PAGE_STYLE_SET_COLS:
        {
            if (pArgs)
            {
                // aktuellen PageDescriptor ermitteln und damit den Set fuellen
                const sal_uInt16 nCurIdx = rSh.GetCurPageDesc();
                SwPageDesc aPageDesc(rSh.GetPageDesc(nCurIdx));

                SwFrmFmt &rFmt = aPageDesc.GetMaster();

                SwFmtCol aFmtCol = rFmt.GetCol();

                sal_uInt16 nCount;
                if(SFX_ITEM_SET == pArgs->GetItemState(nSlot))
                    nCount = ((SfxUInt16Item &)pArgs->Get(nSlot)).GetValue();
                else
                    nCount = ((SfxUInt16Item &)pArgs->Get(SID_ATTR_COLUMNS)).GetValue();
                sal_uInt16 nGutterWidth = DEF_GUTTER_WIDTH;

                aFmtCol.Init(nCount ? nCount : 1, nGutterWidth, USHRT_MAX);
                aFmtCol.SetWishWidth(USHRT_MAX);
                aFmtCol.SetGutterWidth(nGutterWidth, USHRT_MAX);

                rFmt.SetFmtAttr(aFmtCol);

                rSh.ChgPageDesc(nCurIdx, aPageDesc);
            }
            else
                GetView().GetViewFrame()->GetDispatcher()->Execute(FN_FORMAT_PAGE_COLUMN_DLG, sal_False);
        }
        break;
        case FN_CONVERT_TABLE_TO_TEXT:
        case FN_CONVERT_TEXT_TO_TABLE:
        case FN_CONVERT_TEXT_TABLE:
        {
            sal_Unicode cDelim = 0;
            bool bToTable = false;
            if( nSlot == FN_CONVERT_TEXT_TO_TABLE ||
                ( nSlot == FN_CONVERT_TEXT_TABLE && 0 == rSh.GetTableFmt() ))
                bToTable = true;
            SwInsertTableOptions aInsTblOpts( tabopts::ALL_TBL_INS_ATTR, 1 );
            SwTableAutoFmt const* pTAFmt = 0;
            SwTableAutoFmtTbl* pAutoFmtTbl = 0;
            bool bDeleteFormat = true;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_1, sal_True, &pItem))
            {
                aInsTblOpts.mnInsMode = 0;
                //Delimiter
                String sDelim = static_cast< const SfxStringItem* >(pItem)->GetValue();
                if(sDelim.Len())
                    cDelim = sDelim.GetChar(0);
                //AutoFormat
                if(SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_2, sal_True, &pItem))
                {
                    String sAutoFmt = static_cast< const SfxStringItem* >(pItem)->GetValue();

                    pAutoFmtTbl = new SwTableAutoFmtTbl;
                    pAutoFmtTbl->Load();

                    for( sal_uInt16 i = 0, nCount = pAutoFmtTbl->size(); i < nCount; i++ )
                    {
                        SwTableAutoFmt const*const pFmt = &(*pAutoFmtTbl)[ i ];
                        if( pFmt->GetName() == sAutoFmt )
                        {
                            pTAFmt = pFmt;
                            bDeleteFormat = false;
                            break;
                        }
                    }
                }
                //WithHeader
                if(SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_3, sal_True, &pItem) &&
                            static_cast< const SfxBoolItem* >(pItem)->GetValue())
                    aInsTblOpts.mnInsMode |= tabopts::HEADLINE;
                // RepeatHeaderLines
                if(SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_4, sal_True, &pItem))
                   aInsTblOpts.mnRowsToRepeat =
                            (sal_uInt16)static_cast< const SfxInt16Item* >(pItem)->GetValue();
                //WithBorder
                if(SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_5, sal_True, &pItem) &&
                    static_cast< const SfxBoolItem* >(pItem)->GetValue())
                    aInsTblOpts.mnInsMode |= tabopts::DEFAULT_BORDER;
                //DontSplitTable
                if(SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_6, sal_True, &pItem) &&
                    !static_cast< const SfxBoolItem* >(pItem)->GetValue() )
                    aInsTblOpts.mnInsMode |= tabopts::SPLIT_LAYOUT;
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                AbstractSwConvertTableDlg* pDlg = pFact->CreateSwConvertTableDlg(
                            GetView(),DLG_CONV_TEXT_TABLE , bToTable);
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if( RET_OK == pDlg->Execute() )
                {
                    pDlg->GetValues( cDelim, aInsTblOpts, pTAFmt );

                }
                delete pDlg;
            }

            if( cDelim )
            {
                //Shellwechsel!
                SwView& rSaveView = rView;
                sal_Bool bInserted = sal_False;
                //recording:

                SfxViewFrame* pViewFrame = GetView().GetViewFrame();
                if( SfxRequest::HasMacroRecorder(pViewFrame) )
                {
                    SfxRequest aReq( pViewFrame, nSlot);
                    aReq.AppendItem( SfxStringItem( FN_PARAM_1, rtl::OUString(cDelim) ));
                    if(bToTable)
                    {
                        if(pTAFmt)
                            aReq.AppendItem( SfxStringItem( FN_PARAM_2, pTAFmt->GetName()));
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_3, 0 != (aInsTblOpts.mnInsMode & tabopts::HEADLINE)));
                        aReq.AppendItem( SfxInt16Item( FN_PARAM_4, (short)aInsTblOpts.mnRowsToRepeat ));
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_5, 0 != (aInsTblOpts.mnInsMode & tabopts::DEFAULT_BORDER) ));
                        aReq.AppendItem( SfxBoolItem ( FN_PARAM_6, !(aInsTblOpts.mnInsMode & tabopts::SPLIT_LAYOUT)));
                    }
                    aReq.Done();
                }

                if( !bToTable )
                    rSh.TableToText( cDelim );
                else
                {
                    bInserted = rSh.TextToTable( aInsTblOpts, cDelim, text::HoriOrientation::FULL, pTAFmt );
                }
                rSh.EnterStdMode();

                if( bInserted )
                    rSaveView.AutoCaption( TABLE_CAP );
            }
            if(bDeleteFormat)
                delete pTAFmt;
            delete pAutoFmtTbl;
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
                    const sal_Bool bLockedView = rSh.IsViewLocked();
                    rSh.LockView( sal_True );    //lock visible section

                    GetView().GetDocShell()->ExecStyleSheet(rReq);

                    rSh.LockView( bLockedView );
                }
                else
                // wird von der DocShell aufgezeichnet
                    GetView().GetDocShell()->ExecStyleSheet(rReq);
            }
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

            if ( pVFrame->HasChildWindow( nId ) && rSh.IsFrmSelected() )
                lcl_UpdateIMapDlg( rSh );
        }
        break;
        case SID_IMAP_EXEC:
        {
            SvxIMapDlg* pDlg = SWIMAPDLG(GetView());

            // Kontrolle, ob Zuweisung ueberhaupt sinnvoll/erlaubt
            if ( rSh.IsFrmSelected() &&
                 pDlg->GetEditingObject() == rSh.GetIMapInventor() )
            {
                    SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                    rSh.GetFlyFrmAttr( aSet );
                    SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );
                    aURL.SetMap( &pDlg->GetImageMap() );
                    aSet.Put( aURL );
                    rSh.SetFlyFrmAttr( aSet );
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
            SvxContourDlg *pDlg = SWCONTOURDLG(GetView());

            // Kontrolle, ob Zuweisung ueberhaupt sinnvoll/erlaubt
            int nSel = rSh.GetSelectionType();
            if ( nSel & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_OLE) )
            {
                if ( pDlg->GetEditingObject() == rSh.GetIMapInventor() )
                {
                    rSh.StartAction();
                    SfxItemSet aSet( rSh.GetAttrPool(), RES_SURROUND, RES_SURROUND);
                    rSh.GetFlyFrmAttr( aSet );
                    SwFmtSurround aSur( (SwFmtSurround&)aSet.Get( RES_SURROUND ) );
                    if ( !aSur.IsContour() )
                    {
                        aSur.SetContour( sal_True );
                        if ( aSur.GetSurround() == SURROUND_NONE )
                            aSur.SetSurround( SURROUND_PARALLEL );
                        aSet.Put( aSur );
                        rSh.SetFlyFrmAttr( aSet );
                    }
                    const PolyPolygon aPoly( pDlg->GetPolyPolygon() );
                    rSh.SetGraphicPolygon( &aPoly );
                    if ( pDlg->IsGraphicChanged() )
                        rSh.ReRead( aEmptyStr, aEmptyStr, &pDlg->GetGraphic());
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
            if( rSh.IsObjSelected() )
                rSh.ChgAnchor( eSet );
            else if( rSh.IsFrmSelected() )
            {
                // Der Set beinhaltet auch VERT/HORI_ORIENT, da in FEShell::
                // SetFlyFrmAttr/SetFlyFrmAnchor ggf. als Folge des Umankerns
                // die Ausrichtungen veraendert werden sollen.
                SfxItemSet aSet( GetPool(), RES_VERT_ORIENT, RES_ANCHOR );
                SwFmtAnchor aAnc( eSet, rSh.GetPhyPageNum() );
                aSet.Put( aAnc );
                rSh.SetFlyFrmAttr(aSet);
            }

            // if new anchor is 'as char' and it is a Math object and the usual
            // pre-conditions are met then align the formula to the baseline of the text
            const uno::Reference < embed::XEmbeddedObject > xObj( rSh.GetOleRef() );
            const bool bDoMathBaselineAlignment = xObj.is() && SotExchange::IsMath( xObj->getClassID() )
                    && FLY_AS_CHAR == eSet && rSh.GetDoc()->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT );
            if (bDoMathBaselineAlignment)
                rSh.AlignFormulaToBaseline( xObj );

            sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
            if( nHtmlMode )
            {
                SfxItemSet aSet(GetPool(), RES_SURROUND, RES_HORI_ORIENT);
                rSh.GetFlyFrmAttr(aSet);

                const SwFmtSurround& rSurround = (const SwFmtSurround&)aSet.Get(RES_SURROUND);
                const SwFmtVertOrient& rVert = (const SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT);
                const SwFmtHoriOrient& rHori = (const SwFmtHoriOrient&)aSet.Get(RES_HORI_ORIENT);
                sal_Int16 eVOrient = rVert.GetVertOrient();
                sal_Int16 eHOrient = rHori.GetHoriOrient();
                SwSurround eSurround = rSurround.GetSurround();

                switch( eSet )
                {
                case FLY_AT_FLY:
                case FLY_AT_PAGE:

                    //Durchlauf, links oder von links, oben, von oben
                    if(eSurround != SURROUND_THROUGHT)
                        aSet.Put(SwFmtSurround(SURROUND_THROUGHT));

                    if( eVOrient != text::VertOrientation::TOP && eVOrient != text::VertOrientation::NONE)
                        aSet.Put(SwFmtVertOrient(0, text::VertOrientation::TOP));

                    if(eHOrient != text::HoriOrientation::NONE || eHOrient != text::HoriOrientation::LEFT)
                        aSet.Put(SwFmtHoriOrient(0, text::HoriOrientation::LEFT));
                    break;

                case FLY_AT_PARA:
                    //links, von links, rechts, oben, kein Uml, li+re Umlauf,
                    if(eSurround != SURROUND_LEFT || eSurround != SURROUND_RIGHT)
                        aSet.Put(SwFmtSurround(SURROUND_LEFT));

                    if( eVOrient != text::VertOrientation::TOP)
                        aSet.Put(SwFmtVertOrient(0, text::VertOrientation::TOP));

                    if(eHOrient != text::HoriOrientation::NONE || eHOrient != text::HoriOrientation::LEFT || eHOrient != text::HoriOrientation::RIGHT)
                        aSet.Put(SwFmtHoriOrient(0, text::HoriOrientation::LEFT));
                    break;

                case FLY_AT_CHAR:
                    //links, von links, rechts, oben,  Durchlauf
                    if(eSurround != SURROUND_THROUGHT)
                        aSet.Put(SwFmtSurround(SURROUND_THROUGHT));

                    if( eVOrient != text::VertOrientation::TOP)
                        aSet.Put(SwFmtVertOrient(0, text::VertOrientation::TOP));

                    if(eHOrient != text::HoriOrientation::NONE || eHOrient != text::HoriOrientation::LEFT || eHOrient != text::HoriOrientation::RIGHT)
                        aSet.Put(SwFmtHoriOrient(0, text::HoriOrientation::LEFT));
                    break;

                default:
                    ;
                }

                if( aSet.Count() )
                    rSh.SetFlyFrmAttr( aSet );
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
                    sal_Bool bUpdateGrf = sal_False, bCallErrHdl = sal_False;
                    rSh.EnterStdMode();
                    rSh.StartAllAction();
                    rSh.GetLinkManager().UpdateAllLinks( sal_False, bCallErrHdl, bUpdateGrf );
                    rSh.EndAllAction();
                }
            }
            break;

        case FN_XFORMS_DESIGN_MODE:
            if( pArgs != NULL
                && pArgs->GetItemState( nSlot, sal_True, &pItem ) == SFX_ITEM_SET
                && pItem != NULL
                && pItem->ISA( SfxBoolItem ) )
            {
                sal_Bool bDesignMode =
                    static_cast<const SfxBoolItem*>( pItem )->GetValue();

                // set form design mode
                OSL_ENSURE( GetView().GetFormShell() != NULL, "form shell?" );
                SfxRequest aReq( GetView().GetViewFrame(), SID_FM_DESIGN_MODE );
                aReq.AppendItem( SfxBoolItem( SID_FM_DESIGN_MODE, bDesignMode ) );
                GetView().GetFormShell()->Execute( aReq );
                aReq.Done();

                // also set suitable view options
                SwViewOption aViewOption = *rSh.GetViewOptions();
                aViewOption.SetFormView( ! bDesignMode );
                rSh.ApplyViewOptions( aViewOption );
            }
            break;

        default:
            bMore = sal_True;
    }
    if(bMore && pArgs)
    {
        pItem = 0;
        pArgs->GetItemState(GetPool().GetWhich(nSlot), sal_False, &pItem);
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
            // Tabellenzelle(n) selektiert?
            if ( rSh.IsTableMode() )
            {
                SwFrmFmt *pFmt = rSh.GetTableFmt();
                pFmt->SetFmtAttr( *pItem );
            }
            else if ( rSh.IsFrmSelected() )
            {
                // Umrandungsattribute ueber Frame-Manager setzen
                SwFlyFrmAttrMgr aMgr( sal_False, &rSh, FRMMGR_TYPE_NONE );
                aMgr.SetAttrSet( *pArgs );
                aMgr.UpdateFlyFrm();
            }
            else
            {
                rSh.SetAttr( *pArgs );
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
            // Tabellenzelle(n) selektiert?
            if ( rSh.IsTableMode() )
            {
                // Umrandungattribute Get/SetTabBorders() setzen
                rSh.SetTabBorders(*pArgs);
            }
            else if ( rSh.IsFrmSelected() )
            {
                // Umrandungsattribute ueber Frame-Manager setzen
                SwFlyFrmAttrMgr aMgr( sal_False, &rSh, FRMMGR_TYPE_NONE );
                aMgr.SetAttrSet(*pArgs);
                aMgr.UpdateFlyFrm();
            }
            else
            {
                // Umrandungsattribute ganz normal ueber Shell setzen
                rSh.SetAttr( *pItem );
            }
        }
        break;
        default:
                OSL_FAIL("wrong Dispatcher");
        }

    }
}

/* --------------------------------------------------
 * Hier wird der State fuer SID_IMAP / SID_CONTOUR behandelt,
 * wenn die Grafik ausgeswappt ist
 * --------------------------------------------------*/
IMPL_LINK_NOARG(SwBaseShell, GraphicArrivedHdl)
{
    sal_uInt16 nGrfType;
    SwWrtShell &rSh = GetShell();
    if( CNT_GRF == rSh.SwEditShell::GetCntType() &&
        GRAPHIC_NONE != ( nGrfType = rSh.GetGraphicType() ) &&
        !aGrfUpdateSlots.empty() )
    {
        sal_Bool bProtect = 0 != rSh.IsSelObjProtected(FLYPROTECT_CONTENT|FLYPROTECT_PARENT);
        SfxViewFrame* pVFrame = GetView().GetViewFrame();
        sal_uInt16 nSlot;
        std::set<sal_uInt16>::iterator it;
        for( it = aGrfUpdateSlots.begin(); it != aGrfUpdateSlots.end(); ++it )
        {
            sal_Bool bSetState = sal_False;
            sal_Bool bState = sal_False;
            switch( nSlot = *it )
            {
            case SID_IMAP:
            case SID_IMAP_EXEC:
                {
                    sal_uInt16 nId = SvxIMapDlgChildWindow::GetChildWindowId();
                    SvxIMapDlg *pDlg = pVFrame->HasChildWindow( nId ) ?
                        (SvxIMapDlg*) ( pVFrame->GetChildWindow( nId )
                                            ->GetWindow()) : 0;

                    if( pDlg && ( SID_IMAP_EXEC == nSlot ||
                                ( SID_IMAP == nSlot && !bProtect)) &&
                        pDlg->GetEditingObject() != rSh.GetIMapInventor())
                            lcl_UpdateIMapDlg( rSh );

                    if( !bProtect && SID_IMAP == nSlot )
                        bSetState = sal_True, bState = 0 != pDlg;
                }
                break;

            case SID_CONTOUR_DLG:
                if( !bProtect )
                {
                    sal_uInt16 nId = SvxContourDlgChildWindow::GetChildWindowId();
                    SvxIMapDlg *pDlg = pVFrame->HasChildWindow( nId ) ?
                        (SvxIMapDlg*) ( pVFrame->GetChildWindow( nId )
                                            ->GetWindow()) : 0;
                    if( pDlg && pDlg->GetEditingObject() !=
                                rSh.GetIMapInventor() )
                        lcl_UpdateContourDlg( rSh, nsSelectionType::SEL_GRF );

                    bSetState = sal_True;
                    bState = 0 != pDlg;
                }
                break;

            case FN_FRAME_WRAP_CONTOUR:
                if( !bProtect )
                {
                    SfxItemSet aSet(GetPool(), RES_SURROUND, RES_SURROUND);
                    rSh.GetFlyFrmAttr(aSet);
                    const SwFmtSurround& rWrap = (const SwFmtSurround&)aSet.Get(RES_SURROUND);
                    bSetState = sal_True;
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
                bSetState = bState = GRAPHIC_BITMAP == nGrfType;
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
    return 0;
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
                     (rSh.IsSelFrmMode() &&
                      !(rSh.GetSelectionType() & nsSelectionType::SEL_GRF)) )
                    rSet.DisableItem( nWhich );
                break;
            case SID_GALLERY_ENABLE_ADDCOPY:
                // #108230# allow copy from gallery in Writer AND Writer/Web!
                rSet.Put( SfxBoolItem( SID_GALLERY_ENABLE_ADDCOPY, sal_True ) );
                break;
            case FN_EDIT_REGION:
                if( !rSh.IsAnySectionInDoc() )
                    rSet.DisableItem(nWhich);
                break;

            case FN_INSERT_REGION:
                if( rSh.IsSelFrmMode() ||
                    !rSh.IsInsRegionAvailable() )
                    rSet.DisableItem( nWhich );
                break;
            case FN_CONVERT_TABLE_TO_TEXT:
            {
                sal_uInt16 eFrmType = rSh.GetFrmType(0,sal_True);
                if( (eFrmType & FRMTYPE_FOOTNOTE) ||
                    !rSh.GetTableFmt() )
                    rSet.DisableItem( nWhich );
            }
            break;
            case FN_CONVERT_TEXT_TO_TABLE:
            {
                sal_uInt16 eFrmType = rSh.GetFrmType(0,sal_True);
                if( (eFrmType & FRMTYPE_FOOTNOTE) ||
                    !rSh.IsTextToTableAvailable()  )
                    rSet.DisableItem( nWhich );
            }
            break;
            case FN_CONVERT_TEXT_TABLE:
            {
                sal_uInt16 eFrmType = rSh.GetFrmType(0,sal_True);
                if( (eFrmType & FRMTYPE_FOOTNOTE) ||
                    (!rSh.GetTableFmt() && !rSh.IsTextToTableAvailable() ) )
                    rSet.DisableItem( nWhich );
            }
            break;
            case RES_SHADOW:
            {
                SfxItemSet   aSet( rSh.GetAttrPool(),
                                    RES_SHADOW, RES_SHADOW );

                // Tabellenzelle(n) selektiert?
                if ( rSh.IsTableMode() )
                {
                    SwFrmFmt *pFmt = rSh.GetTableFmt();
                    aSet.Put(pFmt->GetFmtAttr( nWhich, sal_True ));
                }
                else if( rSh.IsFrmSelected() )
                {
                    SwFlyFrmAttrMgr aMgr( sal_False, &rSh, FRMMGR_TYPE_NONE );
                    aSet.Put( aMgr.GetAttrSet() );
                }
                else
                    rSh.GetCurAttr( aSet );

                const SvxShadowItem& rShItem = (const SvxShadowItem&)aSet.Get(nWhich);
                rSet.Put(rShItem);
            }
            break;
            case SID_IMAP:
            {
                // #i59688#
                // improve efficiency:
                // If selected object is protected, item has to disabled.
                const sal_Bool bProtect = 0 != rSh.IsSelObjProtected(FLYPROTECT_CONTENT|FLYPROTECT_PARENT);
                if ( bProtect )
                {
                    rSet.DisableItem( nWhich );
                }
                else
                {
                    const sal_uInt16 nId = SvxIMapDlgChildWindow::GetChildWindowId();
                    const sal_Bool bHas = pVFrame->HasChildWindow( nId );
                    const sal_Bool bFrmSel = rSh.IsFrmSelected();
                    const sal_Bool bIsGraphicSelection =
                                rSh.GetSelectionType() == nsSelectionType::SEL_GRF;

                    // #i59688#
                    // avoid unnecessary loading of selected graphic.
                    // The graphic is only needed, if the dialog is open.
                    //wenn die Grafik ausgeswappt ist, dann muss der
                    //Status asynchron ermittelt werden
                    //bis dahin wird der Slot disabled
                    if ( bHas && bIsGraphicSelection && rSh.IsGrfSwapOut( sal_True ) )
                    {
                        if( AddGrfUpdateSlot( nWhich ))
                            rSh.GetGraphic(sal_False);  // start the loading
                    }
                    else
                    {
                        if ( !bHas &&
                             ( !bFrmSel ||
                               ( bIsGraphicSelection &&
                                 rSh.GetGraphicType() == GRAPHIC_NONE ) ) )
                        {
                            rSet.DisableItem( nWhich );
                        }
                        else
                        {
                            SfxBoolItem aBool(nWhich, bHas);
                            if ( bHas && bFrmSel )
                                lcl_UpdateIMapDlg( rSh );
                            rSet.Put(aBool);
                        }
                    }
                }
            }
            break;
            case SID_IMAP_EXEC:
            {
                sal_Bool bDisable = sal_False;
                if( !rSh.IsFrmSelected())
                    bDisable = sal_True;
                sal_uInt16 nId = SvxIMapDlgChildWindow::GetChildWindowId();
                if(!bDisable && pVFrame->HasChildWindow( nId ))
                {
                    if(rSh.GetSelectionType() == nsSelectionType::SEL_GRF
                                    && rSh.IsGrfSwapOut(sal_True))
                    {
                        if( AddGrfUpdateSlot( nWhich ))
                            rSh.GetGraphic(sal_False);  // start the loading
                    }
                    else
                    {
                        SvxIMapDlg *pDlg = SWIMAPDLG(GetView());
                        if( pDlg->GetEditingObject() != rSh.GetIMapInventor() )
                            lcl_UpdateIMapDlg( rSh );
                    }
                }
                rSet.Put(SfxBoolItem(nWhich, bDisable));
            }
            break;
            case FN_BACKSPACE:
            case SID_DELETE:
                if (rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) != 0)
                    rSet.DisableItem( nWhich );
                break;
            case SID_CONTOUR_DLG:
            {
                sal_Bool bParentCntProt = 0 != rSh.IsSelObjProtected(FLYPROTECT_CONTENT|FLYPROTECT_PARENT );

                if( bParentCntProt || 0 != (HTMLMODE_ON & ::GetHtmlMode(
                                            GetView().GetDocShell() )) )
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_uInt16 nId = SvxContourDlgChildWindow::GetChildWindowId();
                    sal_Bool bHas = GetView().GetViewFrame()->HasChildWindow( nId );
                    int nSel = rSh.GetSelectionType();
                    sal_Bool bOk = 0 != (nSel & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_OLE));

                    sal_Bool bDisable = sal_False;
                    if( !bHas && !bOk )
                        bDisable = sal_True;
                    // #i59688#
                    // avoid unnecessary loading of selected graphic.
                    // The graphic is only needed, if the dialog is open.
                    // wenn die Grafik ausgeswappt ist, dann muss der Status
                    // asynchron ermittelt werden bis dahin wird der Slot
                    // disabled
                    else if ( bHas && (nSel & nsSelectionType::SEL_GRF) &&
                              rSh.IsGrfSwapOut(sal_True) )
                    {
                        if( AddGrfUpdateSlot( nWhich ))
                            rSh.GetGraphic(sal_False);  // start the loading
                        // #i75481#
                        bDisable = sal_True;
                    }
                    else if( bHas && bOk )
                        bDisable = !lcl_UpdateContourDlg( rSh, nSel );
                    else if( bOk )
                    {
                        // #i75481#
                        // apply fix #i59688# only for selected graphics
                        if ( nSel & nsSelectionType::SEL_GRF )
                            bDisable = GRAPHIC_NONE == rSh.GetGraphicType();
                        else
                            bDisable = GRAPHIC_NONE == rSh.GetIMapGraphic().GetType();
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
                sal_Bool bDisable = sal_False;
                int nSel = rSh.GetSelectionType();
                if( !(nSel & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_OLE)) )
                    bDisable = sal_True;
                sal_uInt16 nId = SvxContourDlgChildWindow::GetChildWindowId();
                if( !bDisable && GetView().GetViewFrame()->HasChildWindow( nId ))
                {
                    SvxContourDlg *pDlg = SWCONTOURDLG(GetView());
                    if( pDlg->GetEditingObject() != rSh.GetIMapInventor() )
                        bDisable = sal_True;
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
                sal_Bool bObj = 0 != rSh.IsObjSelected();
                sal_Bool bParentCntProt = rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) != 0;

                if( !bParentCntProt && (bObj || rSh.IsFrmSelected()))
                {
                    SfxItemSet aSet(GetPool(), RES_ANCHOR, RES_ANCHOR);
                    if(bObj)
                        rSh.GetObjAttr(aSet);
                    else
                        rSh.GetFlyFrmAttr(aSet);
                    RndStdIds eSet = ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetAnchorId();
                    const sal_Bool bSet =
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
                sal_Bool bObj = 0 != rSh.IsObjSelected();
                sal_Bool bParentCntProt = rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) != 0;

                if( !bParentCntProt && (bObj || rSh.IsFrmSelected()))
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
                        rSh.GetFlyFrmAttr(aSet);
                        nAnchorType = ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetAnchorId();
                    }
                    const SwFmtSurround& rWrap = (const SwFmtSurround&)aSet.Get(RES_SURROUND);

                    const SvxOpaqueItem& rOpaque = (const SvxOpaqueItem&)aSet.Get(RES_OPAQUE);
                    sal_Bool bOpaque = rOpaque.GetValue();
                    SwSurround nSurround = rWrap.GetSurround();
                    sal_Bool bSet = sal_False;

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
                            bSet = rWrap.IsContour();
                            if( !bDisable )
                            {
                                int nSel = rSh.GetSelectionType();
                                if( (nSel & nsSelectionType::SEL_GRF) &&
                                            rSh.IsGrfSwapOut(sal_True))
                                {
                                    if( AddGrfUpdateSlot( nWhich ))
                                        rSh.GetGraphic(sal_False);  // start the loading
                                }
                                else if( rSh.IsFrmSelected() )
                                {
                                    // #i102253# applied patch from OD (see task)
                                    bDisable =
                                        nSel & nsSelectionType::SEL_FRM ||
                                        GRAPHIC_NONE == rSh.GetIMapGraphic().GetType();
                                }
                            }
                            bSet = bDisable ? sal_False : rWrap.IsContour();

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
                    sal_Bool bValue = ! rSh.GetViewOptions()->IsFormView();
                    rSet.Put( SfxBoolItem( nWhich, bValue ) );
                }
                else
                    rSet.Put( SfxVisibilityItem( nWhich, sal_False ) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
    pGetStateSet = 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Slots mit dieser Statusmethode disablen
 --------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------
    Beschreibung:   Slots mit dieser Statusmethode disablen
 --------------------------------------------------------------------*/
void SwBaseShell::StateStyle( SfxItemSet &rSet )
{
    sal_Bool bParentCntProt = GetShell().IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) != 0;
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
    sal_Bool bObj = 0 != rSh.IsObjSelected();
    if( bObj || rSh.IsFrmSelected())
    {
        SfxItemSet aSet(GetPool(), RES_OPAQUE, RES_SURROUND);
        if(bObj)
            rSh.GetObjAttr(aSet);
        else
            rSh.GetFlyFrmAttr(aSet);
        SwFmtSurround aWrap( (SwFmtSurround&)aSet.Get(RES_SURROUND) );
        SwSurround nOldSurround(aWrap.GetSurround());
        SwSurround nSurround = SURROUND_PARALLEL;

        switch (nSlot)
        {
            case FN_FRAME_NOWRAP:
                nSurround = SURROUND_NONE;
                if (aWrap.IsContour())
                    aWrap.SetContour(sal_False);
                break;
            case FN_FRAME_WRAP_IDEAL:
                nSurround = SURROUND_IDEAL;
                break;
            case FN_WRAP_ANCHOR_ONLY:
                aWrap.SetAnchorOnly(!aWrap.IsAnchorOnly());
                //
                // keep previous wrapping
                //
                // switch to wrap SURROUND_PARALLEL, if previous wrap is SURROUND_NONE
                if ( nOldSurround != SURROUND_NONE )
                {
                    nSurround = nOldSurround;
                }
                break;
            case FN_FRAME_WRAP_CONTOUR:
                aWrap.SetContour(!aWrap.IsContour());
                if (nSurround == SURROUND_THROUGHT)
                    nSurround = SURROUND_PARALLEL;
                break;
            case FN_FRAME_WRAPTHRU_TRANSP:
                if (aWrap.IsContour())
                    aWrap.SetContour(sal_False);
                // kein break!!!
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
            // Konturumfluss bei Draw-Objekten defaulten
            if (bObj && nOldSurround != nSurround &&
                (nOldSurround == SURROUND_NONE || nOldSurround == SURROUND_THROUGHT))
            {
                aWrap.SetContour(sal_True);
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
            rSh.SetFlyFrmAttr(aSet);
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Update der Statuszeile erzwingen
 --------------------------------------------------------------------*/
void SwBaseShell::SetFrmMode(FlyMode eMode, SwWrtShell *pSh )
{
    eFrameMode = eMode;
    SfxBindings &rBnd = pSh->GetView().GetViewFrame()->GetBindings();

    if( eMode == FLY_DRAG ||
        (pSh && (pSh->IsFrmSelected() || pSh->IsObjSelected())) )
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

/*--------------------------------------------------------------------
    Beschreibung:   Ctor
 --------------------------------------------------------------------*/

SwBaseShell::SwBaseShell(SwView& rVw) :
    SfxShell( &rVw ),
    rView(rVw),
    pGetStateSet(0)
{
    SwWrtShell& rWrtSh = rView.GetWrtShell();

    SetPool(&rWrtSh.GetAttrPool());
    SetName(rtl::OUString("Base"));
    rWrtSh.SetGrfArrivedLnk( LINK( this, SwBaseShell, GraphicArrivedHdl));
}

SwBaseShell::~SwBaseShell()
{
    if( rView.GetCurShell() == this )
        rView.ResetSubShell();

    Link aTmp( LINK( this, SwBaseShell, GraphicArrivedHdl));
    if( aTmp == rView.GetWrtShell().GetGrfArrivedLnk() )
        rView.GetWrtShell().SetGrfArrivedLnk( Link() );
}

void SwBaseShell::ExecTxtCtrl( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();

    if( pArgs)
    {
        SwWrtShell &rSh = GetShell();
        SvxScriptSetItem* pSSetItem = 0;
        sal_uInt16 nSlot = rReq.GetSlot();
        SfxItemPool& rPool = rSh.GetAttrPool();
        sal_uInt16 nWhich = rPool.GetWhich( nSlot );
        sal_uInt16 nScripts = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
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
            }
            case SID_ATTR_CHAR_POSTURE:
            case SID_ATTR_CHAR_WEIGHT:
            {
                pSSetItem = new SvxScriptSetItem( nSlot, rPool );
                pSSetItem->PutItemForScriptType( nScripts, pArgs->Get( nWhich ));
                pArgs = &pSSetItem->GetItemSet();
            }
            break;
            case SID_ATTR_CHAR_FONTHEIGHT:
            {
                if(rSh.HasSelection())
                {
                    pSSetItem = new SvxScriptSetItem( nSlot, rPool );
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
                        case SCRIPTTYPE_LATIN:
                            nCJKSize = nHeight * nCJKSize / nWesternSize;
                            nCTLSize = nHeight * nCTLSize / nWesternSize;
                            nWesternSize = (sal_Int32) nHeight;
                        break;
                        case SCRIPTTYPE_ASIAN:
                            nCTLSize = nHeight* nCTLSize / nCJKSize;
                            nWesternSize = nHeight * nWesternSize / nCJKSize;
                            nCJKSize = (sal_Int32) nHeight;
                        break;
                        case SCRIPTTYPE_COMPLEX:
                            nCJKSize = nHeight * nCJKSize / nCTLSize;
                            nWesternSize = nHeight * nWesternSize / nCTLSize;
                            nCTLSize = (sal_Int32) nHeight;
                        break;
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
                SwTxtFmtColl * pColl = rSh.GetCurTxtFmtColl();
                if ( pColl && pColl->IsAutoUpdateFmt() )
                {
                    rSh.AutoUpdatePara( pColl, *pArgs );
                    bAuto = true;
                }
            }

            if (!bAuto)
            {
                rSh.SetAttr( *pArgs );
            }
        }
        delete pSSetItem;
    }
    else
        GetView().GetViewFrame()->GetDispatcher()->Execute( SID_CHAR_DLG, sal_False);
    rReq.Done();
}

void SwBaseShell::GetTxtCtrlState( SfxItemSet& rSet )
{
    SwWrtShell &rSh = GetShell();
    rSh.GetCurAttr( rSet );
}

void SwBaseShell::GetTxtFontCtrlState( SfxItemSet& rSet )
{
    SwWrtShell &rSh = GetShell();
    sal_Bool bFirst = sal_True;
    SfxItemSet* pFntCoreSet = 0;
    sal_uInt16 nScriptType = SCRIPTTYPE_LATIN;
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
                    pFntCoreSet = new SfxItemSet( *rSet.GetPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END-1 );
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
                aSetItem.GetItemSet().Put( *pFntCoreSet, sal_False );
                const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                if( pI )
                    rSet.Put( *pI, nWhich );
                else
                    rSet.InvalidateItem( nWhich );
                //set input context of the SwEditWin according to the selected font and script type
                if(RES_CHRATR_FONT == nWhich)
                {
                    Font aFont;
                    if(pI && pI->ISA(SvxFontItem))
                    {
                        aFont.SetName( ((const SvxFontItem*)pI)->GetFamilyName());
                        aFont.SetStyleName(((const SvxFontItem*)pI)->GetStyleName());
                        aFont.SetFamily(((const SvxFontItem*)pI)->GetFamily());
                        aFont.SetPitch(((const SvxFontItem*)pI)->GetPitch());
                        aFont.SetCharSet(((const SvxFontItem*)pI)->GetCharSet());
                    }

                    sal_Bool bVertical = rSh.IsInVerticalText();
                    aFont.SetOrientation(bVertical ? 2700 : 0);
                    aFont.SetVertical(bVertical);
                    GetView().GetEditWin().SetInputContext( InputContext( aFont, INPUTCONTEXT_TEXT |
                                                        INPUTCONTEXT_EXTTEXTINPUT ) );
                }
            }
            break;

        default:
            if( bFirst )
            {
                rSh.GetCurAttr( rSet );
                bFirst = sal_False;
            }
        }
        nWhich = aIter.NextWhich();
    }
    delete pFntCoreSet;
}

void SwBaseShell::GetBckColState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    int nSelType = rSh.GetSelectionType();

    if( nSelType & nsSelectionType::SEL_OLE )
    {
        rSet.DisableItem( SID_BACKGROUND_COLOR );
        return;
    }

    if ( nSelType & nsSelectionType::SEL_FRM )
    {
        sal_Bool bParentCntProt = rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) != 0;
        if (bParentCntProt)
        {
            rSet.DisableItem( SID_BACKGROUND_COLOR );
            return;
        }
    }

    SvxBrushItem aBrushItem( RES_BACKGROUND );

    if( nsSelectionType::SEL_TBL_CELLS & nSelType )
        rSh.GetBoxBackground( aBrushItem );
    else
    {
        SfxItemSet aCoreSet(GetPool(), RES_BACKGROUND, RES_BACKGROUND);
        if( nSelType & nsSelectionType::SEL_GRF || nsSelectionType::SEL_FRM & nSelType )
            rSh.GetFlyFrmAttr( aCoreSet );
        else
            rSh.GetCurAttr( aCoreSet );
        aBrushItem = (const SvxBrushItem&)aCoreSet.Get(RES_BACKGROUND);
    }

    while ( nWhich )
    {
        switch(nWhich)
        {
            case SID_BACKGROUND_COLOR  :
            {
                SvxColorItem aColorItem(aBrushItem.GetColor(), SID_BACKGROUND_COLOR);
                rSet.Put( aColorItem, SID_BACKGROUND_COLOR );
            }
            break;
            case SID_ATTR_BRUSH:
            case RES_BACKGROUND:
                rSet.Put( aBrushItem, GetPool().GetWhich(nWhich) );
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void SwBaseShell::ExecBckCol(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    int nSelType = rSh.GetSelectionType();
    if ( nSelType & nsSelectionType::SEL_OLE )
    {
        return;
    }

    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    if( !pArgs  && nSlot != SID_BACKGROUND_COLOR)
        return ;

    SvxBrushItem aBrushItem( RES_BACKGROUND );

    if( nsSelectionType::SEL_TBL_CELLS & nSelType )
    {
        rSh.GetBoxBackground( aBrushItem );
    }
    else
    {
        SfxItemSet aCoreSet(GetPool(), RES_BACKGROUND, RES_BACKGROUND);
        if( (nsSelectionType::SEL_FRM & nSelType) || (nsSelectionType::SEL_GRF & nSelType) )
            rSh.GetFlyFrmAttr( aCoreSet );
        else
            rSh.GetCurAttr( aCoreSet );
        aBrushItem = (const SvxBrushItem&)aCoreSet.Get(RES_BACKGROUND);
    }

    switch (nSlot)
    {
        // RES_BACKGROUND (=SID_ATTR_BRUSH) muss ueber zwei IDs
        // gesetzt werden:
        case SID_BACKGROUND_COLOR:
            {
                aBrushItem.SetGraphicPos(GPOS_NONE);

                if(pArgs)
                {
                    const SvxColorItem& rNewColorItem = (const SvxColorItem&)
                                            pArgs->Get(SID_BACKGROUND_COLOR);
                    const Color& rNewColor = rNewColorItem.GetValue();
                    aBrushItem.SetColor( rNewColor );
                    GetView().GetViewFrame()->GetBindings().SetState(rNewColorItem);
                }
                else
                {
                    aBrushItem.SetColor( COL_TRANSPARENT );
                    rReq.AppendItem( SvxColorItem( Color( COL_TRANSPARENT ), nSlot ) );
                }
            }
            break;

        case SID_ATTR_BRUSH:
        case RES_BACKGROUND:
        {
            const SvxBrushItem& rNewBrushItem = (const SvxBrushItem&)
                                    pArgs->Get( GetPool().GetWhich(nSlot) );
            aBrushItem = rNewBrushItem;
        }
        break;
        default:
            rReq.Ignore();
            OSL_FAIL("unknown message in ExecuteAttr!" );
            return;
    }

    if( nsSelectionType::SEL_TBL_CELLS & nSelType )
    {
        rSh.SetBoxBackground( aBrushItem );
    }
    else if( (nsSelectionType::SEL_FRM & nSelType) ||
        (nsSelectionType::SEL_GRF & nSelType)  )
    {
        SfxItemSet aCoreSet(GetPool(), RES_BACKGROUND, RES_BACKGROUND);
        aCoreSet.Put( aBrushItem );
        // Vorlagen-AutoUpdate
        SwFrmFmt* pFmt = rSh.GetCurFrmFmt();
        if(pFmt && pFmt->IsAutoUpdateFmt())
            rSh.AutoUpdateFrame( pFmt, aCoreSet);
        else
            rSh.SetFlyFrmAttr( aCoreSet );
    }
    else
    {
        SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
        if( pColl && pColl->IsAutoUpdateFmt())
        {
            SfxItemSet aSet(GetPool(), RES_BACKGROUND, RES_BACKGROUND );
            aSet.Put(aBrushItem);
            rSh.AutoUpdatePara( pColl, aSet);
        }
        else
            rSh.SetAttr( aBrushItem );
    }

    rReq.Done();
}

void SwBaseShell::GetBorderState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    // Tabellenzelle(n) selektiert?
    sal_Bool bPrepare = sal_True;
    sal_Bool bTableMode = rSh.IsTableMode();
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
    else if ( rSh.IsFrmSelected() )
    {
        SwFlyFrmAttrMgr aMgr( sal_False, &rSh, FRMMGR_TYPE_NONE );
        rSet.Put( aMgr.GetAttrSet() );
        bPrepare = sal_False;
    }
    else
        // Umrandungsattribute ganz normal ueber Shell holen
        rSh.GetCurAttr( rSet );
    if ( bPrepare )
        ::PrepareBoxInfo( rSet, rSh );
    // switch the border toolbox controller mode
    rSet.Put( SfxBoolItem( SID_BORDER_REDUCED_MODE, !bTableMode ));
}

void SwBaseShell::ExecDlg(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    Window *pMDI = &GetView().GetViewFrame()->GetWindow();
    //Damit aus dem Basic keine Dialoge fuer Hintergrund-Views aufgerufen werden:
    sal_Bool bBackground = (&GetView() != GetActiveView());
    const SfxPoolItem* pItem = 0;
    const SfxItemSet* pArgs = rReq.GetArgs();

    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pOutSet = 0;
    bool bDone = false;
    if(pArgs)
        pArgs->GetItemState( GetPool().GetWhich(nSlot), sal_False, &pItem );

    switch ( nSlot )
    {
        case FN_FORMAT_TITLEPAGE_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            VclAbstractDialog* pDlg = pFact->CreateTitlePageDlg( pMDI );
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_FORMAT_PAGE_COLUMN_DLG:
        case FN_FORMAT_PAGE_DLG:
        {
            if( !bBackground )
            {
                const sal_uInt16 nCurIdx = rSh.GetCurPageDesc();
                const SwPageDesc& rPageDesc = rSh.GetPageDesc( nCurIdx );
                //temp. View, weil die Shell nach dem Dialog nicht mehr gueltig sein muss
                //z.B. Kopfzeile ausschalten
                SwView& rTempView = GetView();
                sal_uInt16 nPageId = 0;
                if ( nSlot == FN_FORMAT_PAGE_COLUMN_DLG )
                    nPageId = TP_COLUMN ;
                rTempView.GetDocShell()->FormatPage(rPageDesc.GetName(),
                                    nPageId, &rSh );
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
            SfxAbstractDialog * pDlg = 0;
            // Tabellenzelle(n) selektiert?
            if ( rSh.IsTableMode() )
            {
                // Umrandungattribute Get/SetTabBorders() setzen
                ::PrepareBoxInfo( aSet, rSh );
                rSh.GetTabBorders( aSet );
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                pDlg = pFact->CreateSwBorderDlg( pMDI, aSet, SW_BORDER_MODE_TABLE, RC_DLG_SWBORDERDLG );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetTabBorders( *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else if ( rSh.IsFrmSelected() )
            {
                // Umrandungsattribute ueber Frame-Manager setzen
                SwFlyFrmAttrMgr aMgr( sal_False, &rSh, FRMMGR_TYPE_NONE );
                aSet.Put( aMgr.GetAttrSet() );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                pDlg = pFact->CreateSwBorderDlg( pMDI, aSet, SW_BORDER_MODE_FRAME, RC_DLG_SWBORDERDLG );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if ( pDlg->Execute() == RET_OK )
                {
                    aMgr.SetAttrSet( *pDlg->GetOutputItemSet() );
                    aMgr.UpdateFlyFrm();
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else
            {
                // Umrandungsattribute ganz normal ueber Shell setzen
                rSh.GetCurAttr( aSet );
                ::PrepareBoxInfo( aSet, rSh );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                pDlg = pFact->CreateSwBorderDlg( pMDI, aSet, SW_BORDER_MODE_PARA, RC_DLG_SWBORDERDLG );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetAttr( *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            if(pOutSet)
            {
                rReq.Done(*pOutSet);
                bDone = true;
            }
            delete pDlg;
        }
        break;
        case FN_FORMAT_BACKGROUND_DLG:
        {
            SfxItemSet aSet( rSh.GetAttrPool(),
                             RES_BACKGROUND, RES_BACKGROUND );

            SfxAbstractDialog * pDlg = 0;
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");


            // Tabellenzelle(n) selektiert?
            if ( rSh.IsTableMode() )
            {
                //Hintergrundattribute der Tabelle holen und in den Set packen
                SvxBrushItem aBrush(RES_BACKGROUND);
                rSh.GetBoxBackground( aBrush );
                pDlg = pFact->CreateSfxDialog( pMDI, aSet,
                    rView.GetViewFrame()->GetFrame().GetFrameInterface(),
                    RC_SWDLG_BACKGROUND );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                aSet.Put( aBrush );
                if ( pDlg->Execute() == RET_OK )
                {

                    rSh.SetBoxBackground( (SvxBrushItem&)
                        pDlg->GetOutputItemSet()->Get( RES_BACKGROUND ));
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else if ( rSh.IsFrmSelected() )
            {

                rSh.GetFlyFrmAttr( aSet );

                pDlg = pFact->CreateSfxDialog( pMDI, aSet,
                    rView.GetViewFrame()->GetFrame().GetFrameInterface(),
                    RC_SWDLG_BACKGROUND );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetFlyFrmAttr((SfxItemSet &) *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            else
            {
                // Umrandungsattribute ganz normal ueber Shell setzen
                rSh.GetCurAttr( aSet );

                pDlg = pFact->CreateSfxDialog( pMDI, aSet,
                    rView.GetViewFrame()->GetFrame().GetFrameInterface(),
                    RC_SWDLG_BACKGROUND );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if ( pDlg->Execute() == RET_OK )
                {
                    rSh.SetAttr( *pDlg->GetOutputItemSet() );
                    pOutSet = pDlg->GetOutputItemSet();
                }
            }
            if(pOutSet)
            {
                rReq.Done(*pOutSet);
                bDone = true;
            }
            delete pDlg;

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

    if ( !( rSh.GetFrmType( 0, sal_True ) & FRMTYPE_FOOTNOTE ) )
    {
        SwView &rTempView = GetView(); // Da GetView() nach Shellwechsel nicht mehr geht
        sal_Bool bHTMLMode = 0 != (::GetHtmlMode(rTempView.GetDocShell())&HTMLMODE_ON);
        sal_Bool bCallEndUndo = sal_False;

        if( !pArgs && rSh.IsSelection() && !rSh.IsInClickToEdit() &&
            !rSh.IsTableMode() )
        {
            const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
            SwInsertTableOptions aInsTblOpts = pModOpt->GetInsTblFlags(bHTMLMode);

            rSh.StartUndo(UNDO_INSTABLE);
            bCallEndUndo = sal_True;

            sal_Bool bInserted = rSh.TextToTable( aInsTblOpts, '\t', text::HoriOrientation::FULL );
            rSh.EnterStdMode();
            if (bInserted)
                rTempView.AutoCaption(TABLE_CAP);
            _rRequest.Done();
        }
        else
        {
            sal_uInt16 nCols = 0;
            sal_uInt16 nRows = 0;
            SwInsertTableOptions aInsTblOpts( tabopts::ALL_TBL_INS_ATTR, 1 );
            String aTableName, aAutoName;
            SwTableAutoFmt* pTAFmt = 0;

            if( pArgs && pArgs->Count() >= 2 )
            {
                SFX_REQUEST_ARG( _rRequest, pName, SfxStringItem, FN_INSERT_TABLE, sal_False );
                SFX_REQUEST_ARG( _rRequest, pCols, SfxUInt16Item, SID_ATTR_TABLE_COLUMN, sal_False );
                SFX_REQUEST_ARG( _rRequest, pRows, SfxUInt16Item, SID_ATTR_TABLE_ROW, sal_False );
                SFX_REQUEST_ARG( _rRequest, pFlags, SfxInt32Item, FN_PARAM_1, sal_False );
                SFX_REQUEST_ARG( _rRequest, pAuto, SfxStringItem, FN_PARAM_2, sal_False );

                if ( pName )
                    aTableName = pName->GetValue();
                if ( pCols )
                    nCols = pCols->GetValue();
                if ( pRows )
                    nRows = pRows->GetValue();
                if ( pAuto )
                {
                    aAutoName = pAuto->GetValue();
                    if ( aAutoName.Len() )
                    {
                        SwTableAutoFmtTbl aTableTbl;
                        aTableTbl.Load();
                        for ( sal_uInt16 n=0; n<aTableTbl.size(); n++ )
                        {
                            if ( aTableTbl[n].GetName() == aAutoName )
                            {
                                pTAFmt = new SwTableAutoFmt( aTableTbl[n] );
                                break;
                            }
                        }
                    }
                }

                if ( pFlags )
                    aInsTblOpts.mnInsMode = (sal_uInt16) pFlags->GetValue();
                else
                {
                    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                    aInsTblOpts = pModOpt->GetInsTblFlags(bHTMLMode);
                }
            }

            if( !nCols || !nRows )
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                AbstractInsTableDlg* pDlg = pFact->CreateInsTableDlg( DLG_INSERT_TABLE, rTempView );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if( RET_OK == pDlg->Execute() )
                {
                    pDlg->GetValues( aTableName, nRows, nCols, aInsTblOpts, aAutoName, pTAFmt );
                }
                else
                    _rRequest.Ignore();
                delete pDlg;
            }

            if( nCols && nRows )
            {
                // record before shell change
                _rRequest.AppendItem( SfxStringItem( FN_INSERT_TABLE, aTableName ) );
                if ( aAutoName.Len() )
                    _rRequest.AppendItem( SfxStringItem( FN_PARAM_2, aAutoName ) );
                _rRequest.AppendItem( SfxUInt16Item( SID_ATTR_TABLE_COLUMN, nCols ) );
                _rRequest.AppendItem( SfxUInt16Item( SID_ATTR_TABLE_ROW, nRows ) );
                _rRequest.AppendItem( SfxInt32Item( FN_PARAM_1, (sal_Int32) aInsTblOpts.mnInsMode ) );
                _rRequest.Done();

                rSh.StartUndo(UNDO_INSTABLE);
                bCallEndUndo = sal_True;

                rSh.StartAllAction();
                if( rSh.HasSelection() )
                    rSh.DelRight();

                rSh.InsertTable( aInsTblOpts, nRows, nCols, text::HoriOrientation::FULL, pTAFmt );
                rSh.MoveTable( fnTablePrev, fnTableStart );

                if( aTableName.Len() && !rSh.GetTblStyle( aTableName ) )
                    rSh.GetTableFmt()->SetName( aTableName );

                rSh.EndAllAction();
                rTempView.AutoCaption(TABLE_CAP);
            }
            delete pTAFmt;
        }

        if( bCallEndUndo )
        {
            SwRewriter aRewriter;

            if (rSh.GetTableFmt())
            {
                aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_START_QUOTE));
                aRewriter.AddRule(UndoArg2, rSh.GetTableFmt()->GetName());
                aRewriter.AddRule(UndoArg3, SW_RESSTR(STR_END_QUOTE));

            }
            rSh.EndUndo(UNDO_INSTABLE, &aRewriter); // wegen moegl. Shellwechsel
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
            std::vector<String> &rLst = aLst.GetList();
            nParagraphPos = nGraphicPos = nOlePos = nFramePos = nTablePos =
            nTableRowPos  = nTableCellPos = nPagePos =
            nHeaderPos    = nFooterPos = 0;
            sal_uInt8 nPos = 1;
            rLst.push_back( SW_RESSTR( STR_SWBG_PAGE ) );
            nPagePos = nPos++;
            sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
            sal_Bool bHtmlMode = 0 != (nHtmlMode & HTMLMODE_ON);

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
                const sal_uInt16 nType = rSh.GetFrmType(0,sal_True);
                if ( nType & FRMTYPE_HEADER )
                {
                    rLst.push_back( SW_RESSTR( STR_SWBG_HEADER ) );
                    nHeaderPos = nPos++;
                }
                if ( nType & FRMTYPE_FOOTER )
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
            int nSel = rSh.GetSelectionType();
            if ( nSel & nsSelectionType::SEL_DRW_TXT )
                break;

            sal_uInt8 nPos = (sal_uInt8)((SfxUInt16Item &)pArgs->Get(SID_GALLERY_BG_POS)).GetValue();
            ++nPos;

            SvxBrushItem aBrush( (SvxBrushItem&)pArgs->Get(SID_GALLERY_BG_BRUSH));
            aBrush.SetWhich( RES_BACKGROUND );
            if ( nPos == nParagraphPos )
                rSh.SetAttr( aBrush );
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
                rSh.SetFlyFrmAttr( aCoreSet );
            }
            else if ( nPos == nPagePos || nPos == nHeaderPos || nPos == nFooterPos )
            {
                sal_uInt16 nDesc = rSh.GetCurPageDesc();
                SwPageDesc aDesc( rSh.GetPageDesc( nDesc ) );
                if ( nPos == nPagePos )
                    aDesc.GetMaster().SetFmtAttr( aBrush );
                else if ( nPos == nHeaderPos )
                {
                    SwFmtHeader aHead( aDesc.GetMaster().GetHeader() );
                    aHead.GetHeaderFmt()->SetFmtAttr( aBrush );
                    aDesc.GetMaster().SetFmtAttr( aHead );
                }
                else if ( nPos == nFooterPos )
                {
                    SwFmtFooter aFoot( aDesc.GetMaster().GetFooter() );
                    aFoot.GetFooterFmt()->SetFmtAttr( aBrush );
                    aDesc.GetMaster().SetFmtAttr( aFoot );
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

            VclAbstractDialog* pDlg = pFact->CreateSwChangeDBDlg(GetView(), DLG_CHANGE_DB );
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
        }
        break;
        default:
            OSL_FAIL("wrong dispatcher");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
