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

#include <memory>
#include <com/sun/star/text/XTextRange.hpp>

#include <hintids.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/frmdiritem.hxx>
#include <sfx2/viewsh.hxx>
#include <SwSmartTagMgr.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <viewimp.hxx>
#include <pam.hxx>
#include <swselectionlist.hxx>
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
#include "callnk.hxx"
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
#include <vcl/settings.hxx>
#include <numrule.hxx>
#include <IGrammarContact.hxx>
#include <comphelper/flagguard.hxx>
#include <globals.hrc>
#include <strings.hrc>
#include <IDocumentLayoutAccess.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <editeng/editview.hxx>
#include <PostItMgr.hxx>
#include <DocumentSettingManager.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

using namespace com::sun::star;
using namespace util;


/**
 * Delete all overlapping Cursors from a Cursor ring.
 * @param pointer to SwCursor (ring)
 */
void CheckRange( SwCursor* );

/**
 * Check if pCurrentCursor points into already existing ranges and delete those.
 * @param Pointer to SwCursor object
 */
void CheckRange( SwCursor* pCurrentCursor )
{
    const SwPosition *pStt = pCurrentCursor->Start(),
        *pEnd = pCurrentCursor->GetPoint() == pStt ? pCurrentCursor->GetMark() : pCurrentCursor->GetPoint();

    SwPaM *pTmpDel = nullptr,
          *pTmp = pCurrentCursor->GetNext();

    // Search the complete ring
    while( pTmp != pCurrentCursor )
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

         // If Point or Mark is within the Cursor range, we need to remove the old
        // range. Take note that Point does not belong to the range anymore.
        pTmp = pTmp->GetNext();
        delete pTmpDel;         // Remove old range
        pTmpDel = nullptr;
    }
}

// SwCursorShell

SwPaM * SwCursorShell::CreateCursor()
{
    // don't create new Cursor with active table Selection
    assert(!IsTableMode());

    // New cursor as copy of current one. Add to the ring.
    // Links point to previously created one, ie forward.
    SwShellCursor* pNew = new SwShellCursor( *m_pCurrentCursor );

    // Hide PaM logically, to avoid undoing the inverting from
    // copied PaM (#i75172#)
    pNew->swapContent(*m_pCurrentCursor);

    m_pCurrentCursor->DeleteMark();

    UpdateCursor( SwCursorShell::SCROLLWIN );
    return pNew;
}

/**
 * Delete current Cursor, making the following one the current.
 * Note, this function does not delete anything if there is no other cursor.
 * @return - returns true if there was another cursor and we deleted one.
 */
void SwCursorShell::DestroyCursor()
{
    // don't delete Cursor with active table Selection
    assert(!IsTableMode());

    // Is there a next one? Don't do anything if not.
    if(!m_pCurrentCursor->IsMultiSelection())
        return;

    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursor* pNextCursor = static_cast<SwCursor*>(m_pCurrentCursor->GetNext());
    delete m_pCurrentCursor;
    m_pCurrentCursor = dynamic_cast<SwShellCursor*>(pNextCursor);
    UpdateCursor();
}

/**
 * Create and return a new shell cursor.
 * Simply returns the current shell cursor if there is no selection
 * (HasSelection()).
 */
SwPaM & SwCursorShell::CreateNewShellCursor()
{
    if (HasSelection())
    {
        (void) CreateCursor(); // n.b. returns old cursor
    }
    return *GetCursor();
}

/**
 * Return the current shell cursor
 * @return - returns current `SwPaM` shell cursor
 */
SwPaM & SwCursorShell::GetCurrentShellCursor()
{
    return *GetCursor();
}

/**
 * Return pointer to the current shell cursor
 * @return - returns pointer to current `SwPaM` shell cursor
 */
SwPaM* SwCursorShell::GetCursor( bool bMakeTableCursor ) const
{
    if( m_pTableCursor )
    {
        if( bMakeTableCursor && m_pTableCursor->IsCursorMovedUpdate() )
        {
            //don't re-create 'parked' cursors
            const SwContentNode* pCNd;
            if( m_pTableCursor->GetPoint()->nNode.GetIndex() &&
                m_pTableCursor->GetMark()->nNode.GetIndex() &&
                nullptr != ( pCNd = m_pTableCursor->GetContentNode() ) && pCNd->getLayoutFrame( GetLayout() ) &&
                nullptr != ( pCNd = m_pTableCursor->GetContentNode(false) ) && pCNd->getLayoutFrame( GetLayout() ) )
            {
                SwShellTableCursor* pTC = m_pTableCursor;
                GetLayout()->MakeTableCursors( *pTC );
            }
        }

        if( m_pTableCursor->IsChgd() )
        {
            const_cast<SwCursorShell*>(this)->m_pCurrentCursor =
                dynamic_cast<SwShellCursor*>(m_pTableCursor->MakeBoxSels( m_pCurrentCursor ));
        }
    }
    return m_pCurrentCursor;
}

void SwCursorShell::StartAction()
{
    if( !ActionPend() )
    {
        // save for update of the ribbon bar
        const SwNode& rNd = m_pCurrentCursor->GetPoint()->nNode.GetNode();
        m_nCurrentNode = rNd.GetIndex();
        m_nCurrentContent = m_pCurrentCursor->GetPoint()->nContent.GetIndex();
        m_nCurrentNdTyp = rNd.GetNodeType();
        if( rNd.IsTextNode() )
            m_nLeftFramePos = SwCallLink::getLayoutFrame( GetLayout(), *rNd.GetTextNode(), m_nCurrentContent, true );
        else
            m_nLeftFramePos = 0;
    }
    SwViewShell::StartAction(); // to the SwViewShell
}

void SwCursorShell::EndAction( const bool bIdleEnd, const bool DoSetPosX )
{
    comphelper::FlagRestorationGuard g(mbSelectAll, StartsWithTable() && ExtendedSelectedAll());
    bool bVis = m_bSVCursorVis;

    sal_uInt16 eFlags = SwCursorShell::CHKRANGE;
    if ( !DoSetPosX )
        eFlags |= SwCursorShell::UPDOWN;


    // Idle-formatting?
    if( bIdleEnd && Imp()->GetRegion() )
    {
        m_pCurrentCursor->Hide();
    }

    // Update all invalid numberings before the last action
    if( 1 == mnStartAction )
        GetDoc()->UpdateNumRule();

    // #i76923#: Don't show the cursor in the SwViewShell::EndAction() - call.
    //           Only the UpdateCursor shows the cursor.
    bool bSavSVCursorVis = m_bSVCursorVis;
    m_bSVCursorVis = false;

    SwViewShell::EndAction( bIdleEnd );   // have SwViewShell go first

    m_bSVCursorVis = bSavSVCursorVis;

    if( ActionPend() )
    {
        if( bVis )    // display SV-Cursor again
            m_pVisibleCursor->Show();

        return;
    }

    if ( !bIdleEnd )
        eFlags |= SwCursorShell::SCROLLWIN;

    UpdateCursor( eFlags, bIdleEnd );      // Show Cursor changes

    {
        SwCallLink aLk( *this );           // Watch cursor moves,
        aLk.nNode = m_nCurrentNode;        // possibly call the link
        aLk.nNdTyp = m_nCurrentNdTyp;
        aLk.nContent = m_nCurrentContent;
        aLk.nLeftFramePos = m_nLeftFramePos;

        if( !m_nCursorMove ||
            ( 1 == m_nCursorMove && m_bInCMvVisportChgd ) )
            // display Cursor & Selections again
            ShowCursors( m_bSVCursorVis );
    }
    // call ChgCall if there is still one
    if( m_bCallChgLnk && m_bChgCallFlag && m_aChgLnk.IsSet() )
    {
        m_aChgLnk.Call( this );
        m_bChgCallFlag = false;       // reset flag
    }
}

void SwCursorShell::SttCursorMove()
{
#ifdef DBG_UTIL
    OSL_ENSURE( m_nCursorMove < USHRT_MAX, "To many nested CursorMoves." );
#endif
    ++m_nCursorMove;
    StartAction();
}

void SwCursorShell::EndCursorMove( const bool bIdleEnd )
{
#ifdef DBG_UTIL
    OSL_ENSURE( m_nCursorMove, "EndCursorMove() without SttCursorMove()." );
#endif
    EndAction( bIdleEnd, true );
    --m_nCursorMove;
#ifdef DBG_UTIL
    if( !m_nCursorMove )
        m_bInCMvVisportChgd = false;
#endif
}

bool SwCursorShell::LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
                             bool bVisualAllowed )
{
    if( IsTableMode() )
        return bLeft ? GoPrevCell() : GoNextCell();

    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    bool bRet = false;

    // #i27615# Handle cursor in front of label.
    const SwTextNode* pTextNd = nullptr;

    if( m_pBlockCursor )
        m_pBlockCursor->clearPoints();

    // 1. CASE: Cursor is in front of label. A move to the right
    // will simply reset the bInFrontOfLabel flag:
    SwShellCursor* pShellCursor = getShellCursor( true );
    if ( !bLeft && pShellCursor->IsInFrontOfLabel() )
    {
        SetInFrontOfLabel( false );
        bRet = true;
    }
    // 2. CASE: Cursor is at beginning of numbered paragraph. A move
    // to the left will simply set the bInFrontOfLabel flag:
    else if ( bLeft && 0 == pShellCursor->GetPoint()->nContent.GetIndex() &&
             !pShellCursor->IsInFrontOfLabel() && !pShellCursor->HasMark() &&
             nullptr != ( pTextNd = pShellCursor->GetNode().GetTextNode() ) &&
             pTextNd->HasVisibleNumberingOrBullet() )
    {
        SetInFrontOfLabel( true );
        bRet = true;
    }
    // 3. CASE: Regular cursor move. Reset the bInFrontOfLabel flag:
    else
    {
        const bool bSkipHidden = !GetViewOptions()->IsShowHiddenChar();
        // #i107447#
        // To avoid loop the reset of <bInFrontOfLabel> flag is no longer
        // reflected in the return value <bRet>.
        const bool bResetOfInFrontOfLabel = SetInFrontOfLabel( false );
        bRet = pShellCursor->LeftRight( bLeft, nCnt, nMode, bVisualAllowed,
                                      bSkipHidden, !IsOverwriteCursor() );
        if ( !bRet && bLeft && bResetOfInFrontOfLabel )
        {
            // undo reset of <bInFrontOfLabel> flag
            SetInFrontOfLabel( true );
        }
    }

    if( bRet )
    {
        UpdateCursor();
    }

    return bRet;
}

void SwCursorShell::MarkListLevel( const OUString& sListId,
                                 const int nListLevel )
{
    if ( sListId != m_sMarkedListId ||
         nListLevel != m_nMarkedListLevel)
    {
        if ( !m_sMarkedListId.isEmpty() )
            mxDoc->MarkListLevel( m_sMarkedListId, m_nMarkedListLevel, false );

        if ( !sListId.isEmpty() )
        {
            mxDoc->MarkListLevel( sListId, nListLevel, true );
        }

        m_sMarkedListId = sListId;
        m_nMarkedListLevel = nListLevel;
    }
}

void SwCursorShell::UpdateMarkedListLevel()
{
    SwTextNode * pTextNd = GetCursor_()->GetNode().GetTextNode();

    if ( pTextNd )
    {
        if ( !pTextNd->IsNumbered() )
        {
            m_pCurrentCursor->SetInFrontOfLabel_( false );
            MarkListLevel( OUString(), 0 );
        }
        else if ( m_pCurrentCursor->IsInFrontOfLabel() )
        {
            if ( pTextNd->IsInList() )
            {
                assert(pTextNd->GetActualListLevel() >= 0 &&
                       pTextNd->GetActualListLevel() < MAXLEVEL);
                MarkListLevel( pTextNd->GetListId(),
                               pTextNd->GetActualListLevel() );
            }
        }
        else
        {
            MarkListLevel( OUString(), 0 );
        }
    }
}

void SwCursorShell::FirePageChangeEvent(sal_uInt16 nOldPage, sal_uInt16 nNewPage)
{
#ifdef ACCESSIBLE_LAYOUT
    if( Imp()->IsAccessible() )
        Imp()->FirePageChangeEvent( nOldPage, nNewPage );
#else
    (void)nOldPage;
    (void)nNewPage;
#endif
}

void SwCursorShell::FireColumnChangeEvent(sal_uInt16 nOldColumn, sal_uInt16 nNewColumn)
{
#ifdef ACCESSIBLE_LAYOUT
    if( Imp()->IsAccessible() )
        Imp()->FireColumnChangeEvent( nOldColumn,  nNewColumn);
#else
    (void)nOldColumn;
    (void)nNewColumn;
#endif
}

void SwCursorShell::FireSectionChangeEvent(sal_uInt16 nOldSection, sal_uInt16 nNewSection)
{
#ifdef ACCESSIBLE_LAYOUT
    if( Imp()->IsAccessible() )
        Imp()->FireSectionChangeEvent( nOldSection, nNewSection );
#else
    (void)nOldSection;
    (void)nNewSection;
#endif
}

bool SwCursorShell::bColumnChange()
{
    SwFrame* pCurrFrame = GetCurrFrame(false);

    if (pCurrFrame == nullptr)
    {
        return false;
    }

    SwFrame* pCurrCol=pCurrFrame->FindColFrame();

    while(pCurrCol== nullptr && pCurrFrame!=nullptr )
    {
        SwLayoutFrame* pParent = pCurrFrame->GetUpper();
        if(pParent!=nullptr)
        {
            pCurrCol=static_cast<SwFrame*>(pParent)->FindColFrame();
            pCurrFrame = static_cast<SwFrame*>(pParent);
        }
        else
        {
            break;
        }
    }

    if(m_oldColFrame == pCurrCol)
        return false;
    else
    {
        m_oldColFrame = pCurrCol;
        return true;
    }
}

bool SwCursorShell::UpDown( bool bUp, sal_uInt16 nCnt )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed

    bool bTableMode = IsTableMode();
    SwShellCursor* pTmpCursor = getShellCursor( true );

    bool bRet = pTmpCursor->UpDown( bUp, nCnt );
    // #i40019# UpDown should always reset the bInFrontOfLabel flag:
    bRet |= SetInFrontOfLabel(false);

    if( m_pBlockCursor )
        m_pBlockCursor->clearPoints();

    if( bRet )
    {
        m_eMvState = MV_UPDOWN; // status for Cursor travelling - GetCursorOfst
        if( !ActionPend() )
        {
            CursorFlag eUpdateMode = SwCursorShell::SCROLLWIN;
            if( !bTableMode )
                eUpdateMode = static_cast<CursorFlag>(eUpdateMode
                            | SwCursorShell::UPDOWN | SwCursorShell::CHKRANGE);
            UpdateCursor( static_cast<sal_uInt16>(eUpdateMode) );
        }
    }
    return bRet;
}

