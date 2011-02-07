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
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <hintids.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/frmdiritem.hxx>

#include <SwSmartTagMgr.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <viewimp.hxx>
#include <pam.hxx>
#include <swselectionlist.hxx>
#include <IBlockCursor.hxx>
#include "BlockCursor.hxx"
#include <ndtxt.hxx>
#include <flyfrm.hxx>
#include <dview.hxx>
#include <viewopt.hxx>
#include <frmtool.hxx>
#include <crsrsh.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <sectfrm.hxx>
#include <swtable.hxx>
#include <callnk.hxx>
#include <viscrs.hxx>
#include <section.hxx>
#include <docsh.hxx>
#include <scriptinfo.hxx>
#include <globdoc.hxx>
#include <pamtyp.hxx>
#include <mdiexp.hxx>           // ...Percent()
#include <fmteiro.hxx>
#include <wrong.hxx> // SMARTTAGS
#include <unotextrange.hxx> // SMARTTAGS
#include <vcl/svapp.hxx>
#include <numrule.hxx>
#include <IGrammarContact.hxx>

#include <globals.hrc>

#include <comcore.hrc>

using namespace com::sun::star;
using namespace util;

TYPEINIT2(SwCrsrShell,ViewShell,SwModify);


// Funktion loescht, alle ueberlappenden Cursor aus einem Cursor-Ring
void CheckRange( SwCursor* );

//-----------------------------------------------------------------------

/*
 * Ueberpruefe ob der pCurCrsr in einen schon bestehenden Bereich zeigt.
 * Wenn ja, dann hebe den alten Bereich auf.
 */


void CheckRange( SwCursor* pCurCrsr )
{
    const SwPosition *pStt = pCurCrsr->Start(),
        *pEnd = pCurCrsr->GetPoint() == pStt ? pCurCrsr->GetMark() : pCurCrsr->GetPoint();

    SwPaM *pTmpDel = 0,
          *pTmp = (SwPaM*)pCurCrsr->GetNext();

    // durchsuche den gesamten Ring
    while( pTmp != pCurCrsr )
    {
        const SwPosition *pTmpStt = pTmp->Start(),
                        *pTmpEnd = pTmp->GetPoint() == pTmpStt ?
                                        pTmp->GetMark() : pTmp->GetPoint();
        if( *pStt <= *pTmpStt )
        {
            if( *pEnd > *pTmpStt ||
                ( *pEnd == *pTmpStt && *pEnd == *pTmpEnd ))
                pTmpDel = pTmp;
        }
        else
            if( *pStt < *pTmpEnd )
                pTmpDel = pTmp;
        /*
         * liegt ein SPoint oder GetMark innerhalb vom Crsr-Bereich
         * muss der alte Bereich aufgehoben werden.
         * Beim Vergleich ist darauf zu achten, das SPoint nicht mehr zum
         * Bereich gehoert !
         */
        pTmp = (SwPaM*)pTmp->GetNext();
        if( pTmpDel )
        {
            delete pTmpDel;         // hebe alten Bereich auf
            pTmpDel = 0;
        }
    }
}

// -------------- Methoden von der SwCrsrShell -------------

SwPaM * SwCrsrShell::CreateCrsr()
{
    // Innerhalb der Tabellen-SSelection keinen neuen Crsr anlegen
    ASSERT( !IsTableMode(), "in Tabellen SSelection" );

    // neuen Cursor als Kopie vom akt. und in den Ring aufnehmen
    // Verkettung zeigt immer auf den zuerst erzeugten, also vorwaerts
    SwShellCrsr* pNew = new SwShellCrsr( *pCurCrsr );

    // hier den akt. Pam nur logisch Hiden, weil sonst die Invertierung
    // vom kopierten Pam aufgehoben wird !!

    // #i75172# to be able to make a complete content swap, i moved this to a method
    // pNew->Insert( pCurCrsr, 0 );
    // pCurCrsr->Remove( 0, pCurCrsr->Count() );
    pNew->swapContent(*pCurCrsr);

    pCurCrsr->DeleteMark();

    UpdateCrsr( SwCrsrShell::SCROLLWIN );
//  return pCurCrsr;
    return pNew;
}

// loesche den aktuellen Cursor und der folgende wird zum Aktuellen


sal_Bool SwCrsrShell::DestroyCrsr()
{
    // Innerhalb der Tabellen-SSelection keinen neuen Crsr loeschen
    ASSERT( !IsTableMode(), "in Tabellen SSelection" );

    // ist ueberhaupt ein naechtser vorhanden ?
    if(pCurCrsr->GetNext() == pCurCrsr)
        return sal_False;

    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    SwCursor* pNextCrsr = (SwCursor*)pCurCrsr->GetNext();
    delete pCurCrsr;
    pCurCrsr = dynamic_cast<SwShellCrsr*>(pNextCrsr);
    UpdateCrsr();
    return sal_True;
}


SwPaM & SwCrsrShell::CreateNewShellCursor()
{
    if (HasSelection())
    {
        (void) CreateCrsr(); // n.b. returns old cursor
    }
    return *GetCrsr();
}

SwPaM & SwCrsrShell::GetCurrentShellCursor()
{
    return *GetCrsr();
}


// gebe den aktuellen zurueck

SwPaM* SwCrsrShell::GetCrsr( sal_Bool bMakeTblCrsr ) const
{
    if( pTblCrsr )
    {
        if( bMakeTblCrsr && pTblCrsr->IsCrsrMovedUpdt() )
        {
            // geparkte Cursor werden nicht wieder erzeugt
            const SwCntntNode* pCNd;
            if( pTblCrsr->GetPoint()->nNode.GetIndex() &&
                pTblCrsr->GetMark()->nNode.GetIndex() &&
                0 != ( pCNd = pTblCrsr->GetCntntNode() ) && pCNd->GetFrm() &&
                0 != ( pCNd = pTblCrsr->GetCntntNode(sal_False) ) && pCNd->GetFrm())
            {
                SwShellTableCrsr* pTC = (SwShellTableCrsr*)pTblCrsr;
                GetLayout()->MakeTblCrsrs( *pTC );
            }
        }

        if( pTblCrsr->IsChgd() )
        {
            const_cast<SwCrsrShell*>(this)->pCurCrsr =
                dynamic_cast<SwShellCrsr*>(pTblCrsr->MakeBoxSels( pCurCrsr ));
        }
    }
    return pCurCrsr;
}


void SwCrsrShell::StartAction()
{
    if( !ActionPend() )
    {
        // fuer das Update des Ribbon-Bars merken
        const SwNode& rNd = pCurCrsr->GetPoint()->nNode.GetNode();
        nAktNode = rNd.GetIndex();
        nAktCntnt = pCurCrsr->GetPoint()->nContent.GetIndex();
        nAktNdTyp = rNd.GetNodeType();
        bAktSelection = *pCurCrsr->GetPoint() != *pCurCrsr->GetMark();
        if( ND_TEXTNODE & nAktNdTyp )
            nLeftFrmPos = SwCallLink::GetFrm( (SwTxtNode&)rNd, nAktCntnt, sal_True );
        else
            nLeftFrmPos = 0;
    }
    ViewShell::StartAction();           // zur ViewShell
}


void SwCrsrShell::EndAction( const sal_Bool bIdleEnd )
{
/*
//OS: Wird z.B. eine Basic-Action im Hintergrund ausgefuehrt, geht es so nicht
    if( !bHasFocus )
    {
        // hat die Shell nicht den Focus, dann nur das EndAction an
        // die ViewShell weitergeben.
        ViewShell::EndAction( bIdleEnd );
        return;
    }
*/

    sal_Bool bVis = bSVCrsrVis;

    // Idle-Formatierung ?
    if( bIdleEnd && Imp()->GetRegion() )
    {
        pCurCrsr->Hide();

#ifdef SHOW_IDLE_REGION
if( GetWin() )
{
    GetWin()->Push();
    GetWin()->ChangePen( Pen( Color( COL_YELLOW )));
    for( sal_uInt16 n = 0; n < aPntReg.Count(); ++n )
    {
        SwRect aIRect( aPntReg[n] );
        GetWin()->DrawRect( aIRect.SVRect() );
    }
    GetWin()->Pop();
}
#endif

    }

    // vor der letzten Action alle invaliden Numerierungen updaten
    if( 1 == nStartAction )
        GetDoc()->UpdateNumRule();

    // Task: 76923: dont show the cursor in the ViewShell::EndAction() - call.
    //              Only the UpdateCrsr shows the cursor.
    sal_Bool bSavSVCrsrVis = bSVCrsrVis;
    bSVCrsrVis = sal_False;

    ViewShell::EndAction( bIdleEnd );   //der ViewShell den Vortritt lassen

    bSVCrsrVis = bSavSVCrsrVis;

    if( ActionPend() )
    {
        if( bVis )    // auch SV-Cursor wieder anzeigen
            pVisCrsr->Show();

        // falls noch ein ChgCall vorhanden ist und nur noch die Basic
        // Klammerung vorhanden ist, dann rufe ihn. Dadurch wird die interne
        // mit der Basic-Klammerung entkoppelt; die Shells werden umgeschaltet
        if( !BasicActionPend() )
        {
            //JP 12.01.98: Bug #46496# - es muss innerhalb einer BasicAction
            //              der Cursor geupdatet werden; um z.B. den
            //              TabellenCursor zu erzeugen. Im UpdateCrsr wird
            //              das jetzt beruecksichtigt!
            UpdateCrsr( SwCrsrShell::CHKRANGE, bIdleEnd );

            {
                // Crsr-Moves ueberwachen, evt. Link callen
                // der DTOR ist das interressante!!
                SwCallLink aLk( *this, nAktNode, nAktCntnt, (sal_uInt8)nAktNdTyp,
                                nLeftFrmPos, bAktSelection );

            }
            if( bCallChgLnk && bChgCallFlag && aChgLnk.IsSet() )
            {
                aChgLnk.Call( this );
                bChgCallFlag = sal_False;       // Flag zuruecksetzen
            }
        }
        return;
    }

    sal_uInt16 nParm = SwCrsrShell::CHKRANGE;
    if ( !bIdleEnd )
        nParm |= SwCrsrShell::SCROLLWIN;
    UpdateCrsr( nParm, bIdleEnd );      // Cursor-Aenderungen anzeigen

    {
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        aLk.nNode = nAktNode;           // evt. Link callen
        aLk.nNdTyp = (sal_uInt8)nAktNdTyp;
        aLk.nCntnt = nAktCntnt;
        aLk.nLeftFrmPos = nLeftFrmPos;

        if( !nCrsrMove ||
            ( 1 == nCrsrMove && bInCMvVisportChgd ) )
            ShowCrsrs( bSVCrsrVis ? sal_True : sal_False );    // Cursor & Selektionen wieder anzeigen
    }
    // falls noch ein ChgCall vorhanden ist, dann rufe ihn
    if( bCallChgLnk && bChgCallFlag && aChgLnk.IsSet() )
    {
        aChgLnk.Call( this );
        bChgCallFlag = sal_False;       // Flag zuruecksetzen
    }
}


#if defined(DBG_UTIL)

void SwCrsrShell::SttCrsrMove()
{
    ASSERT( nCrsrMove < USHRT_MAX, "To many nested CrsrMoves." );
    ++nCrsrMove;
    StartAction();
}

void SwCrsrShell::EndCrsrMove( const sal_Bool bIdleEnd )
{
    ASSERT( nCrsrMove, "EndCrsrMove() ohne SttCrsrMove()." );
    EndAction( bIdleEnd );
    if( !--nCrsrMove )
        bInCMvVisportChgd = sal_False;
}

#endif


sal_Bool SwCrsrShell::LeftRight( sal_Bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
                                 sal_Bool bVisualAllowed )
{
    if( IsTableMode() )
        return bLeft ? GoPrevCell() : GoNextCell();

    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    sal_Bool bRet = sal_False;

    // #i27615# Handle cursor in front of label.
    const SwTxtNode* pTxtNd = 0;

    if( pBlockCrsr )
        pBlockCrsr->clearPoints();

    //
    // 1. CASE: Cursor is in front of label. A move to the right
    // will simply reset the bInFrontOfLabel flag:
    //
    SwShellCrsr* pShellCrsr = getShellCrsr( true );
    if ( !bLeft && pShellCrsr->IsInFrontOfLabel() )
    {
        SetInFrontOfLabel( sal_False );
        bRet = sal_True;
    }
    //
    // 2. CASE: Cursor is at beginning of numbered paragraph. A move
    // to the left will simply set the bInFrontOfLabel flag:
    //
    else if ( bLeft && 0 == pShellCrsr->GetPoint()->nContent.GetIndex() &&
             !pShellCrsr->IsInFrontOfLabel() && !pShellCrsr->HasMark() &&
             0 != ( pTxtNd = pShellCrsr->GetNode()->GetTxtNode() ) &&
             pTxtNd->HasVisibleNumberingOrBullet() )
    {
        SetInFrontOfLabel( sal_True );
        bRet = sal_True;
    }
    //
    // 3. CASE: Regular cursor move. Reset the bInFrontOfLabel flag:
    //
    else
    {
        const sal_Bool bSkipHidden = !GetViewOptions()->IsShowHiddenChar();
        // --> OD 2009-12-30 #i107447#
        // To avoid loop the reset of <bInFrontOfLabel> flag is no longer
        // reflected in the return value <bRet>.
        const bool bResetOfInFrontOfLabel = SetInFrontOfLabel( sal_False );
        bRet = pShellCrsr->LeftRight( bLeft, nCnt, nMode, bVisualAllowed,
                                      bSkipHidden, !IsOverwriteCrsr() );
        if ( !bRet && bLeft && bResetOfInFrontOfLabel )
        {
            // undo reset of <bInFrontOfLabel> flag
            SetInFrontOfLabel( sal_True );
        }
        // <--
    }

    if( bRet )
    {
        UpdateCrsr();
    }
    return bRet;
}

// --> OD 2008-04-02 #refactorlists#
void SwCrsrShell::MarkListLevel( const String& sListId,
                                 const int nListLevel )
{
    if ( sListId != sMarkedListId ||
         nListLevel != nMarkedListLevel)
    {
        if ( sMarkedListId.Len() > 0 )
            pDoc->MarkListLevel( sMarkedListId, nMarkedListLevel, sal_False );

        if ( sListId.Len() > 0 )
        {
            pDoc->MarkListLevel( sListId, nListLevel, sal_True );
        }

        sMarkedListId = sListId;
        nMarkedListLevel = nListLevel;
    }
}

void SwCrsrShell::UpdateMarkedListLevel()
{
    SwTxtNode * pTxtNd = _GetCrsr()->GetNode()->GetTxtNode();

    if ( pTxtNd )
    {
        if ( !pTxtNd->IsNumbered() )
        {
            pCurCrsr->_SetInFrontOfLabel( sal_False );
            MarkListLevel( String(), 0 );
        }
        else if ( pCurCrsr->IsInFrontOfLabel() )
        {
            if ( pTxtNd->IsInList() )
            {
                ASSERT( pTxtNd->GetActualListLevel() >= 0 &&
                        pTxtNd->GetActualListLevel() < MAXLEVEL, "Which level?")
                MarkListLevel( pTxtNd->GetListId(),
                               pTxtNd->GetActualListLevel() );
            }
        }
        else
        {
            MarkListLevel( String(), 0 );
        }
    }
}
// <--

sal_Bool SwCrsrShell::UpDown( sal_Bool bUp, sal_uInt16 nCnt )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen

    sal_Bool bTableMode = IsTableMode();
    SwShellCrsr* pTmpCrsr = getShellCrsr( true );

    sal_Bool bRet = pTmpCrsr->UpDown( bUp, nCnt );
    // --> FME 2005-01-10 #i40019# UpDown should always reset the
    // bInFrontOfLabel flag:
    bRet = SetInFrontOfLabel(sal_False) || bRet;
    // <--

    if( pBlockCrsr )
        pBlockCrsr->clearPoints();

    if( bRet )
    {
        eMvState = MV_UPDOWN;       // Status fuers Crsr-Travelling - GetCrsrOfst
        if( !ActionPend() )
        {
            CrsrFlag eUpdtMode = SwCrsrShell::SCROLLWIN;
            if( !bTableMode )
                eUpdtMode = (CrsrFlag) (eUpdtMode
                            | SwCrsrShell::UPDOWN | SwCrsrShell::CHKRANGE);
            UpdateCrsr( static_cast<sal_uInt16>(eUpdtMode) );
        }
    }
    return bRet;
}


