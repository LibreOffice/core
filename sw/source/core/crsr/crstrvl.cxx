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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <svx/svdobj.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <swtable.hxx>
#include <docary.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <txtftn.hxx>
#include <txtinet.hxx>
#include <fmtinfmt.hxx>
#include <txttxmrk.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>
#include <doctxm.hxx>
#include <docfld.hxx>
#include <expfld.hxx>
#include <reffld.hxx>
#include <flddat.hxx>
#include <cellatr.hxx>
#include <swundo.hxx>
#include <redline.hxx>
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <pagedesc.hxx>
#include <fesh.hxx>
#include <charfmt.hxx>
#include <fmturl.hxx>
#include "txtfrm.hxx"
#include <wrong.hxx>
#include <calbck.hxx>
#include <vcl/window.hxx>
#include <docufld.hxx>

using namespace ::com::sun::star;

void SwCrsrShell::MoveCrsrToNum()
{
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *m_pCurCrsr );
    if( ActionPend() )
        return;
    SET_CURR_SHELL( this );
    // try to set cursor onto this position, at half of the char-
    // SRectangle's height
    Point aPt( m_pCurCrsr->GetPtPos() );
    SwCntntFrm * pFrm = m_pCurCrsr->GetCntntNode()->getLayoutFrm( GetLayout(), &aPt,
                                                m_pCurCrsr->GetPoint() );
    pFrm->GetCharRect( m_aCharRect, *m_pCurCrsr->GetPoint() );
    pFrm->Calc();
    if( pFrm->IsVertical() )
    {
        aPt.setX(m_aCharRect.Center().getX());
        aPt.setY(pFrm->Frm().Top() + GetUpDownX());
    }
    else
    {
        aPt.setY(m_aCharRect.Center().getY());
        aPt.setX(pFrm->Frm().Left() + GetUpDownX());
    }
    pFrm->GetCrsrOfst( m_pCurCrsr->GetPoint(), aPt );
    if ( !m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ))
    {
        UpdateCrsr(SwCrsrShell::UPDOWN |
                SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                SwCrsrShell::READONLY );
    }
}

/// go to next/previous point on the same level
bool SwCrsrShell::GotoNextNum()
{
    if (!SwDoc::GotoNextNum( *m_pCurCrsr->GetPoint() ))
        return false;
    MoveCrsrToNum();
    return true;
}

bool SwCrsrShell::GotoPrevNum()
{
    if (!SwDoc::GotoPrevNum( *m_pCurCrsr->GetPoint() ))
        return false;
    MoveCrsrToNum();
    return true;
}

/// jump from content to header
bool SwCrsrShell::GotoHeaderTxt()
{
    const SwFrm* pFrm = GetCurrFrm()->FindPageFrm();
    while( pFrm && !pFrm->IsHeaderFrm() )
        pFrm = pFrm->GetLower();
    // found header, search 1. content frame
    while( pFrm && !pFrm->IsCntntFrm() )
        pFrm = pFrm->GetLower();

    if( pFrm )
    {
        SET_CURR_SHELL( this );
        // get header frame
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCursor *pTmpCrsr = getShellCrsr( true );
        SwCrsrSaveState aSaveState( *pTmpCrsr );
        pFrm->Calc();
        Point aPt( pFrm->Frm().Pos() + pFrm->Prt().Pos() );
        pFrm->GetCrsrOfst( pTmpCrsr->GetPoint(), aPt );
        if( !pTmpCrsr->IsSelOvr() )
            UpdateCrsr();
        else
            pFrm = 0;
    }
    return 0 != pFrm;
}

/// jump from content to footer
bool SwCrsrShell::GotoFooterTxt()
{
    const SwPageFrm* pFrm = GetCurrFrm()->FindPageFrm();
    if( pFrm )
    {
        const SwFrm* pLower = pFrm->GetLastLower();

        while( pLower && !pLower->IsFooterFrm() )
            pLower = pLower->GetLower();
        // found footer, search 1. content frame
        while( pLower && !pLower->IsCntntFrm() )
            pLower = pLower->GetLower();

        if( pLower )
        {
            SwCursor *pTmpCrsr = getShellCrsr( true );
            SET_CURR_SHELL( this );
            // get position in footer
            SwCallLink aLk( *this ); // watch Crsr-Moves
            SwCrsrSaveState aSaveState( *pTmpCrsr );
            pLower->Calc();
            Point aPt( pLower->Frm().Pos() + pLower->Prt().Pos() );
            pLower->GetCrsrOfst( pTmpCrsr->GetPoint(), aPt );
            if( !pTmpCrsr->IsSelOvr() )
                UpdateCrsr();
            else
                pFrm = 0;
        }
        else
            pFrm = 0;
    }
    else
        pFrm = 0;
    return 0 != pFrm;
}

bool SwCrsrShell::SetCrsrInHdFt( size_t nDescNo, bool bInHeader )
{
    bool bRet = false;
    SwDoc *pMyDoc = GetDoc();
    const SwPageDesc* pDesc = NULL;

    SET_CURR_SHELL( this );

    if( SIZE_MAX == nDescNo )
    {
        // take the current one
        const SwPageFrm* pPage = GetCurrFrm()->FindPageFrm();
        if( pPage && pMyDoc->ContainsPageDesc(
                pPage->GetPageDesc(), &nDescNo) )
            pDesc = pPage->GetPageDesc();
    }
    else
        if (nDescNo < pMyDoc->GetPageDescCnt())
            pDesc = &pMyDoc->GetPageDesc( nDescNo );

    if( pDesc )
    {
        // check if the attribute exists
        const SwFmtCntnt* pCnt = 0;
        if( bInHeader )
        {
            // mirrored pages? ignore for now
            const SwFmtHeader& rHd = pDesc->GetMaster().GetHeader();
            if( rHd.GetHeaderFmt() )
                pCnt = &rHd.GetHeaderFmt()->GetCntnt();
        }
        else
        {
            const SwFmtFooter& rFt = pDesc->GetMaster().GetFooter();
            if( rFt.GetFooterFmt() )
                pCnt = &rFt.GetFooterFmt()->GetCntnt();
        }

        if( pCnt && pCnt->GetCntntIdx() )
        {
            SwNodeIndex aIdx( *pCnt->GetCntntIdx(), 1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = pMyDoc->GetNodes().GoNext( &aIdx );

            Point aPt( m_pCurCrsr->GetPtPos() );

            if( pCNd && 0 != pCNd->getLayoutFrm( GetLayout(), &aPt, 0, false ) )
            {
                // then we can set the cursor in here
                SwCallLink aLk( *this ); // watch Crsr-Moves
                SwCrsrSaveState aSaveState( *m_pCurCrsr );

                ClearMark();

                SwPosition& rPos = *m_pCurCrsr->GetPoint();
                rPos.nNode = *pCNd;
                rPos.nContent.Assign( pCNd, 0 );

                bRet = !m_pCurCrsr->IsSelOvr();
                if( bRet )
                    UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                                SwCrsrShell::READONLY );
            }
        }
    }
    return bRet;
}

