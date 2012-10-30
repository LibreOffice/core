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

#include <sfx2/bindings.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <crsskip.hxx>

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
    sal_Bool bAct;
public:
    inline ShellMoveCrsr( SwWrtShell* pWrtSh, sal_Bool bSel )
    {
        bAct = !pWrtSh->ActionPend() && (pWrtSh->GetFrmType(0,sal_False) & FRMTYPE_FLY_ANY);
        ( pSh = pWrtSh )->MoveCrsr( sal_Bool(bSel) );
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

void SwWrtShell::MoveCrsr( sal_Bool bWithSelect )
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
        (this->*fnKillSel)( 0, sal_False );
    }
}

sal_Bool SwWrtShell::SimpleMove( FNSimpleMove FnSimpleMove, sal_Bool bSelect )
{
    sal_Bool nRet;
    if( bSelect )
    {
        SttCrsrMove();
        MoveCrsr( sal_True );
        nRet = (this->*FnSimpleMove)();
        EndCrsrMove();
    }
    else if( 0 != ( nRet = (this->*FnSimpleMove)() ) )
        MoveCrsr( sal_False );
    return nRet;
}


sal_Bool SwWrtShell::Left( sal_uInt16 nMode, sal_Bool bSelect,
                            sal_uInt16 nCount, sal_Bool bBasicCall, sal_Bool bVisual )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() -= VisArea().Width() * nReadOnlyScrollOfst / 100;
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Left( nCount, nMode, bVisual );
    }
}



sal_Bool SwWrtShell::Right( sal_uInt16 nMode, sal_Bool bSelect,
                            sal_uInt16 nCount, sal_Bool bBasicCall, sal_Bool bVisual )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() && !GetViewOptions()->IsSelectionInReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() += VisArea().Width() * nReadOnlyScrollOfst / 100;
        aTmp.X() = rView.SetHScrollMax( aTmp.X() );
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Right( nCount, nMode, bVisual );
    }
}



sal_Bool SwWrtShell::Up( sal_Bool bSelect, sal_uInt16 nCount, sal_Bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly()  && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() -= VisArea().Height() * nReadOnlyScrollOfst / 100;
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Up( nCount );
    }
}



sal_Bool SwWrtShell::Down( sal_Bool bSelect, sal_uInt16 nCount, sal_Bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() && !GetViewOptions()->IsSelectionInReadonly())
    {
        Point aTmp( VisArea().Pos() );
        aTmp.Y() += VisArea().Height() * nReadOnlyScrollOfst / 100;
        aTmp.Y() = rView.SetVScrollMax( aTmp.Y() );
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::Down( nCount );
    }
}



sal_Bool SwWrtShell::LeftMargin( sal_Bool bSelect, sal_Bool bBasicCall )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = DOCUMENTBORDER;
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::LeftMargin();
    }
}



sal_Bool SwWrtShell::RightMargin( sal_Bool bSelect, sal_Bool bBasicCall  )
{
    if ( !bSelect && !bBasicCall && IsCrsrReadonly() )
    {
        Point aTmp( VisArea().Pos() );
        aTmp.X() = GetDocSize().Width() - VisArea().Width() + DOCUMENTBORDER;
        if( DOCUMENTBORDER > aTmp.X() )
            aTmp.X() = DOCUMENTBORDER;
        rView.SetVisArea( aTmp );
        return sal_True;
    }
    else
    {
        ShellMoveCrsr aTmp( this, bSelect );
        return SwCrsrShell::RightMargin(bBasicCall);
    }
}



sal_Bool SwWrtShell::GoStart( sal_Bool bKeepArea, sal_Bool *pMoveTable,
                            sal_Bool bSelect, sal_Bool bDontMoveRegion )
{
    if ( IsCrsrInTbl() )
    {
        const sal_Bool bBoxSelection = HasBoxSelection();
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
                *pMoveTable = sal_False;
            return sal_True;
        }
        if( MoveTable( fnTableCurr, fnTableStart ) || bDontMoveRegion )
        {
            if ( pMoveTable )
                *pMoveTable = sal_True;
            return sal_True;
        }
        else if( bBoxSelection && pMoveTable )
        {
            // JP 09.01.96: wir haben eine Boxselektion (oder leere Zelle)
            //              und wollen selektieren (pMoveTable wird im
            //              SelAll gesetzt). Dann darf die Tabelle nicht
            //              verlassen werden; sonst ist keine Selektion der
            //              gesamten Tabelle moeglich!
            *pMoveTable = sal_True;
            return sal_True;
        }
    }

    if( !bBlockMode )
    {
        if ( !bSelect )
            EnterStdMode();
        else
            SttSelect();
    }
    const sal_uInt16 nFrmType = GetFrmType(0,sal_False);
    if ( FRMTYPE_FLY_ANY & nFrmType )
    {
        if( MoveSection( fnSectionCurr, fnSectionStart ) )
            return sal_True;
        else if ( FRMTYPE_FLY_FREE & nFrmType || bDontMoveRegion )
            return sal_False;
    }
    if(( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FOOTNOTE ) & nFrmType )
    {
        if ( MoveSection( fnSectionCurr, fnSectionStart ) )
            return sal_True;
        else if ( bKeepArea )
            return sal_True;
    }
    // Bereiche ???
    return SwCrsrShell::MoveRegion( fnRegionCurrAndSkip, fnRegionStart ) ||
           SwCrsrShell::SttEndDoc(sal_True);
}