sal_Bool SwCrsrShell::LRMargin( sal_Bool bLeft, sal_Bool bAPI)
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SET_CURR_SHELL( this );
    eMvState = MV_LEFTMARGIN;       // Status fuers Crsr-Travelling - GetCrsrOfst

    const sal_Bool bTableMode = IsTableMode();
    SwShellCrsr* pTmpCrsr = getShellCrsr( true );

    if( pBlockCrsr )
        pBlockCrsr->clearPoints();

    const sal_Bool bWasAtLM =
            ( 0 == _GetCrsr()->GetPoint()->nContent.GetIndex() );

    sal_Bool bRet = pTmpCrsr->LeftRightMargin( bLeft, bAPI );

    if ( bLeft && !bTableMode && bRet && bWasAtLM && !_GetCrsr()->HasMark() )
    {
        const SwTxtNode * pTxtNd = _GetCrsr()->GetNode()->GetTxtNode();
        if ( pTxtNd && pTxtNd->HasVisibleNumberingOrBullet() )
            SetInFrontOfLabel( sal_True );
    }
    else if ( !bLeft )
    {
        bRet = SetInFrontOfLabel( sal_False ) || bRet;
    }

    if( bRet )
    {
        UpdateCrsr();
    }
    return bRet;
}

sal_Bool SwCrsrShell::IsAtLRMargin( sal_Bool bLeft, sal_Bool bAPI ) const
{
    const SwShellCrsr* pTmpCrsr = getShellCrsr( true );
    return pTmpCrsr->IsAtLeftRightMargin( bLeft, bAPI );
}


sal_Bool SwCrsrShell::SttEndDoc( sal_Bool bStt )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen

    SwShellCrsr* pTmpCrsr = pBlockCrsr ? &pBlockCrsr->getShellCrsr() : pCurCrsr;
    sal_Bool bRet = pTmpCrsr->SttEndDoc( bStt );
    if( bRet )
    {
        if( bStt )
            pTmpCrsr->GetPtPos().Y() = 0;       // expl. 0 setzen (TabellenHeader)
        if( pBlockCrsr )
        {
            pBlockCrsr->clearPoints();
            RefreshBlockCursor();
        }

        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

void SwCrsrShell::ExtendedSelectAll()
{
    SwNodes& rNodes = GetDoc()->GetNodes();
    SwPosition* pPos = pCurCrsr->GetPoint();
    pPos->nNode = rNodes.GetEndOfPostIts();
    pPos->nContent.Assign( rNodes.GoNext( &pPos->nNode ), 0 );
    pPos = pCurCrsr->GetMark();
    pPos->nNode = rNodes.GetEndOfContent();
    SwCntntNode* pCNd = rNodes.GoPrevious( &pPos->nNode );
    pPos->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );
}

sal_Bool SwCrsrShell::MovePage( SwWhichPage fnWhichPage, SwPosPage fnPosPage )
{
    sal_Bool bRet = sal_False;

    // Springe beim Selektieren nie ueber Section-Grenzen !!
    if( !pCurCrsr->HasMark() || !pCurCrsr->IsNoCntnt() )
    {
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
        SET_CURR_SHELL( this );

        SwCrsrSaveState aSaveState( *pCurCrsr );
        Point& rPt = pCurCrsr->GetPtPos();
        SwCntntFrm * pFrm = pCurCrsr->GetCntntNode()->
                            GetFrm( &rPt, pCurCrsr->GetPoint() );
        if( pFrm && sal_True == ( bRet = GetFrmInPage( pFrm, fnWhichPage,
                                                fnPosPage, pCurCrsr )  ) &&
            !pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                 nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ))
            UpdateCrsr();
        else
            bRet = sal_False;
    }
    return bRet;
}


sal_Bool SwCrsrShell::MovePara(SwWhichPara fnWhichPara, SwPosPara fnPosPara )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCursor* pTmpCrsr = getShellCrsr( true );
    sal_Bool bRet = pTmpCrsr->MovePara( fnWhichPara, fnPosPara );
    if( bRet )
        UpdateCrsr();
    return bRet;
}


sal_Bool SwCrsrShell::MoveSection( SwWhichSection fnWhichSect,
                                SwPosSection fnPosSect)
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCursor* pTmpCrsr = getShellCrsr( true );
    sal_Bool bRet = pTmpCrsr->MoveSection( fnWhichSect, fnPosSect );
    if( bRet )
        UpdateCrsr();
    return bRet;

}


// Positionieren des Cursors


SwFrm* lcl_IsInHeaderFooter( const SwNodeIndex& rIdx, Point& rPt )
{
    SwFrm* pFrm = 0;
    SwCntntNode* pCNd = rIdx.GetNode().GetCntntNode();
    if( pCNd )
    {
        pFrm = pCNd->GetFrm( &rPt, 0, sal_False )->GetUpper();
        while( pFrm && !pFrm->IsHeaderFrm() && !pFrm->IsFooterFrm() )
            pFrm = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->AnchorFrm()
                                    : pFrm->GetUpper();
    }
    return pFrm;
}

sal_Bool SwCrsrShell::IsInHeaderFooter( sal_Bool* pbInHeader ) const
{
    Point aPt;
    SwFrm* pFrm = ::lcl_IsInHeaderFooter( pCurCrsr->GetPoint()->nNode, aPt );
    if( pFrm && pbInHeader )
        *pbInHeader = pFrm->IsHeaderFrm();
    return 0 != pFrm;
}

int SwCrsrShell::SetCrsr( const Point &rLPt, sal_Bool bOnlyText, bool bBlock )
{
    SET_CURR_SHELL( this );

    SwShellCrsr* pCrsr = getShellCrsr( bBlock );
    SwPosition aPos( *pCrsr->GetPoint() );
    Point aPt( rLPt );
    Point & rAktCrsrPt = pCrsr->GetPtPos();
    SwCrsrMoveState aTmpState( IsTableMode() ? MV_TBLSEL :
                                    bOnlyText ?  MV_SETONLYTEXT : MV_NONE );
    aTmpState.bSetInReadOnly = IsReadOnlyAvailable();

    SwTxtNode * pTxtNd = pCrsr->GetNode()->GetTxtNode();

    if ( pTxtNd && !IsTableMode() &&
        // --> FME 2004-11-25 #i37515# No bInFrontOfLabel during selection
        !pCrsr->HasMark() &&
        // <--
        pTxtNd->HasVisibleNumberingOrBullet() )
    {
        aTmpState.bInFrontOfLabel = sal_True; // #i27615#
    }
    else
    {
        aTmpState.bInFrontOfLabel = sal_False;
    }

    int bRet = CRSR_POSOLD |
                ( GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState )
                    ? 0 : CRSR_POSCHG );

    const bool bOldInFrontOfLabel = IsInFrontOfLabel();
    const bool bNewInFrontOfLabel = aTmpState.bInFrontOfLabel;

    pCrsr->SetCrsrBidiLevel( aTmpState.nCursorBidiLevel );

    if( MV_RIGHTMARGIN == aTmpState.eState )
        eMvState = MV_RIGHTMARGIN;
    // steht neu Pos im Header/Footer ?
    SwFrm* pFrm = lcl_IsInHeaderFooter( aPos.nNode, aPt );
    if( IsTableMode() && !pFrm && aPos.nNode.GetNode().StartOfSectionNode() ==
        pCrsr->GetPoint()->nNode.GetNode().StartOfSectionNode() )
        // gleiche Tabellenzelle und nicht im Header/Footer
        // -> zurueck
        return bRet;

    if( pBlockCrsr && bBlock )
    {
        pBlockCrsr->setEndPoint( rLPt );
        if( !pCrsr->HasMark() )
            pBlockCrsr->setStartPoint( rLPt );
        else if( !pBlockCrsr->getStartPoint() )
            pBlockCrsr->setStartPoint( pCrsr->GetMkPos() );
    }
    if( !pCrsr->HasMark() )
    {
        // steht an der gleichen Position und wenn im Header/Footer,
        // dann im gleichen
        if( aPos == *pCrsr->GetPoint() &&
            bOldInFrontOfLabel == bNewInFrontOfLabel )
        {
            if( pFrm )
            {
                if( pFrm->Frm().IsInside( rAktCrsrPt ))
                    return bRet;
            }
            else if( aPos.nNode.GetNode().IsCntntNode() )
            {
                // im gleichen Frame gelandet?
                SwFrm* pOld = ((SwCntntNode&)aPos.nNode.GetNode()).GetFrm(
                                        &aCharRect.Pos(), 0, sal_False );
                SwFrm* pNew = ((SwCntntNode&)aPos.nNode.GetNode()).GetFrm(
                                        &aPt, 0, sal_False );
                if( pNew == pOld )
                    return bRet;
            }
        }
    }
    else
    {
        // SSelection ueber nicht erlaubte Sections oder wenn im Header/Footer
        // dann in verschiedene
        if( !CheckNodesRange( aPos.nNode, pCrsr->GetMark()->nNode, sal_True )
            || ( pFrm && !pFrm->Frm().IsInside( pCrsr->GetMkPos() ) ))
            return bRet;

        // steht an der gleichen Position und nicht im Header/Footer
        if( aPos == *pCrsr->GetPoint() )
            return bRet;
    }

    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCrsr );

    *pCrsr->GetPoint() = aPos;
    rAktCrsrPt = aPt;

    // --> FME 2005-01-31 #i41424# Only update the marked number levels if necessary
    // Force update of marked number levels if necessary.
    if ( bNewInFrontOfLabel || bOldInFrontOfLabel )
        pCurCrsr->_SetInFrontOfLabel( !bNewInFrontOfLabel );
    SetInFrontOfLabel( bNewInFrontOfLabel );
    // <--

    if( !pCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ) )
    {
        sal_uInt16 nFlag = SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE;
        UpdateCrsr( nFlag );
        bRet &= ~CRSR_POSOLD;
    }
    else if( bOnlyText && !pCurCrsr->HasMark() )
    {
        if( FindValidCntntNode( bOnlyText ) )
        {
            // Cursor in einen gueltigen Content stellen
            if( aPos == *pCrsr->GetPoint() )
                bRet = CRSR_POSOLD;
            else
            {
                UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE );
                bRet &= ~CRSR_POSOLD;
            }
        }
        else
        {
            // es gibt keinen gueltigen Inhalt -> Cursor verstecken
            pVisCrsr->Hide();       // sichtbaren Cursor immer verstecken
            eMvState = MV_NONE;     // Status fuers Crsr-Travelling
            bAllProtect = sal_True;
            if( GetDoc()->GetDocShell() )
            {
                GetDoc()->GetDocShell()->SetReadOnlyUI( sal_True );
                CallChgLnk();           // UI bescheid sagen!
            }
        }
    }

    return bRet;
}


void SwCrsrShell::TblCrsrToCursor()
{
    ASSERT( pTblCrsr, "TblCrsrToCursor: Why?" );
    delete pTblCrsr, pTblCrsr = 0;
}

void SwCrsrShell::BlockCrsrToCrsr()
{
    ASSERT( pBlockCrsr, "BlockCrsrToCrsr: Why?" );
    if( pBlockCrsr && !HasSelection() )
    {
        SwPaM& rPam = pBlockCrsr->getShellCrsr();
        pCurCrsr->SetMark();
        *pCurCrsr->GetPoint() = *rPam.GetPoint();
        if( rPam.HasMark() )
            *pCurCrsr->GetMark() = *rPam.GetMark();
        else
            pCurCrsr->DeleteMark();
    }
    delete pBlockCrsr, pBlockCrsr = 0;
}

void SwCrsrShell::CrsrToBlockCrsr()
{
    if( !pBlockCrsr )
    {
        SwPosition aPos( *pCurCrsr->GetPoint() );
        pBlockCrsr = createBlockCursor( *this, aPos );
        SwShellCrsr &rBlock = pBlockCrsr->getShellCrsr();
        rBlock.GetPtPos() = pCurCrsr->GetPtPos();
        if( pCurCrsr->HasMark() )
        {
            rBlock.SetMark();
            *rBlock.GetMark() = *pCurCrsr->GetMark();
            rBlock.GetMkPos() = pCurCrsr->GetMkPos();
        }
    }
    pBlockCrsr->clearPoints();
    RefreshBlockCursor();
}

void SwCrsrShell::ClearMark()
{
    // ist ueberhaupt ein GetMark gesetzt ?
    if( pTblCrsr )
    {
        while( pCurCrsr->GetNext() != pCurCrsr )
            delete pCurCrsr->GetNext();
        pTblCrsr->DeleteMark();

        if( pCurCrsr->HasMark() )
        {
            // falls doch nicht alle Indizies richtig verschoben werden
            //  (z.B.: Kopf-/Fusszeile loeschen) den Content-Anteil vom
            //  Mark aufs Nodes-Array setzen
            SwPosition& rPos = *pCurCrsr->GetMark();
            rPos.nNode.Assign( pDoc->GetNodes(), 0 );
            rPos.nContent.Assign( 0, 0 );
            pCurCrsr->DeleteMark();
        }

        *pCurCrsr->GetPoint() = *pTblCrsr->GetPoint();
        pCurCrsr->GetPtPos() = pTblCrsr->GetPtPos();
        delete pTblCrsr, pTblCrsr = 0;
        pCurCrsr->SwSelPaintRects::Show();
    }
    else
    {
        if( !pCurCrsr->HasMark() )
            return;
        // falls doch nicht alle Indizies richtig verschoben werden
        //  (z.B.: Kopf-/Fusszeile loeschen) den Content-Anteil vom
        //  Mark aufs Nodes-Array setzen
        SwPosition& rPos = *pCurCrsr->GetMark();
        rPos.nNode.Assign( pDoc->GetNodes(), 0 );
        rPos.nContent.Assign( 0, 0 );
        pCurCrsr->DeleteMark();
        if( !nCrsrMove )
            pCurCrsr->SwSelPaintRects::Show();
    }
}


void SwCrsrShell::NormalizePam(sal_Bool bPointFirst)
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    pCurCrsr->Normalize(bPointFirst);
}

void SwCrsrShell::SwapPam()
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    pCurCrsr->Exchange();
}


// suche innerhalb der Selektierten-Bereiche nach einer Selektion, die
// den angebenen SPoint umschliesst
// Ist das Flag bTstOnly gesetzt, dann wird nur getestet, ob dort eine
// SSelection besteht; des akt. Cursr wird nicht umgesetzt!
// Ansonsten wird er auf die gewaehlte SSelection gesetzt.


sal_Bool SwCrsrShell::ChgCurrPam( const Point & rPt,
                              sal_Bool bTstOnly, sal_Bool bTstHit )
{
    SET_CURR_SHELL( this );

    // Pruefe ob der SPoint in einer Tabellen-Selektion liegt
    if( bTstOnly && pTblCrsr )
        return pTblCrsr->IsInside( rPt );

    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    // Suche die Position rPt im Dokument
    SwPosition aPtPos( *pCurCrsr->GetPoint() );
    Point aPt( rPt );

    SwCrsrMoveState aTmpState( MV_NONE );
    aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
    if ( !GetLayout()->GetCrsrOfst( &aPtPos, aPt, &aTmpState ) && bTstHit )
        return sal_False;

    // suche in allen Selektionen nach dieser Position
    SwShellCrsr* pCmp = (SwShellCrsr*)pCurCrsr;        // sicher den Pointer auf Cursor
    do {
        if( pCmp->HasMark() &&
            *pCmp->Start() <= aPtPos && *pCmp->End() > aPtPos )
        {
            if( bTstOnly || pCurCrsr == pCmp )     // ist der aktuelle.
                return sal_True;                       // return ohne Update

            pCurCrsr = pCmp;
            UpdateCrsr();     // Cursor steht schon richtig
            return sal_True;
        }
    } while( pCurCrsr !=
        ( pCmp = dynamic_cast<SwShellCrsr*>(pCmp->GetNext()) ) );
    return sal_False;
}