/// jump to the next index
bool SwCrsrShell::GotoNextTOXBase( const OUString* pName )
{
    bool bRet = false;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    SwCntntNode* pFnd = 0;
    for( SwSectionFmts::size_type n = rFmts.size(); n; )
    {
        const SwSection* pSect = rFmts[ --n ]->GetSection();
        if (TOX_CONTENT_SECTION == pSect->GetType())
        {
            SwSectionNode const*const pSectNd(
                    pSect->GetFmt()->GetSectionNode());
            if (   pSectNd
                && m_pCurCrsr->GetPoint()->nNode < pSectNd->GetIndex()
                && (!pFnd  || pFnd->GetIndex() > pSectNd->GetIndex())
                && (!pName || *pName ==
                    static_cast<SwTOXBaseSection const*>(pSect)->GetTOXName()))
            {
                SwNodeIndex aIdx(*pSectNd, 1);
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if (!pCNd)
                    pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                if (pCNd &&
                    pCNd->EndOfSectionIndex() <= pSectNd->EndOfSectionIndex())
                {
                    SwCntntFrm const*const pCFrm(
                            pCNd->getLayoutFrm(GetLayout()));
                    if (pCFrm &&
                        (IsReadOnlyAvailable() || !pCFrm->IsProtected()))
                    {
                        pFnd = pCNd;
                    }
                }
            }
        }
    }
    if( pFnd )
    {
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *m_pCurCrsr );
        m_pCurCrsr->GetPoint()->nNode = *pFnd;
        m_pCurCrsr->GetPoint()->nContent.Assign( pFnd, 0 );
        bRet = !m_pCurCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

/// jump to previous index
bool SwCrsrShell::GotoPrevTOXBase( const OUString* pName )
{
    bool bRet = false;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    SwCntntNode* pFnd = 0;
    for( SwSectionFmts::size_type n = rFmts.size(); n; )
    {
        const SwSection* pSect = rFmts[ --n ]->GetSection();
        if (TOX_CONTENT_SECTION == pSect->GetType())
        {
            SwSectionNode const*const pSectNd(
                    pSect->GetFmt()->GetSectionNode());
            if (   pSectNd
                && m_pCurCrsr->GetPoint()->nNode > pSectNd->EndOfSectionIndex()
                && (!pFnd  || pFnd->GetIndex() < pSectNd->GetIndex())
                && (!pName || *pName ==
                    static_cast<SwTOXBaseSection const*>(pSect)->GetTOXName()))
            {
                SwNodeIndex aIdx(*pSectNd, 1);
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if (!pCNd)
                    pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                if (pCNd &&
                    pCNd->EndOfSectionIndex() <= pSectNd->EndOfSectionIndex())
                {
                    SwCntntFrm const*const pCFrm(
                            pCNd->getLayoutFrm(GetLayout()));
                    if (pCFrm &&
                        (IsReadOnlyAvailable() || !pCFrm->IsProtected()))
                    {
                        pFnd = pCNd;
                    }
                }
            }
        }
    }

    if( pFnd )
    {
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *m_pCurCrsr );
        m_pCurCrsr->GetPoint()->nNode = *pFnd;
        m_pCurCrsr->GetPoint()->nContent.Assign( pFnd, 0 );
        bRet = !m_pCurCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

/// jump to index of TOXMark
bool SwCrsrShell::GotoTOXMarkBase()
{
    bool bRet = false;

    SwTOXMarks aMarks;
    sal_uInt16 nCnt = SwDoc::GetCurTOXMark( *m_pCurCrsr->GetPoint(), aMarks );
    if( nCnt )
    {
        // Take the 1. and get the index type. Search in its dependency list
        // for the actual index
        const SwTOXType* pType = aMarks[0]->GetTOXType();
        SwIterator<SwTOXBase,SwTOXType> aIter( *pType );
        const SwSectionNode* pSectNd;
        const SwSectionFmt* pSectFmt;

        for( SwTOXBase* pTOX = aIter.First(); pTOX; pTOX = aIter.Next() )
        {
            if( pTOX->ISA( SwTOXBaseSection ) &&
                0 != ( pSectFmt = static_cast<SwTOXBaseSection*>(pTOX)->GetFmt() ) &&
                0 != ( pSectNd = pSectFmt->GetSectionNode() ))
            {
                SwNodeIndex aIdx( *pSectNd, 1 );
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if( !pCNd )
                    pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                const SwCntntFrm* pCFrm;
                if( pCNd &&
                    pCNd->EndOfSectionIndex() < pSectNd->EndOfSectionIndex() &&
                    0 != ( pCFrm = pCNd->getLayoutFrm( GetLayout() ) ) &&
                    ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
                {
                    SwCallLink aLk( *this ); // watch Crsr-Moves
                    SwCrsrSaveState aSaveState( *m_pCurCrsr );
                    m_pCurCrsr->GetPoint()->nNode = *pCNd;
                    m_pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
                    bRet = !m_pCurCrsr->IsInProtectTable() &&
                            !m_pCurCrsr->IsSelOvr();
                    if( bRet )
                        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
                    break;
                }
            }
    }
    }
    return bRet;
}

/// Jump to next/previous table formula
/// Optionally it is possible to also jump to broken formulas
bool SwCrsrShell::GotoNxtPrvTblFormula( bool bNext, bool bOnlyErrors )
{
    if( IsTableMode() )
        return false;

    bool bFnd = false;
    SwPosition& rPos = *m_pCurCrsr->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.nNode = 0;
    _SetGetExpFld aFndGEF( aFndPos ), aCurGEF( rPos );

    {
        const SwNode* pSttNd = rPos.nNode.GetNode().FindTableBoxStartNode();
        if( pSttNd )
        {
            const SwTableBox* pTBox = pSttNd->FindTableNode()->GetTable().
                                        GetTblBox( pSttNd->GetIndex() );
            if( pTBox )
                aCurGEF = _SetGetExpFld( *pTBox );
        }
    }

    if( rPos.nNode < GetDoc()->GetNodes().GetEndOfExtras() )
        // also at collection use only the first frame
        aCurGEF.SetBodyPos( *rPos.nNode.GetNode().GetCntntNode()->getLayoutFrm( GetLayout(),
                                &aPt, &rPos, false ) );
    {
        const SfxPoolItem* pItem;
        const SwTableBox* pTBox;
        sal_uInt32 n, nMaxItems = GetDoc()->GetAttrPool().GetItemCount2( RES_BOXATR_FORMULA );

        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = GetDoc()->GetAttrPool().GetItem2(
                                        RES_BOXATR_FORMULA, n ) ) &&
                0 != (pTBox = static_cast<const SwTblBoxFormula*>(pItem)->GetTableBox() ) &&
                pTBox->GetSttNd() &&
                pTBox->GetSttNd()->GetNodes().IsDocNodes() &&
                ( !bOnlyErrors ||
                  !static_cast<const SwTblBoxFormula*>(pItem)->HasValidBoxes() ) )
            {
                const SwCntntFrm* pCFrm;
                SwNodeIndex aIdx( *pTBox->GetSttNd() );
                const SwCntntNode* pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                if( pCNd && 0 != ( pCFrm = pCNd->getLayoutFrm( GetLayout(), &aPt, 0, false ) ) &&
                    (IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
                {
                    _SetGetExpFld aCmp( *pTBox );
                    aCmp.SetBodyPos( *pCFrm );

                    if( bNext ? ( aCurGEF < aCmp && aCmp < aFndGEF )
                              : ( aCmp < aCurGEF && aFndGEF < aCmp ))
                    {
                        aFndGEF = aCmp;
                        bFnd = true;
                    }
                }
            }
    }

    if( bFnd )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *m_pCurCrsr );

        aFndGEF.GetPosOfContent( rPos );
        m_pCurCrsr->DeleteMark();

        bFnd = !m_pCurCrsr->IsSelOvr();
        if( bFnd )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
    }
    return bFnd;
}

/// jump to next/previous index marker
bool SwCrsrShell::GotoNxtPrvTOXMark( bool bNext )
{
    if( IsTableMode() )
        return false;

    bool bFnd = false;
    SwPosition& rPos = *m_pCurCrsr->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.nNode = 0;
    _SetGetExpFld aFndGEF( aFndPos ), aCurGEF( rPos );

    if( rPos.nNode.GetIndex() < GetDoc()->GetNodes().GetEndOfExtras().GetIndex() )
        // also at collection use only the first frame
        aCurGEF.SetBodyPos( *rPos.nNode.GetNode().
                        GetCntntNode()->getLayoutFrm( GetLayout(), &aPt, &rPos, false ) );

    {
        const SwTxtNode* pTxtNd;
        const SwTxtTOXMark* pTxtTOX;
        sal_uInt32 n, nMaxItems = GetDoc()->GetAttrPool().GetItemCount2( RES_TXTATR_TOXMARK );

        for( n = 0; n < nMaxItems; ++n )
        {
            const SfxPoolItem* pItem;
            const SwCntntFrm* pCFrm;

            if( 0 != (pItem = GetDoc()->GetAttrPool().GetItem2(
                                        RES_TXTATR_TOXMARK, n ) ) &&
                0 != (pTxtTOX = static_cast<const SwTOXMark*>(pItem)->GetTxtTOXMark() ) &&
                ( pTxtNd = &pTxtTOX->GetTxtNode())->GetNodes().IsDocNodes() &&
                0 != ( pCFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt, 0, false )) &&
                ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
            {
                SwNodeIndex aNdIndex( *pTxtNd ); // UNIX needs this object
                _SetGetExpFld aCmp( aNdIndex, *pTxtTOX, 0 );
                aCmp.SetBodyPos( *pCFrm );

                if( bNext ? ( aCurGEF < aCmp && aCmp < aFndGEF )
                          : ( aCmp < aCurGEF && aFndGEF < aCmp ))
                {
                    aFndGEF = aCmp;
                    bFnd = true;
                }
            }
        }
    }

    if( bFnd )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *m_pCurCrsr );

        aFndGEF.GetPosOfContent( rPos );

        bFnd = !m_pCurCrsr->IsSelOvr();
        if( bFnd )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
    }
    return bFnd;
}

/// traveling between marks
const SwTOXMark& SwCrsrShell::GotoTOXMark( const SwTOXMark& rStart,
                                            SwTOXSearch eDir )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *m_pCurCrsr );

    const SwTOXMark& rNewMark = GetDoc()->GotoTOXMark( rStart, eDir,
                                                    IsReadOnlyAvailable() );
    // set position
    SwPosition& rPos = *GetCrsr()->GetPoint();
    rPos.nNode = rNewMark.GetTxtTOXMark()->GetTxtNode();
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(),
                         rNewMark.GetTxtTOXMark()->GetStart() );

    if( !m_pCurCrsr->IsSelOvr() )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );

    return rNewMark;
}

/// jump to next/previous field type
void lcl_MakeFldLst(
    _SetGetExpFlds& rLst,
    const SwFieldType& rFldType,
    const bool bInReadOnly,
    const bool bChkInpFlag = false )
{
    // always search the 1. frame
    Point aPt;
    SwTxtFld* pTxtFld = NULL;
    SwIterator<SwFmtFld,SwFieldType> aIter(rFldType);
    for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
    {
        pTxtFld = pFmtFld->GetTxtFld();
        if ( pTxtFld != NULL
             && ( !bChkInpFlag
                  || static_cast<const SwSetExpField*>(pTxtFld->GetFmtFld().GetField())->GetInputFlag() ) )
        {
            const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
            const SwCntntFrm* pCFrm =
                rTxtNode.getLayoutFrm( rTxtNode.GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );
            if ( pCFrm != NULL
                 && ( bInReadOnly || !pCFrm->IsProtected() ) )
            {
                _SetGetExpFld* pNew = new _SetGetExpFld( SwNodeIndex( rTxtNode ), pTxtFld );
                pNew->SetBodyPos( *pCFrm );
                rLst.insert( pNew );
            }
        }
    }
}

