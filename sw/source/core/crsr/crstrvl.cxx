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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/brkitem.hxx>
#include <svx/svdobj.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
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
#include <switerator.hxx>
#include <vcl/window.hxx>
#include <docufld.hxx>

using namespace ::com::sun::star;

/// go to next/previous point on the same level
sal_Bool SwCrsrShell::GotoNextNum()
{
    sal_Bool bRet = GetDoc()->GotoNextNum( *pCurCrsr->GetPoint() );
    if( bRet )
    {
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCurCrsr );
        if( !ActionPend() )
        {
            SET_CURR_SHELL( this );
            // try to set cursor onto this position, at half of the char-
            // SRectangle's height
            Point aPt( pCurCrsr->GetPtPos() );
            SwCntntFrm * pFrm = pCurCrsr->GetCntntNode()->getLayoutFrm( GetLayout(), &aPt,
                                                        pCurCrsr->GetPoint() );
            pFrm->GetCharRect( aCharRect, *pCurCrsr->GetPoint() );
            pFrm->Calc();
            if( pFrm->IsVertical() )
            {
                aPt.X() = aCharRect.Center().X();
                aPt.Y() = pFrm->Frm().Top() + nUpDownX;
            }
            else
            {
                aPt.Y() = aCharRect.Center().Y();
                aPt.X() = pFrm->Frm().Left() + nUpDownX;
            }
            pFrm->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );
            bRet = !pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                        nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
            if( bRet )
                UpdateCrsr(SwCrsrShell::UPDOWN |
                        SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
        }
    }
    return bRet;
}


sal_Bool SwCrsrShell::GotoPrevNum()
{
    sal_Bool bRet = GetDoc()->GotoPrevNum( *pCurCrsr->GetPoint() );
    if( bRet )
    {
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCurCrsr );
        if( !ActionPend() )
        {
            SET_CURR_SHELL( this );
            // try to set cursor onto this position, at half of the char-
            // SRectangle's height
            Point aPt( pCurCrsr->GetPtPos() );
            SwCntntFrm * pFrm = pCurCrsr->GetCntntNode()->getLayoutFrm( GetLayout(), &aPt,
                                                        pCurCrsr->GetPoint() );
            pFrm->GetCharRect( aCharRect, *pCurCrsr->GetPoint() );
            pFrm->Calc();
            if( pFrm->IsVertical() )
            {
                aPt.X() = aCharRect.Center().X();
                aPt.Y() = pFrm->Frm().Top() + nUpDownX;
            }
            else
            {
                aPt.Y() = aCharRect.Center().Y();
                aPt.X() = pFrm->Frm().Left() + nUpDownX;
            }
            pFrm->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );
            bRet = !pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                        nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
            if( bRet )
                UpdateCrsr(SwCrsrShell::UPDOWN |
                        SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
        }
    }
    return bRet;
}

/// jump from content to header
sal_Bool SwCrsrShell::GotoHeaderTxt()
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
sal_Bool SwCrsrShell::GotoFooterTxt()
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

sal_Bool SwCrsrShell::SetCrsrInHdFt( sal_uInt16 nDescNo, sal_Bool bInHeader )
{
    sal_Bool bRet = sal_False;
    SwDoc *pMyDoc = GetDoc();

    SET_CURR_SHELL( this );

    if( USHRT_MAX == nDescNo )
    {
        // take the current one
        const SwPageFrm* pPage = GetCurrFrm()->FindPageFrm();
        if( pPage )
            for( sal_uInt16 i = 0; i < pMyDoc->GetPageDescCnt(); ++i )
                if( pPage->GetPageDesc() == &pMyDoc->GetPageDesc( i ) )
                {
                    nDescNo = i;
                    break;
                }
    }

    if( USHRT_MAX != nDescNo && nDescNo < pMyDoc->GetPageDescCnt() )
    {
        // check if the attribute exists
        const SwPageDesc& rDesc = const_cast<const SwDoc *>(pMyDoc)
            ->GetPageDesc( nDescNo );
        const SwFmtCntnt* pCnt = 0;
        if( bInHeader )
        {
            // mirrored pages? ignore for now
            const SwFmtHeader& rHd = rDesc.GetMaster().GetHeader();
            if( rHd.GetHeaderFmt() )
                pCnt = &rHd.GetHeaderFmt()->GetCntnt();
        }
        else
        {
            const SwFmtFooter& rFt = rDesc.GetMaster().GetFooter();
            if( rFt.GetFooterFmt() )
                pCnt = &rFt.GetFooterFmt()->GetCntnt();
        }

        if( pCnt && pCnt->GetCntntIdx() )
        {
            SwNodeIndex aIdx( *pCnt->GetCntntIdx(), 1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = pMyDoc->GetNodes().GoNext( &aIdx );

            const SwFrm* pFrm;
            Point aPt( pCurCrsr->GetPtPos() );

            if( pCNd && 0 != ( pFrm = pCNd->getLayoutFrm( GetLayout(), &aPt, 0, sal_False ) ))
            {
                // then we can set the cursor in here
                SwCallLink aLk( *this ); // watch Crsr-Moves
                SwCrsrSaveState aSaveState( *pCurCrsr );

                ClearMark();

                SwPosition& rPos = *pCurCrsr->GetPoint();
                rPos.nNode = *pCNd;
                rPos.nContent.Assign( pCNd, 0 );

                bRet = !pCurCrsr->IsSelOvr();
                if( bRet )
                    UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                                SwCrsrShell::READONLY );
            }
        }
    }
    return bRet;
}

