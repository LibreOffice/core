/*************************************************************************
 *
 *  $RCSfile: move.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
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

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

/*  Immer:
    -   Zuruecksetzen des Cursorstacks
    -   Timer nachtriggern
    -   gfs. GCAttr

    bei Selektion
    -   SttSelect()

    sonst
    -   EndSelect()
 */

const long nReadOnlyScrollOfst = 10;

class ShellMoveCrsr
{
    SwWrtShell* pSh;
    BOOL bAct;
public:
    inline ShellMoveCrsr( SwWrtShell* pWrtSh, FASTBOOL bSel )
    {
        bAct = !pWrtSh->ActionPend() && (pWrtSh->GetFrmType(0,FALSE) & FRMTYPE_FLY_ANY);
        ( pSh = pWrtSh )->MoveCrsr( BOOL(bSel) );
        pWrtSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_HYPERLINK_GETLINK);
    }
    inline ~ShellMoveCrsr()
    {
        if( bAct )
        {
            //Die Action wird fuer das Scrollen in "einabsaetzigen" Rahmen mit
            //fester Hoehe gebraucht.
            pSh->StartAllAction();
            pSh->EndAllAction();
        }
    }
};

void SwWrtShell::MoveCrsr( FASTBOOL bWithSelect )
{
    ResetCursorStack();
    if ( IsGCAttr() )
    {
        GCAttr();
        ClearGCAttr();
    }
    if ( bWithSelect )
        SttSelect();
    else
    {
        EndSelect();
        (this->*fnKillSel)( 0, FALSE );
    }
}

FASTBOOL SwWrtShell::SimpleMove( FNSimpleMove FnSimpleMove, FASTBOOL bSelect )
{
    FASTBOOL nRet;
    if( bSelect )
    {
        SttCrsrMove();
        MoveCrsr( TRUE );
        nRet = (this->*FnSimpleMove)();
        EndCrsrMove();
    }
    else if( 0 != ( nRet = (this->*FnSimpleMove)() ) )
        MoveCrsr( FALSE );
    return nRet;
}


FASTBOOL SwWrtShell::Left( FASTBOOL bSelect, USHORT nCount, BOOL bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() -= VisArea().Width() * nReadOnlyScrollOfst / 100;
        rView.SetVisArea( aTmp );
        return TRUE;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Left( nCount );
    }
}



FASTBOOL SwWrtShell::Right( FASTBOOL bSelect, USHORT nCount, BOOL bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() += VisArea().Width() * nReadOnlyScrollOfst / 100;
        aTmp.X() = rView.SetHScrollMax( aTmp.X() );
        rView.SetVisArea( aTmp );
        return TRUE;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Right( nCount );
    }
}



FASTBOOL SwWrtShell::Up( FASTBOOL bSelect, USHORT nCount, BOOL bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() -= VisArea().Height() * nReadOnlyScrollOfst / 100;
        rView.SetVisArea( aTmp );
        return TRUE;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Up( nCount );
    }
}



FASTBOOL SwWrtShell::Down( FASTBOOL bSelect, USHORT nCount, BOOL bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() += VisArea().Height() * nReadOnlyScrollOfst / 100;
        aTmp.Y() = rView.SetVScrollMax( aTmp.Y() );
        rView.SetVisArea( aTmp );
        return TRUE;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Down( nCount );
    }
}



FASTBOOL SwWrtShell::LeftMargin( FASTBOOL bSelect, FASTBOOL bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = DOCUMENTBORDER;
        rView.SetVisArea( aTmp );
        return TRUE;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::LeftMargin();
    }
}



FASTBOOL SwWrtShell::RightMargin( FASTBOOL bSelect, FASTBOOL bBasicCall  )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = GetDocSize().Width() - VisArea().Width() + DOCUMENTBORDER;
        if( DOCUMENTBORDER > aTmp.X() )
            aTmp.X() = DOCUMENTBORDER;
        rView.SetVisArea( aTmp );
        return TRUE;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::RightMargin(bBasicCall);
    }
}