static _SetGetExpFlds::const_iterator
lcl_FindField(bool & o_rFound, _SetGetExpFlds const& rSrtLst,
        SwRootFrm *const pLayout, SwTxtNode *const pTxtNode,
        SwTxtFld *const pTxtFld, SwPosition const& rPos,
        sal_Int32 const nContentOffset)
{
    boost::scoped_ptr<_SetGetExpFld> pSrch;
    boost::scoped_ptr<SwIndex> pIndex;
    if (-1 == nContentOffset)
    {
        pSrch.reset(new _SetGetExpFld(rPos.nNode, pTxtFld, &rPos.nContent));
    }
    else
    {
        pIndex.reset(new SwIndex(rPos.nNode.GetNode().GetCntntNode(), nContentOffset));
        pSrch.reset(new _SetGetExpFld(rPos.nNode, pTxtFld, pIndex.get()));
    }

    if (rPos.nNode.GetIndex() < pTxtNode->GetNodes().GetEndOfExtras().GetIndex())
    {
        // also at collection use only the first frame
        Point aPt;
        pSrch->SetBodyPos(*pTxtNode->getLayoutFrm(pLayout, &aPt, &rPos, false));
    }

    _SetGetExpFlds::const_iterator it = rSrtLst.lower_bound(pSrch.get());

    o_rFound = (it != rSrtLst.end()) && (**it == *pSrch);
    return it;
}

bool SwCrsrShell::MoveFldType(
    const SwFieldType* pFldType,
    const bool bNext,
    const sal_uInt16 nResType,
    const bool bAddSetExpressionFldsToInputFlds )
{
    // sorted list of all fields
    _SetGetExpFlds aSrtLst;

    if ( pFldType )
    {
        if( RES_INPUTFLD != pFldType->Which() && !pFldType->HasWriterListeners() )
        {
            return false;
        }

        // found Modify object, add all fields to array
        ::lcl_MakeFldLst( aSrtLst, *pFldType, IsReadOnlyAvailable() );

        if( RES_INPUTFLD == pFldType->Which() && bAddSetExpressionFldsToInputFlds )
        {
            // there are hidden input fields in the set exp. fields
            const SwFldTypes& rFldTypes = *mpDoc->getIDocumentFieldsAccess().GetFldTypes();
            const size_t nSize = rFldTypes.size();
            for( size_t i=0; i < nSize; ++i )
            {
                pFldType = rFldTypes[ i ];
                if ( RES_SETEXPFLD == pFldType->Which() )
                {
                    ::lcl_MakeFldLst( aSrtLst, *pFldType, IsReadOnlyAvailable(), true );
                }
            }
        }
    }
    else
    {
        const SwFldTypes& rFldTypes = *mpDoc->getIDocumentFieldsAccess().GetFldTypes();
        const size_t nSize = rFldTypes.size();
        for( size_t i=0; i < nSize; ++i )
        {
            pFldType = rFldTypes[ i ];
            if( nResType == pFldType->Which() )
            {
                ::lcl_MakeFldLst( aSrtLst, *pFldType, IsReadOnlyAvailable() );
            }
        }
    }

    // found no fields?
    if( aSrtLst.empty() )
        return false;

    _SetGetExpFlds::const_iterator it;
    SwCursor* pCrsr = getShellCrsr( true );
    {
        // (1998): Always use field for search so that the right one is found as
        // well some are in frames that are anchored to a paragraph that has a
        // field
        const SwPosition& rPos = *pCrsr->GetPoint();

        SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
        OSL_ENSURE( pTNd, "No CntntNode" );

        SwTxtFld * pTxtFld = pTNd->GetFldTxtAttrAt( rPos.nContent.GetIndex(), true );
        const bool bDelFld = ( pTxtFld == NULL );
        sal_Int32 nContentOffset = -1;

        if( bDelFld )
        {
            // create dummy for the search
            SwFmtFld* pFmtFld = new SwFmtFld( SwDateTimeField(
                static_cast<SwDateTimeFieldType*>(mpDoc->getIDocumentFieldsAccess().GetSysFldType( RES_DATETIMEFLD ) ) ) );

            pTxtFld = new SwTxtFld( *pFmtFld, rPos.nContent.GetIndex(),
                        mpDoc->IsClipBoard() );
            pTxtFld->ChgTxtNode( pTNd );
        }
        else
        {
            // the cursor might be anywhere inside the input field,
            // but we will be searching for the field start
            if (pTxtFld->Which() == RES_TXTATR_INPUTFIELD
                    && rPos.nContent.GetIndex() != pTxtFld->GetStart())
                nContentOffset = pTxtFld->GetStart();
        }
        bool isSrch;
        it = lcl_FindField(isSrch, aSrtLst,
                GetLayout(), pTNd, pTxtFld, rPos, nContentOffset);

        if( bDelFld )
        {
            delete static_cast<SwFmtFld*>(&pTxtFld->GetAttr());
            delete pTxtFld;
        }

        if( it != aSrtLst.end() && isSrch ) // found
        {
            if( bNext )
            {
                if( ++it == aSrtLst.end() )
                    return false; // already at the end
            }
            else
            {
                if( it == aSrtLst.begin() )
                    return false; // no more steps backward possible
                --it;
            }
        }
        else // not found
        {
            if( bNext )
            {
                if( it == aSrtLst.end() )
                    return false;
            }
            else
            {
                if( it == aSrtLst.begin() )
                    return false; // no more steps backward possible
                --it;
            }
        }
    }
    const _SetGetExpFld& rFnd = **it;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCrsr );

    rFnd.GetPosOfContent( *pCrsr->GetPoint() );
    bool bRet = !m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                     nsSwCursorSelOverFlags::SELOVER_TOGGLE );
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}

bool SwCrsrShell::GotoFld( const SwFmtFld& rFld )
{
    bool bRet = false;
    if( rFld.GetTxtFld() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves

        SwCursor* pCrsr = getShellCrsr( true );
        SwCrsrSaveState aSaveState( *pCrsr );

        SwTxtNode* pTNd = rFld.GetTxtFld()->GetpTxtNode();
        pCrsr->GetPoint()->nNode = *pTNd;
        pCrsr->GetPoint()->nContent.Assign( pTNd, rFld.GetTxtFld()->GetStart() );

        bRet = !pCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

SwTxtFld * SwCrsrShell::GetTxtFldAtPos(
    const SwPosition* pPos,
    const bool bIncludeInputFldAtStart )
{
    SwTxtFld* pTxtFld = NULL;

    SwTxtNode * const pNode = pPos->nNode.GetNode().GetTxtNode();
    if ( pNode != NULL )
    {
        pTxtFld = pNode->GetFldTxtAttrAt( pPos->nContent.GetIndex(), bIncludeInputFldAtStart );
    }

    return pTxtFld;
}

SwField* SwCrsrShell::GetFieldAtCrsr(
    const SwPaM* pCrsr,
    const bool bIncludeInputFldAtStart )
{
    SwField* pFieldAtCrsr = NULL;

    SwTxtFld* pTxtFld = GetTxtFldAtPos( pCrsr->Start(), bIncludeInputFldAtStart );
    if ( pTxtFld != NULL
        && pCrsr->Start()->nNode == pCrsr->End()->nNode )
    {
        const sal_Int32 nTxtFldLength =
            pTxtFld->End() != NULL
            ? *(pTxtFld->End()) - pTxtFld->GetStart()
            : 1;
        if ( ( pCrsr->End()->nContent.GetIndex() - pCrsr->Start()->nContent.GetIndex() ) <= nTxtFldLength )
        {
            pFieldAtCrsr = const_cast<SwField*>(pTxtFld->GetFmtFld().GetField());
        }
    }

    return pFieldAtCrsr;
}

SwField* SwCrsrShell::GetCurFld( const bool bIncludeInputFldAtStart ) const
{
    SwPaM* pCrsr = GetCrsr();
    if ( pCrsr->IsMultiSelection() )
    {
        // multi selection not handled.
        return NULL;
    }

    SwField* pCurFld = GetFieldAtCrsr( pCrsr, bIncludeInputFldAtStart );;
    if ( pCurFld != NULL
         && RES_TABLEFLD == pCurFld->GetTyp()->Which() )
    {
        // TabellenFormel ? wandel internen in externen Namen um
        const SwTableNode* pTblNd = IsCrsrInTbl();
        static_cast<SwTblField*>(pCurFld)->PtrToBoxNm( pTblNd ? &pTblNd->GetTable() : 0 );
    }

    return pCurFld;
}

bool SwCrsrShell::CrsrInsideInputFld() const
{
    for(SwPaM& rCrsr : GetCrsr()->GetRingContainer())
    {
        if(dynamic_cast<const SwInputField*>(GetFieldAtCrsr( &rCrsr, false )))
            return true;
    }
    return false;
}

bool SwCrsrShell::PosInsideInputFld( const SwPosition& rPos )
{
    return dynamic_cast<const SwTxtInputFld*>(GetTxtFldAtPos( &rPos, false )) != NULL;
}

bool SwCrsrShell::DocPtInsideInputFld( const Point& rDocPt ) const
{
    SwPosition aPos( *(GetCrsr()->Start()) );
    Point aDocPt( rDocPt );
    if ( GetLayout()->GetCrsrOfst( &aPos, aDocPt ) )
    {
        return PosInsideInputFld( aPos );
    }
    return false;
}

sal_Int32 SwCrsrShell::StartOfInputFldAtPos( const SwPosition& rPos )
{
    const SwTxtInputFld* pTxtInputFld = dynamic_cast<const SwTxtInputFld*>(GetTxtFldAtPos( &rPos, true ));
    if ( pTxtInputFld == NULL )
    {
        OSL_ENSURE( false, "<SwEditShell::StartOfInputFldAtPos(..)> - no Input Field at given position" );
        return 0;
    }
    return pTxtInputFld->GetStart();
}

sal_Int32 SwCrsrShell::EndOfInputFldAtPos( const SwPosition& rPos )
{
    const SwTxtInputFld* pTxtInputFld = dynamic_cast<const SwTxtInputFld*>(GetTxtFldAtPos( &rPos, true ));
    if ( pTxtInputFld == NULL )
    {
        OSL_ENSURE( false, "<SwEditShell::EndOfInputFldAtPos(..)> - no Input Field at given position" );
        return 0;
    }
    return *(pTxtInputFld->End());
}

void SwCrsrShell::GotoOutline( sal_uInt16 nIdx )
{
    SwCursor* pCrsr = getShellCrsr( true );

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCrsr );

    const SwNodes& rNds = GetDoc()->GetNodes();
    SwTxtNode* pTxtNd = rNds.GetOutLineNds()[ nIdx ]->GetTxtNode();
    pCrsr->GetPoint()->nNode = *pTxtNd;
    pCrsr->GetPoint()->nContent.Assign( pTxtNd, 0 );

    if( !pCrsr->IsSelOvr() )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
}

bool SwCrsrShell::GotoOutline( const OUString& rName )
{
    SwCursor* pCrsr = getShellCrsr( true );

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCrsr );

    bool bRet = false;
    if( mpDoc->GotoOutline( *pCrsr->GetPoint(), rName ) && !pCrsr->IsSelOvr() )
    {
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        bRet = true;
    }
    return bRet;
}

