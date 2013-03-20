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
    SwShellCrsr* pNew = new SwShellCrsr( *m_pCurCrsr );

    // Hide PaM logically, to avoid undoing the inverting from
    // copied PaM (#i75172#)
    pNew->swapContent(*m_pCurCrsr);

    m_pCurCrsr->DeleteMark();

    UpdateCrsr( SwCrsrShell::SCROLLWIN );
    return pNew;
}

/**
 * Delete current Cursor, making the following one the current.
 * Note, this function does not delete anything if there is no other cursor.
 * @return - returns true if there was another cursor and we deleted one.
 */
bool SwCrsrShell::DestroyCrsr()
{
    // don't delete Crsr within table selection
    OSL_ENSURE( !IsTableMode(), "in table Selection" );

    // Is there a next one? Don't do anything if not.
    if(m_pCurCrsr->GetNext() == m_pCurCrsr)
        return false;

    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCursor* pNextCrsr = (SwCursor*)m_pCurCrsr->GetNext();
    delete m_pCurCrsr;
    m_pCurCrsr = dynamic_cast<SwShellCrsr*>(pNextCrsr);
    UpdateCrsr();
    return true;
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
    if( m_pTblCrsr )
    {
        if( bMakeTblCrsr && m_pTblCrsr->IsCrsrMovedUpdt() )
        {
            //don't re-create 'parked' cursors
            const SwCntntNode* pCNd;
            if( m_pTblCrsr->GetPoint()->nNode.GetIndex() &&
                m_pTblCrsr->GetMark()->nNode.GetIndex() &&
                0 != ( pCNd = m_pTblCrsr->GetCntntNode() ) && pCNd->getLayoutFrm( GetLayout() ) &&
                0 != ( pCNd = m_pTblCrsr->GetCntntNode(sal_False) ) && pCNd->getLayoutFrm( GetLayout() ) )
            {
                SwShellTableCrsr* pTC = (SwShellTableCrsr*)m_pTblCrsr;
                GetLayout()->MakeTblCrsrs( *pTC );
            }
        }

        if( m_pTblCrsr->IsChgd() )
        {
            const_cast<SwCrsrShell*>(this)->m_pCurCrsr =
                dynamic_cast<SwShellCrsr*>(m_pTblCrsr->MakeBoxSels( m_pCurCrsr ));
        }
    }
    return m_pCurCrsr;
}

void SwCrsrShell::StartAction()
{
    if( !ActionPend() )
    {
        // save for update of the ribbon bar
        const SwNode& rNd = m_pCurCrsr->GetPoint()->nNode.GetNode();
        m_nAktNode = rNd.GetIndex();
        m_nAktCntnt = m_pCurCrsr->GetPoint()->nContent.GetIndex();
        m_nAktNdTyp = rNd.GetNodeType();
        m_bAktSelection = *m_pCurCrsr->GetPoint() != *m_pCurCrsr->GetMark();
        if( rNd.IsTxtNode() )
            m_nLeftFrmPos = SwCallLink::getLayoutFrm( GetLayout(), (SwTxtNode&)rNd, m_nAktCntnt, sal_True );
        else
            m_nLeftFrmPos = 0;
    }
    ViewShell::StartAction(); // to the ViewShell
}

