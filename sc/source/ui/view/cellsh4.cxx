/*************************************************************************
 *
 *  $RCSfile: cellsh4.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#ifdef WNT
#pragma optimize ("", off)
#endif

// INCLUDE ---------------------------------------------------------------

#include <sfx2/request.hxx>

#include "cellsh.hxx"
#include "tabvwsh.hxx"
#include "global.hxx"
#include "scmod.hxx"
#include "sc.hrc"


//------------------------------------------------------------------

#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)


void ScCellShell::ExecuteCursor( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    USHORT              nSlotId  = rReq.GetSlot();
    short               nRepeat = 1;
    BOOL                bSel = FALSE;

    if ( pReqArgs != NULL )
    {
        const   SfxPoolItem* pItem;
        if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
            nRepeat = ((const SfxInt16Item*)pItem)->GetValue();
        if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
            bSel = ((const SfxBoolItem*)pItem)->GetValue();
    }

    // einmal extra, damit der Cursor bei ExecuteInputDirect nicht zuoft gemalt wird:
    pTabViewShell->HideAllCursors();

    //OS: einmal fuer alle wird doch reichen!
    pTabViewShell->ExecuteInputDirect();
    switch ( nSlotId )
    {
        case SID_CURSORDOWN:
            pTabViewShell->MoveCursorRel(   0,  nRepeat, SC_FOLLOW_LINE, bSel );
            break;

        case SID_CURSORBLKDOWN:
            pTabViewShell->MoveCursorArea( 0, nRepeat, SC_FOLLOW_JUMP, bSel );
            break;

        case SID_CURSORUP:
            pTabViewShell->MoveCursorRel(   0,  -nRepeat, SC_FOLLOW_LINE, bSel );
            break;

        case SID_CURSORBLKUP:
            pTabViewShell->MoveCursorArea( 0, -nRepeat, SC_FOLLOW_JUMP, bSel );
            break;

        case SID_CURSORLEFT:
            pTabViewShell->MoveCursorRel( -nRepeat, 0, SC_FOLLOW_LINE, bSel );
            break;

        case SID_CURSORBLKLEFT:
            pTabViewShell->MoveCursorArea( -nRepeat, 0, SC_FOLLOW_JUMP, bSel );
            break;

        case SID_CURSORRIGHT:
            pTabViewShell->MoveCursorRel(   nRepeat, 0, SC_FOLLOW_LINE, bSel );
            break;

        case SID_CURSORBLKRIGHT:
            pTabViewShell->MoveCursorArea( nRepeat, 0, SC_FOLLOW_JUMP, bSel );
            break;

        case SID_CURSORPAGEDOWN:
            pTabViewShell->MoveCursorPage(  0, nRepeat, SC_FOLLOW_FIX, bSel );
            break;

        case SID_CURSORPAGEUP:
            pTabViewShell->MoveCursorPage(  0, -nRepeat, SC_FOLLOW_FIX, bSel );
            break;

        case SID_CURSORPAGERIGHT_: //XXX !!!
            pTabViewShell->MoveCursorPage( nRepeat, 0, SC_FOLLOW_FIX, bSel );
            break;

        case SID_CURSORPAGELEFT_: //XXX !!!
            pTabViewShell->MoveCursorPage( -nRepeat, 0, SC_FOLLOW_FIX, bSel );
            break;

        default:
            DBG_ERROR("Unbekannte Message bei ViewShell (Cursor)");
            return;
    }

    pTabViewShell->ShowAllCursors();

    rReq.AppendItem( SfxInt16Item(FN_PARAM_1, nRepeat) );
    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, bSel) );
    rReq.Done();
}

void ScCellShell::GetStateCursor( SfxItemSet& rSet )
{
}

void ScCellShell::ExecuteCursorSel( SfxRequest& rReq )
{
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    USHORT              nSlotId  = rReq.GetSlot();
    short               nRepeat = 1;

    if ( pReqArgs != NULL )
    {
        const   SfxPoolItem* pItem;
        if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
            nRepeat = ((const SfxInt16Item*)pItem)->GetValue();
    }

    switch ( nSlotId )
    {
        case SID_CURSORDOWN_SEL:        rReq.SetSlot( SID_CURSORDOWN );  break;
        case SID_CURSORBLKDOWN_SEL:     rReq.SetSlot( SID_CURSORBLKDOWN );  break;
        case SID_CURSORUP_SEL:          rReq.SetSlot( SID_CURSORUP );  break;
        case SID_CURSORBLKUP_SEL:       rReq.SetSlot( SID_CURSORBLKUP );  break;
        case SID_CURSORLEFT_SEL:        rReq.SetSlot( SID_CURSORLEFT );  break;
        case SID_CURSORBLKLEFT_SEL:     rReq.SetSlot( SID_CURSORBLKLEFT );  break;
        case SID_CURSORRIGHT_SEL:       rReq.SetSlot( SID_CURSORRIGHT );  break;
        case SID_CURSORBLKRIGHT_SEL:    rReq.SetSlot( SID_CURSORBLKRIGHT );  break;
        case SID_CURSORPAGEDOWN_SEL:    rReq.SetSlot( SID_CURSORPAGEDOWN );  break;
        case SID_CURSORPAGEUP_SEL:      rReq.SetSlot( SID_CURSORPAGEUP );  break;
        case SID_CURSORPAGERIGHT_SEL:   rReq.SetSlot( SID_CURSORPAGERIGHT_ );  break;
        case SID_CURSORPAGELEFT_SEL:    rReq.SetSlot( SID_CURSORPAGELEFT_ );  break;
        default:
            DBG_ERROR("Unbekannte Message bei ViewShell (CursorSel)");
            return;
    }
    rReq.AppendItem( SfxInt16Item(FN_PARAM_1, nRepeat ) );
    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, TRUE) );
    ExecuteSlot( rReq, GetInterface() );
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
            pTabViewShell->SelectNextTab( 1 );
            break;

        case SID_PREV_TABLE:
            pTabViewShell->SelectNextTab( -1 );
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

        case SID_SETINPUTMODE:
            SC_MOD()->SetInputMode( SC_INPUT_TABLE );
            break;

        case SID_CURSORTOPOFSCREEN:
            pTabViewShell->MoveCursorScreen( 0, -1, SC_FOLLOW_LINE, FALSE );
            break;

        case SID_CURSORENDOFSCREEN:
            pTabViewShell->MoveCursorScreen( 0, 1, SC_FOLLOW_LINE, FALSE );
            break;

        default:
            DBG_ERROR("Unbekannte Message bei ViewShell (Cursor)");
            return;
    }

    rReq.Done();
}

void ScCellShell::ExecutePageSel( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    USHORT              nSlotId  = rReq.GetSlot();
    switch ( nSlotId )
    {
        case SID_CURSORHOME_SEL:        rReq.SetSlot( SID_CURSORHOME );  break;
        case SID_CURSOREND_SEL:         rReq.SetSlot( SID_CURSOREND );  break;
        case SID_CURSORTOPOFFILE_SEL:   rReq.SetSlot( SID_CURSORTOPOFFILE );  break;
        case SID_CURSORENDOFFILE_SEL:   rReq.SetSlot( SID_CURSORENDOFFILE );  break;
        default:
            DBG_ERROR("Unbekannte Message bei ViewShell (ExecutePageSel)");
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

    if ( pReqArgs != NULL )
    {
        const   SfxPoolItem* pItem;
        if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
            bSel = ((const SfxBoolItem*)pItem)->GetValue();
    }

    pTabViewShell->ExecuteInputDirect();
    switch ( nSlotId )
    {
        case SID_CURSORHOME:
            pTabViewShell->MoveCursorEnd( -1, 0, SC_FOLLOW_LINE, bSel );
            break;

        case SID_CURSOREND:
            pTabViewShell->MoveCursorEnd( 1, 0, SC_FOLLOW_JUMP, bSel );
            break;

        case SID_CURSORTOPOFFILE:
            pTabViewShell->MoveCursorEnd( -1, -1, SC_FOLLOW_LINE, bSel );
            break;

        case SID_CURSORENDOFFILE:
            pTabViewShell->MoveCursorEnd( 1, 1, SC_FOLLOW_JUMP, bSel );
            break;

        default:
            DBG_ERROR("Unbekannte Message bei ViewShell (ExecutePage)");
            return;
    }

    rReq.AppendItem( SfxBoolItem(FN_PARAM_2, bSel) );
    rReq.Done();
}