void SwCrsrShell::KillPams()
{
    // keiner zum loeschen vorhanden?
    if( !pTblCrsr && !pBlockCrsr && pCurCrsr->GetNext() == pCurCrsr )
        return;

    while( pCurCrsr->GetNext() != pCurCrsr )
        delete pCurCrsr->GetNext();
    pCurCrsr->SetColumnSelection( false );

    if( pTblCrsr )
    {
        // Cursor Ring loeschen
        pCurCrsr->DeleteMark();
        *pCurCrsr->GetPoint() = *pTblCrsr->GetPoint();
        pCurCrsr->GetPtPos() = pTblCrsr->GetPtPos();
        delete pTblCrsr;
        pTblCrsr = 0;
    }
    else if( pBlockCrsr )
    {
        // delete the ring of cursors
        pCurCrsr->DeleteMark();
        SwShellCrsr &rBlock = pBlockCrsr->getShellCrsr();
        *pCurCrsr->GetPoint() = *rBlock.GetPoint();
        pCurCrsr->GetPtPos() = rBlock.GetPtPos();
        rBlock.DeleteMark();
        pBlockCrsr->clearPoints();
    }
    UpdateCrsr( SwCrsrShell::SCROLLWIN );
}


int SwCrsrShell::CompareCursor( CrsrCompareType eType ) const
{
    int nRet = 0;
    const SwPosition *pFirst = 0, *pSecond = 0;
    const SwPaM *pCur = GetCrsr(), *pStk = pCrsrStk;
    if( CurrPtCurrMk != eType && pStk )
    {
        switch ( eType)
        {
        case StackPtStackMk:
            pFirst = pStk->GetPoint();
            pSecond = pStk->GetMark();
            break;
        case StackPtCurrPt:
            pFirst = pStk->GetPoint();
            pSecond = pCur->GetPoint();
            break;
        case StackPtCurrMk:
            pFirst = pStk->GetPoint();
            pSecond = pCur->GetMark();
            break;
        case StackMkCurrPt:
            pFirst = pStk->GetMark();
            pSecond = pCur->GetPoint();
            break;
        case StackMkCurrMk:
            pFirst = pStk->GetMark();
            pSecond = pStk->GetMark();
            break;
        case CurrPtCurrMk:
            pFirst = pCur->GetPoint();
            pSecond = pCur->GetMark();
            break;
        }
    }
    if( !pFirst || !pSecond )
        nRet = INT_MAX;
    else if( *pFirst < *pSecond )
        nRet = -1;
    else if( *pFirst == *pSecond )
        nRet = 0;
    else
        nRet = 1;
    return nRet;
}


sal_Bool SwCrsrShell::IsSttPara() const
{   return( pCurCrsr->GetPoint()->nContent == 0 ? sal_True : sal_False ); }


sal_Bool SwCrsrShell::IsEndPara() const
{   return( pCurCrsr->GetPoint()->nContent == pCurCrsr->GetCntntNode()->Len() ? sal_True : sal_False ); }


sal_Bool SwCrsrShell::IsInFrontOfLabel() const
{
    return pCurCrsr->IsInFrontOfLabel();
}

bool SwCrsrShell::SetInFrontOfLabel( sal_Bool bNew )
{
    if ( bNew != IsInFrontOfLabel() )
    {
        pCurCrsr->_SetInFrontOfLabel( bNew );
        UpdateMarkedListLevel();
        return true;
    }
    return false;
}

sal_Bool SwCrsrShell::GotoPage( sal_uInt16 nPage )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCurCrsr );
    sal_Bool bRet = GetLayout()->SetCurrPage( pCurCrsr, nPage ) &&
                    !pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                         nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}


void SwCrsrShell::GetPageNum( sal_uInt16 &rnPhyNum, sal_uInt16 &rnVirtNum,
                              sal_Bool bAtCrsrPos, const sal_Bool bCalcFrm )
{
    SET_CURR_SHELL( this );
    // Seitennummer: die erste sichtbare Seite oder die am Cursor
    const SwCntntFrm* pCFrm;
    const SwPageFrm *pPg = 0;

    if( !bAtCrsrPos || 0 == (pCFrm = GetCurrFrm( bCalcFrm )) ||
                       0 == (pPg   = pCFrm->FindPageFrm()) )
    {
        pPg = Imp()->GetFirstVisPage();
        while( pPg && pPg->IsEmptyPage() )
            pPg = (const SwPageFrm *)pPg->GetNext();
    }
    // Abfrage auf pPg muss fuer den Sonderfall Writerstart mit
    // standard.vor sein.
    rnPhyNum  = pPg? pPg->GetPhyPageNum() : 1;
    rnVirtNum = pPg? pPg->GetVirtPageNum() : 1;
}


sal_uInt16 SwCrsrShell::GetNextPrevPageNum( sal_Bool bNext )
{
    SET_CURR_SHELL( this );

    // Seitennummer: die erste sichtbare Seite oder die am Cursor
    const SwPageFrm *pPg = Imp()->GetFirstVisPage();
    if( pPg )
    {
        const SwTwips nPageTop = pPg->Frm().Top();

        if( bNext )
        {
            // go to next view layout row:
            do
            {
                pPg = (const SwPageFrm *)pPg->GetNext();
            }
            while( pPg && pPg->Frm().Top() == nPageTop );

            while( pPg && pPg->IsEmptyPage() )
                pPg = (const SwPageFrm *)pPg->GetNext();
        }
        else
        {
            // go to previous view layout row:
            do
            {
                pPg = (const SwPageFrm *)pPg->GetPrev();
            }
            while( pPg && pPg->Frm().Top() == nPageTop );

            while( pPg && pPg->IsEmptyPage() )
                pPg = (const SwPageFrm *)pPg->GetPrev();
        }
    }
    // Abfrage auf pPg muss fuer den Sonderfall Writerstart mit
    // standard.vor sein.
    return pPg ? pPg->GetPhyPageNum() : USHRT_MAX;
}


sal_uInt16 SwCrsrShell::GetPageCnt()
{
    SET_CURR_SHELL( this );
    // gebe die Anzahl der Seiten zurueck
    return GetLayout()->GetPageNum();
}

// Gehe zur naechsten SSelection


sal_Bool SwCrsrShell::GoNextCrsr()
{
    // besteht ueberhaupt ein Ring ?
    if( pCurCrsr->GetNext() == pCurCrsr )
        return sal_False;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    pCurCrsr = dynamic_cast<SwShellCrsr*>(pCurCrsr->GetNext());

    // Bug 24086: auch alle anderen anzeigen
    if( !ActionPend() )
    {
        UpdateCrsr();
        pCurCrsr->Show();
    }
    return sal_True;
}

// gehe zur vorherigen SSelection


sal_Bool SwCrsrShell::GoPrevCrsr()
{
    // besteht ueberhaupt ein Ring ?
    if( pCurCrsr->GetNext() == pCurCrsr )
        return sal_False;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    pCurCrsr = dynamic_cast<SwShellCrsr*>(pCurCrsr->GetPrev());

    // Bug 24086: auch alle anderen anzeigen
    if( !ActionPend() )
    {
        UpdateCrsr();
        pCurCrsr->Show();
    }

    return sal_True;
}


void SwCrsrShell::Paint( const Rectangle &rRect)
{
    SET_CURR_SHELL( this );

    // beim Painten immer alle Cursor ausschalten
    SwRect aRect( rRect );

    sal_Bool bVis = sal_False;
    // ist Cursor sichtbar, dann verstecke den SV-Cursor
    if( pVisCrsr->IsVisible() && !aRect.IsOver( aCharRect ) )   //JP 18.06.97: ???
    {
        bVis = sal_True;
        pVisCrsr->Hide();
    }

    // Bereich neu painten
    ViewShell::Paint( rRect );

    if( bHasFocus && !bBasicHideCrsr )
    {
        SwShellCrsr* pAktCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;
//      pAktCrsr->Invalidate( aRect );
        if( !ActionPend() )
        {
            // damit nicht rechts/unten die Raender abgeschnitten werden
            pAktCrsr->Invalidate( VisArea() );
            pAktCrsr->Show();
        }
        else
            pAktCrsr->Invalidate( aRect );

    }
    if( bSVCrsrVis && bVis )        // auch SV-Cursor wieder anzeigen
        pVisCrsr->Show();
}



void SwCrsrShell::VisPortChgd( const SwRect & rRect )
{
    SET_CURR_SHELL( this );
    sal_Bool bVis;      // beim Scrollen immer alle Cursor ausschalten

    // ist Cursor sichtbar, dann verstecke den SV-Cursor
    if( sal_True == ( bVis = pVisCrsr->IsVisible() ))
        pVisCrsr->Hide();

    bVisPortChgd = sal_True;
    aOldRBPos.X() = VisArea().Right();
    aOldRBPos.Y() = VisArea().Bottom();

    //Damit es es keine Probleme mit dem SV-Cursor gibt, wird in
    //ViewShell::VisPo.. ein Update() auf das Window gerufen.
    //Waehrend des Paintens duerfen aber nun wieder keine Selectionen
    //angezeigt werden, deshalb wird der Aufruf hier geklammert.
    ViewShell::VisPortChgd( rRect );        // Bereich verschieben

/*
    SwRect aRect( rRect );
    if( VisArea().IsOver( aRect ) )
        pCurCrsr->Invalidate( aRect );
*/

    if( bSVCrsrVis && bVis )    // auch SV-Cursor wieder anzeigen
        pVisCrsr->Show();

    if( nCrsrMove )
        bInCMvVisportChgd = sal_True;

    bVisPortChgd = sal_False;
}

// aktualisiere den Crsrs, d.H. setze ihn wieder in den Content.
// Das sollte nur aufgerufen werden, wenn der Cursor z.B. beim
// Loeschen von Rahmen irgendwohin gesetzt wurde. Die Position
// ergibt sich aus seiner aktuellen Position im Layout !!


void SwCrsrShell::UpdateCrsrPos()
{
    SET_CURR_SHELL( this );
    ++nStartAction;
    SwShellCrsr* pShellCrsr = getShellCrsr( true );
    Size aOldSz( GetDocSize() );
    SwCntntNode *pCNode = pShellCrsr->GetCntntNode();
    SwCntntFrm  *pFrm = pCNode ?
        pCNode->GetFrm( &pShellCrsr->GetPtPos(), pShellCrsr->GetPoint() ) :0;
    if( !pFrm || (pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow()) )
    {
        SwCrsrMoveState aTmpState( MV_NONE );
        aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
        GetLayout()->GetCrsrOfst( pShellCrsr->GetPoint(), pShellCrsr->GetPtPos(),
                                     &aTmpState );
        if( pShellCrsr->HasMark())
            pShellCrsr->DeleteMark();
    }
    IGrammarContact *pGrammarContact = GetDoc() ? GetDoc()->getGrammarContact() : 0;
    if( pGrammarContact )
        pGrammarContact->updateCursorPosition( *pCurCrsr->GetPoint() );
    --nStartAction;
    if( aOldSz != GetDocSize() )
        SizeChgNotify();
}

// JP 30.04.99: Bug 65475 - falls Point/Mark in versteckten Bereichen
//              stehen, so mussen diese daraus verschoben werden
static void lcl_CheckHiddenSection( SwNodeIndex& rIdx )
{
    const SwSectionNode* pSectNd = rIdx.GetNode().FindSectionNode();
    if( pSectNd && pSectNd->GetSection().IsHiddenFlag() )
    {
        SwNodeIndex aTmp( *pSectNd );
#if OSL_DEBUG_LEVEL > 1
        const SwNode* pFrmNd =
#endif
        rIdx.GetNodes().FindPrvNxtFrmNode( aTmp, pSectNd->EndOfSectionNode() );

#if OSL_DEBUG_LEVEL > 1
        (void) pFrmNd;
        ASSERT( pFrmNd, "keinen Node mit Frames gefunden" );
#endif
        rIdx = aTmp;
    }
}

// Try to set the cursor to the next visible content node.
static void lcl_CheckHiddenPara( SwPosition& rPos )
{
    SwNodeIndex aTmp( rPos.nNode );
    SwTxtNode* pTxtNd = aTmp.GetNode().GetTxtNode();
    while( pTxtNd && pTxtNd->HasHiddenCharAttribute( true ) )
    {
        SwCntntNode* pCntnt = aTmp.GetNodes().GoNext( &aTmp );
        if ( pCntnt && pCntnt->IsTxtNode() )
            pTxtNd = (SwTxtNode*)pCntnt;
        else
            pTxtNd = 0;
    }

    if ( pTxtNd )
        rPos = SwPosition( aTmp, SwIndex( pTxtNd, 0 ) );
}

// --> OD 2005-12-14 #i27301# - helper class, which notifies the accessibility
// about invalid text selections in its destructor
class SwNotifyAccAboutInvalidTextSelections
{
    private:
        SwCrsrShell& mrCrsrSh;

    public:
        SwNotifyAccAboutInvalidTextSelections( SwCrsrShell& _rCrsrSh )
            : mrCrsrSh( _rCrsrSh )
        {}

