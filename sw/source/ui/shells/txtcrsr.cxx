/*************************************************************************
 *
 *  $RCSfile: txtcrsr.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:44:56 $
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


#pragma hdrstop

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _ARGS_HXX //autogen
#include <svtools/args.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif

#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _NUM_HXX
#include <num.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_H
#include <globals.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif



void SwTextShell::ExecBasicMove(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    GetView().GetEditWin().FlushInBuffer( &rSh );
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

    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
            GetView().GetViewFrame()->GetBindings().GetRecorder();
    if ( xRecorder.is() )
    {
        rReq.AppendItem( SfxInt16Item(FN_PARAM_MOVE_COUNT, nCount) );
        rReq.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, bSelect) );
    }
    USHORT nSlot = rReq.GetSlot();
    rReq.Done();
    for( USHORT i = 0; i < nCount; i++ )
    {
        switch(nSlot)
        {
        case FN_CHAR_LEFT:  rSh.Left( CRSR_SKIP_CELLS,  bSelect, 1, FALSE, TRUE ); break;
        case FN_CHAR_RIGHT: rSh.Right( CRSR_SKIP_CELLS, bSelect, 1, FALSE, TRUE ); break;
        case FN_LINE_UP:    rSh.Up   ( bSelect, 1 ); break;
        case FN_LINE_DOWN:  rSh.Down ( bSelect, 1 ); break;
        default:            ASSERT(FALSE, falscher Dispatcher); return;
        }
    }
}

void SwTextShell::ExecMove(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    GetView().GetEditWin().FlushInBuffer( &rSh );

    USHORT nSlot = rReq.GetSlot();
    BOOL bRet = FALSE;
    switch ( nSlot )
    {
        case FN_START_OF_LINE_SEL:
        case FN_START_OF_LINE:      bRet = rSh.LeftMargin ( FN_START_OF_LINE_SEL == nSlot );
        break;

        case FN_END_OF_LINE_SEL:
        case FN_END_OF_LINE:        bRet = rSh.RightMargin( FN_END_OF_LINE_SEL == nSlot );
        break;

        case FN_START_OF_DOCUMENT_SEL:
        case FN_START_OF_DOCUMENT:  bRet = rSh.SttDoc      ( FN_START_OF_DOCUMENT_SEL == nSlot);
        break;

        case FN_END_OF_DOCUMENT_SEL:
        case FN_END_OF_DOCUMENT:    bRet = rSh.EndDoc( FN_END_OF_DOCUMENT_SEL == nSlot );
        break;

        case FN_SELECT_WORD:            bRet = rSh.SelNearestWrd(); break;

        case SID_SELECTALL:             bRet = rSh.SelAll();            break;
        default:                    ASSERT(FALSE, falscher Dispatcher); return;
    }

    if ( bRet )
        rReq.Done();
    else
        rReq.Ignore();
}

void SwTextShell::ExecMovePage(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    GetView().GetEditWin().FlushInBuffer( &rSh );

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
        default:                    ASSERT(FALSE, falscher Dispatcher); return;
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
        default:                      ASSERT(FALSE, falscher Dispatcher); return;
    }
    rReq.Done();
}

void SwTextShell::ExecMoveLingu(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    GetView().GetEditWin().FlushInBuffer( GetShellPtr() );

    USHORT nSlot = rReq.GetSlot();
    BOOL bRet = FALSE;
    switch ( nSlot )
    {
        case FN_NEXT_WORD_SEL:
        case FN_NEXT_WORD:      bRet = rSh.NxtWrd( FN_NEXT_WORD_SEL == nSlot );
        break;

        case FN_START_OF_PARA_SEL:
        case FN_START_OF_PARA:  bRet = rSh.SttPara( FN_START_OF_PARA_SEL == nSlot );
        break;

        case FN_END_OF_PARA_SEL:
        case FN_END_OF_PARA:    bRet = rSh.EndPara( FN_END_OF_PARA_SEL == nSlot );
        break;

        case FN_PREV_WORD_SEL:
        case FN_PREV_WORD:      bRet = rSh.PrvWrd( FN_PREV_WORD_SEL == nSlot );
        break;

        case FN_NEXT_SENT_SEL:
        case FN_NEXT_SENT:      bRet = rSh.FwdSentence( FN_NEXT_SENT_SEL == nSlot );
        break;

        case FN_PREV_SENT_SEL:
        case FN_PREV_SENT:      bRet = rSh.BwdSentence( FN_PREV_SENT_SEL == nSlot );
        break;

        case FN_NEXT_PARA:      bRet = rSh.FwdPara    ( FALSE );
        break;

        case FN_PREV_PARA:      bRet = rSh.BwdPara    ( FALSE );
        break;
        default:                ASSERT(FALSE, falscher Dispatcher); return;
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
        case FN_CNTNT_TO_NEXT_FRAME:
            bRet = rSh.GotoObj(TRUE, GOTO_ANY);
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
            ASSERT(FALSE, falscher Dispatcher);
            return;
    }

    if( bSetRetVal )
        rReq.SetReturnValue(SfxBoolItem( nSlot, bRet ));
    rReq.Done();

}