/// jump to the next index
sal_Bool SwCrsrShell::GotoNextTOXBase( const String* pName )
{
    sal_Bool bRet = sal_False;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    SwCntntNode* pFnd = 0;
    for( sal_uInt16 n = rFmts.size(); n; )
    {
        const SwSection* pSect = rFmts[ --n ]->GetSection();
        const SwSectionNode* pSectNd;
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            0 != ( pSectNd = pSect->GetFmt()->GetSectionNode() ) &&
             pCurCrsr->GetPoint()->nNode < pSectNd->GetIndex() &&
            ( !pFnd || pFnd->GetIndex() > pSectNd->GetIndex() ) &&
            ( !pName || *pName == ((SwTOXBaseSection*)pSect)->GetTOXName() )
            )
        {
            SwNodeIndex aIdx( *pSectNd, 1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
            const SwCntntFrm* pCFrm;
            if( pCNd &&
                pCNd->EndOfSectionIndex() <= pSectNd->EndOfSectionIndex() &&
                0 != ( pCFrm = pCNd->getLayoutFrm( GetLayout() ) ) &&
                ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
            {
                pFnd = pCNd;
            }
        }
    }
    if( pFnd )
    {
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCurCrsr );
        pCurCrsr->GetPoint()->nNode = *pFnd;
        pCurCrsr->GetPoint()->nContent.Assign( pFnd, 0 );
        bRet = !pCurCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

/// jump to previous index
sal_Bool SwCrsrShell::GotoPrevTOXBase( const String* pName )
{
    sal_Bool bRet = sal_False;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    SwCntntNode* pFnd = 0;
    for( sal_uInt16 n = rFmts.size(); n; )
    {
        const SwSection* pSect = rFmts[ --n ]->GetSection();
        const SwSectionNode* pSectNd;
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            0 != ( pSectNd = pSect->GetFmt()->GetSectionNode() ) &&
            pCurCrsr->GetPoint()->nNode > pSectNd->EndOfSectionIndex() &&
            ( !pFnd || pFnd->GetIndex() < pSectNd->GetIndex() ) &&
            ( !pName || *pName == ((SwTOXBaseSection*)pSect)->GetTOXName() )
            )
        {
            SwNodeIndex aIdx( *pSectNd, 1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
            const SwCntntFrm* pCFrm;
            if( pCNd &&
                pCNd->EndOfSectionIndex() <= pSectNd->EndOfSectionIndex() &&
                0 != ( pCFrm = pCNd->getLayoutFrm( GetLayout() ) ) &&
                ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
            {
                pFnd = pCNd;
            }
        }
    }

    if( pFnd )
    {
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCurCrsr );
        pCurCrsr->GetPoint()->nNode = *pFnd;
        pCurCrsr->GetPoint()->nContent.Assign( pFnd, 0 );
        bRet = !pCurCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

/// jump to index of TOXMark
sal_Bool SwCrsrShell::GotoTOXMarkBase()
{
    sal_Bool bRet = sal_False;

    SwTOXMarks aMarks;
    sal_uInt16 nCnt = GetDoc()->GetCurTOXMark( *pCurCrsr->GetPoint(), aMarks );
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
                0 != ( pSectFmt = ((SwTOXBaseSection*)pTOX)->GetFmt() ) &&
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
                    SwCrsrSaveState aSaveState( *pCurCrsr );
                    pCurCrsr->GetPoint()->nNode = *pCNd;
                    pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
                    bRet = !pCurCrsr->IsInProtectTable() &&
                            !pCurCrsr->IsSelOvr();
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
sal_Bool SwCrsrShell::GotoNxtPrvTblFormula( sal_Bool bNext, sal_Bool bOnlyErrors )
{
    if( IsTableMode() )
        return sal_False;

    sal_Bool bFnd = sal_False;
    SwPosition& rPos = *pCurCrsr->GetPoint();

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
                                &aPt, &rPos, sal_False ) );
    {
        const SfxPoolItem* pItem;
        const SwTableBox* pTBox;
        sal_uInt32 n, nMaxItems = GetDoc()->GetAttrPool().GetItemCount2( RES_BOXATR_FORMULA );

        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = GetDoc()->GetAttrPool().GetItem2(
                                        RES_BOXATR_FORMULA, n ) ) &&
                0 != (pTBox = ((SwTblBoxFormula*)pItem)->GetTableBox() ) &&
                pTBox->GetSttNd() &&
                pTBox->GetSttNd()->GetNodes().IsDocNodes() &&
                ( !bOnlyErrors ||
                  !((SwTblBoxFormula*)pItem)->HasValidBoxes() ) )
            {
                const SwCntntFrm* pCFrm;
                SwNodeIndex aIdx( *pTBox->GetSttNd() );
                const SwCntntNode* pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                if( pCNd && 0 != ( pCFrm = pCNd->getLayoutFrm( GetLayout(), &aPt, 0, sal_False ) ) &&
                    (IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
                {
                    _SetGetExpFld aCmp( *pTBox );
                    aCmp.SetBodyPos( *pCFrm );

                    if( bNext ? ( aCurGEF < aCmp && aCmp < aFndGEF )
                              : ( aCmp < aCurGEF && aFndGEF < aCmp ))
                    {
                        aFndGEF = aCmp;
                        bFnd = sal_True;
                    }
                }
            }
    }

    if( bFnd )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCurCrsr );

        aFndGEF.GetPosOfContent( rPos );
        pCurCrsr->DeleteMark();

        bFnd = !pCurCrsr->IsSelOvr();
        if( bFnd )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
    }
    return bFnd;
}

/// jump to next/previous index marker
sal_Bool SwCrsrShell::GotoNxtPrvTOXMark( sal_Bool bNext )
{
    if( IsTableMode() )
        return sal_False;

    sal_Bool bFnd = sal_False;
    SwPosition& rPos = *pCurCrsr->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.nNode = 0;
    _SetGetExpFld aFndGEF( aFndPos ), aCurGEF( rPos );

    if( rPos.nNode.GetIndex() < GetDoc()->GetNodes().GetEndOfExtras().GetIndex() )
        // also at collection use only the first frame
        aCurGEF.SetBodyPos( *rPos.nNode.GetNode().
                        GetCntntNode()->getLayoutFrm( GetLayout(), &aPt, &rPos, sal_False ) );
    {
        const SfxPoolItem* pItem;
        const SwCntntFrm* pCFrm;
        const SwTxtNode* pTxtNd;
        const SwTxtTOXMark* pTxtTOX;
        sal_uInt32 n, nMaxItems = GetDoc()->GetAttrPool().GetItemCount2( RES_TXTATR_TOXMARK );

        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = GetDoc()->GetAttrPool().GetItem2(
                                        RES_TXTATR_TOXMARK, n ) ) &&
                0 != (pTxtTOX = ((SwTOXMark*)pItem)->GetTxtTOXMark() ) &&
                ( pTxtNd = &pTxtTOX->GetTxtNode())->GetNodes().IsDocNodes() &&
                0 != ( pCFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt, 0, sal_False )) &&
                ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
            {
                SwNodeIndex aNdIndex( *pTxtNd ); // UNIX needs this object
                _SetGetExpFld aCmp( aNdIndex, *pTxtTOX, 0 );
                aCmp.SetBodyPos( *pCFrm );

                if( bNext ? ( aCurGEF < aCmp && aCmp < aFndGEF )
                          : ( aCmp < aCurGEF && aFndGEF < aCmp ))
                {
                    aFndGEF = aCmp;
                    bFnd = sal_True;
                }
            }
    }

    if( bFnd )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCurCrsr );

        aFndGEF.GetPosOfContent( rPos );

        bFnd = !pCurCrsr->IsSelOvr();
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
    SwCrsrSaveState aSaveState( *pCurCrsr );

    const SwTOXMark& rNewMark = GetDoc()->GotoTOXMark( rStart, eDir,
                                                    IsReadOnlyAvailable() );
    // set position
    SwPosition& rPos = *GetCrsr()->GetPoint();
    rPos.nNode = rNewMark.GetTxtTOXMark()->GetTxtNode();
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(),
                         *rNewMark.GetTxtTOXMark()->GetStart() );

    if( !pCurCrsr->IsSelOvr() )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );

    return rNewMark;
}