        ~SwNotifyAccAboutInvalidTextSelections()
        {
            mrCrsrSh.InvalidateAccessibleParaTextSelection();
        }
};
// <--
void SwCrsrShell::UpdateCrsr( sal_uInt16 eFlags, sal_Bool bIdleEnd )
{
    SET_CURR_SHELL( this );

    ClearUpCrsrs();

    // erfrage den Count fuer die Start-/End-Actions und ob die Shell
    // ueberhaupt den Focus hat
//  if( ActionPend() /*|| !bHasFocus*/ )
    //JP 12.01.98: Bug #46496# - es muss innerhalb einer BasicAction der
    //              Cursor geupdatet werden; um z.B. den TabellenCursor zu
    //              erzeugen. Im EndAction wird jetzt das UpdateCrsr gerufen!
    if( ActionPend() && BasicActionPend() )
    {
        if ( eFlags & SwCrsrShell::READONLY )
            bIgnoreReadonly = sal_True;
        return;             // wenn nicht, dann kein Update !!
    }

    // --> OD 2005-12-14 #i27301#
    SwNotifyAccAboutInvalidTextSelections aInvalidateTextSelections( *this );
    // <--

    if ( bIgnoreReadonly )
    {
        bIgnoreReadonly = sal_False;
        eFlags |= SwCrsrShell::READONLY;
    }

    if( eFlags & SwCrsrShell::CHKRANGE )    // alle Cursor-Bewegungen auf
        CheckRange( pCurCrsr );         // ueberlappende Bereiche testen

    if( !bIdleEnd )
        CheckTblBoxCntnt();

    // steht der akt. Crsr in einer Tabelle und in unterschiedlichen Boxen
    // (oder ist noch TabellenMode), dann gilt der Tabellen Mode
    SwPaM* pTstCrsr = getShellCrsr( true );
    if( pTstCrsr->HasMark() && !pBlockCrsr &&
        pDoc->IsIdxInTbl( pTstCrsr->GetPoint()->nNode ) &&
          ( pTblCrsr ||
            pTstCrsr->GetNode( sal_True )->StartOfSectionNode() !=
            pTstCrsr->GetNode( sal_False )->StartOfSectionNode() ) )
    {
        SwShellCrsr* pITmpCrsr = getShellCrsr( true );
        Point aTmpPt( pITmpCrsr->GetPtPos() );
        Point aTmpMk( pITmpCrsr->GetMkPos() );
        SwPosition* pPos = pITmpCrsr->GetPoint();

        // JP 30.04.99: Bug 65475 - falls Point/Mark in versteckten Bereichen
        //              stehen, so mussen diese daraus verschoben werden
        lcl_CheckHiddenSection( pPos->nNode );
        lcl_CheckHiddenSection( pITmpCrsr->GetMark()->nNode );

        // Move cursor out of hidden paragraphs
        if ( !GetViewOptions()->IsShowHiddenChar() )
        {
            lcl_CheckHiddenPara( *pPos );
            lcl_CheckHiddenPara( *pITmpCrsr->GetMark() );
        }

        SwCntntFrm *pTblFrm = pPos->nNode.GetNode().GetCntntNode()->
                                                GetFrm( &aTmpPt, pPos );

        ASSERT( pTblFrm, "Tabelle Crsr nicht im Content ??" );

        // --> FME 2005-12-02 #126107# Make code robust. The table
        // cursor may point to a table in a currently inactive header.
        SwTabFrm *pTab = pTblFrm ? pTblFrm->FindTabFrm() : 0;
        // <--

        if ( pTab && pTab->GetTable()->GetRowsToRepeat() > 0 )
        {
            // First check if point is in repeated headline:
            bool bInRepeatedHeadline = pTab->IsFollow() && pTab->IsInHeadline( *pTblFrm );

            // Second check if mark is in repeated headline:
            if ( !bInRepeatedHeadline )
            {
                SwCntntFrm* pMarkTblFrm = pITmpCrsr->GetCntntNode( sal_False )->GetFrm( &aTmpMk, pITmpCrsr->GetMark() );
                ASSERT( pMarkTblFrm, "Tabelle Crsr nicht im Content ??" );

                if ( pMarkTblFrm )
                {
                    SwTabFrm* pMarkTab = pMarkTblFrm->FindTabFrm();
                    ASSERT( pMarkTab, "Tabelle Crsr nicht im Content ??" );

                    // --> FME 2005-11-28 #120360# Make code robust:
                    if ( pMarkTab )
                    {
                        bInRepeatedHeadline = pMarkTab->IsFollow() && pMarkTab->IsInHeadline( *pMarkTblFrm );
                    }
                    // <--
                }
            }

            // No table cursor in repeaded headlines:
            if ( bInRepeatedHeadline )
            {
                pTblFrm = 0;

                SwPosSection fnPosSect = *pPos <  *pITmpCrsr->GetMark()
                                            ? fnSectionStart
                                            : fnSectionEnd;

                // dann nur innerhalb der Box selektieren
                if( pTblCrsr )
                {
                    pCurCrsr->SetMark();
                    *pCurCrsr->GetMark() = *pTblCrsr->GetMark();
                    pCurCrsr->GetMkPos() = pTblCrsr->GetMkPos();
                    pTblCrsr->DeleteMark();
                    pTblCrsr->SwSelPaintRects::Hide();
                }

                *pCurCrsr->GetPoint() = *pCurCrsr->GetMark();
                (*fnSectionCurr)( *pCurCrsr, fnPosSect );
            }
        }

        // wir wollen wirklich eine Tabellen-Selektion
        if( pTab && pTblFrm )
        {
            if( !pTblCrsr )
            {
                pTblCrsr = new SwShellTableCrsr( *this,
                                *pCurCrsr->GetMark(), pCurCrsr->GetMkPos(),
                                *pPos, aTmpPt );
                pCurCrsr->DeleteMark();
                pCurCrsr->SwSelPaintRects::Hide();

                CheckTblBoxCntnt();
            }

            SwCrsrMoveState aTmpState( MV_NONE );
            aTmpState.bRealHeight = sal_True;
            if( !pTblFrm->GetCharRect( aCharRect, *pTblCrsr->GetPoint(), &aTmpState ) )
            {
                Point aCentrPt( aCharRect.Center() );
                aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
                pTblFrm->GetCrsrOfst( pTblCrsr->GetPoint(), aCentrPt, &aTmpState );
#ifndef DBG_UTIL
                pTblFrm->GetCharRect( aCharRect, *pTblCrsr->GetPoint() );
#else
                if ( !pTblFrm->GetCharRect( aCharRect, *pTblCrsr->GetPoint() ) )
                    ASSERT( !this, "GetCharRect failed." );
#endif
            }
//          ALIGNRECT( aCharRect );

            pVisCrsr->Hide();       // sichtbaren Cursor immer verstecken
            // Curosr in den sichtbaren Bereich scrollen
            if( (eFlags & SwCrsrShell::SCROLLWIN) &&
                (HasSelection() || eFlags & SwCrsrShell::READONLY ||
                 !IsCrsrReadonly()) )
            {
                SwFrm* pBoxFrm = pTblFrm;
                while( pBoxFrm && !pBoxFrm->IsCellFrm() )
                    pBoxFrm = pBoxFrm->GetUpper();
                if( pBoxFrm && pBoxFrm->Frm().HasArea() )
                    MakeVisible( pBoxFrm->Frm() );
                else
                    MakeVisible( aCharRect );
            }

            // lasse vom Layout die Crsr in den Boxen erzeugen
            if( pTblCrsr->IsCrsrMovedUpdt() )
                GetLayout()->MakeTblCrsrs( *pTblCrsr );
            if( bHasFocus && !bBasicHideCrsr )
                pTblCrsr->Show();

            // Cursor-Points auf die neuen Positionen setzen
            pTblCrsr->GetPtPos().X() = aCharRect.Left();
            pTblCrsr->GetPtPos().Y() = aCharRect.Top();

            if( bSVCrsrVis )
            {
                aCrsrHeight.X() = 0;
                aCrsrHeight.Y() = aTmpState.aRealHeight.Y() < 0 ?
                                  -aCharRect.Width() : aCharRect.Height();
                pVisCrsr->Show();           // wieder anzeigen
            }
            eMvState = MV_NONE;     // Status fuers Crsr-Travelling - GetCrsrOfst
            if( pTblFrm && Imp()->IsAccessible() )
                Imp()->InvalidateAccessibleCursorPosition( pTblFrm );
            return;
        }
    }

    if( pTblCrsr )
    {
        // Cursor Ring loeschen
        while( pCurCrsr->GetNext() != pCurCrsr )
            delete pCurCrsr->GetNext();
        pCurCrsr->DeleteMark();
        *pCurCrsr->GetPoint() = *pTblCrsr->GetPoint();
        pCurCrsr->GetPtPos() = pTblCrsr->GetPtPos();
        delete pTblCrsr, pTblCrsr = 0;
    }

    pVisCrsr->Hide();       // sichtbaren Cursor immer verstecken

    // sind wir vielleicht in einer geschuetzten/versteckten Section ?
    {
        SwShellCrsr* pShellCrsr = getShellCrsr( true );
        sal_Bool bChgState = sal_True;
        const SwSectionNode* pSectNd = pShellCrsr->GetNode()->FindSectionNode();
        if( pSectNd && ( pSectNd->GetSection().IsHiddenFlag() ||
            ( !IsReadOnlyAvailable() &&
              pSectNd->GetSection().IsProtectFlag() &&
             ( !pDoc->GetDocShell() ||
               !pDoc->GetDocShell()->IsReadOnly() || bAllProtect )) ) )
        {
            if( !FindValidCntntNode( !HasDrawView() ||
                    0 == Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount()))
            {
                // alles ist geschuetzt / versteckt -> besonderer Mode
                if( bAllProtect && !IsReadOnlyAvailable() &&
                    pSectNd->GetSection().IsProtectFlag() )
                    bChgState = sal_False;
                else
                {
                    eMvState = MV_NONE;     // Status fuers Crsr-Travelling
                    bAllProtect = sal_True;
                    if( GetDoc()->GetDocShell() )
                    {
                        GetDoc()->GetDocShell()->SetReadOnlyUI( sal_True );
                        CallChgLnk();       // UI bescheid sagen!
                    }
                    return;
                }
            }
        }
        if( bChgState )
        {
            sal_Bool bWasAllProtect = bAllProtect;
            bAllProtect = sal_False;
            if( bWasAllProtect && GetDoc()->GetDocShell() &&
                GetDoc()->GetDocShell()->IsReadOnlyUI() )
            {
                GetDoc()->GetDocShell()->SetReadOnlyUI( sal_False );
                CallChgLnk();       // UI bescheid sagen!
            }
        }
    }

    UpdateCrsrPos();

    // #100722# The cursor must always point into content; there's some code
    // that relies on this. (E.g. in SwEditShell::GetScriptType, which always
    // loops _behind_ the last node in the selection, which always works if you
    // are in content.) To achieve this, we'll force cursor(s) to point into
    // content, if UpdateCrsrPos() hasn't already done so.
    SwPaM* pCmp = pCurCrsr;
    do
    {
        // start will move forwards, end will move backwards
        bool bPointIsStart = ( pCmp->Start() == pCmp->GetPoint() );

        // move point; forward if it's the start, backwards if it's the end
        if( ! pCmp->GetPoint()->nNode.GetNode().IsCntntNode() )
            pCmp->Move( bPointIsStart ? fnMoveForward : fnMoveBackward,
                        fnGoCntnt );

        // move mark (if exists); forward if it's the start, else backwards
        if( pCmp->HasMark() )
        {
            if( ! pCmp->GetMark()->nNode.GetNode().IsCntntNode() )
            {
                pCmp->Exchange();
                pCmp->Move( !bPointIsStart ? fnMoveForward : fnMoveBackward,
                            fnGoCntnt );
                pCmp->Exchange();
            }
        }

        // iterate to next PaM in ring
        pCmp = static_cast<SwPaM*>( pCmp->GetNext() );
    }
    while( pCmp != pCurCrsr );


    SwRect aOld( aCharRect );
    sal_Bool bFirst = sal_True;
    SwCntntFrm *pFrm;
    int nLoopCnt = 100;
    SwShellCrsr* pShellCrsr = getShellCrsr( true );

    do {
        sal_Bool bAgainst;
        do {
            bAgainst = sal_False;
            pFrm = pShellCrsr->GetCntntNode()->GetFrm(
                        &pShellCrsr->GetPtPos(), pShellCrsr->GetPoint() );
            // ist der Frm nicht mehr vorhanden, dann muss das gesamte Layout
            // erzeugt werden, weil ja mal hier einer vorhanden war !!
            if ( !pFrm )
            {
                do
                {
                    CalcLayout();
                    pFrm = pShellCrsr->GetCntntNode()->GetFrm(
                                &pShellCrsr->GetPtPos(), pShellCrsr->GetPoint() );
                }  while( !pFrm );
            }
            else if ( Imp()->IsIdleAction() )
                //Wir stellen sicher, dass anstaendig Formatiert wurde #42224#
                pFrm->PrepareCrsr();

            // im geschuetzten Fly? aber bei Rahmenselektion ignorieren
            if( !IsReadOnlyAvailable() && pFrm->IsProtected() &&
                ( !Imp()->GetDrawView() ||
                  !Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() ) &&
                (!pDoc->GetDocShell() ||
                 !pDoc->GetDocShell()->IsReadOnly() || bAllProtect ) )
            {
                // dann suche eine gueltige Position
                sal_Bool bChgState = sal_True;
                if( !FindValidCntntNode(!HasDrawView() ||
                    0 == Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount()))
                {
                    // alles ist geschuetzt / versteckt -> besonderer Mode
                    if( bAllProtect )
                        bChgState = sal_False;
                    else
                    {
                        eMvState = MV_NONE;     // Status fuers Crsr-Travelling
                        bAllProtect = sal_True;
                        if( GetDoc()->GetDocShell() )
                        {
                            GetDoc()->GetDocShell()->SetReadOnlyUI( sal_True );
                            CallChgLnk();       // UI bescheid sagen!
                        }
                        return;
                    }
                }

                if( bChgState )
                {
                    sal_Bool bWasAllProtect = bAllProtect;
                    bAllProtect = sal_False;
                    if( bWasAllProtect && GetDoc()->GetDocShell() &&
                        GetDoc()->GetDocShell()->IsReadOnlyUI() )
                    {
                        GetDoc()->GetDocShell()->SetReadOnlyUI( sal_False );
                        CallChgLnk();       // UI bescheid sagen!
                    }
                    bAllProtect = sal_False;
                    bAgainst = sal_True;        // nochmal den richigen Frm suchen
                }
            }
        } while( bAgainst );

        if( !( eFlags & SwCrsrShell::NOCALRECT ))
        {
            SwCrsrMoveState aTmpState( eMvState );
            aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
            aTmpState.bRealHeight = sal_True;
            aTmpState.bRealWidth = IsOverwriteCrsr();
            aTmpState.nCursorBidiLevel = pShellCrsr->GetCrsrBidiLevel();

            // #i27615#,#i30453#
            SwSpecialPos aSpecialPos;
            aSpecialPos.nExtendRange = SP_EXTEND_RANGE_BEFORE;
            if (pShellCrsr->IsInFrontOfLabel())
            {
                aTmpState.pSpecialPos = &aSpecialPos;
            }

            if( !pFrm->GetCharRect( aCharRect, *pShellCrsr->GetPoint(), &aTmpState ) )
            {
                Point& rPt = pShellCrsr->GetPtPos();
                rPt = aCharRect.Center();
                pFrm->GetCrsrOfst( pShellCrsr->GetPoint(), rPt, &aTmpState );
            }
//          ALIGNRECT( aCharRect );

            if( !pShellCrsr->HasMark() )
                aCrsrHeight = aTmpState.aRealHeight;
            else
            {
                aCrsrHeight.X() = 0;
                aCrsrHeight.Y() = aTmpState.aRealHeight.Y() < 0 ?
                                  -aCharRect.Width() : aCharRect.Height();
            }
        }
        else
        {
            aCrsrHeight.X() = 0;
            aCrsrHeight.Y() = aCharRect.Height();
        }

        if( !bFirst && aOld == aCharRect )
            break;

        // falls das Layout meint, nach dem 100 durchlauf ist man immer noch
        // im Fluss, sollte man die akt. Pos. als gegeben hinnehmen!
        // siehe Bug: 29658
        if( !--nLoopCnt )
        {
            ASSERT( !this, "Endlosschleife? CharRect != OldCharRect ");
            break;
        }
        aOld = aCharRect;
        bFirst = sal_False;

        // Cursor-Points auf die neuen Positionen setzen
        pShellCrsr->GetPtPos().X() = aCharRect.Left();
        pShellCrsr->GetPtPos().Y() = aCharRect.Top();

        if( !(eFlags & SwCrsrShell::UPDOWN ))   // alte Pos. von Up/Down loeschen
        {
            pFrm->Calc();
            nUpDownX = pFrm->IsVertical() ?
                       aCharRect.Top() - pFrm->Frm().Top() :
                       aCharRect.Left() - pFrm->Frm().Left();
        }

        // Curosr in den sichtbaren Bereich scrollen
        if( bHasFocus && eFlags & SwCrsrShell::SCROLLWIN &&
            (HasSelection() || eFlags & SwCrsrShell::READONLY ||
             !IsCrsrReadonly() || GetViewOptions()->IsSelectionInReadonly()) )
        {
            //JP 30.04.99:  damit das EndAction, beim evtuellen Scrollen, den
            //      SV-Crsr nicht wieder sichtbar macht, wird hier das Flag
            //      gesichert und zurueckgesetzt.
            sal_Bool bSav = bSVCrsrVis; bSVCrsrVis = sal_False;
            MakeSelVisible();
            bSVCrsrVis = bSav;
        }

    } while( eFlags & SwCrsrShell::SCROLLWIN );

    if( pBlockCrsr )
        RefreshBlockCursor();

    if( !bIdleEnd && bHasFocus && !bBasicHideCrsr )
    {
        if( pTblCrsr )
            pTblCrsr->SwSelPaintRects::Show();
        else
        {
            pCurCrsr->SwSelPaintRects::Show();
            if( pBlockCrsr )
            {
                SwShellCrsr* pNxt = dynamic_cast<SwShellCrsr*>(pCurCrsr->GetNext());
                while( pNxt && pNxt != pCurCrsr )
                {
                    pNxt->SwSelPaintRects::Show();
                    pNxt = dynamic_cast<SwShellCrsr*>(pNxt->GetNext());
                }
            }
        }
    }

    eMvState = MV_NONE;     // Status fuers Crsr-Travelling - GetCrsrOfst

    if( pFrm && Imp()->IsAccessible() )
        Imp()->InvalidateAccessibleCursorPosition( pFrm );

    // switch from blinking cursor to read-only-text-selection cursor
    static const long nNoBlinkTime = STYLE_CURSOR_NOBLINKTIME;
    const long nBlinkTime = GetOut()->GetSettings().GetStyleSettings().
                            GetCursorBlinkTime();

    if ( (IsCrsrReadonly() && GetViewOptions()->IsSelectionInReadonly()) ==
        ( nBlinkTime != nNoBlinkTime ) )
    {
        // non blinking cursor in read only - text selection mode
        AllSettings aSettings = GetOut()->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        const long nNewBlinkTime = nBlinkTime == nNoBlinkTime ?
                                   Application::GetSettings().GetStyleSettings().GetCursorBlinkTime() :
                                   nNoBlinkTime;
        aStyleSettings.SetCursorBlinkTime( nNewBlinkTime );
        aSettings.SetStyleSettings( aStyleSettings );
        GetOut()->SetSettings( aSettings );
    }

    if( bSVCrsrVis )
        pVisCrsr->Show();           // wieder anzeigen
}