bool SwCursorShell::LRMargin( bool bLeft, bool bAPI)
{
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    SET_CURR_SHELL( this );
    m_eMvState = MV_LEFTMARGIN; // status for Cursor travelling - GetCursorOfst

    const bool bTableMode = IsTableMode();
    SwShellCursor* pTmpCursor = getShellCursor( true );

    if( m_pBlockCursor )
        m_pBlockCursor->clearPoints();

    const bool bWasAtLM =
            ( 0 == GetCursor_()->GetPoint()->nContent.GetIndex() );

    bool bRet = pTmpCursor->LeftRightMargin( bLeft, bAPI );

    if ( bLeft && !bTableMode && bRet && bWasAtLM && !GetCursor_()->HasMark() )
    {
        const SwTextNode * pTextNd = GetCursor_()->GetNode().GetTextNode();
        if ( pTextNd && pTextNd->HasVisibleNumberingOrBullet() )
            SetInFrontOfLabel( true );
    }
    else if ( !bLeft )
    {
        bRet = SetInFrontOfLabel( false ) || bRet;
    }

    if( bRet )
    {
        UpdateCursor();
    }
    return bRet;
}

bool SwCursorShell::IsAtLRMargin( bool bLeft, bool bAPI ) const
{
    const SwShellCursor* pTmpCursor = getShellCursor( true );
    return pTmpCursor->IsAtLeftRightMargin( bLeft, bAPI );
}

bool SwCursorShell::SttEndDoc( bool bStt )
{
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed

    SwShellCursor* pTmpCursor = m_pBlockCursor ? &m_pBlockCursor->getShellCursor() : m_pCurrentCursor;
    bool bRet = pTmpCursor->SttEndDoc( bStt );
    if( bRet )
    {
        if( bStt )
            pTmpCursor->GetPtPos().setY( 0 ); // set to 0 explicitly (table header)
        if( m_pBlockCursor )
        {
            m_pBlockCursor->clearPoints();
            RefreshBlockCursor();
        }

        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    }
    return bRet;
}

void SwCursorShell::ExtendedSelectAll(bool bFootnotes)
{
    SwNodes& rNodes = GetDoc()->GetNodes();
    SwPosition* pPos = m_pCurrentCursor->GetPoint();
    pPos->nNode = bFootnotes ? rNodes.GetEndOfPostIts() : rNodes.GetEndOfAutotext();
    pPos->nContent.Assign( rNodes.GoNext( &pPos->nNode ), 0 );
    pPos = m_pCurrentCursor->GetMark();
    pPos->nNode = rNodes.GetEndOfContent();
    SwContentNode* pCNd = SwNodes::GoPrevious( &pPos->nNode );
    pPos->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );
}

bool SwCursorShell::ExtendedSelectedAll()
{
    SwNodes& rNodes = GetDoc()->GetNodes();
    SwNodeIndex nNode = rNodes.GetEndOfAutotext();
    SwContentNode* pStart = rNodes.GoNext(&nNode);

    nNode = rNodes.GetEndOfContent();
    SwContentNode* pEnd = SwNodes::GoPrevious(&nNode);

    if (!pStart || !pEnd)
        return false;

    SwPosition aStart(*pStart, 0);
    SwPosition aEnd(*pEnd, pEnd->Len());
    SwShellCursor* pShellCursor = getShellCursor(false);
    return aStart == *pShellCursor->Start() && aEnd == *pShellCursor->End();
}

bool SwCursorShell::StartsWithTable()
{
    SwNodes& rNodes = GetDoc()->GetNodes();
    SwNodeIndex nNode(rNodes.GetEndOfExtras());
    SwContentNode* pContentNode = rNodes.GoNext(&nNode);
    return pContentNode->FindTableNode();
}

bool SwCursorShell::MovePage( SwWhichPage fnWhichPage, SwPosPage fnPosPage )
{
    bool bRet = false;

    // never jump of section borders at selection
    if( !m_pCurrentCursor->HasMark() || !m_pCurrentCursor->IsNoContent() )
    {
        SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
        SET_CURR_SHELL( this );

        SwCursorSaveState aSaveState( *m_pCurrentCursor );
        Point& rPt = m_pCurrentCursor->GetPtPos();
        SwContentFrame * pFrame = m_pCurrentCursor->GetContentNode()->
                            getLayoutFrame( GetLayout(), &rPt, m_pCurrentCursor->GetPoint(), false );
        if( pFrame && ( bRet = GetFrameInPage( pFrame, fnWhichPage,
                                           fnPosPage, m_pCurrentCursor )  ) &&
            !m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::Toggle |
                                 SwCursorSelOverFlags::ChangePos ))
            UpdateCursor();
        else
            bRet = false;
    }
    return bRet;
}

bool SwCursorShell::isInHiddenTextFrame(SwShellCursor* pShellCursor)
{
    SwContentNode *pCNode = pShellCursor->GetContentNode();
    SwContentFrame  *pFrame = pCNode ?
        pCNode->getLayoutFrame( GetLayout(), &pShellCursor->GetPtPos(), pShellCursor->GetPoint(), false ) : nullptr;
    return !pFrame || (pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->IsHiddenNow());
}

bool SwCursorShell::MovePara(SwWhichPara fnWhichPara, SwMoveFnCollection const & fnPosPara )
{
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    SwShellCursor* pTmpCursor = getShellCursor( true );
    bool bRet = pTmpCursor->MovePara( fnWhichPara, fnPosPara );
    if( bRet )
    {
        //keep going until we get something visible, i.e. skip
        //over hidden paragraphs, don't get stuck at the start
        //which is what SwCursorShell::UpdateCursorPos will reset
        //the position to if we pass it a position in an
        //invisible hidden paragraph field
        while (isInHiddenTextFrame(pTmpCursor))
        {
            if (!pTmpCursor->MovePara(fnWhichPara, fnPosPara))
                break;
        }

        UpdateCursor();
    }
    return bRet;
}

bool SwCursorShell::MoveSection( SwWhichSection fnWhichSect,
                                SwMoveFnCollection const & fnPosSect)
{
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    SwCursor* pTmpCursor = getShellCursor( true );
    bool bRet = pTmpCursor->MoveSection( fnWhichSect, fnPosSect );
    if( bRet )
        UpdateCursor();
    return bRet;

}

// position cursor

static SwFrame* lcl_IsInHeaderFooter( const SwNodeIndex& rIdx, Point& rPt )
{
    SwFrame* pFrame = nullptr;
    SwContentNode* pCNd = rIdx.GetNode().GetContentNode();
    if( pCNd )
    {
        SwContentFrame *pContentFrame = pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &rPt, nullptr, false );
        pFrame = pContentFrame ? pContentFrame->GetUpper() : nullptr;
        while( pFrame && !pFrame->IsHeaderFrame() && !pFrame->IsFooterFrame() )
            pFrame = pFrame->IsFlyFrame() ? static_cast<SwFlyFrame*>(pFrame)->AnchorFrame()
                                    : pFrame->GetUpper();
    }
    return pFrame;
}

bool SwCursorShell::IsInHeaderFooter( bool* pbInHeader ) const
{
    Point aPt;
    SwFrame* pFrame = ::lcl_IsInHeaderFooter( m_pCurrentCursor->GetPoint()->nNode, aPt );
    if( pFrame && pbInHeader )
        *pbInHeader = pFrame->IsHeaderFrame();
    return nullptr != pFrame;
}

int SwCursorShell::SetCursor( const Point &rLPt, bool bOnlyText, bool bBlock )
{
    SET_CURR_SHELL( this );

    SwShellCursor* pCursor = getShellCursor( bBlock );
    SwPosition aPos( *pCursor->GetPoint() );
    Point aPt( rLPt );
    Point & rCurrentCursorPt = pCursor->GetPtPos();
    SwCursorMoveState aTmpState( IsTableMode() ? MV_TBLSEL :
                                    bOnlyText ?  MV_SETONLYTEXT : MV_NONE );
    aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();

    SwTextNode * pTextNd = pCursor->GetNode().GetTextNode();

    if ( pTextNd && !IsTableMode() &&
        // #i37515# No bInFrontOfLabel during selection
        !pCursor->HasMark() &&
        pTextNd->HasVisibleNumberingOrBullet() )
    {
        aTmpState.m_bInFrontOfLabel = true; // #i27615#
    }
    else
    {
        aTmpState.m_bInFrontOfLabel = false;
    }

    int bRet = CRSR_POSOLD |
                ( GetLayout()->GetCursorOfst( &aPos, aPt, &aTmpState )
                    ? 0 : CRSR_POSCHG );

    const bool bOldInFrontOfLabel = IsInFrontOfLabel();
    const bool bNewInFrontOfLabel = aTmpState.m_bInFrontOfLabel;

    pCursor->SetCursorBidiLevel( aTmpState.m_nCursorBidiLevel );

    if( MV_RIGHTMARGIN == aTmpState.m_eState )
        m_eMvState = MV_RIGHTMARGIN;
    // is the new position in header or footer?
    SwFrame* pFrame = lcl_IsInHeaderFooter( aPos.nNode, aPt );
    if( IsTableMode() && !pFrame && aPos.nNode.GetNode().StartOfSectionNode() ==
        pCursor->GetPoint()->nNode.GetNode().StartOfSectionNode() )
        // same table column and not in header/footer -> back
        return bRet;

    if( m_pBlockCursor && bBlock )
    {
        m_pBlockCursor->setEndPoint( rLPt );
        if( !pCursor->HasMark() )
            m_pBlockCursor->setStartPoint( rLPt );
        else if( !m_pBlockCursor->getStartPoint() )
            m_pBlockCursor->setStartPoint( pCursor->GetMkPos() );
    }
    if( !pCursor->HasMark() )
    {
        // is at the same position and if in header/footer -> in the same
        if( aPos == *pCursor->GetPoint() &&
            bOldInFrontOfLabel == bNewInFrontOfLabel )
        {
            if( pFrame )
            {
                if( pFrame->getFrameArea().IsInside( rCurrentCursorPt ))
                    return bRet;
            }
            else if( aPos.nNode.GetNode().IsContentNode() )
            {
                // in the same frame?
                SwFrame* pOld = static_cast<SwContentNode&>(aPos.nNode.GetNode()).getLayoutFrame(
                                GetLayout(), &m_aCharRect.Pos(), nullptr, false );
                SwFrame* pNew = static_cast<SwContentNode&>(aPos.nNode.GetNode()).getLayoutFrame(
                                GetLayout(), &aPt, nullptr, false );
                if( pNew == pOld )
                    return bRet;
            }
        }
    }
    else
    {
        // SSelection over not allowed sections or if in header/footer -> different
        if( !CheckNodesRange( aPos.nNode, pCursor->GetMark()->nNode, true )
            || ( pFrame && !pFrame->getFrameArea().IsInside( pCursor->GetMkPos() ) ))
            return bRet;

        // is at same position but not in header/footer
        if( aPos == *pCursor->GetPoint() )
            return bRet;
    }

    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    SwCursorSaveState aSaveState( *pCursor );

    *pCursor->GetPoint() = aPos;
    rCurrentCursorPt = aPt;

    // #i41424# Only update the marked number levels if necessary
    // Force update of marked number levels if necessary.
    if ( bNewInFrontOfLabel || bOldInFrontOfLabel )
        m_pCurrentCursor->SetInFrontOfLabel_( !bNewInFrontOfLabel );
    SetInFrontOfLabel( bNewInFrontOfLabel );

    if( !pCursor->IsSelOvr( SwCursorSelOverFlags::ChangePos ) )
    {
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE );
        bRet &= ~CRSR_POSOLD;
    }
    else if( bOnlyText && !m_pCurrentCursor->HasMark() )
    {
        if( FindValidContentNode( bOnlyText ) )
        {
            // position cursor in a valid content
            if( aPos == *pCursor->GetPoint() )
                bRet = CRSR_POSOLD;
            else
            {
                UpdateCursor();
                bRet &= ~CRSR_POSOLD;
            }
        }
        else
        {
            // there is no valid content -> hide cursor
            m_pVisibleCursor->Hide(); // always hide visible cursor
            m_eMvState = MV_NONE; // status for Cursor travelling
            m_bAllProtect = true;
            if( GetDoc()->GetDocShell() )
            {
                GetDoc()->GetDocShell()->SetReadOnlyUI();
                CallChgLnk(); // notify UI
            }
        }
    }
    return bRet;
}

void SwCursorShell::TableCursorToCursor()
{
    assert(m_pTableCursor);
    delete m_pTableCursor;
    m_pTableCursor = nullptr;
}

void SwCursorShell::BlockCursorToCursor()
{
    assert(m_pBlockCursor);
    if( m_pBlockCursor && !HasSelection() )
    {
        SwPaM& rPam = m_pBlockCursor->getShellCursor();
        m_pCurrentCursor->SetMark();
        *m_pCurrentCursor->GetPoint() = *rPam.GetPoint();
        if( rPam.HasMark() )
            *m_pCurrentCursor->GetMark() = *rPam.GetMark();
        else
            m_pCurrentCursor->DeleteMark();
    }
    delete m_pBlockCursor;
    m_pBlockCursor = nullptr;
}

void SwCursorShell::CursorToBlockCursor()
{
    if( !m_pBlockCursor )
    {
        SwPosition aPos( *m_pCurrentCursor->GetPoint() );
        m_pBlockCursor = new SwBlockCursor( *this, aPos );
        SwShellCursor &rBlock = m_pBlockCursor->getShellCursor();
        rBlock.GetPtPos() = m_pCurrentCursor->GetPtPos();
        if( m_pCurrentCursor->HasMark() )
        {
            rBlock.SetMark();
            *rBlock.GetMark() = *m_pCurrentCursor->GetMark();
            rBlock.GetMkPos() = m_pCurrentCursor->GetMkPos();
        }
    }
    m_pBlockCursor->clearPoints();
    RefreshBlockCursor();
}

void SwCursorShell::ClearMark()
{
    // is there any GetMark?
    if( m_pTableCursor )
    {
        std::vector<SwPaM*> vCursors;
        for(auto& rCursor : m_pCurrentCursor->GetRingContainer())
            if(&rCursor != m_pCurrentCursor)
                vCursors.push_back(&rCursor);
        for(auto pCursor : vCursors)
            delete pCursor;
        m_pTableCursor->DeleteMark();

        m_pCurrentCursor->DeleteMark();

        *m_pCurrentCursor->GetPoint() = *m_pTableCursor->GetPoint();
        m_pCurrentCursor->GetPtPos() = m_pTableCursor->GetPtPos();
        delete m_pTableCursor;
        m_pTableCursor = nullptr;
        m_pCurrentCursor->SwSelPaintRects::Show();
    }
    else
    {
        if( !m_pCurrentCursor->HasMark() )
            return;
        m_pCurrentCursor->DeleteMark();
        if( !m_nCursorMove )
            m_pCurrentCursor->SwSelPaintRects::Show();
    }
}

void SwCursorShell::NormalizePam(bool bPointFirst)
{
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    m_pCurrentCursor->Normalize(bPointFirst);
}

void SwCursorShell::SwapPam()
{
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    m_pCurrentCursor->Exchange();
}

//TODO: provide documentation
/** Search in the selected area for a Selection that covers the given point.

    It checks if a Selection exists but does
    not move the current cursor.

    @param rPt      The point to search at.
    @param bTstHit ???
*/
bool SwCursorShell::TestCurrPam(
    const Point & rPt,
    bool bTstHit )
{
    SET_CURR_SHELL( this );

    // check if the SPoint is in a table selection
    if( m_pTableCursor )
        return m_pTableCursor->IsInside( rPt );

    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    // search position <rPt> in document
    SwPosition aPtPos( *m_pCurrentCursor->GetPoint() );
    Point aPt( rPt );

    SwCursorMoveState aTmpState( MV_NONE );
    aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();
    if ( !GetLayout()->GetCursorOfst( &aPtPos, aPt, &aTmpState ) && bTstHit )
        return false;

    // search in all selections for this position
    SwShellCursor* pCmp = m_pCurrentCursor; // keep the pointer on cursor
    do
    {
        if (pCmp->HasMark() && *pCmp->Start() <= aPtPos && *pCmp->End() > aPtPos)
            return true;               // return without update
        pCmp = pCmp->GetNext();
    } while (m_pCurrentCursor != pCmp);
    return false;
}

