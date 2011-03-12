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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <basic/sbxvar.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <textsh.hxx>
#include <num.hxx>
#include <edtwin.hxx>
#include <crsskip.hxx>
#include <doc.hxx>
#include <docsh.hxx>

#include <cmdid.h>
#include <globals.h>
#include <globals.hrc>

#include <svx/svdouno.hxx>
#include <svx/fmshell.hxx>
#include <svx/sdrobjectfilter.hxx>

using namespace ::com::sun::star;

void SwTextShell::ExecBasicMove(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    GetView().GetEditWin().FlushInBuffer();
    const SfxItemSet *pArgs = rReq.GetArgs();
    BOOL bSelect = FALSE;
    USHORT nCount = 1;
    if(pArgs)
    {
        const SfxPoolItem *pItem;
        if(SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_MOVE_COUNT, TRUE, &pItem))
            nCount = ((const SfxInt16Item *)pItem)->GetValue();
        if(SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_MOVE_SELECTION, TRUE, &pItem))
            bSelect = ((const SfxBoolItem *)pItem)->GetValue();
    }
    switch(rReq.GetSlot())
    {
        case FN_CHAR_LEFT_SEL:  rReq.SetSlot( FN_CHAR_LEFT );  bSelect = TRUE; break;
        case FN_CHAR_RIGHT_SEL: rReq.SetSlot( FN_CHAR_RIGHT ); bSelect = TRUE; break;
        case FN_LINE_UP_SEL:    rReq.SetSlot( FN_LINE_UP );    bSelect = TRUE; break;
        case FN_LINE_DOWN_SEL:  rReq.SetSlot( FN_LINE_DOWN );  bSelect = TRUE; break;
    }

    uno::Reference< frame::XDispatchRecorder > xRecorder =
            GetView().GetViewFrame()->GetBindings().GetRecorder();
    if ( xRecorder.is() )
    {
        rReq.AppendItem( SfxInt16Item(FN_PARAM_MOVE_COUNT, nCount) );
        rReq.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, bSelect) );
    }
    USHORT nSlot = rReq.GetSlot();
    rReq.Done();
    // Get EditWin before calling the move functions (shell change may occur!)
    SwEditWin& rTmpEditWin = GetView().GetEditWin();
    for( USHORT i = 0; i < nCount; i++ )
    {
        switch(nSlot)
        {
        case FN_CHAR_LEFT:  rSh.Left( CRSR_SKIP_CELLS,  bSelect, 1, FALSE, TRUE ); break;
        case FN_CHAR_RIGHT: rSh.Right( CRSR_SKIP_CELLS, bSelect, 1, FALSE, TRUE ); break;
        case FN_LINE_UP:    rSh.Up   ( bSelect, 1 ); break;
        case FN_LINE_DOWN:  rSh.Down ( bSelect, 1 ); break;
        default: OSL_FAIL("wrong Dispatcher"); return;
        }
    }

    //#i42732# - notify the edit window that from now on we do not use the input language
    rTmpEditWin.SetUseInputLanguage( sal_False );
}

void SwTextShell::ExecMove(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    SwEditWin& rTmpEditWin = GetView().GetEditWin();
    rTmpEditWin.FlushInBuffer();

    USHORT nSlot = rReq.GetSlot();
    BOOL bRet = FALSE;
    switch ( nSlot )
    {
        case FN_START_OF_LINE_SEL:
        case FN_START_OF_LINE:      bRet = rSh.LeftMargin ( FN_START_OF_LINE_SEL == nSlot, FALSE );
        break;

        case FN_END_OF_LINE_SEL:
        case FN_END_OF_LINE:        bRet = rSh.RightMargin( FN_END_OF_LINE_SEL == nSlot, FALSE );
        break;

        case FN_START_OF_DOCUMENT_SEL:
        case FN_START_OF_DOCUMENT:  bRet = rSh.SttDoc      ( FN_START_OF_DOCUMENT_SEL == nSlot);
        break;

        case FN_END_OF_DOCUMENT_SEL:
        case FN_END_OF_DOCUMENT:    bRet = rSh.EndDoc( FN_END_OF_DOCUMENT_SEL == nSlot );
        break;

        case FN_SELECT_WORD:            bRet = rSh.SelNearestWrd(); break;

        case SID_SELECTALL:             bRet = 0 != rSh.SelAll();   break;
        default: OSL_FAIL("wrong dispatcher"); return;
    }

    if ( bRet )
        rReq.Done();
    else
        rReq.Ignore();

    //#i42732# - notify the edit window that from now on we do not use the input language
    rTmpEditWin.SetUseInputLanguage( sal_False );
}