void SwCrsrShell::RefreshBlockCursor()
{
    ASSERT( pBlockCrsr, "Don't call me without a block cursor" );
    SwShellCrsr &rBlock = pBlockCrsr->getShellCrsr();
    Point aPt = rBlock.GetPtPos();
    SwCntntFrm* pFrm = rBlock.GetCntntNode()->GetFrm( &aPt, rBlock.GetPoint() );
    Point aMk;
    if( pBlockCrsr->getEndPoint() && pBlockCrsr->getStartPoint() )
    {
        aPt = *pBlockCrsr->getStartPoint();
        aMk = *pBlockCrsr->getEndPoint();
    }
    else
    {
        aPt = rBlock.GetPtPos();
        if( pFrm )
        {
            if( pFrm->IsVertical() )
                aPt.Y() = pFrm->Frm().Top() + GetUpDownX();
            else
                aPt.X() = pFrm->Frm().Left() + GetUpDownX();
        }
        aMk = rBlock.GetMkPos();
    }
    SwRect aRect( aMk, aPt );
    aRect.Justify();
    SwSelectionList aSelList( pFrm );

    if( GetLayout()->FillSelection( aSelList, aRect ) )
    {
        SwCursor* pNxt = (SwCursor*)pCurCrsr->GetNext();
        while( pNxt != pCurCrsr )
        {
            delete pNxt;
            pNxt = (SwCursor*)pCurCrsr->GetNext();
        }

        std::list<SwPaM*>::iterator pStart = aSelList.getStart();
        std::list<SwPaM*>::iterator pPam = aSelList.getEnd();
        ASSERT( pPam != pStart, "FillSelection should deliver at least one PaM" )
        pCurCrsr->SetMark();
        --pPam;
        // If there is only one text portion inside the rectangle, a simple
        // selection is created
        if( pPam == pStart )
        {
            *pCurCrsr->GetPoint() = *(*pPam)->GetPoint();
            if( (*pPam)->HasMark() )
                *pCurCrsr->GetMark() = *(*pPam)->GetMark();
            else
                pCurCrsr->DeleteMark();
            delete *pPam;
            pCurCrsr->SetColumnSelection( false );
        }
        else
        {
            // The order of the SwSelectionList has to be preserved but
            // the order inside the ring created by CreateCrsr() is not like
            // exspected => First create the selections before the last one
            // downto the first selection.
            // At least create the cursor for the last selection
            --pPam;
            *pCurCrsr->GetPoint() = *(*pPam)->GetPoint(); // n-1 (if n == number of selections)
            if( (*pPam)->HasMark() )
                *pCurCrsr->GetMark() = *(*pPam)->GetMark();
            else
                pCurCrsr->DeleteMark();
            delete *pPam;
            pCurCrsr->SetColumnSelection( true );
            while( pPam != pStart )
            {
                --pPam;

                SwShellCrsr* pNew = new SwShellCrsr( *pCurCrsr );
                pNew->Insert( pCurCrsr, 0 );
                pCurCrsr->Remove( 0, pCurCrsr->Count() );
                pCurCrsr->DeleteMark();

                *pCurCrsr->GetPoint() = *(*pPam)->GetPoint(); // n-2, n-3, .., 2, 1
                if( (*pPam)->HasMark() )
                {
                    pCurCrsr->SetMark();
                    *pCurCrsr->GetMark() = *(*pPam)->GetMark();
                }
                else
                    pCurCrsr->DeleteMark();
                pCurCrsr->SetColumnSelection( true );
                delete *pPam;
            }
            {
                SwShellCrsr* pNew = new SwShellCrsr( *pCurCrsr );
                pNew->Insert( pCurCrsr, 0 );
                pCurCrsr->Remove( 0, pCurCrsr->Count() );
                pCurCrsr->DeleteMark();
            }
            pPam = aSelList.getEnd();
            --pPam;
            *pCurCrsr->GetPoint() = *(*pPam)->GetPoint(); // n, the last selection
            if( (*pPam)->HasMark() )
            {
                pCurCrsr->SetMark();
                *pCurCrsr->GetMark() = *(*pPam)->GetMark();
            }
            else
                pCurCrsr->DeleteMark();
            pCurCrsr->SetColumnSelection( true );
            delete *pPam;
        }
    }
}

// erzeuge eine Kopie vom Cursor und speicher diese im Stack


void SwCrsrShell::Push()
{
    pCrsrStk = new SwShellCrsr( *this, *pCurCrsr->GetPoint(),
                                    pCurCrsr->GetPtPos(), pCrsrStk );

    if( pCurCrsr->HasMark() )
    {
        pCrsrStk->SetMark();
        *pCrsrStk->GetMark() = *pCurCrsr->GetMark();
    }
}

/*
 *  Loescht einen Cursor (gesteuert durch bOldCrsr)
 *      - vom Stack oder    ( bOldCrsr = sal_True )
 *      - den aktuellen und der auf dem Stack stehende wird zum aktuellen
 *
 *  Return:  es war auf dem Stack noch einer vorhanden
 */


sal_Bool SwCrsrShell::Pop( sal_Bool bOldCrsr )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen

    // noch weitere vorhanden ?
    if( 0 == pCrsrStk )
        return sal_False;

    SwShellCrsr *pTmp = 0, *pOldStk = pCrsrStk;

    // der Nachfolger wird der Aktuelle
    if( pCrsrStk->GetNext() != pCrsrStk )
    {
        pTmp = dynamic_cast<SwShellCrsr*>(pCrsrStk->GetNext());
    }

    if( bOldCrsr )              // loesche vom Stack
        delete pCrsrStk;        //

    pCrsrStk = pTmp;            // neu zuweisen

    if( !bOldCrsr )
    {
        SwCrsrSaveState aSaveState( *pCurCrsr );

        // wurde die sichtbare SSelection nicht veraendert
        if( pOldStk->GetPtPos() == pCurCrsr->GetPtPos() ||
            pOldStk->GetPtPos() == pCurCrsr->GetMkPos() )
        {
            // "Selektions-Rechtecke" verschieben
            pCurCrsr->Insert( pOldStk, 0 );
            pOldStk->Remove( 0, pOldStk->Count() );
        }

        if( pOldStk->HasMark() )
        {
            pCurCrsr->SetMark();
            *pCurCrsr->GetMark() = *pOldStk->GetMark();
            pCurCrsr->GetMkPos() = pOldStk->GetMkPos();
        }
        else
            // keine Selection also alte aufheben und auf die alte Pos setzen
            pCurCrsr->DeleteMark();
        *pCurCrsr->GetPoint() = *pOldStk->GetPoint();
        pCurCrsr->GetPtPos() = pOldStk->GetPtPos();
        delete pOldStk;

        if( !pCurCrsr->IsInProtectTable( sal_True ) &&
            !pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                 nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ) )
            UpdateCrsr();             // akt. Cursor Updaten
    }
    return sal_True;
}

/*
 * Verbinde zwei Cursor miteinander.
 * Loesche vom Stack den obersten und setzen dessen GetMark im Aktuellen.
 */


void SwCrsrShell::Combine()
{
    // noch weitere vorhanden ?
    if( 0 == pCrsrStk )
        return;

    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCurCrsr );
    if( pCrsrStk->HasMark() )           // nur wenn GetMark gesetzt wurde
    {
#ifndef DBG_UTIL
        CheckNodesRange( pCrsrStk->GetMark()->nNode, pCurCrsr->GetPoint()->nNode, sal_True );
#else
        if( !CheckNodesRange( pCrsrStk->GetMark()->nNode, pCurCrsr->GetPoint()->nNode, sal_True ))
            ASSERT( !this, "StackCrsr & akt. Crsr nicht in gleicher Section." );
#endif
        // kopiere das GetMark
        if( !pCurCrsr->HasMark() )
            pCurCrsr->SetMark();
        *pCurCrsr->GetMark() = *pCrsrStk->GetMark();
        pCurCrsr->GetMkPos() = pCrsrStk->GetMkPos();
    }

    SwShellCrsr * pTmp = 0;
    if( pCrsrStk->GetNext() != pCrsrStk )
    {
        pTmp = dynamic_cast<SwShellCrsr*>(pCrsrStk->GetNext());
    }
    delete pCrsrStk;
    pCrsrStk = pTmp;
    if( !pCurCrsr->IsInProtectTable( sal_True ) &&
        !pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                             nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ) )
        UpdateCrsr();             // akt. Cursor Updaten
}


void SwCrsrShell::HideCrsrs()
{
    if( !bHasFocus || bBasicHideCrsr )
        return;

    // ist Cursor sichtbar, dann verstecke den SV-Cursor
    if( pVisCrsr->IsVisible() )
    {
        SET_CURR_SHELL( this );
        pVisCrsr->Hide();
    }
    // hebe die Invertierung der SSelection auf
    SwShellCrsr* pAktCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;
    pAktCrsr->Hide();
}



void SwCrsrShell::ShowCrsrs( sal_Bool bCrsrVis )
{
    if( !bHasFocus || bAllProtect || bBasicHideCrsr )
        return;

    SET_CURR_SHELL( this );
    SwShellCrsr* pAktCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;
    pAktCrsr->Show();

    if( bSVCrsrVis && bCrsrVis )    // auch SV-Cursor wieder anzeigen
        pVisCrsr->Show();
}

// Methoden zum Anzeigen bzw. Verstecken des sichtbaren Text-Cursors


void SwCrsrShell::ShowCrsr()
{
    if( !bBasicHideCrsr )
    {
        bSVCrsrVis = sal_True;
        UpdateCrsr();
    }
}


void SwCrsrShell::HideCrsr()
{
    if( !bBasicHideCrsr )
    {
        bSVCrsrVis = sal_False;
        // evt. die sel. Bereiche aufheben !!
        SET_CURR_SHELL( this );
        pVisCrsr->Hide();
    }
}


void SwCrsrShell::ShLooseFcs()
{
    if( !bBasicHideCrsr )
        HideCrsrs();
    bHasFocus = sal_False;
}


void SwCrsrShell::ShGetFcs( sal_Bool bUpdate )
{
    bHasFocus = sal_True;
    if( !bBasicHideCrsr && VisArea().Width() )
    {
        UpdateCrsr( static_cast<sal_uInt16>( bUpdate ?
                    SwCrsrShell::CHKRANGE|SwCrsrShell::SCROLLWIN
                    : SwCrsrShell::CHKRANGE ) );
        ShowCrsrs( bSVCrsrVis ? sal_True : sal_False );
    }
}

// gebe den aktuellen Frame, in dem der Cursor steht, zurueck

SwCntntFrm *SwCrsrShell::GetCurrFrm( const sal_Bool bCalcFrm ) const
{
    SET_CURR_SHELL( (ViewShell*)this );
    SwCntntFrm *pRet = 0;
    SwCntntNode *pNd = pCurCrsr->GetCntntNode();
    if ( pNd )
    {
        if ( bCalcFrm )
        {
            const sal_uInt16* pST = &nStartAction;
            ++(*((sal_uInt16*)pST));
            const Size aOldSz( GetDocSize() );
            pRet = pNd->GetFrm( &pCurCrsr->GetPtPos(), pCurCrsr->GetPoint() );
            --(*((sal_uInt16*)pST));
            if( aOldSz != GetDocSize() )
                ((SwCrsrShell*)this)->SizeChgNotify();
        }
        else
            pRet = pNd->GetFrm( &pCurCrsr->GetPtPos(), pCurCrsr->GetPoint(), sal_False);
    }
    return pRet;
}


// alle Attribut/Format-Aenderungen am akt. Node werden an den
// Link weitergeleitet.


void SwCrsrShell::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    const sal_uInt16 nWhich = pOld ?
                          pOld->Which() :
                          pNew ?
                          pNew->Which() :
                          sal::static_int_cast<sal_uInt16>(RES_MSG_BEGIN);

    if( bCallChgLnk &&
        ( nWhich < RES_MSG_BEGIN || nWhich >= RES_MSG_END ||
            nWhich == RES_FMT_CHG || nWhich == RES_UPDATE_ATTR ||
            nWhich == RES_ATTRSET_CHG ))
        // die Messages werden nicht weitergemeldet
        //MA 07. Apr. 94 fix(6681): RES_UPDATE_ATTR wird implizit vom
        //SwTxtNode::Insert(SwTxtHint*, sal_uInt16) abgesetzt; hier wird reagiert und
        //vom Insert brauch nicht mehr die Keule RES_FMT_CHG versandt werden.
        CallChgLnk();

    if( aGrfArrivedLnk.IsSet() &&
        ( RES_GRAPHIC_ARRIVED == nWhich || RES_GRAPHIC_SWAPIN == nWhich ))
        aGrfArrivedLnk.Call( this );
}


// Abfrage, ob der aktuelle Cursor eine Selektion aufspannt,
// also, ob GetMark gesetzt und SPoint und GetMark unterschiedlich sind.


sal_Bool SwCrsrShell::HasSelection() const
{
    const SwPaM* pCrsr = getShellCrsr( true );
    return( IsTableMode() || ( pCrsr->HasMark() &&
            *pCrsr->GetPoint() != *pCrsr->GetMark())
        ? sal_True : sal_False );
}


void SwCrsrShell::CallChgLnk()
{
    // innerhalb von Start-/End-Action kein Call, sondern nur merken,
    // das sich etwas geaendert hat. Wird bei EndAction beachtet.
    if( BasicActionPend() )
        bChgCallFlag = sal_True;        // das Change merken
    else if( aChgLnk.IsSet() )
    {
        if( bCallChgLnk )
            aChgLnk.Call( this );
        bChgCallFlag = sal_False;       // Flag zuruecksetzen
    }
}

// returne den am akt.Cursor selektierten Text eines Nodes.


String SwCrsrShell::GetSelTxt() const
{
    String aTxt;
    if( pCurCrsr->GetPoint()->nNode.GetIndex() ==
        pCurCrsr->GetMark()->nNode.GetIndex() )
    {
        SwTxtNode* pTxtNd = pCurCrsr->GetNode()->GetTxtNode();
        if( pTxtNd )
        {
            xub_StrLen nStt = pCurCrsr->Start()->nContent.GetIndex();
            aTxt = pTxtNd->GetExpandTxt( nStt,
                    pCurCrsr->End()->nContent.GetIndex() - nStt );
        }
    }
    return aTxt;
}

// gebe nur den Text ab der akt. Cursor Position zurueck (bis zum NodeEnde)


String SwCrsrShell::GetText() const
{
    String aTxt;
    if( pCurCrsr->GetPoint()->nNode.GetIndex() ==
        pCurCrsr->GetMark()->nNode.GetIndex() )
    {
        SwTxtNode* pTxtNd = pCurCrsr->GetNode()->GetTxtNode();
        if( pTxtNd )
            aTxt = pTxtNd->GetTxt().Copy(
                    pCurCrsr->GetPoint()->nContent.GetIndex() );
    }
    return aTxt;
}

// hole vom Start/Ende der akt. SSelection das nte Zeichen
sal_Unicode SwCrsrShell::GetChar( sal_Bool bEnd, long nOffset )
{
    if( IsTableMode() )         // im TabelleMode nicht moeglich
        return 0;

    const SwPosition* pPos = !pCurCrsr->HasMark() ? pCurCrsr->GetPoint()
                                : bEnd ? pCurCrsr->End() : pCurCrsr->Start();
    SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
    if( !pTxtNd )
        return 0;

    xub_StrLen nPos = pPos->nContent.GetIndex();
    const String& rStr = pTxtNd->GetTxt();
    sal_Unicode cCh = 0;

    if( ((nPos+nOffset) >= 0 ) && (nPos+nOffset) < rStr.Len() )
        cCh = rStr.GetChar( static_cast<xub_StrLen>(nPos+nOffset) );

    return cCh;
}