/// jump to next node with outline num.
bool SwCrsrShell::GotoNextOutline()
{
    SwCursor* pCrsr = getShellCrsr( true );
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = &(pCrsr->GetNode());
    sal_uInt16 nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        ++nPos;

    if( nPos == rNds.GetOutLineNds().size() )
        return false;

    pNd = rNds.GetOutLineNds()[ nPos ];

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCrsr );
    pCrsr->GetPoint()->nNode = *pNd;
    pCrsr->GetPoint()->nContent.Assign( pNd->GetTxtNode(), 0 );

    bool bRet = !pCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}

/// jump to previous node with outline num.
bool SwCrsrShell::GotoPrevOutline()
{
    SwCursor* pCrsr = getShellCrsr( true );
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = &(pCrsr->GetNode());
    sal_uInt16 nPos;
    bool bRet = rNds.GetOutLineNds().Seek_Entry(pNd, &nPos);
    if (bRet && nPos)
    {
        --nPos; // before

        pNd = rNds.GetOutLineNds()[ nPos ];
        if( pNd->GetIndex() > pCrsr->GetPoint()->nNode.GetIndex() )
            return false;

        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCrsr );
        pCrsr->GetPoint()->nNode = *pNd;
        pCrsr->GetPoint()->nContent.Assign( pNd->GetTxtNode(), 0 );

        bRet = !pCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

/// search "outline position" before previous outline node at given level
sal_uInt16 SwCrsrShell::GetOutlinePos( sal_uInt8 nLevel )
{
    SwPaM* pCrsr = getShellCrsr( true );
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = &(pCrsr->GetNode());
    sal_uInt16 nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        nPos++; // is at correct position; take next for while

    while( nPos-- ) // check the one in front of the current
    {
        pNd = rNds.GetOutLineNds()[ nPos ];

        if( pNd->GetTxtNode()->GetAttrOutlineLevel()-1 <= nLevel )
            return nPos;

    }
    return USHRT_MAX; // no more left
}

bool SwCrsrShell::MakeOutlineSel( sal_uInt16 nSttPos, sal_uInt16 nEndPos,
                                  bool bWithChildren )
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    if( rOutlNds.empty() )
        return false;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves

    if( nSttPos > nEndPos ) // parameters switched?
    {
        OSL_ENSURE( false, "Start > End for array access" );
        sal_uInt16 nTmp = nSttPos;
        nSttPos = nEndPos;
        nEndPos = nTmp;
    }

    SwNode* pSttNd = rOutlNds[ nSttPos ];
    SwNode* pEndNd = rOutlNds[ nEndPos ];

    if( bWithChildren )
    {
        const int nLevel = pEndNd->GetTxtNode()->GetAttrOutlineLevel()-1;
        for( ++nEndPos; nEndPos < rOutlNds.size(); ++nEndPos )
        {
            pEndNd = rOutlNds[ nEndPos ];
            const int nNxtLevel = pEndNd->GetTxtNode()->GetAttrOutlineLevel()-1;
            if( nNxtLevel <= nLevel )
                break; // EndPos is now on the next one
        }
    }
    // if without children then set onto next one
    else if( ++nEndPos < rOutlNds.size() )
        pEndNd = rOutlNds[ nEndPos ];

    if( nEndPos == rOutlNds.size() ) // no end found
        pEndNd = &rNds.GetEndOfContent();

    KillPams();

    SwCrsrSaveState aSaveState( *m_pCurCrsr );

    // set end to the end of the previous content node
    m_pCurCrsr->GetPoint()->nNode = *pSttNd;
    m_pCurCrsr->GetPoint()->nContent.Assign( pSttNd->GetCntntNode(), 0 );
    m_pCurCrsr->SetMark();
    m_pCurCrsr->GetPoint()->nNode = *pEndNd;
    m_pCurCrsr->Move( fnMoveBackward, fnGoNode ); // end of predecessor

    // and everything is already selected
    bool bRet = !m_pCurCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}

/// jump to reference marker
bool SwCrsrShell::GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType,
                                    sal_uInt16 nSeqNo )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *m_pCurCrsr );

    sal_Int32 nPos = -1;
    SwTxtNode* pTxtNd = SwGetRefFieldType::FindAnchor( GetDoc(), rRefMark,
                                                    nSubType, nSeqNo, &nPos );
    if( pTxtNd && pTxtNd->GetNodes().IsDocNodes() )
    {
        m_pCurCrsr->GetPoint()->nNode = *pTxtNd;
        m_pCurCrsr->GetPoint()->nContent.Assign( pTxtNd, nPos );

        if( !m_pCurCrsr->IsSelOvr() )
        {
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
            return true;
        }
    }
    return false;
}

bool SwCrsrShell::IsPageAtPos( const Point &rPt ) const
{
    if( GetLayout() )
        return 0 != GetLayout()->GetPageAtPos( rPt );
    return false;
}

