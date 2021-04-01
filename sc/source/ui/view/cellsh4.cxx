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

#include <sfx2/request.hxx>
#include <osl/diagnose.h>

#include <cellsh.hxx>
#include <tabvwsh.hxx>
#include <global.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <inputwin.hxx>
#include <document.hxx>
#include <sc.hrc>

void ScCellShell::ExecuteCursor( SfxRequest& rReq )
{
    ScViewData& rData = GetViewData();
    ScTabViewShell* pTabViewShell = rData.GetViewShell();
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    sal_uInt16          nSlotId  = rReq.GetSlot();
    SCCOLROW            nRepeat = 1;
    bool                bSel = false;
    bool                bKeep = false;

    if ( pReqArgs != nullptr )
    {
        const   SfxPoolItem* pItem;
        if (pReqArgs->HasItem(FN_PARAM_1, &pItem))
            nRepeat = static_cast<SCCOLROW>(static_cast<const SfxInt16Item*>(pItem)->GetValue());
        if (pReqArgs->HasItem(FN_PARAM_2, &pItem))
            bSel = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    }
    else
    {
        //  evaluate locked selection mode

        sal_uInt16 nLocked = pTabViewShell->GetLockedModifiers();
        if ( nLocked & KEY_SHIFT )
            bSel = true;                // EXT
        else if ( nLocked & KEY_MOD1 )
        {
            // ADD mode: keep the selection, start a new block when marking with shift again
            bKeep = true;
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

    SCCOLROW nRTLSign = 1;
    if ( rData.GetDocument().IsLayoutRTL( rData.GetTabNo() ) )
    {
        //! evaluate cursor movement option?
        nRTLSign = -1;
    }

    // once extra, so that the cursor will not be painted too often with ExecuteInputDirect:
    pTabViewShell->HideAllCursors();

    // #i123629#
    if( pTabViewShell->GetCurObjectSelectionType() == OST_Editing )
        pTabViewShell->SetForceFocusOnCurCell(true);
    else
        pTabViewShell->SetForceFocusOnCurCell(false);

    // If ScrollLock key is active, cell cursor stays on the current cell while
    // scrolling the grid.
    bool bScrollLock = false;
    KeyIndicatorState eState = pFrameWin->GetIndicatorState();
    if (eState & KeyIndicatorState::SCROLLLOCK)
        bScrollLock = true;

    //OS: once for all should do, however!
    pTabViewShell->ExecuteInputDirect();
    switch ( nSlotId )
    {
        case SID_CURSORDOWN:
            if (bScrollLock)
                pTabViewShell->ScrollY( nRepeat, SC_SPLIT_BOTTOM );
            else
                pTabViewShell->MoveCursorRel( 0, nRepeat, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORBLKDOWN:
            pTabViewShell->MoveCursorArea( 0, nRepeat, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORUP:
            if (bScrollLock)
                pTabViewShell->ScrollY( -nRepeat, SC_SPLIT_BOTTOM);
            else
                pTabViewShell->MoveCursorRel( 0, -nRepeat, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORBLKUP:
            pTabViewShell->MoveCursorArea( 0, -nRepeat, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORLEFT:
            if (bScrollLock)
                pTabViewShell->ScrollX( static_cast<SCCOL>(-nRepeat * nRTLSign), SC_SPLIT_LEFT);
            else
                pTabViewShell->MoveCursorRel( static_cast<SCCOL>(-nRepeat * nRTLSign), 0, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORBLKLEFT:
            pTabViewShell->MoveCursorArea( static_cast<SCCOL>(-nRepeat * nRTLSign), 0, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORRIGHT:
            if (bScrollLock)
                pTabViewShell->ScrollX( static_cast<SCCOL>(nRepeat * nRTLSign), SC_SPLIT_LEFT);
            else
                pTabViewShell->MoveCursorRel( static_cast<SCCOL>(nRepeat * nRTLSign), 0, SC_FOLLOW_LINE, bSel, bKeep );
            break;

        case SID_CURSORBLKRIGHT:
            pTabViewShell->MoveCursorArea( static_cast<SCCOL>(nRepeat * nRTLSign), 0, SC_FOLLOW_JUMP, bSel, bKeep );
            break;

        case SID_CURSORPAGEDOWN:
            if (bScrollLock)
            {
                SCCOL nPageX;
                SCROW nPageY;
                pTabViewShell->GetPageMoveEndPosition( 0, nRepeat, nPageX, nPageY);
                pTabViewShell->ScrollY( nPageY, SC_SPLIT_BOTTOM);
            }
            else
                pTabViewShell->MoveCursorPage( 0, nRepeat, SC_FOLLOW_FIX, bSel, bKeep );
            break;

        case SID_CURSORPAGEUP:
            if (bScrollLock)
            {
                SCCOL nPageX;
                SCROW nPageY;
                pTabViewShell->GetPageMoveEndPosition( 0, nRepeat, nPageX, nPageY);
                pTabViewShell->ScrollY( -nPageY, SC_SPLIT_BOTTOM);
            }
            else
                pTabViewShell->MoveCursorPage( 0, -nRepeat, SC_FOLLOW_FIX, bSel, bKeep );
            break;

        case SID_CURSORPAGERIGHT_: //XXX !!!
            if (bScrollLock)
            {
                SCCOL nPageX;
                SCROW nPageY;
                pTabViewShell->GetPageMoveEndPosition( static_cast<SCCOL>(nRepeat), 0, nPageX, nPageY);
                pTabViewShell->ScrollX( nPageX, SC_SPLIT_LEFT);
            }
            else
                pTabViewShell->MoveCursorPage( static_cast<SCCOL>(nRepeat), 0, SC_FOLLOW_FIX, bSel, bKeep );
            break;

        case SID_CURSORPAGELEFT_: //XXX !!!
            if (bScrollLock)
            {
                SCCOL nPageX;
                SCROW nPageY;
                pTabViewShell->GetPageMoveEndPosition( static_cast<SCCOL>(nRepeat), 0, nPageX, nPageY);
                pTabViewShell->ScrollX( -nPageX, SC_SPLIT_LEFT);
            }
            else
                pTabViewShell->MoveCursorPage( static_cast<SCCOL>(-nRepeat), 0, SC_FOLLOW_FIX, bSel, bKeep );
            break;

        default:
            OSL_FAIL("Unknown message in ViewShell (Cursor)");
            return;
    }

    pTabViewShell->ShowAllCursors();

    rReq.AppendItem( SfxInt16Item(FN_PARAM_1, static_cast<sal_Int16>(nRepeat)) );
    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, bSel) );
    rReq.Done();
}

void ScCellShell::GetStateCursor( SAL_UNUSED_PARAMETER SfxItemSet& /* rSet */ )
{
}

void ScCellShell::ExecuteCursorSel( SfxRequest& rReq )
{
    sal_uInt16 nSlotId  = rReq.GetSlot();
    ScTabViewShell* pViewShell = GetViewData().GetViewShell();
    ScInputHandler* pInputHdl = pViewShell->GetInputHandler();
    pViewShell->HideAllCursors();
    if (pInputHdl && pInputHdl->IsInputMode())
    {
        // the current cell is in edit mode.  Commit the text before moving on.
        pViewShell->ExecuteInputDirect();
    }

    SCCOLROW nRepeat = 1;
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    // get repetition
    if ( pReqArgs != nullptr )
    {
        const SfxPoolItem* pItem;
        if (pReqArgs->HasItem(FN_PARAM_1, &pItem))
            nRepeat = static_cast<SCCOLROW>(static_cast<const SfxInt16Item*>(pItem)->GetValue());
    }

    SCROW nMovY = nRepeat;
    // Horizontal direction depends on whether or not the UI language is RTL.
    SCCOL nMovX = nRepeat;
    if (GetViewData().GetDocument().IsLayoutRTL(GetViewData().GetTabNo()))
    {
        // mirror horizontal movement for right-to-left mode.
        nMovX = -nRepeat;
    }

    switch (nSlotId)
    {
        case SID_CURSORDOWN_SEL:
            pViewShell->ExpandBlock(0, nMovY, SC_FOLLOW_LINE);
        break;
        case SID_CURSORUP_SEL:
            pViewShell->ExpandBlock(0, -nMovY, SC_FOLLOW_LINE);
        break;
        case SID_CURSORRIGHT_SEL:
            pViewShell->ExpandBlock(nMovX, 0, SC_FOLLOW_LINE);
        break;
        case SID_CURSORLEFT_SEL:
            pViewShell->ExpandBlock(-nMovX, 0, SC_FOLLOW_LINE);
        break;
        case SID_CURSORPAGEUP_SEL:
            pViewShell->ExpandBlockPage(0, -nMovY);
        break;
        case SID_CURSORPAGEDOWN_SEL:
            pViewShell->ExpandBlockPage(0, nMovY);
        break;
        case SID_CURSORPAGERIGHT_SEL:
            pViewShell->ExpandBlockPage(nMovX, 0);
        break;
        case SID_CURSORPAGELEFT_SEL:
            pViewShell->ExpandBlockPage(-nMovX, 0);
        break;
        case SID_CURSORBLKDOWN_SEL:
            pViewShell->ExpandBlockArea(0, nMovY);
        break;
        case SID_CURSORBLKUP_SEL:
            pViewShell->ExpandBlockArea(0, -nMovY);
        break;
        case SID_CURSORBLKRIGHT_SEL:
            pViewShell->ExpandBlockArea(nMovX , 0);
        break;
        case SID_CURSORBLKLEFT_SEL:
            pViewShell->ExpandBlockArea(-nMovX, 0);
        break;
        default:
            ;
    }
    pViewShell->ShowAllCursors();

    rReq.AppendItem( SfxInt16Item(FN_PARAM_1,static_cast<sal_Int16>(nRepeat)) );
    rReq.Done();
}

void ScCellShell::ExecuteMove( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    sal_uInt16 nSlotId  = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

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

        //  cursor movements in range do not originate from Basic,
        //  because the ScSbxRange-object changes the marking at input

        case SID_NEXT_UNPROTECT:
            pTabViewShell->FindNextUnprot( false, !rReq.IsAPI() );
            break;

        case SID_PREV_UNPROTECT:
            pTabViewShell->FindNextUnprot( true, !rReq.IsAPI() );
            break;

        case SID_CURSORENTERUP:
            if (rReq.IsAPI())
                pTabViewShell->MoveCursorRel( 0, -1, SC_FOLLOW_LINE, false );
            else
                pTabViewShell->MoveCursorEnter( true );
            break;

        case SID_CURSORENTERDOWN:
            if (rReq.IsAPI())
                pTabViewShell->MoveCursorRel( 0, 1, SC_FOLLOW_LINE, false );
            else
                pTabViewShell->MoveCursorEnter( false );
            break;

        case SID_SELECT_COL:
            {
                const SfxPoolItem* pColItem;
                const SfxPoolItem* pModifierItem;
                if ( pReqArgs && pReqArgs->HasItem( FN_PARAM_1, &pColItem ) &&
                     pReqArgs->HasItem( FN_PARAM_2, &pModifierItem ) )
                {
                    SCCOL nCol = static_cast<SCCOL>(static_cast<const SfxInt32Item*>(pColItem)->GetValue());
                    sal_Int16 nModifier = static_cast<const SfxInt16Item*>(pModifierItem)->GetValue();

                    pTabViewShell->MarkColumns( nCol, nModifier );
                }
                else
                    pTabViewShell->MarkColumns();
            }
            break;

        case SID_SELECT_ROW:
            {
                const SfxPoolItem* pRowItem;
                const SfxPoolItem* pModifierItem;
                if ( pReqArgs && pReqArgs->HasItem( FN_PARAM_1, &pRowItem ) &&
                     pReqArgs->HasItem( FN_PARAM_2, &pModifierItem ) )
                {
                    SCROW nRow = static_cast<SCROW>(static_cast<const SfxInt32Item*>(pRowItem)->GetValue());
                    sal_Int16 nModifier = static_cast<const SfxInt16Item*>(pModifierItem)->GetValue();

                    pTabViewShell->MarkRows( nRow, nModifier );
                }
                else
                    pTabViewShell->MarkRows();
            }
            break;

        case SID_SELECT_NONE:
            pTabViewShell->Unmark();
            break;

        case SID_ALIGNCURSOR:
            pTabViewShell->AlignToCursor( GetViewData().GetCurX(), GetViewData().GetCurY(), SC_FOLLOW_JUMP );
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
            pTabViewShell->MoveCursorScreen( 0, -1, SC_FOLLOW_LINE, false );
            break;

        case SID_CURSORENDOFSCREEN:
            pTabViewShell->MoveCursorScreen( 0, 1, SC_FOLLOW_LINE, false );
            break;

        default:
            OSL_FAIL("Unknown message in ViewShell (Cursor)");
            return;
    }

    rReq.Done();
}

void ScCellShell::ExecutePageSel( SfxRequest& rReq )
{
    sal_uInt16              nSlotId  = rReq.GetSlot();
    switch ( nSlotId )
    {
        case SID_CURSORHOME_SEL:        rReq.SetSlot( SID_CURSORHOME );  break;
        case SID_CURSOREND_SEL:         rReq.SetSlot( SID_CURSOREND );  break;
        case SID_CURSORTOPOFFILE_SEL:   rReq.SetSlot( SID_CURSORTOPOFFILE );  break;
        case SID_CURSORENDOFFILE_SEL:   rReq.SetSlot( SID_CURSORENDOFFILE );  break;
        default:
            OSL_FAIL("Unknown message in ViewShell (ExecutePageSel)");
            return;
    }
    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, true) );
    ExecuteSlot( rReq, GetInterface() );
}

void ScCellShell::ExecutePage( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    sal_uInt16              nSlotId  = rReq.GetSlot();
    bool                bSel = false;
    bool                bKeep = false;

    if ( pReqArgs != nullptr )
    {
        const   SfxPoolItem* pItem;
        if (pReqArgs->HasItem(FN_PARAM_2, &pItem))
            bSel = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    }
    else
    {
        //  evaluate locked selection mode

        sal_uInt16 nLocked = pTabViewShell->GetLockedModifiers();
        if ( nLocked & KEY_SHIFT )
            bSel = true;                // EXT
        else if ( nLocked & KEY_MOD1 )
        {
            // ADD mode: keep the selection, start a new block when marking with shift again
            bKeep = true;
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
            OSL_FAIL("Unknown message in ViewShell (ExecutePage)");
            return;
    }

    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, bSel) );
    rReq.Done();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
