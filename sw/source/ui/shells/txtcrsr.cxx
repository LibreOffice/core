/*************************************************************************
 *
 *  $RCSfile: txtcrsr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:47 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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


#include "view.hxx"
#include "cmdid.h"
#include "wrtsh.hxx"
#include "globals.hrc"
#include "textsh.hxx"
#include "num.hxx"
#include "bookmrk.hxx"
#include "edtwin.hxx"
#include "globals.h"





void SwTextShell::ExecBasicMove(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    GetView().GetEditWin().FlushInBuffer( &rSh );

    USHORT nSlot = rReq.GetSlot();
    switch(nSlot)
    {
        case FN_CHAR_LEFT_SEL:
        case FN_CHAR_LEFT:  rSh.Left ( FN_CHAR_LEFT_SEL == nSlot, 1 ); break;
        case FN_CHAR_RIGHT_SEL:
        case FN_CHAR_RIGHT: rSh.Right( FN_CHAR_RIGHT_SEL == nSlot, 1 ); break;
        case FN_LINE_UP_SEL:
        case FN_LINE_UP:    rSh.Up   ( FN_LINE_UP_SEL == nSlot, 1 ); break;
        case FN_LINE_DOWN_SEL:
        case FN_LINE_DOWN:  rSh.Down ( FN_LINE_DOWN_SEL == nSlot, 1 ); break;
        default:            ASSERT(FALSE, falscher Dispatcher); return;
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

        case FN_SELECT_WORD:            rSh.SelNearestWrd();    break;

        case SID_SELECTALL:             rSh.SelAll();           break;
        default:                    ASSERT(FALSE, falscher Dispatcher); return;
    }
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
}

void SwTextShell::ExecMoveMisc(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    USHORT nSlot = rReq.GetSlot();
    BOOL bSetRetVal = TRUE, bRet = TRUE;
    switch ( nSlot )
    {
        case FN_CNTNT_TO_NEXT_FRAME:
            bRet = rSh.GotoFlyTxt();
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

}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.47  2000/09/18 16:06:07  willem.vandorp
    OpenOffice header added.

    Revision 1.46  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.45  2000/05/09 14:41:35  os
    BASIC interface partially removed

    Revision 1.44  1999/04/26 11:06:12  KZ
    #include <svtools/args.hxx> eingefuegt


      Rev 1.43   26 Apr 1999 13:06:12   KZ
   #include <svtools/args.hxx> eingefuegt

      Rev 1.42   14 Jan 1999 15:50:26   JP
   Bug #60794#: Fehlererkennung beim Tabellenrechnen und anspringen von Formeln

      Rev 1.41   27 Nov 1998 14:56:08   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.40   17 Aug 1998 16:09:12   OS
   GPF nach Shellwechsel waehrend Recording #55041#

      Rev 1.39   12 Feb 1998 20:12:44   JP
   Left-/RightMargin: zus. Flag fuer BasicCall

      Rev 1.38   24 Nov 1997 09:47:54   MA
   includes

      Rev 1.37   03 Nov 1997 13:55:54   MA
   precomp entfernt

      Rev 1.36   04 Sep 1997 17:12:04   MA
   includes

      Rev 1.35   07 Apr 1997 17:48:12   MH
   chg: header

      Rev 1.34   06 Mar 1997 12:28:12   JP
   Left/Right/Up/Down: bei API-Calls in ReadOnly Docs normale verhalten

      Rev 1.33   23 Feb 1997 22:27:04   OS
   diverse Move-Slots mit BOOL - return

      Rev 1.32   20 Feb 1997 17:17:38   OS
   JumpToPrev/NextRegion

      Rev 1.31   07 Feb 1997 14:06:06   OS
   GoLeft/Right/Up/Down ueber for-Schleife aufrufen #35596#

      Rev 1.30   29 Aug 1996 11:40:04   OS
   Next/PrevFootnote liefern BOOL

      Rev 1.29   28 Aug 1996 15:54:56   OS
   includes

      Rev 1.28   13 Feb 1996 09:51:28   OS
   JumpToPrev/NextTable/Frame erhalten return type BOOL

      Rev 1.27   24 Jan 1996 15:18:36   OS
   Selection-Param ist Bool und nicht Int16, Bug #23549#

      Rev 1.26   24 Nov 1995 16:59:36   OM
   PCH->PRECOMPILED

      Rev 1.25   23 Nov 1995 19:52:02   OS
   +FN_SELECT_ALL fuer PM2, +Next/PrevPara

      Rev 1.24   10 Nov 1995 15:29:44   OS
   GotoVariable/Region an die Baseshell

      Rev 1.23   09 Nov 1995 17:59:54   OS
   neue Slots fuer Grosskunden

      Rev 1.22   23 Oct 1995 17:33:36   OS
   *_SEL auf * umgeappt

      Rev 1.21   22 Sep 1995 16:58:18   OS
   Schoenheitskorrektur: GetArgs() fuellt immer pArgs!

      Rev 1.20   18 Sep 1995 14:04:38   OS
   geht wieder mit PCH

      Rev 1.19   04 Sep 1995 14:13:08   HJS
   add: #include <sbx.hxx>

------------------------------------------------------------------------*/