BOOL SwWrtShell::GoStart( BOOL bKeepArea, BOOL *pMoveTable,
                            BOOL bSelect, BOOL bDontMoveRegion )
{
    if ( IsCrsrInTbl() )
    {
        const BOOL bBoxSelection = HasBoxSelection();
        if ( !bSelect )
            EnterStdMode();
        else
            SttSelect();
            // Tabellenzelle?
        if ( !bBoxSelection && (MoveSection( fnSectionCurr, fnSectionStart)
                || bDontMoveRegion))
        {
            if ( pMoveTable )
                *pMoveTable = FALSE;
            return TRUE;
        }
        if( MoveTable( fnTableCurr, fnTableStart ) || bDontMoveRegion )
        {
            if ( pMoveTable )
                *pMoveTable = TRUE;
            return TRUE;
        }
        else if( bBoxSelection && pMoveTable )
        {
            // JP 09.01.96: wir haben eine Boxselektion (oder leere Zelle)
            //              und wollen selektieren (pMoveTable wird im
            //              SelAll gesetzt). Dann darf die Tabelle nicht
            //              verlassen werden; sonst ist keine Selektion der
            //              gesamten Tabelle moeglich!
            *pMoveTable = TRUE;
            return TRUE;
        }
    }

    if ( !bSelect )
        EnterStdMode();
    else
        SttSelect();
    const USHORT nFrmType = GetFrmType(0,FALSE);
    if ( FRMTYPE_FLY_ANY & nFrmType )
    {
        if( MoveSection( fnSectionCurr, fnSectionStart ) )
            return TRUE;
        else if ( FRMTYPE_FLY_FREE & nFrmType || bDontMoveRegion )
            return FALSE;
    }
    if(( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FOOTNOTE ) & nFrmType )
    {
        if ( MoveSection( fnSectionCurr, fnSectionStart ) )
            return TRUE;
        else if ( bKeepArea )
            return TRUE;
    }
    // Bereiche ???
    return SwCrsrShell::MoveRegion( fnRegionCurrAndSkip, fnRegionStart ) ||
           SwCrsrShell::SttDoc();
}



BOOL SwWrtShell::GoEnd(BOOL bKeepArea, BOOL *pMoveTable)
{
    if ( pMoveTable && *pMoveTable )
        return MoveTable( fnTableCurr, fnTableEnd );

    if ( IsCrsrInTbl() )
    {
        if ( MoveSection( fnSectionCurr, fnSectionEnd ) ||
             MoveTable( fnTableCurr, fnTableEnd ) )
            return TRUE;
    }
    else
    {
        const USHORT nFrmType = GetFrmType(0,FALSE);
        if ( FRMTYPE_FLY_ANY & nFrmType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd ) )
                return TRUE;
            else if ( FRMTYPE_FLY_FREE & nFrmType )
                return FALSE;
        }
        if(( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FOOTNOTE ) & nFrmType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd) )
                return TRUE;
            else if ( bKeepArea )
                return TRUE;
        }
    }
    // Bereiche ???
    return SwCrsrShell::MoveRegion( fnRegionCurrAndSkip, fnRegionEnd ) ||
           SwCrsrShell::EndDoc();
}



FASTBOOL SwWrtShell::SttDoc( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoStart(FALSE, 0, bSelect );
}



FASTBOOL SwWrtShell::EndDoc( FASTBOOL bSelect)
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoEnd();
}


FASTBOOL SwWrtShell::SttNxtPg( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageStart );
}



FASTBOOL SwWrtShell::SttPrvPg( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageStart );
}



FASTBOOL SwWrtShell::EndNxtPg( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageEnd );
}



FASTBOOL SwWrtShell::EndPrvPg( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageEnd );
}



FASTBOOL SwWrtShell::SttPg( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageStart );
}



FASTBOOL SwWrtShell::EndPg( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageEnd );
}



FASTBOOL SwWrtShell::SttPara( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara( fnParaCurr, fnParaStart );
}



FASTBOOL SwWrtShell::EndPara( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara(fnParaCurr,fnParaEnd);
}


