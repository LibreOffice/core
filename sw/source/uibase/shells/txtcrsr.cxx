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

#include <memory>

#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <textsh.hxx>
#include <edtwin.hxx>
#include <doc.hxx>
#include <docsh.hxx>

#include <cmdid.h>
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
    bool bSelect = false;
    sal_Int32 nCount = 1;
    if(pArgs)
    {
        const SfxPoolItem *pItem;
        if(SfxItemState::SET == pArgs->GetItemState(FN_PARAM_MOVE_COUNT, true, &pItem))
            nCount = static_cast<const SfxInt32Item *>(pItem)->GetValue();
        if(SfxItemState::SET == pArgs->GetItemState(FN_PARAM_MOVE_SELECTION, true, &pItem))
            bSelect = static_cast<const SfxBoolItem *>(pItem)->GetValue();
    }
    switch(rReq.GetSlot())
    {
        case FN_CHAR_LEFT_SEL:
            rReq.SetSlot( FN_CHAR_LEFT );
            bSelect = true;
            break;
        case FN_CHAR_RIGHT_SEL:
            rReq.SetSlot( FN_CHAR_RIGHT );
            bSelect = true;
            break;
        case FN_LINE_UP_SEL:
            rReq.SetSlot( FN_LINE_UP );
            bSelect = true;
            break;
        case FN_LINE_DOWN_SEL:
            rReq.SetSlot( FN_LINE_DOWN );
            bSelect = true;
            break;
    }

    uno::Reference< frame::XDispatchRecorder > xRecorder =
            GetView().GetViewFrame()->GetBindings().GetRecorder();
    if ( xRecorder.is() )
    {
        rReq.AppendItem( SfxInt32Item(FN_PARAM_MOVE_COUNT, nCount) );
        rReq.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, bSelect) );
    }
    const sal_uInt16 nSlot = rReq.GetSlot();
    rReq.Done();
    // Get EditWin before calling the move functions (shell change may occur!)
    SwEditWin& rTmpEditWin = GetView().GetEditWin();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        switch(nSlot)
        {
            case FN_CHAR_LEFT:
                rSh.Left( CRSR_SKIP_CELLS,  bSelect, 1, false, true );
                break;
            case FN_CHAR_RIGHT:
                rSh.Right( CRSR_SKIP_CELLS, bSelect, 1, false, true );
                break;
            case FN_LINE_UP:
                rSh.Up( bSelect );
                break;
            case FN_LINE_DOWN:
                rSh.Down( bSelect );
                break;
            default:
                OSL_FAIL("wrong Dispatcher");
                return;
        }
    }

    //#i42732# - notify the edit window that from now on we do not use the input language
    rTmpEditWin.SetUseInputLanguage( false );
}

void SwTextShell::ExecMove(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    rSh.addCurrentPosition();
    SwEditWin& rTmpEditWin = GetView().GetEditWin();
    rTmpEditWin.FlushInBuffer();

    bool bRet = false;
    switch ( rReq.GetSlot() )
    {
        case FN_START_OF_LINE_SEL:
            bRet = rSh.LeftMargin( true, false );
            break;
        case FN_START_OF_LINE:
            bRet = rSh.LeftMargin( false, false );
            break;
        case FN_END_OF_LINE_SEL:
            bRet = rSh.RightMargin( true, false );
            break;
        case FN_END_OF_LINE:
            bRet = rSh.RightMargin( false, false );
            break;
        case FN_START_OF_DOCUMENT_SEL:
            bRet = rSh.StartOfSection( true );
            break;
        case FN_START_OF_DOCUMENT:
            bRet = rSh.StartOfSection();
            break;
        case FN_END_OF_DOCUMENT_SEL:
            bRet = rSh.EndOfSection( true );
            break;
        case FN_END_OF_DOCUMENT:
            bRet = rSh.EndOfSection();
            break;
        case FN_SELECT_WORD:
            bRet = rSh.SelNearestWrd();
            break;
        case FN_SELECT_SENTENCE:
            rSh.SelSentence( nullptr );
            bRet = true;
            break;
        case SID_SELECTALL:
            rSh.SelAll();
            bRet = true;
            break;
        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }

    if ( bRet )
        rReq.Done();
    else
        rReq.Ignore();

    //#i42732# - notify the edit window that from now on we do not use the input language
    rTmpEditWin.SetUseInputLanguage( false );
}

