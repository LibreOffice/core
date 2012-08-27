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
#include <mdiexp.hxx>
#include <fmteiro.hxx>
#include <wrong.hxx>
#include <unotextrange.hxx>
#include <vcl/svapp.hxx>
#include <numrule.hxx>
#include <IGrammarContact.hxx>

#include <globals.hrc>

#include <comcore.hrc>

using namespace com::sun::star;
using namespace util;

TYPEINIT2(SwCrsrShell,ViewShell,SwModify);


/**
 * Delete all overlapping Cursors from a Cursor ring.
 * @param pointer to SwCursor (ring)
 */
void CheckRange( SwCursor* );


/**
 * Check if pCurCrsr points into already existing ranges and delete those.
 * @param Pointer to SwCursor object
 */
void CheckRange( SwCursor* pCurCrsr )
{
    const SwPosition *pStt = pCurCrsr->Start(),
        *pEnd = pCurCrsr->GetPoint() == pStt ? pCurCrsr->GetMark() : pCurCrsr->GetPoint();

    SwPaM *pTmpDel = 0,
          *pTmp = (SwPaM*)pCurCrsr->GetNext();

    // Search the complete ring
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

         // If Point or Mark is within the Crsr range, we need to remove the old
        // range. Take note that Point does not belong to the range anymore.
        pTmp = (SwPaM*)pTmp->GetNext();
        delete pTmpDel;         // Remove old range
        pTmpDel = 0;
    }
}

// -----------
// SwCrsrShell
// -----------




SwPaM * SwCrsrShell::CreateCrsr()
{
    // don't create Crsr in a table Selection (sic!)
    OSL_ENSURE( !IsTableMode(), "in table Selection" );

    // New cursor as copy of current one. Add to the ring.
    // Links point to previously created one, ie forward.
    SwShellCrsr* pNew = new SwShellCrsr( *pCurCrsr );

    // Hide PaM logically, to avoid undoing the inverting from
    // copied PaM (#i75172#)
    pNew->swapContent(*pCurCrsr);

    pCurCrsr->DeleteMark();

    UpdateCrsr( SwCrsrShell::SCROLLWIN );
    return pNew;
}


/**
 * Delete current Cursor, making the following one the current.
 * Note, this function does not delete anything if there is no other cursor.
 * @return - returns sal_True if there was another cursor and we deleted one.
 */
sal_Bool SwCrsrShell::DestroyCrsr()
{
    // don't delete Crsr within table selection
    OSL_ENSURE( !IsTableMode(), "in table Selection" );

    // Is there a next one? Don't do anything if not.
    if(pCurCrsr->GetNext() == pCurCrsr)
        return sal_False;

    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCursor* pNextCrsr = (SwCursor*)pCurCrsr->GetNext();
    delete pCurCrsr;
    pCurCrsr = dynamic_cast<SwShellCrsr*>(pNextCrsr);
    UpdateCrsr();
    return sal_True;
}


/**
 * Create and return a new shell cursor.
 * Simply returns the current shell cursor if there is no selection
 * (HasSelection()).
 */
SwPaM & SwCrsrShell::CreateNewShellCursor()
{
    if (HasSelection())
    {
        (void) CreateCrsr(); // n.b. returns old cursor
    }
    return *GetCrsr();
}

/**
 * Return the current shell cursor
 * @return - returns current `SwPaM` shell cursor
 */
SwPaM & SwCrsrShell::GetCurrentShellCursor()
{
    return *GetCrsr();
}

/**
 * Return pointer to the current shell cursor
 * @return - returns pointer to current `SwPaM` shell cursor
 */