/// jump to next/previous field type
static void lcl_MakeFldLst( _SetGetExpFlds& rLst, const SwFieldType& rFldType,
                        sal_uInt16 nSubType, sal_Bool bInReadOnly,
                        sal_Bool bChkInpFlag = sal_False )
{
    // always search the 1. frame
    Point aPt;
    SwTxtFld* pTxtFld;
    SwIterator<SwFmtFld,SwFieldType> aIter(rFldType);
    bool bSubType = nSubType != USHRT_MAX;
    for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
        if( 0 != ( pTxtFld = pFmtFld->GetTxtFld() ) &&
            ( !bChkInpFlag || ((SwSetExpField*)pTxtFld->GetFld().GetFld())
                                ->GetInputFlag() ) &&
            (!bSubType || (pFmtFld->GetFld()->GetSubType()
                                & 0xff ) == nSubType ))
        {
            SwCntntFrm* pCFrm;
            const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
            if( 0 != ( pCFrm = rTxtNode.getLayoutFrm( rTxtNode.GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False )) &&
                ( bInReadOnly || !pCFrm->IsProtected() ))
            {
                _SetGetExpFld* pNew = new _SetGetExpFld(
                                SwNodeIndex( rTxtNode ), pTxtFld );
                pNew->SetBodyPos( *pCFrm );
                rLst.insert( pNew );
            }
        }
}


sal_Bool SwCrsrShell::MoveFldType( const SwFieldType* pFldType, sal_Bool bNext,
                                            sal_uInt16 nSubType, sal_uInt16 nResType )
{
    // sorted list of all fields
    _SetGetExpFlds aSrtLst;

    if (pFldType)
    {
        if( RES_INPUTFLD != pFldType->Which() && !pFldType->GetDepends() )
            return sal_False;

        // found Modify object, add all fields to array
        ::lcl_MakeFldLst( aSrtLst, *pFldType, nSubType, IsReadOnlyAvailable() );

        if( RES_INPUTFLD == pFldType->Which() )
        {
            // there are hidden input fields in the set exp. fields
            const SwFldTypes& rFldTypes = *pDoc->GetFldTypes();
            const sal_uInt16 nSize = rFldTypes.size();

            // iterate over all types
            for( sal_uInt16 i=0; i < nSize; ++i )
                if( RES_SETEXPFLD == ( pFldType = rFldTypes[ i ] )->Which() )
                    ::lcl_MakeFldLst( aSrtLst, *pFldType, nSubType,
                                IsReadOnlyAvailable(), sal_True );
        }
    }
    else
    {
        const SwFldTypes& rFldTypes = *pDoc->GetFldTypes();
        const sal_uInt16 nSize = rFldTypes.size();

        // iterate over all types
        for( sal_uInt16 i=0; i < nSize; ++i )
            if( nResType == ( pFldType = rFldTypes[ i ] )->Which() )
                ::lcl_MakeFldLst( aSrtLst, *pFldType, nSubType,
                                IsReadOnlyAvailable() );
    }

    // found no fields?
    if( aSrtLst.empty() )
        return sal_False;

    _SetGetExpFlds::const_iterator it;
    SwCursor* pCrsr = getShellCrsr( true );
    {
        // (1998): Always use field for search so that the right one is found as
        // well some are in frames that are anchored to a paragraph that has a
        // field
        const SwPosition& rPos = *pCrsr->GetPoint();

        SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
        OSL_ENSURE( pTNd, "No CntntNode" );

        SwTxtFld * pTxtFld = static_cast<SwTxtFld *>(
            pTNd->GetTxtAttrForCharAt(rPos.nContent.GetIndex(),
                RES_TXTATR_FIELD));
        bool bDelFld = 0 == pTxtFld;
        if( bDelFld )
        {
            SwFmtFld* pFmtFld = new SwFmtFld( SwDateTimeField(
                (SwDateTimeFieldType*)pDoc->GetSysFldType( RES_DATETIMEFLD ) ) );

            pTxtFld = new SwTxtFld( *pFmtFld, rPos.nContent.GetIndex(),
                        pDoc->IsClipBoard() );
            pTxtFld->ChgTxtNode( pTNd );
        }

        _SetGetExpFld aSrch( rPos.nNode, pTxtFld, &rPos.nContent );
        if( rPos.nNode.GetIndex() < pDoc->GetNodes().GetEndOfExtras().GetIndex() )
        {
            // also at collection use only the first frame
            Point aPt;
            aSrch.SetBodyPos( *pTNd->getLayoutFrm( GetLayout(), &aPt, &rPos, sal_False ) );
        }

        it = aSrtLst.lower_bound( &aSrch );
        if( bDelFld )
        {
            delete (SwFmtFld*)&pTxtFld->GetAttr();
            delete pTxtFld;
        }

        if( it != aSrtLst.end() && **it == aSrch ) // found
        {
            if( bNext )
            {
                if( ++it == aSrtLst.end() )
                    return sal_False; // already at the end
            }
            else
            {
                if( it == aSrtLst.begin() )
                    return sal_False; // no more steps backward possible
                --it;
            }
        }
        else // not found
        {
            if( bNext )
            {
                if( it == aSrtLst.end() )
                    return sal_False;
            }
            else
            {
                if( it == aSrtLst.begin() )
                    return sal_False; // no more steps backward possible
                --it;
            }
        }
    }
    const _SetGetExpFld& rFnd = **it;


    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCrsr );

    rFnd.GetPosOfContent( *pCrsr->GetPoint() );
    sal_Bool bRet = !pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                     nsSwCursorSelOverFlags::SELOVER_TOGGLE );
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}