void SwTextShell::ExecMovePage(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    rSh.addCurrentPosition();
    GetView().GetEditWin().FlushInBuffer();

    switch( rReq.GetSlot() )
    {
        case FN_START_OF_NEXT_PAGE_SEL :
            rSh.SttNxtPg( true );
            break;
        case FN_START_OF_NEXT_PAGE:
            rSh.SttNxtPg();
            break;
        case FN_END_OF_NEXT_PAGE_SEL:
            rSh.EndNxtPg( true );
            break;
        case FN_END_OF_NEXT_PAGE:
            rSh.EndNxtPg();
            break;
        case FN_START_OF_PREV_PAGE_SEL:
            rSh.SttPrvPg( true );
            break;
        case FN_START_OF_PREV_PAGE:
            rSh.SttPrvPg();
            break;
        case FN_END_OF_PREV_PAGE_SEL:
            rSh.EndPrvPg( true );
            break;
        case FN_END_OF_PREV_PAGE:
            rSh.EndPrvPg();
            break;
        case FN_START_OF_PAGE_SEL:
            rSh.SttPg( true );
            break;
        case FN_START_OF_PAGE:
            rSh.SttPg();
            break;
        case FN_END_OF_PAGE_SEL:
            rSh.EndPg( true );
            break;
        case FN_END_OF_PAGE:
            rSh.EndPg();
            break;
        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }
    rReq.Done();
}

void SwTextShell::ExecMoveCol(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    rSh.addCurrentPosition();
    switch ( rReq.GetSlot() )
    {
        case FN_START_OF_COLUMN:
            rSh.StartOfColumn();
            break;
        case FN_END_OF_COLUMN:
            rSh.EndOfColumn();
            break;
        case FN_START_OF_NEXT_COLUMN:
            rSh.StartOfNextColumn() ;
            break;
        case FN_END_OF_NEXT_COLUMN:
            rSh.EndOfNextColumn();
            break;
        case FN_START_OF_PREV_COLUMN:
            rSh.StartOfPrevColumn();
            break;
        case FN_END_OF_PREV_COLUMN:
            rSh.EndOfPrevColumn();
            break;
        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }
    rReq.Done();
}

void SwTextShell::ExecMoveLingu(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    rSh.addCurrentPosition();
    GetView().GetEditWin().FlushInBuffer();

    switch ( rReq.GetSlot() )
    {
        case FN_NEXT_WORD_SEL:
            rSh.NxtWrd( true );
            break;
        case FN_NEXT_WORD:
            rSh.NxtWrd();
            break;
        case FN_START_OF_PARA_SEL:
            rSh.SttPara( true );
            break;
        case FN_START_OF_PARA:
            rSh.SttPara();
            break;
        case FN_END_OF_PARA_SEL:
            rSh.EndPara( true );
            break;
        case FN_END_OF_PARA:
            rSh.EndPara();
            break;
        case FN_PREV_WORD_SEL:
            rSh.PrvWrd( true );
            break;
        case FN_PREV_WORD:
            rSh.PrvWrd();
            break;
        case FN_NEXT_SENT_SEL:
            rSh.FwdSentence( true );
            break;
        case FN_NEXT_SENT:
            rSh.FwdSentence();
            break;
        case FN_PREV_SENT_SEL:
            rSh.BwdSentence( true );
            break;
        case FN_PREV_SENT:
            rSh.BwdSentence();
            break;
        case FN_NEXT_PARA:
            rSh.FwdPara();
            break;
        case FN_PREV_PARA:
            rSh.BwdPara();
            break;
        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }
    rReq.Done();
}