sal_Bool SwWrtShell::GoEnd(sal_Bool bKeepArea, sal_Bool *pMoveTable)
{
    if ( pMoveTable && *pMoveTable )
        return MoveTable( fnTableCurr, fnTableEnd );

    if ( IsCrsrInTbl() )
    {
        if ( MoveSection( fnSectionCurr, fnSectionEnd ) ||
             MoveTable( fnTableCurr, fnTableEnd ) )
            return sal_True;
    }
    else
    {
        const sal_uInt16 nFrmType = GetFrmType(0,sal_False);
        if ( FRMTYPE_FLY_ANY & nFrmType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd ) )
                return sal_True;
            else if ( FRMTYPE_FLY_FREE & nFrmType )
                return sal_False;
        }
        if(( FRMTYPE_HEADER | FRMTYPE_FOOTER | FRMTYPE_FOOTNOTE ) & nFrmType )
        {
            if ( MoveSection( fnSectionCurr, fnSectionEnd) )
                return sal_True;
            else if ( bKeepArea )
                return sal_True;
        }
    }
    // Bereiche ???
    return SwCrsrShell::MoveRegion( fnRegionCurrAndSkip, fnRegionEnd ) ||
           SwCrsrShell::SttEndDoc(sal_False);
}



sal_Bool SwWrtShell::SttDoc( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoStart(sal_False, 0, bSelect );
}



sal_Bool SwWrtShell::EndDoc( sal_Bool bSelect)
{
    ShellMoveCrsr aTmp( this, bSelect );
    return GoEnd();
}


sal_Bool SwWrtShell::SttNxtPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageStart );
}



sal_Bool SwWrtShell::SttPrvPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageStart );
}



sal_Bool SwWrtShell::EndNxtPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageNext, fnPageEnd );
}



sal_Bool SwWrtShell::EndPrvPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPagePrev, fnPageEnd );
}



sal_Bool SwWrtShell::SttPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageStart );
}



sal_Bool SwWrtShell::EndPg( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePage( fnPageCurr, fnPageEnd );
}



sal_Bool SwWrtShell::SttPara( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara( fnParaCurr, fnParaStart );
}



sal_Bool SwWrtShell::EndPara( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect );
    return MovePara(fnParaCurr,fnParaEnd);
}


/*------------------------------------------------------------------------
 Beschreibung:  Spaltenweises Springen
 Parameter:     mit oder ohne SSelection
 Return:        Erfolg oder Misserfolg
------------------------------------------------------------------------*/



sal_Bool SwWrtShell::StartOfColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnStart);
}



sal_Bool SwWrtShell::EndOfColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnCurr, fnColumnEnd);
}



sal_Bool SwWrtShell::StartOfNextColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn( fnColumnNext, fnColumnStart);
}



sal_Bool SwWrtShell::EndOfNextColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnNext, fnColumnEnd);
}



sal_Bool SwWrtShell::StartOfPrevColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnStart);
}



sal_Bool SwWrtShell::EndOfPrevColumn( sal_Bool bSelect )
{
    ShellMoveCrsr aTmp( this, bSelect);
    return MoveColumn(fnColumnPrev, fnColumnEnd);
}



sal_Bool SwWrtShell::PushCrsr(SwTwips lOffset, sal_Bool bSelect)
{
    sal_Bool bDiff = sal_False;
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
        bDestOnStack = sal_True;
    }

    //falls wir eine Rahmenselektion hatten, muss diese nach dem
    //fnSetCrsr entfernt werden und damit wir da wieder hinkommen
    //auf dem Stack gemerkt werden.
    sal_Bool bIsFrmSel = sal_False;

    sal_Bool bIsObjSel = sal_False;

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

        (this->*fnSetCrsr)( &aDest, sal_True );

        bDiff = aOldRect != GetCharRect();

        if( bIsFrmSel )
        {
            // bei Frames immer nur die obere Ecke nehmen, damit dieser
            // wieder selektiert werden kann
            aOldRect.SSize( 5, 5 );
        }

            // Zuruecksetzen des Dest. SPoint Flags
        bDestOnStack = sal_False;
    }

    // Position auf den Stack; bDiff besagt, ob ein Unterschied zwischen
    // der alten und der neuen Cursorposition besteht.
    pCrsrStack = new CrsrStack( bDiff, bIsFrmSel, aOldRect.Center(),
                                lOffset, pCrsrStack );
    return !bDestOnStack && bDiff;
}