/*------------------------------------------------------------------------
 Beschreibung:  Spaltenweises Springen
 Parameter:     mit oder ohne SSelection
 Return:        Erfolg oder Misserfolg
------------------------------------------------------------------------*/



FASTBOOL SwWrtShell::StartOfColumn( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnStart);
}



FASTBOOL SwWrtShell::EndOfColumn( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnEnd);
}



FASTBOOL SwWrtShell::StartOfNextColumn( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn( fnColumnNext, fnColumnStart);
}



FASTBOOL SwWrtShell::EndOfNextColumn( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnNext, fnColumnEnd);
}



FASTBOOL SwWrtShell::StartOfPrevColumn( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnStart);
}



FASTBOOL SwWrtShell::EndOfPrevColumn( FASTBOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnEnd);
}



BOOL SwWrtShell::PushCrsr(SwTwips lOffset, BOOL bSelect)
{
    BOOL bDiff = FALSE;
    SwRect aOldRect( GetCharRect() ), aTmpArea( VisArea() );

    //bDestOnStack besagt, ob ich den Cursor nicht an die aktuelle Position
    //setzen konnte, da in diesem Bereich kein Inhalt vorhanden ist.
    if( !bDestOnStack )
    {
        Point aPt( aOldRect.Center() );

        if( !IsCrsrVisible() )
            // set CrsrPos to top-/bottom left pos. So the pagescroll is not
            // be dependent on the current cursor, but on the visarea.
            aPt.Y() = aTmpArea.Top() + aTmpArea.Height() / 2;

        aPt.Y() += lOffset;
        aDest = GetCntntPos(aPt,lOffset > 0);
        aDest.X() = aPt.X();
        bDestOnStack = TRUE;
    }

    //falls wir eine Rahmenselektion hatten, muss diese nach dem
    //fnSetCrsr entfernt werden und damit wir da wieder hinkommen
    //auf dem Stack gemerkt werden.
    BOOL bIsFrmSel = FALSE;

    BOOL bIsObjSel = FALSE;

    //Zielposition liegt jetzt innerhalb des sichtbaren Bereiches -->
    //Cursor an die Zielposition setzen; merken, dass keine Ziel-
    //position mehr auf dem Stack steht.
    //Der neue sichtbare Bereich wird zuvor ermittelt.
    aTmpArea.Pos().Y() += lOffset;
    if( aTmpArea.IsInside(aDest) )
    {
        if( bSelect )
            SttSelect();
        else
            EndSelect();

        bIsFrmSel = IsFrmSelected();
        bIsObjSel = 0 != IsObjSelected();

        // Rahmenselektion aufheben
        if( bIsFrmSel || bIsObjSel )
        {
            UnSelectFrm();
            LeaveSelFrmMode();
            if ( bIsObjSel )
            {
                GetView().SetDrawFuncPtr( NULL );
                GetView().LeaveDrawCreate();
            }

            CallChgLnk();
        }

        (this->*fnSetCrsr)( &aDest, TRUE );

        bDiff = aOldRect != GetCharRect();

        if( bIsFrmSel )
        {
//          CallChgLnk();
            // bei Frames immer nur die obere Ecke nehmen, damit dieser
            // wieder selektiert werden kann
            aOldRect.SSize( 5, 5 );
        }

            // Zuruecksetzen des Dest. SPoint Flags
        bDestOnStack = FALSE;
    }

    // Position auf den Stack; bDiff besagt, ob ein Unterschied zwischen
    // der alten und der neuen Cursorposition besteht.
    pCrsrStack = new CrsrStack( bDiff, bIsFrmSel, aOldRect.Center(),
                                lOffset, pCrsrStack );
    return !bDestOnStack && bDiff;
}