void SwCursorShell::KillPams()
{
    // Does any exist for deletion?
    if( !m_pTableCursor && !m_pBlockCursor && !m_pCurrentCursor->IsMultiSelection() )
        return;

    while( m_pCurrentCursor->GetNext() != m_pCurrentCursor )
        delete m_pCurrentCursor->GetNext();
    m_pCurrentCursor->SetColumnSelection( false );

    if( m_pTableCursor )
    {
        // delete the ring of cursors
        m_pCurrentCursor->DeleteMark();
        *m_pCurrentCursor->GetPoint() = *m_pTableCursor->GetPoint();
        m_pCurrentCursor->GetPtPos() = m_pTableCursor->GetPtPos();
        delete m_pTableCursor;
        m_pTableCursor = nullptr;
    }
    else if( m_pBlockCursor )
    {
        // delete the ring of cursors
        m_pCurrentCursor->DeleteMark();
        SwShellCursor &rBlock = m_pBlockCursor->getShellCursor();
        *m_pCurrentCursor->GetPoint() = *rBlock.GetPoint();
        m_pCurrentCursor->GetPtPos() = rBlock.GetPtPos();
        rBlock.DeleteMark();
        m_pBlockCursor->clearPoints();
    }
    UpdateCursor( SwCursorShell::SCROLLWIN );
}

int SwCursorShell::CompareCursorStackMkCurrPt() const
{
    int nRet = 0;
    const SwPosition *pFirst = nullptr, *pSecond = nullptr;
    const SwPaM *pCur = GetCursor(), *pStack = m_pStackCursor;
    // cursor on stack is needed if we compare against stack
    if( pStack  )
    {
        pFirst = pStack->GetMark();
        pSecond = pCur->GetPoint();
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

bool SwCursorShell::IsSttPara() const
{   return m_pCurrentCursor->GetPoint()->nContent == 0; }

bool SwCursorShell::IsEndPara() const
{   return m_pCurrentCursor->GetPoint()->nContent == m_pCurrentCursor->GetContentNode()->Len(); }

bool SwCursorShell::IsEndOfTable() const
{
    if (IsTableMode() || IsBlockMode() || !IsEndPara())
    {
        return false;
    }
    SwTableNode const*const pTableNode( IsCursorInTable() );
    if (!pTableNode)
    {
        return false;
    }
    SwEndNode const*const pEndTableNode(pTableNode->EndOfSectionNode());
    SwNodeIndex const lastNode(*pEndTableNode, -2);
    SAL_WARN_IF(!lastNode.GetNode().GetTextNode(), "sw.core",
            "text node expected");
    return (lastNode == m_pCurrentCursor->GetPoint()->nNode);
}

bool SwCursorShell::IsCursorInFootnote() const
{
    SwStartNodeType aStartNodeType = m_pCurrentCursor->GetNode().StartOfSectionNode()->GetStartNodeType();
    return aStartNodeType == SwStartNodeType::SwFootnoteStartNode;
}

bool SwCursorShell::IsInFrontOfLabel() const
{
    return m_pCurrentCursor->IsInFrontOfLabel();
}

bool SwCursorShell::SetInFrontOfLabel( bool bNew )
{
    if ( bNew != IsInFrontOfLabel() )
    {
        m_pCurrentCursor->SetInFrontOfLabel_( bNew );
        UpdateMarkedListLevel();
        return true;
    }
    return false;
}

namespace {

void collectUIInformation(OUString aPage)
{
    EventDescription aDescription;
    aDescription.aAction = "GOTO";
    aDescription.aParameters = {{"PAGE", aPage}};
    aDescription.aID = "writer_edit";
    aDescription.aKeyWord = "SwEditWinUIObject";
    aDescription.aParent = "MainWindow";
    UITestLogger::getInstance().logEvent(aDescription);
}

}

bool SwCursorShell::GotoPage( sal_uInt16 nPage )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    SwCursorSaveState aSaveState( *m_pCurrentCursor );
    bool bRet = GetLayout()->SetCurrPage( m_pCurrentCursor, nPage ) &&
                    !m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::Toggle |
                                         SwCursorSelOverFlags::ChangePos );
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);

    collectUIInformation(OUString::number(nPage));
    return bRet;
}

void SwCursorShell::GetCharRectAt(SwRect& rRect, const SwPosition* pPos)
{
    SwContentFrame* pFrame = GetCurrFrame();
    pFrame->GetCharRect( rRect, *pPos );
}

void SwCursorShell::GetPageNum( sal_uInt16 &rnPhyNum, sal_uInt16 &rnVirtNum,
                              bool bAtCursorPos, const bool bCalcFrame )
{
    SET_CURR_SHELL( this );
    // page number: first visible page or the one at the cursor
    const SwContentFrame* pCFrame;
    const SwPageFrame *pPg = nullptr;

    if( !bAtCursorPos || nullptr == (pCFrame = GetCurrFrame( bCalcFrame )) ||
                       nullptr == (pPg   = pCFrame->FindPageFrame()) )
    {
        pPg = Imp()->GetFirstVisPage(GetOut());
        while( pPg && pPg->IsEmptyPage() )
            pPg = static_cast<const SwPageFrame *>(pPg->GetNext());
    }
    // pPg has to exist with a default of 1 for the special case "Writerstart"
    rnPhyNum  = pPg? pPg->GetPhyPageNum() : 1;
    rnVirtNum = pPg? pPg->GetVirtPageNum() : 1;
}

sal_uInt16 SwCursorShell::GetPageNumSeqNonEmpty(bool bAtCursorPos, bool bCalcFrame)
{
    SET_CURR_SHELL(this);
    // page number: first visible page or the one at the cursor
    const SwContentFrame* pCFrame = GetCurrFrame(bCalcFrame);
    const SwPageFrame* pPg = nullptr;

    if (!bAtCursorPos || !pCFrame || nullptr == (pPg = pCFrame->FindPageFrame()))
    {
        pPg = Imp()->GetFirstVisPage(GetOut());
        while (pPg && pPg->IsEmptyPage())
            pPg = static_cast<const SwPageFrame*>(pPg->GetNext());
    }

    sal_uInt16 nPageNo = 0;
    while (pPg)
    {
        if (!pPg->IsEmptyPage())
            ++nPageNo;
        pPg = static_cast<const SwPageFrame*>(pPg->GetPrev());
    }
    return nPageNo;
}

sal_uInt16 SwCursorShell::GetNextPrevPageNum( bool bNext )
{
    SET_CURR_SHELL( this );
    // page number: first visible page or the one at the cursor
    const SwPageFrame *pPg = Imp()->GetFirstVisPage(GetOut());
    if( pPg )
    {
        const SwTwips nPageTop = pPg->getFrameArea().Top();

        if( bNext )
        {
            // go to next view layout row:
            do
            {
                pPg = static_cast<const SwPageFrame *>(pPg->GetNext());
            }
            while( pPg && pPg->getFrameArea().Top() == nPageTop );

            while( pPg && pPg->IsEmptyPage() )
                pPg = static_cast<const SwPageFrame *>(pPg->GetNext());
        }
        else
        {
            // go to previous view layout row:
            do
            {
                pPg = static_cast<const SwPageFrame *>(pPg->GetPrev());
            }
            while( pPg && pPg->getFrameArea().Top() == nPageTop );

            while( pPg && pPg->IsEmptyPage() )
                pPg = static_cast<const SwPageFrame *>(pPg->GetPrev());
        }
    }
    // pPg has to exist with a default of 1 for the special case "Writerstart"
    return pPg ? pPg->GetPhyPageNum() : USHRT_MAX;
}

sal_uInt16 SwCursorShell::GetPageCnt()
{
    SET_CURR_SHELL( this );
    // return number of pages
    return GetLayout()->GetPageNum();
}

OUString SwCursorShell::getPageRectangles()
{
    CurrShell aCurr(this);
    SwRootFrame* pLayout = GetLayout();
    OUStringBuffer aBuf;
    for (const SwFrame* pFrame = pLayout->GetLower(); pFrame; pFrame = pFrame->GetNext())
    {
        aBuf.append(pFrame->getFrameArea().Left());
        aBuf.append(", ");
        aBuf.append(pFrame->getFrameArea().Top());
        aBuf.append(", ");
        aBuf.append(pFrame->getFrameArea().Width());
        aBuf.append(", ");
        aBuf.append(pFrame->getFrameArea().Height());
        aBuf.append("; ");
    }
    if (!aBuf.isEmpty())
        aBuf.setLength( aBuf.getLength() - 2); // remove the last "; "
    return aBuf.makeStringAndClear();
}

void SwCursorShell::NotifyCursor(SfxViewShell* pOtherShell) const
{
    auto pView = const_cast<SdrView*>(GetDrawView());
    if (pView->GetTextEditObject())
    {
        // Blinking cursor.
        EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
        rEditView.RegisterOtherShell(pOtherShell);
        rEditView.ShowCursor();
        rEditView.RegisterOtherShell(nullptr);
        // Text selection, if any.
        rEditView.DrawSelectionXOR(pOtherShell);

        // Shape text lock.
        if (OutlinerView* pOutlinerView = pView->GetTextEditOutlinerView())
        {
            OString sRect = pOutlinerView->GetOutputArea().toString();
            SfxLokHelper::notifyOtherView(GetSfxViewShell(), pOtherShell, LOK_CALLBACK_VIEW_LOCK, "rectangle", sRect);
        }
    }
    else
    {
        // Cursor position.
        m_pVisibleCursor->SetPosAndShow(pOtherShell);
        // Cursor visibility.
        if (GetSfxViewShell() != pOtherShell)
        {
            OString aPayload = OString::boolean(m_bSVCursorVis);
            SfxLokHelper::notifyOtherView(GetSfxViewShell(), pOtherShell, LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible", aPayload);
        }
        // Text selection.
        m_pCurrentCursor->Show(pOtherShell);
        // Graphic selection.
        pView->AdjustMarkHdl(pOtherShell);
    }
}

/// go to the next SSelection
bool SwCursorShell::GoNextCursor()
{
    if( !m_pCurrentCursor->IsMultiSelection() )
        return false;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    m_pCurrentCursor = m_pCurrentCursor->GetNext();

    // #i24086#: show also all others
    if( !ActionPend() )
    {
        UpdateCursor();
        m_pCurrentCursor->Show(nullptr);
    }
    return true;
}

/// go to the previous SSelection
bool SwCursorShell::GoPrevCursor()
{
    if( !m_pCurrentCursor->IsMultiSelection() )
        return false;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    m_pCurrentCursor = m_pCurrentCursor->GetPrev();

    // #i24086#: show also all others
    if( !ActionPend() )
    {
        UpdateCursor();
        m_pCurrentCursor->Show(nullptr);
    }
    return true;
}

void SwCursorShell::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect)
{
    comphelper::FlagRestorationGuard g(mbSelectAll, StartsWithTable() && ExtendedSelectedAll());
    SET_CURR_SHELL( this );

    // always switch off all cursors when painting
    SwRect aRect( rRect );

    bool bVis = false;
    // if a cursor is visible then hide the SV cursor
    if( m_pVisibleCursor->IsVisible() && !aRect.IsOver( m_aCharRect ) )
    {
        bVis = true;
        m_pVisibleCursor->Hide();
    }

    // re-paint area
    SwViewShell::Paint(rRenderContext, rRect);

    if( m_bHasFocus && !m_bBasicHideCursor )
    {
        SwShellCursor* pCurrentCursor = m_pTableCursor ? m_pTableCursor : m_pCurrentCursor;

        if( !ActionPend() )
        {
            // so that right/bottom borders will not be cropped
            pCurrentCursor->Invalidate( VisArea() );
            pCurrentCursor->Show(nullptr);
        }
        else
            pCurrentCursor->Invalidate( aRect );

    }

    if (SwPostItMgr* pPostItMgr = GetPostItMgr())
    {
        // No point in showing the cursor for Writer text when there is an
        // active annotation edit.
        if (bVis)
            bVis = !pPostItMgr->HasActiveSidebarWin();
    }

    if( m_bSVCursorVis && bVis ) // also show SV cursor again
        m_pVisibleCursor->Show();
}

void SwCursorShell::VisPortChgd( const SwRect & rRect )
{
    SET_CURR_SHELL( this );
    bool bVis; // switch off all cursors when scrolling

    // if a cursor is visible then hide the SV cursor
    bVis = m_pVisibleCursor->IsVisible();
    if( bVis )
        m_pVisibleCursor->Hide();

    m_bVisPortChgd = true;
    m_aOldRBPos.setX(VisArea().Right());
    m_aOldRBPos.setY(VisArea().Bottom());

    // For not having problems with the SV cursor, Update() is called for the
    // Window in SwViewShell::VisPo...
    // During painting no selections should be shown, thus the call is encapsulated. <- TODO: old artefact?
    SwViewShell::VisPortChgd( rRect ); // move area

    if( m_bSVCursorVis && bVis ) // show SV cursor again
        m_pVisibleCursor->Show();

    if( m_nCursorMove )
        m_bInCMvVisportChgd = true;

    m_bVisPortChgd = false;
}

/** Set the cursor back into content.

    This should only be called if the cursor was move somewhere else (e.g. when
    deleting a border). The new position is calculated from its current position
    in the layout.
*/
void SwCursorShell::UpdateCursorPos()
{
    SET_CURR_SHELL( this );
    ++mnStartAction;
    SwShellCursor* pShellCursor = getShellCursor( true );
    Size aOldSz( GetDocSize() );

    if( isInHiddenTextFrame(pShellCursor) )
    {
        SwCursorMoveState aTmpState( MV_NONE );
        aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();
        GetLayout()->GetCursorOfst( pShellCursor->GetPoint(), pShellCursor->GetPtPos(),
                                     &aTmpState );
        pShellCursor->DeleteMark();
    }
    IGrammarContact *pGrammarContact = GetDoc() ? GetDoc()->getGrammarContact() : nullptr;
    if( pGrammarContact )
        pGrammarContact->updateCursorPosition( *m_pCurrentCursor->GetPoint() );
    --mnStartAction;
    if( aOldSz != GetDocSize() )
        SizeChgNotify();
}

// #i65475# - if Point/Mark in hidden sections, move them out
static bool lcl_CheckHiddenSection( SwNodeIndex& rIdx )
{
    bool bOk = true;
    const SwSectionNode* pSectNd = rIdx.GetNode().FindSectionNode();
    if( pSectNd && pSectNd->GetSection().IsHiddenFlag() )
    {
        SwNodeIndex aTmp( *pSectNd );
        const SwNode* pFrameNd =
            rIdx.GetNodes().FindPrvNxtFrameNode( aTmp, pSectNd->EndOfSectionNode() );
        bOk = pFrameNd != nullptr;
        SAL_WARN_IF(!bOk, "sw.core", "found no Node with Frames");
        rIdx = aTmp;
    }
    return bOk;
}

