/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: move.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:42:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
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
    inline ShellMoveCrsr( SwWrtShell* pWrtSh, BOOL bSel )
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

void SwWrtShell::MoveCrsr( BOOL bWithSelect )
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

BOOL SwWrtShell::SimpleMove( FNSimpleMove FnSimpleMove, BOOL bSelect )
{
    BOOL nRet;
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


BOOL SwWrtShell::Left( USHORT nMode, BOOL bSelect,
                            USHORT nCount, BOOL bBasicCall, BOOL bVisual )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() -= VisArea().Width() * nReadOnlyScrollOfst / 100;
        rView.SetVisArea( aTmp );
        return TRUE;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Left( nCount, nMode, bVisual );
    }
}



BOOL SwWrtShell::Right( USHORT nMode, BOOL bSelect,
                            USHORT nCount, BOOL bBasicCall, BOOL bVisual )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() && !GetViewOptions()->IsSelectionInReadonly() )
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
        return SwCrsrShell::Right( nCount, nMode, bVisual );
    }
}



BOOL SwWrtShell::Up( BOOL bSelect, USHORT nCount, BOOL bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
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



BOOL SwWrtShell::Down( BOOL bSelect, USHORT nCount, BOOL bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() && !GetViewOptions()->IsSelectionInReadonly())
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



BOOL SwWrtShell::LeftMargin( BOOL bSelect, BOOL bBasicCall )
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



BOOL SwWrtShell::RightMargin( BOOL bSelect, BOOL bBasicCall  )
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
        if( !bBlockMode )
        {
            if ( !bSelect )
                EnterStdMode();
            else
                SttSelect();
        }
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

    if( !bBlockMode )
    {
        if ( !bSelect )
            EnterStdMode();
        else
            SttSelect();
    }
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
           SwCrsrShell::SttEndDoc(TRUE);
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
           SwCrsrShell::SttEndDoc(FALSE);
}



BOOL SwWrtShell::SttDoc( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoStart(FALSE, 0, bSelect );
}



BOOL SwWrtShell::EndDoc( BOOL bSelect)
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoEnd();
}


BOOL SwWrtShell::SttNxtPg( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageStart );
}



BOOL SwWrtShell::SttPrvPg( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageStart );
}



BOOL SwWrtShell::EndNxtPg( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageEnd );
}



BOOL SwWrtShell::EndPrvPg( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageEnd );
}



BOOL SwWrtShell::SttPg( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageStart );
}



BOOL SwWrtShell::EndPg( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageEnd );
}



BOOL SwWrtShell::SttPara( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara( fnParaCurr, fnParaStart );
}



BOOL SwWrtShell::EndPara( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara(fnParaCurr,fnParaEnd);
}


/*------------------------------------------------------------------------
 Beschreibung:  Spaltenweises Springen
 Parameter:     mit oder ohne SSelection
 Return:        Erfolg oder Misserfolg
------------------------------------------------------------------------*/



BOOL SwWrtShell::StartOfColumn( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnStart);
}



BOOL SwWrtShell::EndOfColumn( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnEnd);
}



BOOL SwWrtShell::StartOfNextColumn( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn( fnColumnNext, fnColumnStart);
}



BOOL SwWrtShell::EndOfNextColumn( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnNext, fnColumnEnd);
}



BOOL SwWrtShell::StartOfPrevColumn( BOOL bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnStart);
}



BOOL SwWrtShell::EndOfPrevColumn( BOOL bSelect )
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



BOOL SwWrtShell::GotoBookmark( USHORT nPos, BOOL bSelect, BOOL bStart )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return SwCrsrShell::GotoBookmark( nPos, bStart );
}



BOOL SwWrtShell::SelectTxtAttr( USHORT nWhich, const SwTxtAttr* pAttr )
{
    BOOL bRet;
    {
        MV_KONTEXT(this);
        SttSelect();
        bRet = SwCrsrShell::SelectTxtAttr( nWhich, FALSE, pAttr );
    }
    EndSelect();
    return bRet;
}