sal_Bool SwWrtShell::PopCrsr(sal_Bool bUpdate, sal_Bool bSelect)
{
    if( 0 == pCrsrStack)
        return sal_False;

    const sal_Bool bValidPos = pCrsrStack->bValidCurPos;
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
            return sal_False;
        }
    }
    CrsrStack *pTmp = pCrsrStack;
    pCrsrStack = pCrsrStack->pNext;
    delete pTmp;
    if( 0 == pCrsrStack )
    {
        ePageMove = MV_NO;
        bDestOnStack = sal_False;
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
    bDestOnStack = sal_False;
}
/**************

    falls kein Stack existiert --> Selektionen aufheben
    falls Stack && Richtungswechsel
        --> Cursor poppen und return
    sonst
        --> Cursor pushen
             Cursor umsetzen

***************/



sal_Bool SwWrtShell::PageCrsr(SwTwips lOffset, sal_Bool bSelect)
{
    // nichts tun, wenn ein Offset von 0 angegeben wurde
    if(!lOffset) return sal_False;
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
    if( eDir != ePageMove && ePageMove != MV_NO && PopCrsr( sal_True, bSelect ))
        return sal_True;

    const sal_Bool bRet = PushCrsr(lOffset, bSelect);
    ePageMove = eDir;
    return bRet;
}



sal_Bool SwWrtShell::GotoPage(sal_uInt16 nPage, sal_Bool bRecord)
{
    ShellMoveCrsr aTmp( this, sal_False);
    if( SwCrsrShell::GotoPage(nPage) && bRecord)
    {
        if(IsSelFrmMode())
        {
            UnSelectFrm();
            LeaveSelFrmMode();
        }
        return sal_True;
    }
    return sal_False;
}



sal_Bool SwWrtShell::GotoMark( const ::sw::mark::IMark* const pMark, sal_Bool bSelect, sal_Bool bStart )
{
    ShellMoveCrsr aTmp( this, bSelect );
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoMark( pMark, bStart );
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

sal_Bool SwWrtShell::GotoFly( const String& rName, FlyCntType eType, sal_Bool bSelFrame )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwFEShell::GotoFly(rName, eType, bSelFrame);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoINetAttr( const SwTxtINetFmt& rAttr )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoINetAttr(rAttr);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

void SwWrtShell::GotoOutline( sal_uInt16 nIdx )
{
    addCurrentPosition();
    SwCrsrShell::GotoOutline (nIdx);
}

bool SwWrtShell::GotoOutline( const String& rName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoOutline (rName);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoRegion( const String& rName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoRegion (rName);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
 }

sal_Bool SwWrtShell::GotoRefMark( const String& rRefMark, sal_uInt16 nSubType,
                                    sal_uInt16 nSeqNo )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoRefMark(rRefMark, nSubType, nSeqNo);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

sal_Bool SwWrtShell::GotoNextTOXBase( const String* pName )
{
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoNextTOXBase(pName);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

bool SwWrtShell::GotoTable( const String& rName )
{
   SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoTable(rName);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

sal_Bool SwWrtShell::GotoFld( const SwFmtFld& rFld ) {
    SwPosition aPos = *GetCrsr()->GetPoint();
    bool bRet = SwCrsrShell::GotoFld(rFld);
    if (bRet)
        aNavigationMgr.addEntry(aPos);
    return bRet;
}

const SwRedline* SwWrtShell::GotoRedline( sal_uInt16 nArrPos, sal_Bool bSelect ) {
    SwPosition aPos = *GetCrsr()->GetPoint();
    const SwRedline *pRedline = SwCrsrShell::GotoRedline(nArrPos, bSelect);
    if (pRedline)
        aNavigationMgr.addEntry(aPos);
    return pRedline;
}



sal_Bool SwWrtShell::SelectTxtAttr( sal_uInt16 nWhich, const SwTxtAttr* pAttr )
{
    sal_Bool bRet;
    {
        SwMvContext aMvContext(this);
        SttSelect();
        bRet = SwCrsrShell::SelectTxtAttr( nWhich, sal_False, pAttr );
    }
    EndSelect();
    return bRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