/// Try to set the cursor to the next visible content node.
static void lcl_CheckHiddenPara( SwPosition& rPos )
{
    SwNodeIndex aTmp( rPos.nNode );
    SwTextNode* pTextNd = aTmp.GetNode().GetTextNode();
    while( pTextNd && pTextNd->HasHiddenCharAttribute( true ) )
    {
        SwContentNode* pContent = aTmp.GetNodes().GoNext( &aTmp );
        if ( pContent && pContent->IsTextNode() )
            pTextNd = pContent->GetTextNode();
        else
            pTextNd = nullptr;
    }

    if ( pTextNd )
        rPos = SwPosition( aTmp, SwIndex( pTextNd, 0 ) );
}

// #i27301# - helper class that notifies the accessibility about invalid text
// selections in its destructor
class SwNotifyAccAboutInvalidTextSelections
{
    private:
        SwCursorShell& mrCursorSh;

    public:
        explicit SwNotifyAccAboutInvalidTextSelections( SwCursorShell& _rCursorSh )
            : mrCursorSh( _rCursorSh )
        {}

        ~SwNotifyAccAboutInvalidTextSelections() COVERITY_NOEXCEPT_FALSE
        {
            mrCursorSh.InvalidateAccessibleParaTextSelection();
        }
};

void SwCursorShell::UpdateCursor( sal_uInt16 eFlags, bool bIdleEnd )
{
    SET_CURR_SHELL( this );
    ClearUpCursors();

    if (ActionPend())
    {
        if ( eFlags & SwCursorShell::READONLY )
            m_bIgnoreReadonly = true;
        return; // if not then no update
    }

    SwNotifyAccAboutInvalidTextSelections aInvalidateTextSelections( *this );

    if ( m_bIgnoreReadonly )
    {
        m_bIgnoreReadonly = false;
        eFlags |= SwCursorShell::READONLY;
    }

    if( eFlags & SwCursorShell::CHKRANGE )    // check all cursor moves for
        CheckRange( m_pCurrentCursor );             // overlapping ranges

    if( !bIdleEnd )
        CheckTableBoxContent();

    // If the current cursor is in a table and point/mark in different boxes,
    // then the table mode is active (also if it is already active: m_pTableCursor)
    SwPaM* pTstCursor = getShellCursor( true );
    if( pTstCursor->HasMark() && !m_pBlockCursor &&
        mxDoc->IsIdxInTable( pTstCursor->GetPoint()->nNode ) &&
          ( m_pTableCursor ||
            pTstCursor->GetNode().StartOfSectionNode() !=
            pTstCursor->GetNode( false ).StartOfSectionNode() ) && !mbSelectAll)
    {
        SwShellCursor* pITmpCursor = getShellCursor( true );
        Point aTmpPt( pITmpCursor->GetPtPos() );
        Point aTmpMk( pITmpCursor->GetMkPos() );
        SwPosition* pPos = pITmpCursor->GetPoint();

        // Bug 65475 (1999) - if Point/Mark in hidden sections, move them out
        lcl_CheckHiddenSection( pPos->nNode );
        lcl_CheckHiddenSection( pITmpCursor->GetMark()->nNode );

        // Move cursor out of hidden paragraphs
        if ( !GetViewOptions()->IsShowHiddenChar() )
        {
            lcl_CheckHiddenPara( *pPos );
            lcl_CheckHiddenPara( *pITmpCursor->GetMark() );
        }

        SwContentFrame *pTableFrame = pPos->nNode.GetNode().GetContentNode()->
                              getLayoutFrame( GetLayout(), &aTmpPt, pPos, false );

        OSL_ENSURE( pTableFrame, "Table Cursor not in Content ??" );

        // --> Make code robust. The table cursor may point
        // to a table in a currently inactive header.
        SwTabFrame *pTab = pTableFrame ? pTableFrame->FindTabFrame() : nullptr;

        if ( pTab && pTab->GetTable()->GetRowsToRepeat() > 0 )
        {
            // First check if point is in repeated headline:
            bool bInRepeatedHeadline = pTab->IsFollow() && pTab->IsInHeadline( *pTableFrame );

            // Second check if mark is in repeated headline:
            if ( !bInRepeatedHeadline )
            {
                SwContentFrame* pMarkTableFrame = pITmpCursor->GetContentNode( false )->
                    getLayoutFrame( GetLayout(), &aTmpMk, pITmpCursor->GetMark(), false );
                OSL_ENSURE( pMarkTableFrame, "Table Cursor not in Content ??" );

                if ( pMarkTableFrame )
                {
                    SwTabFrame* pMarkTab = pMarkTableFrame->FindTabFrame();
                    OSL_ENSURE( pMarkTab, "Table Cursor not in Content ??" );

                    // Make code robust:
                    if ( pMarkTab )
                    {
                        bInRepeatedHeadline = pMarkTab->IsFollow() && pMarkTab->IsInHeadline( *pMarkTableFrame );
                    }
                }
            }

            // No table cursor in repeated headlines:
            if ( bInRepeatedHeadline )
            {
                pTableFrame = nullptr;

                SwMoveFnCollection const & fnPosSect = *pPos <  *pITmpCursor->GetMark()
                                            ? fnSectionStart
                                            : fnSectionEnd;

                // then only select inside the Box
                if( m_pTableCursor )
                {
                    m_pCurrentCursor->SetMark();
                    *m_pCurrentCursor->GetMark() = *m_pTableCursor->GetMark();
                    m_pCurrentCursor->GetMkPos() = m_pTableCursor->GetMkPos();
                    m_pTableCursor->DeleteMark();
                    m_pTableCursor->SwSelPaintRects::Hide();
                }

                *m_pCurrentCursor->GetPoint() = *m_pCurrentCursor->GetMark();
                GoCurrSection( *m_pCurrentCursor, fnPosSect );
            }
        }

        // we really want a table selection
        if( pTab && pTableFrame )
        {
            if( !m_pTableCursor )
            {
                m_pTableCursor = new SwShellTableCursor( *this,
                                *m_pCurrentCursor->GetMark(), m_pCurrentCursor->GetMkPos(),
                                *pPos, aTmpPt );
                m_pCurrentCursor->DeleteMark();
                m_pCurrentCursor->SwSelPaintRects::Hide();

                CheckTableBoxContent();
                if(!m_pTableCursor)
                {
                    SAL_WARN("sw.core", "fdo#74854: "
                        "this should not happen, but better lose the selection "
                        "rather than crashing");
                    return;
                }
            }

            SwCursorMoveState aTmpState( MV_NONE );
            aTmpState.m_bRealHeight = true;
            {
                DisableCallbackAction a(*GetLayout());
                if (!pTableFrame->GetCharRect( m_aCharRect, *m_pTableCursor->GetPoint(), &aTmpState))
                {
                    Point aCentrPt( m_aCharRect.Center() );
                    aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();
                    pTableFrame->GetCursorOfst(m_pTableCursor->GetPoint(), aCentrPt, &aTmpState);
                    bool const bResult =
                        pTableFrame->GetCharRect(m_aCharRect, *m_pTableCursor->GetPoint());
                    OSL_ENSURE( bResult, "GetCharRect failed." );
                }
            }

            m_pVisibleCursor->Hide(); // always hide visible Cursor
            // scroll Cursor to visible area
            if( eFlags & SwCursorShell::SCROLLWIN &&
                (HasSelection() || eFlags & SwCursorShell::READONLY ||
                 !IsCursorReadonly()) )
            {
                SwFrame* pBoxFrame = pTableFrame;
                while( pBoxFrame && !pBoxFrame->IsCellFrame() )
                    pBoxFrame = pBoxFrame->GetUpper();
                if( pBoxFrame && pBoxFrame->getFrameArea().HasArea() )
                    MakeVisible( pBoxFrame->getFrameArea() );
                else
                    MakeVisible( m_aCharRect );
            }

            // let Layout create the Cursors in the Boxes
            if( m_pTableCursor->IsCursorMovedUpdate() )
                GetLayout()->MakeTableCursors( *m_pTableCursor );
            if( m_bHasFocus && !m_bBasicHideCursor )
                m_pTableCursor->Show(nullptr);

            // set Cursor-Points to the new Positions
            m_pTableCursor->GetPtPos().setX(m_aCharRect.Left());
            m_pTableCursor->GetPtPos().setY(m_aCharRect.Top());

            if( m_bSVCursorVis )
            {
                m_aCursorHeight.setX(0);
                m_aCursorHeight.setY(aTmpState.m_aRealHeight.getY() < 0 ?
                                  -m_aCharRect.Width() : m_aCharRect.Height());
                m_pVisibleCursor->Show(); // show again
            }
            m_eMvState = MV_NONE;  // state for cursor travelling - GetCursorOfst
            if( pTableFrame && Imp()->IsAccessible() )
                Imp()->InvalidateAccessibleCursorPosition( pTableFrame );
            return;
        }
    }

    if( m_pTableCursor )
    {
        // delete Ring
        while( m_pCurrentCursor->GetNext() != m_pCurrentCursor )
            delete m_pCurrentCursor->GetNext();
        m_pCurrentCursor->DeleteMark();
        *m_pCurrentCursor->GetPoint() = *m_pTableCursor->GetPoint();
        m_pCurrentCursor->GetPtPos() = m_pTableCursor->GetPtPos();
        delete m_pTableCursor;
        m_pTableCursor = nullptr;
    }

    m_pVisibleCursor->Hide(); // always hide visible Cursor

    // are we perhaps in a protected / hidden Section ?
    {
        SwShellCursor* pShellCursor = getShellCursor( true );
        bool bChgState = true;
        const SwSectionNode* pSectNd = pShellCursor->GetNode().FindSectionNode();
        if( pSectNd && ( pSectNd->GetSection().IsHiddenFlag() ||
            ( !IsReadOnlyAvailable() &&
              pSectNd->GetSection().IsProtectFlag() &&
             ( !mxDoc->GetDocShell() ||
               !mxDoc->GetDocShell()->IsReadOnly() || m_bAllProtect )) ) )
        {
            if( !FindValidContentNode( !HasDrawView() ||
                    0 == Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount()))
            {
                // everything protected/hidden -> special mode
                if( m_bAllProtect && !IsReadOnlyAvailable() &&
                    pSectNd->GetSection().IsProtectFlag() )
                    bChgState = false;
                else
                {
                    m_eMvState = MV_NONE;     // state for cursor travelling
                    m_bAllProtect = true;
                    if( GetDoc()->GetDocShell() )
                    {
                        GetDoc()->GetDocShell()->SetReadOnlyUI();
                        CallChgLnk();       // notify UI!
                    }
                    return;
                }
            }
        }
        if( bChgState )
        {
            bool bWasAllProtect = m_bAllProtect;
            m_bAllProtect = false;
            if( bWasAllProtect && GetDoc()->GetDocShell() &&
                GetDoc()->GetDocShell()->IsReadOnlyUI() )
            {
                GetDoc()->GetDocShell()->SetReadOnlyUI( false );
                CallChgLnk();       // notify UI!
            }
        }
    }

    UpdateCursorPos();

    // The cursor must always point into content; there's some code
    // that relies on this. (E.g. in SwEditShell::GetScriptType, which always
    // loops _behind_ the last node in the selection, which always works if you
    // are in content.) To achieve this, we'll force cursor(s) to point into
    // content, if UpdateCursorPos() hasn't already done so.
    for(SwPaM& rCmp : m_pCurrentCursor->GetRingContainer())
    {
        // start will move forwards, end will move backwards
        bool bPointIsStart = ( rCmp.Start() == rCmp.GetPoint() );

        // move point; forward if it's the start, backwards if it's the end
        if( ! rCmp.GetPoint()->nNode.GetNode().IsContentNode() )
            rCmp.Move( bPointIsStart ? fnMoveForward : fnMoveBackward,
                        GoInContent );

        // move mark (if exists); forward if it's the start, else backwards
        if( rCmp.HasMark() )
        {
            if( ! rCmp.GetMark()->nNode.GetNode().IsContentNode() )
            {
                rCmp.Exchange();
                rCmp.Move( !bPointIsStart ? fnMoveForward : fnMoveBackward,
                            GoInContent );
                rCmp.Exchange();
            }
        }
    }

    SwRect aOld( m_aCharRect );
    bool bFirst = true;
    SwContentFrame *pFrame;
    int nLoopCnt = 100;
    SwShellCursor* pShellCursor = getShellCursor( true );

    do {
        bool bAgainst;
        do {
            bAgainst = false;
            pFrame = pShellCursor->GetContentNode()->getLayoutFrame( GetLayout(),
                        &pShellCursor->GetPtPos(), pShellCursor->GetPoint(), false );
            // if the Frame doesn't exist anymore, the complete Layout has to be
            // created, because there used to be a Frame here!
            if ( !pFrame )
            {
                do
                {
                    CalcLayout();
                    pFrame = pShellCursor->GetContentNode()->getLayoutFrame( GetLayout(),
                                &pShellCursor->GetPtPos(), pShellCursor->GetPoint(), false );
                }  while( !pFrame );
            }
            else if ( Imp()->IsIdleAction() )
                // Guarantee everything's properly formatted
                pFrame->PrepareCursor();

            // In protected Fly? but ignore in case of frame selection
            if( !IsReadOnlyAvailable() && pFrame->IsProtected() &&
                ( !Imp()->GetDrawView() ||
                  !Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() ) &&
                (!mxDoc->GetDocShell() ||
                 !mxDoc->GetDocShell()->IsReadOnly() || m_bAllProtect ) )
            {
                // look for a valid position
                bool bChgState = true;
                if( !FindValidContentNode(!HasDrawView() ||
                    0 == Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount()))
                {
                    // everything is protected / hidden -> special Mode
                    if( m_bAllProtect )
                        bChgState = false;
                    else
                    {
                        m_eMvState = MV_NONE;     // state for cursor travelling
                        m_bAllProtect = true;
                        if( GetDoc()->GetDocShell() )
                        {
                            GetDoc()->GetDocShell()->SetReadOnlyUI();
                            CallChgLnk();       // notify UI!
                        }
                        return;
                    }
                }

                if( bChgState )
                {
                    bool bWasAllProtect = m_bAllProtect;
                    m_bAllProtect = false;
                    if( bWasAllProtect && GetDoc()->GetDocShell() &&
                        GetDoc()->GetDocShell()->IsReadOnlyUI() )
                    {
                        GetDoc()->GetDocShell()->SetReadOnlyUI( false );
                        CallChgLnk();       // notify UI!
                    }
                    m_bAllProtect = false;
                    bAgainst = true; // look for the right Frame again
                }
            }
        } while( bAgainst );

        SwCursorMoveState aTmpState( m_eMvState );
        aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();
        aTmpState.m_bRealHeight = true;
        aTmpState.m_bRealWidth = IsOverwriteCursor();
        aTmpState.m_nCursorBidiLevel = pShellCursor->GetCursorBidiLevel();

        // #i27615#,#i30453#
        SwSpecialPos aSpecialPos;
        aSpecialPos.nExtendRange = SwSPExtendRange::BEFORE;
        if (pShellCursor->IsInFrontOfLabel())
        {
            aTmpState.m_pSpecialPos = &aSpecialPos;
        }

        {
            DisableCallbackAction a(*GetLayout()); // tdf#91602 prevent recursive Action
            if (!pFrame->GetCharRect(m_aCharRect, *pShellCursor->GetPoint(), &aTmpState))
            {
                Point& rPt = pShellCursor->GetPtPos();
                rPt = m_aCharRect.Center();
                pFrame->GetCursorOfst( pShellCursor->GetPoint(), rPt, &aTmpState );
            }
        }
        UISizeNotify(); // tdf#96256 update view size

        if( !pShellCursor->HasMark() )
            m_aCursorHeight = aTmpState.m_aRealHeight;
        else
        {
            m_aCursorHeight.setX(0);
            m_aCursorHeight.setY(aTmpState.m_aRealHeight.getY() < 0 ?
                              -m_aCharRect.Width() : m_aCharRect.Height());
        }

        if( !bFirst && aOld == m_aCharRect )
            break;

        // if the layout says that we are after the 100th iteration still in
        // flow then we should always take the current position for granted.
        // (see bug: 29658)
        if( !--nLoopCnt )
        {
            OSL_ENSURE( false, "endless loop? CharRect != OldCharRect ");
            break;
        }
        aOld = m_aCharRect;
        bFirst = false;

        // update cursor Points to the new Positions
        pShellCursor->GetPtPos().setX(m_aCharRect.Left());
        pShellCursor->GetPtPos().setY(m_aCharRect.Top());

        if( !(eFlags & SwCursorShell::UPDOWN ))   // delete old Pos. of Up/Down
        {
            DisableCallbackAction a(*GetLayout());
            pFrame->Calc(GetOut());
            m_nUpDownX = pFrame->IsVertical() ?
                       m_aCharRect.Top() - pFrame->getFrameArea().Top() :
                       m_aCharRect.Left() - pFrame->getFrameArea().Left();
        }

        // scroll Cursor to visible area
        if( m_bHasFocus && eFlags & SwCursorShell::SCROLLWIN &&
            (HasSelection() || eFlags & SwCursorShell::READONLY ||
             !IsCursorReadonly() || GetViewOptions()->IsSelectionInReadonly()) )
        {
            // in case of scrolling this EndAction doesn't show the SV cursor
            // again, thus save and reset the flag here
            bool bSav = m_bSVCursorVis;
            m_bSVCursorVis = false;
            MakeSelVisible();
            m_bSVCursorVis = bSav;
        }

    } while( eFlags & SwCursorShell::SCROLLWIN );

    if( m_pBlockCursor )
        RefreshBlockCursor();

    // We should not restrict cursor update to the active view when using LOK
    bool bCheckFocus = m_bHasFocus || comphelper::LibreOfficeKit::isActive();

    if( !bIdleEnd && bCheckFocus && !m_bBasicHideCursor )
    {
        if( m_pTableCursor )
            m_pTableCursor->SwSelPaintRects::Show();
        else
        {
            m_pCurrentCursor->SwSelPaintRects::Show();
            if( m_pBlockCursor )
            {
                SwShellCursor* pNxt = m_pCurrentCursor->GetNext();
                while( pNxt && pNxt != m_pCurrentCursor )
                {
                    pNxt->SwSelPaintRects::Show();
                    pNxt = pNxt->GetNext();
                }
            }
        }
    }

    m_eMvState = MV_NONE; // state for cursor travelling - GetCursorOfst

    if( pFrame && Imp()->IsAccessible() )
        Imp()->InvalidateAccessibleCursorPosition( pFrame );

    // switch from blinking cursor to read-only-text-selection cursor
    const sal_uInt64 nBlinkTime = GetOut()->GetSettings().GetStyleSettings().
                            GetCursorBlinkTime();

    if ( (IsCursorReadonly() && GetViewOptions()->IsSelectionInReadonly()) ==
        ( nBlinkTime != STYLE_CURSOR_NOBLINKTIME ) )
    {
        // non blinking cursor in read only - text selection mode
        AllSettings aSettings = GetOut()->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        const sal_uInt64 nNewBlinkTime = nBlinkTime == STYLE_CURSOR_NOBLINKTIME ?
                                   Application::GetSettings().GetStyleSettings().GetCursorBlinkTime() :
                                   STYLE_CURSOR_NOBLINKTIME;
        aStyleSettings.SetCursorBlinkTime( nNewBlinkTime );
        aSettings.SetStyleSettings( aStyleSettings );
        GetOut()->SetSettings( aSettings );
    }

    if( m_bSVCursorVis )
        m_pVisibleCursor->Show(); // show again
}