sal_Bool SwCrsrShell::GotoFld( const SwFmtFld& rFld )
{
    sal_Bool bRet = sal_False;
    if( rFld.GetTxtFld() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves

        SwCursor* pCrsr = getShellCrsr( true );
        SwCrsrSaveState aSaveState( *pCrsr );

        SwTxtNode* pTNd = (SwTxtNode*)rFld.GetTxtFld()->GetpTxtNode();
        pCrsr->GetPoint()->nNode = *pTNd;
        pCrsr->GetPoint()->nContent.Assign( pTNd, *rFld.GetTxtFld()->GetStart() );

        bRet = !pCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}


void SwCrsrShell::GotoOutline( sal_uInt16 nIdx )
{
    SwCursor* pCrsr = getShellCrsr( true );

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCrsr );

    const SwNodes& rNds = GetDoc()->GetNodes();
    SwTxtNode* pTxtNd = (SwTxtNode*)rNds.GetOutLineNds()[ nIdx ]->GetTxtNode();
    pCrsr->GetPoint()->nNode = *pTxtNd;
    pCrsr->GetPoint()->nContent.Assign( pTxtNd, 0 );

    if( !pCrsr->IsSelOvr() )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
}


sal_Bool SwCrsrShell::GotoOutline( const String& rName )
{
    SwCursor* pCrsr = getShellCrsr( true );

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCrsr );

    sal_Bool bRet = sal_False;
    if( pDoc->GotoOutline( *pCrsr->GetPoint(), rName ) && !pCrsr->IsSelOvr() )
    {
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        bRet = sal_True;
    }
    return bRet;
}

/// jump to next node with outline num.
sal_Bool SwCrsrShell::GotoNextOutline()
{
    SwCursor* pCrsr = getShellCrsr( true );
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = pCrsr->GetNode();
    sal_uInt16 nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        ++nPos;

    if( nPos == rNds.GetOutLineNds().size() )
        return sal_False;

    pNd = rNds.GetOutLineNds()[ nPos ];

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCrsr );
    pCrsr->GetPoint()->nNode = *pNd;
    pCrsr->GetPoint()->nContent.Assign( (SwTxtNode*)pNd, 0 );

    sal_Bool bRet = !pCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}

/// jump to previous node with outline num.
sal_Bool SwCrsrShell::GotoPrevOutline()
{
    SwCursor* pCrsr = getShellCrsr( true );
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = pCrsr->GetNode();
    sal_uInt16 nPos;
    rNds.GetOutLineNds().Seek_Entry( pNd, &nPos );

    sal_Bool bRet = sal_False;
    if( nPos )
    {
        --nPos; // before

        pNd = rNds.GetOutLineNds()[ nPos ];
        if( pNd->GetIndex() > pCrsr->GetPoint()->nNode.GetIndex() )
            return sal_False;

        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCrsr );
        pCrsr->GetPoint()->nNode = *pNd;
        pCrsr->GetPoint()->nContent.Assign( (SwTxtNode*)pNd, 0 );

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

    SwNode* pNd = pCrsr->GetNode();
    sal_uInt16 nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        nPos++; // is at correct position; take next for while

    while( nPos-- ) // check the one in front of the current
    {
        pNd = rNds.GetOutLineNds()[ nPos ];

        if( ((SwTxtNode*)pNd)->GetAttrOutlineLevel()-1 <= nLevel )
            return nPos;

    }
    return USHRT_MAX; // no more left
}


sal_Bool SwCrsrShell::MakeOutlineSel( sal_uInt16 nSttPos, sal_uInt16 nEndPos,
                                    sal_Bool bWithChildren )
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    if( rOutlNds.empty() )
        return sal_False;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves

    if( nSttPos > nEndPos ) // parameters switched?
    {
        OSL_ENSURE( !this, "Start > End for array access" );
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

    SwCrsrSaveState aSaveState( *pCurCrsr );

    // set end to the end of the previous content node
    pCurCrsr->GetPoint()->nNode = *pSttNd;
    pCurCrsr->GetPoint()->nContent.Assign( pSttNd->GetCntntNode(), 0 );
    pCurCrsr->SetMark();
    pCurCrsr->GetPoint()->nNode = *pEndNd;
    pCurCrsr->Move( fnMoveBackward, fnGoNode ); // end of predecessor

    // and everything is already selected
    sal_Bool bRet = !pCurCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}