void SwCrsrShell::EndAction( const sal_Bool bIdleEnd )
{
    sal_Bool bVis = m_bSVCrsrVis;

    // Idle-formatting?
    if( bIdleEnd && Imp()->GetRegion() )
    {
        m_pCurCrsr->Hide();
    }

    // Update all invalid numberings before the last action
    if( 1 == mnStartAction )
        GetDoc()->UpdateNumRule();

    // #i76923#: Don't show the cursor in the ViewShell::EndAction() - call.
    //           Only the UpdateCrsr shows the cursor.
    sal_Bool bSavSVCrsrVis = m_bSVCrsrVis;
    m_bSVCrsrVis = sal_False;

    ViewShell::EndAction( bIdleEnd );   // have ViewShell go first

    m_bSVCrsrVis = bSavSVCrsrVis;

    if( ActionPend() )
    {
        if( bVis )    // display SV-Cursor again
            m_pVisCrsr->Show();

        // If there is still a ChgCall and just the "basic
        // parenthiszing(?) (Basic-Klammerung)" exists, call it. This
        // decouples the internal with the Basic-parenthising, the
        // Shells are switched.
        if( !BasicActionPend() )
        {
            // Within a Basic action, one needs to update the cursor,
            // to e.g. create the table cursor. This is being done in
            // UpdateCrsr.
            UpdateCrsr( SwCrsrShell::CHKRANGE, bIdleEnd );

            {
                // watch Crsr-Moves, call Link if needed, the DTOR is key here!
                SwCallLink aLk( *this, m_nAktNode, m_nAktCntnt, (sal_uInt8)m_nAktNdTyp,
                                m_nLeftFrmPos, m_bAktSelection );

            }
            if( m_bCallChgLnk && m_bChgCallFlag && m_aChgLnk.IsSet() )
            {
                m_aChgLnk.Call( this );
                m_bChgCallFlag = sal_False; // reset flag
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
        aLk.nNode = m_nAktNode;           // call Link if needed
        aLk.nNdTyp = (sal_uInt8)m_nAktNdTyp;
        aLk.nCntnt = m_nAktCntnt;
        aLk.nLeftFrmPos = m_nLeftFrmPos;

        if( !m_nCrsrMove ||
            ( 1 == m_nCrsrMove && m_bInCMvVisportChgd ) )
            // display Cursor & Selektions again
            ShowCrsrs( m_bSVCrsrVis ? sal_True : sal_False );
    }
    // call ChgCall if there is still one
    if( m_bCallChgLnk && m_bChgCallFlag && m_aChgLnk.IsSet() )
    {
        m_aChgLnk.Call( this );
        m_bChgCallFlag = sal_False;       // reset flag
    }
}

#ifdef DBG_UTIL
void SwCrsrShell::SttCrsrMove()
{
    OSL_ENSURE( m_nCrsrMove < USHRT_MAX, "To many nested CrsrMoves." );
    ++m_nCrsrMove;
    StartAction();
}

void SwCrsrShell::EndCrsrMove( const sal_Bool bIdleEnd )
{
    OSL_ENSURE( m_nCrsrMove, "EndCrsrMove() without SttCrsrMove()." );
    EndAction( bIdleEnd );
    if( !--m_nCrsrMove )
        m_bInCMvVisportChgd = sal_False;
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

    if( m_pBlockCrsr )
        m_pBlockCrsr->clearPoints();

    // 1. CASE: Cursor is in front of label. A move to the right
    // will simply reset the bInFrontOfLabel flag:
    SwShellCrsr* pShellCrsr = getShellCrsr( true );
    if ( !bLeft && pShellCrsr->IsInFrontOfLabel() )
    {
        SetInFrontOfLabel( false );
        bRet = sal_True;
    }
    // 2. CASE: Cursor is at beginning of numbered paragraph. A move
    // to the left will simply set the bInFrontOfLabel flag:
    else if ( bLeft && 0 == pShellCrsr->GetPoint()->nContent.GetIndex() &&
             !pShellCrsr->IsInFrontOfLabel() && !pShellCrsr->HasMark() &&
             0 != ( pTxtNd = pShellCrsr->GetNode()->GetTxtNode() ) &&
             pTxtNd->HasVisibleNumberingOrBullet() )
    {
        SetInFrontOfLabel( true );
        bRet = sal_True;
    }
    // 3. CASE: Regular cursor move. Reset the bInFrontOfLabel flag:
    else
    {
        const sal_Bool bSkipHidden = !GetViewOptions()->IsShowHiddenChar();
        // #i107447#
        // To avoid loop the reset of <bInFrontOfLabel> flag is no longer
        // reflected in the return value <bRet>.
        const bool bResetOfInFrontOfLabel = SetInFrontOfLabel( false );
        bRet = pShellCrsr->LeftRight( bLeft, nCnt, nMode, bVisualAllowed,
                                      bSkipHidden, !IsOverwriteCrsr() );
        if ( !bRet && bLeft && bResetOfInFrontOfLabel )
        {
            // undo reset of <bInFrontOfLabel> flag
            SetInFrontOfLabel( true );
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
    if ( sListId != m_sMarkedListId ||
         nListLevel != m_nMarkedListLevel)
    {
        if ( m_sMarkedListId.Len() > 0 )
            mpDoc->MarkListLevel( m_sMarkedListId, m_nMarkedListLevel, false );

        if ( sListId.Len() > 0 )
        {
            mpDoc->MarkListLevel( sListId, nListLevel, true );
        }

        m_sMarkedListId = sListId;
        m_nMarkedListLevel = nListLevel;
    }
}

void SwCrsrShell::UpdateMarkedListLevel()
{
    SwTxtNode * pTxtNd = _GetCrsr()->GetNode()->GetTxtNode();

    if ( pTxtNd )
    {
        if ( !pTxtNd->IsNumbered() )
        {
            m_pCurCrsr->_SetInFrontOfLabel( false );
            MarkListLevel( String(), 0 );
        }
        else if ( m_pCurCrsr->IsInFrontOfLabel() )
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
    bRet = SetInFrontOfLabel(false) || bRet;

    if( m_pBlockCrsr )
        m_pBlockCrsr->clearPoints();

    if( bRet )
    {
        m_eMvState = MV_UPDOWN; // status for Crsr travelling - GetCrsrOfst
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
    m_eMvState = MV_LEFTMARGIN; // status for Crsr travelling - GetCrsrOfst

    const sal_Bool bTableMode = IsTableMode();
    SwShellCrsr* pTmpCrsr = getShellCrsr( true );

    if( m_pBlockCrsr )
        m_pBlockCrsr->clearPoints();

    const bool bWasAtLM =
            ( 0 == _GetCrsr()->GetPoint()->nContent.GetIndex() );

    sal_Bool bRet = pTmpCrsr->LeftRightMargin( bLeft, bAPI );

    if ( bLeft && !bTableMode && bRet && bWasAtLM && !_GetCrsr()->HasMark() )
    {
        const SwTxtNode * pTxtNd = _GetCrsr()->GetNode()->GetTxtNode();
        if ( pTxtNd && pTxtNd->HasVisibleNumberingOrBullet() )
            SetInFrontOfLabel( true );
    }
    else if ( !bLeft )
    {
        bRet = SetInFrontOfLabel( false ) || bRet;
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

    SwShellCrsr* pTmpCrsr = m_pBlockCrsr ? &m_pBlockCrsr->getShellCrsr() : m_pCurCrsr;
    sal_Bool bRet = pTmpCrsr->SttEndDoc( bStt );
    if( bRet )
    {
        if( bStt )
            pTmpCrsr->GetPtPos().Y() = 0; // set to 0 explicitly (table header)
        if( m_pBlockCrsr )
        {
            m_pBlockCrsr->clearPoints();
            RefreshBlockCursor();
        }

        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

void SwCrsrShell::ExtendedSelectAll()
{
    SwNodes& rNodes = GetDoc()->GetNodes();
    SwPosition* pPos = m_pCurCrsr->GetPoint();
    pPos->nNode = rNodes.GetEndOfPostIts();
    pPos->nContent.Assign( rNodes.GoNext( &pPos->nNode ), 0 );
    pPos = m_pCurCrsr->GetMark();
    pPos->nNode = rNodes.GetEndOfContent();
    SwCntntNode* pCNd = rNodes.GoPrevious( &pPos->nNode );
    pPos->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );
}

sal_Bool SwCrsrShell::MovePage( SwWhichPage fnWhichPage, SwPosPage fnPosPage )
{
    sal_Bool bRet = sal_False;

    // never jump of section borders at selection
    if( !m_pCurCrsr->HasMark() || !m_pCurCrsr->IsNoCntnt() )
    {
        SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
        SET_CURR_SHELL( this );

        SwCrsrSaveState aSaveState( *m_pCurCrsr );
        Point& rPt = m_pCurCrsr->GetPtPos();
        SwCntntFrm * pFrm = m_pCurCrsr->GetCntntNode()->
                            getLayoutFrm( GetLayout(), &rPt, m_pCurCrsr->GetPoint(), sal_False );
        if( pFrm && sal_True == ( bRet = GetFrmInPage( pFrm, fnWhichPage,
                                                fnPosPage, m_pCurCrsr )  ) &&
            !m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                 nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ))
            UpdateCrsr();
        else
            bRet = sal_False;
    }
    return bRet;
}

bool SwCrsrShell::isInHiddenTxtFrm(SwShellCrsr* pShellCrsr)
{
    SwCntntNode *pCNode = pShellCrsr->GetCntntNode();
    SwCntntFrm  *pFrm = pCNode ?
        pCNode->getLayoutFrm( GetLayout(), &pShellCrsr->GetPtPos(), pShellCrsr->GetPoint(), sal_False ) : 0;
    return !pFrm || (pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow());
}

sal_Bool SwCrsrShell::MovePara(SwWhichPara fnWhichPara, SwPosPara fnPosPara )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    SwShellCrsr* pTmpCrsr = getShellCrsr( true );
    sal_Bool bRet = pTmpCrsr->MovePara( fnWhichPara, fnPosPara );
    if( bRet )
    {
        //keep going until we get something visible, i.e. skip
        //over hidden paragraphs, don't get stuck at the start
        //which is what SwCrsrShell::UpdateCrsrPos will reset
        //the position to if we pass it a position in an
        //invisible hidden paragraph field
        while (isInHiddenTxtFrm(pTmpCrsr))
        {
            if (!pTmpCrsr->MovePara(fnWhichPara, fnPosPara))
                break;
        }

        UpdateCrsr();
    }
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

static SwFrm* lcl_IsInHeaderFooter( const SwNodeIndex& rIdx, Point& rPt )
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
    SwFrm* pFrm = ::lcl_IsInHeaderFooter( m_pCurCrsr->GetPoint()->nNode, aPt );
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
        m_eMvState = MV_RIGHTMARGIN;
    // is the new position in header or footer?
    SwFrm* pFrm = lcl_IsInHeaderFooter( aPos.nNode, aPt );
    if( IsTableMode() && !pFrm && aPos.nNode.GetNode().StartOfSectionNode() ==
        pCrsr->GetPoint()->nNode.GetNode().StartOfSectionNode() )
        // same table column and not in header/footer -> back
        return bRet;

    if( m_pBlockCrsr && bBlock )
    {
        m_pBlockCrsr->setEndPoint( rLPt );
        if( !pCrsr->HasMark() )
            m_pBlockCrsr->setStartPoint( rLPt );
        else if( !m_pBlockCrsr->getStartPoint() )
            m_pBlockCrsr->setStartPoint( pCrsr->GetMkPos() );
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
                                GetLayout(), &m_aCharRect.Pos(), 0, sal_False );
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
        if( !CheckNodesRange( aPos.nNode, pCrsr->GetMark()->nNode, true )
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
        m_pCurCrsr->_SetInFrontOfLabel( !bNewInFrontOfLabel );
    SetInFrontOfLabel( bNewInFrontOfLabel );

    if( !pCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ) )
    {
        sal_uInt16 nFlag = SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE;
        UpdateCrsr( nFlag );
        bRet &= ~CRSR_POSOLD;
    }
    else if( bOnlyText && !m_pCurCrsr->HasMark() )
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
            m_pVisCrsr->Hide(); // always hide visible cursor
            m_eMvState = MV_NONE; // status for Crsr travelling
            m_bAllProtect = sal_True;
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
    OSL_ENSURE( m_pTblCrsr, "TblCrsrToCursor: Why?" );
    delete m_pTblCrsr, m_pTblCrsr = 0;
}

void SwCrsrShell::BlockCrsrToCrsr()
{
    OSL_ENSURE( m_pBlockCrsr, "BlockCrsrToCrsr: Why?" );
    if( m_pBlockCrsr && !HasSelection() )
    {
        SwPaM& rPam = m_pBlockCrsr->getShellCrsr();
        m_pCurCrsr->SetMark();
        *m_pCurCrsr->GetPoint() = *rPam.GetPoint();
        if( rPam.HasMark() )
            *m_pCurCrsr->GetMark() = *rPam.GetMark();
        else
            m_pCurCrsr->DeleteMark();
    }
    delete m_pBlockCrsr, m_pBlockCrsr = 0;
}

void SwCrsrShell::CrsrToBlockCrsr()
{
    if( !m_pBlockCrsr )
    {
        SwPosition aPos( *m_pCurCrsr->GetPoint() );
        m_pBlockCrsr = createBlockCursor( *this, aPos );
        SwShellCrsr &rBlock = m_pBlockCrsr->getShellCrsr();
        rBlock.GetPtPos() = m_pCurCrsr->GetPtPos();
        if( m_pCurCrsr->HasMark() )
        {
            rBlock.SetMark();
            *rBlock.GetMark() = *m_pCurCrsr->GetMark();
            rBlock.GetMkPos() = m_pCurCrsr->GetMkPos();
        }
    }
    m_pBlockCrsr->clearPoints();
    RefreshBlockCursor();
}

void SwCrsrShell::ClearMark()
{
    // is there any GetMark?
    if( m_pTblCrsr )
    {
        while( m_pCurCrsr->GetNext() != m_pCurCrsr )
            delete m_pCurCrsr->GetNext();
        m_pTblCrsr->DeleteMark();

        m_pCurCrsr->DeleteMark();

        *m_pCurCrsr->GetPoint() = *m_pTblCrsr->GetPoint();
        m_pCurCrsr->GetPtPos() = m_pTblCrsr->GetPtPos();
        delete m_pTblCrsr, m_pTblCrsr = 0;
        m_pCurCrsr->SwSelPaintRects::Show();
    }
    else
    {
        if( !m_pCurCrsr->HasMark() )
            return;
        m_pCurCrsr->DeleteMark();
        if( !m_nCrsrMove )
            m_pCurCrsr->SwSelPaintRects::Show();
    }
}

void SwCrsrShell::NormalizePam(sal_Bool bPointFirst)
{
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    m_pCurCrsr->Normalize(bPointFirst);
}

void SwCrsrShell::SwapPam()
{
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    m_pCurCrsr->Exchange();
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
    if( bTstOnly && m_pTblCrsr )
        return m_pTblCrsr->IsInside( rPt );

    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    // search position <rPt> in document
    SwPosition aPtPos( *m_pCurCrsr->GetPoint() );
    Point aPt( rPt );

    SwCrsrMoveState aTmpState( MV_NONE );
    aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
    if ( !GetLayout()->GetCrsrOfst( &aPtPos, aPt, &aTmpState ) && bTstHit )
        return sal_False;

    // search in all selections for this position
    SwShellCrsr* pCmp = (SwShellCrsr*)m_pCurCrsr; // keep the pointer on cursor
    do {
        if( pCmp->HasMark() &&
            *pCmp->Start() <= aPtPos && *pCmp->End() > aPtPos )
        {
            if( bTstOnly || m_pCurCrsr == pCmp ) // is the current
                return sal_True;               // return without update

            m_pCurCrsr = pCmp;
            UpdateCrsr(); // cursor is already at the right position
            return sal_True;
        }
    } while( m_pCurCrsr !=
        ( pCmp = dynamic_cast<SwShellCrsr*>(pCmp->GetNext()) ) );
    return sal_False;
}

void SwCrsrShell::KillPams()
{
    // Does any exist for deletion?
    if( !m_pTblCrsr && !m_pBlockCrsr && m_pCurCrsr->GetNext() == m_pCurCrsr )
        return;

    while( m_pCurCrsr->GetNext() != m_pCurCrsr )
        delete m_pCurCrsr->GetNext();
    m_pCurCrsr->SetColumnSelection( false );

    if( m_pTblCrsr )
    {
        // delete the ring of cursors
        m_pCurCrsr->DeleteMark();
        *m_pCurCrsr->GetPoint() = *m_pTblCrsr->GetPoint();
        m_pCurCrsr->GetPtPos() = m_pTblCrsr->GetPtPos();
        delete m_pTblCrsr;
        m_pTblCrsr = 0;
    }
    else if( m_pBlockCrsr )
    {
        // delete the ring of cursors
        m_pCurCrsr->DeleteMark();
        SwShellCrsr &rBlock = m_pBlockCrsr->getShellCrsr();
        *m_pCurCrsr->GetPoint() = *rBlock.GetPoint();
        m_pCurCrsr->GetPtPos() = rBlock.GetPtPos();
        rBlock.DeleteMark();
        m_pBlockCrsr->clearPoints();
    }
    UpdateCrsr( SwCrsrShell::SCROLLWIN );
}

int SwCrsrShell::CompareCursor( CrsrCompareType eType ) const
{
    int nRet = 0;
    const SwPosition *pFirst = 0, *pSecond = 0;
    const SwPaM *pCur = GetCrsr(), *pStk = m_pCrsrStk;
    // cursor on stack is needed if we compare against stack
    if( pStk || ( eType == CurrPtCurrMk ) )
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

bool SwCrsrShell::IsSttPara() const
{   return m_pCurCrsr->GetPoint()->nContent == 0; }

bool SwCrsrShell::IsEndPara() const
{   return m_pCurCrsr->GetPoint()->nContent == m_pCurCrsr->GetCntntNode()->Len(); }

bool SwCrsrShell::IsEndOfTable() const
{
    if (IsTableMode() || IsBlockMode() || !IsEndPara())
    {
        return false;
    }
    SwTableNode const*const pTableNode( IsCrsrInTbl() );
    if (!pTableNode)
    {
        return false;
    }
    SwEndNode const*const pEndTableNode(pTableNode->EndOfSectionNode());
    SwNodeIndex const lastNode(*pEndTableNode, -2);
    SAL_WARN_IF(!lastNode.GetNode().GetTxtNode(), "sw.core",
            "text node expected");
    return (lastNode == m_pCurCrsr->GetPoint()->nNode);
}

bool SwCrsrShell::IsInFrontOfLabel() const
{
    return m_pCurCrsr->IsInFrontOfLabel();
}

bool SwCrsrShell::SetInFrontOfLabel( bool bNew )
{
    if ( bNew != IsInFrontOfLabel() )
    {
        m_pCurCrsr->_SetInFrontOfLabel( bNew );
        UpdateMarkedListLevel();
        return true;
    }
    return false;
}

sal_Bool SwCrsrShell::GotoPage( sal_uInt16 nPage )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    SwCrsrSaveState aSaveState( *m_pCurCrsr );
    sal_Bool bRet = GetLayout()->SetCurrPage( m_pCurCrsr, nPage ) &&
                    !m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                         nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}

sal_Bool SwCrsrShell::GetCharRectAt(SwRect& rRect, const SwPosition* pPos)
{
    SwCntntFrm* pFrm = GetCurrFrm();
    return pFrm->GetCharRect( rRect, *pPos );
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
    if( m_pCurCrsr->GetNext() == m_pCurCrsr )
        return sal_False;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    m_pCurCrsr = dynamic_cast<SwShellCrsr*>(m_pCurCrsr->GetNext());

    // #i24086#: show also all others
    if( !ActionPend() )
    {
        UpdateCrsr();
        m_pCurCrsr->Show();
    }
    return sal_True;
}

/// go to the previous SSelection
sal_Bool SwCrsrShell::GoPrevCrsr()
{
    // is there a ring of cursors?
    if( m_pCurCrsr->GetNext() == m_pCurCrsr )
        return sal_False;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    m_pCurCrsr = dynamic_cast<SwShellCrsr*>(m_pCurCrsr->GetPrev());

    // #i24086#: show also all others
    if( !ActionPend() )
    {
        UpdateCrsr();
        m_pCurCrsr->Show();
    }

    return sal_True;
}

void SwCrsrShell::Paint( const Rectangle &rRect)
{
    SET_CURR_SHELL( this );

    // always switch off all cursors when painting
    SwRect aRect( rRect );

    bool bVis = false;
    // if a cursor is visible then hide the SV cursor
    if( m_pVisCrsr->IsVisible() && !aRect.IsOver( m_aCharRect ) )
    {
        bVis = true;
        m_pVisCrsr->Hide();
    }

    // re-paint area
    ViewShell::Paint( rRect );

    if( m_bHasFocus && !m_bBasicHideCrsr )
    {
        SwShellCrsr* pAktCrsr = m_pTblCrsr ? m_pTblCrsr : m_pCurCrsr;

        if( !ActionPend() )
        {
            // so that right/bottom borders will not be cropped
            pAktCrsr->Invalidate( VisArea() );
            pAktCrsr->Show();
        }
        else
            pAktCrsr->Invalidate( aRect );

    }
    if( m_bSVCrsrVis && bVis ) // also show SV cursor again
        m_pVisCrsr->Show();
}

void SwCrsrShell::VisPortChgd( const SwRect & rRect )
{
    SET_CURR_SHELL( this );
    bool bVis; // switch off all cursors when scrolling

    // if a cursor is visible then hide the SV cursor
    if( ( bVis = m_pVisCrsr->IsVisible() ) )
        m_pVisCrsr->Hide();

    m_bVisPortChgd = sal_True;
    m_aOldRBPos.setX(VisArea().Right());
    m_aOldRBPos.setY(VisArea().Bottom());

    // For not having problems with the SV cursor, Update() is called for the
    // Window in ViewShell::VisPo...
    // During painting no selections should be shown, thus the call is encapsulated. <- TODO: old artefact?
    ViewShell::VisPortChgd( rRect ); // move area

    if( m_bSVCrsrVis && bVis ) // show SV cursor again
        m_pVisCrsr->Show();

    if( m_nCrsrMove )
        m_bInCMvVisportChgd = sal_True;

    m_bVisPortChgd = sal_False;
}

/** Set the cursor back into content.

    This should only be called if the cursor was move somewhere else (e.g. when
    deleting a border). The new position is calculated from its current position
    in the layout.
*/
void SwCrsrShell::UpdateCrsrPos()
{
    SET_CURR_SHELL( this );
    ++mnStartAction;
    SwShellCrsr* pShellCrsr = getShellCrsr( true );
    Size aOldSz( GetDocSize() );

    if( isInHiddenTxtFrm(pShellCrsr) )
    {
        SwCrsrMoveState aTmpState( MV_NONE );
        aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
        GetLayout()->GetCrsrOfst( pShellCrsr->GetPoint(), pShellCrsr->GetPtPos(),
                                     &aTmpState );
        pShellCrsr->DeleteMark();
    }
    IGrammarContact *pGrammarContact = GetDoc() ? GetDoc()->getGrammarContact() : 0;
    if( pGrammarContact )
        pGrammarContact->updateCursorPosition( *m_pCurCrsr->GetPoint() );
    --mnStartAction;
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
            m_bIgnoreReadonly = sal_True;
        return; // if not then no update
    }

    // #i27301#
    SwNotifyAccAboutInvalidTextSelections aInvalidateTextSelections( *this );

    if ( m_bIgnoreReadonly )
    {
        m_bIgnoreReadonly = sal_False;
        eFlags |= SwCrsrShell::READONLY;
    }

    if( eFlags & SwCrsrShell::CHKRANGE )    // check all cursor moves for
        CheckRange( m_pCurCrsr );             // overlapping ranges

    if( !bIdleEnd )
        CheckTblBoxCntnt();

    // If the current cursor is in a table and point/mark in different boxes,
    // then the table mode is active (also if it is already active: m_pTblCrsr)
    SwPaM* pTstCrsr = getShellCrsr( true );
    if( pTstCrsr->HasMark() && !m_pBlockCrsr &&
        mpDoc->IsIdxInTbl( pTstCrsr->GetPoint()->nNode ) &&
          ( m_pTblCrsr ||
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

            // No table cursor in repeated headlines:
            if ( bInRepeatedHeadline )
            {
                pTblFrm = 0;

                SwPosSection fnPosSect = *pPos <  *pITmpCrsr->GetMark()
                                            ? fnSectionStart
                                            : fnSectionEnd;

                // then only select inside the Box
                if( m_pTblCrsr )
                {
                    m_pCurCrsr->SetMark();
                    *m_pCurCrsr->GetMark() = *m_pTblCrsr->GetMark();
                    m_pCurCrsr->GetMkPos() = m_pTblCrsr->GetMkPos();
                    m_pTblCrsr->DeleteMark();
                    m_pTblCrsr->SwSelPaintRects::Hide();
                }

                *m_pCurCrsr->GetPoint() = *m_pCurCrsr->GetMark();
                (*fnSectionCurr)( *m_pCurCrsr, fnPosSect );
            }
        }

        // we really want a table selection
        if( pTab && pTblFrm )
        {
            if( !m_pTblCrsr )
            {
                m_pTblCrsr = new SwShellTableCrsr( *this,
                                *m_pCurCrsr->GetMark(), m_pCurCrsr->GetMkPos(),
                                *pPos, aTmpPt );
                m_pCurCrsr->DeleteMark();
                m_pCurCrsr->SwSelPaintRects::Hide();

                CheckTblBoxCntnt();
            }

            SwCrsrMoveState aTmpState( MV_NONE );
            aTmpState.bRealHeight = sal_True;
            if( !pTblFrm->GetCharRect( m_aCharRect, *m_pTblCrsr->GetPoint(), &aTmpState ) )
            {
                Point aCentrPt( m_aCharRect.Center() );
                aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
                pTblFrm->GetCrsrOfst( m_pTblCrsr->GetPoint(), aCentrPt, &aTmpState );
                bool const bResult =
                    pTblFrm->GetCharRect( m_aCharRect, *m_pTblCrsr->GetPoint() );
                OSL_ENSURE( bResult, "GetCharRect failed." );
                (void) bResult; // non-debug: unused
            }

            m_pVisCrsr->Hide(); // always hide visible Cursor
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
                    MakeVisible( m_aCharRect );
            }

            // let Layout create the Cursors in the Boxes
            if( m_pTblCrsr->IsCrsrMovedUpdt() )
                GetLayout()->MakeTblCrsrs( *m_pTblCrsr );
            if( m_bHasFocus && !m_bBasicHideCrsr )
                m_pTblCrsr->Show();

            // set Cursor-Points to the new Positions
            m_pTblCrsr->GetPtPos().setX(m_aCharRect.Left());
            m_pTblCrsr->GetPtPos().setY(m_aCharRect.Top());

            if( m_bSVCrsrVis )
            {
                m_aCrsrHeight.setX(0);
                m_aCrsrHeight.setY(aTmpState.aRealHeight.getY() < 0 ?
                                  -m_aCharRect.Width() : m_aCharRect.Height());
                m_pVisCrsr->Show(); // show again
            }
            m_eMvState = MV_NONE;  // state for cursor travelling - GetCrsrOfst
            if( pTblFrm && Imp()->IsAccessible() )
                Imp()->InvalidateAccessibleCursorPosition( pTblFrm );
            return;
        }
    }

    if( m_pTblCrsr )
    {
        // delete Ring
        while( m_pCurCrsr->GetNext() != m_pCurCrsr )
            delete m_pCurCrsr->GetNext();
        m_pCurCrsr->DeleteMark();
        *m_pCurCrsr->GetPoint() = *m_pTblCrsr->GetPoint();
        m_pCurCrsr->GetPtPos() = m_pTblCrsr->GetPtPos();
        delete m_pTblCrsr, m_pTblCrsr = 0;
    }

    m_pVisCrsr->Hide(); // always hide visible Cursor

    // are we perhaps in a protected / hidden Section ?
    {
        SwShellCrsr* pShellCrsr = getShellCrsr( true );
        bool bChgState = true;
        const SwSectionNode* pSectNd = pShellCrsr->GetNode()->FindSectionNode();
        if( pSectNd && ( pSectNd->GetSection().IsHiddenFlag() ||
            ( !IsReadOnlyAvailable() &&
              pSectNd->GetSection().IsProtectFlag() &&
             ( !mpDoc->GetDocShell() ||
               !mpDoc->GetDocShell()->IsReadOnly() || m_bAllProtect )) ) )
        {
            if( !FindValidCntntNode( !HasDrawView() ||
                    0 == Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount()))
            {
                // everything protected/hidden -> special mode
                if( m_bAllProtect && !IsReadOnlyAvailable() &&
                    pSectNd->GetSection().IsProtectFlag() )
                    bChgState = false;
                else
                {
                    m_eMvState = MV_NONE;     // state for cursor travelling
                    m_bAllProtect = sal_True;
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
            sal_Bool bWasAllProtect = m_bAllProtect;
            m_bAllProtect = sal_False;
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
    SwPaM* pCmp = m_pCurCrsr;
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
    while( pCmp != m_pCurCrsr );

    SwRect aOld( m_aCharRect );
    bool bFirst = true;
    SwCntntFrm *pFrm;
    int nLoopCnt = 100;
    SwShellCrsr* pShellCrsr = getShellCrsr( true );

    do {
        bool bAgainst;
        do {
            bAgainst = false;
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
                (!mpDoc->GetDocShell() ||
                 !mpDoc->GetDocShell()->IsReadOnly() || m_bAllProtect ) )
            {
                // look for a valid position
                bool bChgState = true;
                if( !FindValidCntntNode(!HasDrawView() ||
                    0 == Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount()))
                {
                    // everything is protected / hidden -> special Mode
                    if( m_bAllProtect )
                        bChgState = false;
                    else
                    {
                        m_eMvState = MV_NONE;     // state for crusor travelling
                        m_bAllProtect = sal_True;
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
                    sal_Bool bWasAllProtect = m_bAllProtect;
                    m_bAllProtect = sal_False;
                    if( bWasAllProtect && GetDoc()->GetDocShell() &&
                        GetDoc()->GetDocShell()->IsReadOnlyUI() )
                    {
                        GetDoc()->GetDocShell()->SetReadOnlyUI( sal_False );
                        CallChgLnk();       // notify UI!
                    }
                    m_bAllProtect = sal_False;
                    bAgainst = true; // look for the right Frm again
                }
            }
        } while( bAgainst );

        if( !( eFlags & SwCrsrShell::NOCALRECT ))
        {
            SwCrsrMoveState aTmpState( m_eMvState );
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

            if( !pFrm->GetCharRect( m_aCharRect, *pShellCrsr->GetPoint(), &aTmpState ) )
            {
                Point& rPt = pShellCrsr->GetPtPos();
                rPt = m_aCharRect.Center();
                pFrm->GetCrsrOfst( pShellCrsr->GetPoint(), rPt, &aTmpState );
            }

            if( !pShellCrsr->HasMark() )
                m_aCrsrHeight = aTmpState.aRealHeight;
            else
            {
                m_aCrsrHeight.setX(0);
                m_aCrsrHeight.setY(aTmpState.aRealHeight.getY() < 0 ?
                                  -m_aCharRect.Width() : m_aCharRect.Height());
            }
        }
        else
        {
            m_aCrsrHeight.setX(0);
            m_aCrsrHeight.setY(m_aCharRect.Height());
        }

        if( !bFirst && aOld == m_aCharRect )
            break;

        // if the layout says that we are after the 100th iteration still in
        // flow then we should always take the current position for granted.
        // (see bug: 29658)
        if( !--nLoopCnt )
        {
            OSL_ENSURE( !this, "endless loop? CharRect != OldCharRect ");
            break;
        }
        aOld = m_aCharRect;
        bFirst = false;

        // update cursor Points to the new Positions
        pShellCrsr->GetPtPos().setX(m_aCharRect.Left());
        pShellCrsr->GetPtPos().setY(m_aCharRect.Top());

        if( !(eFlags & SwCrsrShell::UPDOWN ))   // delete old Pos. of Up/Down
        {
            pFrm->Calc();
            m_nUpDownX = pFrm->IsVertical() ?
                       m_aCharRect.Top() - pFrm->Frm().Top() :
                       m_aCharRect.Left() - pFrm->Frm().Left();
        }

        // scroll Cursor to visible area
        if( m_bHasFocus && eFlags & SwCrsrShell::SCROLLWIN &&
            (HasSelection() || eFlags & SwCrsrShell::READONLY ||
             !IsCrsrReadonly() || GetViewOptions()->IsSelectionInReadonly()) )
        {
            // in case of scrolling this EndAction doesn't show the SV cursor
            // again, thus save and reset the flag here
            sal_Bool bSav = m_bSVCrsrVis;
            m_bSVCrsrVis = sal_False;
            MakeSelVisible();
            m_bSVCrsrVis = bSav;
        }

    } while( eFlags & SwCrsrShell::SCROLLWIN );

    if( m_pBlockCrsr )
        RefreshBlockCursor();

    if( !bIdleEnd && m_bHasFocus && !m_bBasicHideCrsr )
    {
        if( m_pTblCrsr )
            m_pTblCrsr->SwSelPaintRects::Show();
        else
        {
            m_pCurCrsr->SwSelPaintRects::Show();
            if( m_pBlockCrsr )
            {
                SwShellCrsr* pNxt = dynamic_cast<SwShellCrsr*>(m_pCurCrsr->GetNext());
                while( pNxt && pNxt != m_pCurCrsr )
                {
                    pNxt->SwSelPaintRects::Show();
                    pNxt = dynamic_cast<SwShellCrsr*>(pNxt->GetNext());
                }
            }
        }
    }

    m_eMvState = MV_NONE; // state for cursor tavelling - GetCrsrOfst

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

    if( m_bSVCrsrVis )
        m_pVisCrsr->Show(); // show again
}

void SwCrsrShell::RefreshBlockCursor()
{
    OSL_ENSURE( m_pBlockCrsr, "Don't call me without a block cursor" );
    SwShellCrsr &rBlock = m_pBlockCrsr->getShellCrsr();
    Point aPt = rBlock.GetPtPos();
    SwCntntFrm* pFrm = rBlock.GetCntntNode()->getLayoutFrm( GetLayout(), &aPt, rBlock.GetPoint(), sal_False );
    Point aMk;
    if( m_pBlockCrsr->getEndPoint() && m_pBlockCrsr->getStartPoint() )
    {
        aPt = *m_pBlockCrsr->getStartPoint();
        aMk = *m_pBlockCrsr->getEndPoint();
    }
    else
    {
        aPt = rBlock.GetPtPos();
        if( pFrm )
        {
            if( pFrm->IsVertical() )
                aPt.setY(pFrm->Frm().Top() + GetUpDownX());
            else
                aPt.setX(pFrm->Frm().Left() + GetUpDownX());
        }
        aMk = rBlock.GetMkPos();
    }
    SwRect aRect( aMk, aPt );
    aRect.Justify();
    SwSelectionList aSelList( pFrm );

    if( GetLayout()->FillSelection( aSelList, aRect ) )
    {
        SwCursor* pNxt = (SwCursor*)m_pCurCrsr->GetNext();
        while( pNxt != m_pCurCrsr )
        {
            delete pNxt;
            pNxt = (SwCursor*)m_pCurCrsr->GetNext();
        }

        std::list<SwPaM*>::iterator pStart = aSelList.getStart();
        std::list<SwPaM*>::iterator pPam = aSelList.getEnd();
        OSL_ENSURE( pPam != pStart, "FillSelection should deliver at least one PaM" );
        m_pCurCrsr->SetMark();
        --pPam;
        // If there is only one text portion inside the rectangle, a simple
        // selection is created
        if( pPam == pStart )
        {
            *m_pCurCrsr->GetPoint() = *(*pPam)->GetPoint();
            if( (*pPam)->HasMark() )
                *m_pCurCrsr->GetMark() = *(*pPam)->GetMark();
            else
                m_pCurCrsr->DeleteMark();
            delete *pPam;
            m_pCurCrsr->SetColumnSelection( false );
        }
        else
        {
            // The order of the SwSelectionList has to be preserved but
            // the order inside the ring created by CreateCrsr() is not like
            // exspected => First create the selections before the last one
            // downto the first selection.
            // At least create the cursor for the last selection
            --pPam;
            *m_pCurCrsr->GetPoint() = *(*pPam)->GetPoint(); // n-1 (if n == number of selections)
            if( (*pPam)->HasMark() )
                *m_pCurCrsr->GetMark() = *(*pPam)->GetMark();
            else
                m_pCurCrsr->DeleteMark();
            delete *pPam;
            m_pCurCrsr->SetColumnSelection( true );
            while( pPam != pStart )
            {
                --pPam;

                SwShellCrsr* pNew = new SwShellCrsr( *m_pCurCrsr );
                pNew->insert( pNew->begin(), m_pCurCrsr->begin(),  m_pCurCrsr->end());
                m_pCurCrsr->clear();
                m_pCurCrsr->DeleteMark();

                *m_pCurCrsr->GetPoint() = *(*pPam)->GetPoint(); // n-2, n-3, .., 2, 1
                if( (*pPam)->HasMark() )
                {
                    m_pCurCrsr->SetMark();
                    *m_pCurCrsr->GetMark() = *(*pPam)->GetMark();
                }
                else
                    m_pCurCrsr->DeleteMark();
                m_pCurCrsr->SetColumnSelection( true );
                delete *pPam;
            }
            {
                SwShellCrsr* pNew = new SwShellCrsr( *m_pCurCrsr );
                pNew->insert( pNew->begin(), m_pCurCrsr->begin(), m_pCurCrsr->end() );
                m_pCurCrsr->clear();
                m_pCurCrsr->DeleteMark();
            }
            pPam = aSelList.getEnd();
            --pPam;
            *m_pCurCrsr->GetPoint() = *(*pPam)->GetPoint(); // n, the last selection
            if( (*pPam)->HasMark() )
            {
                m_pCurCrsr->SetMark();
                *m_pCurCrsr->GetMark() = *(*pPam)->GetMark();
            }
            else
                m_pCurCrsr->DeleteMark();
            m_pCurCrsr->SetColumnSelection( true );
            delete *pPam;
        }
    }
}

/// create a copy of the cursor and save it in the stack
void SwCrsrShell::Push()
{
    // fdo#60513: if we have a table cursor, copy that; else copy current.
    // This seems to work because UpdateCrsr() will fix this up on Pop(),
    // then MakeBoxSels() will re-create the current m_pCurCrsr cell ring.
    SwShellCrsr *const pCurrent((m_pTblCrsr) ? m_pTblCrsr : m_pCurCrsr);
    m_pCrsrStk = new SwShellCrsr( *this, *pCurrent->GetPoint(),
                                    pCurrent->GetPtPos(), m_pCrsrStk );

    if (pCurrent->HasMark())
    {
        m_pCrsrStk->SetMark();
        *m_pCrsrStk->GetMark() = *pCurrent->GetMark();
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
    if( 0 == m_pCrsrStk )
        return sal_False;

    SwShellCrsr *pTmp = 0, *pOldStk = m_pCrsrStk;

    // the successor becomes the current one
    if( m_pCrsrStk->GetNext() != m_pCrsrStk )
    {
        pTmp = dynamic_cast<SwShellCrsr*>(m_pCrsrStk->GetNext());
    }

    if( bOldCrsr ) // delete from stack
        delete m_pCrsrStk;

    m_pCrsrStk = pTmp; // assign new one

    if( !bOldCrsr )
    {
        SwCrsrSaveState aSaveState( *m_pCurCrsr );

        // If the visible SSelection was not changed
        if( pOldStk->GetPtPos() == m_pCurCrsr->GetPtPos() ||
            pOldStk->GetPtPos() == m_pCurCrsr->GetMkPos() )
        {
            // move "Selections Rectangles"
            m_pCurCrsr->insert( m_pCurCrsr->begin(), pOldStk->begin(), pOldStk->end() );
            pOldStk->clear();
        }

        if( pOldStk->HasMark() )
        {
            m_pCurCrsr->SetMark();
            *m_pCurCrsr->GetMark() = *pOldStk->GetMark();
            m_pCurCrsr->GetMkPos() = pOldStk->GetMkPos();
        }
        else
            // no selection so revoke old one and set to old position
            m_pCurCrsr->DeleteMark();
        *m_pCurCrsr->GetPoint() = *pOldStk->GetPoint();
        m_pCurCrsr->GetPtPos() = pOldStk->GetPtPos();
        delete pOldStk;

        if( !m_pCurCrsr->IsInProtectTable( sal_True ) &&
            !m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
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
    if( 0 == m_pCrsrStk )
        return;

    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    // rhbz#689053: IsSelOvr must restore the saved stack position, not the
    // current one, because current point + stack mark may be invalid PaM
    SwCrsrSaveState aSaveState(*m_pCrsrStk);
    // stack cursor & current cursor in same Section?
    assert(!m_pCrsrStk->HasMark() ||
            CheckNodesRange(m_pCrsrStk->GetMark()->nNode,
                            m_pCurCrsr->GetPoint()->nNode, true));
    *m_pCrsrStk->GetPoint() = *m_pCurCrsr->GetPoint();
    m_pCrsrStk->GetPtPos() = m_pCurCrsr->GetPtPos();

    SwShellCrsr * pTmp = 0;
    if( m_pCrsrStk->GetNext() != m_pCrsrStk )
    {
        pTmp = dynamic_cast<SwShellCrsr*>(m_pCrsrStk->GetNext());
    }
    delete m_pCurCrsr;
    m_pCurCrsr = m_pCrsrStk;
    m_pCrsrStk->MoveTo(0); // remove from ring
    m_pCrsrStk = pTmp;
    if( !m_pCurCrsr->IsInProtectTable( sal_True ) &&
        !m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                             nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ) )
    {
        UpdateCrsr(); // update current cursor
    }
}

void SwCrsrShell::HideCrsrs()
{
    if( !m_bHasFocus || m_bBasicHideCrsr )
        return;

    // if cursor is visible then hide SV cursor
    if( m_pVisCrsr->IsVisible() )
    {
        SET_CURR_SHELL( this );
        m_pVisCrsr->Hide();
    }
    // revoke inversion of SSelection
    SwShellCrsr* pAktCrsr = m_pTblCrsr ? m_pTblCrsr : m_pCurCrsr;
    pAktCrsr->Hide();
}

void SwCrsrShell::ShowCrsrs( sal_Bool bCrsrVis )
{
    if( !m_bHasFocus || m_bAllProtect || m_bBasicHideCrsr )
        return;

    SET_CURR_SHELL( this );
    SwShellCrsr* pAktCrsr = m_pTblCrsr ? m_pTblCrsr : m_pCurCrsr;
    pAktCrsr->Show();

    if( m_bSVCrsrVis && bCrsrVis ) // also show SV cursor again
        m_pVisCrsr->Show();
}

void SwCrsrShell::ShowCrsr()
{
    if( !m_bBasicHideCrsr )
    {
        m_bSVCrsrVis = sal_True;
        UpdateCrsr();
    }
}

void SwCrsrShell::HideCrsr()
{
    if( !m_bBasicHideCrsr )
    {
        m_bSVCrsrVis = sal_False;
        // possibly reverse selected areas!!
        SET_CURR_SHELL( this );
        m_pVisCrsr->Hide();
    }
}

void SwCrsrShell::ShLooseFcs()
{
    if( !m_bBasicHideCrsr )
        HideCrsrs();
    m_bHasFocus = sal_False;
}

void SwCrsrShell::ShGetFcs( sal_Bool bUpdate )
{
    m_bHasFocus = sal_True;
    if( !m_bBasicHideCrsr && VisArea().Width() )
    {
        UpdateCrsr( static_cast<sal_uInt16>( bUpdate ?
                    SwCrsrShell::CHKRANGE|SwCrsrShell::SCROLLWIN
                    : SwCrsrShell::CHKRANGE ) );
        ShowCrsrs( m_bSVCrsrVis ? sal_True : sal_False );
    }
}

/** Get current frame in which the cursor is positioned. */
SwCntntFrm *SwCrsrShell::GetCurrFrm( const sal_Bool bCalcFrm ) const
{
    SET_CURR_SHELL( (ViewShell*)this );
    SwCntntFrm *pRet = 0;
    SwCntntNode *pNd = m_pCurCrsr->GetCntntNode();
    if ( pNd )
    {
        if ( bCalcFrm )
        {
            const sal_uInt16* pST = &mnStartAction;
            ++(*((sal_uInt16*)pST));
            const Size aOldSz( GetDocSize() );
            pRet = pNd->getLayoutFrm( GetLayout(), &m_pCurCrsr->GetPtPos(), m_pCurCrsr->GetPoint() );
            --(*((sal_uInt16*)pST));
            if( aOldSz != GetDocSize() )
                ((SwCrsrShell*)this)->SizeChgNotify();
        }
        else
            pRet = pNd->getLayoutFrm( GetLayout(), &m_pCurCrsr->GetPtPos(), m_pCurCrsr->GetPoint(), sal_False);
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

    if( m_bCallChgLnk &&
        ( nWhich < RES_MSG_BEGIN || nWhich >= RES_MSG_END ||
            nWhich == RES_FMT_CHG || nWhich == RES_UPDATE_ATTR ||
            nWhich == RES_ATTRSET_CHG ))
        // messages are not forwarded
        // #i6681#: RES_UPDATE_ATTR is implicitly unset in
        // SwTxtNode::Insert(SwTxtHint*, sal_uInt16); we react here and thus do
        // not need to send the expensive RES_FMT_CHG in Insert.
        CallChgLnk();

    if( m_aGrfArrivedLnk.IsSet() &&
        ( RES_GRAPHIC_ARRIVED == nWhich || RES_GRAPHIC_SWAPIN == nWhich ))
        m_aGrfArrivedLnk.Call( this );
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
        m_bChgCallFlag = sal_True; // remember change
    else if( m_aChgLnk.IsSet() )
    {
        if( m_bCallChgLnk )
            m_aChgLnk.Call( this );
        m_bChgCallFlag = sal_False; // reset flag
    }
}

/// get selected text of a node at current cursor
String SwCrsrShell::GetSelTxt() const
{
    String aTxt;
    if( m_pCurCrsr->GetPoint()->nNode.GetIndex() ==
        m_pCurCrsr->GetMark()->nNode.GetIndex() )
    {
        SwTxtNode* pTxtNd = m_pCurCrsr->GetNode()->GetTxtNode();
        if( pTxtNd )
        {
            xub_StrLen nStt = m_pCurCrsr->Start()->nContent.GetIndex();
            aTxt = pTxtNd->GetExpandTxt( nStt,
                    m_pCurCrsr->End()->nContent.GetIndex() - nStt );
        }
    }
    return aTxt;
}

/// get text only from current cursor position (until end of node)
String SwCrsrShell::GetText() const
{
    String aTxt;
    if( m_pCurCrsr->GetPoint()->nNode.GetIndex() ==
        m_pCurCrsr->GetMark()->nNode.GetIndex() )
    {
        SwTxtNode* pTxtNd = m_pCurCrsr->GetNode()->GetTxtNode();
        if( pTxtNd )
            aTxt = pTxtNd->GetTxt().copy(
                    m_pCurCrsr->GetPoint()->nContent.GetIndex() );
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

    const SwPosition* pPos = !m_pCurCrsr->HasMark() ? m_pCurCrsr->GetPoint()
                                : bEnd ? m_pCurCrsr->End() : m_pCurCrsr->Start();
    SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
    if( !pTxtNd )
        return 0;

    xub_StrLen nPos = pPos->nContent.GetIndex();
    const OUString& rStr = pTxtNd->GetTxt();
    sal_Unicode cCh = 0;

    if (((nPos+nOffset) >= 0 ) && (nPos+nOffset) < rStr.getLength())
        cCh = rStr[nPos + nOffset];

    return cCh;
}

/** extend current SSelection by n characters

    @param bEnd   Start counting from the end? From start otherwise.
    @param nCount Number of characters.
*/
sal_Bool SwCrsrShell::ExtendSelection( sal_Bool bEnd, xub_StrLen nCount )
{
    if( !m_pCurCrsr->HasMark() || IsTableMode() )
        return sal_False; // no selection

    SwPosition* pPos = bEnd ? m_pCurCrsr->End() : m_pCurCrsr->Start();
    SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
    OSL_ENSURE( pTxtNd, "no text node; how should this then be extended?" );

    xub_StrLen nPos = pPos->nContent.GetIndex();
    if( bEnd )
    {
        if ((nPos + nCount) <= pTxtNd->GetTxt().getLength())
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
    SwPosition aPos( *m_pCurCrsr->GetPoint() );
    SwCrsrMoveState aTmpState( MV_SETONLYTEXT );
    aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
    aTmpState.bRealHeight = sal_True;

    sal_Bool bRet = GetLayout()->GetCrsrOfst( &aPos, aPt /*, &aTmpState*/ );

    SetInFrontOfLabel( false ); // #i27615#

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
    SwRect aTmp( m_aCharRect );

    pFrm->GetCharRect( m_aCharRect, aPos, &aTmpState );

    // #i10137#
    if( aTmp == m_aCharRect && m_pVisCrsr->IsVisible() )
        return sal_True;

    m_pVisCrsr->Hide(); // always hide visible cursor
    if( IsScrollMDI( this, m_aCharRect ))
    {
        MakeVisible( m_aCharRect );
        m_pCurCrsr->Show();
    }

    {
        if( aTmpState.bRealHeight )
            m_aCrsrHeight = aTmpState.aRealHeight;
        else
        {
            m_aCrsrHeight.setX(0);
            m_aCrsrHeight.setY(m_aCharRect.Height());
        }

        m_pVisCrsr->SetDragCrsr( true );
        m_pVisCrsr->Show(); // show again
    }
    return bRet;
}

sal_Bool SwCrsrShell::IsOverReadOnlyPos( const Point& rPt ) const
{
    Point aPt( rPt );
    SwPaM aPam( *m_pCurCrsr->GetPoint() );
    GetLayout()->GetCrsrOfst( aPam.GetPoint(), aPt );
    // Formular view
    return aPam.HasReadonlySel( GetViewOptions()->IsFormView() );
}

/** Get the number of elements in the ring of cursors

    @param bAll If <false> get only spanned ones (= with selections) (Basic).
*/
sal_uInt16 SwCrsrShell::GetCrsrCnt( sal_Bool bAll ) const
{
    Ring* pTmp = GetCrsr()->GetNext();
    sal_uInt16 n = (bAll || ( m_pCurCrsr->HasMark() &&
                    *m_pCurCrsr->GetPoint() != *m_pCurCrsr->GetMark())) ? 1 : 0;
    while( pTmp != m_pCurCrsr )
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
    if( m_pCurCrsr->GetPoint()->nContent.GetIndex() )
        return sal_False;

    // after EndOfIcons comes the content selection (EndNd+StNd+CntntNd)
    SwNodeIndex aIdx( GetDoc()->GetNodes().GetEndOfExtras(), 2 );
    if( !aIdx.GetNode().IsCntntNode() )
        GetDoc()->GetNodes().GoNext( &aIdx );
    return aIdx == m_pCurCrsr->GetPoint()->nNode;
}

sal_Bool SwCrsrShell::IsEndOfDoc() const
{
    SwNodeIndex aIdx( GetDoc()->GetNodes().GetEndOfContent(), -1 );
    SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = GetDoc()->GetNodes().GoPrevious( &aIdx );

    return aIdx == m_pCurCrsr->GetPoint()->nNode &&
            pCNd->Len() == m_pCurCrsr->GetPoint()->nContent.GetIndex();
}

/** Invalidate cursors

    Delete all created cursors, set table crsr and last crsr to their TextNode
    (or StartNode?). They will then all re-created at the next ::GetCrsr() call.

    This is needed for Drag&Drop/ Clipboard-paste in tables.
*/
sal_Bool SwCrsrShell::ParkTblCrsr()
{
    if( !m_pTblCrsr )
        return sal_False;

    m_pTblCrsr->ParkCrsr();

    while( m_pCurCrsr->GetNext() != m_pCurCrsr )
        delete m_pCurCrsr->GetNext();

    // *always* move cursor's Point and Mark
    m_pCurCrsr->DeleteMark();
    *m_pCurCrsr->GetPoint() = *m_pTblCrsr->GetPoint();

    return sal_True;
}

void SwCrsrShell::_ParkPams( SwPaM* pDelRg, SwShellCrsr** ppDelRing )
{
    const SwPosition *pStt = pDelRg->Start(),
        *pEnd = pDelRg->GetPoint() == pStt ? pDelRg->GetMark() : pDelRg->GetPoint();

    SwPaM *pTmpDel = 0, *pTmp = *ppDelRing;

    // search over the whole ring
    bool bGoNext;
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

        bGoNext = true;
        if (pTmpDel) // is the pam in the range -> delete
        {
            sal_Bool bDelete = sal_True;
            if( *ppDelRing == pTmpDel )
            {
                if( *ppDelRing == m_pCurCrsr )
                {
                    if( sal_True == ( bDelete = GoNextCrsr() ))
                    {
                        bGoNext = false;
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
                pTmpDel->DeleteMark();
            }
            pTmpDel = 0;
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
            if( pSh->m_pCrsrStk )
                pSh->_ParkPams( pNew, &pSh->m_pCrsrStk );

            pSh->_ParkPams( pNew, &pSh->m_pCurCrsr );
            if( pSh->m_pTblCrsr )
            {
                // set table cursor always to 0 and the current one always to
                // the beginning of the table
                SwPaM* pTCrsr = pSh->GetTblCrs();
                SwNode* pTblNd = pTCrsr->GetPoint()->nNode.GetNode().FindTableNode();
                if ( pTblNd )
                {
                    pTCrsr->DeleteMark();
                    pSh->m_pCurCrsr->GetPoint()->nNode = *pTblNd;
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
    SwModify( 0 ), m_pCrsrStk( 0 ), m_pBlockCrsr( 0 ), m_pTblCrsr( 0 ),
    m_pBoxIdx( 0 ), m_pBoxPtr( 0 ), m_nCrsrMove( 0 ), m_nBasicActionCnt( 0 ),
    m_eMvState( MV_NONE ),
    m_sMarkedListId(),
    m_nMarkedListLevel( 0 )
{
    SET_CURR_SHELL( this );
    // only keep the position of the current cursor of the copy shell
    m_pCurCrsr = new SwShellCrsr( *this, *(rShell.m_pCurCrsr->GetPoint()) );
    m_pCurCrsr->GetCntntNode()->Add( this );

    m_bAllProtect = m_bVisPortChgd = m_bChgCallFlag = m_bInCMvVisportChgd =
    m_bGCAttr = m_bIgnoreReadonly = m_bSelTblCells = m_bBasicHideCrsr =
    m_bOverwriteCrsr = sal_False;
    m_bCallChgLnk = m_bHasFocus = m_bSVCrsrVis = m_bAutoUpdateCells = sal_True;
    m_bSetCrsrInReadOnly = sal_True;
    m_pVisCrsr = new SwVisCrsr( this );
    m_bMacroExecAllowed = rShell.IsMacroExecAllowed();
}

/// default constructor
SwCrsrShell::SwCrsrShell( SwDoc& rDoc, Window *pInitWin,
                            const SwViewOption *pInitOpt )
    : ViewShell( rDoc, pInitWin, pInitOpt ),
    SwModify( 0 ), m_pCrsrStk( 0 ), m_pBlockCrsr( 0 ), m_pTblCrsr( 0 ),
    m_pBoxIdx( 0 ), m_pBoxPtr( 0 ), m_nCrsrMove( 0 ), m_nBasicActionCnt( 0 ),
    m_eMvState( MV_NONE ), // state for crsr-travelling - GetCrsrOfst
    m_sMarkedListId(),
    m_nMarkedListLevel( 0 )
{
    SET_CURR_SHELL( this );
    // create initial cursor and set it to first content position
    SwNodes& rNds = rDoc.GetNodes();

    SwNodeIndex aNodeIdx( *rNds.GetEndOfContent().StartOfSectionNode() );
    SwCntntNode* pCNd = rNds.GoNext( &aNodeIdx ); // go to the first ContentNode

    m_pCurCrsr = new SwShellCrsr( *this, SwPosition( aNodeIdx, SwIndex( pCNd, 0 )));

    // Register shell as dependent at current node. As a result all attribute
    // changes can be forwarded via the Link.
    pCNd->Add( this );

    m_bAllProtect = m_bVisPortChgd = m_bChgCallFlag = m_bInCMvVisportChgd =
    m_bGCAttr = m_bIgnoreReadonly = m_bSelTblCells = m_bBasicHideCrsr =
    m_bOverwriteCrsr = sal_False;
    m_bCallChgLnk = m_bHasFocus = m_bSVCrsrVis = m_bAutoUpdateCells = sal_True;
    m_bSetCrsrInReadOnly = sal_True;

    m_pVisCrsr = new SwVisCrsr( this );
    m_bMacroExecAllowed = true;
}

SwCrsrShell::~SwCrsrShell()
{
    // if it is not the last view then at least the field should be updated
    if( GetNext() != this )
        CheckTblBoxCntnt( m_pCurCrsr->GetPoint() );
    else
        ClearTblBoxCntnt();

    delete m_pVisCrsr;
    delete m_pBlockCrsr;
    delete m_pTblCrsr;

    // release cursors
    while(m_pCurCrsr->GetNext() != m_pCurCrsr)
        delete m_pCurCrsr->GetNext();
    delete m_pCurCrsr;

    // free stack
    if( m_pCrsrStk )
    {
        while( m_pCrsrStk->GetNext() != m_pCrsrStk )
            delete m_pCrsrStk->GetNext();
        delete m_pCrsrStk;
    }

    // #i54025# - do not give a HTML parser that might potentially hang as
    // a client at the cursor shell the chance to hang itself on a TextNode
    if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );
}

SwShellCrsr* SwCrsrShell::getShellCrsr( bool bBlock )
{
    if( m_pTblCrsr )
        return m_pTblCrsr;
    if( m_pBlockCrsr && bBlock )
        return &m_pBlockCrsr->getShellCrsr();
    return m_pCurCrsr;
}

/** Should WaitPtr be switched on for the clipboard?

    Wait for TableMode, multiple selections and more than x selected paragraphs.
*/
bool SwCrsrShell::ShouldWait() const
{
    if ( IsTableMode() || GetCrsrCnt() > 1 )
        return true;

    if( HasDrawView() && GetDrawView()->GetMarkedObjectList().GetMarkCount() )
        return true;

    SwPaM* pPam = GetCrsr();
    return pPam->Start()->nNode.GetIndex() + 10 <
            pPam->End()->nNode.GetIndex();
}

size_t SwCrsrShell::UpdateTblSelBoxes()
{
    if (m_pTblCrsr && (m_pTblCrsr->IsChgd() || !m_pTblCrsr->GetSelectedBoxesCount()))
    {
         GetLayout()->MakeTblCrsrs( *m_pTblCrsr );
    }
    return (m_pTblCrsr) ? m_pTblCrsr->GetSelectedBoxesCount() : 0;
}

/// show the current selected "object"
void SwCrsrShell::MakeSelVisible()
{
    OSL_ENSURE( m_bHasFocus, "no focus but cursor should be made visible?" );
    if( m_aCrsrHeight.Y() < m_aCharRect.Height() && m_aCharRect.Height() > VisArea().Height() )
    {
        SwRect aTmp( m_aCharRect );
        long nDiff = m_aCharRect.Height() - VisArea().Height();
        if( nDiff < m_aCrsrHeight.getX() )
            aTmp.Top( nDiff + m_aCharRect.Top() );
        else
        {
            aTmp.Top( m_aCrsrHeight.getX() + m_aCharRect.Top() );
            aTmp.Height( m_aCrsrHeight.getY() );
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
        if( m_aCharRect.HasArea() )
            MakeVisible( m_aCharRect );
        else
        {
            SwRect aTmp( m_aCharRect );
            aTmp.SSize().Height() += 1; aTmp.SSize().Width() += 1;
            MakeVisible( aTmp );
        }
    }
}

/// search a valid content position (not protected/hidden)
sal_Bool SwCrsrShell::FindValidCntntNode( sal_Bool bOnlyText )
{
    if( m_pTblCrsr )
    {
        OSL_ENSURE( !this, "Did not remove table selection!" );
        return sal_False;
    }

    // #i45129# - everything is allowed in UI-readonly
    if( !m_bAllProtect && GetDoc()->GetDocShell() &&
        GetDoc()->GetDocShell()->IsReadOnlyUI() )
        return sal_True;

    if( m_pCurCrsr->HasMark() )
        ClearMark();

    // first check for frames
    SwNodeIndex& rNdIdx = m_pCurCrsr->GetPoint()->nNode;
    sal_uLong nNdIdx = rNdIdx.GetIndex(); // keep backup
    SwNodes& rNds = mpDoc->GetNodes();
    SwCntntNode* pCNd = rNdIdx.GetNode().GetCntntNode();
    const SwCntntFrm * pFrm;

    if( pCNd && 0 != (pFrm = pCNd->getLayoutFrm( GetLayout(),0,m_pCurCrsr->GetPoint(),sal_False)) &&
        !IsReadOnlyAvailable() && pFrm->IsProtected() &&
        nNdIdx < rNds.GetEndOfExtras().GetIndex() )
    {
        // skip protected frame
        SwPaM aPam( *m_pCurCrsr->GetPoint() );
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
        *m_pCurCrsr->GetPoint() = *aPam.GetPoint();
    }
    else if( bOnlyText && pCNd && pCNd->IsNoTxtNode() )
    {
        // set to beginning of document
        rNdIdx = mpDoc->GetNodes().GetEndOfExtras();
        m_pCurCrsr->GetPoint()->nContent.Assign( mpDoc->GetNodes().GoNext(
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
            *m_pCurCrsr->GetPoint() = *aPam.GetPoint();
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
                        SwCrsrSaveState aSaveState( *m_pCurCrsr );
                        aTmp.nNdTyp = 0; // don't do anything in DTOR
                        if( !m_pCurCrsr->IsInProtectTable( sal_True, sal_True ) )
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
        m_pCurCrsr->GetPoint()->nContent.Assign( pCNd, nCntnt );
    }
    else
    {
        pCNd = rNdIdx.GetNode().GetCntntNode();
        // if cursor in hidden frame, always move it
        if( !pCNd || !pCNd->getLayoutFrm( GetLayout(),0,0,sal_False) )
        {
            SwCrsrMoveState aTmpState( MV_NONE );
            aTmpState.bSetInReadOnly = IsReadOnlyAvailable();
            GetLayout()->GetCrsrOfst( m_pCurCrsr->GetPoint(), m_pCurCrsr->GetPtPos(),
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
        bFlag != m_bSetCrsrInReadOnly )
    {
        // If the flag is switched off then all selections need to be
        // invalidated. Otherwise we would trust that nothing protected is selected.
        if( !bFlag )
        {
            ClearMark();
        }
        m_bSetCrsrInReadOnly = bFlag;
        UpdateCrsr();
    }
}

sal_Bool SwCrsrShell::HasReadonlySel(bool bAnnotationMode) const
{
    sal_Bool bRet = sal_False;
    if( IsReadOnlyAvailable() || GetViewOptions()->IsFormView() )
    {
        if( m_pTblCrsr )
            bRet = m_pTblCrsr->HasReadOnlyBoxSel() ||
                   m_pTblCrsr->HasReadonlySel( GetViewOptions()->IsFormView() );
        else
        {
            const SwPaM* pCrsr = m_pCurCrsr;

            do {
                if( pCrsr->HasReadonlySel( GetViewOptions()->IsFormView(), bAnnotationMode ) )
                    bRet = sal_True;
            } while( !bRet && m_pCurCrsr != ( pCrsr = (SwPaM*)pCrsr->GetNext() ));
        }
    }
    return bRet;
}

sal_Bool SwCrsrShell::IsSelFullPara() const
{
    sal_Bool bRet = sal_False;

    if( m_pCurCrsr->GetPoint()->nNode.GetIndex() ==
        m_pCurCrsr->GetMark()->nNode.GetIndex() && m_pCurCrsr == m_pCurCrsr->GetNext() )
    {
        xub_StrLen nStt = m_pCurCrsr->GetPoint()->nContent.GetIndex(),
                   nEnd = m_pCurCrsr->GetMark()->nContent.GetIndex();
        if( nStt > nEnd )
        {
            xub_StrLen nTmp = nStt;
            nStt = nEnd;
            nEnd = nTmp;
        }
        const SwCntntNode* pCNd = m_pCurCrsr->GetCntntNode();
        bRet = pCNd && !nStt && nEnd == pCNd->Len();
    }
    return bRet;
}

short SwCrsrShell::GetTextDirection( const Point* pPt ) const
{
    SwPosition aPos( *m_pCurCrsr->GetPoint() );
    Point aPt( pPt ? *pPt : m_pCurCrsr->GetPtPos() );
    if( pPt )
    {
        SwCrsrMoveState aTmpState( MV_NONE );
        aTmpState.bSetInReadOnly = IsReadOnlyAvailable();

        GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState );
    }

    return mpDoc->GetTextDirection( aPos, &aPt );
}

bool SwCrsrShell::IsInVerticalText( const Point* pPt ) const
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
    if ( !GetViewOptions()->IsShowHiddenChar() && !m_pCurCrsr->HasMark() )
    {
        SwPosition& rPt = *(SwPosition*)m_pCurCrsr->GetPoint();
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
                m_pCurCrsr->SetMark();
                m_pCurCrsr->GetMark()->nContent = nHiddenEnd;
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
    if( m_pTblCrsr )
        GetCrsr();
    delete m_pTblCrsr, m_pTblCrsr = 0;
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    sal_uLong nRet = m_pCurCrsr->Find( rSearchOpt, bSearchInNotes, eStart, eEnd,
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
    if( m_pTblCrsr )
        GetCrsr();
    delete m_pTblCrsr, m_pTblCrsr = 0;
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    sal_uLong nRet = m_pCurCrsr->Find( rFmtColl, eStart, eEnd, bCancel, eRng,
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
    if( m_pTblCrsr )
        GetCrsr();
    delete m_pTblCrsr, m_pTblCrsr = 0;
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
    sal_uLong nRet = m_pCurCrsr->Find( rSet, bNoCollections, eStart, eEnd,
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

static const SwStartNode* lcl_NodeContext( const SwNode& rNode )
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
bool sw_PosOk(const SwPosition & aPos)
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
    return sw_PosOk(*aPam.GetPoint()) && (! aPam.HasMark()
        || sw_PosOk(*aPam.GetMark()));
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

    if( pStartCrsr->HasMark() && !sw_PosOk( *pStartCrsr->GetMark() ) )
    {
        pStartCrsr->DeleteMark();
        bChanged = true;
    }
    if( !sw_PosOk( *pStartCrsr->GetPoint() ) )
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
    if (m_pTblCrsr != NULL && bChanged)
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

static void lcl_FillRecognizerData( uno::Sequence< OUString >& rSmartTagTypes,
                             uno::Sequence< uno::Reference< container::XStringKeyMap > >& rStringKeyMaps,
                             const SwWrongList& rSmartTagList, xub_StrLen nCurrent )
{
    // Insert smart tag information
    std::vector< OUString > aSmartTagTypes;
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

        std::vector< OUString >::const_iterator aTypesIter = aSmartTagTypes.begin();
        sal_uInt16 i = 0;
        for ( aTypesIter = aSmartTagTypes.begin(); aTypesIter != aSmartTagTypes.end(); ++aTypesIter )
            rSmartTagTypes[i++] = *aTypesIter;

        std::vector< uno::Reference< container::XStringKeyMap > >::const_iterator aMapsIter = aStringKeyMaps.begin();
        i = 0;
        for ( aMapsIter = aStringKeyMaps.begin(); aMapsIter != aStringKeyMaps.end(); ++aMapsIter )
            rStringKeyMaps[i++] = *aMapsIter;
    }
}

static void lcl_FillTextRange( uno::Reference<text::XTextRange>& rRange,
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

void SwCrsrShell::GetSmartTagTerm( uno::Sequence< OUString >& rSmartTagTypes,
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
                                   uno::Sequence< OUString >& rSmartTagTypes,
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
            String aText( pNode->GetTxt().copy(nBegin, nLen) );

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