bool SwCrsrShell::GetContentAtPos( const Point& rPt,
                                   SwContentAtPos& rCntntAtPos,
                                   bool bSetCrsr,
                                   SwRect* pFldRect )
{
    SET_CURR_SHELL( this );
    bool bRet = false;

    if( !IsTableMode() )
    {
        Point aPt( rPt );
        SwPosition aPos( *m_pCurCrsr->GetPoint() );

        SwTxtNode* pTxtNd;
        SwCntntFrm *pFrm(0);
        SwTxtAttr* pTxtAttr;
        SwCrsrMoveState aTmpState;
        aTmpState.bFieldInfo = true;
        aTmpState.bExactOnly = !( SwContentAtPos::SW_OUTLINE & rCntntAtPos.eCntntAtPos );
        aTmpState.bCntntCheck = (SwContentAtPos::SW_CONTENT_CHECK & rCntntAtPos.eCntntAtPos) != 0;
        aTmpState.bSetInReadOnly = IsReadOnlyAvailable();

        SwSpecialPos aSpecialPos;
        aTmpState.pSpecialPos = ( SwContentAtPos::SW_SMARTTAG & rCntntAtPos.eCntntAtPos ) ?
                                &aSpecialPos : 0;

        const bool bCrsrFoundExact = GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState );
        pTxtNd = aPos.nNode.GetNode().GetTxtNode();

        const SwNodes& rNds = GetDoc()->GetNodes();
        if( pTxtNd
            && SwContentAtPos::SW_OUTLINE & rCntntAtPos.eCntntAtPos
            && !rNds.GetOutLineNds().empty() )
        {
            const SwTxtNode* pONd = pTxtNd->FindOutlineNodeOfLevel( MAXLEVEL-1);
            if( pONd )
            {
                rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_OUTLINE;
                rCntntAtPos.sStr = pONd->GetExpandTxt( 0, -1, true, true );
                bRet = true;
            }
        }
        else if ( SwContentAtPos::SW_CONTENT_CHECK & rCntntAtPos.eCntntAtPos
                  && bCrsrFoundExact )
        {
            bRet = true;
        }
        else if( pTxtNd
                 && SwContentAtPos::SW_NUMLABEL & rCntntAtPos.eCntntAtPos)
        {
            bRet = aTmpState.bInNumPortion;
            rCntntAtPos.aFnd.pNode = pTxtNd;

            Size aSizeLogic(aTmpState.nInNumPostionOffset, 0);
            Size aSizePixel = GetWin()->LogicToPixel(aSizeLogic);
            rCntntAtPos.nDist = aSizePixel.Width();
        }
        else if( bCrsrFoundExact && pTxtNd )
        {
            if( !aTmpState.bPosCorr )
            {
                if ( !bRet
                     && SwContentAtPos::SW_SMARTTAG & rCntntAtPos.eCntntAtPos
                     && !aTmpState.bFtnNoInfo )
                {
                    const SwWrongList* pSmartTagList = pTxtNd->GetSmartTags();
                    sal_Int32 nCurrent = aPos.nContent.GetIndex();
                    const sal_Int32 nBegin = nCurrent;
                    sal_Int32 nLen = 1;

                    if ( pSmartTagList && pSmartTagList->InWrongWord( nCurrent, nLen ) && !pTxtNd->IsSymbol(nBegin) )
                    {
                        const sal_uInt16 nIndex = pSmartTagList->GetWrongPos( nBegin );
                        const SwWrongList* pSubList = pSmartTagList->SubList( nIndex );
                        if ( pSubList )
                        {
                            nCurrent = aTmpState.pSpecialPos->nCharOfst;

                            if ( pSubList->InWrongWord( nCurrent, nLen ) )
                                bRet = true;
                        }
                        else
                            bRet = true;

                        if( bRet && bSetCrsr )
                        {
                            SwCrsrSaveState aSaveState( *m_pCurCrsr );
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            m_pCurCrsr->DeleteMark();
                            *m_pCurCrsr->GetPoint() = aPos;
                            if( m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION | nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = false;
                            else
                                UpdateCrsr();
                        }
                        if( bRet )
                        {
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_SMARTTAG;

                            if( pFldRect && 0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt ) ) )
                                pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                        }
                    }
                }

                if ( !bRet
                     && ( SwContentAtPos::SW_FIELD | SwContentAtPos::SW_CLICKFIELD ) & rCntntAtPos.eCntntAtPos
                     && !aTmpState.bFtnNoInfo )
                {
                    pTxtAttr = pTxtNd->GetFldTxtAttrAt( aPos.nContent.GetIndex() );
                    const SwField* pFld = pTxtAttr != NULL
                                          ? pTxtAttr->GetFmtFld().GetField()
                                          : 0;
                    if ( SwContentAtPos::SW_CLICKFIELD & rCntntAtPos.eCntntAtPos
                         && pFld && !pFld->HasClickHdl() )
                    {
                        pFld = 0;
                    }

                    if ( pFld )
                    {
                        if( pFldRect && 0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt ) ) )
                            pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );

                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            SwCrsrSaveState aSaveState( *m_pCurCrsr );
                            m_pCurCrsr->DeleteMark();
                            *m_pCurCrsr->GetPoint() = aPos;
                            if( m_pCurCrsr->IsSelOvr() )
                            {
                                // allow click fields in protected sections
                                // only placeholder is not possible
                                if( SwContentAtPos::SW_FIELD & rCntntAtPos.eCntntAtPos
                                    || RES_JUMPEDITFLD == pFld->Which() )
                                    pFld = 0;
                            }
                            else
                                UpdateCrsr();
                        }
                        else if( RES_TABLEFLD == pFld->Which() &&
                            static_cast<const SwTblField*>(pFld)->IsIntrnlName() )
                        {
                            // create from internal (for CORE) the external
                            // (for UI) formula
                            const SwTableNode* pTblNd = pTxtNd->FindTableNode();
                            if( pTblNd )        // steht in einer Tabelle
                                const_cast<SwTblField*>(static_cast<const SwTblField*>(pFld))->PtrToBoxNm( &pTblNd->GetTable() );
                        }
                    }

                    if( pFld )
                    {
                        rCntntAtPos.aFnd.pFld = pFld;
                        rCntntAtPos.pFndTxtAttr = pTxtAttr;
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FIELD;
                        bRet = true;
                    }
                }

                if( !bRet && SwContentAtPos::SW_FORMCTRL & rCntntAtPos.eCntntAtPos )
                {
                    IDocumentMarkAccess* pMarksAccess = GetDoc()->getIDocumentMarkAccess( );
                    sw::mark::IFieldmark* pFldBookmark = pMarksAccess->getFieldmarkFor( aPos );
                    if( bCrsrFoundExact && pTxtNd && pFldBookmark) {
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FORMCTRL;
                        rCntntAtPos.aFnd.pFldmark = pFldBookmark;
                        bRet=true;
                    }
                }

                if( !bRet && SwContentAtPos::SW_FTN & rCntntAtPos.eCntntAtPos )
                {
                    if( aTmpState.bFtnNoInfo )
                    {
                        // over the footnote's char
                        bRet = true;
                        if( bSetCrsr )
                        {
                            *m_pCurCrsr->GetPoint() = aPos;
                            if( !GotoFtnAnchor() )
                                bRet = false;
                        }
                        if( bRet )
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FTN;
                    }
                    else if ( 0 != ( pTxtAttr = pTxtNd->GetTxtAttrForCharAt(
                        aPos.nContent.GetIndex(), RES_TXTATR_FTN )) )
                    {
                        bRet = true;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            SwCrsrSaveState aSaveState( *m_pCurCrsr );
                            m_pCurCrsr->GetPoint()->nNode = *static_cast<SwTxtFtn*>(pTxtAttr)->GetStartNode();
                            SwCntntNode* pCNd = GetDoc()->GetNodes().GoNextSection(
                                &m_pCurCrsr->GetPoint()->nNode,
                                true, !IsReadOnlyAvailable() );

                            if( pCNd )
                            {
                                m_pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
                                if( m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                    nsSwCursorSelOverFlags::SELOVER_TOGGLE ))
                                    bRet = false;
                                else
                                    UpdateCrsr();
                            }
                            else
                                bRet = false;
                        }

                        if( bRet )
                        {
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FTN;
                            rCntntAtPos.pFndTxtAttr = pTxtAttr;
                            rCntntAtPos.aFnd.pAttr = &pTxtAttr->GetAttr();

                            if( pFldRect && 0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt ) ) )
                                pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                        }
                    }
                }

                if( !bRet
                    && ( SwContentAtPos::SW_TOXMARK | SwContentAtPos::SW_REFMARK ) & rCntntAtPos.eCntntAtPos
                    && !aTmpState.bFtnNoInfo )
                {
                    pTxtAttr = 0;
                    if( SwContentAtPos::SW_TOXMARK & rCntntAtPos.eCntntAtPos )
                    {
                        ::std::vector<SwTxtAttr *> const marks(
                            pTxtNd->GetTxtAttrsAt(
                               aPos.nContent.GetIndex(), RES_TXTATR_TOXMARK));
                        if (marks.size())
                        {   // hmm... can only return 1 here
                            pTxtAttr = *marks.begin();
                        }
                    }

                    if( !pTxtAttr &&
                        SwContentAtPos::SW_REFMARK & rCntntAtPos.eCntntAtPos )
                    {
                        ::std::vector<SwTxtAttr *> const marks(
                            pTxtNd->GetTxtAttrsAt(
                               aPos.nContent.GetIndex(), RES_TXTATR_REFMARK));
                        if (marks.size())
                        {   // hmm... can only return 1 here
                            pTxtAttr = *marks.begin();
                        }
                    }

                    if( pTxtAttr )
                    {
                        bRet = true;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            SwCrsrSaveState aSaveState( *m_pCurCrsr );
                            m_pCurCrsr->DeleteMark();
                            *m_pCurCrsr->GetPoint() = aPos;
                            if( m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION | nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
                                bRet = false;
                            else
                                UpdateCrsr();
                        }

                        if( bRet )
                        {
                            const sal_Int32* pEnd = pTxtAttr->GetEnd();
                            if( pEnd )
                                rCntntAtPos.sStr =
                                    pTxtNd->GetExpandTxt( pTxtAttr->GetStart(), *pEnd - pTxtAttr->GetStart() );
                            else if( RES_TXTATR_TOXMARK == pTxtAttr->Which())
                                rCntntAtPos.sStr =
                                    pTxtAttr->GetTOXMark().GetAlternativeText();

                            rCntntAtPos.eCntntAtPos =
                                RES_TXTATR_TOXMARK == pTxtAttr->Which()
                                ? SwContentAtPos::SW_TOXMARK
                                : SwContentAtPos::SW_REFMARK;
                            rCntntAtPos.pFndTxtAttr = pTxtAttr;
                            rCntntAtPos.aFnd.pAttr = &pTxtAttr->GetAttr();

                            if( pFldRect && 0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt ) ) )
                                pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                        }
                    }
                }

                if ( !bRet
                     && SwContentAtPos::SW_INETATTR & rCntntAtPos.eCntntAtPos
                     && !aTmpState.bFtnNoInfo )
                {
                    pTxtAttr = pTxtNd->GetTxtAttrAt(
                            aPos.nContent.GetIndex(), RES_TXTATR_INETFMT);
                    // "detect" only INetAttrs with URLs
                    if( pTxtAttr && !pTxtAttr->GetINetFmt().GetValue().isEmpty() )
                    {
                        bRet = true;
                        if( bSetCrsr )
                        {
                            SwCrsrSaveState aSaveState( *m_pCurCrsr );
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            m_pCurCrsr->DeleteMark();
                            *m_pCurCrsr->GetPoint() = aPos;
                            if( m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = false;
                            else
                                UpdateCrsr();
                        }
                        if( bRet )
                        {
                            rCntntAtPos.sStr = pTxtNd->GetExpandTxt(
                                pTxtAttr->GetStart(),
                                *pTxtAttr->GetEnd() - pTxtAttr->GetStart() );

                            rCntntAtPos.aFnd.pAttr = &pTxtAttr->GetAttr();
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_INETATTR;
                            rCntntAtPos.pFndTxtAttr = pTxtAttr;

                            if( pFldRect && 0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt ) ) )
                                pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                        }
                    }
                }

                if( !bRet && SwContentAtPos::SW_REDLINE & rCntntAtPos.eCntntAtPos )
                {
                    const SwRangeRedline* pRedl = GetDoc()->getIDocumentRedlineAccess().GetRedline(aPos, NULL);
                    if( pRedl )
                    {
                        rCntntAtPos.aFnd.pRedl = pRedl;
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_REDLINE;
                        rCntntAtPos.pFndTxtAttr = 0;
                        bRet = true;

                        if( pFldRect && 0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt ) ) )
                            pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                    }
                }
            }

            if( !bRet
                 && ( SwContentAtPos::SW_TABLEBOXFML & rCntntAtPos.eCntntAtPos
#ifdef DBG_UTIL
                      || SwContentAtPos::SW_TABLEBOXVALUE & rCntntAtPos.eCntntAtPos
#endif
                ) )
            {
                const SwTableNode* pTblNd;
                const SwTableBox* pBox;
                const SwStartNode* pSttNd = pTxtNd->FindTableBoxStartNode();
                const SfxPoolItem* pItem;
                if( pSttNd && 0 != ( pTblNd = pTxtNd->FindTableNode()) &&
                    0 != ( pBox = pTblNd->GetTable().GetTblBox(
                    pSttNd->GetIndex() )) &&
#ifdef DBG_UTIL
                    ( SfxItemState::SET == pBox->GetFrmFmt()->GetItemState(
                    RES_BOXATR_FORMULA, false, &pItem ) ||
                    SfxItemState::SET == pBox->GetFrmFmt()->GetItemState(
                    RES_BOXATR_VALUE, false, &pItem ))
#else
                    SfxItemState::SET == pBox->GetFrmFmt()->GetItemState(
                    RES_BOXATR_FORMULA, false, &pItem )
#endif
                    )
                {
                    SwFrm* pF = pTxtNd->getLayoutFrm( GetLayout(), &aPt );
                    if( pF )
                    {
                        // then the CellFrame
                        pFrm = static_cast<SwCntntFrm*>(pF);
                        while( pF && !pF->IsCellFrm() )
                            pF = pF->GetUpper();
                    }

                    if( aTmpState.bPosCorr )
                    {
                        if( pF && !pF->Frm().IsInside( aPt ))
                            pF = 0;
                    }
                    else if( !pF )
                        pF = pFrm;

                    if( pF ) // only then it is valid
                    {
                        // create from internal (for CORE) the external
                        // (for UI) formula
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_TABLEBOXFML;
#ifdef DBG_UTIL
                        if( RES_BOXATR_VALUE == pItem->Which() )
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_TABLEBOXVALUE;
                        else
#endif
                            const_cast<SwTblBoxFormula*>(static_cast<const SwTblBoxFormula*>(pItem))->PtrToBoxNm( &pTblNd->GetTable() );

                        bRet = true;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            SwCrsrSaveState aSaveState( *m_pCurCrsr );
                            *m_pCurCrsr->GetPoint() = aPos;
                            if( m_pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = false;
                            else
                                UpdateCrsr();
                        }

                        if( bRet )
                        {
                            if( pFldRect )
                            {
                                *pFldRect = pF->Prt();
                                *pFldRect += pF->Frm().Pos();
                            }
                            rCntntAtPos.pFndTxtAttr = 0;
                            rCntntAtPos.aFnd.pAttr = pItem;
                        }
                    }
                }
            }