BOOL SwWrtShell::PopCrsr(BOOL bUpdate, BOOL bSelect)
{
    if( 0 == pCrsrStack)
        return FALSE;

    const BOOL bValidPos = pCrsrStack->bValidCurPos;
    if( bUpdate && bValidPos )
    {
            // falls ein Vorgaenger auf dem Stack steht, dessen Flag fuer eine
            // gueltige Position verwenden.
        SwRect aTmpArea(VisArea());
        aTmpArea.Pos().Y() -= pCrsrStack->lOffset;
        if( aTmpArea.IsInside( pCrsrStack->aDocPos ) )
        {
            if( bSelect )
                SttSelect();
            else
                EndSelect();

            (this->*fnSetCrsr)(&pCrsrStack->aDocPos, !pCrsrStack->bIsFrmSel);
            if( pCrsrStack->bIsFrmSel && IsObjSelectable(pCrsrStack->aDocPos))
            {
                HideCrsr();
                SelectObj( pCrsrStack->aDocPos );
                EnterSelFrmMode( &pCrsrStack->aDocPos );
            }
        }
            // Falls eine Verschiebung zwischen dem sichtbaren Bereich
            // und der gemerkten Cursorpositionen auftritt, werden
            // alle gemerkten Positionen weggeschmissen
        else
        {
            _ResetCursorStack();
            return FALSE;
        }
    }
    CrsrStack *pTmp = pCrsrStack;
    pCrsrStack = pCrsrStack->pNext;
    delete pTmp;
    if( 0 == pCrsrStack )
    {
        ePageMove = MV_NO;
        bDestOnStack = FALSE;
    }
    return bValidPos;
}

/*
 * Zuruecksetzen aller gepushten Cursorpositionen; dieser werden nicht
 * zur Anzeige gebracht ( --> Kein Start-/EndAction!!)
 */



void SwWrtShell::_ResetCursorStack()
{
    CrsrStack *pTmp = pCrsrStack;
    while(pCrsrStack)
    {
        pTmp = pCrsrStack->pNext;
        delete pCrsrStack;
        pCrsrStack = pTmp;
    }
    ePageMove = MV_NO;
    bDestOnStack = FALSE;
}
/**************

    falls kein Stack existiert --> Selektionen aufheben
    falls Stack && Richtungswechsel
        --> Cursor poppen und return
    sonst
        --> Cursor pushen
             Cursor umsetzen

***************/



BOOL SwWrtShell::PageCrsr(SwTwips lOffset, BOOL bSelect)
{
    // nichts tun, wenn ein Offset von 0 angegeben wurde
    if(!lOffset) return FALSE;
        // Diente mal dazu, eine Neuformatierung fuer das Layout
        // zu erzwingen.
        // Hat so nicht funktioniert, da der Cursor nicht gesetzt
        // wurde, da dies innerhalb einer Start- / EndActionklammerung
        // nicht geschieht.
        // Da am Ende nur ViewShell::EndAction() gerufen wird,
        // findet auch hier keine Aktualisierung der Anzeige
        // der Cursorposition statt.
        // Die CrsrShell- Actionklammerung kann nicht verwendet werden,
        // da sie immer zu einer Anzeige des Cursors fuehrt, also auch,
        // wenn nach dem Blaettern in einen Bereich ohne gueltige Position
        // geblaettert wurde.
        //  ViewShell::StartAction();
    PageMove eDir = lOffset > 0? MV_PAGE_DOWN: MV_PAGE_UP;
        // Richtungswechsel und Stack vorhanden
    if( eDir != ePageMove && ePageMove != MV_NO && PopCrsr( TRUE, bSelect ))
        return TRUE;

    const BOOL bRet = PushCrsr(lOffset, bSelect);
    ePageMove = eDir;
    return bRet;
}



BOOL SwWrtShell::GotoPage(USHORT nPage, BOOL bRecord)
{
    ShellMoveCrsr aTmp( this, FALSE);
    if( SwCrsrShell::GotoPage(nPage) && bRecord)
    {
        if(IsSelFrmMode())
        {
            UnSelectFrm();
            LeaveSelFrmMode();
        }
        return TRUE;
    }
    return FALSE;
}



FASTBOOL SwWrtShell::GotoBookmark( USHORT nPos, BOOL bSelect, BOOL bStart )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return SwCrsrShell::GotoBookmark( nPos, bStart );
}