void SwTextShell::ExecMovePage(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    GetView().GetEditWin().FlushInBuffer();

    USHORT nSlot = rReq.GetSlot();
    switch( nSlot )
    {
        case FN_START_OF_NEXT_PAGE_SEL :
        case FN_START_OF_NEXT_PAGE: rSh.SttNxtPg( FN_START_OF_NEXT_PAGE_SEL == nSlot ); break;

        case FN_END_OF_NEXT_PAGE_SEL:
        case FN_END_OF_NEXT_PAGE:   rSh.EndNxtPg( FN_END_OF_NEXT_PAGE_SEL == nSlot ); break;

        case FN_START_OF_PREV_PAGE_SEL:
        case FN_START_OF_PREV_PAGE: rSh.SttPrvPg( FN_START_OF_PREV_PAGE_SEL == nSlot ); break;

        case FN_END_OF_PREV_PAGE_SEL:
        case FN_END_OF_PREV_PAGE:   rSh.EndPrvPg( FN_END_OF_PREV_PAGE_SEL == nSlot ); break;

        case FN_START_OF_PAGE_SEL:
        case FN_START_OF_PAGE:      rSh.SttPg   ( FN_START_OF_PAGE_SEL == nSlot ); break;

        case FN_END_OF_PAGE_SEL:
        case FN_END_OF_PAGE:        rSh.EndPg   ( FN_END_OF_PAGE_SEL == nSlot ); break;
        default: OSL_FAIL("wrong dispatcher"); return;
    }
    rReq.Done();
}


void SwTextShell::ExecMoveCol(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    switch ( rReq.GetSlot() )
    {
        case FN_START_OF_COLUMN:      rSh.StartOfColumn    ( FALSE ); break;
        case FN_END_OF_COLUMN:        rSh.EndOfColumn      ( FALSE ); break;
        case FN_START_OF_NEXT_COLUMN: rSh.StartOfNextColumn( FALSE ) ; break;
        case FN_END_OF_NEXT_COLUMN:   rSh.EndOfNextColumn  ( FALSE ); break;
        case FN_START_OF_PREV_COLUMN: rSh.StartOfPrevColumn( FALSE ); break;
        case FN_END_OF_PREV_COLUMN:   rSh.EndOfPrevColumn  ( FALSE ); break;
        default: OSL_FAIL("wrong dispatcher"); return;
    }
    rReq.Done();
}

void SwTextShell::ExecMoveLingu(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    GetView().GetEditWin().FlushInBuffer();

    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case FN_NEXT_WORD_SEL:
        case FN_NEXT_WORD:      rSh.NxtWrd( FN_NEXT_WORD_SEL == nSlot );
        break;

        case FN_START_OF_PARA_SEL:
        case FN_START_OF_PARA:  rSh.SttPara( FN_START_OF_PARA_SEL == nSlot );
        break;

        case FN_END_OF_PARA_SEL:
        case FN_END_OF_PARA:    rSh.EndPara( FN_END_OF_PARA_SEL == nSlot );
        break;

        case FN_PREV_WORD_SEL:
        case FN_PREV_WORD:      rSh.PrvWrd( FN_PREV_WORD_SEL == nSlot );
        break;

        case FN_NEXT_SENT_SEL:
        case FN_NEXT_SENT:      rSh.FwdSentence( FN_NEXT_SENT_SEL == nSlot );
        break;

        case FN_PREV_SENT_SEL:
        case FN_PREV_SENT:      rSh.BwdSentence( FN_PREV_SENT_SEL == nSlot );
        break;

        case FN_NEXT_PARA:      rSh.FwdPara    ( FALSE );
        break;

        case FN_PREV_PARA:      rSh.BwdPara    ( FALSE );
        break;
        default: OSL_FAIL("wrong dispatcher"); return;
    }
    rReq.Done();
}