#ifdef DBG_UTIL
            if( !bRet && SwContentAtPos::SW_CURR_ATTRS & rCntntAtPos.eCntntAtPos )
            {
                const sal_Int32 n = aPos.nContent.GetIndex();
                SfxItemSet aSet( GetDoc()->GetAttrPool(), POOLATTR_BEGIN,
                    POOLATTR_END - 1 );
                if( pTxtNd->GetpSwpHints() )
                {
                    for( size_t i = 0; i < pTxtNd->GetSwpHints().Count(); ++i )
                    {
                        const SwTxtAttr* pHt = pTxtNd->GetSwpHints()[i];
                        const sal_Int32 nAttrStart = pHt->GetStart();
                        if( nAttrStart > n ) // over the section
                            break;

                        if( 0 != pHt->End() && (
                            ( nAttrStart < n &&
                            ( pHt->DontExpand() ? n < *pHt->End()
                            : n <= *pHt->End() )) ||
                            ( n == nAttrStart &&
                            ( nAttrStart == *pHt->End() || !n ))) )
                        {
                            aSet.Put( pHt->GetAttr() );
                        }
                    }
                    if( pTxtNd->HasSwAttrSet() &&
                        pTxtNd->GetpSwAttrSet()->Count() )
                    {
                        SfxItemSet aFmtSet( pTxtNd->GetSwAttrSet() );
                        // remove all from format set that are also in TextSet
                        aFmtSet.Differentiate( aSet );
                        // now merge all together
                        aSet.Put( aFmtSet );
                    }
                }
                else
                    pTxtNd->SwCntntNode::GetAttr( aSet );

                rCntntAtPos.sStr = "Pos: (";
                rCntntAtPos.sStr += OUString::number( aPos.nNode.GetIndex());
                rCntntAtPos.sStr += ":";
                rCntntAtPos.sStr += OUString::number( aPos.nContent.GetIndex());
                rCntntAtPos.sStr += ")";
                rCntntAtPos.sStr += "\nParagraph Style: ";
                rCntntAtPos.sStr += pTxtNd->GetFmtColl()->GetName();
                if( pTxtNd->GetCondFmtColl() )
                {
                    rCntntAtPos.sStr += "\nConditional Style: " + pTxtNd->GetCondFmtColl()->GetName();
                }

                if( aSet.Count() )
                {
                    OUString sAttrs;
                    SfxItemIter aIter( aSet );
                    const SfxPoolItem* pItem = aIter.FirstItem();
                    while( true )
                    {
                        if( !IsInvalidItem( pItem ))
                        {
                            OUString aStr;
                            GetDoc()->GetAttrPool().GetPresentation( *pItem,
                                SFX_MAPUNIT_CM, aStr );
                            if (!sAttrs.isEmpty())
                                sAttrs += ", ";
                            sAttrs += aStr;
                        }
                        if( aIter.IsAtEnd() )
                            break;
                        pItem = aIter.NextItem();
                    }
                    if (!sAttrs.isEmpty())
                    {
                        if( !rCntntAtPos.sStr.isEmpty() )
                            rCntntAtPos.sStr += "\n";
                        rCntntAtPos.sStr += "Attr: " + sAttrs;
                    }
                }
                bRet = true;
                rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_CURR_ATTRS;
            }
#endif
        }
    }

    if( !bRet )
    {
        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_NOTHING;
        rCntntAtPos.aFnd.pFld = 0;
    }
    return bRet;
}

// #i90516#
const SwPostItField* SwCrsrShell::GetPostItFieldAtCursor() const
{
    const SwPostItField* pPostItFld = 0;

    if ( !IsTableMode() )
    {
        const SwPosition* pCursorPos = _GetCrsr()->GetPoint();
        const SwTxtNode* pTxtNd = pCursorPos->nNode.GetNode().GetTxtNode();
        if ( pTxtNd )
        {
            SwTxtAttr* pTxtAttr = pTxtNd->GetFldTxtAttrAt( pCursorPos->nContent.GetIndex() );
            const SwField* pFld = pTxtAttr != NULL ? pTxtAttr->GetFmtFld().GetField() : 0;
            if ( pFld && pFld->Which()== RES_POSTITFLD )
            {
                pPostItFld = static_cast<const SwPostItField*>(pFld);
            }
        }
    }

    return pPostItFld;
}

/// is the node in a protected section?
bool SwContentAtPos::IsInProtectSect() const
{
    const SwTxtNode* pNd = 0;
    if( pFndTxtAttr )
    {
        switch( eCntntAtPos )
        {
        case SW_FIELD:
        case SW_CLICKFIELD:
            pNd = static_txtattr_cast<SwTxtFld const*>(pFndTxtAttr)->GetpTxtNode();
            break;

        case SW_FTN:
            pNd = &static_cast<const SwTxtFtn*>(pFndTxtAttr)->GetTxtNode();
            break;

        case SW_INETATTR:
            pNd = static_txtattr_cast<SwTxtINetFmt const*>(pFndTxtAttr)->GetpTxtNode();
            break;

        default:
            break;
        }
    }

    const SwCntntFrm* pFrm;
    return pNd && ( pNd->IsInProtectSect() ||
                    ( 0 != ( pFrm = pNd->getLayoutFrm( pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), 0, 0, false)) &&
                        pFrm->IsProtected() ));
}

bool SwContentAtPos::IsInRTLText()const
{
    bool bRet = false;
    const SwTxtNode* pNd = 0;
    if (pFndTxtAttr && (eCntntAtPos == SW_FTN))
    {
        const SwTxtFtn* pTxtFtn = static_cast<const SwTxtFtn*>(pFndTxtAttr);
        if(pTxtFtn->GetStartNode())
        {
            SwStartNode* pSttNd = pTxtFtn->GetStartNode()->GetNode().GetStartNode();
            SwPaM aTemp( *pSttNd );
            aTemp.Move(fnMoveForward, fnGoNode);
            SwCntntNode* pCntntNode = aTemp.GetCntntNode();
            if(pCntntNode && pCntntNode->IsTxtNode())
                pNd = pCntntNode->GetTxtNode();
        }
    }
    if(pNd)
    {
        SwIterator<SwTxtFrm,SwTxtNode> aIter(*pNd);
        SwTxtFrm* pTmpFrm = aIter.First();
        while( pTmpFrm )
        {
                if ( !pTmpFrm->IsFollow())
                {
                    bRet = pTmpFrm->IsRightToLeft();
                    break;
                }
            pTmpFrm = aIter.Next();
        }
    }
    return bRet;
}

bool SwCrsrShell::SelectTxt( const sal_Int32 nStart,
                                 const sal_Int32 nEnd )
{
    SET_CURR_SHELL( this );
    bool bRet = false;

    SwCallLink aLk( *this );
    SwCrsrSaveState aSaveState( *m_pCurCrsr );

    SwPosition& rPos = *m_pCurCrsr->GetPoint();
    m_pCurCrsr->DeleteMark();
    rPos.nContent = nStart;
    m_pCurCrsr->SetMark();
    rPos.nContent = nEnd;

    if( !m_pCurCrsr->IsSelOvr() )
    {
        UpdateCrsr();
        bRet = true;
    }

    return bRet;
}

bool SwCrsrShell::SelectTxtAttr( sal_uInt16 nWhich,
                                     bool bExpand,
                                     const SwTxtAttr* pTxtAttr )
{
    SET_CURR_SHELL( this );
    bool bRet = false;

    if( !IsTableMode() )
    {
        if( !pTxtAttr )
        {
            SwPosition& rPos = *m_pCurCrsr->GetPoint();
            SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
            pTxtAttr = (pTxtNd)
                ? pTxtNd->GetTxtAttrAt(rPos.nContent.GetIndex(),
                        static_cast<RES_TXTATR>(nWhich),
                        (bExpand) ? SwTxtNode::EXPAND : SwTxtNode::DEFAULT)
                : 0;
        }

        if( pTxtAttr )
        {
            const sal_Int32* pEnd = pTxtAttr->End();
            bRet = SelectTxt( pTxtAttr->GetStart(), ( pEnd ? *pEnd : pTxtAttr->GetStart() + 1 ) );
        }
    }
    return bRet;
}