FASTBOOL SwWrtShell::SelectTxtAttr( USHORT nWhich, const SwTxtAttr* pAttr )
{
    ShellMoveCrsr aTmp( this, TRUE );
    return SwCrsrShell::SelectTxtAttr( nWhich, FALSE, pAttr );
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.100  2000/09/18 16:06:26  willem.vandorp
      OpenOffice header added.

      Revision 1.99  2000/09/07 15:59:35  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.98  2000/06/27 19:42:49  jp
      Bug #70447#: select the current TOXMark if the EditDialog is open

      Revision 1.97  1999/10/12 07:24:36  os
      #69115# API parameter in GotoLeftRight/LRMargin

      Revision 1.96  1999/10/06 12:58:10  jp
      PushCrsr: if cursor not in visarea, thne use the visarea for new position

      Revision 1.95  1998/11/27 13:59:32  AMA
      Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten


      Rev 1.94   27 Nov 1998 14:59:32   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.93   14 Jun 1998 13:46:24   MA
   ein paar inlines mittels function pointer

      Rev 1.92   04 May 1998 18:00:20   JP
   Bug #49993#: IsSelOver darf nicht immer den Cursor verschieben; GotoBookmark mit ReturntWert

      Rev 1.91   12 Feb 1998 20:12:22   JP
   Left-/RightMargin: zus. Flag fuer BasicCall, im ReadOnlyCall an Anfang/Ende des Randes scrollen

      Rev 1.90   24 Nov 1997 14:35:06   MA
   includes

      Rev 1.89   03 Nov 1997 14:02:54   MA
   precomp entfernt

      Rev 1.88   29 Jul 1997 09:24:26   OM
   Hyperlink Statusmethode

      Rev 1.87   12 Jun 1997 11:20:20   MA
   chg: CursorTimer vorlaeufig entfernt

      Rev 1.86   29 Apr 1997 16:30:56   AMA
   New: Scrolling bei abgeschnittenen Absaetzen in Rahmen mit fester Hoehe.

      Rev 1.85   06 Mar 1997 12:27:28   JP
   Left/Right/Up/Down: bei API-Calls in ReadOnly Docs normale verhalten

      Rev 1.84   18 Feb 1997 13:40:44   MA
   new: Rahmen im Readonly  editierbar

      Rev 1.83   21 Nov 1996 16:48:52   AMA
   Fix #33452#: Das GCAttr-Flag wandert von der WriterShell in die CrsrShell

      Rev 1.82   30 Oct 1996 10:14:50   JP
   Bug #32859#: PushCrsr - Rahmenselektin aufheben; PopCrsr: Rahmenselektion wieder erzeugen

      Rev 1.81   01 Oct 1996 16:55:32   OM
   Hyperlinks editieren

      Rev 1.80   24 Sep 1996 12:33:38   MA
   chg: Travelling fuer ReadOnly

      Rev 1.79   29 Aug 1996 09:26:02   OS
   includes

      Rev 1.78   16 Aug 1996 10:26:08   OS
   richtiges Verhalten bei SelAll

      Rev 1.77   08 Aug 1996 15:51:50   JP
   neu: SelectTextAttr - vom Attribut umspannten Bereich Selectieren

      Rev 1.76   15 Jul 1996 16:31:54   OM
   Bei Selektionsaenderung Slot invalidieren

      Rev 1.75   10 May 1996 15:55:22   OS
   GotoBookmark markiert jetzt immer bug #27690#

      Rev 1.74   08 May 1996 12:58:44   OS
   neu: GotoBookmark mit Select- und Start Flag

      Rev 1.73   09 Jan 1996 09:46:58   JP
   GoStart: an Dok-Anfang springen, wenn die 1. Box leer ist!

      Rev 1.72   22 Dec 1995 20:17:38   JP
   Bug 22611: Rahmenselecktion wird verlassen/wiederhergestellt - dadurch die richtigen Shells!

      Rev 1.71   24 Nov 1995 16:59:08   OM
   PCH->PRECOMPILED

      Rev 1.70   23 Nov 1995 19:50:54   OS
   +Fwd/BwdPara

      Rev 1.69   30 Oct 1995 18:43:08   OS
   Select-Param im EndPg auch auswerten

*************************************************************************/