// erweiter die akt. SSelection am Anfang/Ende um n Zeichen


sal_Bool SwCrsrShell::ExtendSelection( sal_Bool bEnd, xub_StrLen nCount )
{
    if( !pCurCrsr->HasMark() || IsTableMode() )
        return sal_False;           // keine Selektion

    SwPosition* pPos = bEnd ? pCurCrsr->End() : pCurCrsr->Start();
    SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
    ASSERT( pTxtNd, "kein TextNode, wie soll erweitert werden?" );

    xub_StrLen nPos = pPos->nContent.GetIndex();
    if( bEnd )
    {
        if( ( nPos + nCount ) <= pTxtNd->GetTxt().Len() )
            nPos = nPos + nCount;
        else
            return sal_False;       // nicht mehr moeglich
    }
    else if( nPos >= nCount )
        nPos = nPos - nCount;
    else
        return sal_False;           // nicht mehr moeglich

    SwCallLink aLk( *this );    // Crsr-Moves ueberwachen,

    pPos->nContent = nPos;
    UpdateCrsr();

    return sal_True;
}

// setze nur den sichtbaren Cursor an die angegebene Dokument-Pos.
// returnt sal_False: wenn der SPoint vom Layout korrigiert wurde.

sal_Bool SwCrsrShell::SetVisCrsr( const Point &rPt )
{
    SET_CURR_SHELL( this );
    Point aPt( rPt );
    SwPosition aPos( *pCurCrsr->GetPoint() );
    SwCrsrMoveState aTmpState( MV_SETONLYTEXT );
    aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
    aTmpState.bRealHeight = sal_True;

    sal_Bool bRet = GetLayout()->GetCrsrOfst( &aPos, aPt /*, &aTmpState*/ );

    SetInFrontOfLabel( sal_False ); // #i27615#

    // nur in TextNodes anzeigen !!
    SwTxtNode* pTxtNd = aPos.nNode.GetNode().GetTxtNode();
    if( !pTxtNd )
        return sal_False;

    const SwSectionNode* pSectNd = pTxtNd->FindSectionNode();
    if( pSectNd && (pSectNd->GetSection().IsHiddenFlag() ||
                    ( !IsReadOnlyAvailable() &&
                      pSectNd->GetSection().IsProtectFlag())) )
        return sal_False;

    SwCntntFrm *pFrm = pTxtNd->GetFrm( &aPt, &aPos );
    if ( Imp()->IsIdleAction() )
        pFrm->PrepareCrsr();
    SwRect aTmp( aCharRect );

    pFrm->GetCharRect( aCharRect, aPos, &aTmpState );
//  ALIGNRECT( aCharRect );

    if( aTmp == aCharRect &&        // BUG 10137: bleibt der Cursor auf der
        pVisCrsr->IsVisible() )     // Position nicht hidden & showen
        return sal_True;

    pVisCrsr->Hide();       // sichtbaren Cursor immer verstecken
    if( IsScrollMDI( this, aCharRect ))
    {
        MakeVisible( aCharRect );
        pCurCrsr->Show();
    }

    // Bug 29584: bei Rahmenselektion ist der Cursor versteckt, aber den
    //          D&D-Cursor will man trotzdem haben
//  if( bSVCrsrVis )
    {
        if( aTmpState.bRealHeight )
            aCrsrHeight = aTmpState.aRealHeight;
        else
        {
            aCrsrHeight.X() = 0;
            aCrsrHeight.Y() = aCharRect.Height();
        }

        pVisCrsr->SetDragCrsr( sal_True );
        pVisCrsr->Show();           // wieder anzeigen
    }
    return bRet;
}

sal_Bool SwCrsrShell::IsOverReadOnlyPos( const Point& rPt ) const
{
    Point aPt( rPt );
    SwPaM aPam( *pCurCrsr->GetPoint() );
    GetLayout()->GetCrsrOfst( aPam.GetPoint(), aPt );
    // --> FME 2004-06-29 #114856# Formular view
    return aPam.HasReadonlySel( GetViewOptions()->IsFormView() );
    // <--
}


    // returne die Anzahl der Cursor im Ring (Flag besagt ob man nur
    // aufgepspannte haben will - sprich etwas selektiert ist (Basic))
sal_uInt16 SwCrsrShell::GetCrsrCnt( sal_Bool bAll ) const
{
    Ring* pTmp = GetCrsr()->GetNext();
    sal_uInt16 n = (bAll || ( pCurCrsr->HasMark() &&
                    *pCurCrsr->GetPoint() != *pCurCrsr->GetMark())) ? 1 : 0;
    while( pTmp != pCurCrsr )
    {
        if( bAll || ( ((SwPaM*)pTmp)->HasMark() &&
                *((SwPaM*)pTmp)->GetPoint() != *((SwPaM*)pTmp)->GetMark()))
            ++n;
        pTmp = pTmp->GetNext();
    }
    return n;
}


sal_Bool SwCrsrShell::IsStartOfDoc() const
{
    if( pCurCrsr->GetPoint()->nContent.GetIndex() )
        return sal_False;

    // Hinter EndOfIcons kommt die Content-Section (EndNd+StNd+CntntNd)
    SwNodeIndex aIdx( GetDoc()->GetNodes().GetEndOfExtras(), 2 );
    if( !aIdx.GetNode().IsCntntNode() )
        GetDoc()->GetNodes().GoNext( &aIdx );
    return aIdx == pCurCrsr->GetPoint()->nNode;
}


sal_Bool SwCrsrShell::IsEndOfDoc() const
{
    SwNodeIndex aIdx( GetDoc()->GetNodes().GetEndOfContent(), -1 );
    SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = GetDoc()->GetNodes().GoPrevious( &aIdx );

    return aIdx == pCurCrsr->GetPoint()->nNode &&
            pCNd->Len() == pCurCrsr->GetPoint()->nContent.GetIndex();
}


// loesche alle erzeugten Crsr, setze den Tabellen-Crsr und den letzten
// Cursor auf seinen TextNode (oder StartNode?).
// Beim naechsten ::GetCrsr werden sie wieder alle erzeugt
// Wird fuers Drag&Drop / ClipBorad-Paste in Tabellen benoetigt.
sal_Bool SwCrsrShell::ParkTblCrsr()
{
    if( !pTblCrsr )
        return sal_False;

    pTblCrsr->ParkCrsr();

    while( pCurCrsr->GetNext() != pCurCrsr )
        delete pCurCrsr->GetNext();

    // vom Cursor !immer! SPoint und Mark umsetzen
    pCurCrsr->SetMark();
    *pCurCrsr->GetMark() = *pCurCrsr->GetPoint() = *pTblCrsr->GetPoint();
    pCurCrsr->DeleteMark();

    return sal_True;
}

/***********************************************************************
#*  Class       :  SwCrsrShell
#*  Methode     :  ParkCrsr
#*  Beschreibung:  Vernichtet Selektionen und zus. Crsr aller Shell der
#*                 verbleibende Crsr der Shell wird geparkt.
#*  Datum       :  MA 05. Nov. 92
#*  Update      :  JP 19.09.97
#***********************************************************************/

void SwCrsrShell::_ParkPams( SwPaM* pDelRg, SwShellCrsr** ppDelRing )
{
    const SwPosition *pStt = pDelRg->Start(),
        *pEnd = pDelRg->GetPoint() == pStt ? pDelRg->GetMark() : pDelRg->GetPoint();

    SwPaM *pTmpDel = 0, *pTmp = *ppDelRing;

    // durchsuche den gesamten Ring
    sal_Bool bGoNext;
    do {
        const SwPosition *pTmpStt = pTmp->Start(),
                        *pTmpEnd = pTmp->GetPoint() == pTmpStt ?
                                        pTmp->GetMark() : pTmp->GetPoint();
        /*
         * liegt ein SPoint oder GetMark innerhalb vom Crsr-Bereich
         * muss der alte Bereich aufgehoben werden.
         * Beim Vergleich ist darauf zu achten, das End() nicht mehr zum
         * Bereich gehoert !
         */
        if( *pStt <= *pTmpStt )
        {
            if( *pEnd > *pTmpStt ||
                ( *pEnd == *pTmpStt && *pEnd == *pTmpEnd ))
                pTmpDel = pTmp;
        }
        else
            if( *pStt < *pTmpEnd )
                pTmpDel = pTmp;

        bGoNext = sal_True;
        if( pTmpDel )           // ist der Pam im Bereich ?? loesche ihn
        {
            sal_Bool bDelete = sal_True;
            if( *ppDelRing == pTmpDel )
            {
                if( *ppDelRing == pCurCrsr )
                {
                    if( sal_True == ( bDelete = GoNextCrsr() ))
                    {
                        bGoNext = sal_False;
                        pTmp = (SwPaM*)pTmp->GetNext();
                    }
                }
                else
                    bDelete = sal_False;        // StackCrsr nie loeschen !!
            }

            if( bDelete )
                delete pTmpDel;         // hebe alten Bereich auf
            else
            {
                pTmpDel->GetPoint()->nContent.Assign( 0, 0 );
                pTmpDel->GetPoint()->nNode = 0;
                pTmpDel->SetMark();
                pTmpDel->DeleteMark();
            }
            pTmpDel = 0;
        }
        else if( !pTmp->HasMark() )     // sorge auf jedenfall dafuer, das
        {                       // nicht benutzte Indizies beachtet werden!
            pTmp->SetMark();            // SPoint liegt nicht im Bereich,
            pTmp->DeleteMark();         // aber vielleicht GetMark, also setzen
        }
        if( bGoNext )
            pTmp = (SwPaM*)pTmp->GetNext();
    } while( !bGoNext || *ppDelRing != pTmp );
}

void SwCrsrShell::ParkCrsr( const SwNodeIndex &rIdx )
{
    SwNode *pNode = &rIdx.GetNode();

    // erzeuge einen neuen Pam
    SwPaM * pNew = new SwPaM( *GetCrsr()->GetPoint() );
    if( pNode->GetStartNode() )
    {
        if( ( pNode = pNode->StartOfSectionNode())->IsTableNode() )
        {
            // der angegebene Node steht in einer Tabelle, also Parke
            // den Crsr auf dem Tabellen-Node (ausserhalb der Tabelle)
            pNew->GetPoint()->nNode = *pNode->StartOfSectionNode();
        }
        else    // also auf dem StartNode selbst.
                // Dann immer ueber seinen EndNode den StartNode erfragen !!!
                // (StartOfSection vom StartNode ist der Parent !)
            pNew->GetPoint()->nNode = *pNode->EndOfSectionNode()->StartOfSectionNode();
    }
    else
        pNew->GetPoint()->nNode = *pNode->StartOfSectionNode();
    pNew->SetMark();
    pNew->GetPoint()->nNode = *pNode->EndOfSectionNode();

    //Alle Shells wollen etwas davon haben.
    ViewShell *pTmp = this;
    do {
        if( pTmp->IsA( TYPE( SwCrsrShell )))
        {
            SwCrsrShell* pSh = (SwCrsrShell*)pTmp;
            if( pSh->pCrsrStk )
                pSh->_ParkPams( pNew, &pSh->pCrsrStk );

            pSh->_ParkPams( pNew, &pSh->pCurCrsr );
            if( pSh->pTblCrsr )
            {
                // setze den Tabellen Cursor immer auf 0, den aktuellen
                // immer auf den Anfang der Tabelle
                SwPaM* pTCrsr = pSh->GetTblCrs();
                SwNode* pTblNd = pTCrsr->GetPoint()->nNode.GetNode().FindTableNode();
                if ( pTblNd )
                {
                    pTCrsr->GetPoint()->nContent.Assign( 0, 0 );
                    pTCrsr->GetPoint()->nNode = 0;
                    pTCrsr->SetMark();
                    pTCrsr->DeleteMark();
                    pSh->pCurCrsr->GetPoint()->nNode = *pTblNd;
                }
            }
        }
    } while ( this != (pTmp = (ViewShell*)pTmp->GetNext() ));
    delete pNew;
}

//=========================================================================

/*
 * der Copy-Constructor
 * Cursor-Position kopieren, in den Ring eingetragen.
 * Alle Ansichten eines Dokumentes stehen im Ring der Shells.
 */

SwCrsrShell::SwCrsrShell( SwCrsrShell& rShell, Window *pInitWin )
    : ViewShell( rShell, pInitWin ),
    SwModify( 0 ), pCrsrStk( 0 ), pBlockCrsr( 0 ), pTblCrsr( 0 ),
    pBoxIdx( 0 ), pBoxPtr( 0 ), nCrsrMove( 0 ), nBasicActionCnt( 0 ),
    eMvState( MV_NONE ),
    // --> OD 2008-04-02 #refactorlists#
    sMarkedListId(),
    nMarkedListLevel( 0 )
    // <--
{
    SET_CURR_SHELL( this );
    // Nur die Position vom aktuellen Cursor aus der Copy-Shell uebernehmen
    pCurCrsr = new SwShellCrsr( *this, *(rShell.pCurCrsr->GetPoint()) );
    pCurCrsr->GetCntntNode()->Add( this );

    bAllProtect = bVisPortChgd = bChgCallFlag = bInCMvVisportChgd =
    bGCAttr = bIgnoreReadonly = bSelTblCells = bBasicHideCrsr =
    bOverwriteCrsr = sal_False;
    bCallChgLnk = bHasFocus = bSVCrsrVis = bAutoUpdateCells = sal_True;
    bSetCrsrInReadOnly = sal_True;
    pVisCrsr = new SwVisCrsr( this );
//  UpdateCrsr( 0 );
    // OD 11.02.2003 #100556#
    mbMacroExecAllowed = rShell.IsMacroExecAllowed();
}


/*
 * der normale Constructor
 */

SwCrsrShell::SwCrsrShell( SwDoc& rDoc, Window *pInitWin,
                            const SwViewOption *pInitOpt )
    : ViewShell( rDoc, pInitWin, pInitOpt ),
    SwModify( 0 ), pCrsrStk( 0 ), pBlockCrsr( 0 ), pTblCrsr( 0 ),
    pBoxIdx( 0 ), pBoxPtr( 0 ), nCrsrMove( 0 ), nBasicActionCnt( 0 ),
    eMvState( MV_NONE ), // state for crsr-travelling - GetCrsrOfst
    // --> OD 2008-04-02 #refactorlists#
    sMarkedListId(),
    nMarkedListLevel( 0 )
    // <--
{
    SET_CURR_SHELL( this );
    /*
     * Erzeugen des initialen Cursors, wird auf die erste
     * Inhaltsposition gesetzt
     */
    SwNodes& rNds = rDoc.GetNodes();

    SwNodeIndex aNodeIdx( *rNds.GetEndOfContent().StartOfSectionNode() );
    SwCntntNode* pCNd = rNds.GoNext( &aNodeIdx ); // gehe zum 1. ContentNode

    pCurCrsr = new SwShellCrsr( *this, SwPosition( aNodeIdx, SwIndex( pCNd, 0 )));

    // melde die Shell beim akt. Node als abhaengig an, dadurch koennen alle
    // Attribut-Aenderungen ueber den Link weiter gemeldet werden.
    pCNd->Add( this );

    bAllProtect = bVisPortChgd = bChgCallFlag = bInCMvVisportChgd =
    bGCAttr = bIgnoreReadonly = bSelTblCells = bBasicHideCrsr =
    bOverwriteCrsr = sal_False;
    bCallChgLnk = bHasFocus = bSVCrsrVis = bAutoUpdateCells = sal_True;
    bSetCrsrInReadOnly = sal_True;

    pVisCrsr = new SwVisCrsr( this );
//  UpdateCrsr( 0 );
    // OD 11.02.2003 #100556#
    mbMacroExecAllowed = true;
}