bool SwCrsrShell::GotoINetAttr( const SwTxtINetFmt& rAttr )
{
    bool bRet = false;
    if( rAttr.GetpTxtNode() )
    {
        SwCursor* pCrsr = getShellCrsr( true );

        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCrsr );

        pCrsr->GetPoint()->nNode = *rAttr.GetpTxtNode();
        pCrsr->GetPoint()->nContent.Assign( const_cast<SwTxtNode*>(rAttr.GetpTxtNode()),
                                            rAttr.GetStart() );
        bRet = !pCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

const SwFmtINetFmt* SwCrsrShell::FindINetAttr( const OUString& rName ) const
{
    return mpDoc->FindINetAttr( rName );
}

bool SwCrsrShell::GetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode,
                                SwRect& rRect, sal_Int16& rOrient )
{

    SET_CURR_SHELL( this );
    bool bRet = false;

    if (!IsTableMode() && !HasSelection()
        && GetDoc()->GetIDocumentUndoRedo().DoesUndo())
    {
        Point aPt( rPt );
        SwPosition aPos( *m_pCurCrsr->GetPoint() );

        SwFillCrsrPos aFPos( eFillMode );
        SwCrsrMoveState aTmpState( &aFPos );

        if( GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState ) &&
            !aPos.nNode.GetNode().IsProtect())
        {
            // start position in protected section?
            rRect = aFPos.aCrsr;
            rOrient = aFPos.eOrient;
            bRet = true;
        }
    }
    return bRet;
}

bool SwCrsrShell::SetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode )
{
    SET_CURR_SHELL( this );
    bool bRet = false;

    if (!IsTableMode() && !HasSelection()
        && GetDoc()->GetIDocumentUndoRedo().DoesUndo())
    {
        Point aPt( rPt );
        SwPosition aPos( *m_pCurCrsr->GetPoint() );

        SwFillCrsrPos aFPos( eFillMode );
        SwCrsrMoveState aTmpState( &aFPos );

        if( GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState ) )
        {
            SwCallLink aLk( *this ); // watch Crsr-Moves
            StartAction();

            SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
            SwUndoId nUndoId = UNDO_INS_FROM_SHADOWCRSR;
            // If only the paragraph attributes "Adjust" or "LRSpace" are set,
            // then the following should not delete those again.
            if( 0 == aFPos.nParaCnt + aFPos.nColumnCnt &&
                ( FILL_INDENT == aFPos.eMode ||
                  ( text::HoriOrientation::NONE != aFPos.eOrient &&
                    0 == aFPos.nTabCnt + aFPos.nSpaceCnt )) &&
                pCNd && pCNd->Len() )
                nUndoId = UNDO_EMPTY;

            GetDoc()->GetIDocumentUndoRedo().StartUndo( nUndoId, NULL );

            SwTxtFmtColl* pNextFmt = 0;
            SwTxtNode* pTNd = pCNd ? pCNd->GetTxtNode() : NULL;
            if( pTNd )
                pNextFmt = &pTNd->GetTxtColl()->GetNextTxtFmtColl();

            const SwSectionNode* pSectNd = pCNd ? pCNd->FindSectionNode() : NULL;
            if( pSectNd && aFPos.nParaCnt )
            {
                SwNodeIndex aEnd( aPos.nNode, 1 );
                while( aEnd.GetNode().IsEndNode() &&
                        (const SwNode*)&aEnd.GetNode() !=
                        pSectNd->EndOfSectionNode() )
                    ++aEnd;

                if( aEnd.GetNode().IsEndNode() &&
                    pCNd->Len() == aPos.nContent.GetIndex() )
                    aPos.nNode = *pSectNd->EndOfSectionNode();
            }

            for( sal_uInt16 n = 0; n < aFPos.nParaCnt + aFPos.nColumnCnt; ++n )
            {
                GetDoc()->getIDocumentContentOperations().AppendTxtNode( aPos );
                if( !n && pNextFmt )
                {
                    *m_pCurCrsr->GetPoint() = aPos;
                    GetDoc()->SetTxtFmtColl( *m_pCurCrsr, pNextFmt, false );
                }
                if( n < aFPos.nColumnCnt )
                {
                    *m_pCurCrsr->GetPoint() = aPos;
                    GetDoc()->getIDocumentContentOperations().InsertPoolItem( *m_pCurCrsr,
                            SvxFmtBreakItem( SVX_BREAK_COLUMN_BEFORE, RES_BREAK ) );
                }
            }

            *m_pCurCrsr->GetPoint() = aPos;
            switch( aFPos.eMode )
            {
            case FILL_INDENT:
                if( 0 != (pCNd = aPos.nNode.GetNode().GetCntntNode() ))
                {
                    SfxItemSet aSet( GetDoc()->GetAttrPool(),
                                    RES_LR_SPACE, RES_LR_SPACE,
                                    RES_PARATR_ADJUST, RES_PARATR_ADJUST,
                                    0 );
                    SvxLRSpaceItem aLR( static_cast<const SvxLRSpaceItem&>(
                                        pCNd->GetAttr( RES_LR_SPACE ) ) );
                    aLR.SetTxtLeft( aFPos.nTabCnt );
                    aLR.SetTxtFirstLineOfst( 0 );
                    aSet.Put( aLR );

                    const SvxAdjustItem& rAdj = static_cast<const SvxAdjustItem&>(pCNd->
                                        GetAttr( RES_PARATR_ADJUST ));
                    if( SVX_ADJUST_LEFT != rAdj.GetAdjust() )
                        aSet.Put( SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ) );

                    GetDoc()->getIDocumentContentOperations().InsertItemSet( *m_pCurCrsr, aSet );
                }
                else {
                    OSL_ENSURE( false, "No CntntNode" );
                }
                break;

            case FILL_TAB:
            case FILL_SPACE:
                {
                    OUStringBuffer sInsert;
                    if (aFPos.nTabCnt)
                        comphelper::string::padToLength(sInsert, aFPos.nTabCnt, '\t');
                    if (aFPos.nSpaceCnt)
                        comphelper::string::padToLength(sInsert, sInsert.getLength() + aFPos.nSpaceCnt, ' ');
                    if (!sInsert.isEmpty())
                        GetDoc()->getIDocumentContentOperations().InsertString( *m_pCurCrsr, sInsert.makeStringAndClear());
                }
                // no break - still need to set orientation
            case FILL_MARGIN:
                if( text::HoriOrientation::NONE != aFPos.eOrient )
                {
                    SvxAdjustItem aAdj( SVX_ADJUST_LEFT, RES_PARATR_ADJUST );
                    switch( aFPos.eOrient )
                    {
                    case text::HoriOrientation::CENTER:
                        aAdj.SetAdjust( SVX_ADJUST_CENTER );
                        break;
                    case text::HoriOrientation::RIGHT:
                        aAdj.SetAdjust( SVX_ADJUST_RIGHT );
                        break;
                    default:
                        break;
                    }
                    GetDoc()->getIDocumentContentOperations().InsertPoolItem( *m_pCurCrsr, aAdj );
                }
                break;
            }

            GetDoc()->GetIDocumentUndoRedo().EndUndo( nUndoId, NULL );
            EndAction();

            bRet = true;
        }
    }
    return bRet;
}

const SwRangeRedline* SwCrsrShell::SelNextRedline()
{
    const SwRangeRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *m_pCurCrsr );

        pFnd = GetDoc()->getIDocumentRedlineAccess().SelNextRedline( *m_pCurCrsr );
        if( pFnd && !m_pCurCrsr->IsInProtectTable() && !m_pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        else
            pFnd = 0;
    }
    return pFnd;
}

const SwRangeRedline* SwCrsrShell::SelPrevRedline()
{
    const SwRangeRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *m_pCurCrsr );

        pFnd = GetDoc()->getIDocumentRedlineAccess().SelPrevRedline( *m_pCurCrsr );
        if( pFnd && !m_pCurCrsr->IsInProtectTable() && !m_pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        else
            pFnd = 0;
    }
    return pFnd;
}

const SwRangeRedline* SwCrsrShell::_GotoRedline( sal_uInt16 nArrPos, bool bSelect )
{
    const SwRangeRedline* pFnd = 0;
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *m_pCurCrsr );

    pFnd = GetDoc()->getIDocumentRedlineAccess().GetRedlineTbl()[ nArrPos ];
    if( pFnd )
    {
        *m_pCurCrsr->GetPoint() = *pFnd->Start();

        SwCntntNode* pCNd;
        SwNodeIndex* pIdx = &m_pCurCrsr->GetPoint()->nNode;
        if( !pIdx->GetNode().IsCntntNode() &&
            0 != ( pCNd = GetDoc()->GetNodes().GoNextSection( pIdx,
                                    true, IsReadOnlyAvailable() )) )
        {
            if( *pIdx <= pFnd->End()->nNode )
                m_pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
            else
                pFnd = 0;
        }

        if( pFnd && bSelect )
        {
            m_pCurCrsr->SetMark();
            if( nsRedlineType_t::REDLINE_FMTCOLL == pFnd->GetType() )
            {
                pCNd = pIdx->GetNode().GetCntntNode();
                m_pCurCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                m_pCurCrsr->GetMark()->nContent.Assign( pCNd, 0 );
            }
            else
                *m_pCurCrsr->GetPoint() = *pFnd->End();

            pIdx = &m_pCurCrsr->GetPoint()->nNode;
            if( !pIdx->GetNode().IsCntntNode() &&
                0 != ( pCNd = SwNodes::GoPrevSection( pIdx,
                                            true, IsReadOnlyAvailable() )) )
            {
                if( *pIdx >= m_pCurCrsr->GetMark()->nNode )
                    m_pCurCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                else
                    pFnd = 0;
            }
        }

        if( !pFnd )
        {
            m_pCurCrsr->DeleteMark();
            m_pCurCrsr->RestoreSavePos();
        }
        else if( bSelect && *m_pCurCrsr->GetMark() == *m_pCurCrsr->GetPoint() )
            m_pCurCrsr->DeleteMark();

        if( pFnd && !m_pCurCrsr->IsInProtectTable() && !m_pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE
                        | SwCrsrShell::READONLY );
        else
        {
            pFnd = 0;
            if( bSelect )
                m_pCurCrsr->DeleteMark();
        }
    }
    return pFnd;
}