void SwCursorShell::RefreshBlockCursor()
{
    assert(m_pBlockCursor);
    SwShellCursor &rBlock = m_pBlockCursor->getShellCursor();
    Point aPt = rBlock.GetPtPos();
    SwContentFrame* pFrame = rBlock.GetContentNode()->getLayoutFrame( GetLayout(), &aPt, rBlock.GetPoint(), false );
    Point aMk;
    if( m_pBlockCursor->getEndPoint() && m_pBlockCursor->getStartPoint() )
    {
        aPt = *m_pBlockCursor->getStartPoint();
        aMk = *m_pBlockCursor->getEndPoint();
    }
    else
    {
        aPt = rBlock.GetPtPos();
        if( pFrame )
        {
            if( pFrame->IsVertical() )
                aPt.setY(pFrame->getFrameArea().Top() + GetUpDownX());
            else
                aPt.setX(pFrame->getFrameArea().Left() + GetUpDownX());
        }
        aMk = rBlock.GetMkPos();
    }
    SwRect aRect( aMk, aPt );
    aRect.Justify();
    SwSelectionList aSelList( pFrame );

    if( GetLayout()->FillSelection( aSelList, aRect ) )
    {
        SwCursor* pNxt = static_cast<SwCursor*>(m_pCurrentCursor->GetNext());
        while( pNxt != m_pCurrentCursor )
        {
            delete pNxt;
            pNxt = static_cast<SwCursor*>(m_pCurrentCursor->GetNext());
        }

        std::list<SwPaM*>::iterator pStart = aSelList.getStart();
        std::list<SwPaM*>::iterator pPam = aSelList.getEnd();
        OSL_ENSURE( pPam != pStart, "FillSelection should deliver at least one PaM" );
        m_pCurrentCursor->SetMark();
        --pPam;
        // If there is only one text portion inside the rectangle, a simple
        // selection is created
        if( pPam == pStart )
        {
            *m_pCurrentCursor->GetPoint() = *(*pPam)->GetPoint();
            if( (*pPam)->HasMark() )
                *m_pCurrentCursor->GetMark() = *(*pPam)->GetMark();
            else
                m_pCurrentCursor->DeleteMark();
            delete *pPam;
            m_pCurrentCursor->SetColumnSelection( false );
        }
        else
        {
            // The order of the SwSelectionList has to be preserved but
            // the order inside the ring created by CreateCursor() is not like
            // expected => First create the selections before the last one
            // downto the first selection.
            // At least create the cursor for the last selection
            --pPam;
            *m_pCurrentCursor->GetPoint() = *(*pPam)->GetPoint(); // n-1 (if n == number of selections)
            if( (*pPam)->HasMark() )
                *m_pCurrentCursor->GetMark() = *(*pPam)->GetMark();
            else
                m_pCurrentCursor->DeleteMark();
            delete *pPam;
            m_pCurrentCursor->SetColumnSelection( true );
            while( pPam != pStart )
            {
                --pPam;

                SwShellCursor* pNew = new SwShellCursor( *m_pCurrentCursor );
                pNew->insert( pNew->begin(), m_pCurrentCursor->begin(),  m_pCurrentCursor->end());
                m_pCurrentCursor->clear();
                m_pCurrentCursor->DeleteMark();

                *m_pCurrentCursor->GetPoint() = *(*pPam)->GetPoint(); // n-2, n-3, .., 2, 1
                if( (*pPam)->HasMark() )
                {
                    m_pCurrentCursor->SetMark();
                    *m_pCurrentCursor->GetMark() = *(*pPam)->GetMark();
                }
                else
                    m_pCurrentCursor->DeleteMark();
                m_pCurrentCursor->SetColumnSelection( true );
                delete *pPam;
            }
            {
                SwShellCursor* pNew = new SwShellCursor( *m_pCurrentCursor );
                pNew->insert( pNew->begin(), m_pCurrentCursor->begin(), m_pCurrentCursor->end() );
                m_pCurrentCursor->clear();
                m_pCurrentCursor->DeleteMark();
            }
            pPam = aSelList.getEnd();
            --pPam;
            *m_pCurrentCursor->GetPoint() = *(*pPam)->GetPoint(); // n, the last selection
            if( (*pPam)->HasMark() )
            {
                m_pCurrentCursor->SetMark();
                *m_pCurrentCursor->GetMark() = *(*pPam)->GetMark();
            }
            else
                m_pCurrentCursor->DeleteMark();
            m_pCurrentCursor->SetColumnSelection( true );
            delete *pPam;
        }
    }
}

/// create a copy of the cursor and save it in the stack
void SwCursorShell::Push()
{
    // fdo#60513: if we have a table cursor, copy that; else copy current.
    // This seems to work because UpdateCursor() will fix this up on Pop(),
    // then MakeBoxSels() will re-create the current m_pCurrentCursor cell ring.
    SwShellCursor *const pCurrent((m_pTableCursor) ? m_pTableCursor : m_pCurrentCursor);
    m_pStackCursor = new SwShellCursor( *this, *pCurrent->GetPoint(),
                                    pCurrent->GetPtPos(), m_pStackCursor );

    if (pCurrent->HasMark())
    {
        m_pStackCursor->SetMark();
        *m_pStackCursor->GetMark() = *pCurrent->GetMark();
    }
}

/** delete cursor

    @param eDelete  delete from stack, or delete current
                    and assign the one from stack as the new current cursor.
    @return <true> if there was one on the stack, <false> otherwise
*/
bool SwCursorShell::Pop(PopMode const eDelete)
{
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed

    // are there any left?
    if (nullptr == m_pStackCursor)
        return false;

    SwShellCursor *pTmp = nullptr, *pOldStack = m_pStackCursor;

    // the successor becomes the current one
    if (m_pStackCursor->GetNext() != m_pStackCursor)
    {
        pTmp = m_pStackCursor->GetNext();
    }

    if (PopMode::DeleteStack == eDelete)
        delete m_pStackCursor;

    m_pStackCursor = pTmp; // assign new one

    if (PopMode::DeleteCurrent == eDelete)
    {
        SwCursorSaveState aSaveState( *m_pCurrentCursor );

        // If the visible SSelection was not changed
        const Point& rPoint = pOldStack->GetPtPos();
        if (rPoint == m_pCurrentCursor->GetPtPos() || rPoint == m_pCurrentCursor->GetMkPos())
        {
            // move "Selections Rectangles"
            m_pCurrentCursor->insert( m_pCurrentCursor->begin(), pOldStack->begin(), pOldStack->end() );
            pOldStack->clear();
        }

        if( pOldStack->HasMark() )
        {
            m_pCurrentCursor->SetMark();
            *m_pCurrentCursor->GetMark() = *pOldStack->GetMark();
            m_pCurrentCursor->GetMkPos() = pOldStack->GetMkPos();
        }
        else
            // no selection so revoke old one and set to old position
            m_pCurrentCursor->DeleteMark();
        *m_pCurrentCursor->GetPoint() = *pOldStack->GetPoint();
        m_pCurrentCursor->GetPtPos() = pOldStack->GetPtPos();
        delete pOldStack;

        if( !m_pCurrentCursor->IsInProtectTable( true ) &&
            !m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::Toggle |
                                 SwCursorSelOverFlags::ChangePos ) )
        {
            UpdateCursor(); // update current cursor
            if (m_pTableCursor)
            { // tdf#106929 ensure m_pCurrentCursor ring is recreated from table
                m_pTableCursor->SetChgd();
            }
        }
    }
    return true;
}

/** Combine two cursors

    Delete topmost from stack and use its GetMark in the current.
*/
void SwCursorShell::Combine()
{
    // any others left?
    if (nullptr == m_pStackCursor)
        return;

    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    // rhbz#689053: IsSelOvr must restore the saved stack position, not the
    // current one, because current point + stack mark may be invalid PaM
    SwCursorSaveState aSaveState(*m_pStackCursor);
    // stack cursor & current cursor in same Section?
    assert(!m_pStackCursor->HasMark() ||
            CheckNodesRange(m_pStackCursor->GetMark()->nNode,
                            m_pCurrentCursor->GetPoint()->nNode, true));
    *m_pStackCursor->GetPoint() = *m_pCurrentCursor->GetPoint();
    m_pStackCursor->GetPtPos() = m_pCurrentCursor->GetPtPos();

    SwShellCursor * pTmp = nullptr;
    if (m_pStackCursor->GetNext() != m_pStackCursor)
    {
        pTmp = m_pStackCursor->GetNext();
    }
    delete m_pCurrentCursor;
    m_pCurrentCursor = m_pStackCursor;
    m_pStackCursor->MoveTo(nullptr); // remove from ring
    m_pStackCursor = pTmp;
    if( !m_pCurrentCursor->IsInProtectTable( true ) &&
        !m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::Toggle |
                             SwCursorSelOverFlags::ChangePos ) )
    {
        UpdateCursor(); // update current cursor
    }
}

void SwCursorShell::HideCursors()
{
    if( !m_bHasFocus || m_bBasicHideCursor )
        return;

    // if cursor is visible then hide SV cursor
    if( m_pVisibleCursor->IsVisible() )
    {
        SET_CURR_SHELL( this );
        m_pVisibleCursor->Hide();
    }
    // revoke inversion of SSelection
    SwShellCursor* pCurrentCursor = m_pTableCursor ? m_pTableCursor : m_pCurrentCursor;
    pCurrentCursor->Hide();
}

void SwCursorShell::ShowCursors( bool bCursorVis )
{
    if( !m_bHasFocus || m_bAllProtect || m_bBasicHideCursor )
        return;

    SET_CURR_SHELL( this );
    SwShellCursor* pCurrentCursor = m_pTableCursor ? m_pTableCursor : m_pCurrentCursor;
    pCurrentCursor->Show(nullptr);

    if( m_bSVCursorVis && bCursorVis ) // also show SV cursor again
        m_pVisibleCursor->Show();
}

void SwCursorShell::ShowCursor()
{
    if( !m_bBasicHideCursor )
    {
        m_bSVCursorVis = true;
        m_pCurrentCursor->SetShowTextInputFieldOverlay( true );

        if (comphelper::LibreOfficeKit::isActive())
        {
            OString aPayload = OString::boolean(m_bSVCursorVis);
            GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_CURSOR_VISIBLE, aPayload.getStr());
            SfxLokHelper::notifyOtherViews(GetSfxViewShell(), LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible", aPayload);
        }

        UpdateCursor();
    }
}

void SwCursorShell::HideCursor()
{
    if( !m_bBasicHideCursor )
    {
        m_bSVCursorVis = false;
        // possibly reverse selected areas!!
        SET_CURR_SHELL( this );
        m_pCurrentCursor->SetShowTextInputFieldOverlay( false );
        m_pVisibleCursor->Hide();

        if (comphelper::LibreOfficeKit::isActive())
        {
            OString aPayload = OString::boolean(m_bSVCursorVis);
            GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_CURSOR_VISIBLE, aPayload.getStr());
            SfxLokHelper::notifyOtherViews(GetSfxViewShell(), LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible", aPayload);
        }
    }
}