SwCrsrShell::~SwCrsrShell()
{
    // wenn es nicht die letzte View so sollte zu mindest das
    // Feld noch geupdatet werden.
    if( GetNext() != this )
        CheckTblBoxCntnt( pCurCrsr->GetPoint() );
    else
        ClearTblBoxCntnt();

    delete pVisCrsr;
    delete pBlockCrsr;
    delete pTblCrsr;

    /*
     * Freigabe der Cursor
     */
    while(pCurCrsr->GetNext() != pCurCrsr)
        delete pCurCrsr->GetNext();
    delete pCurCrsr;

    // Stack freigeben
    if( pCrsrStk )
    {
        while( pCrsrStk->GetNext() != pCrsrStk )
            delete pCrsrStk->GetNext();
        delete pCrsrStk;
    }

    // JP 27.07.98: Bug 54025 - ggfs. den HTML-Parser, der als Client in
    //              der CursorShell haengt keine Chance geben, sich an den
    //              TextNode zu haengen.
    if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );
}

SwShellCrsr* SwCrsrShell::getShellCrsr( bool bBlock )
{
    if( pTblCrsr )
        return pTblCrsr;
    if( pBlockCrsr && bBlock )
        return &pBlockCrsr->getShellCrsr();
    return pCurCrsr;
}

//Sollte fuer das Clipboard der WaitPtr geschaltet werden?
//Warten bei TableMode, Mehrfachselektion und mehr als x Selektieren Absaetzen.

sal_Bool SwCrsrShell::ShouldWait() const
{
    if ( IsTableMode() || GetCrsrCnt() > 1 )
        return sal_True;

    if( HasDrawView() && GetDrawView()->GetMarkedObjectList().GetMarkCount() )
        return sal_True;

    SwPaM* pPam = GetCrsr();
    return pPam->Start()->nNode.GetIndex() + 10 <
            pPam->End()->nNode.GetIndex();
}


sal_uInt16 SwCrsrShell::UpdateTblSelBoxes()
{
    if( pTblCrsr && ( pTblCrsr->IsChgd() || !pTblCrsr->GetBoxesCount() ))
         GetLayout()->MakeTblCrsrs( *pTblCrsr );
    return pTblCrsr ? pTblCrsr->GetBoxesCount() : 0;
}

// zeige das akt. selektierte "Object" an
void SwCrsrShell::MakeSelVisible()
{
    ASSERT( bHasFocus, "kein Focus aber Cursor sichtbar machen?" );
    if( aCrsrHeight.Y() < aCharRect.Height() && aCharRect.Height() > VisArea().Height() )
    {
        SwRect aTmp( aCharRect );
        long nDiff = aCharRect.Height() - VisArea().Height();
        if( nDiff < aCrsrHeight.X() )
            aTmp.Top( nDiff + aCharRect.Top() );
        else
        {
            aTmp.Top( aCrsrHeight.X() + aCharRect.Top() );
            aTmp.Height( aCrsrHeight.Y() );
        }
        if( !aTmp.HasArea() )
        {
            aTmp.SSize().Height() += 1;
            aTmp.SSize().Width() += 1;
        }
        MakeVisible( aTmp );
    }
    else
    {
        if( aCharRect.HasArea() )
            MakeVisible( aCharRect );
        else
        {
            SwRect aTmp( aCharRect );
            aTmp.SSize().Height() += 1; aTmp.SSize().Width() += 1;
            MakeVisible( aTmp );
        }
    }
}


// suche eine gueltige ContentPosition (nicht geschuetzt/nicht versteckt)
sal_Bool SwCrsrShell::FindValidCntntNode( sal_Bool bOnlyText )
{
    if( pTblCrsr )      // was soll ich jetzt machen ??
    {
        ASSERT( !this, "TabellenSelection nicht aufgehoben!" );
        return sal_False;
    }

    //JP 28.10.97: Bug 45129 - im UI-ReadOnly ist alles erlaubt
    if( !bAllProtect && GetDoc()->GetDocShell() &&
        GetDoc()->GetDocShell()->IsReadOnlyUI() )
        return sal_True;

    // dann raus da!
    if( pCurCrsr->HasMark() )
        ClearMark();

    // als erstes mal auf Rahmen abpruefen
    SwNodeIndex& rNdIdx = pCurCrsr->GetPoint()->nNode;
    sal_uLong nNdIdx = rNdIdx.GetIndex();       // sichern
    SwNodes& rNds = pDoc->GetNodes();
    SwCntntNode* pCNd = rNdIdx.GetNode().GetCntntNode();
    const SwCntntFrm * pFrm;

    if( pCNd && 0 != (pFrm = pCNd->GetFrm(0,pCurCrsr->GetPoint(),sal_False)) &&
        !IsReadOnlyAvailable() && pFrm->IsProtected() &&
        nNdIdx < rNds.GetEndOfExtras().GetIndex() )
    {
        // geschuetzter Rahmen ueberspringen
        SwPaM aPam( *pCurCrsr->GetPoint() );
        aPam.SetMark();
        aPam.GetMark()->nNode = rNds.GetEndOfContent();
        aPam.GetPoint()->nNode = *pCNd->EndOfSectionNode();

        sal_Bool bFirst = sal_False;
        if( 0 == (pCNd = ::GetNode( aPam, bFirst, fnMoveForward, sal_False )))
        {
            aPam.GetMark()->nNode = *rNds.GetEndOfPostIts().StartOfSectionNode();
            pCNd = ::GetNode( aPam, bFirst, fnMoveBackward, sal_False );
        }

        if( !pCNd )     // sollte nie passieren !!!
        {
            rNdIdx = nNdIdx;        // alten Node zurueck
            return sal_False;
        }
        *pCurCrsr->GetPoint() = *aPam.GetPoint();
    }
    else if( bOnlyText && pCNd && pCNd->IsNoTxtNode() )
    {
        // dann auf den Anfang vom Doc stellen
        rNdIdx = pDoc->GetNodes().GetEndOfExtras();
        pCurCrsr->GetPoint()->nContent.Assign( pDoc->GetNodes().GoNext(
                                                            &rNdIdx ), 0 );
        nNdIdx = rNdIdx.GetIndex();
    }

    sal_Bool bOk = sal_True;

    // #i9059# cursor may not stand in protected cells
    //         (unless cursor in protected areas is OK.)
    const SwTableNode* pTableNode = rNdIdx.GetNode().FindTableNode();
    if( !IsReadOnlyAvailable()  &&
        pTableNode != NULL  &&  rNdIdx.GetNode().IsProtect() )
    {
        // we're in a table, and we're in a protected area, so we're
        // probably in a protected cell.

        // move forward into non-protected area.
        SwPaM aPam( rNdIdx.GetNode(), 0 );
        while( aPam.GetNode()->IsProtect() &&
               aPam.Move( fnMoveForward, fnGoCntnt ) )
            ; // nothing to do in the loop; the aPam.Move does the moving!

        // didn't work? then go backwards!
        if( aPam.GetNode()->IsProtect() )
        {
            SwPaM aTmpPaM( rNdIdx.GetNode(), 0 );
            aPam = aTmpPaM;
            while( aPam.GetNode()->IsProtect() &&
                   aPam.Move( fnMoveBackward, fnGoCntnt ) )
                ; // nothing to do in the loop; the aPam.Move does the moving!
        }

        // if we're successful, set the new position
        if( ! aPam.GetNode()->IsProtect() )
        {
            *pCurCrsr->GetPoint() = *aPam.GetPoint();
        }
    }

    // in einem geschuetzten Bereich
    const SwSectionNode* pSectNd = rNdIdx.GetNode().FindSectionNode();
    if( pSectNd && ( pSectNd->GetSection().IsHiddenFlag() ||
        ( !IsReadOnlyAvailable() &&
           pSectNd->GetSection().IsProtectFlag() )) )
    {
        typedef SwCntntNode* (SwNodes:: *FNGoSection)( SwNodeIndex *, int, int ) const;
        FNGoSection funcGoSection = &SwNodes::GoNextSection;

        bOk = sal_False;

        for( int nLoopCnt = 0; !bOk && nLoopCnt < 2; ++nLoopCnt )
        {
            sal_Bool bWeiter;
            do {
                bWeiter = sal_False;
                while( 0 != ( pCNd = (rNds.*funcGoSection)( &rNdIdx,
                                            sal_True, !IsReadOnlyAvailable() )) )
                {
                    // in eine Tabelle verschoben -> pruefe ob die
                    // vielleicht geschuetzt ist
                    if( pCNd->FindTableNode() )
                    {
                        SwCallLink aTmp( *this );
                        SwCrsrSaveState aSaveState( *pCurCrsr );
                        aTmp.nNdTyp = 0;        // im DTOR nichts machen!
                        if( !pCurCrsr->IsInProtectTable( sal_True, sal_True ) )
                        {
                            const SwSectionNode* pSNd = pCNd->FindSectionNode();
                            if( !pSNd || !pSNd->GetSection().IsHiddenFlag()
                                || (!IsReadOnlyAvailable()  &&
                                    pSNd->GetSection().IsProtectFlag() ))
                            {
                                bOk = sal_True;
                                break;      // eine nicht geschuetzte Zelle gef.
                            }
                            continue;       // dann weiter suchen
                        }
                    }
                    else
                    {
                        bOk = sal_True;
                        break;      // eine nicht geschuetzte Zelle gef.
                    }
                }

                if( bOk && rNdIdx.GetIndex() < rNds.GetEndOfExtras().GetIndex() )
                {
                    // Teste mal auf Fly - kann auch noch geschuetzt sein!!
                    if( 0 == (pFrm = pCNd->GetFrm(0,0,sal_False)) ||
                        ( !IsReadOnlyAvailable() && pFrm->IsProtected() ) ||
                        ( bOnlyText && pCNd->IsNoTxtNode() ) )
                    {
                        // dann weiter suchen!
                        bOk = sal_False;
                        bWeiter = sal_True;
                    }
                }
            } while( bWeiter );

            if( !bOk )
            {
                if( !nLoopCnt )
                    funcGoSection = &SwNodes::GoPrevSection;
                rNdIdx = nNdIdx;
            }
        }
    }
    if( bOk )
    {
        pCNd = rNdIdx.GetNode().GetCntntNode();
//      sal_uInt16 nCntnt = Min( pCNd->Len(), pCurCrsr->GetPoint()->nContent.GetIndex() );
        xub_StrLen nCntnt = rNdIdx.GetIndex() < nNdIdx ? pCNd->Len() : 0;
        pCurCrsr->GetPoint()->nContent.Assign( pCNd, nCntnt );
    }
    else
    {
        pCNd = rNdIdx.GetNode().GetCntntNode();

        // falls Cursor im versteckten Bereich ist, auf jedenfall schon mal
        // verschieben!!
        if( !pCNd || !pCNd->GetFrm(0,0,sal_False) )
        {
            SwCrsrMoveState aTmpState( MV_NONE );
            aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
            GetLayout()->GetCrsrOfst( pCurCrsr->GetPoint(), pCurCrsr->GetPtPos(),
                                        &aTmpState );
        }
    }
    return bOk;
}


void SwCrsrShell::NewCoreSelection()
{
}


sal_Bool SwCrsrShell::IsCrsrReadonly() const
{
    if ( GetViewOptions()->IsReadonly() ||
         // --> FME 2004-06-29 #114856# Formular view
         GetViewOptions()->IsFormView() )
         // <--
    {
        SwFrm *pFrm = GetCurrFrm( sal_False );
        const SwFlyFrm* pFly;
        const SwSection* pSection;

        if( pFrm && pFrm->IsInFly() &&
             (pFly = pFrm->FindFlyFrm())->GetFmt()->GetEditInReadonly().GetValue() &&
             pFly->Lower() &&
             !pFly->Lower()->IsNoTxtFrm() &&
             !GetDrawView()->GetMarkedObjectList().GetMarkCount() )
        {
            return sal_False;
        }
        // --> FME 2004-06-22 #114856# edit in readonly sections
        else if ( pFrm && pFrm->IsInSct() &&
                  0 != ( pSection = pFrm->FindSctFrm()->GetSection() ) &&
                  pSection->IsEditInReadonlyFlag() )
        {
            return sal_False;
        }
        // <--

        return sal_True;
    }
    return sal_False;
}


// darf der Cursor in ReadOnlyBereiche?
void SwCrsrShell::SetReadOnlyAvailable( sal_Bool bFlag )
{
    // im GlobalDoc darf NIE umgeschaltet werden
    if( (!GetDoc()->GetDocShell() ||
         !GetDoc()->GetDocShell()->IsA( SwGlobalDocShell::StaticType() )) &&
        bFlag != bSetCrsrInReadOnly )
    {
        // wenn das Flag ausgeschaltet wird, dann muessen erstmal alle
        // Selektionen aufgehoben werden. Denn sonst wird sich darauf
        // verlassen, das nichts geschuetztes selektiert ist!
        if( !bFlag )
        {
            ClearMark();
        }
        bSetCrsrInReadOnly = bFlag;
        UpdateCrsr();
    }
}

sal_Bool SwCrsrShell::HasReadonlySel() const
{
    sal_Bool bRet = sal_False;
    if( IsReadOnlyAvailable() ||
        // --> FME 2004-06-29 #114856# Formular view
        GetViewOptions()->IsFormView() )
        // <--
    {
        if( pTblCrsr )
            bRet = pTblCrsr->HasReadOnlyBoxSel() ||
                   pTblCrsr->HasReadonlySel(
                            // --> FME 2004-06-29 #114856# Formular view
                            GetViewOptions()->IsFormView() );
                            // <--
        else
        {
            const SwPaM* pCrsr = pCurCrsr;

            do {
                if( pCrsr->HasReadonlySel(
                        // --> FME 2004-06-29 #114856# Formular view
                        GetViewOptions()->IsFormView() ) )
                        // <--
                    bRet = sal_True;
            } while( !bRet && pCurCrsr != ( pCrsr = (SwPaM*)pCrsr->GetNext() ));
        }
    }
    return bRet;
}

sal_Bool SwCrsrShell::IsSelFullPara() const
{
    sal_Bool bRet = sal_False;

    if( pCurCrsr->GetPoint()->nNode.GetIndex() ==
        pCurCrsr->GetMark()->nNode.GetIndex() && pCurCrsr == pCurCrsr->GetNext() )
    {
        xub_StrLen nStt = pCurCrsr->GetPoint()->nContent.GetIndex(),
                   nEnd = pCurCrsr->GetMark()->nContent.GetIndex();
        if( nStt > nEnd )
        {
            xub_StrLen nTmp = nStt;
            nStt = nEnd;
            nEnd = nTmp;
        }
        const SwCntntNode* pCNd = pCurCrsr->GetCntntNode();
        bRet = pCNd && !nStt && nEnd == pCNd->Len();
    }
    return bRet;
}

short SwCrsrShell::GetTextDirection( const Point* pPt ) const
{
    SwPosition aPos( *pCurCrsr->GetPoint() );
    Point aPt( pPt ? *pPt : pCurCrsr->GetPtPos() );
    if( pPt )
    {
        SwCrsrMoveState aTmpState( MV_NONE );
        aTmpState.bSetInReadOnly = IsReadOnlyAvailable();

        GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState );
    }

    return pDoc->GetTextDirection( aPos, &aPt );
}

sal_Bool SwCrsrShell::IsInVerticalText( const Point* pPt ) const
{
    const short nDir = GetTextDirection( pPt );
    return FRMDIR_VERT_TOP_RIGHT == nDir || FRMDIR_VERT_TOP_LEFT == nDir;
}

sal_Bool SwCrsrShell::IsInRightToLeftText( const Point* pPt ) const
{
    const short nDir = GetTextDirection( pPt );
    // GetTextDirection uses FRMDIR_VERT_TOP_LEFT to indicate RTL in
    // vertical environment
    return FRMDIR_VERT_TOP_LEFT == nDir || FRMDIR_HORI_RIGHT_TOP == nDir;
}