/// jump to reference marker
sal_Bool SwCrsrShell::GotoRefMark( const String& rRefMark, sal_uInt16 nSubType,
                                    sal_uInt16 nSeqNo )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCurCrsr );

    sal_uInt16 nPos;
    SwTxtNode* pTxtNd = SwGetRefFieldType::FindAnchor( GetDoc(), rRefMark,
                                                    nSubType, nSeqNo, &nPos );
    if( pTxtNd && pTxtNd->GetNodes().IsDocNodes() )
    {
        pCurCrsr->GetPoint()->nNode = *pTxtNd;
        pCurCrsr->GetPoint()->nContent.Assign( pTxtNd, nPos );

        if( !pCurCrsr->IsSelOvr() )
        {
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool SwCrsrShell::IsPageAtPos( const Point &rPt ) const
{
    if( GetLayout() )
        return 0 != GetLayout()->GetPageAtPos( rPt );
    return sal_False;
}

sal_Bool SwCrsrShell::GetContentAtPos( const Point& rPt,
                                   SwContentAtPos& rCntntAtPos,
                                   sal_Bool bSetCrsr,
                                   SwRect* pFldRect )
{
    SET_CURR_SHELL( this );
    sal_Bool bRet = sal_False;

    if( !IsTableMode() )
    {
        Point aPt( rPt );
        SwPosition aPos( *pCurCrsr->GetPoint() );

        SwTxtNode* pTxtNd;
        SwCntntFrm *pFrm(0);
        SwTxtAttr* pTxtAttr;
        SwCrsrMoveState aTmpState;
        aTmpState.bFieldInfo = sal_True;
        aTmpState.bExactOnly = !( SwContentAtPos::SW_OUTLINE & rCntntAtPos.eCntntAtPos );
        aTmpState.bCntntCheck = (SwContentAtPos::SW_CONTENT_CHECK & rCntntAtPos.eCntntAtPos) ?  sal_True : sal_False;
        aTmpState.bSetInReadOnly = IsReadOnlyAvailable();

        SwSpecialPos aSpecialPos;
        aTmpState.pSpecialPos = ( SwContentAtPos::SW_SMARTTAG & rCntntAtPos.eCntntAtPos ) ?
                                &aSpecialPos : 0;

        const sal_Bool bCrsrFoundExact = GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState );
        pTxtNd = aPos.nNode.GetNode().GetTxtNode();

        const SwNodes& rNds = GetDoc()->GetNodes();
        if( pTxtNd && SwContentAtPos::SW_OUTLINE & rCntntAtPos.eCntntAtPos
            && !rNds.GetOutLineNds().empty() )
        {
            const SwTxtNode* pONd = pTxtNd->FindOutlineNodeOfLevel( MAXLEVEL-1);
            if( pONd )
            {
                rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_OUTLINE;
                rCntntAtPos.sStr = pONd->GetExpandTxt( 0, STRING_LEN, true, true );
                bRet = sal_True;
            }
        }
        // #i43742# New function
        else if ( SwContentAtPos::SW_CONTENT_CHECK & rCntntAtPos.eCntntAtPos &&
                  bCrsrFoundExact )
        {
            bRet = sal_True;
        }
        // #i23726#
        else if( pTxtNd &&
                 SwContentAtPos::SW_NUMLABEL & rCntntAtPos.eCntntAtPos)
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
                if( !bRet && SwContentAtPos::SW_SMARTTAG & rCntntAtPos.eCntntAtPos
                    && !aTmpState.bFtnNoInfo )
                {
                    const SwWrongList* pSmartTagList = pTxtNd->GetSmartTags();
                    xub_StrLen nCurrent = aPos.nContent.GetIndex();
                    xub_StrLen nBegin = nCurrent;
                    xub_StrLen nLen = 1;

                    if ( pSmartTagList && pSmartTagList->InWrongWord( nCurrent, nLen ) && !pTxtNd->IsSymbol(nBegin) )
                    {
                        const sal_uInt16 nIndex = pSmartTagList->GetWrongPos( nBegin );
                        const SwWrongList* pSubList = pSmartTagList->SubList( nIndex );
                        if ( pSubList )
                        {
                            nCurrent = aTmpState.pSpecialPos->nCharOfst;

                            if ( pSubList->InWrongWord( nCurrent, nLen ) )
                                bRet = sal_True;
                        }
                        else
                            bRet = sal_True;

                        if( bRet && bSetCrsr )
                        {
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            pCurCrsr->DeleteMark();
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                                    nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = sal_False;
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

                if( !bRet && ( SwContentAtPos::SW_FIELD | SwContentAtPos::SW_CLICKFIELD )
                    & rCntntAtPos.eCntntAtPos && !aTmpState.bFtnNoInfo )
                {
                    pTxtAttr = pTxtNd->GetTxtAttrForCharAt(
                            aPos.nContent.GetIndex(), RES_TXTATR_FIELD );
                    const SwField* pFld = pTxtAttr
                                            ? pTxtAttr->GetFld().GetFld()
                                            : 0;
                    if( SwContentAtPos::SW_CLICKFIELD & rCntntAtPos.eCntntAtPos &&
                        pFld && !pFld->HasClickHdl() )
                        pFld = 0;

                    if( pFld )
                    {
                        if( pFldRect && 0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt ) ) )
                            pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );

                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            pCurCrsr->DeleteMark();
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr() )
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
                                ((SwTblField*)pFld)->IsIntrnlName() )
                        {
                            // create from internal (for CORE) the external
                            // (for UI) formula
                            const SwTableNode* pTblNd = pTxtNd->FindTableNode();
                            if( pTblNd )        // steht in einer Tabelle
                                ((SwTblField*)pFld)->PtrToBoxNm( &pTblNd->GetTable() );
                        }
                    }

                    if( pFld )
                    {
                        rCntntAtPos.aFnd.pFld = pFld;
                        rCntntAtPos.pFndTxtAttr = pTxtAttr;
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FIELD;
                        bRet = sal_True;
                    }
                }

        if( !bRet && SwContentAtPos::SW_FORMCTRL & rCntntAtPos.eCntntAtPos )
        {
            IDocumentMarkAccess* pMarksAccess = GetDoc()->getIDocumentMarkAccess( );
            sw::mark::IFieldmark* pFldBookmark = pMarksAccess->getFieldmarkFor( aPos );
            if( bCrsrFoundExact && pTxtNd && pFldBookmark) {
                rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FORMCTRL;
                rCntntAtPos.aFnd.pFldmark = pFldBookmark;
                bRet=sal_True;
            }
        }

                if( !bRet && SwContentAtPos::SW_FTN & rCntntAtPos.eCntntAtPos )
                {
                    if( aTmpState.bFtnNoInfo )
                    {
                        // over the footnote's char
                        bRet = sal_True;
                        if( bSetCrsr )
                        {
                            *pCurCrsr->GetPoint() = aPos;
                            if( !GotoFtnAnchor() )
                                bRet = sal_False;
                        }
                        if( bRet )
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FTN;
                    }
                    else if ( 0 != ( pTxtAttr = pTxtNd->GetTxtAttrForCharAt(
                                aPos.nContent.GetIndex(), RES_TXTATR_FTN )) )
                    {
                        bRet = sal_True;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            pCurCrsr->GetPoint()->nNode = *((SwTxtFtn*)pTxtAttr)->GetStartNode();
                            SwCntntNode* pCNd = GetDoc()->GetNodes().GoNextSection(
                                            &pCurCrsr->GetPoint()->nNode,
                                            sal_True, !IsReadOnlyAvailable() );

                            if( pCNd )
                            {
                                pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
                                if( pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                                        nsSwCursorSelOverFlags::SELOVER_TOGGLE ))
                                    bRet = sal_False;
                                else
                                    UpdateCrsr();
                            }
                            else
                                bRet = sal_False;
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

                if( !bRet && ( SwContentAtPos::SW_TOXMARK |
                               SwContentAtPos::SW_REFMARK ) &
                        rCntntAtPos.eCntntAtPos && !aTmpState.bFtnNoInfo )
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
                        bRet = sal_True;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            pCurCrsr->DeleteMark();
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                                    nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
                                bRet = sal_False;
                            else
                                UpdateCrsr();
                        }

                        if( bRet )
                        {
                            const xub_StrLen* pEnd = pTxtAttr->GetEnd();
                            if( pEnd )
                                rCntntAtPos.sStr = pTxtNd->GetExpandTxt(
                                            *pTxtAttr->GetStart(),
                                            *pEnd - *pTxtAttr->GetStart() );
                            else if( RES_TXTATR_TOXMARK == pTxtAttr->Which())
                                rCntntAtPos.sStr = pTxtAttr->GetTOXMark().
                                                    GetAlternativeText();

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

                if( !bRet && SwContentAtPos::SW_INETATTR & rCntntAtPos.eCntntAtPos
                    && !aTmpState.bFtnNoInfo )
                {
                    pTxtAttr = pTxtNd->GetTxtAttrAt(
                            aPos.nContent.GetIndex(), RES_TXTATR_INETFMT);
                    // "detect" only INetAttrs with URLs
                    if( pTxtAttr && pTxtAttr->GetINetFmt().GetValue().Len() )
                    {
                        bRet = sal_True;
                        if( bSetCrsr )
                        {
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            pCurCrsr->DeleteMark();
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                                    nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = sal_False;
                            else
                                UpdateCrsr();
                        }
                        if( bRet )
                        {
                            rCntntAtPos.sStr = pTxtNd->GetExpandTxt(
                                        *pTxtAttr->GetStart(),
                                        *pTxtAttr->GetEnd() - *pTxtAttr->GetStart() );

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
                    const SwRedline* pRedl = GetDoc()->GetRedline(aPos, NULL);
                    if( pRedl )
                    {
                        rCntntAtPos.aFnd.pRedl = pRedl;
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_REDLINE;
                        rCntntAtPos.pFndTxtAttr = 0;
                        bRet = sal_True;

                        if( pFldRect && 0 != ( pFrm = pTxtNd->getLayoutFrm( GetLayout(), &aPt ) ) )
                            pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                    }
                }
            }

            if( !bRet && (
                SwContentAtPos::SW_TABLEBOXFML & rCntntAtPos.eCntntAtPos
#ifdef DBG_UTIL
                || SwContentAtPos::SW_TABLEBOXVALUE & rCntntAtPos.eCntntAtPos
#endif
                ))
            {
                const SwTableNode* pTblNd;
                const SwTableBox* pBox;
                const SwStartNode* pSttNd = pTxtNd->FindTableBoxStartNode();
                const SfxPoolItem* pItem;
                if( pSttNd && 0 != ( pTblNd = pTxtNd->FindTableNode()) &&
                    0 != ( pBox = pTblNd->GetTable().GetTblBox(
                                    pSttNd->GetIndex() )) &&
#ifdef DBG_UTIL
                    ( SFX_ITEM_SET == pBox->GetFrmFmt()->GetItemState(
                        RES_BOXATR_FORMULA, sal_False, &pItem ) ||
                      SFX_ITEM_SET == pBox->GetFrmFmt()->GetItemState(
                        RES_BOXATR_VALUE, sal_False, &pItem ))
#else
                    SFX_ITEM_SET == pBox->GetFrmFmt()->GetItemState(
                        RES_BOXATR_FORMULA, sal_False, &pItem )
#endif
                    )
                {
                    SwFrm* pF = pTxtNd->getLayoutFrm( GetLayout(), &aPt );
                    if( pF )
                    {
                        // then the CellFrame
                        pFrm = (SwCntntFrm*)pF;
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
                        ((SwTblBoxFormula*)pItem)->PtrToBoxNm( &pTblNd->GetTable() );

                        bRet = sal_True;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this ); // watch Crsr-Moves
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                                    nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = sal_False;
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
                xub_StrLen n = aPos.nContent.GetIndex();
                SfxItemSet aSet( GetDoc()->GetAttrPool(), POOLATTR_BEGIN,
                                                          POOLATTR_END - 1 );
                if( pTxtNd->GetpSwpHints() )
                {
                    for( sal_uInt16 i = 0; i < pTxtNd->GetSwpHints().Count(); ++i )
                    {
                        const SwTxtAttr* pHt = pTxtNd->GetSwpHints()[i];
                        xub_StrLen nAttrStart = *pHt->GetStart();
                        if( nAttrStart > n ) // over the section
                            break;

                        if( 0 != pHt->GetEnd() && (
                            ( nAttrStart < n &&
                                ( pHt->DontExpand() ? n < *pHt->GetEnd()
                                                    : n <= *pHt->GetEnd() )) ||
                            ( n == nAttrStart &&
                                ( nAttrStart == *pHt->GetEnd() || !n ))) )
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

                rCntntAtPos.sStr.AssignAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "Pos: (" ));
                rCntntAtPos.sStr += String::CreateFromInt32( aPos.nNode.GetIndex());
                rCntntAtPos.sStr += ':';
                rCntntAtPos.sStr += String::CreateFromInt32( aPos.nContent.GetIndex());
                rCntntAtPos.sStr += ')';
                rCntntAtPos.sStr.AppendAscii(
                                RTL_CONSTASCII_STRINGPARAM( "\nAbs.Vorl.: " )); // translation *might be* "paragraph template"
                rCntntAtPos.sStr += pTxtNd->GetFmtColl()->GetName();
                if( pTxtNd->GetCondFmtColl() )
                    rCntntAtPos.sStr.AppendAscii(
                                RTL_CONSTASCII_STRINGPARAM( "\nBed.Vorl.: " )) // translation *might be* "conditional template"
                        += pTxtNd->GetCondFmtColl()->GetName();

                if( aSet.Count() )
                {
                    String sAttrs;
                    SfxItemIter aIter( aSet );
                    const SfxPoolItem* pItem = aIter.FirstItem();
                    while( sal_True )
                    {
                        if( !IsInvalidItem( pItem ))
                        {
                            String aStr;
                            GetDoc()->GetAttrPool().GetPresentation( *pItem,
                                            SFX_ITEM_PRESENTATION_COMPLETE,
                                            SFX_MAPUNIT_CM, aStr );
                            if( sAttrs.Len() )
                                sAttrs.AppendAscii(
                                        RTL_CONSTASCII_STRINGPARAM( ", " ));
                            sAttrs += aStr;
                        }
                        if( aIter.IsAtEnd() )
                            break;
                        pItem = aIter.NextItem();
                    }
                    if( sAttrs.Len() )
                    {
                        if( rCntntAtPos.sStr.Len() )
                            rCntntAtPos.sStr += '\n';
                        rCntntAtPos.sStr.AppendAscii(
                                RTL_CONSTASCII_STRINGPARAM( "Attr: " ) )
                                += sAttrs;
                    }
                }
                bRet = sal_True;
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
            SwTxtAttr* pTxtAttr = pTxtNd->GetTxtAttrForCharAt(
                    pCursorPos->nContent.GetIndex(), RES_TXTATR_FIELD );
            const SwField* pFld = pTxtAttr ? pTxtAttr->GetFld().GetFld() : 0;
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
            pNd = ((SwTxtFld*)pFndTxtAttr)->GetpTxtNode();
            break;

        case SW_FTN:
            pNd = &((SwTxtFtn*)pFndTxtAttr)->GetTxtNode();
            break;

        case SW_INETATTR:
            pNd = ((SwTxtINetFmt*)pFndTxtAttr)->GetpTxtNode();
            break;

        default:
            break;
        }
    }

    const SwCntntFrm* pFrm;
    return pNd && ( pNd->IsInProtectSect() ||
                    ( 0 != ( pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), 0,0,sal_False)) &&
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
                pNd = static_cast<SwTxtNode*>(pCntntNode);
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

sal_Bool SwCrsrShell::SelectTxtAttr( sal_uInt16 nWhich, sal_Bool bExpand,
                                    const SwTxtAttr* pTxtAttr )
{
    SET_CURR_SHELL( this );
    sal_Bool bRet = sal_False;

    if( !IsTableMode() )
    {
        SwPosition& rPos = *pCurCrsr->GetPoint();
        if( !pTxtAttr )
        {
            SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
            pTxtAttr = (pTxtNd)
                ? pTxtNd->GetTxtAttrAt(rPos.nContent.GetIndex(),
                        static_cast<RES_TXTATR>(nWhich),
                        (bExpand) ? SwTxtNode::EXPAND : SwTxtNode::DEFAULT)
                : 0;
        }

        if( pTxtAttr )
        {
            SwCallLink aLk( *this ); // watch Crsr-Moves
            SwCrsrSaveState aSaveState( *pCurCrsr );

            pCurCrsr->DeleteMark();
            rPos.nContent = *pTxtAttr->GetStart();
            pCurCrsr->SetMark();
            const xub_StrLen* pEnd = pTxtAttr->GetEnd();
            rPos.nContent = pEnd ? *pEnd : *pTxtAttr->GetStart() + 1;

            if( !pCurCrsr->IsSelOvr() )
            {
                UpdateCrsr();
                bRet = sal_True;
            }
        }
    }
    return bRet;
}


sal_Bool SwCrsrShell::GotoINetAttr( const SwTxtINetFmt& rAttr )
{
    sal_Bool bRet = sal_False;
    if( rAttr.GetpTxtNode() )
    {
        SwCursor* pCrsr = getShellCrsr( true );

        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCrsr );

        pCrsr->GetPoint()->nNode = *rAttr.GetpTxtNode();
        pCrsr->GetPoint()->nContent.Assign( (SwTxtNode*)rAttr.GetpTxtNode(),
                                            *rAttr.GetStart() );
        bRet = !pCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}


const SwFmtINetFmt* SwCrsrShell::FindINetAttr( const String& rName ) const
{
    return pDoc->FindINetAttr( rName );
}

sal_Bool SwCrsrShell::GetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode,
                                SwRect& rRect, sal_Int16& rOrient )
{

    SET_CURR_SHELL( this );
    sal_Bool bRet = sal_False;

    if (!IsTableMode() && !HasSelection()
        && GetDoc()->GetIDocumentUndoRedo().DoesUndo())
    {
        Point aPt( rPt );
        SwPosition aPos( *pCurCrsr->GetPoint() );

        SwFillCrsrPos aFPos( eFillMode );
        SwCrsrMoveState aTmpState( &aFPos );

        if( GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState ) &&
            !aPos.nNode.GetNode().IsProtect())
        {
            // start position in protected section?
            rRect = aFPos.aCrsr;
            rOrient = aFPos.eOrient;
            bRet = sal_True;
        }
    }
    return bRet;
}

sal_Bool SwCrsrShell::SetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode )
{
    SET_CURR_SHELL( this );
    sal_Bool bRet = sal_False;

    if (!IsTableMode() && !HasSelection()
        && GetDoc()->GetIDocumentUndoRedo().DoesUndo())
    {
        Point aPt( rPt );
        SwPosition aPos( *pCurCrsr->GetPoint() );

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
            SwTxtNode* pTNd = pCNd->GetTxtNode();
            if( pTNd )
                pNextFmt = &pTNd->GetTxtColl()->GetNextTxtFmtColl();

            const SwSectionNode* pSectNd = pCNd->FindSectionNode();
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
                GetDoc()->AppendTxtNode( aPos );
                if( !n && pNextFmt )
                {
                    *pCurCrsr->GetPoint() = aPos;
                    GetDoc()->SetTxtFmtColl( *pCurCrsr, pNextFmt, false );
                }
                if( n < aFPos.nColumnCnt )
                {
                    *pCurCrsr->GetPoint() = aPos;
                    GetDoc()->InsertPoolItem( *pCurCrsr,
                            SvxFmtBreakItem( SVX_BREAK_COLUMN_BEFORE, RES_BREAK ), 0);
                }
            }

            *pCurCrsr->GetPoint() = aPos;
            switch( aFPos.eMode )
            {
            case FILL_INDENT:
                if( 0 != (pCNd = aPos.nNode.GetNode().GetCntntNode() ))
                {
                    SfxItemSet aSet( GetDoc()->GetAttrPool(),
                                    RES_LR_SPACE, RES_LR_SPACE,
                                    RES_PARATR_ADJUST, RES_PARATR_ADJUST,
                                    0 );
                    SvxLRSpaceItem aLR( (SvxLRSpaceItem&)
                                        pCNd->GetAttr( RES_LR_SPACE ) );
                    aLR.SetTxtLeft( aFPos.nTabCnt );
                    aLR.SetTxtFirstLineOfst( 0 );
                    aSet.Put( aLR );

                    const SvxAdjustItem& rAdj = (SvxAdjustItem&)pCNd->
                                        GetAttr( RES_PARATR_ADJUST );
                    if( SVX_ADJUST_LEFT != rAdj.GetAdjust() )
                        aSet.Put( SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ) );

                    GetDoc()->InsertItemSet( *pCurCrsr, aSet, 0 );
                }
                else {
                    OSL_ENSURE( !this, "No CntntNode" );
                }
                break;

            case FILL_TAB:
            case FILL_SPACE:
                {
                    String sInsert;
                    if( aFPos.nTabCnt )
                        sInsert.Fill( aFPos.nTabCnt, '\t' );
                    if( aFPos.nSpaceCnt )
                    {
                        String sSpace;
                        sSpace.Fill( aFPos.nSpaceCnt );
                        sInsert += sSpace;
                    }
                    if( sInsert.Len() )
                    {
                        GetDoc()->InsertString( *pCurCrsr, sInsert );
                    }
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
                    GetDoc()->InsertPoolItem( *pCurCrsr, aAdj, 0 );
                }
                break;
            }

            GetDoc()->GetIDocumentUndoRedo().EndUndo( nUndoId, NULL );
            EndAction();

            bRet = sal_True;
        }
    }
    return bRet;
}