void SwCursorShell::ShellLoseFocus()
{
    if( !m_bBasicHideCursor )
        HideCursors();
    m_bHasFocus = false;
}

void SwCursorShell::ShellGetFocus()
{
    m_bHasFocus = true;
    if( !m_bBasicHideCursor && VisArea().Width() )
    {
        UpdateCursor( static_cast<sal_uInt16>( SwCursorShell::CHKRANGE ) );
        ShowCursors( m_bSVCursorVis );
    }
}

/** Get current frame in which the cursor is positioned. */
SwContentFrame *SwCursorShell::GetCurrFrame( const bool bCalcFrame ) const
{
    SET_CURR_SHELL( const_cast<SwCursorShell*>(this) );
    SwContentFrame *pRet = nullptr;
    SwContentNode *pNd = m_pCurrentCursor->GetContentNode();
    if ( pNd )
    {
        if ( bCalcFrame )
        {
            sal_uInt16* pST = const_cast<sal_uInt16*>(&mnStartAction);
            ++(*pST);
            const Size aOldSz( GetDocSize() );
            pRet = pNd->getLayoutFrame( GetLayout(), &m_pCurrentCursor->GetPtPos(), m_pCurrentCursor->GetPoint() );
            --(*pST);
            if( aOldSz != GetDocSize() )
                const_cast<SwCursorShell*>(this)->SizeChgNotify();
        }
        else
            pRet = pNd->getLayoutFrame( GetLayout(), &m_pCurrentCursor->GetPtPos(), m_pCurrentCursor->GetPoint(), false);
    }
    return pRet;
}

//TODO: provide documentation
/** forward all attribute/format changes at the current node to the Link

    @param pOld ???
    @param pNew ???
*/
void SwCursorShell::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
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
        // SwTextNode::Insert(SwTextHint*, sal_uInt16); we react here and thus do
        // not need to send the expensive RES_FMT_CHG in Insert.
        CallChgLnk();

    if( m_aGrfArrivedLnk.IsSet() &&
        ( RES_GRAPHIC_ARRIVED == nWhich || RES_GRAPHIC_SWAPIN == nWhich ))
        m_aGrfArrivedLnk.Call( *this );
}

/** Does the current cursor create a selection?

    This means checking if GetMark is set and if SPoint and GetMark differ.
*/
bool SwCursorShell::HasSelection() const
{
    const SwPaM* pCursor = getShellCursor( true );
    return IsTableMode() || ( pCursor->HasMark() && *pCursor->GetPoint() != *pCursor->GetMark() );
}

void SwCursorShell::CallChgLnk()
{
    // Do not make any call in StartAction/EndAction but just set the flag.
    // This will be handled in EndAction.
    if (ActionPend())
        m_bChgCallFlag = true; // remember change
    else if( m_aChgLnk.IsSet() )
    {
        if( m_bCallChgLnk )
            m_aChgLnk.Call( this );
        m_bChgCallFlag = false; // reset flag
    }
}

/// get selected text of a node at current cursor
OUString SwCursorShell::GetSelText() const
{
    OUString aText;
    if( m_pCurrentCursor->GetPoint()->nNode.GetIndex() ==
        m_pCurrentCursor->GetMark()->nNode.GetIndex() )
    {
        SwTextNode* pTextNd = m_pCurrentCursor->GetNode().GetTextNode();
        if( pTextNd )
        {
            const sal_Int32 nStt = m_pCurrentCursor->Start()->nContent.GetIndex();
            aText = pTextNd->GetExpandText( nStt,
                    m_pCurrentCursor->End()->nContent.GetIndex() - nStt );
        }
    }
    return aText;
}

/// get text only from current cursor position (until end of node)
OUString SwCursorShell::GetText() const
{
    OUString aText;
    if( m_pCurrentCursor->GetPoint()->nNode.GetIndex() ==
        m_pCurrentCursor->GetMark()->nNode.GetIndex() )
    {
        SwTextNode* pTextNd = m_pCurrentCursor->GetNode().GetTextNode();
        if( pTextNd )
            aText = pTextNd->GetText().copy(
                    m_pCurrentCursor->GetPoint()->nContent.GetIndex() );
    }
    return aText;
}

/** get the nth character of the current SSelection

    @param bEnd    Start counting from the end? From start otherwise.
    @param nOffset position of the character
*/
sal_Unicode SwCursorShell::GetChar( bool bEnd, long nOffset )
{
    if( IsTableMode() ) // not possible in table mode
        return 0;

    const SwPosition* pPos = !m_pCurrentCursor->HasMark() ? m_pCurrentCursor->GetPoint()
                                : bEnd ? m_pCurrentCursor->End() : m_pCurrentCursor->Start();
    SwTextNode* pTextNd = pPos->nNode.GetNode().GetTextNode();
    if( !pTextNd )
        return 0;

    const sal_Int32 nPos = pPos->nContent.GetIndex();
    const OUString& rStr = pTextNd->GetText();
    sal_Unicode cCh = 0;

    if (((nPos+nOffset) >= 0 ) && (nPos+nOffset) < rStr.getLength())
        cCh = rStr[nPos + nOffset];

    return cCh;
}

/** extend current SSelection by n characters

    @param bEnd   Start counting from the end? From start otherwise.
    @param nCount Number of characters.
*/
bool SwCursorShell::ExtendSelection( bool bEnd, sal_Int32 nCount )
{
    if( !m_pCurrentCursor->HasMark() || IsTableMode() )
        return false; // no selection

    SwPosition* pPos = bEnd ? m_pCurrentCursor->End() : m_pCurrentCursor->Start();
    SwTextNode* pTextNd = pPos->nNode.GetNode().GetTextNode();
    assert(pTextNd);

    sal_Int32 nPos = pPos->nContent.GetIndex();
    if( bEnd )
    {
        if ((nPos + nCount) <= pTextNd->GetText().getLength())
            nPos = nPos + nCount;
        else
            return false; // not possible
    }
    else if( nPos >= nCount )
        nPos = nPos - nCount;
    else
        return false; // not possible anymore

    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed

    pPos->nContent = nPos;
    UpdateCursor();

    return true;
}

/** Move visible cursor to given position in document.

    @param rPt The position to move the visible cursor to.
    @return <false> if SPoint was corrected by the layout.
*/
bool SwCursorShell::SetVisibleCursor( const Point &rPt )
{
    SET_CURR_SHELL( this );
    Point aPt( rPt );
    SwPosition aPos( *m_pCurrentCursor->GetPoint() );
    SwCursorMoveState aTmpState( MV_SETONLYTEXT );
    aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();
    aTmpState.m_bRealHeight = true;

    const bool bRet = GetLayout()->GetCursorOfst( &aPos, aPt /*, &aTmpState*/ );

    SetInFrontOfLabel( false ); // #i27615#

    // show only in TextNodes
    SwTextNode* pTextNd = aPos.nNode.GetNode().GetTextNode();
    if( !pTextNd )
        return false;

    const SwSectionNode* pSectNd = pTextNd->FindSectionNode();
    if( pSectNd && (pSectNd->GetSection().IsHiddenFlag() ||
                    ( !IsReadOnlyAvailable() &&
                      pSectNd->GetSection().IsProtectFlag())) )
        return false;

    SwContentFrame *pFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt, &aPos );
    if ( Imp()->IsIdleAction() )
        pFrame->PrepareCursor();
    SwRect aTmp( m_aCharRect );

    pFrame->GetCharRect( m_aCharRect, aPos, &aTmpState );

    // #i10137#
    if( aTmp == m_aCharRect && m_pVisibleCursor->IsVisible() )
        return true;

    m_pVisibleCursor->Hide(); // always hide visible cursor
    if( IsScrollMDI( this, m_aCharRect ))
    {
        MakeVisible( m_aCharRect );
        m_pCurrentCursor->Show(nullptr);
    }

    {
        if( aTmpState.m_bRealHeight )
            m_aCursorHeight = aTmpState.m_aRealHeight;
        else
        {
            m_aCursorHeight.setX(0);
            m_aCursorHeight.setY(m_aCharRect.Height());
        }

        m_pVisibleCursor->SetDragCursor();
        m_pVisibleCursor->Show(); // show again
    }
    return bRet;
}

bool SwCursorShell::IsOverReadOnlyPos( const Point& rPt ) const
{
    Point aPt( rPt );
    SwPaM aPam( *m_pCurrentCursor->GetPoint() );
    GetLayout()->GetCursorOfst( aPam.GetPoint(), aPt );
    // form view
    return aPam.HasReadonlySel( GetViewOptions()->IsFormView() );
}

/** Get the number of elements in the ring of cursors

    @param bAll If <false> get only spanned ones (= with selections) (Basic).
*/
sal_uInt16 SwCursorShell::GetCursorCnt( bool bAll ) const
{
    SwPaM* pTmp = GetCursor()->GetNext();
    sal_uInt16 n = (bAll || ( m_pCurrentCursor->HasMark() &&
                    *m_pCurrentCursor->GetPoint() != *m_pCurrentCursor->GetMark())) ? 1 : 0;
    while( pTmp != m_pCurrentCursor )
    {
        if( bAll || ( pTmp->HasMark() &&
                *pTmp->GetPoint() != *pTmp->GetMark()))
            ++n;
        pTmp = pTmp->GetNext();
    }
    return n;
}

bool SwCursorShell::IsStartOfDoc() const
{
    if( m_pCurrentCursor->GetPoint()->nContent.GetIndex() )
        return false;

    // after EndOfIcons comes the content selection (EndNd+StNd+ContentNd)
    SwNodeIndex aIdx( GetDoc()->GetNodes().GetEndOfExtras(), 2 );
    if( !aIdx.GetNode().IsContentNode() )
        GetDoc()->GetNodes().GoNext( &aIdx );
    return aIdx == m_pCurrentCursor->GetPoint()->nNode;
}

bool SwCursorShell::IsEndOfDoc() const
{
    SwNodeIndex aIdx( GetDoc()->GetNodes().GetEndOfContent(), -1 );
    SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
    if( !pCNd )
        pCNd = SwNodes::GoPrevious( &aIdx );

    return aIdx == m_pCurrentCursor->GetPoint()->nNode &&
            pCNd->Len() == m_pCurrentCursor->GetPoint()->nContent.GetIndex();
}

/** Invalidate cursors

    Delete all created cursors, set table crsr and last crsr to their TextNode
    (or StartNode?). They will then all re-created at the next ::GetCursor() call.

    This is needed for Drag&Drop/ Clipboard-paste in tables.
*/
bool SwCursorShell::ParkTableCursor()
{
    if( !m_pTableCursor )
        return false;

    m_pTableCursor->ParkCursor();

    while( m_pCurrentCursor->GetNext() != m_pCurrentCursor )
        delete m_pCurrentCursor->GetNext();

    // *always* move cursor's Point and Mark
    m_pCurrentCursor->DeleteMark();
    *m_pCurrentCursor->GetPoint() = *m_pTableCursor->GetPoint();

    return true;
}

void SwCursorShell::ParkPams( SwPaM* pDelRg, SwShellCursor** ppDelRing )
{
    const SwPosition *pStt = pDelRg->Start(),
        *pEnd = pDelRg->GetPoint() == pStt ? pDelRg->GetMark() : pDelRg->GetPoint();

    SwPaM *pTmpDel = nullptr, *pTmp = *ppDelRing;

    // search over the whole ring
    bool bGoNext;
    do {

        if (!pTmp)
            break;

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
            bool bDelete = true;
            if( *ppDelRing == pTmpDel )
            {
                if( *ppDelRing == m_pCurrentCursor )
                {
                    bDelete = GoNextCursor();
                    if( bDelete )
                    {
                        bGoNext = false;
                        pTmp = pTmp->GetNext();
                    }
                }
                else
                    bDelete = false; // never delete the StackCursor
            }

            if( bDelete )
            {
                if (pTmp == pTmpDel)
                    pTmp = nullptr;
                delete pTmpDel; // invalidate old area
            }
            else
            {
                pTmpDel->GetPoint()->nContent.Assign(nullptr, 0);
                pTmpDel->GetPoint()->nNode = 0;
                pTmpDel->DeleteMark();
            }
            pTmpDel = nullptr;
        }
        if( bGoNext && pTmp )
            pTmp = pTmp->GetNext();

    } while( !bGoNext || *ppDelRing != pTmp );
}

//TODO: provide documentation
/** Remove selections and additional cursors of all shells.

    The remaining cursor of the shell is parked.

    @param rIdx ???
*/
void SwCursorShell::ParkCursor( const SwNodeIndex &rIdx )
{
    SwNode *pNode = &rIdx.GetNode();

    // create a new PaM
    std::unique_ptr<SwPaM> pNew( new SwPaM( *GetCursor()->GetPoint() ) );
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
    for(SwViewShell& rTmp : GetRingContainer())
    {
        if( dynamic_cast<const SwCursorShell *>(&rTmp) != nullptr)
        {
            SwCursorShell* pSh = static_cast<SwCursorShell*>(&rTmp);
            if (pSh->m_pStackCursor)
                pSh->ParkPams(pNew.get(), &pSh->m_pStackCursor);

            pSh->ParkPams( pNew.get(), &pSh->m_pCurrentCursor );
            if( pSh->m_pTableCursor )
            {
                // set table cursor always to 0 and the current one always to
                // the beginning of the table
                SwPaM* pTCursor = pSh->GetTableCrs();
                SwNode* pTableNd = pTCursor->GetPoint()->nNode.GetNode().FindTableNode();
                if ( pTableNd )
                {
                    pTCursor->GetPoint()->nContent.Assign(nullptr, 0);
                    pTCursor->GetPoint()->nNode = 0;
                    pTCursor->DeleteMark();
                    pSh->m_pCurrentCursor->GetPoint()->nNode = *pTableNd;
                }
            }
        }
    }
}

/** Copy constructor

    Copy cursor position and add it to the ring.
    All views of a document are in the ring of the shell.
*/
SwCursorShell::SwCursorShell( SwCursorShell& rShell, vcl::Window *pInitWin )
    : SwViewShell( rShell, pInitWin )
    , SwModify( nullptr )
    , m_pStackCursor( nullptr )
    , m_pBlockCursor( nullptr )
    , m_pTableCursor( nullptr )
    , m_pBoxIdx( nullptr )
    , m_pBoxPtr( nullptr )
    , m_nUpDownX(0)
    , m_nLeftFramePos(0)
    , m_nCurrentNode(0)
    , m_nCurrentContent(0)
    , m_nCurrentNdTyp(SwNodeType::NONE)
    , m_nCursorMove( 0 )
    , m_eMvState( MV_NONE )
    , m_sMarkedListId()
    , m_nMarkedListLevel( 0 )
    , m_oldColFrame(nullptr)
{
    SET_CURR_SHELL( this );
    // only keep the position of the current cursor of the copy shell
    m_pCurrentCursor = new SwShellCursor( *this, *(rShell.m_pCurrentCursor->GetPoint()) );
    m_pCurrentCursor->GetContentNode()->Add( this );

    m_bAllProtect = m_bVisPortChgd = m_bChgCallFlag = m_bInCMvVisportChgd =
    m_bGCAttr = m_bIgnoreReadonly = m_bSelTableCells = m_bBasicHideCursor =
    m_bOverwriteCursor = false;
    m_bCallChgLnk = m_bHasFocus = m_bAutoUpdateCells = true;
    m_bSVCursorVis = true;
    m_bSetCursorInReadOnly = true;
    m_pVisibleCursor = new SwVisibleCursor( this );
    m_bMacroExecAllowed = rShell.IsMacroExecAllowed();
}