const SwRangeRedline* SwCrsrShell::GotoRedline( sal_uInt16 nArrPos, bool bSelect )
{
    const SwRangeRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );

        const SwRedlineTbl& rTbl = GetDoc()->getIDocumentRedlineAccess().GetRedlineTbl();
        const SwRangeRedline* pTmp = rTbl[ nArrPos ];
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();
        if( nSeqNo && bSelect )
        {
            bool bCheck = false;
            int nLoopCnt = 2;
            sal_uInt16 nArrSavPos = nArrPos;

            do {
                pTmp = _GotoRedline( nArrPos, true );

                if( !pFnd )
                    pFnd = pTmp;

                if( pTmp && bCheck )
                {
                    // Check for overlaps. These can happen when FmtColl-
                    // Redlines were stretched over a whole paragraph
                    SwPaM* pCur = m_pCurCrsr;
                    SwPaM* pNextPam = pCur->GetNext();
                    SwPosition* pCStt = pCur->Start(), *pCEnd = pCur->End();
                    while( pCur != pNextPam )
                    {
                        const SwPosition *pNStt = pNextPam->Start(),
                                         *pNEnd = pNextPam->End();

                        bool bDel = true;
                        switch( ::ComparePosition( *pCStt, *pCEnd,
                                                   *pNStt, *pNEnd ))
                        {
                        case POS_INSIDE:         // Pos1 is completely in Pos2
                            if( !pCur->HasMark() )
                            {
                                pCur->SetMark();
                                *pCur->GetMark() = *pNStt;
                            }
                            else
                                *pCStt = *pNStt;
                            *pCEnd = *pNEnd;
                            break;

                        case POS_OUTSIDE:        // Pos2 is completely in Pos1
                        case POS_EQUAL:          // Pos1 has same size as Pos2
                            break;

                        case POS_OVERLAP_BEFORE: // Pos1 overlaps Pos2 at beginning
                            if( !pCur->HasMark() )
                                pCur->SetMark();
                            *pCEnd = *pNEnd;
                            break;
                        case POS_OVERLAP_BEHIND: // Pos1 overlaps Pos2 at end
                            if( !pCur->HasMark() )
                            {
                                pCur->SetMark();
                                *pCur->GetMark() = *pNStt;
                            }
                            else
                                *pCStt = *pNStt;
                            break;

                        default:
                            bDel = false;
                        }

                        if( bDel )
                        {
                            // not needed anymore
                            SwPaM* pPrevPam = static_cast<SwPaM*>(pNextPam->GetPrev());
                            delete pNextPam;
                            pNextPam = pPrevPam;
                        }
                        pNextPam = pNextPam->GetNext();
                    }
                }

                sal_uInt16 nFndPos = 2 == nLoopCnt
                                    ? rTbl.FindNextOfSeqNo( nArrPos )
                                    : rTbl.FindPrevOfSeqNo( nArrPos );
                if( USHRT_MAX != nFndPos ||
                    ( 0 != ( --nLoopCnt ) && USHRT_MAX != (
                            nFndPos = rTbl.FindPrevOfSeqNo( nArrSavPos ))) )
                {
                    if( pTmp )
                    {
                        // create new cursor
                        CreateCrsr();
                        bCheck = true;
                    }
                    nArrPos = nFndPos;
                }
                else
                    nLoopCnt = 0;

            } while( nLoopCnt );
        }
        else
            pFnd = _GotoRedline( nArrPos, bSelect );
    }
    return pFnd;
}

bool SwCrsrShell::SelectNxtPrvHyperlink( bool bNext )
{
    SwNodes& rNds = GetDoc()->GetNodes();
    const SwNode* pBodyEndNd = &rNds.GetEndOfContent();
    const SwNode* pBodySttNd = pBodyEndNd->StartOfSectionNode();
    sal_uLong nBodySttNdIdx = pBodySttNd->GetIndex();
    Point aPt;

    _SetGetExpFld aCmpPos( SwPosition( bNext ? *pBodyEndNd : *pBodySttNd ) );
    _SetGetExpFld aCurPos( bNext ? *m_pCurCrsr->End() : *m_pCurCrsr->Start() );
    if( aCurPos.GetNode() < nBodySttNdIdx )
    {
        const SwCntntNode* pCNd = aCurPos.GetNodeFromCntnt()->GetCntntNode();
        SwCntntFrm* pFrm;
        if( pCNd && 0 != ( pFrm = pCNd->getLayoutFrm( GetLayout(), &aPt )) )
            aCurPos.SetBodyPos( *pFrm );
    }

    // check first all the hyperlink fields
    {
        const SwTxtNode* pTxtNd;
        const SwCharFmts* pFmts = GetDoc()->GetCharFmts();
        for( SwCharFmts::size_type n = pFmts->size(); 1 < n; )
        {
            SwIterator<SwTxtINetFmt,SwCharFmt> aIter(*(*pFmts)[--n]);

            for( SwTxtINetFmt* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
                if( 0 != ( pTxtNd = pFnd->GetpTxtNode()) &&
                    pTxtNd->GetNodes().IsDocNodes() )
                {
                    SwTxtINetFmt& rAttr = *pFnd;
                    SwPosition aTmpPos( *pTxtNd );
                    _SetGetExpFld aPos( aTmpPos.nNode, rAttr );
                    SwCntntFrm* pFrm;
                    if( pTxtNd->GetIndex() < nBodySttNdIdx &&
                        0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt )) )
                        aPos.SetBodyPos( *pFrm );

                    if( bNext
                        ? ( aPos < aCmpPos && aCurPos < aPos )
                        : ( aCmpPos < aPos && aPos < aCurPos ))
                    {
                        OUString sTxt( pTxtNd->GetExpandTxt( rAttr.GetStart(),
                                        *rAttr.GetEnd() - rAttr.GetStart() ) );

                        sTxt = comphelper::string::remove(sTxt, 0x0a);
                        sTxt = comphelper::string::strip(sTxt, ' ');

                        if( !sTxt.isEmpty() )
                            aCmpPos = aPos;
                    }
                }
        }
    }

    // then check all the Flys with a URL or imapge map
    {
        const SwFrmFmts* pFmts = GetDoc()->GetSpzFrmFmts();
        for( SwFrmFmts::size_type n = 0, nEnd = pFmts->size(); n < nEnd; ++n )
        {
            SwFlyFrmFmt* pFmt = static_cast<SwFlyFrmFmt*>((*pFmts)[ n ]);
            const SwFmtURL& rURLItem = pFmt->GetURL();
            if( rURLItem.GetMap() || !rURLItem.GetURL().isEmpty() )
            {
                SwFlyFrm* pFly = pFmt->GetFrm( &aPt, false );
                SwPosition aTmpPos( *pBodySttNd );
                if( pFly &&
                    GetBodyTxtNode( *GetDoc(), aTmpPos, *pFly->GetLower() ) )
                {
                    _SetGetExpFld aPos( *pFmt, &aTmpPos );

                    if( bNext
                            ? ( aPos < aCmpPos && aCurPos < aPos )
                            : ( aCmpPos < aPos && aPos < aCurPos ))
                        aCmpPos = aPos;
                }
            }
        }
    }

    // found any URL ?
    bool bRet = false;
    const SwTxtINetFmt* pFndAttr = aCmpPos.GetINetFmt();
    const SwFlyFrmFmt* pFndFmt = aCmpPos.GetFlyFmt();
    if( pFndAttr || pFndFmt )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );

        // found a text attribute ?
        if( pFndAttr )
        {
            SwCrsrSaveState aSaveState( *m_pCurCrsr );

            aCmpPos.GetPosOfContent( *m_pCurCrsr->GetPoint() );
            m_pCurCrsr->DeleteMark();
            m_pCurCrsr->SetMark();
            m_pCurCrsr->GetPoint()->nContent = *pFndAttr->End();

            if( !m_pCurCrsr->IsInProtectTable() && !m_pCurCrsr->IsSelOvr() )
            {
                UpdateCrsr( SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|
                                    SwCrsrShell::READONLY );
                bRet = true;
            }
        }
        // found a draw object ?
        else if( RES_DRAWFRMFMT == pFndFmt->Which() )
        {
            const SdrObject* pSObj = pFndFmt->FindSdrObject();
            if (pSObj)
            {
                static_cast<SwFEShell*>(this)->SelectObj( pSObj->GetCurrentBoundRect().Center() );
                MakeSelVisible();
                bRet = true;
            }
        }
        else // then is it a fly
        {
            SwFlyFrm* pFly = pFndFmt->GetFrm(&aPt, false );
            if( pFly )
            {
                static_cast<SwFEShell*>(this)->SelectFlyFrm( *pFly, true );
                MakeSelVisible();
                bRet = true;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