const SwRedline* SwCrsrShell::SelNextRedline()
{
    const SwRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCurCrsr );

        pFnd = GetDoc()->SelNextRedline( *pCurCrsr );
        if( pFnd && !pCurCrsr->IsInProtectTable() && !pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        else
            pFnd = 0;
    }
    return pFnd;
}

const SwRedline* SwCrsrShell::SelPrevRedline()
{
    const SwRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Crsr-Moves
        SwCrsrSaveState aSaveState( *pCurCrsr );

        pFnd = GetDoc()->SelPrevRedline( *pCurCrsr );
        if( pFnd && !pCurCrsr->IsInProtectTable() && !pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        else
            pFnd = 0;
    }
    return pFnd;
}

const SwRedline* SwCrsrShell::_GotoRedline( sal_uInt16 nArrPos, sal_Bool bSelect )
{
    const SwRedline* pFnd = 0;
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *pCurCrsr );

    pFnd = GetDoc()->GetRedlineTbl()[ nArrPos ];
    if( pFnd )
    {
        *pCurCrsr->GetPoint() = *pFnd->Start();

        SwCntntNode* pCNd;
        SwNodeIndex* pIdx = &pCurCrsr->GetPoint()->nNode;
        if( !pIdx->GetNode().IsCntntNode() &&
            0 != ( pCNd = GetDoc()->GetNodes().GoNextSection( pIdx,
                                    sal_True, IsReadOnlyAvailable() )) )
        {
            if( *pIdx <= pFnd->End()->nNode )
                pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
            else
                pFnd = 0;
        }

        if( pFnd && bSelect )
        {
            pCurCrsr->SetMark();
            if( nsRedlineType_t::REDLINE_FMTCOLL == pFnd->GetType() )
            {
                pCNd = pIdx->GetNode().GetCntntNode();
                pCurCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                pCurCrsr->GetMark()->nContent.Assign( pCNd, 0 );
            }
            else
                *pCurCrsr->GetPoint() = *pFnd->End();

            pIdx = &pCurCrsr->GetPoint()->nNode;
            if( !pIdx->GetNode().IsCntntNode() &&
                0 != ( pCNd = GetDoc()->GetNodes().GoPrevSection( pIdx,
                                            sal_True, IsReadOnlyAvailable() )) )
            {
                if( *pIdx >= pCurCrsr->GetMark()->nNode )
                    pCurCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                else
                    pFnd = 0;
            }
        }

        if( !pFnd )
        {
            pCurCrsr->DeleteMark();
            pCurCrsr->RestoreSavePos();
        }
        else if( bSelect && *pCurCrsr->GetMark() == *pCurCrsr->GetPoint() )
            pCurCrsr->DeleteMark();

        if( pFnd && !pCurCrsr->IsInProtectTable() && !pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE
                        | SwCrsrShell::READONLY );
        else
        {
            pFnd = 0;
            if( bSelect )
                pCurCrsr->DeleteMark();
        }
    }
    return pFnd;
}