/// default constructor
SwCursorShell::SwCursorShell( SwDoc& rDoc, vcl::Window *pInitWin,
                            const SwViewOption *pInitOpt )
    : SwViewShell( rDoc, pInitWin, pInitOpt )
    , SwModify( nullptr )
    , m_pStackCursor( nullptr )
    , m_pBlockCursor( nullptr )
    , m_pTableCursor( nullptr )
    , m_pBoxIdx( nullptr )
    , m_pBoxPtr( nullptr )
    , m_nUpDownX(0)
    , m_nLeftFramePos(0)
    , m_nCurrentNode(0)
    , m_nCurrentContent(0)
    , m_nCurrentNdTyp(SwNodeType::NONE)
    , m_nCursorMove( 0 )
    , m_eMvState( MV_NONE ) // state for crsr-travelling - GetCursorOfst
    , m_sMarkedListId()
    , m_nMarkedListLevel( 0 )
    , m_oldColFrame(nullptr)
{
    SET_CURR_SHELL( this );
    // create initial cursor and set it to first content position
    SwNodes& rNds = rDoc.GetNodes();

    SwNodeIndex aNodeIdx( *rNds.GetEndOfContent().StartOfSectionNode() );
    SwContentNode* pCNd = rNds.GoNext( &aNodeIdx ); // go to the first ContentNode

    m_pCurrentCursor = new SwShellCursor( *this, SwPosition( aNodeIdx, SwIndex( pCNd, 0 )));

    // Register shell as dependent at current node. As a result all attribute
    // changes can be forwarded via the Link.
    pCNd->Add( this );

    m_bAllProtect = m_bVisPortChgd = m_bChgCallFlag = m_bInCMvVisportChgd =
    m_bGCAttr = m_bIgnoreReadonly = m_bSelTableCells = m_bBasicHideCursor =
    m_bOverwriteCursor = false;
    m_bCallChgLnk = m_bHasFocus = m_bAutoUpdateCells = true;
    m_bSVCursorVis = true;
    m_bSetCursorInReadOnly = true;

    m_pVisibleCursor = new SwVisibleCursor( this );
    m_bMacroExecAllowed = true;
}

SwCursorShell::~SwCursorShell()
{
    // if it is not the last view then at least the field should be updated
    if( !unique() )
        CheckTableBoxContent( m_pCurrentCursor->GetPoint() );
    else
        ClearTableBoxContent();

    delete m_pVisibleCursor;
    delete m_pBlockCursor;
    delete m_pTableCursor;

    // release cursors
    while(m_pCurrentCursor->GetNext() != m_pCurrentCursor)
        delete m_pCurrentCursor->GetNext();
    delete m_pCurrentCursor;

    // free stack
    if (m_pStackCursor)
    {
        while (m_pStackCursor->GetNext() != m_pStackCursor)
            delete m_pStackCursor->GetNext();
        delete m_pStackCursor;
    }

    // #i54025# - do not give a HTML parser that might potentially hang as
    // a client at the cursor shell the chance to hang itself on a TextNode
    EndListeningAll();
}

SwShellCursor* SwCursorShell::getShellCursor( bool bBlock )
{
    if( m_pTableCursor )
        return m_pTableCursor;
    if( m_pBlockCursor && bBlock )
        return &m_pBlockCursor->getShellCursor();
    return m_pCurrentCursor;
}

/** Should WaitPtr be switched on for the clipboard?

    Wait for TableMode, multiple selections and more than x selected paragraphs.
*/
bool SwCursorShell::ShouldWait() const
{
    if ( IsTableMode() || GetCursorCnt() > 1 )
        return true;

    if( HasDrawView() && GetDrawView()->GetMarkedObjectList().GetMarkCount() )
        return true;

    SwPaM* pPam = GetCursor();
    return pPam->Start()->nNode.GetIndex() + 10 <
            pPam->End()->nNode.GetIndex();
}

size_t SwCursorShell::UpdateTableSelBoxes()
{
    if (m_pTableCursor && (m_pTableCursor->IsChgd() || !m_pTableCursor->GetSelectedBoxesCount()))
    {
         GetLayout()->MakeTableCursors( *m_pTableCursor );
    }
    return (m_pTableCursor) ? m_pTableCursor->GetSelectedBoxesCount() : 0;
}

/// show the current selected "object"
void SwCursorShell::MakeSelVisible()
{
    OSL_ENSURE( m_bHasFocus, "no focus but cursor should be made visible?" );
    if( m_aCursorHeight.Y() < m_aCharRect.Height() && m_aCharRect.Height() > VisArea().Height() )
    {
        SwRect aTmp( m_aCharRect );
        long nDiff = m_aCharRect.Height() - VisArea().Height();
        if( nDiff < m_aCursorHeight.getX() )
            aTmp.Top( nDiff + m_aCharRect.Top() );
        else
        {
            aTmp.Top( m_aCursorHeight.getX() + m_aCharRect.Top() );
            aTmp.Height( m_aCursorHeight.getY() );
        }
        if( !aTmp.HasArea() )
        {
            aTmp.SSize().AdjustHeight(1 );
            aTmp.SSize().AdjustWidth(1 );
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
            aTmp.SSize().AdjustHeight(1 ); aTmp.SSize().AdjustWidth(1 );
            MakeVisible( aTmp );
        }
    }
}

/// search a valid content position (not protected/hidden)
bool SwCursorShell::FindValidContentNode( bool bOnlyText )
{
    if( m_pTableCursor )
    {
        assert(!"Did not remove table selection!");
        return false;
    }

    // #i45129# - everything is allowed in UI-readonly
    if( !m_bAllProtect && GetDoc()->GetDocShell() &&
        GetDoc()->GetDocShell()->IsReadOnlyUI() )
        return true;

    if( m_pCurrentCursor->HasMark() )
        ClearMark();

    // first check for frames
    SwNodeIndex& rNdIdx = m_pCurrentCursor->GetPoint()->nNode;
    sal_uLong nNdIdx = rNdIdx.GetIndex(); // keep backup
    SwNodes& rNds = mxDoc->GetNodes();
    SwContentNode* pCNd = rNdIdx.GetNode().GetContentNode();
    const SwContentFrame * pFrame;

    if( pCNd && nullptr != (pFrame = pCNd->getLayoutFrame( GetLayout(), nullptr, m_pCurrentCursor->GetPoint(), false)) &&
        !IsReadOnlyAvailable() && pFrame->IsProtected() &&
        nNdIdx < rNds.GetEndOfExtras().GetIndex() )
    {
        // skip protected frame
        SwPaM aPam( *m_pCurrentCursor->GetPoint() );
        aPam.SetMark();
        aPam.GetMark()->nNode = rNds.GetEndOfContent();
        aPam.GetPoint()->nNode = *pCNd->EndOfSectionNode();

        bool bFirst = false;
        if( nullptr == (pCNd = ::GetNode( aPam, bFirst, fnMoveForward )))
        {
            aPam.GetMark()->nNode = *rNds.GetEndOfPostIts().StartOfSectionNode();
            pCNd = ::GetNode( aPam, bFirst, fnMoveBackward );
        }

        if( !pCNd ) // should *never* happen
        {
            rNdIdx = nNdIdx; // back to old node
            return false;
        }
        *m_pCurrentCursor->GetPoint() = *aPam.GetPoint();
    }
    else if( bOnlyText && pCNd && pCNd->IsNoTextNode() )
    {
        // set to beginning of document
        rNdIdx = mxDoc->GetNodes().GetEndOfExtras();
        m_pCurrentCursor->GetPoint()->nContent.Assign( mxDoc->GetNodes().GoNext(
                                                            &rNdIdx ), 0 );
        nNdIdx = rNdIdx.GetIndex();
    }

    bool bOk = true;

    // #i9059# cursor may not stand in protected cells
    //         (unless cursor in protected areas is OK.)
    const SwTableNode* pTableNode = rNdIdx.GetNode().FindTableNode();
    if( !IsReadOnlyAvailable()  &&
        pTableNode != nullptr  &&  rNdIdx.GetNode().IsProtect() )
    {
        // we're in a table, and we're in a protected area, so we're
        // probably in a protected cell.

        // move forward into non-protected area.
        SwPaM aPam( rNdIdx.GetNode(), 0 );
        while( aPam.GetNode().IsProtect() &&
               aPam.Move( fnMoveForward, GoInContent ) )
            ; // nothing to do in the loop; the aPam.Move does the moving!

        // didn't work? then go backwards!
        if( aPam.GetNode().IsProtect() )
        {
            SwPaM aTmpPaM( rNdIdx.GetNode(), 0 );
            aPam = aTmpPaM;
            while( aPam.GetNode().IsProtect() &&
                   aPam.Move( fnMoveBackward, GoInContent ) )
                ; // nothing to do in the loop; the aPam.Move does the moving!
        }

        // if we're successful, set the new position
        if( ! aPam.GetNode().IsProtect() )
        {
            *m_pCurrentCursor->GetPoint() = *aPam.GetPoint();
        }
    }

    // in a protected frame
    const SwSectionNode* pSectNd = rNdIdx.GetNode().FindSectionNode();
    if( pSectNd && ( pSectNd->GetSection().IsHiddenFlag() ||
        ( !IsReadOnlyAvailable() &&
           pSectNd->GetSection().IsProtectFlag() )) )
    {
        bOk = false;
        bool bGoNextSection = true;
        for( int nLoopCnt = 0; !bOk && nLoopCnt < 2; ++nLoopCnt )
        {
            bool bContinue;
            do {
                bContinue = false;
                for (;;)
                {
                    if (bGoNextSection)
                        pCNd = rNds.GoNextSection( &rNdIdx,
                                            true, !IsReadOnlyAvailable() );
                    else
                        pCNd = SwNodes::GoPrevSection( &rNdIdx,
                                            true, !IsReadOnlyAvailable() );
                    if ( pCNd == nullptr) break;
                    // moved inside a table -> check if it is protected
                    if( pCNd->FindTableNode() )
                    {
                        SwCallLink aTmp( *this );
                        SwCursorSaveState aSaveState( *m_pCurrentCursor );
                        aTmp.nNdTyp = SwNodeType::NONE; // don't do anything in DTOR
                        if( !m_pCurrentCursor->IsInProtectTable( true ) )
                        {
                            const SwSectionNode* pSNd = pCNd->FindSectionNode();
                            if( !pSNd || !pSNd->GetSection().IsHiddenFlag()
                                || (!IsReadOnlyAvailable()  &&
                                    pSNd->GetSection().IsProtectFlag() ))
                            {
                                bOk = true;
                                break; // found non-protected cell
                            }
                            continue; // continue search
                        }
                    }
                    else
                    {
                        bOk = true;
                        break; // found non-protected cell
                    }
                }

                if( bOk && rNdIdx.GetIndex() < rNds.GetEndOfExtras().GetIndex() )
                {
                    // also check for Fly - might be protected as well
                    if( nullptr == (pFrame = pCNd->getLayoutFrame( GetLayout(), nullptr, nullptr, false)) ||
                        ( !IsReadOnlyAvailable() && pFrame->IsProtected() ) ||
                        ( bOnlyText && pCNd->IsNoTextNode() ) )
                    {
                        // continue search
                        bOk = false;
                        bContinue = true;
                    }
                }
            } while( bContinue );

            if( !bOk )
            {
                if( !nLoopCnt )
                    bGoNextSection = false;
                rNdIdx = nNdIdx;
            }
        }
    }
    if( bOk )
    {
        pCNd = rNdIdx.GetNode().GetContentNode();
        const sal_Int32 nContent = rNdIdx.GetIndex() < nNdIdx ? pCNd->Len() : 0;
        m_pCurrentCursor->GetPoint()->nContent.Assign( pCNd, nContent );
    }
    else
    {
        pCNd = rNdIdx.GetNode().GetContentNode();
        // if cursor in hidden frame, always move it
        if( !pCNd || !pCNd->getLayoutFrame( GetLayout(), nullptr, nullptr, false) )
        {
            SwCursorMoveState aTmpState( MV_NONE );
            aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();
            GetLayout()->GetCursorOfst( m_pCurrentCursor->GetPoint(), m_pCurrentCursor->GetPtPos(),
                                        &aTmpState );
        }
    }
    return bOk;
}

bool SwCursorShell::IsCursorReadonly() const
{
    if ( GetViewOptions()->IsReadonly() ||
         GetViewOptions()->IsFormView() /* Formula view */ )
    {
        SwFrame *pFrame = GetCurrFrame( false );
        const SwFlyFrame* pFly;
        const SwSection* pSection;

        if( pFrame && pFrame->IsInFly() &&
            (pFly = pFrame->FindFlyFrame())->GetFormat()->GetEditInReadonly().GetValue() &&
            pFly->Lower() &&
            !pFly->Lower()->IsNoTextFrame() &&
            !GetDrawView()->GetMarkedObjectList().GetMarkCount() )
        {
            return false;
        }
        // edit in readonly sections
        else if ( pFrame && pFrame->IsInSct() &&
            nullptr != ( pSection = pFrame->FindSctFrame()->GetSection() ) &&
            pSection->IsEditInReadonlyFlag() )
        {
            return false;
        }
        else if ( !IsMultiSelection() && CursorInsideInputField() )
        {
            return false;
        }

        return true;
    }
    return false;
}

/// is the cursor allowed to enter ReadOnly sections?
void SwCursorShell::SetReadOnlyAvailable( bool bFlag )
{
    // *never* switch in GlobalDoc
    if( (!GetDoc()->GetDocShell() ||
         dynamic_cast<const SwGlobalDocShell*>(GetDoc()->GetDocShell()) == nullptr ) &&
        bFlag != m_bSetCursorInReadOnly )
    {
        // If the flag is switched off then all selections need to be
        // invalidated. Otherwise we would trust that nothing protected is selected.
        if( !bFlag )
        {
            ClearMark();
        }
        m_bSetCursorInReadOnly = bFlag;
        UpdateCursor();
    }
}

bool SwCursorShell::HasReadonlySel() const
{
    bool bRet = false;
    // If protected area is to be ignored, then selections are never read-only.
    if ((IsReadOnlyAvailable() || GetViewOptions()->IsFormView() ||
        GetDoc()->GetDocumentSettingManager().get( DocumentSettingId::PROTECT_FORM )) &&
        !SwViewOption::IsIgnoreProtectedArea())
    {
        if ( m_pTableCursor != nullptr )
        {
            bRet = m_pTableCursor->HasReadOnlyBoxSel()
                   || m_pTableCursor->HasReadonlySel( GetViewOptions()->IsFormView() );
        }
        else
        {
            for(const SwPaM& rCursor : m_pCurrentCursor->GetRingContainer())
            {
                if( rCursor.HasReadonlySel( GetViewOptions()->IsFormView() ) )
                {
                    bRet = true;
                    break;
                }
            }
        }
    }
    return bRet;
}