void SwTextShell::ExecMoveMisc(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    USHORT nSlot = rReq.GetSlot();
    BOOL bSetRetVal = TRUE, bRet = TRUE;
    switch ( nSlot )
    {
        case SID_FM_TOGGLECONTROLFOCUS:
            {
                const SwDoc* pDoc = rSh.GetDoc();
                const SwDocShell* pDocShell = pDoc ? pDoc->GetDocShell() : NULL;
                const SwView* pView = pDocShell ? pDocShell->GetView() : NULL;
                const FmFormShell* pFormShell = pView ? pView->GetFormShell() : NULL;
                SdrView* pDrawView = pView ? pView->GetDrawView() : NULL;
                Window* pWindow = pView ? pView->GetWrtShell().GetWin() : NULL;

                OSL_ENSURE( pFormShell && pDrawView && pWindow, "SwXTextView::ExecMoveMisc: no chance!" );
                if ( !pFormShell || !pDrawView || !pWindow )
                    break;

                ::std::auto_ptr< ::svx::ISdrObjectFilter > pFilter( pFormShell->CreateFocusableControlFilter(
                    *pDrawView, *pWindow ) );
                if ( !pFilter.get() )
                    break;

                const SdrObject* pNearestControl = rSh.GetBestObject( TRUE, GOTOOBJ_DRAW_CONTROL, FALSE, pFilter.get() );
                if ( !pNearestControl )
                    break;

                const SdrUnoObj* pUnoObject = dynamic_cast< const SdrUnoObj* >( pNearestControl );
                OSL_ENSURE( pUnoObject, "SwTextShell::ExecMoveMisc: GetBestObject returned nonsense!" );
                if ( !pUnoObject )
                    break;

                pFormShell->ToggleControlFocus( *pUnoObject, *pDrawView, *pWindow );
            }
            break;
        case FN_CNTNT_TO_NEXT_FRAME:
            bRet = rSh.GotoObj(TRUE, GOTOOBJ_GOTO_ANY);
            if(bRet)
            {
                rSh.HideCrsr();
                rSh.EnterSelFrmMode();
            }
        break;
        case FN_NEXT_FOOTNOTE:
            rSh.MoveCrsr();
            bRet = rSh.GotoNextFtnAnchor();
            break;
        case FN_PREV_FOOTNOTE:
            rSh.MoveCrsr();
            bRet = rSh.GotoPrevFtnAnchor();
            break;
        case FN_TO_HEADER:
            rSh.MoveCrsr();
            if ( FRMTYPE_HEADER & rSh.GetFrmType(0,FALSE) )
                rSh.SttPg();
            else
                rSh.GotoHeaderTxt();
            bSetRetVal = FALSE;
            break;
        case FN_TO_FOOTER:
            rSh.MoveCrsr();
            if ( FRMTYPE_FOOTER & rSh.GetFrmType(0,FALSE) )
                rSh.EndPg();
            else
                rSh.GotoFooterTxt();
            bSetRetVal = FALSE;
            break;
        case FN_FOOTNOTE_TO_ANCHOR:
            rSh.MoveCrsr();
            if ( FRMTYPE_FOOTNOTE & rSh.GetFrmType(0,FALSE) )
                rSh.GotoFtnAnchor();
            else
                rSh.GotoFtnTxt();
            bSetRetVal = FALSE;
            break;
        case FN_TO_FOOTNOTE_AREA :
            rSh.GotoFtnTxt();
        break;
        case FN_PREV_TABLE:
            bRet = rSh.MoveTable( fnTablePrev, fnTableStart);
            break;
        case FN_NEXT_TABLE:
            bRet = rSh.MoveTable(fnTableNext, fnTableStart);
            break;
        case FN_GOTO_NEXT_REGION :
            bRet = rSh.MoveRegion(fnRegionNext, fnRegionStart);
        break;
        case FN_GOTO_PREV_REGION :
            bRet = rSh.MoveRegion(fnRegionPrev, fnRegionStart);
        break;

        case FN_NEXT_TOXMARK:
            bRet = rSh.GotoNxtPrvTOXMark( TRUE );
            break;
        case FN_PREV_TOXMARK:
            bRet = rSh.GotoNxtPrvTOXMark( FALSE );
            break;
        case FN_NEXT_TBLFML:
            bRet = rSh.GotoNxtPrvTblFormula( TRUE, FALSE );
            break;
        case FN_PREV_TBLFML:
            bRet = rSh.GotoNxtPrvTblFormula( FALSE, FALSE );
            break;
        case FN_NEXT_TBLFML_ERR:
            bRet = rSh.GotoNxtPrvTblFormula( TRUE, TRUE );
            break;
        case FN_PREV_TBLFML_ERR:
            bRet = rSh.GotoNxtPrvTblFormula( FALSE, TRUE );
            break;

        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }

    if( bSetRetVal )
        rReq.SetReturnValue(SfxBoolItem( nSlot, bRet ));
    rReq.Done();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