const SwRedline* SwCrsrShell::GotoRedline( sal_uInt16 nArrPos, sal_Bool bSelect )
{
    const SwRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );

        const SwRedlineTbl& rTbl = GetDoc()->GetRedlineTbl();
        const SwRedline* pTmp = rTbl[ nArrPos ];
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();
        if( nSeqNo && bSelect )
        {
            bool bCheck = false;
            int nLoopCnt = 2;
            sal_uInt16 nArrSavPos = nArrPos;

            do {
                pTmp = _GotoRedline( nArrPos, sal_True );

                if( !pFnd )
                    pFnd = pTmp;

                if( pTmp && bCheck )
                {
                    // Check for overlaps. These can happen when FmtColl-
                    // Redlines were streched over a whole paragraph
                    SwPaM* pCur = pCurCrsr;
                    SwPaM* pNextPam = (SwPaM*)pCur->GetNext();
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
                            SwPaM* pPrevPam = (SwPaM*)pNextPam->GetPrev();
                            delete pNextPam;
                            pNextPam = pPrevPam;
                        }
                        pNextPam = (SwPaM*)pNextPam->GetNext();
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


sal_Bool SwCrsrShell::SelectNxtPrvHyperlink( sal_Bool bNext )
{
    SwNodes& rNds = GetDoc()->GetNodes();
    const SwNode* pBodyEndNd = &rNds.GetEndOfContent();
    const SwNode* pBodySttNd = pBodyEndNd->StartOfSectionNode();
    sal_uLong nBodySttNdIdx = pBodySttNd->GetIndex();
    Point aPt;

    _SetGetExpFld aCmpPos( SwPosition( bNext ? *pBodyEndNd : *pBodySttNd ) );
    _SetGetExpFld aCurPos( bNext ? *pCurCrsr->End() : *pCurCrsr->Start() );
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
        for( sal_uInt16 n = pFmts->size(); 1 < n; )
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
                        String sTxt( pTxtNd->GetExpandTxt( *rAttr.GetStart(),
                                        *rAttr.GetEnd() - *rAttr.GetStart() ) );

                        sTxt = comphelper::string::remove(sTxt, 0x0a);
                        sTxt = comphelper::string::strip(sTxt, ' ');

                        if( sTxt.Len() )
                            aCmpPos = aPos;
                    }
                }
        }
    }
    // then check all the Flys with a URL or imapge map
    {
        const SwFrmFmts* pFmts = GetDoc()->GetSpzFrmFmts();
        for( sal_uInt16 n = 0, nEnd = pFmts->size(); n < nEnd; ++n )
        {
            SwFlyFrmFmt* pFmt = (SwFlyFrmFmt*)(*pFmts)[ n ];
            const SwFmtURL& rURLItem = pFmt->GetURL();
            if( rURLItem.GetMap() || rURLItem.GetURL().Len() )
            {
                SwFlyFrm* pFly = pFmt->GetFrm( &aPt, sal_False );
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
    sal_Bool bRet = sal_False;
    const SwTxtINetFmt* pFndAttr = aCmpPos.GetINetFmt();
    const SwFlyFrmFmt* pFndFmt = aCmpPos.GetFlyFmt();
    if( pFndAttr || pFndFmt )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );

        // found a text attribute ?
        if( pFndAttr )
        {
            SwCrsrSaveState aSaveState( *pCurCrsr );

            aCmpPos.GetPosOfContent( *pCurCrsr->GetPoint() );
            pCurCrsr->DeleteMark();
            pCurCrsr->SetMark();
            pCurCrsr->GetPoint()->nContent = *pFndAttr->SwTxtAttr::GetEnd();

            if( !pCurCrsr->IsInProtectTable() && !pCurCrsr->IsSelOvr() )
            {
                UpdateCrsr( SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|
                                    SwCrsrShell::READONLY );
                bRet = sal_True;
            }
        }
        // found a draw object ?
        else if( RES_DRAWFRMFMT == pFndFmt->Which() )
        {
            const SdrObject* pSObj = pFndFmt->FindSdrObject();
            ((SwFEShell*)this)->SelectObj( pSObj->GetCurrentBoundRect().Center() );
            MakeSelVisible();
            bRet = sal_True;
        }
        else // then is it a fly
        {
            SwFlyFrm* pFly = pFndFmt->GetFrm(&aPt, sal_False );
            if( pFly )
            {
                ((SwFEShell*)this)->SelectFlyFrm( *pFly, sal_True );
                MakeSelVisible();
                bRet = sal_True;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