bool SwCursorShell::IsSelFullPara() const
{
    bool bRet = false;

    if( m_pCurrentCursor->GetPoint()->nNode.GetIndex() ==
        m_pCurrentCursor->GetMark()->nNode.GetIndex() && !m_pCurrentCursor->IsMultiSelection() )
    {
        sal_Int32 nStt = m_pCurrentCursor->GetPoint()->nContent.GetIndex();
        sal_Int32 nEnd = m_pCurrentCursor->GetMark()->nContent.GetIndex();
        if( nStt > nEnd )
        {
            sal_Int32 nTmp = nStt;
            nStt = nEnd;
            nEnd = nTmp;
        }
        const SwContentNode* pCNd = m_pCurrentCursor->GetContentNode();
        bRet = pCNd && !nStt && nEnd == pCNd->Len();
    }
    return bRet;
}

SvxFrameDirection SwCursorShell::GetTextDirection( const Point* pPt ) const
{
    SwPosition aPos( *m_pCurrentCursor->GetPoint() );
    Point aPt( pPt ? *pPt : m_pCurrentCursor->GetPtPos() );
    if( pPt )
    {
        SwCursorMoveState aTmpState( MV_NONE );
        aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();

        GetLayout()->GetCursorOfst( &aPos, aPt, &aTmpState );
    }

    return mxDoc->GetTextDirection( aPos, &aPt );
}

bool SwCursorShell::IsInVerticalText( const Point* pPt ) const
{
    const SvxFrameDirection nDir = GetTextDirection( pPt );
    return SvxFrameDirection::Vertical_RL_TB == nDir || SvxFrameDirection::Vertical_LR_TB == nDir;
}

bool SwCursorShell::IsInRightToLeftText() const
{
    const SvxFrameDirection nDir = GetTextDirection();
    // GetTextDirection uses SvxFrameDirection::Vertical_LR_TB to indicate RTL in
    // vertical environment
    return SvxFrameDirection::Vertical_LR_TB == nDir || SvxFrameDirection::Horizontal_RL_TB == nDir;
}

/// If the current cursor position is inside a hidden range, the hidden range
/// is selected.
bool SwCursorShell::SelectHiddenRange()
{
    bool bRet = false;
    if ( !GetViewOptions()->IsShowHiddenChar() && !m_pCurrentCursor->HasMark() )
    {
        SwPosition& rPt = *m_pCurrentCursor->GetPoint();
        const SwTextNode* pNode = rPt.nNode.GetNode().GetTextNode();
        if ( pNode )
        {
            const sal_Int32 nPos = rPt.nContent.GetIndex();

            // check if nPos is in hidden range
            sal_Int32 nHiddenStart;
            sal_Int32 nHiddenEnd;
            SwScriptInfo::GetBoundsOfHiddenRange( *pNode, nPos, nHiddenStart, nHiddenEnd );
            if ( COMPLETE_STRING != nHiddenStart )
            {
                // make selection:
                m_pCurrentCursor->SetMark();
                m_pCurrentCursor->GetMark()->nContent = nHiddenEnd;
                bRet = true;
            }
        }
    }

    return bRet;
}

sal_uLong SwCursorShell::Find( const i18nutil::SearchOptions2& rSearchOpt,
                             bool bSearchInNotes,
                             SwDocPositions eStart, SwDocPositions eEnd,
                             bool& bCancel,
                             FindRanges eRng,
                             bool bReplace )
{
    if( m_pTableCursor )
        GetCursor();
    delete m_pTableCursor;
    m_pTableCursor = nullptr;
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    sal_uLong nRet = m_pCurrentCursor->Find( rSearchOpt, bSearchInNotes, eStart, eEnd,
                                     bCancel, eRng, bReplace );
    if( nRet || bCancel )
        UpdateCursor();
    return nRet;
}

sal_uLong SwCursorShell::Find( const SwTextFormatColl& rFormatColl,
                             SwDocPositions eStart, SwDocPositions eEnd,
                             bool& bCancel,
                             FindRanges eRng,
                             const SwTextFormatColl* pReplFormat )
{
    if( m_pTableCursor )
        GetCursor();
    delete m_pTableCursor;
    m_pTableCursor = nullptr;
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    sal_uLong nRet = m_pCurrentCursor->Find( rFormatColl, eStart, eEnd, bCancel, eRng,
                                     pReplFormat );
    if( nRet )
        UpdateCursor();
    return nRet;
}

sal_uLong SwCursorShell::Find( const SfxItemSet& rSet,
                             bool bNoCollections,
                             SwDocPositions eStart, SwDocPositions eEnd,
                             bool& bCancel,
                             FindRanges eRng,
                             const i18nutil::SearchOptions2* pSearchOpt,
                             const SfxItemSet* rReplSet )
{
    if( m_pTableCursor )
        GetCursor();
    delete m_pTableCursor;
    m_pTableCursor = nullptr;
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed
    sal_uLong nRet = m_pCurrentCursor->Find( rSet, bNoCollections, eStart, eEnd,
                                     bCancel, eRng, pSearchOpt, rReplSet );
    if( nRet )
        UpdateCursor();
    return nRet;
}

void SwCursorShell::SetSelection( const SwPaM& rCursor )
{
    StartAction();
    SwPaM* pCursor = GetCursor();
    *pCursor->GetPoint() = *rCursor.GetPoint();
    if(rCursor.HasMark())
    {
        pCursor->SetMark();
        *pCursor->GetMark() = *rCursor.GetMark();
    }
    if(rCursor.GetNext() != &rCursor)
    {
        const SwPaM *_pStartCursor = rCursor.GetNext();
        do
        {
            SwPaM* pCurrentCursor = CreateCursor();
            *pCurrentCursor->GetPoint() = *_pStartCursor->GetPoint();
            if(_pStartCursor->HasMark())
            {
                pCurrentCursor->SetMark();
                *pCurrentCursor->GetMark() = *_pStartCursor->GetMark();
            }
        } while( (_pStartCursor = _pStartCursor->GetNext()) != &rCursor );
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
    return nullptr != aPos.nNode.GetNode().GetContentNode() &&
           aPos.nContent.GetIdxReg();
}

/**
   Checks if a PaM is valid. For a PaM to be valid its point must be
   valid. Additionally if the PaM has a mark this has to be valid, too.

   @param aPam the PaM to check
*/
static bool lcl_CursorOk(SwPaM & aPam)
{
    return sw_PosOk(*aPam.GetPoint()) && (! aPam.HasMark()
        || sw_PosOk(*aPam.GetMark()));
}

void SwCursorShell::ClearUpCursors()
{
    // start of the ring
    SwPaM * pStartCursor = GetCursor();
    // start loop with second entry of the ring
    SwPaM * pCursor = pStartCursor->GetNext();
    SwPaM * pTmpCursor;
    bool bChanged = false;

    // For all entries in the ring except the start entry delete the entry if
    // it is invalid.
    while (pCursor != pStartCursor)
    {
        pTmpCursor = pCursor->GetNext();
        if ( ! lcl_CursorOk(*pCursor))
        {
            delete pCursor;
            bChanged = true;
        }
        pCursor = pTmpCursor;
    }

    if( pStartCursor->HasMark() && !sw_PosOk( *pStartCursor->GetMark() ) )
    {
        pStartCursor->DeleteMark();
        bChanged = true;
    }
    if( !sw_PosOk( *pStartCursor->GetPoint() ) )
    {
        SwNodes & aNodes = GetDoc()->GetNodes();
        const SwNode* pStart = lcl_NodeContext( pStartCursor->GetPoint()->nNode.GetNode() );
        SwNodeIndex aIdx( pStartCursor->GetPoint()->nNode );
        SwNode * pNode = SwNodes::GoPrevious(&aIdx);
        if( pNode == nullptr || lcl_NodeContext( *pNode ) != pStart )
            aNodes.GoNext( &aIdx );
        if( pNode == nullptr || lcl_NodeContext( *pNode ) != pStart )
        {
            // If the start entry of the ring is invalid replace it with a
            // cursor pointing to the beginning of the first content node in the
            // document.
            aIdx = *(aNodes.GetEndOfContent().StartOfSectionNode());
            pNode = aNodes.GoNext( &aIdx );
        }
        bool bFound = (pNode != nullptr);

        assert(bFound);

        if (bFound)
        {
            SwPaM aTmpPam(*pNode);
            *pStartCursor = aTmpPam;
        }

        bChanged = true;
    }

    // If at least one of the cursors in the ring have been deleted or replaced,
    // remove the table cursor.
    if (m_pTableCursor != nullptr && bChanged)
        TableCursorToCursor();
}

OUString SwCursorShell::GetCursorDescr() const
{
    OUString aResult;

    if (IsMultiSelection())
        aResult += SwResId(STR_MULTISEL);
    else
        aResult = SwDoc::GetPaMDescr(*GetCursor());

    return aResult;
}

void SwCursorShell::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwCursorShell"));

    SwViewShell::dumpAsXml(pWriter);

    xmlTextWriterStartElement(pWriter, BAD_CAST("m_pCurrentCursor"));
    for (SwPaM& rPaM : m_pCurrentCursor->GetRingContainer())
        rPaM.dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);

    xmlTextWriterEndElement(pWriter);
}

static void lcl_FillRecognizerData( std::vector< OUString >& rSmartTagTypes,
                             uno::Sequence< uno::Reference< container::XStringKeyMap > >& rStringKeyMaps,
                             const SwWrongList& rSmartTagList, sal_Int32 nCurrent )
{
    // Insert smart tag information
    std::vector< uno::Reference< container::XStringKeyMap > > aStringKeyMaps;

    for ( sal_uInt16 i = 0; i < rSmartTagList.Count(); ++i )
    {
        const sal_Int32 nSTPos = rSmartTagList.Pos( i );
        const sal_Int32 nSTLen = rSmartTagList.Len( i );

        if ( nSTPos <= nCurrent && nCurrent < nSTPos + nSTLen )
        {
            const SwWrongArea* pArea = rSmartTagList.GetElement( i );
            if ( pArea )
            {
                rSmartTagTypes.push_back( pArea->maType );
                aStringKeyMaps.push_back( pArea->mxPropertyBag );
            }
        }
    }

    if ( !rSmartTagTypes.empty() )
    {
        rStringKeyMaps.realloc( rSmartTagTypes.size() );

        std::vector< uno::Reference< container::XStringKeyMap > >::const_iterator aMapsIter = aStringKeyMaps.begin();
        sal_uInt16 i = 0;
        for ( aMapsIter = aStringKeyMaps.begin(); aMapsIter != aStringKeyMaps.end(); ++aMapsIter )
            rStringKeyMaps[i++] = *aMapsIter;
    }
}

static void lcl_FillTextRange( uno::Reference<text::XTextRange>& rRange,
                   SwTextNode& rNode, sal_Int32 nBegin, sal_Int32 nLen )
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

void SwCursorShell::GetSmartTagTerm( std::vector< OUString >& rSmartTagTypes,
                                   uno::Sequence< uno::Reference< container::XStringKeyMap > >& rStringKeyMaps,
                                   uno::Reference< text::XTextRange>& rRange ) const
{
    if ( !SwSmartTagMgr::Get().IsSmartTagsEnabled() )
        return;

    SwPaM* pCursor = GetCursor();
    SwPosition aPos( *pCursor->GetPoint() );
    SwTextNode *pNode = aPos.nNode.GetNode().GetTextNode();
    if ( pNode && !pNode->IsInProtectSect() )
    {
        const SwWrongList *pSmartTagList = pNode->GetSmartTags();
        if ( pSmartTagList )
        {
            sal_Int32 nCurrent = aPos.nContent.GetIndex();
            sal_Int32 nBegin = nCurrent;
            sal_Int32 nLen = 1;

            if (pSmartTagList->InWrongWord(nBegin, nLen) && !pNode->IsSymbolAt(nBegin))
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
void SwCursorShell::GetSmartTagRect( const Point& rPt, SwRect& rSelectRect )
{
    SwPaM* pCursor = GetCursor();
    SwPosition aPos( *pCursor->GetPoint() );
    Point aPt( rPt );
    SwCursorMoveState eTmpState( MV_SETONLYTEXT );
    SwSpecialPos aSpecialPos;
    eTmpState.m_pSpecialPos = &aSpecialPos;
    SwTextNode *pNode;
    const SwWrongList *pSmartTagList;

    if( GetLayout()->GetCursorOfst( &aPos, aPt, &eTmpState ) &&
        nullptr != (pNode = aPos.nNode.GetNode().GetTextNode()) &&
        nullptr != (pSmartTagList = pNode->GetSmartTags()) &&
        !pNode->IsInProtectSect() )
    {
        sal_Int32 nBegin = aPos.nContent.GetIndex();
        sal_Int32 nLen = 1;

        if (pSmartTagList->InWrongWord(nBegin, nLen) && !pNode->IsSymbolAt(nBegin))
        {
            // get smarttag word
            OUString aText( pNode->GetText().copy(nBegin, nLen) );

            //save the start and end positions of the line and the starting point
            Push();
            LeftMargin();
            const sal_Int32 nLineStart = GetCursor()->GetPoint()->nContent.GetIndex();
            RightMargin();
            const sal_Int32 nLineEnd = GetCursor()->GetPoint()->nContent.GetIndex();
            Pop(PopMode::DeleteCurrent);

            // make sure the selection build later from the data below does not
            // include "in word" character to the left and right in order to
            // preserve those. Therefore count those "in words" in order to
            // modify the selection accordingly.
            const sal_Unicode* pChar = aText.getStr();
            sal_Int32 nLeft = 0;
            while (*pChar++ == CH_TXTATR_INWORD)
                ++nLeft;
            pChar = aText.getLength() ? aText.getStr() + aText.getLength() - 1 : nullptr;
            sal_Int32 nRight = 0;
            while (pChar && *pChar-- == CH_TXTATR_INWORD)
                ++nRight;

            aPos.nContent = nBegin + nLeft;
            pCursor = GetCursor();
            *pCursor->GetPoint() = aPos;
            pCursor->SetMark();
            ExtendSelection( true, nLen - nLeft - nRight );
            // do not determine the rectangle in the current line
            const sal_Int32 nWordStart = (nBegin + nLeft) < nLineStart ? nLineStart : nBegin + nLeft;
            // take one less than the line end - otherwise the next line would
            // be calculated
            const sal_Int32 nWordEnd = std::min(nBegin + nLen - nLeft - nRight, nLineEnd);
            Push();
            pCursor->DeleteMark();
            SwIndex& rContent = GetCursor()->GetPoint()->nContent;
            rContent = nWordStart;
            SwRect aStartRect;
            SwCursorMoveState aState;
            aState.m_bRealWidth = true;
            SwContentNode* pContentNode = pCursor->GetContentNode();
            SwContentFrame *pContentFrame = pContentNode->getLayoutFrame( GetLayout(), &rPt, pCursor->GetPoint(), false);

            pContentFrame->GetCharRect( aStartRect, *pCursor->GetPoint(), &aState );
            rContent = nWordEnd - 1;
            SwRect aEndRect;
            pContentFrame->GetCharRect( aEndRect, *pCursor->GetPoint(),&aState );
            rSelectRect = aStartRect.Union( aEndRect );
            Pop(PopMode::DeleteCurrent);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