void SwTextShell::ExecMoveMisc(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    rSh.addCurrentPosition();
    const sal_uInt16 nSlot = rReq.GetSlot();
    bool bSetRetVal = true, bRet = true;
    switch ( nSlot )
    {
        case SID_FM_TOGGLECONTROLFOCUS:
            {
                const SwDoc* pDoc = rSh.GetDoc();
                const SwDocShell* pDocShell = pDoc ? pDoc->GetDocShell() : nullptr;
                const SwView* pView = pDocShell ? pDocShell->GetView() : nullptr;
                const FmFormShell* pFormShell = pView ? pView->GetFormShell() : nullptr;
                SdrView* pDrawView = pView ? pView->GetDrawView() : nullptr;
                vcl::Window* pWindow = pView ? pView->GetWrtShell().GetWin() : nullptr;

                OSL_ENSURE( pFormShell && pDrawView && pWindow, "SwXTextView::ExecMoveMisc: no chance!" );
                if ( !pFormShell || !pDrawView || !pWindow )
                    break;

                std::unique_ptr< svx::ISdrObjectFilter > pFilter( FmFormShell::CreateFocusableControlFilter(
                    *pDrawView, *pWindow ) );
                if (!pFilter)
                    break;

                const SdrObject* pNearestControl = rSh.GetBestObject( true, GotoObjFlags::DrawControl, false, pFilter.get() );
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
            bRet = rSh.GotoObj(true, GotoObjFlags::Any);
            if(bRet)
            {
                rSh.HideCursor();
                rSh.EnterSelFrameMode();
            }
        break;
        case FN_NEXT_FOOTNOTE:
            rSh.MoveCursor();
            bRet = rSh.GotoNextFootnoteAnchor();
            break;
        case FN_PREV_FOOTNOTE:
            rSh.MoveCursor();
            bRet = rSh.GotoPrevFootnoteAnchor();
            break;
        case FN_TO_HEADER:
            rSh.MoveCursor();
            if ( FrameTypeFlags::HEADER & rSh.GetFrameType(nullptr,false) )
                rSh.SttPg();
            else
            {
                bool bMoved = rSh.GotoHeaderText();
                if ( !bMoved )
                    rSh.SttPg();
            }
            bSetRetVal = false;
            break;
        case FN_TO_FOOTER:
            rSh.MoveCursor();
            if ( FrameTypeFlags::FOOTER & rSh.GetFrameType(nullptr,false) )
                rSh.EndPg();
            else
            {
                bool bMoved = rSh.GotoFooterText();
                if ( !bMoved )
                    rSh.EndPg();
            }
            bSetRetVal = false;
            break;
        case FN_FOOTNOTE_TO_ANCHOR:
            rSh.MoveCursor();
            if ( FrameTypeFlags::FOOTNOTE & rSh.GetFrameType(nullptr,false) )
                rSh.GotoFootnoteAnchor();
            else
                rSh.GotoFootnoteText();
            bSetRetVal = false;
            break;
        case FN_TO_FOOTNOTE_AREA :
            rSh.GotoFootnoteText();
            break;
        case FN_PREV_TABLE:
            bRet = rSh.MoveTable( GotoPrevTable, fnTableStart);
            break;
        case FN_NEXT_TABLE:
            bRet = rSh.MoveTable(GotoNextTable, fnTableStart);
            break;
        case FN_GOTO_NEXT_REGION :
            bRet = rSh.MoveRegion(GotoNextRegion, fnRegionStart);
            break;
        case FN_GOTO_PREV_REGION :
            bRet = rSh.MoveRegion(GotoPrevRegion, fnRegionStart);
            break;
        case FN_NEXT_TOXMARK:
            bRet = rSh.GotoNxtPrvTOXMark();
            break;
        case FN_PREV_TOXMARK:
            bRet = rSh.GotoNxtPrvTOXMark( false );
            break;
        case FN_NEXT_TBLFML:
            bRet = rSh.GotoNxtPrvTableFormula();
            break;
        case FN_PREV_TBLFML:
            bRet = rSh.GotoNxtPrvTableFormula( false );
            break;
        case FN_NEXT_TBLFML_ERR:
            bRet = rSh.GotoNxtPrvTableFormula( true, true );
            break;
        case FN_PREV_TBLFML_ERR:
            bRet = rSh.GotoNxtPrvTableFormula( false, true );
            break;
        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }

    if( bSetRetVal )
        rReq.SetReturnValue(SfxBoolItem( nSlot, bRet ));
    rReq.Done();

    bool bInHeader = true;
    if ( rSh.IsInHeaderFooter( &bInHeader ) )
    {
        if ( !bInHeader )
        {
            rSh.SetShowHeaderFooterSeparator( FrameControlType::Footer, true );
            rSh.SetShowHeaderFooterSeparator( FrameControlType::Header, false );
        }
        else
        {
            rSh.SetShowHeaderFooterSeparator( FrameControlType::Header, true );
            rSh.SetShowHeaderFooterSeparator( FrameControlType::Footer, false );
        }

        // Force repaint
        rSh.GetWin()->Invalidate();
    }
    if ( rSh.IsInHeaderFooter() != rSh.IsHeaderFooterEdit() )
        rSh.ToggleHeaderFooterEdit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