SwPaM* SwCrsrShell::GetCrsr( sal_Bool bMakeTblCrsr ) const
{
    if( pTblCrsr )
    {
        if( bMakeTblCrsr && pTblCrsr->IsCrsrMovedUpdt() )
        {
            //don't re-create 'parked' cursors
            const SwCntntNode* pCNd;
            if( pTblCrsr->GetPoint()->nNode.GetIndex() &&
                pTblCrsr->GetMark()->nNode.GetIndex() &&
                0 != ( pCNd = pTblCrsr->GetCntntNode() ) && pCNd->getLayoutFrm( GetLayout() ) &&
                0 != ( pCNd = pTblCrsr->GetCntntNode(sal_False) ) && pCNd->getLayoutFrm( GetLayout() ) )
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
        // save for update of the ribbon bar
        const SwNode& rNd = pCurCrsr->GetPoint()->nNode.GetNode();
        nAktNode = rNd.GetIndex();
        nAktCntnt = pCurCrsr->GetPoint()->nContent.GetIndex();
        nAktNdTyp = rNd.GetNodeType();
        bAktSelection = *pCurCrsr->GetPoint() != *pCurCrsr->GetMark();
        if( rNd.IsTxtNode() )
            nLeftFrmPos = SwCallLink::getLayoutFrm( GetLayout(), (SwTxtNode&)rNd, nAktCntnt, sal_True );
        else
            nLeftFrmPos = 0;
    }
    ViewShell::StartAction(); // to the ViewShell
}


void SwCrsrShell::EndAction( const sal_Bool bIdleEnd )
{
    sal_Bool bVis = bSVCrsrVis;

    // Idle-formatting?
    if( bIdleEnd && Imp()->GetRegion() )
    {
        pCurCrsr->Hide();
    }

    // Update all invalid numberings before the last action
    if( 1 == nStartAction )
        GetDoc()->UpdateNumRule();

    // #i76923#: Don't show the cursor in the ViewShell::EndAction() - call.
    //           Only the UpdateCrsr shows the cursor.
    sal_Bool bSavSVCrsrVis = bSVCrsrVis;
    bSVCrsrVis = sal_False;

    ViewShell::EndAction( bIdleEnd );   // have ViewShell go first

    bSVCrsrVis = bSavSVCrsrVis;

    if( ActionPend() )
    {
        if( bVis )    // display SV-Cursor again
            pVisCrsr->Show();

        // If there is still a ChgCall and just the "basic
        // parenthiszing(?) (Basic-Klammerung)" exists, call it. This
        // decouples the internal with the Basic-parenthising, the
        // Shells are switched.
        if( !BasicActionPend() )
        {
            // Within a Basic action, one needs to update the cursor,
            // to e.g. create the table cursos. This is being done in
            // UpdateCrsr.
            UpdateCrsr( SwCrsrShell::CHKRANGE, bIdleEnd );

            {
                // watch Crsr-Moves, call Link if needed, the DTOR is key here!
                SwCallLink aLk( *this, nAktNode, nAktCntnt, (sal_uInt8)nAktNdTyp,
                                nLeftFrmPos, bAktSelection );

            }
            if( bCallChgLnk && bChgCallFlag && aChgLnk.IsSet() )
            {
                aChgLnk.Call( this );
                bChgCallFlag = sal_False; // reset flag
            }
        }
        return;
    }

    sal_uInt16 nParm = SwCrsrShell::CHKRANGE;
    if ( !bIdleEnd )
        nParm |= SwCrsrShell::SCROLLWIN;

    UpdateCrsr( nParm, bIdleEnd );      // Show Cursor changes

    {
        SwCallLink aLk( *this );        // watch Crsr-Moves
        aLk.nNode = nAktNode;           // call Link if needed
        aLk.nNdTyp = (sal_uInt8)nAktNdTyp;
        aLk.nCntnt = nAktCntnt;
        aLk.nLeftFrmPos = nLeftFrmPos;

        if( !nCrsrMove ||
            ( 1 == nCrsrMove && bInCMvVisportChgd ) )
            // display Cursor & Selektions again
            ShowCrsrs( bSVCrsrVis ? sal_True : sal_False );
    }
    // call ChgCall if there is still one
    if( bCallChgLnk && bChgCallFlag && aChgLnk.IsSet() )
    {
        aChgLnk.Call( this );
        bChgCallFlag = sal_False;       // reset flag
    }
}


#ifdef DBG_UTIL

void SwCrsrShell::SttCrsrMove()
{
    OSL_ENSURE( nCrsrMove < USHRT_MAX, "To many nested CrsrMoves." );
    ++nCrsrMove;
    StartAction();
}

void SwCrsrShell::EndCrsrMove( const sal_Bool bIdleEnd )
{
    OSL_ENSURE( nCrsrMove, "EndCrsrMove() without SttCrsrMove()." );
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

    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    sal_Bool bRet = sal_False;

    // #i27615# Handle cursor in front of label.
    const SwTxtNode* pTxtNd = 0;

    if( pBlockCrsr )
        pBlockCrsr->clearPoints();

    // 1. CASE: Cursor is in front of label. A move to the right
    // will simply reset the bInFrontOfLabel flag:
    SwShellCrsr* pShellCrsr = getShellCrsr( true );
    if ( !bLeft && pShellCrsr->IsInFrontOfLabel() )
    {
        SetInFrontOfLabel( sal_False );
        bRet = sal_True;
    }
    // 2. CASE: Cursor is at beginning of numbered paragraph. A move
    // to the left will simply set the bInFrontOfLabel flag:
    else if ( bLeft && 0 == pShellCrsr->GetPoint()->nContent.GetIndex() &&
             !pShellCrsr->IsInFrontOfLabel() && !pShellCrsr->HasMark() &&
             0 != ( pTxtNd = pShellCrsr->GetNode()->GetTxtNode() ) &&
             pTxtNd->HasVisibleNumberingOrBullet() )
    {
        SetInFrontOfLabel( sal_True );
        bRet = sal_True;
    }
    // 3. CASE: Regular cursor move. Reset the bInFrontOfLabel flag:
    else
    {
        const sal_Bool bSkipHidden = !GetViewOptions()->IsShowHiddenChar();
        // #i107447#
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
    }

    if( bRet )
    {
        UpdateCrsr();
    }
    return bRet;
}

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
                OSL_ENSURE( pTxtNd->GetActualListLevel() >= 0 &&
                        pTxtNd->GetActualListLevel() < MAXLEVEL, "Which level?");
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

sal_Bool SwCrsrShell::UpDown( sal_Bool bUp, sal_uInt16 nCnt )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed

    sal_Bool bTableMode = IsTableMode();
    SwShellCrsr* pTmpCrsr = getShellCrsr( true );

    sal_Bool bRet = pTmpCrsr->UpDown( bUp, nCnt );
    // #i40019# UpDown should always reset the bInFrontOfLabel flag:
    bRet = SetInFrontOfLabel(sal_False) || bRet;

    if( pBlockCrsr )
        pBlockCrsr->clearPoints();

    if( bRet )
    {
        eMvState = MV_UPDOWN; // status for Crsr travelling - GetCrsrOfst
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
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    SET_CURR_SHELL( this );
    eMvState = MV_LEFTMARGIN; // status for Crsr travelling - GetCrsrOfst

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
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed

    SwShellCrsr* pTmpCrsr = pBlockCrsr ? &pBlockCrsr->getShellCrsr() : pCurCrsr;
    sal_Bool bRet = pTmpCrsr->SttEndDoc( bStt );
    if( bRet )
    {
        if( bStt )
            pTmpCrsr->GetPtPos().Y() = 0; // set to 0 explicitly (table header)
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

    // never jump of section borders at selection
    if( !pCurCrsr->HasMark() || !pCurCrsr->IsNoCntnt() )
    {
        SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
        SET_CURR_SHELL( this );

        SwCrsrSaveState aSaveState( *pCurCrsr );
        Point& rPt = pCurCrsr->GetPtPos();
        SwCntntFrm * pFrm = pCurCrsr->GetCntntNode()->
                            getLayoutFrm( GetLayout(), &rPt, pCurCrsr->GetPoint(), sal_False );
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
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    SwCursor* pTmpCrsr = getShellCrsr( true );
    sal_Bool bRet = pTmpCrsr->MovePara( fnWhichPara, fnPosPara );
    if( bRet )
        UpdateCrsr();
    return bRet;
}


sal_Bool SwCrsrShell::MoveSection( SwWhichSection fnWhichSect,
                                SwPosSection fnPosSect)
{
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    SwCursor* pTmpCrsr = getShellCrsr( true );
    sal_Bool bRet = pTmpCrsr->MoveSection( fnWhichSect, fnPosSect );
    if( bRet )
        UpdateCrsr();
    return bRet;

}


// position cursor


SwFrm* lcl_IsInHeaderFooter( const SwNodeIndex& rIdx, Point& rPt )
{
    SwFrm* pFrm = 0;
    SwCntntNode* pCNd = rIdx.GetNode().GetCntntNode();
    if( pCNd )
    {
        SwCntntFrm *pCntFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &rPt, 0, sal_False );
        pFrm = pCntFrm ? pCntFrm->GetUpper() : NULL;
        while( pFrm && !pFrm->IsHeaderFrm() && !pFrm->IsFooterFrm() )
            pFrm = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->AnchorFrm()
                                    : pFrm->GetUpper();
    }
    return pFrm;
}

bool SwCrsrShell::IsInHeaderFooter( sal_Bool* pbInHeader ) const
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
        // #i37515# No bInFrontOfLabel during selection
        !pCrsr->HasMark() &&
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
    // is the new position in header or footer?
    SwFrm* pFrm = lcl_IsInHeaderFooter( aPos.nNode, aPt );
    if( IsTableMode() && !pFrm && aPos.nNode.GetNode().StartOfSectionNode() ==
        pCrsr->GetPoint()->nNode.GetNode().StartOfSectionNode() )
        // same table column and not in header/footer -> back
        return bRet;

    // toggle the header/footer mode if needed
    bool bInHeaderFooter = pFrm && ( pFrm->IsHeaderFrm() || pFrm->IsFooterFrm() );
    if ( bInHeaderFooter != IsHeaderFooterEdit() )
        ToggleHeaderFooterEdit();
    else
    {
        // Make sure we have the proper Header/Footer separators shown
        // as these may be changed if clicking on an empty Header/Footer
        SetShowHeaderFooterSeparator( Header, pFrm != NULL && pFrm->IsHeaderFrm( ) );
        SetShowHeaderFooterSeparator( Footer, pFrm != NULL && pFrm->IsFooterFrm( ) );

        // Repaint everything
        GetWin()->Invalidate();
    }


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
        // is at the same position and if in header/footer -> in the same
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
                // in the same frame?
                SwFrm* pOld = ((SwCntntNode&)aPos.nNode.GetNode()).getLayoutFrm(
                                GetLayout(), &aCharRect.Pos(), 0, sal_False );
                SwFrm* pNew = ((SwCntntNode&)aPos.nNode.GetNode()).getLayoutFrm(
                                GetLayout(), &aPt, 0, sal_False );
                if( pNew == pOld )
                    return bRet;
            }
        }
    }
    else
    {
        // SSelection over not allowed sections or if in header/footer -> different
        if( !CheckNodesRange( aPos.nNode, pCrsr->GetMark()->nNode, sal_True )
            || ( pFrm && !pFrm->Frm().IsInside( pCrsr->GetMkPos() ) ))
            return bRet;

        // is at same position but not in header/footer
        if( aPos == *pCrsr->GetPoint() )
            return bRet;
    }

    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    SwCrsrSaveState aSaveState( *pCrsr );

    *pCrsr->GetPoint() = aPos;
    rAktCrsrPt = aPt;

    // #i41424# Only update the marked number levels if necessary
    // Force update of marked number levels if necessary.
    if ( bNewInFrontOfLabel || bOldInFrontOfLabel )
        pCurCrsr->_SetInFrontOfLabel( !bNewInFrontOfLabel );
    SetInFrontOfLabel( bNewInFrontOfLabel );

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
            // position cursor in a valid content
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
            // there is no valid content -> hide cursor
            pVisCrsr->Hide(); // always hide visible cursor
            eMvState = MV_NONE; // status for Crsr travelling
            bAllProtect = sal_True;
            if( GetDoc()->GetDocShell() )
            {
                GetDoc()->GetDocShell()->SetReadOnlyUI( sal_True );
                CallChgLnk(); // notify UI
            }
        }
    }

    return bRet;
}