//
// If the current cursor position is inside a hidden range, the hidden range
// is selected:
//
bool SwCrsrShell::SelectHiddenRange()
{
    bool bRet = false;
    if ( !GetViewOptions()->IsShowHiddenChar() && !pCurCrsr->HasMark() )
    {
        SwPosition& rPt = *(SwPosition*)pCurCrsr->GetPoint();
        const SwTxtNode* pNode = rPt.nNode.GetNode().GetTxtNode();
        if ( pNode )
        {
            const xub_StrLen nPos = rPt.nContent.GetIndex();

            // check if nPos is in hidden range
            xub_StrLen nHiddenStart;
            xub_StrLen nHiddenEnd;
            SwScriptInfo::GetBoundsOfHiddenRange( *pNode, nPos, nHiddenStart, nHiddenEnd );
            if ( STRING_LEN != nHiddenStart )
            {
                // make selection:
                pCurCrsr->SetMark();
                pCurCrsr->GetMark()->nContent = nHiddenEnd;
                bRet = true;
            }
        }
    }

    return bRet;
}

/*  */

    // die Suchfunktionen
sal_uLong SwCrsrShell::Find( const SearchOptions& rSearchOpt, sal_Bool bSearchInNotes,
                            SwDocPositions eStart, SwDocPositions eEnde,
                            sal_Bool& bCancel,
                            FindRanges eRng, int bReplace )
{
    if( pTblCrsr )
        GetCrsr();
    delete pTblCrsr, pTblCrsr = 0;
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    sal_uLong nRet = pCurCrsr->Find( rSearchOpt, bSearchInNotes, eStart, eEnde, bCancel, eRng, bReplace );
    if( nRet || bCancel )
        UpdateCrsr();
    return nRet;
}

sal_uLong SwCrsrShell::Find( const SwTxtFmtColl& rFmtColl,
                            SwDocPositions eStart, SwDocPositions eEnde,
                            sal_Bool& bCancel,
                            FindRanges eRng, const SwTxtFmtColl* pReplFmt )
{
    if( pTblCrsr )
        GetCrsr();
    delete pTblCrsr, pTblCrsr = 0;
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    sal_uLong nRet = pCurCrsr->Find( rFmtColl, eStart, eEnde, bCancel, eRng, pReplFmt );
    if( nRet )
        UpdateCrsr();
    return nRet;
}

sal_uLong SwCrsrShell::Find( const SfxItemSet& rSet, sal_Bool bNoCollections,
                            SwDocPositions eStart, SwDocPositions eEnde,
                            sal_Bool& bCancel,
                            FindRanges eRng, const SearchOptions* pSearchOpt,
                            const SfxItemSet* rReplSet )
{
    if( pTblCrsr )
        GetCrsr();
    delete pTblCrsr, pTblCrsr = 0;
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    sal_uLong nRet = pCurCrsr->Find( rSet, bNoCollections, eStart, eEnde, bCancel,
                                eRng, pSearchOpt, rReplSet );
    if( nRet )
        UpdateCrsr();
    return nRet;
}

void SwCrsrShell::SetSelection( const SwPaM& rCrsr )
{
    StartAction();
    SwPaM* pCrsr = GetCrsr();
    *pCrsr->GetPoint() = *rCrsr.GetPoint();
    if(rCrsr.HasMark())
    {
        pCrsr->SetMark();
        *pCrsr->GetMark() = *rCrsr.GetMark();
    }
    if((SwPaM*)rCrsr.GetNext() != &rCrsr)
    {
        const SwPaM *_pStartCrsr = (SwPaM*)rCrsr.GetNext();
        do
        {
            SwPaM* pCurrentCrsr = CreateCrsr();
            *pCurrentCrsr->GetPoint() = *_pStartCrsr->GetPoint();
            if(_pStartCrsr->HasMark())
            {
                pCurrentCrsr->SetMark();
                *pCurrentCrsr->GetMark() = *_pStartCrsr->GetMark();
            }
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != &rCrsr );
    }
    EndAction();
}

void lcl_RemoveMark( SwPaM* pPam )
{
    ASSERT( pPam->HasMark(), "Don't remove pPoint!" )
    pPam->GetMark()->nContent.Assign( 0, 0 );
    pPam->GetMark()->nNode = 0;
    pPam->DeleteMark();
}

const SwStartNode* lcl_NodeContext( const SwNode& rNode )
{
    const SwStartNode *pRet = rNode.StartOfSectionNode();
    while( pRet->IsSectionNode() || pRet->IsTableNode() ||
        pRet->GetStartNodeType() == SwTableBoxStartNode )
    {
        pRet = pRet->StartOfSectionNode();
    }
    return pRet;
}

/**
   Checks if a position is valid. To be valid the position's node must
   be a content node and the content must not be unregistered.

   @param aPos the position to check.
*/
bool lcl_PosOk(const SwPosition & aPos)
{
    return NULL != aPos.nNode.GetNode().GetCntntNode() &&
           SwIndexReg::pEmptyIndexArray != aPos.nContent.GetIdxReg();
}

/**
   Checks if a PaM is valid. For a PaM to be valid its point must be
   valid. Additionaly if the PaM has a mark this has to be valid, too.

   @param aPam the PaM to check
*/
static bool lcl_CrsrOk(SwPaM & aPam)
{
    return lcl_PosOk(*aPam.GetPoint()) && (! aPam.HasMark()
        || lcl_PosOk(*aPam.GetMark()));
}

void SwCrsrShell::ClearUpCrsrs()
{
    // start of the ring
    SwPaM * pStartCrsr = GetCrsr();
    // start loop with second entry of the ring
    SwPaM * pCrsr = (SwPaM *) pStartCrsr->GetNext();
    SwPaM * pTmpCrsr;
    bool bChanged = false;

    /*
       For all entries in the ring except the start entry delete the
       entry if it is invalid.
    */
    while (pCrsr != pStartCrsr)
    {
        pTmpCrsr = (SwPaM *) pCrsr->GetNext();

        if ( ! lcl_CrsrOk(*pCrsr))
        {
            delete pCrsr;

            bChanged = true;
        }

        pCrsr = pTmpCrsr;
    }

    if( pStartCrsr->HasMark() && !lcl_PosOk( *pStartCrsr->GetMark() ) )
    {
        lcl_RemoveMark( pStartCrsr );
        bChanged = true;
    }
    if( !lcl_PosOk( *pStartCrsr->GetPoint() ) )
    {
        SwNodes & aNodes = GetDoc()->GetNodes();
        const SwNode* pStart = lcl_NodeContext( pStartCrsr->GetPoint()->nNode.GetNode() );
        SwNodeIndex aIdx( pStartCrsr->GetPoint()->nNode );
        SwNode * pNode = aNodes.GoPrevious(&aIdx);
        if( pNode == NULL || lcl_NodeContext( *pNode ) != pStart )
            aNodes.GoNext( &aIdx );
        if( pNode == NULL || lcl_NodeContext( *pNode ) != pStart )
        {
            /*
              If the start entry of the ring is invalid replace it with a
              cursor pointing to the beginning of the first content node in
              the document.
            */
            aIdx = (*(aNodes.GetEndOfContent().StartOfSectionNode()));
            pNode = aNodes.GoNext( &aIdx );
        }
        bool bFound = (pNode != NULL);

        ASSERT(bFound, "no content node found");

        if (bFound)
        {
            SwPaM aTmpPam(*pNode);
            *pStartCrsr = aTmpPam;
        }

        bChanged = true;
    }

    /*
      If at least one of the cursors in the ring have been deleted or
      replaced, remove the table cursor.
    */
    if (pTblCrsr != NULL && bChanged)
        TblCrsrToCursor();
}

// #111827#
String SwCrsrShell::GetCrsrDescr() const
{
    String aResult;

    if (IsMultiSelection())
        aResult += String(SW_RES(STR_MULTISEL));
    else
        aResult = GetDoc()->GetPaMDescr(*GetCrsr());

    return aResult;
}

// SMARTTAGS

void lcl_FillRecognizerData( uno::Sequence< rtl::OUString >& rSmartTagTypes,
                             uno::Sequence< uno::Reference< container::XStringKeyMap > >& rStringKeyMaps,
                             const SwWrongList& rSmartTagList, xub_StrLen nCurrent )
{
    // Insert smart tag information
    std::vector< rtl::OUString > aSmartTagTypes;
    std::vector< uno::Reference< container::XStringKeyMap > > aStringKeyMaps;

    for ( sal_uInt16 i = 0; i < rSmartTagList.Count(); ++i )
    {
        const xub_StrLen nSTPos = rSmartTagList.Pos( i );
        const xub_StrLen nSTLen = rSmartTagList.Len( i );

        if ( nSTPos <= nCurrent && nCurrent < nSTPos + nSTLen )
        {
            const SwWrongArea* pArea = rSmartTagList.GetElement( i );
            if ( pArea )
            {
                aSmartTagTypes.push_back( pArea->maType );
                aStringKeyMaps.push_back( pArea->mxPropertyBag );
            }
        }
    }

    if ( aSmartTagTypes.size() )
    {
        rSmartTagTypes.realloc( aSmartTagTypes.size() );
        rStringKeyMaps.realloc( aSmartTagTypes.size() );

        std::vector< rtl::OUString >::const_iterator aTypesIter = aSmartTagTypes.begin();
        sal_uInt16 i = 0;
        for ( aTypesIter = aSmartTagTypes.begin(); aTypesIter != aSmartTagTypes.end(); ++aTypesIter )
            rSmartTagTypes[i++] = *aTypesIter;

        std::vector< uno::Reference< container::XStringKeyMap > >::const_iterator aMapsIter = aStringKeyMaps.begin();
        i = 0;
        for ( aMapsIter = aStringKeyMaps.begin(); aMapsIter != aStringKeyMaps.end(); ++aMapsIter )
            rStringKeyMaps[i++] = *aMapsIter;
    }
}

void lcl_FillTextRange( uno::Reference<text::XTextRange>& rRange,
                   SwTxtNode& rNode, xub_StrLen nBegin, xub_StrLen nLen )
{
    // create SwPosition for nStartIndex
    SwIndex aIndex( &rNode, nBegin );
    SwPosition aStartPos( rNode, aIndex );

    // create SwPosition for nEndIndex
    SwPosition aEndPos( aStartPos );
    aEndPos.nContent = nBegin + nLen;

    const uno::Reference<text::XTextRange> xRange =
        SwXTextRange::CreateXTextRange(*rNode.GetDoc(), aStartPos, &aEndPos);

    rRange = xRange;
}

void SwCrsrShell::GetSmartTagTerm( uno::Sequence< rtl::OUString >& rSmartTagTypes,
                                   uno::Sequence< uno::Reference< container::XStringKeyMap > >& rStringKeyMaps,
                                   uno::Reference< text::XTextRange>& rRange ) const
{
    if ( !SwSmartTagMgr::Get().IsSmartTagsEnabled() )
        return;

    SwPaM* pCrsr = GetCrsr();
    SwPosition aPos( *pCrsr->GetPoint() );
    SwTxtNode *pNode = aPos.nNode.GetNode().GetTxtNode();
    if ( pNode && !pNode->IsInProtectSect() )
    {
        const SwWrongList *pSmartTagList = pNode->GetSmartTags();
        if ( pSmartTagList )
        {
            xub_StrLen nCurrent = aPos.nContent.GetIndex();
            xub_StrLen nBegin = nCurrent;
            xub_StrLen nLen = 1;

            if( pSmartTagList->InWrongWord( nBegin, nLen ) && !pNode->IsSymbol(nBegin) )
            {
                const sal_uInt16 nIndex = pSmartTagList->GetWrongPos( nBegin );
                const SwWrongList* pSubList = pSmartTagList->SubList( nIndex );
                if ( pSubList )
                {
                    pSmartTagList = pSubList;
                    nCurrent = 0;
                }

                lcl_FillRecognizerData( rSmartTagTypes, rStringKeyMaps, *pSmartTagList, nCurrent );
                lcl_FillTextRange( rRange, *pNode, nBegin, nLen );
            }
        }
    }
}

// see also SwEditShell::GetCorrection( const Point* pPt, SwRect& rSelectRect )
void SwCrsrShell::GetSmartTagTerm( const Point& rPt, SwRect& rSelectRect,
                                   uno::Sequence< rtl::OUString >& rSmartTagTypes,
                                   uno::Sequence< uno::Reference< container::XStringKeyMap > >& rStringKeyMaps,
                                   uno::Reference<text::XTextRange>& rRange )
{
    if ( !SwSmartTagMgr::Get().IsSmartTagsEnabled() )
        return;

    SwPaM* pCrsr = GetCrsr();
    SwPosition aPos( *pCrsr->GetPoint() );
    Point aPt( rPt );
    SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
    SwSpecialPos aSpecialPos;
    eTmpState.pSpecialPos = &aSpecialPos;
    SwTxtNode *pNode;
    const SwWrongList *pSmartTagList;

    if( GetLayout()->GetCrsrOfst( &aPos, aPt, &eTmpState ) &&
        0 != (pNode = aPos.nNode.GetNode().GetTxtNode()) &&
        0 != (pSmartTagList = pNode->GetSmartTags()) &&
        !pNode->IsInProtectSect() )
    {
        xub_StrLen nCurrent = aPos.nContent.GetIndex();
        xub_StrLen nBegin = nCurrent;
        xub_StrLen nLen = 1;

        if( pSmartTagList->InWrongWord( nBegin, nLen ) && !pNode->IsSymbol(nBegin) )
        {
            const sal_uInt16 nIndex = pSmartTagList->GetWrongPos( nBegin );
            const SwWrongList* pSubList = pSmartTagList->SubList( nIndex );
            if ( pSubList )
            {
                pSmartTagList = pSubList;
                nCurrent = eTmpState.pSpecialPos->nCharOfst;
            }

            lcl_FillRecognizerData( rSmartTagTypes, rStringKeyMaps, *pSmartTagList, nCurrent );
            lcl_FillTextRange( rRange, *pNode, nBegin, nLen );

            // get smarttag word
            String aText( pNode->GetTxt().Copy( nBegin, nLen ) );

            //save the start and end positons of the line and the starting point
            Push();
            LeftMargin();
            xub_StrLen nLineStart = GetCrsr()->GetPoint()->nContent.GetIndex();
            RightMargin();
            xub_StrLen nLineEnd = GetCrsr()->GetPoint()->nContent.GetIndex();
            Pop(sal_False);

            // make sure the selection build later from the
            // data below does not include footnotes and other
            // "in word" character to the left and right in order
            // to preserve those. Therefore count those "in words"
            // in order to modify the selection accordingly.
            const sal_Unicode* pChar = aText.GetBuffer();
            xub_StrLen nLeft = 0;
            while (pChar && *pChar++ == CH_TXTATR_INWORD)
                ++nLeft;
            pChar = aText.Len() ? aText.GetBuffer() + aText.Len() - 1 : 0;
            xub_StrLen nRight = 0;
            while (pChar && *pChar-- == CH_TXTATR_INWORD)
                ++nRight;

            aPos.nContent = nBegin + nLeft;
            pCrsr = GetCrsr();
            *pCrsr->GetPoint() = aPos;
            pCrsr->SetMark();
            ExtendSelection( sal_True, nLen - nLeft - nRight );
            //no determine the rectangle in the current line
            xub_StrLen nWordStart = (nBegin + nLeft) < nLineStart ? nLineStart : nBegin + nLeft;
            //take one less than the line end - otherwise the next line would be calculated
            xub_StrLen nWordEnd = (nBegin + nLen - nLeft - nRight) > nLineEnd ? nLineEnd - 1: (nBegin + nLen - nLeft - nRight);
            Push();
            pCrsr->DeleteMark();
            SwIndex& rContent = GetCrsr()->GetPoint()->nContent;
            rContent = nWordStart;
            SwRect aStartRect;
            SwCrsrMoveState aState;
            aState.bRealWidth = sal_True;
            SwCntntNode* pCntntNode = pCrsr->GetCntntNode();
            SwCntntFrm *pCntntFrame = pCntntNode->GetFrm( &rPt, pCrsr->GetPoint(), sal_False);

            pCntntFrame->GetCharRect( aStartRect, *pCrsr->GetPoint(), &aState );
            rContent = nWordEnd;
            SwRect aEndRect;
            pCntntFrame->GetCharRect( aEndRect, *pCrsr->GetPoint(),&aState );
            rSelectRect = aStartRect.Union( aEndRect );
            Pop(sal_False);
        }
    }
}

