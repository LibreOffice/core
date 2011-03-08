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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ("", off)
#endif

// INCLUDE ---------------------------------------------------------------

#include <sfx2/request.hxx>

#include "cellsh.hxx"
#include "tabvwsh.hxx"
#include "global.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "inputwin.hxx"
#include "document.hxx"
#include "sc.hrc"

#include "vcl/svapp.hxx"

//------------------------------------------------------------------

#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)


void ScCellShell::ExecuteCursor( SfxRequest& rReq )
{
    ScViewData* pData = GetViewData();
    ScTabViewShell* pTabViewShell   = pData->GetViewShell();
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    USHORT              nSlotId  = rReq.GetSlot();
    SCsCOLROW           nRepeat = 1;
    BOOL                bSel = FALSE;
    BOOL                bKeep = FALSE;

    if ( pReqArgs != NULL )
    {
        const   SfxPoolItem* pItem;
        if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
            nRepeat = static_cast<SCsCOLROW>(((const SfxInt16Item*)pItem)->GetValue());
        if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
            bSel = ((const SfxBoolItem*)pItem)->GetValue();
    }
    else
    {
        //  evaluate locked selection mode

        USHORT nLocked = pTabViewShell->GetLockedModifiers();
        if ( nLocked & KEY_SHIFT )
            bSel = TRUE;                // EXT
        else if ( nLocked & KEY_MOD1 )
        {
            // ADD mode: keep the selection, start a new block when marking with shift again
            bKeep = TRUE;
        }
    }

    if (bSel)
    {
        switch (nSlotId)
        {
            case SID_CURSORDOWN:
                rReq.SetSlot(SID_CURSORDOWN_SEL);
            break;
            case SID_CURSORUP:
                rReq.SetSlot(SID_CURSORUP_SEL);
            break;
            case SID_CURSORRIGHT:
                rReq.SetSlot(SID_CURSORRIGHT_SEL);
            break;
            case SID_CURSORLEFT:
                rReq.SetSlot(SID_CURSORLEFT_SEL);
            break;
            case SID_CURSORPAGEDOWN:
                rReq.SetSlot(SID_CURSORPAGEDOWN_SEL);
            break;
            case SID_CURSORPAGEUP:
                rReq.SetSlot(SID_CURSORPAGEUP_SEL);
            break;
            case SID_CURSORPAGERIGHT:
                rReq.SetSlot(SID_CURSORPAGERIGHT_SEL);
            break;
            case SID_CURSORPAGELEFT:
                rReq.SetSlot(SID_CURSORPAGELEFT_SEL);
            break;
            case SID_CURSORBLKDOWN:
                rReq.SetSlot(SID_CURSORBLKDOWN_SEL);
            break;
            case SID_CURSORBLKUP:
                rReq.SetSlot(SID_CURSORBLKUP_SEL);
            break;
            case SID_CURSORBLKRIGHT:
                rReq.SetSlot(SID_CURSORBLKRIGHT_SEL);
            break;
            case SID_CURSORBLKLEFT:
                rReq.SetSlot(SID_CURSORBLKLEFT_SEL);
            break;
            default:
                ;
        }
        ExecuteCursorSel(rReq);
        return;
    }

    SCsCOLROW nRTLSign = 1;
    if ( pData->GetDocument()->IsLayoutRTL( pData->GetTabNo() ) )
    {
        //! evaluate cursor movement option?
        nRTLSign = -1;
    }

    // einmal extra, damit der Cursor bei ExecuteInputDirect nicht zuoft gemalt wird:
    pTabViewShell->HideAllCursors();

    //OS: einmal fuer alle wird doch reichen!
    pTabViewShell->ExecuteInputDirect();
    switch ( nSlotId )
    {
        case SID_CURSORDOWN:
            pTabViewShell->MoveCursorRel(   0,  nRepeat, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORBLKDOWN:
            pTabViewShell->MoveCursorArea( 0, nRepeat, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORUP:
            pTabViewShell->MoveCursorRel(   0,  -nRepeat, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORBLKUP:
            pTabViewShell->MoveCursorArea( 0, -nRepeat, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORLEFT:
            pTabViewShell->MoveCursorRel( static_cast<SCsCOL>(-nRepeat * nRTLSign), 0, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORBLKLEFT:
            pTabViewShell->MoveCursorArea( static_cast<SCsCOL>(-nRepeat * nRTLSign), 0, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORRIGHT:
            pTabViewShell->MoveCursorRel(   static_cast<SCsCOL>(nRepeat * nRTLSign), 0, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORBLKRIGHT:
            pTabViewShell->MoveCursorArea( static_cast<SCsCOL>(nRepeat * nRTLSign), 0, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORPAGEDOWN:
            pTabViewShell->MoveCursorPage(  0, nRepeat, SC_FOLLOW_FIX, bSel, bKeep );
            break;

        case SID_CURSORPAGEUP:
            pTabViewShell->MoveCursorPage(  0, -nRepeat, SC_FOLLOW_FIX, bSel, bKeep );
            break;

        case SID_CURSORPAGERIGHT_: //XXX !!!
            pTabViewShell->MoveCursorPage( static_cast<SCsCOL>(nRepeat), 0, SC_FOLLOW_FIX, bSel, bKeep );
            break;

        case SID_CURSORPAGELEFT_: //XXX !!!
            pTabViewShell->MoveCursorPage( static_cast<SCsCOL>(-nRepeat), 0, SC_FOLLOW_FIX, bSel, bKeep );
            break;

        default:
            OSL_FAIL("Unbekannte Message bei ViewShell (Cursor)");
            return;
    }

    pTabViewShell->ShowAllCursors();

    rReq.AppendItem( SfxInt16Item(FN_PARAM_1, static_cast<sal_Int16>(nRepeat)) );
    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, bSel) );
    rReq.Done();
}

void ScCellShell::GetStateCursor( SfxItemSet& /* rSet */ )
{
}

void ScCellShell::ExecuteCursorSel( SfxRequest& rReq )
{
    sal_uInt16 nSlotId  = rReq.GetSlot();
    ScTabViewShell* pViewShell = GetViewData()->GetViewShell();
    ScInputHandler* pInputHdl = pViewShell->GetInputHandler();
    pViewShell->HideAllCursors();
    if (pInputHdl && pInputHdl->IsInputMode())
    {
        // the current cell is in edit mode.  Commit the text before moving on.
        pViewShell->ExecuteInputDirect();
    }

    // Horizontal direction depends on whether or not the UI language is RTL.
    SCsCOL nMovX = 1;
    if (GetViewData()->GetDocument()->IsLayoutRTL(GetViewData()->GetTabNo()))
        // mirror horizontal movement for right-to-left mode.
        nMovX = -1;

    switch (nSlotId)
    {
        case SID_CURSORDOWN_SEL:
            pViewShell->ExpandBlock(0, 1, SC_FOLLOW_LINE);
        break;
        case SID_CURSORUP_SEL:
            pViewShell->ExpandBlock(0, -1, SC_FOLLOW_LINE);
        break;
        case SID_CURSORRIGHT_SEL:
            pViewShell->ExpandBlock(nMovX, 0, SC_FOLLOW_LINE);
        break;
        case SID_CURSORLEFT_SEL:
            pViewShell->ExpandBlock(-nMovX, 0, SC_FOLLOW_LINE);
        break;
        case SID_CURSORPAGEUP_SEL:
            pViewShell->ExpandBlockPage(0, -1);
        break;
        case SID_CURSORPAGEDOWN_SEL:
            pViewShell->ExpandBlockPage(0, 1);
        break;
        case SID_CURSORPAGERIGHT_SEL:
            pViewShell->ExpandBlockPage(nMovX, 0);
        break;
        case SID_CURSORPAGELEFT_SEL:
            pViewShell->ExpandBlockPage(-nMovX, 0);
        break;
        case SID_CURSORBLKDOWN_SEL:
            pViewShell->ExpandBlockArea(0, 1);
        break;
        case SID_CURSORBLKUP_SEL:
            pViewShell->ExpandBlockArea(0, -1);
        break;
        case SID_CURSORBLKRIGHT_SEL:
            pViewShell->ExpandBlockArea(nMovX, 0);
        break;
        case SID_CURSORBLKLEFT_SEL:
            pViewShell->ExpandBlockArea(-nMovX, 0);
        break;
        default:
            ;
    }
    pViewShell->ShowAllCursors();
}

void ScCellShell::ExecuteMove( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    USHORT nSlotId  = rReq.GetSlot();

    if(nSlotId != SID_CURSORTOPOFSCREEN && nSlotId != SID_CURSORENDOFSCREEN)
        pTabViewShell->ExecuteInputDirect();
    switch ( nSlotId )
    {
        case SID_NEXT_TABLE:
        case SID_NEXT_TABLE_SEL:
            pTabViewShell->SelectNextTab( 1, (nSlotId == SID_NEXT_TABLE_SEL) );
            break;

        case SID_PREV_TABLE:
        case SID_PREV_TABLE_SEL:
            pTabViewShell->SelectNextTab( -1, (nSlotId == SID_PREV_TABLE_SEL) );
            break;

        //  Cursorbewegungen in Bloecken gehen nicht von Basic aus,
        //  weil das ScSbxRange-Objekt bei Eingaben die Markierung veraendert

        case SID_NEXT_UNPROTECT:
            pTabViewShell->FindNextUnprot( FALSE, !rReq.IsAPI() );
            break;

        case SID_PREV_UNPROTECT:
            pTabViewShell->FindNextUnprot( TRUE, !rReq.IsAPI() );
            break;

        case SID_CURSORENTERUP:
            if (rReq.IsAPI())
                pTabViewShell->MoveCursorRel( 0, -1, SC_FOLLOW_LINE, FALSE );
            else
                pTabViewShell->MoveCursorEnter( TRUE );
            break;

        case SID_CURSORENTERDOWN:
            if (rReq.IsAPI())
                pTabViewShell->MoveCursorRel( 0, 1, SC_FOLLOW_LINE, FALSE );
            else
                pTabViewShell->MoveCursorEnter( FALSE );
            break;

        case SID_SELECT_COL:
            pTabViewShell->MarkColumns();
            break;

        case SID_SELECT_ROW:
            pTabViewShell->MarkRows();
            break;

        case SID_SELECT_NONE:
            pTabViewShell->Unmark();
            break;

        case SID_ALIGNCURSOR:
            pTabViewShell->AlignToCursor( GetViewData()->GetCurX(), GetViewData()->GetCurY(), SC_FOLLOW_JUMP );
            break;

        case SID_MARKDATAAREA:
            pTabViewShell->MarkDataArea();
            break;

        case SID_MARKARRAYFORMULA:
            pTabViewShell->MarkMatrixFormula();
            break;

        case SID_SETINPUTMODE:
            SC_MOD()->SetInputMode( SC_INPUT_TABLE );
            break;

        case SID_FOCUS_INPUTLINE:
            {
                ScInputHandler* pHdl = SC_MOD()->GetInputHdl( pTabViewShell );
                if (pHdl)
                {
                    ScInputWindow* pWin = pHdl->GetInputWindow();
                    if (pWin)
                        pWin->SwitchToTextWin();
                }
            }
            break;

        case SID_CURSORTOPOFSCREEN:
            pTabViewShell->MoveCursorScreen( 0, -1, SC_FOLLOW_LINE, FALSE );
            break;

        case SID_CURSORENDOFSCREEN:
            pTabViewShell->MoveCursorScreen( 0, 1, SC_FOLLOW_LINE, FALSE );
            break;

        default:
            OSL_FAIL("Unbekannte Message bei ViewShell (Cursor)");
            return;
    }

    rReq.Done();
}

void ScCellShell::ExecutePageSel( SfxRequest& rReq )
{
    USHORT              nSlotId  = rReq.GetSlot();
    switch ( nSlotId )
    {
        case SID_CURSORHOME_SEL:        rReq.SetSlot( SID_CURSORHOME );  break;
        case SID_CURSOREND_SEL:         rReq.SetSlot( SID_CURSOREND );  break;
        case SID_CURSORTOPOFFILE_SEL:   rReq.SetSlot( SID_CURSORTOPOFFILE );  break;
        case SID_CURSORENDOFFILE_SEL:   rReq.SetSlot( SID_CURSORENDOFFILE );  break;
        default:
            OSL_FAIL("Unbekannte Message bei ViewShell (ExecutePageSel)");
            return;
    }
    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, TRUE) );
    ExecuteSlot( rReq, GetInterface() );
}

void ScCellShell::ExecutePage( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    USHORT              nSlotId  = rReq.GetSlot();
    BOOL                bSel = FALSE;
    BOOL                bKeep = FALSE;

    if ( pReqArgs != NULL )
    {
        const   SfxPoolItem* pItem;
        if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
            bSel = ((const SfxBoolItem*)pItem)->GetValue();
    }
    else
    {
        //  evaluate locked selection mode

        USHORT nLocked = pTabViewShell->GetLockedModifiers();
        if ( nLocked & KEY_SHIFT )
            bSel = TRUE;                // EXT
        else if ( nLocked & KEY_MOD1 )
        {
            // ADD mode: keep the selection, start a new block when marking with shift again
            bKeep = TRUE;
        }
    }

    pTabViewShell->ExecuteInputDirect();
    switch ( nSlotId )
    {
        case SID_CURSORHOME:
            pTabViewShell->MoveCursorEnd( -1, 0, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSOREND:
            pTabViewShell->MoveCursorEnd( 1, 0, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORTOPOFFILE:
            pTabViewShell->MoveCursorEnd( -1, -1, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORENDOFFILE:
            pTabViewShell->MoveCursorEnd( 1, 1, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        default:
            OSL_FAIL("Unbekannte Message bei ViewShell (ExecutePage)");
            return;
    }

    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, bSel) );
    rReq.Done();
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