void SwCrsrShell::TblCrsrToCursor()
{
    OSL_ENSURE( pTblCrsr, "TblCrsrToCursor: Why?" );
    delete pTblCrsr, pTblCrsr = 0;
}

void SwCrsrShell::BlockCrsrToCrsr()
{
    OSL_ENSURE( pBlockCrsr, "BlockCrsrToCrsr: Why?" );
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
    // is there any GetMark?
    if( pTblCrsr )
    {
        while( pCurCrsr->GetNext() != pCurCrsr )
            delete pCurCrsr->GetNext();
        pTblCrsr->DeleteMark();

        if( pCurCrsr->HasMark() )
        {
            // move content part from mark to nodes array if not all indices
            // were moved correctly (e.g. when deleting header/footer)
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
        // move content part from mark to nodes array if not all indices
        // were moved correctly (e.g. when deleting header/footer)
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
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    pCurCrsr->Normalize(bPointFirst);
}

void SwCrsrShell::SwapPam()
{
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    pCurCrsr->Exchange();
}

//TODO: provide documentation
/** Search in the selected area for a Selection that covers the given point.

    If only a test run is made, then it checks if a SSelection exists but does
    not move the current cursor. In a normal run the cursor will be moved to the
    chosen SSelection.

    @param rPt      The point to search at.
    @param bTstOnly Should I only do a test run? If true so do not move cursor.
    @param bTstHit ???
*/
sal_Bool SwCrsrShell::ChgCurrPam( const Point & rPt,
                              sal_Bool bTstOnly, sal_Bool bTstHit )
{
    SET_CURR_SHELL( this );

    // check if the SPoint is in a table selection
    if( bTstOnly && pTblCrsr )
        return pTblCrsr->IsInside( rPt );

    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    // search position <rPt> in document
    SwPosition aPtPos( *pCurCrsr->GetPoint() );
    Point aPt( rPt );

    SwCrsrMoveState aTmpState( MV_NONE );
    aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
    if ( !GetLayout()->GetCrsrOfst( &aPtPos, aPt, &aTmpState ) && bTstHit )
        return sal_False;

    // search in all selections for this position
    SwShellCrsr* pCmp = (SwShellCrsr*)pCurCrsr; // keep the pointer on cursor
    do {
        if( pCmp->HasMark() &&
            *pCmp->Start() <= aPtPos && *pCmp->End() > aPtPos )
        {
            if( bTstOnly || pCurCrsr == pCmp ) // is the current
                return sal_True;               // return without update

            pCurCrsr = pCmp;
            UpdateCrsr(); // cursor is already at the right position
            return sal_True;
        }
    } while( pCurCrsr !=
        ( pCmp = dynamic_cast<SwShellCrsr*>(pCmp->GetNext()) ) );
    return sal_False;
}


void SwCrsrShell::KillPams()
{
    // Does any exist for deletion?
    if( !pTblCrsr && !pBlockCrsr && pCurCrsr->GetNext() == pCurCrsr )
        return;

    while( pCurCrsr->GetNext() != pCurCrsr )
        delete pCurCrsr->GetNext();
    pCurCrsr->SetColumnSelection( false );

    if( pTblCrsr )
    {
        // delete the ring of cursors
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
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
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
    // page number: first visible page or the one at the cursor
    const SwCntntFrm* pCFrm;
    const SwPageFrm *pPg = 0;

    if( !bAtCrsrPos || 0 == (pCFrm = GetCurrFrm( bCalcFrm )) ||
                       0 == (pPg   = pCFrm->FindPageFrm()) )
    {
        pPg = Imp()->GetFirstVisPage();
        while( pPg && pPg->IsEmptyPage() )
            pPg = (const SwPageFrm *)pPg->GetNext();
    }
    // pPg has to exist with a default of 1 for the special case "Writerstart"
    rnPhyNum  = pPg? pPg->GetPhyPageNum() : 1;
    rnVirtNum = pPg? pPg->GetVirtPageNum() : 1;
}


sal_uInt16 SwCrsrShell::GetNextPrevPageNum( sal_Bool bNext )
{
    SET_CURR_SHELL( this );

    // page number: first visible page or the one at the cursor
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
    // pPg has to exist with a default of 1 for the special case "Writerstart"
    return pPg ? pPg->GetPhyPageNum() : USHRT_MAX;
}


sal_uInt16 SwCrsrShell::GetPageCnt()
{
    SET_CURR_SHELL( this );
    // return number of pages
    return GetLayout()->GetPageNum();
}

/// go to the next SSelection
sal_Bool SwCrsrShell::GoNextCrsr()
{
    // is there a ring of cursors?
    if( pCurCrsr->GetNext() == pCurCrsr )
        return sal_False;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    pCurCrsr = dynamic_cast<SwShellCrsr*>(pCurCrsr->GetNext());

    // #i24086#: show also all others
    if( !ActionPend() )
    {
        UpdateCrsr();
        pCurCrsr->Show();
    }
    return sal_True;
}

/// go to the previous SSelection
sal_Bool SwCrsrShell::GoPrevCrsr()
{
    // is there a ring of cursors?
    if( pCurCrsr->GetNext() == pCurCrsr )
        return sal_False;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    pCurCrsr = dynamic_cast<SwShellCrsr*>(pCurCrsr->GetPrev());

    // #i24086#: show also all others
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

    // always switch off all cursors when painting
    SwRect aRect( rRect );

    sal_Bool bVis = sal_False;
    // if a cursor is visible then hide the SV cursor
    if( pVisCrsr->IsVisible() && !aRect.IsOver( aCharRect ) )
    {
        bVis = sal_True;
        pVisCrsr->Hide();
    }

    // re-paint area
    ViewShell::Paint( rRect );

    if( bHasFocus && !bBasicHideCrsr )
    {
        SwShellCrsr* pAktCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;

        if( !ActionPend() )
        {
            // so that right/bottom borders will not be cropped
            pAktCrsr->Invalidate( VisArea() );
            pAktCrsr->Show();
        }
        else
            pAktCrsr->Invalidate( aRect );

    }
    if( bSVCrsrVis && bVis ) // also show SV cursor again
        pVisCrsr->Show();
}



void SwCrsrShell::VisPortChgd( const SwRect & rRect )
{
    SET_CURR_SHELL( this );
    sal_Bool bVis; // switch off all cursors when scrolling

    // if a cursor is visible then hide the SV cursor
    if( sal_True == ( bVis = pVisCrsr->IsVisible() ))
        pVisCrsr->Hide();

    bVisPortChgd = sal_True;
    aOldRBPos.X() = VisArea().Right();
    aOldRBPos.Y() = VisArea().Bottom();

    // For not having problems with the SV cursor, Update() is called for the
    // Window in ViewShell::VisPo...
    // During painting no selections should be shown, thus the call is encapsulated. <- TODO: old artefact?
    ViewShell::VisPortChgd( rRect ); // move area

    if( bSVCrsrVis && bVis ) // show SV cursor again
        pVisCrsr->Show();

    if( nCrsrMove )
        bInCMvVisportChgd = sal_True;

    bVisPortChgd = sal_False;
}

/** Set the cursor back into content.

    This should only be called if the cursor was move somewhere else (e.g. when
    deleting a border). The new position is calculated from its current position
    in the layout.
*/
void SwCrsrShell::UpdateCrsrPos()
{
    SET_CURR_SHELL( this );
    ++nStartAction;
    SwShellCrsr* pShellCrsr = getShellCrsr( true );
    Size aOldSz( GetDocSize() );
    SwCntntNode *pCNode = pShellCrsr->GetCntntNode();
    SwCntntFrm  *pFrm = pCNode ?
        pCNode->getLayoutFrm( GetLayout(), &pShellCrsr->GetPtPos(), pShellCrsr->GetPoint(), sal_False ) :0;
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

// #i65475# - if Point/Mark in hidden sections, move them out
static void lcl_CheckHiddenSection( SwNodeIndex& rIdx )
{
    const SwSectionNode* pSectNd = rIdx.GetNode().FindSectionNode();
    if( pSectNd && pSectNd->GetSection().IsHiddenFlag() )
    {
        SwNodeIndex aTmp( *pSectNd );
        const SwNode* pFrmNd =
        rIdx.GetNodes().FindPrvNxtFrmNode( aTmp, pSectNd->EndOfSectionNode() );
        SAL_WARN_IF( !pFrmNd, "sw", "found no Node with Frames" );
        rIdx = aTmp;
    }
}

/// Try to set the cursor to the next visible content node.
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

// #i27301# - helper class that notifies the accessibility about invalid text
// selections in its destructor
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
void SwCrsrShell::UpdateCrsr( sal_uInt16 eFlags, sal_Bool bIdleEnd )
{
    SET_CURR_SHELL( this );

    ClearUpCrsrs();

    // In a BasicAction the cursor must be updated, e.g. to create the
    // TableCursor. EndAction now calls UpdateCrsr!
    if( ActionPend() && BasicActionPend() )
    {
        if ( eFlags & SwCrsrShell::READONLY )
            bIgnoreReadonly = sal_True;
        return; // if not then no update
    }

    sal_Bool bInHeader= sal_True;
    if ( IsInHeaderFooter( &bInHeader ) )
    {
        if ( !bInHeader )
        {
            SetShowHeaderFooterSeparator( Footer, true );
            SetShowHeaderFooterSeparator( Header, false );
        }
        else
        {
            SetShowHeaderFooterSeparator( Header, true );
            SetShowHeaderFooterSeparator( Footer, false );
        }
    }
    if ( IsInHeaderFooter() != IsHeaderFooterEdit() )
        ToggleHeaderFooterEdit();


    // #i27301#
    SwNotifyAccAboutInvalidTextSelections aInvalidateTextSelections( *this );

    if ( bIgnoreReadonly )
    {
        bIgnoreReadonly = sal_False;
        eFlags |= SwCrsrShell::READONLY;
    }

    if( eFlags & SwCrsrShell::CHKRANGE )    // check all cursor moves for
        CheckRange( pCurCrsr );             // overlapping ranges

    if( !bIdleEnd )
        CheckTblBoxCntnt();

    // If the current cursor is in a table and point/mark in different boxes,
    // then the table mode is active (also if it is already active: pTblCrsr)
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

        // Bug 65475 (1999) - if Point/Mark in hidden sections, move them out
        lcl_CheckHiddenSection( pPos->nNode );
        lcl_CheckHiddenSection( pITmpCrsr->GetMark()->nNode );

        // Move cursor out of hidden paragraphs
        if ( !GetViewOptions()->IsShowHiddenChar() )
        {
            lcl_CheckHiddenPara( *pPos );
            lcl_CheckHiddenPara( *pITmpCrsr->GetMark() );
        }

        SwCntntFrm *pTblFrm = pPos->nNode.GetNode().GetCntntNode()->
                              getLayoutFrm( GetLayout(), &aTmpPt, pPos, sal_False );

        OSL_ENSURE( pTblFrm, "Tabelle Crsr nicht im Content ??" );

        // --> Make code robust. The table cursor may point
        // to a table in a currently inactive header.
        SwTabFrm *pTab = pTblFrm ? pTblFrm->FindTabFrm() : 0;

        if ( pTab && pTab->GetTable()->GetRowsToRepeat() > 0 )
        {
            // First check if point is in repeated headline:
            bool bInRepeatedHeadline = pTab->IsFollow() && pTab->IsInHeadline( *pTblFrm );

            // Second check if mark is in repeated headline:
            if ( !bInRepeatedHeadline )
            {
                SwCntntFrm* pMarkTblFrm = pITmpCrsr->GetCntntNode( sal_False )->
                    getLayoutFrm( GetLayout(), &aTmpMk, pITmpCrsr->GetMark(), sal_False );
                OSL_ENSURE( pMarkTblFrm, "Tabelle Crsr nicht im Content ??" );

                if ( pMarkTblFrm )
                {
                    SwTabFrm* pMarkTab = pMarkTblFrm->FindTabFrm();
                    OSL_ENSURE( pMarkTab, "Tabelle Crsr nicht im Content ??" );

                    // Make code robust:
                    if ( pMarkTab )
                    {
                        bInRepeatedHeadline = pMarkTab->IsFollow() && pMarkTab->IsInHeadline( *pMarkTblFrm );
                    }
                }
            }

            // No table cursor in repeaded headlines:
            if ( bInRepeatedHeadline )
            {
                pTblFrm = 0;

                SwPosSection fnPosSect = *pPos <  *pITmpCrsr->GetMark()
                                            ? fnSectionStart
                                            : fnSectionEnd;

                // then only select inside the Box
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

        // we really want a table selection
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
                bool const bResult =
                    pTblFrm->GetCharRect( aCharRect, *pTblCrsr->GetPoint() );
                OSL_ENSURE( bResult, "GetCharRect failed." );
                (void) bResult; // non-debug: unused
            }

            pVisCrsr->Hide(); // always hide visible Cursor
            // scroll Cursor to visible area
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

            // let Layout create the Cursors in the Boxes
            if( pTblCrsr->IsCrsrMovedUpdt() )
                GetLayout()->MakeTblCrsrs( *pTblCrsr );
            if( bHasFocus && !bBasicHideCrsr )
                pTblCrsr->Show();

            // set Cursor-Points to the new Positions
            pTblCrsr->GetPtPos().X() = aCharRect.Left();
            pTblCrsr->GetPtPos().Y() = aCharRect.Top();

            if( bSVCrsrVis )
            {
                aCrsrHeight.X() = 0;
                aCrsrHeight.Y() = aTmpState.aRealHeight.Y() < 0 ?
                                  -aCharRect.Width() : aCharRect.Height();
                pVisCrsr->Show(); // show again
            }
            eMvState = MV_NONE;  // state for cursor travelling - GetCrsrOfst
            if( pTblFrm && Imp()->IsAccessible() )
                Imp()->InvalidateAccessibleCursorPosition( pTblFrm );
            return;
        }
    }

    if( pTblCrsr )
    {
        // delete Ring
        while( pCurCrsr->GetNext() != pCurCrsr )
            delete pCurCrsr->GetNext();
        pCurCrsr->DeleteMark();
        *pCurCrsr->GetPoint() = *pTblCrsr->GetPoint();
        pCurCrsr->GetPtPos() = pTblCrsr->GetPtPos();
        delete pTblCrsr, pTblCrsr = 0;
    }

    pVisCrsr->Hide(); // always hide visible Cursor

    // are we perhaps in a protected / hidden Section ?
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
                // everything protected/hidden -> special mode
                if( bAllProtect && !IsReadOnlyAvailable() &&
                    pSectNd->GetSection().IsProtectFlag() )
                    bChgState = sal_False;
                else
                {
                    eMvState = MV_NONE;     // state for cursor travelling
                    bAllProtect = sal_True;
                    if( GetDoc()->GetDocShell() )
                    {
                        GetDoc()->GetDocShell()->SetReadOnlyUI( sal_True );
                        CallChgLnk();       // notify UI!
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
                CallChgLnk();       // notify UI!
            }
        }
    }

    UpdateCrsrPos();

    // The cursor must always point into content; there's some code
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
            pFrm = pShellCrsr->GetCntntNode()->getLayoutFrm( GetLayout(),
                        &pShellCrsr->GetPtPos(), pShellCrsr->GetPoint(), sal_False );
            // if the Frm doesn't exist anymore, the complete Layout has to be
            // created, because there used to be a Frm here!
            if ( !pFrm )
            {
                do
                {
                    CalcLayout();
                    pFrm = pShellCrsr->GetCntntNode()->getLayoutFrm( GetLayout(),
                                &pShellCrsr->GetPtPos(), pShellCrsr->GetPoint(), sal_False );
                }  while( !pFrm );
            }
            else if ( Imp()->IsIdleAction() )
                // Guarantee everything's properly formatted
                pFrm->PrepareCrsr();

            // In protected Fly? but ignore in case of frame selection
            if( !IsReadOnlyAvailable() && pFrm->IsProtected() &&
                ( !Imp()->GetDrawView() ||
                  !Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() ) &&
                (!pDoc->GetDocShell() ||
                 !pDoc->GetDocShell()->IsReadOnly() || bAllProtect ) )
            {
                // look for a valid position
                sal_Bool bChgState = sal_True;
                if( !FindValidCntntNode(!HasDrawView() ||
                    0 == Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount()))
                {
                    // everything is protected / hidden -> special Mode
                    if( bAllProtect )
                        bChgState = sal_False;
                    else
                    {
                        eMvState = MV_NONE;     // state for crusor travelling
                        bAllProtect = sal_True;
                        if( GetDoc()->GetDocShell() )
                        {
                            GetDoc()->GetDocShell()->SetReadOnlyUI( sal_True );
                            CallChgLnk();       // notify UI!
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
                        CallChgLnk();       // notify UI!
                    }
                    bAllProtect = sal_False;
                    bAgainst = sal_True; // look for the right Frm again
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

        // if the layout says that we are after the 100th iteration still in
        // flow then we should always take the current position for granted.
        // (see bug: 29658)
        if( !--nLoopCnt )
        {
            OSL_ENSURE( !this, "endless loop? CharRect != OldCharRect ");
            break;
        }
        aOld = aCharRect;
        bFirst = sal_False;

        // update cursor Points to the new Positions
        pShellCrsr->GetPtPos().X() = aCharRect.Left();
        pShellCrsr->GetPtPos().Y() = aCharRect.Top();

        if( !(eFlags & SwCrsrShell::UPDOWN ))   // delete old Pos. of Up/Down
        {
            pFrm->Calc();
            nUpDownX = pFrm->IsVertical() ?
                       aCharRect.Top() - pFrm->Frm().Top() :
                       aCharRect.Left() - pFrm->Frm().Left();
        }

        // scroll Cursor to visible area
        if( bHasFocus && eFlags & SwCrsrShell::SCROLLWIN &&
            (HasSelection() || eFlags & SwCrsrShell::READONLY ||
             !IsCrsrReadonly() || GetViewOptions()->IsSelectionInReadonly()) )
        {
            // in case of scrolling this EndAction doesn't show the SV cursor
            // again, thus save and reset the flag here
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

    eMvState = MV_NONE; // state for cursor tavelling - GetCrsrOfst

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
        pVisCrsr->Show(); // show again
}

void SwCrsrShell::RefreshBlockCursor()
{
    OSL_ENSURE( pBlockCrsr, "Don't call me without a block cursor" );
    SwShellCrsr &rBlock = pBlockCrsr->getShellCrsr();
    Point aPt = rBlock.GetPtPos();
    SwCntntFrm* pFrm = rBlock.GetCntntNode()->getLayoutFrm( GetLayout(), &aPt, rBlock.GetPoint(), sal_False );
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
        OSL_ENSURE( pPam != pStart, "FillSelection should deliver at least one PaM" );
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
                pNew->insert( pNew->begin(), pCurCrsr->begin(),  pCurCrsr->end());
                pCurCrsr->clear();
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
                pNew->insert( pNew->begin(), pCurCrsr->begin(), pCurCrsr->end() );
                pCurCrsr->clear();
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

/// create a copy of the cursor and save it in the stack
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

/** delete cursor

    @param bOldCrsr If <true> so delete from stack, if <false> delete current
                    and assign the one from stack as the new current cursor.
    @return <true> if there was one on the stack, <false> otherwise
*/
sal_Bool SwCrsrShell::Pop( sal_Bool bOldCrsr )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed

    // are there any left?
    if( 0 == pCrsrStk )
        return sal_False;

    SwShellCrsr *pTmp = 0, *pOldStk = pCrsrStk;

    // the successor becomes the current one
    if( pCrsrStk->GetNext() != pCrsrStk )
    {
        pTmp = dynamic_cast<SwShellCrsr*>(pCrsrStk->GetNext());
    }

    if( bOldCrsr ) // delete from stack
        delete pCrsrStk;

    pCrsrStk = pTmp; // assign new one

    if( !bOldCrsr )
    {
        SwCrsrSaveState aSaveState( *pCurCrsr );

        // If the visible SSelection was not changed
        if( pOldStk->GetPtPos() == pCurCrsr->GetPtPos() ||
            pOldStk->GetPtPos() == pCurCrsr->GetMkPos() )
        {
            // move "Selections Rectangles"
            pCurCrsr->insert( pCurCrsr->begin(), pOldStk->begin(), pOldStk->end() );
            pOldStk->clear();
        }

        if( pOldStk->HasMark() )
        {
            pCurCrsr->SetMark();
            *pCurCrsr->GetMark() = *pOldStk->GetMark();
            pCurCrsr->GetMkPos() = pOldStk->GetMkPos();
        }
        else
            // no selection so revoke old one and set to old position
            pCurCrsr->DeleteMark();
        *pCurCrsr->GetPoint() = *pOldStk->GetPoint();
        pCurCrsr->GetPtPos() = pOldStk->GetPtPos();
        delete pOldStk;

        if( !pCurCrsr->IsInProtectTable( sal_True ) &&
            !pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                 nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ) )
            UpdateCrsr(); // update current cursor
    }
    return sal_True;
}

/** Combine two cursors

    Delete topmost from stack and use its GetMark in the current.
*/
void SwCrsrShell::Combine()
{
    // any others left?
    if( 0 == pCrsrStk )
        return;

    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    SwCrsrSaveState aSaveState( *pCurCrsr );
    if( pCrsrStk->HasMark() ) // only if GetMark was set
    {
        bool const bResult =
        CheckNodesRange( pCrsrStk->GetMark()->nNode, pCurCrsr->GetPoint()->nNode, sal_True );
        OSL_ENSURE(bResult, "StackCrsr & current Crsr not in same Section.");
        (void) bResult; // non-debug: unused
        // copy GetMark
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
        UpdateCrsr(); // update current cursor
}


void SwCrsrShell::HideCrsrs()
{
    if( !bHasFocus || bBasicHideCrsr )
        return;

    // if cursor is visible then hide SV cursor
    if( pVisCrsr->IsVisible() )
    {
        SET_CURR_SHELL( this );
        pVisCrsr->Hide();
    }
    // revoke inversion of SSelection
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

    if( bSVCrsrVis && bCrsrVis ) // also show SV cursor again
        pVisCrsr->Show();
}



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

/** Get current frame in which the cursor is positioned. */
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
            pRet = pNd->getLayoutFrm( GetLayout(), &pCurCrsr->GetPtPos(), pCurCrsr->GetPoint() );
            --(*((sal_uInt16*)pST));
            if( aOldSz != GetDocSize() )
                ((SwCrsrShell*)this)->SizeChgNotify();
        }
        else
            pRet = pNd->getLayoutFrm( GetLayout(), &pCurCrsr->GetPtPos(), pCurCrsr->GetPoint(), sal_False);
    }
    return pRet;
}

//TODO: provide documentation
/** forward all attribute/format changes at the current node to the Link

    @param pOld ???
    @param pNew ???
*/
void SwCrsrShell::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
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
        // messages are not forwarded
        // #i6681#: RES_UPDATE_ATTR is implicitly unset in
        // SwTxtNode::Insert(SwTxtHint*, sal_uInt16); we react here and thus do
        // not need to send the expensive RES_FMT_CHG in Insert.
        CallChgLnk();

    if( aGrfArrivedLnk.IsSet() &&
        ( RES_GRAPHIC_ARRIVED == nWhich || RES_GRAPHIC_SWAPIN == nWhich ))
        aGrfArrivedLnk.Call( this );
}

/** Does the current cursor create a selection?

    This means checking if GetMark is set and if SPoint and GetMark differ.
*/
sal_Bool SwCrsrShell::HasSelection() const
{
    const SwPaM* pCrsr = getShellCrsr( true );
    return( IsTableMode() || ( pCrsr->HasMark() &&
            *pCrsr->GetPoint() != *pCrsr->GetMark())
        ? sal_True : sal_False );
}


void SwCrsrShell::CallChgLnk()
{
    // Do not make any call in start/end action but just remember the change.
    // This will be taken care of in the end action.
    if( BasicActionPend() )
        bChgCallFlag = sal_True; // remember change
    else if( aChgLnk.IsSet() )
    {
        if( bCallChgLnk )
            aChgLnk.Call( this );
        bChgCallFlag = sal_False; // reset flag
    }
}

/// get selected text of a node at current cursor
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

/// get text only from current cursor position (until end of node)
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

/** get the nth character of the current SSelection

    @param bEnd    Start counting from the end? From start otherwise.
    @param nOffset position of the character
*/
sal_Unicode SwCrsrShell::GetChar( sal_Bool bEnd, long nOffset )
{
    if( IsTableMode() ) // not possible in table mode
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

/** extend current SSelection by n characters

    @param bEnd   Start counting from the end? From start otherwise.
    @param nCount Number of characters.
*/
sal_Bool SwCrsrShell::ExtendSelection( sal_Bool bEnd, xub_StrLen nCount )
{
    if( !pCurCrsr->HasMark() || IsTableMode() )
        return sal_False; // no selection

    SwPosition* pPos = bEnd ? pCurCrsr->End() : pCurCrsr->Start();
    SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
    OSL_ENSURE( pTxtNd, "no text node; how should this then be extended?" );

    xub_StrLen nPos = pPos->nContent.GetIndex();
    if( bEnd )
    {
        if( ( nPos + nCount ) <= pTxtNd->GetTxt().Len() )
            nPos = nPos + nCount;
        else
            return sal_False; // not possible
    }
    else if( nPos >= nCount )
        nPos = nPos - nCount;
    else
        return sal_False; // not possible anymore

    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed

    pPos->nContent = nPos;
    UpdateCrsr();

    return sal_True;
}

/** Move visible cursor to given position in document.

    @param rPt The position to move the visible cursor to.
    @return <sal_False> if SPoint was corrected by the layout.
*/
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

    // show only in TextNodes
    SwTxtNode* pTxtNd = aPos.nNode.GetNode().GetTxtNode();
    if( !pTxtNd )
        return sal_False;

    const SwSectionNode* pSectNd = pTxtNd->FindSectionNode();
    if( pSectNd && (pSectNd->GetSection().IsHiddenFlag() ||
                    ( !IsReadOnlyAvailable() &&
                      pSectNd->GetSection().IsProtectFlag())) )
        return sal_False;

    SwCntntFrm *pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt, &aPos );
    if ( Imp()->IsIdleAction() )
        pFrm->PrepareCrsr();
    SwRect aTmp( aCharRect );

    pFrm->GetCharRect( aCharRect, aPos, &aTmpState );

    // #i10137#
    if( aTmp == aCharRect && pVisCrsr->IsVisible() )
        return sal_True;

    pVisCrsr->Hide(); // always hide visible cursor
    if( IsScrollMDI( this, aCharRect ))
    {
        MakeVisible( aCharRect );
        pCurCrsr->Show();
    }

    {
        if( aTmpState.bRealHeight )
            aCrsrHeight = aTmpState.aRealHeight;
        else
        {
            aCrsrHeight.X() = 0;
            aCrsrHeight.Y() = aCharRect.Height();
        }

        pVisCrsr->SetDragCrsr( sal_True );
        pVisCrsr->Show(); // show again
    }
    return bRet;
}

sal_Bool SwCrsrShell::IsOverReadOnlyPos( const Point& rPt ) const
{
    Point aPt( rPt );
    SwPaM aPam( *pCurCrsr->GetPoint() );
    GetLayout()->GetCrsrOfst( aPam.GetPoint(), aPt );
    // Formular view
    return aPam.HasReadonlySel( GetViewOptions()->IsFormView() );
}

sal_Bool SwCrsrShell::IsOverHeaderFooterPos( const Point& rPt ) const
{
    Point aPt( rPt );
    SwPaM aPam( *pCurCrsr->GetPoint() );
    GetLayout()->GetCrsrOfst( aPam.GetPoint(), aPt );

    return GetDoc()->IsInHeaderFooter( aPam.GetPoint()->nNode );
}

/** Get the number of elements in the ring of cursors

    @param bAll If <false> get only spanned ones (= with selections) (Basic).
*/
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

    // after EndOfIcons comes the content selection (EndNd+StNd+CntntNd)
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

/** Invalidate cursors

    Delete all created cursors, set table crsr and last crsr to their TextNode
    (or StartNode?). They will then all re-created at the next ::GetCrsr() call.

    This is needed for Drag&Drop/ Clipboard-paste in tables.
*/
sal_Bool SwCrsrShell::ParkTblCrsr()
{
    if( !pTblCrsr )
        return sal_False;

    pTblCrsr->ParkCrsr();

    while( pCurCrsr->GetNext() != pCurCrsr )
        delete pCurCrsr->GetNext();

    // *always* move cursor's SPoint and Mark
    pCurCrsr->SetMark();
    *pCurCrsr->GetMark() = *pCurCrsr->GetPoint() = *pTblCrsr->GetPoint();
    pCurCrsr->DeleteMark();

    return sal_True;
}

void SwCrsrShell::_ParkPams( SwPaM* pDelRg, SwShellCrsr** ppDelRing )
{
    const SwPosition *pStt = pDelRg->Start(),
        *pEnd = pDelRg->GetPoint() == pStt ? pDelRg->GetMark() : pDelRg->GetPoint();

    SwPaM *pTmpDel = 0, *pTmp = *ppDelRing;

    // search over the whole ring
    sal_Bool bGoNext;
    do {
        const SwPosition *pTmpStt = pTmp->Start(),
                        *pTmpEnd = pTmp->GetPoint() == pTmpStt ?
                                        pTmp->GetMark() : pTmp->GetPoint();
        // If a SPoint or GetMark are in a cursor area than cancel the old area.
        // During comparison keep in mind that End() is outside the area.
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
        if( pTmpDel ) // is the pam in area -> delete
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
                    bDelete = sal_False; // never delete the StackCrsr
            }

            if( bDelete )
                delete pTmpDel; // invalidate old area
            else
            {
                pTmpDel->GetPoint()->nContent.Assign( 0, 0 );
                pTmpDel->GetPoint()->nNode = 0;
                pTmpDel->SetMark();
                pTmpDel->DeleteMark();
            }
            pTmpDel = 0;
        }
        else if( !pTmp->HasMark() )
        {
            // Take care that not used indices are considered.
            // SPoint is not in area but maybe GetMark is, thus set it.
            pTmp->SetMark();
            pTmp->DeleteMark();
        }
        if( bGoNext )
            pTmp = (SwPaM*)pTmp->GetNext();
    } while( !bGoNext || *ppDelRing != pTmp );
}

//TODO: provide documentation
/** Remove selections and additional cursors of all shells.

    The remaining cursor of the shell is parked.

    @param rIdx ???
*/
void SwCrsrShell::ParkCrsr( const SwNodeIndex &rIdx )
{
    SwNode *pNode = &rIdx.GetNode();

    // create a new PaM
    SwPaM * pNew = new SwPaM( *GetCrsr()->GetPoint() );
    if( pNode->GetStartNode() )
    {
        if( ( pNode = pNode->StartOfSectionNode())->IsTableNode() )
        {
            // the given node is in a table, thus park cursor to table node
            // (outside of the table)
            pNew->GetPoint()->nNode = *pNode->StartOfSectionNode();
        }
        else
            // Also on the start node itself. Then we need to request the start
            // node always via its end node! (StartOfSelection of StartNode is
            // the parent)
            pNew->GetPoint()->nNode = *pNode->EndOfSectionNode()->StartOfSectionNode();
    }
    else
        pNew->GetPoint()->nNode = *pNode->StartOfSectionNode();
    pNew->SetMark();
    pNew->GetPoint()->nNode = *pNode->EndOfSectionNode();

    // take care of all shells
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
                // set table cursor always to 0 and the current one always to
                // the beginning of the table
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


/** Copy constructor

    Copy cursor position and add it to the ring.
    All views of a document are in the ring of the shell.
*/
SwCrsrShell::SwCrsrShell( SwCrsrShell& rShell, Window *pInitWin )
    : ViewShell( rShell, pInitWin ),
    SwModify( 0 ), pCrsrStk( 0 ), pBlockCrsr( 0 ), pTblCrsr( 0 ),
    pBoxIdx( 0 ), pBoxPtr( 0 ), nCrsrMove( 0 ), nBasicActionCnt( 0 ),
    eMvState( MV_NONE ),
    sMarkedListId(),
    nMarkedListLevel( 0 )
{
    SET_CURR_SHELL( this );
    // only keep the position of the current cursor of the copy shell
    pCurCrsr = new SwShellCrsr( *this, *(rShell.pCurCrsr->GetPoint()) );
    pCurCrsr->GetCntntNode()->Add( this );

    bAllProtect = bVisPortChgd = bChgCallFlag = bInCMvVisportChgd =
    bGCAttr = bIgnoreReadonly = bSelTblCells = bBasicHideCrsr =
    bOverwriteCrsr = sal_False;
    bCallChgLnk = bHasFocus = bSVCrsrVis = bAutoUpdateCells = sal_True;
    bSetCrsrInReadOnly = sal_True;
    pVisCrsr = new SwVisCrsr( this );
    mbMacroExecAllowed = rShell.IsMacroExecAllowed();
}

/// default constructor
SwCrsrShell::SwCrsrShell( SwDoc& rDoc, Window *pInitWin,
                            const SwViewOption *pInitOpt )
    : ViewShell( rDoc, pInitWin, pInitOpt ),
    SwModify( 0 ), pCrsrStk( 0 ), pBlockCrsr( 0 ), pTblCrsr( 0 ),
    pBoxIdx( 0 ), pBoxPtr( 0 ), nCrsrMove( 0 ), nBasicActionCnt( 0 ),
    eMvState( MV_NONE ), // state for crsr-travelling - GetCrsrOfst
    sMarkedListId(),
    nMarkedListLevel( 0 )
{
    SET_CURR_SHELL( this );
    // create initial cursor and set it to first content position
    SwNodes& rNds = rDoc.GetNodes();

    SwNodeIndex aNodeIdx( *rNds.GetEndOfContent().StartOfSectionNode() );
    SwCntntNode* pCNd = rNds.GoNext( &aNodeIdx ); // gehe zum 1. ContentNode

    pCurCrsr = new SwShellCrsr( *this, SwPosition( aNodeIdx, SwIndex( pCNd, 0 )));

    // Register shell as dependent at current node. As a result all attribute
    // changes can be forwarded via the Link.
    pCNd->Add( this );

    bAllProtect = bVisPortChgd = bChgCallFlag = bInCMvVisportChgd =
    bGCAttr = bIgnoreReadonly = bSelTblCells = bBasicHideCrsr =
    bOverwriteCrsr = sal_False;
    bCallChgLnk = bHasFocus = bSVCrsrVis = bAutoUpdateCells = sal_True;
    bSetCrsrInReadOnly = sal_True;

    pVisCrsr = new SwVisCrsr( this );
    mbMacroExecAllowed = true;
}



SwCrsrShell::~SwCrsrShell()
{
    // if it is not the last view then at least the field should be updated
    if( GetNext() != this )
        CheckTblBoxCntnt( pCurCrsr->GetPoint() );
    else
        ClearTblBoxCntnt();

    delete pVisCrsr;
    delete pBlockCrsr;
    delete pTblCrsr;

    // release cursors
    while(pCurCrsr->GetNext() != pCurCrsr)
        delete pCurCrsr->GetNext();
    delete pCurCrsr;

    // free stack
    if( pCrsrStk )
    {
        while( pCrsrStk->GetNext() != pCrsrStk )
            delete pCrsrStk->GetNext();
        delete pCrsrStk;
    }

    // #i54025# - do not give a HTML parser that might potentially hang as
    // a client at the cursor shell the chance to hang itself on a TextNode
    if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );
}

SwShellCrsr* SwCrsrShell::getShellCrsr( bool bBlock )
{
    if( pTblCrsr )
        return pTblCrsr;
    if( pBlockCrsr && bBlock )
        return &pBlockCrsr->getShellCrsr();
    return pCurCrsr;
}

/** Should WaitPtr be switched on for the clipboard?

    Wait for TableMode, multiple selections and more than x selected paragraphs.
*/
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

/// show the current selected "object"
void SwCrsrShell::MakeSelVisible()
{
    OSL_ENSURE( bHasFocus, "no focus but cursor should be made visible?" );
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

/// search a valid content position (not protected/hidden)
sal_Bool SwCrsrShell::FindValidCntntNode( sal_Bool bOnlyText )
{
    if( pTblCrsr )
    {
        OSL_ENSURE( !this, "Did not remove table selection!" );
        return sal_False;
    }

    // #i45129# - everything is allowed in UI-readonly
    if( !bAllProtect && GetDoc()->GetDocShell() &&
        GetDoc()->GetDocShell()->IsReadOnlyUI() )
        return sal_True;

    if( pCurCrsr->HasMark() )
        ClearMark();

    // first check for frames
    SwNodeIndex& rNdIdx = pCurCrsr->GetPoint()->nNode;
    sal_uLong nNdIdx = rNdIdx.GetIndex(); // keep backup
    SwNodes& rNds = pDoc->GetNodes();
    SwCntntNode* pCNd = rNdIdx.GetNode().GetCntntNode();
    const SwCntntFrm * pFrm;

    if( pCNd && 0 != (pFrm = pCNd->getLayoutFrm( GetLayout(),0,pCurCrsr->GetPoint(),sal_False)) &&
        !IsReadOnlyAvailable() && pFrm->IsProtected() &&
        nNdIdx < rNds.GetEndOfExtras().GetIndex() )
    {
        // skip protected frame
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

        if( !pCNd ) // should *never* happen
        {
            rNdIdx = nNdIdx; // back to old node
            return sal_False;
        }
        *pCurCrsr->GetPoint() = *aPam.GetPoint();
    }
    else if( bOnlyText && pCNd && pCNd->IsNoTxtNode() )
    {
        // set to beginning of document
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

    // in a protected frame
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
            bool bContinue;
            do {
                bContinue = false;
                while( 0 != ( pCNd = (rNds.*funcGoSection)( &rNdIdx,
                                            sal_True, !IsReadOnlyAvailable() )) )
                {
                    // moved inside a table -> check if it is protected
                    if( pCNd->FindTableNode() )
                    {
                        SwCallLink aTmp( *this );
                        SwCrsrSaveState aSaveState( *pCurCrsr );
                        aTmp.nNdTyp = 0; // don't do anything in DTOR
                        if( !pCurCrsr->IsInProtectTable( sal_True, sal_True ) )
                        {
                            const SwSectionNode* pSNd = pCNd->FindSectionNode();
                            if( !pSNd || !pSNd->GetSection().IsHiddenFlag()
                                || (!IsReadOnlyAvailable()  &&
                                    pSNd->GetSection().IsProtectFlag() ))
                            {
                                bOk = sal_True;
                                break; // found non-protected cell
                            }
                            continue; // continue search
                        }
                    }
                    else
                    {
                        bOk = sal_True;
                        break; // found non-protected cell
                    }
                }

                if( bOk && rNdIdx.GetIndex() < rNds.GetEndOfExtras().GetIndex() )
                {
                    // also check for Fly - might be protected as well
                    if( 0 == (pFrm = pCNd->getLayoutFrm( GetLayout(),0,0,sal_False)) ||
                        ( !IsReadOnlyAvailable() && pFrm->IsProtected() ) ||
                        ( bOnlyText && pCNd->IsNoTxtNode() ) )
                    {
                        // continue search
                        bOk = sal_False;
                        bContinue = true;
                    }
                }
            } while( bContinue );

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
        xub_StrLen nCntnt = rNdIdx.GetIndex() < nNdIdx ? pCNd->Len() : 0;
        pCurCrsr->GetPoint()->nContent.Assign( pCNd, nCntnt );
    }
    else
    {
        pCNd = rNdIdx.GetNode().GetCntntNode();
        // if cursor in hidden frame, always move it
        if( !pCNd || !pCNd->getLayoutFrm( GetLayout(),0,0,sal_False) )
        {
            SwCrsrMoveState aTmpState( MV_NONE );
            aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
            GetLayout()->GetCrsrOfst( pCurCrsr->GetPoint(), pCurCrsr->GetPtPos(),
                                        &aTmpState );
        }
    }
    return bOk;
}


sal_Bool SwCrsrShell::IsCrsrReadonly() const
{
    if ( GetViewOptions()->IsReadonly() ||
         GetViewOptions()->IsFormView() /* Formular view */ )
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
        // edit in readonly sections
        else if ( pFrm && pFrm->IsInSct() &&
                  0 != ( pSection = pFrm->FindSctFrm()->GetSection() ) &&
                  pSection->IsEditInReadonlyFlag() )
        {
            return sal_False;
        }

        return sal_True;
    }
    return sal_False;
}

/// is the cursor allowed to enter ReadOnly sections?
void SwCrsrShell::SetReadOnlyAvailable( sal_Bool bFlag )
{
    // *never* switch in GlobalDoc
    if( (!GetDoc()->GetDocShell() ||
         !GetDoc()->GetDocShell()->IsA( SwGlobalDocShell::StaticType() )) &&
        bFlag != bSetCrsrInReadOnly )
    {
        // If the flag is switched off then all selections need to be
        // invalidated. Otherwise we would trust that nothing protected is selected.
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
    if( IsReadOnlyAvailable() || GetViewOptions()->IsFormView() )
    {
        if( pTblCrsr )
            bRet = pTblCrsr->HasReadOnlyBoxSel() ||
                   pTblCrsr->HasReadonlySel( GetViewOptions()->IsFormView() );
        else
        {
            const SwPaM* pCrsr = pCurCrsr;

            do {
                if( pCrsr->HasReadonlySel( GetViewOptions()->IsFormView() ) )
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

/// If the current cursor position is inside a hidden range, the hidden range
/// is selected.
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

sal_uLong SwCrsrShell::Find( const SearchOptions& rSearchOpt,
                             sal_Bool bSearchInNotes,
                             SwDocPositions eStart, SwDocPositions eEnd,
                             sal_Bool& bCancel,
                             FindRanges eRng,
                             int bReplace )
{
    if( pTblCrsr )
        GetCrsr();
    delete pTblCrsr, pTblCrsr = 0;
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    sal_uLong nRet = pCurCrsr->Find( rSearchOpt, bSearchInNotes, eStart, eEnd,
                                     bCancel, eRng, bReplace );
    if( nRet || bCancel )
        UpdateCrsr();
    return nRet;
}

sal_uLong SwCrsrShell::Find( const SwTxtFmtColl& rFmtColl,
                             SwDocPositions eStart, SwDocPositions eEnd,
                             sal_Bool& bCancel,
                             FindRanges eRng,
                             const SwTxtFmtColl* pReplFmt )
{
    if( pTblCrsr )
        GetCrsr();
    delete pTblCrsr, pTblCrsr = 0;
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    sal_uLong nRet = pCurCrsr->Find( rFmtColl, eStart, eEnd, bCancel, eRng,
                                     pReplFmt );
    if( nRet )
        UpdateCrsr();
    return nRet;
}

sal_uLong SwCrsrShell::Find( const SfxItemSet& rSet,
                             sal_Bool bNoCollections,
                             SwDocPositions eStart, SwDocPositions eEnd,
                             sal_Bool& bCancel,
                             FindRanges eRng,
                             const SearchOptions* pSearchOpt,
                             const SfxItemSet* rReplSet )
{
    if( pTblCrsr )
        GetCrsr();
    delete pTblCrsr, pTblCrsr = 0;
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    sal_uLong nRet = pCurCrsr->Find( rSet, bNoCollections, eStart, eEnd,
                                     bCancel, eRng, pSearchOpt, rReplSet );
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
    OSL_ENSURE( pPam->HasMark(), "Don't remove pPoint!" );
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
           aPos.nContent.GetIdxReg();
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

    // For all entries in the ring except the start entry delete the entry if
    // it is invalid.
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
            // If the start entry of the ring is invalid replace it with a
            // cursor pointing to the beginning of the first content node in the
            // document.
            aIdx = (*(aNodes.GetEndOfContent().StartOfSectionNode()));
            pNode = aNodes.GoNext( &aIdx );
        }
        bool bFound = (pNode != NULL);

        OSL_ENSURE(bFound, "no content node found");

        if (bFound)
        {
            SwPaM aTmpPam(*pNode);
            *pStartCrsr = aTmpPam;
        }

        bChanged = true;
    }

    // If at least one of the cursors in the ring have been deleted or replaced,
    // remove the table cursor.
    if (pTblCrsr != NULL && bChanged)
        TblCrsrToCursor();
}

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

    if ( !aSmartTagTypes.empty() )
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

            // make sure the selection build later from the data below does not
            // include "in word" character to the left and right in order to
            // preserve those. Therefore count those "in words" in order to
            // modify the selection accordingly.
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
            // do not determine the rectangle in the current line
            xub_StrLen nWordStart = (nBegin + nLeft) < nLineStart ? nLineStart : nBegin + nLeft;
            // take one less than the line end - otherwise the next line would
            // be calculated
            xub_StrLen nWordEnd = (nBegin + nLen - nLeft - nRight) > nLineEnd ? nLineEnd : (nBegin + nLen - nLeft - nRight);
            Push();
            pCrsr->DeleteMark();
            SwIndex& rContent = GetCrsr()->GetPoint()->nContent;
            rContent = nWordStart;
            SwRect aStartRect;
            SwCrsrMoveState aState;
            aState.bRealWidth = sal_True;
            SwCntntNode* pCntntNode = pCrsr->GetCntntNode();
            SwCntntFrm *pCntntFrame = pCntntNode->getLayoutFrm( GetLayout(), &rPt, pCrsr->GetPoint(), sal_False);

            pCntntFrame->GetCharRect( aStartRect, *pCrsr->GetPoint(), &aState );
            rContent = nWordEnd - 1;
            SwRect aEndRect;
            pCntntFrame->GetCharRect( aEndRect, *pCrsr->GetPoint(),&aState );
            rSelectRect = aStartRect.Union( aEndRect );
            Pop(sal_False);
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
