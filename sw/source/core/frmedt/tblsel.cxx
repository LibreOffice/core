/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <editeng/boxitem.hxx>
#include <editeng/protitem.hxx>

#include <hintids.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <tblsel.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <ndole.hxx>
#include <swtable.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <viscrs.hxx>
#include <swtblfmt.hxx>
#include <UndoTable.hxx>
#include <mvsave.hxx>
#include <sectfrm.hxx>
#include <frmtool.hxx>
#include <switerator.hxx>
#include <deque>
#include <boost/foreach.hpp>


#define COLFUZZY 20L










#undef      DEL_ONLY_EMPTY_LINES
#undef      DEL_EMPTY_BOXES_AT_START_AND_END

struct _CmpLPt
{
    Point aPos;
    const SwTableBox* pSelBox;
    bool bVert;

    _CmpLPt( const Point& rPt, const SwTableBox* pBox, bool bVertical );

    bool operator==( const _CmpLPt& rCmp ) const
    {   return X() == rCmp.X() && Y() == rCmp.Y(); }

    bool operator<( const _CmpLPt& rCmp ) const
    {
        if ( bVert )
            return X() > rCmp.X() || ( X() == rCmp.X() && Y() < rCmp.Y() );
        else
            return Y() < rCmp.Y() || ( Y() == rCmp.Y() && X() < rCmp.X() );
    }

    long X() const { return aPos.X(); }
    long Y() const { return aPos.Y(); }
};


typedef o3tl::sorted_vector<_CmpLPt> _MergePos;


struct _Sort_CellFrm
{
    const SwCellFrm* pFrm;

    _Sort_CellFrm( const SwCellFrm& rCFrm )
        : pFrm( &rCFrm ) {}
};

typedef std::deque< _Sort_CellFrm > _Sort_CellFrms;

static const SwLayoutFrm *lcl_FindCellFrm( const SwLayoutFrm *pLay )
{
    while ( pLay && !pLay->IsCellFrm() )
        pLay = pLay->GetUpper();
    return pLay;
}

static const SwLayoutFrm *lcl_FindNextCellFrm( const SwLayoutFrm *pLay )
{
    
    const SwLayoutFrm *pTmp = pLay;
    do {
        pTmp = pTmp->GetNextLayoutLeaf();
    } while( pLay->IsAnLower( pTmp ) );

    while( pTmp && !pTmp->IsCellFrm() )
        pTmp = pTmp->GetUpper();
    return pTmp;
}

void GetTblSelCrs( const SwCrsrShell &rShell, SwSelBoxes& rBoxes )
{
    rBoxes.clear();
    if( rShell.IsTableMode() && ((SwCrsrShell&)rShell).UpdateTblSelBoxes())
    {
        rBoxes.insert(rShell.GetTableCrsr()->GetSelectedBoxes());
    }
}

void GetTblSelCrs( const SwTableCursor& rTblCrsr, SwSelBoxes& rBoxes )
{
    rBoxes.clear();

    if (rTblCrsr.IsChgd() || !rTblCrsr.GetSelectedBoxesCount())
    {
        SwTableCursor* pTCrsr = (SwTableCursor*)&rTblCrsr;
        pTCrsr->GetDoc()->GetCurrentLayout()->MakeTblCrsrs( *pTCrsr );
    }

    if (rTblCrsr.GetSelectedBoxesCount())
    {
        rBoxes.insert(rTblCrsr.GetSelectedBoxes());
    }
}

void GetTblSel( const SwCrsrShell& rShell, SwSelBoxes& rBoxes,
                const SwTblSearchType eSearchType )
{
    
    if ( !rShell.IsTableMode() )
        rShell.GetCrsr();

    GetTblSel( *rShell.getShellCrsr(false), rBoxes, eSearchType );
}

void GetTblSel( const SwCursor& rCrsr, SwSelBoxes& rBoxes,
                const SwTblSearchType eSearchType )
{
    
    OSL_ENSURE( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( false ),
            "Tabselection not on Cnt." );

    
    
    
    const SwCntntNode* pContentNd = rCrsr.GetNode()->GetCntntNode();
    const SwTableNode* pTblNd = pContentNd ? pContentNd->FindTableNode() : 0;
    if( pTblNd && pTblNd->GetTable().IsNewModel() )
    {
        SwTable::SearchType eSearch;
        switch( nsSwTblSearchType::TBLSEARCH_COL & eSearchType )
        {
            case nsSwTblSearchType::TBLSEARCH_ROW: eSearch = SwTable::SEARCH_ROW; break;
            case nsSwTblSearchType::TBLSEARCH_COL: eSearch = SwTable::SEARCH_COL; break;
            default: eSearch = SwTable::SEARCH_NONE; break;
        }
        const bool bChkP = 0 != ( nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType );
        pTblNd->GetTable().CreateSelection( rCrsr, rBoxes, eSearch, bChkP );
        return;
    }
    if( nsSwTblSearchType::TBLSEARCH_ROW == ((~nsSwTblSearchType::TBLSEARCH_PROTECT ) & eSearchType ) &&
        pTblNd && !pTblNd->GetTable().IsTblComplex() )
    {
        const SwTable& rTbl = pTblNd->GetTable();
        const SwTableLines& rLines = rTbl.GetTabLines();

        const SwNode* pMarkNode = rCrsr.GetNode( false );
        const sal_uLong nMarkSectionStart = pMarkNode->StartOfSectionIndex();
        const SwTableBox* pMarkBox = rTbl.GetTblBox( nMarkSectionStart );

        OSL_ENSURE( pMarkBox, "Point in table, mark outside?" );

        const SwTableLine* pLine = pMarkBox ? pMarkBox->GetUpper() : 0;
        sal_uInt16 nSttPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nSttPos, "Where is my row in the table?" );
        pLine = rTbl.GetTblBox( rCrsr.GetNode( true )->StartOfSectionIndex() )->GetUpper();
        sal_uInt16 nEndPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nEndPos, "Where is my row in the table?" );
        
        if ( nSttPos != USHRT_MAX && nEndPos != USHRT_MAX )
        {
            if( nEndPos < nSttPos )     
            {
                sal_uInt16 nTmp = nSttPos; nSttPos = nEndPos; nEndPos = nTmp;
            }

            int bChkProtected = nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType;
            for( ; nSttPos <= nEndPos; ++nSttPos )
            {
                pLine = rLines[ nSttPos ];
                for( sal_uInt16 n = pLine->GetTabBoxes().size(); n ; )
                {
                    SwTableBox* pBox = pLine->GetTabBoxes()[ --n ];
                    
                    if( !bChkProtected ||
                        !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                        rBoxes.insert( pBox );
                }
            }
        }
    }
    else
    {
        Point aPtPos, aMkPos;
        const SwShellCrsr* pShCrsr = dynamic_cast<const SwShellCrsr*>(&rCrsr);
        if( pShCrsr )
        {
            aPtPos = pShCrsr->GetPtPos();
            aMkPos = pShCrsr->GetMkPos();
        }
        const SwCntntNode *pCntNd = rCrsr.GetCntntNode();
        const SwLayoutFrm *pStart = pCntNd ?
            pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(), &aPtPos )->GetUpper() : 0;
        pCntNd = rCrsr.GetCntntNode(false);
        const SwLayoutFrm *pEnd = pCntNd ?
            pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(), &aMkPos )->GetUpper() : 0;
        if( pStart && pEnd )
            GetTblSel( pStart, pEnd, rBoxes, 0, eSearchType );
    }
}

void GetTblSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
                SwSelBoxes& rBoxes, SwCellFrms* pCells,
                const SwTblSearchType eSearchType )
{
    const SwTabFrm* pStartTab = pStart->FindTabFrm();
    if ( !pStartTab )
    {
        OSL_FAIL( "GetTblSel without start table" );
        return;
    }

    int bChkProtected = nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType;

    bool bTblIsValid;
    
    int nLoopMax = 10;

    do {
        bTblIsValid = true;

        
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

        Point aCurrentTopLeft( LONG_MAX, LONG_MAX );
        Point aCurrentTopRight( 0, LONG_MAX );
        Point aCurrentBottomLeft( LONG_MAX, 0 );
        Point aCurrentBottomRight( 0, 0 );
        const SwCellFrm* pCurrentTopLeftFrm     = 0;
        const SwCellFrm* pCurrentTopRightFrm    = 0;
        const SwCellFrm* pCurrentBottomLeftFrm  = 0;
        const SwCellFrm* pCurrentBottomRightFrm  = 0;

        
        for (size_t i = 0; i < aUnions.size() && bTblIsValid; ++i)
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();
            if( !pTable->IsValid() && nLoopMax )
            {
                bTblIsValid = false;
                break;
            }

            
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                     (const SwLayoutFrm*)pTable->Lower();

            while( pRow && bTblIsValid )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTblIsValid = false;
                    break;
                }

                if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( bTblIsValid && pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->IsValid() && nLoopMax )
                        {
                            bTblIsValid = false;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                        if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwTableBox* pBox = (SwTableBox*)
                                ((SwCellFrm*)pCell)->GetTabBox();
                            
                            if( !bChkProtected ||
                                !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                                rBoxes.insert( pBox );

                            if ( pCells )
                            {
                                const Point aTopLeft( pCell->Frm().TopLeft() );
                                const Point aTopRight( pCell->Frm().TopRight() );
                                const Point aBottomLeft( pCell->Frm().BottomLeft() );
                                const Point aBottomRight( pCell->Frm().BottomRight() );

                                if ( aTopLeft.getY() < aCurrentTopLeft.getY() ||
                                     ( aTopLeft.getY() == aCurrentTopLeft.getY() &&
                                       aTopLeft.getX() <  aCurrentTopLeft.getX() ) )
                                {
                                    aCurrentTopLeft = aTopLeft;
                                    pCurrentTopLeftFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aTopRight.getY() < aCurrentTopRight.getY() ||
                                     ( aTopRight.getY() == aCurrentTopRight.getY() &&
                                       aTopRight.getX() >  aCurrentTopRight.getX() ) )
                                {
                                    aCurrentTopRight = aTopRight;
                                    pCurrentTopRightFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aBottomLeft.getY() > aCurrentBottomLeft.getY() ||
                                     ( aBottomLeft.getY() == aCurrentBottomLeft.getY() &&
                                       aBottomLeft.getX() <  aCurrentBottomLeft.getX() ) )
                                {
                                    aCurrentBottomLeft = aBottomLeft;
                                    pCurrentBottomLeftFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aBottomRight.getY() > aCurrentBottomRight.getY() ||
                                     ( aBottomRight.getY() == aCurrentBottomRight.getY() &&
                                       aBottomRight.getX() >  aCurrentBottomRight.getX() ) )
                                {
                                    aCurrentBottomRight = aBottomRight;
                                    pCurrentBottomRightFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                            }
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                pRow = (const SwLayoutFrm*)pRow->GetNext();
            }
        }

        if ( pCells )
        {
            pCells->clear();
            pCells->push_back( const_cast< SwCellFrm* >(pCurrentTopLeftFrm) );
            pCells->push_back( const_cast< SwCellFrm* >(pCurrentTopRightFrm) );
            pCells->push_back( const_cast< SwCellFrm* >(pCurrentBottomLeftFrm) );
            pCells->push_back( const_cast< SwCellFrm* >(pCurrentBottomRightFrm) );
        }

        if( bTblIsValid )
            break;

        SwDeletionChecker aDelCheck( pStart );

        
        SwTabFrm *pTable = aUnions.front().GetTable();
        while( pTable )
        {
            if( pTable->IsValid() )
                pTable->InvalidatePos();
            pTable->SetONECalcLowers();
            pTable->Calc();
            pTable->SetCompletePaint();
            if( 0 == (pTable = pTable->GetFollow()) )
                break;
        }

        
        
        if ( aDelCheck.HasBeenDeleted() )
        {
            OSL_FAIL( "Current box has been deleted during GetTblSel()" );
            break;
        }

        rBoxes.clear();
        --nLoopMax;

    } while( true );
    OSL_ENSURE( nLoopMax, "Table layout is still invalid!" );
}



sal_Bool ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd )
{
    const SwTableNode* pTNd = rSttNd.FindTableNode();
    if( !pTNd )
        return sal_False;

    Point aNullPos;
    SwNodeIndex aIdx( rSttNd );
    const SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, false, false );

    
    
    
    
    
    
    if ( !pCNd || pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() ) == NULL )
            return sal_False;

    const SwLayoutFrm *pStart = pCNd ? pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aNullPos )->GetUpper() : 0;
    OSL_ENSURE( pStart, "without frame nothing works" );

    aIdx = rEndNd;
    pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, false, false );

    
    if ( !pCNd || pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() ) == NULL )
    {
        return sal_False;
    }

    const SwLayoutFrm *pEnd = pCNd ? pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aNullPos )->GetUpper() : 0;
    OSL_ENSURE( pEnd, "without frame nothing works" );


    bool bTblIsValid, bValidChartSel;
    
    int nLoopMax = 10;      
    sal_uInt16 i = 0;

    do {
        bTblIsValid = true;
        bValidChartSel = true;

        sal_uInt16 nRowCells = USHRT_MAX;

        
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_NO_UNION_CORRECT );

        
        for( i = 0; i < aUnions.size() && bTblIsValid &&
                                    bValidChartSel; ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            SWRECTFN( pTable )
            sal_Bool bRTL = pTable->IsRightToLeft();

            if( !pTable->IsValid() && nLoopMax  )
            {
                bTblIsValid = false;
                break;
            }

            _Sort_CellFrms aCellFrms;

            
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      (const SwLayoutFrm*)pTable->Lower();

            while( pRow && bTblIsValid && bValidChartSel )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTblIsValid = false;
                    break;
                }

                if( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( bValidChartSel && bTblIsValid && pCell &&
                            pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->IsValid() && nLoopMax  )
                        {
                            bTblIsValid = false;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                        const SwRect& rUnion = pUnion->GetUnion(),
                                    & rFrmRect = pCell->Frm();

                        const long nUnionRight = rUnion.Right();
                        const long nUnionBottom = rUnion.Bottom();
                        const long nFrmRight = rFrmRect.Right();
                        const long nFrmBottom = rFrmRect.Bottom();

                        

                        const long nXFuzzy = bVert ? 0 : 20;
                        const long nYFuzzy = bVert ? 20 : 0;

                        if( !(  rUnion.Top()  + nYFuzzy > nFrmBottom ||
                                nUnionBottom < rFrmRect.Top() + nYFuzzy ||
                                rUnion.Left() + nXFuzzy > nFrmRight ||
                                nUnionRight < rFrmRect.Left() + nXFuzzy ))
                        {
                            

                            
                            
                            if( rUnion.Left()   <= rFrmRect.Left() + nXFuzzy &&
                                rFrmRect.Left() <= nUnionRight &&
                                rUnion.Left()   <= nFrmRight &&
                                nFrmRight       <= nUnionRight + nXFuzzy &&
                                rUnion.Top()    <= rFrmRect.Top() + nYFuzzy &&
                                rFrmRect.Top()  <= nUnionBottom &&
                                rUnion.Top()    <= nFrmBottom &&
                                nFrmBottom      <= nUnionBottom+ nYFuzzy )

                                aCellFrms.push_back(
                                        _Sort_CellFrm( *(SwCellFrm*)pCell) );
                            else
                            {
                                bValidChartSel = false;
                                break;
                            }
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                pRow = (const SwLayoutFrm*)pRow->GetNext();
            }

            if( !bValidChartSel )
                break;

            
            
            size_t n;
            sal_uInt16 nCellCnt = 0;
            long nYPos = LONG_MAX;
            long nXPos = 0;
            long nHeight = 0;

            for( n = 0 ; n < aCellFrms.size(); ++n )
            {
                const _Sort_CellFrm& rCF = aCellFrms[ n ];
                if( (rCF.pFrm->Frm().*fnRect->fnGetTop)() != nYPos )
                {
                    
                    if( n )
                    {
                        if( USHRT_MAX == nRowCells )        
                            nRowCells = nCellCnt;
                        else if( nRowCells != nCellCnt )
                        {
                            bValidChartSel = false;
                            break;
                        }
                    }
                    nCellCnt = 1;
                    nYPos = (rCF.pFrm->Frm().*fnRect->fnGetTop)();
                    nHeight = (rCF.pFrm->Frm().*fnRect->fnGetHeight)();

                    nXPos = bRTL ?
                            (rCF.pFrm->Frm().*fnRect->fnGetLeft)() :
                            (rCF.pFrm->Frm().*fnRect->fnGetRight)();
                }
                else if( nXPos == ( bRTL ?
                                    (rCF.pFrm->Frm().*fnRect->fnGetRight)() :
                                    (rCF.pFrm->Frm().*fnRect->fnGetLeft)() ) &&
                         nHeight == (rCF.pFrm->Frm().*fnRect->fnGetHeight)() )
                {
                    nXPos += ( bRTL ? (-1) : 1 ) *
                             (rCF.pFrm->Frm().*fnRect->fnGetWidth)();
                    ++nCellCnt;
                }
                else
                {
                    bValidChartSel = false;
                    break;
                }
            }
            if( bValidChartSel )
            {
                if( USHRT_MAX == nRowCells )
                    nRowCells = nCellCnt;
                else if( nRowCells != nCellCnt )
                    bValidChartSel = false;
            }
        }

        if( bTblIsValid )
            break;

        
        SwTabFrm *pTable = aUnions.front().GetTable();
        for( i = 0; i < aUnions.size(); ++i )
        {
            if( pTable->IsValid() )
                pTable->InvalidatePos();
            pTable->SetONECalcLowers();
            pTable->Calc();
            pTable->SetCompletePaint();
            if( 0 == (pTable = pTable->GetFollow()) )
                break;
        }
        --nLoopMax;
    } while( true );

    OSL_ENSURE( nLoopMax, "table layout is still invalid!" );

    return bValidChartSel ? sal_True : sal_False;
}


sal_Bool IsFrmInTblSel( const SwRect& rUnion, const SwFrm* pCell )
{
    OSL_ENSURE( pCell->IsCellFrm(), "Frame without Gazelle" );

    if( pCell->FindTabFrm()->IsVertical() )
        return ( rUnion.Right() >= pCell->Frm().Right() &&
                 rUnion.Left() <= pCell->Frm().Left() &&
            (( rUnion.Top() <= pCell->Frm().Top()+20 &&
               rUnion.Bottom() > pCell->Frm().Top() ) ||
             ( rUnion.Top() >= pCell->Frm().Top() &&
               rUnion.Bottom() < pCell->Frm().Bottom() )) ? sal_True : sal_False );

    return (
        rUnion.Top() <= pCell->Frm().Top() &&
        rUnion.Bottom() >= pCell->Frm().Bottom() &&

        (( rUnion.Left() <= pCell->Frm().Left()+20 &&
           rUnion.Right() > pCell->Frm().Left() ) ||

         ( rUnion.Left() >= pCell->Frm().Left() &&
           rUnion.Right() < pCell->Frm().Right() )) ? sal_True : sal_False );
}

sal_Bool GetAutoSumSel( const SwCrsrShell& rShell, SwCellFrms& rBoxes )
{
    SwShellCrsr* pCrsr = rShell.m_pCurCrsr;
    if ( rShell.IsTableMode() )
        pCrsr = rShell.m_pTblCrsr;

    const SwLayoutFrm *pStart = pCrsr->GetCntntNode()->getLayoutFrm( rShell.GetLayout(),
                      &pCrsr->GetPtPos() )->GetUpper(),
                      *pEnd   = pCrsr->GetCntntNode(false)->getLayoutFrm( rShell.GetLayout(),
                      &pCrsr->GetMkPos() )->GetUpper();

    const SwLayoutFrm* pSttCell = pStart;
    while( pSttCell && !pSttCell->IsCellFrm() )
        pSttCell = pSttCell->GetUpper();

    
    SwSelUnions aUnions;

    
    ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_COL );

    bool bTstRow = true, bFound = false;
    sal_uInt16 i;

    
    for( i = 0; i < aUnions.size(); ++i )
    {
        SwSelUnion *pUnion = &aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        
        const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                  pTable->GetFirstNonHeadlineRow() :
                                  (const SwLayoutFrm*)pTable->Lower();

        while( pRow )
        {
            if( pRow->Frm().IsOver( pUnion->GetUnion() ) )
            {
                const SwCellFrm* pUpperCell = 0;
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while( pCell && pRow->IsAnLower( pCell ) )
                {
                    if( pCell == pSttCell )
                    {
                        sal_uInt16 nWhichId = 0;
                        for( size_t n = rBoxes.size(); n; )
                            if( USHRT_MAX != ( nWhichId = rBoxes[ --n ]
                                ->GetTabBox()->IsFormulaOrValueBox() ))
                                break;

                        
                        
                        bTstRow = 0 == nWhichId || USHRT_MAX == nWhichId;
                        bFound = true;
                        break;
                    }

                    OSL_ENSURE( pCell->IsCellFrm(), "Frame without cell" );
                    if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        pUpperCell = (SwCellFrm*)pCell;

                    if( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }

                if( pUpperCell )
                    rBoxes.push_back( const_cast< SwCellFrm* >(pUpperCell) );
            }
            if( bFound )
            {
                i = aUnions.size();
                break;
            }
            pRow = (const SwLayoutFrm*)pRow->GetNext();
        }
    }


    
    if( bTstRow )
    {
        bFound = false;

        rBoxes.clear();
        aUnions.clear();
        ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_ROW );

        for( i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      (const SwLayoutFrm*)pTable->Lower();

            while( pRow )
            {
                if( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( pCell == pSttCell )
                        {
                            sal_uInt16 nWhichId = 0;
                            for( size_t n = rBoxes.size(); n; )
                                if( USHRT_MAX != ( nWhichId = rBoxes[ --n ]
                                    ->GetTabBox()->IsFormulaOrValueBox() ))
                                    break;

                            
                            
                            bFound = 0 != nWhichId && USHRT_MAX != nWhichId;
                            bTstRow = false;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame without cell" );
                        if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwCellFrm* pC = (SwCellFrm*)pCell;
                            rBoxes.push_back( pC );
                        }
                        if( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                if( !bTstRow )
                {
                    i = aUnions.size();
                    break;
                }

                pRow = (const SwLayoutFrm*)pRow->GetNext();
            }
        }
    }

    return bFound ? sal_True : sal_False;
}

sal_Bool HasProtectedCells( const SwSelBoxes& rBoxes )
{
    sal_Bool bRet = sal_False;
    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        if( rBoxes[ n ]->GetFrmFmt()->GetProtect().IsCntntProtected() )
        {
            bRet = sal_True;
            break;
        }
    }
    return bRet;
}


_CmpLPt::_CmpLPt( const Point& rPt, const SwTableBox* pBox, bool bVertical )
    : aPos( rPt ), pSelBox( pBox ), bVert( bVertical )
{}

static void lcl_InsTblBox( SwTableNode* pTblNd, SwDoc* pDoc, SwTableBox* pBox,
                        sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 )
{
    OSL_ENSURE( pBox->GetSttNd(), "Box without Start-Node" );
    SwCntntNode* pCNd = pDoc->GetNodes()[ pBox->GetSttIdx() + 1 ]
                                ->GetCntntNode();
    if( pCNd && pCNd->IsTxtNode() )
        pDoc->GetNodes().InsBoxen( pTblNd, pBox->GetUpper(),
                (SwTableBoxFmt*)pBox->GetFrmFmt(),
                ((SwTxtNode*)pCNd)->GetTxtColl(),
                pCNd->GetpSwAttrSet(),
                nInsPos, nCnt );
    else
        pDoc->GetNodes().InsBoxen( pTblNd, pBox->GetUpper(),
                (SwTableBoxFmt*)pBox->GetFrmFmt(),
                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl(), 0,
                nInsPos, nCnt );
}

sal_Bool IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam )
{
    rPam.GetPoint()->nNode = *rBox.GetSttNd()->EndOfSectionNode();
    rPam.Move( fnMoveBackward, fnGoCntnt );
    rPam.SetMark();
    rPam.GetPoint()->nNode = *rBox.GetSttNd();
    rPam.Move( fnMoveForward, fnGoCntnt );
    sal_Bool bRet = *rPam.GetMark() == *rPam.GetPoint()
        && ( rBox.GetSttNd()->GetIndex() + 1 == rPam.GetPoint()->nNode.GetIndex() );

    if( bRet )
    {
        
        const SwFrmFmts& rFmts = *rPam.GetDoc()->GetSpzFrmFmts();
        sal_uLong nSttIdx = rPam.GetPoint()->nNode.GetIndex(),
              nEndIdx = rBox.GetSttNd()->EndOfSectionIndex(),
              nIdx;

        for( sal_uInt16 n = 0; n < rFmts.size(); ++n )
        {
            const SwFmtAnchor& rAnchor = rFmts[n]->GetAnchor();
            const SwPosition* pAPos = rAnchor.GetCntntAnchor();
            if (pAPos &&
                ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                 (FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
                nSttIdx <= ( nIdx = pAPos->nNode.GetIndex() ) &&
                nIdx < nEndIdx )
            {
                bRet = sal_False;
                break;
            }
        }
    }
    return bRet;
}

void GetMergeSel( const SwPaM& rPam, SwSelBoxes& rBoxes,
                SwTableBox** ppMergeBox, SwUndoTblMerge* pUndo )
{
    rBoxes.clear();

    OSL_ENSURE( rPam.GetCntntNode() && rPam.GetCntntNode( false ),
            "Tabselection not on Cnt." );




    Point aPt( 0, 0 );

    const SwCntntNode* pCntNd = rPam.GetCntntNode();
    const SwLayoutFrm *pStart = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                                        &aPt )->GetUpper();
    pCntNd = rPam.GetCntntNode(false);
    const SwLayoutFrm *pEnd = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                                        &aPt )->GetUpper();

    
    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );
    if( aUnions.empty() )
        return;

    const SwTable *pTable = aUnions.front().GetTable()->GetTable();
    SwDoc* pDoc = (SwDoc*)pStart->GetFmt()->GetDoc();
    SwTableNode* pTblNd = (SwTableNode*)pTable->GetTabSortBoxes()[ 0 ]->
                                        GetSttNd()->FindTableNode();

    _MergePos aPosArr;      
    long nWidth;
    SwTableBox* pLastBox = 0;

    SWRECTFN( pStart->GetUpper() )

    for ( sal_uInt16 i = 0; i < aUnions.size(); ++i )
    {
        const SwTabFrm *pTabFrm = aUnions[i].GetTable();

        SwRect &rUnion = aUnions[i].GetUnion();

        
        const SwLayoutFrm* pRow = pTabFrm->IsFollow() ?
                                  pTabFrm->GetFirstNonHeadlineRow() :
                                  (const SwLayoutFrm*)pTabFrm->Lower();

        while ( pRow )
        {
            if ( pRow->Frm().IsOver( rUnion ) )
            {
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    OSL_ENSURE( pCell->IsCellFrm(), "Frame without cell" );
                    
                    if( rUnion.Top() <= pCell->Frm().Top() &&
                        rUnion.Bottom() >= pCell->Frm().Bottom() )
                    {
                        SwTableBox* pBox =(SwTableBox*)((SwCellFrm*)pCell)->GetTabBox();

                        
                        if( ( rUnion.Left() - COLFUZZY ) <= pCell->Frm().Left() &&
                            ( rUnion.Right() - COLFUZZY ) > pCell->Frm().Left() )
                        {
                            if( ( rUnion.Right() + COLFUZZY ) < pCell->Frm().Right() )
                            {
                                sal_uInt16 nInsPos = pBox->GetUpper()->
                                                    GetTabBoxes().GetPos( pBox )+1;
                                lcl_InsTblBox( pTblNd, pDoc, pBox, nInsPos );
                                pBox->ClaimFrmFmt();
                                SwFmtFrmSize aNew(
                                        pBox->GetFrmFmt()->GetFrmSize() );
                                nWidth = rUnion.Right() - pCell->Frm().Left();
                                nWidth = nWidth * aNew.GetWidth() /
                                         pCell->Frm().Width();
                                long nTmpWidth = aNew.GetWidth() - nWidth;
                                aNew.SetWidth( nWidth );
                                pBox->GetFrmFmt()->SetFmtAttr( aNew );
                                
                                pLastBox = pBox;
                                rBoxes.insert( pBox );
                                aPosArr.insert(
                                    _CmpLPt( (pCell->Frm().*fnRect->fnGetPos)(),
                                    pBox, bVert ) );

                                pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                                aNew.SetWidth( nTmpWidth );
                                pBox->ClaimFrmFmt();
                                pBox->GetFrmFmt()->SetFmtAttr( aNew );

                                if( pUndo )
                                    pUndo->AddNewBox( pBox->GetSttIdx() );
                            }
                            else
                            {
                                
                                pLastBox = pBox;
                                rBoxes.insert( pBox );
#if OSL_DEBUG_LEVEL > 1
                                Point aInsPoint( (pCell->Frm().*fnRect->fnGetPos)() );
#endif
                                aPosArr.insert(
                                    _CmpLPt( (pCell->Frm().*fnRect->fnGetPos)(),
                                    pBox, bVert ) );
                            }
                        }
                        
                        else if( ( rUnion.Left() - COLFUZZY ) >= pCell->Frm().Left() &&
                                ( rUnion.Right() + COLFUZZY ) < pCell->Frm().Right() )
                        {
                            sal_uInt16 nInsPos = pBox->GetUpper()->GetTabBoxes().GetPos(
                                            pBox )+1;
                            lcl_InsTblBox( pTblNd, pDoc, pBox, nInsPos, 2 );
                            pBox->ClaimFrmFmt();
                            SwFmtFrmSize aNew(
                                        pBox->GetFrmFmt()->GetFrmSize() );
                            long nLeft = rUnion.Left() - pCell->Frm().Left();
                            nLeft = nLeft * aNew.GetWidth() /
                                    pCell->Frm().Width();
                            long nRight = pCell->Frm().Right() - rUnion.Right();
                            nRight = nRight * aNew.GetWidth() /
                                     pCell->Frm().Width();
                            nWidth = aNew.GetWidth() - nLeft - nRight;

                            aNew.SetWidth( nLeft );
                            pBox->GetFrmFmt()->SetFmtAttr( aNew );

                            {
                            const SfxPoolItem* pItem;
                            if( SFX_ITEM_SET == pBox->GetFrmFmt()->GetAttrSet()
                                        .GetItemState( RES_BOX, false, &pItem ))
                            {
                                SvxBoxItem aBox( *(SvxBoxItem*)pItem );
                                aBox.SetLine( 0, BOX_LINE_RIGHT );
                                pBox->GetFrmFmt()->SetFmtAttr( aBox );
                            }
                            }

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                            aNew.SetWidth( nWidth );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetFmtAttr( aNew );

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );

                            
                            pLastBox = pBox;
                            rBoxes.insert( pBox );
                            aPosArr.insert(
                                _CmpLPt( (pCell->Frm().*fnRect->fnGetPos)(),
                                pBox, bVert ) );

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos+1 ];
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetFmtAttr( aNew );

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );
                        }
                       
                        else if( ( pCell->Frm().Right() - COLFUZZY ) < rUnion.Right() &&
                                 ( pCell->Frm().Right() - COLFUZZY ) > rUnion.Left() &&
                                 ( pCell->Frm().Left() + COLFUZZY ) < rUnion.Left() )
                        {
                            
                            sal_uInt16 nInsPos = pBox->GetUpper()->GetTabBoxes().GetPos(
                                            pBox )+1;
                            lcl_InsTblBox( pTblNd, pDoc, pBox, nInsPos, 1 );

                            SwFmtFrmSize aNew(pBox->GetFrmFmt()->GetFrmSize() );
                            long nLeft = rUnion.Left() - pCell->Frm().Left(),
                                nRight = pCell->Frm().Right() - rUnion.Left();

                            nLeft = nLeft * aNew.GetWidth() /
                                    pCell->Frm().Width();
                            nRight = nRight * aNew.GetWidth() /
                                    pCell->Frm().Width();

                            aNew.SetWidth( nLeft );
                            pBox->ClaimFrmFmt()->SetFmtAttr( aNew );

                                
                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetFmtAttr( aNew );

                            pLastBox = pBox;
                            rBoxes.insert( pBox );
                            aPosArr.insert( _CmpLPt( Point( rUnion.Left(),
                                                pCell->Frm().Top()), pBox, bVert ));

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );
                        }
                    }
                    if ( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }
            }
            pRow = (const SwLayoutFrm*)pRow->GetNext();
        }
    }

    
    if( 1 >= rBoxes.size() )
        return;

    
    
    

    
    
    
    bool bCalcWidth = true;
    const SwTableBox* pFirstBox = aPosArr[ 0 ].pSelBox;

    
    
    
    {
        SwPaM aPam( pDoc->GetNodes() );

#if defined( DEL_ONLY_EMPTY_LINES )
        nWidth = pFirstBox->GetFrmFmt()->GetFrmSize().GetWidth();
        sal_Bool bEmptyLine = sal_True;
        sal_uInt16 n, nSttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  
            {
                if( bEmptyLine && !IsEmptyBox( *rPt.pSelBox, aPam ))
                    bEmptyLine = sal_False;
                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = false;     

                if( bEmptyLine && nSttPos < n )
                {
                    
                    
                    if( pUndo )
                        for( sal_uInt16 i = nSttPos; i < n; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    aPosArr.Remove( nSttPos, n - nSttPos );
                    n = nSttPos;
                }
                else
                    nSttPos = n;

                bEmptyLine = IsEmptyBox( *aPosArr[n].pSelBox, aPam );
            }
        }
        if( bEmptyLine && nSttPos < n )
        {
            if( pUndo )
                for( sal_uInt16 i = nSttPos; i < n; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );
            aPosArr.Remove( nSttPos, n - nSttPos );
        }
#elsif defined( DEL_EMPTY_BOXES_AT_START_AND_END )

        nWidth = pFirstBox->GetFrmFmt()->GetFrmSize().GetWidth();
        sal_uInt16 n, nSttPos = 0, nSEndPos = 0, nESttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  
            {
                sal_Bool bEmptyBox = IsEmptyBox( *rPt.pSelBox, aPam );
                if( bEmptyBox )
                {
                    if( nSEndPos == n )     
                        nESttPos = ++nSEndPos;
                }
                else                        
                    nESttPos = n+1;

                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = false;     

                
                if( nSttPos < nSEndPos )
                {
                    
                    
                    if( pUndo )
                        for( sal_uInt16 i = nSttPos; i < nSEndPos; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    sal_uInt16 nCnt = nSEndPos - nSttPos;
                    aPosArr.Remove( nSttPos, nCnt );
                    nESttPos -= nCnt;
                    n -= nCnt;
                }

                if( nESttPos < n )
                {
                    
                    
                    if( pUndo )
                        for( sal_uInt16 i = nESttPos; i < n; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    sal_uInt16 nCnt = n - nESttPos;
                    aPosArr.Remove( nESttPos, nCnt );
                    n -= nCnt;
                }

                nSttPos = nSEndPos = nESttPos = n;
                if( IsEmptyBox( *aPosArr[n].pSelBox, aPam ))
                    ++nSEndPos;
                else
                    ++nESttPos;
            }
        }

        
        if( nSttPos < nSEndPos )
        {
            
            
            if( pUndo )
                for( sal_uInt16 i = nSttPos; i < nSEndPos; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

            sal_uInt16 nCnt = nSEndPos - nSttPos;
            aPosArr.Remove( nSttPos, nCnt );
            nESttPos -= nCnt;
            n -= nCnt;
        }
        if( nESttPos < n )
        {
            
            
            if( pUndo )
                for( sal_uInt16 i = nESttPos; i < n; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

            sal_uInt16 nCnt = n - nESttPos;
            aPosArr.Remove( nESttPos, nCnt );
        }
#else


        nWidth = 0;
        long nY = !aPosArr.empty() ?
                    ( bVert ?
                      aPosArr[ 0 ].X() :
                      aPosArr[ 0 ].Y() ) :
                  0;

        for( sal_uInt16 n = 0; n < aPosArr.size(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( bCalcWidth )
            {
                if( nY == ( bVert ? rPt.X() : rPt.Y() ) ) 
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
                else
                    bCalcWidth = false;     
            }

            if( IsEmptyBox( *rPt.pSelBox, aPam ) )
            {
                if( pUndo )
                    pUndo->SaveCollection( *rPt.pSelBox );

                aPosArr.erase( aPosArr.begin() + n );
                --n;
            }
        }
#endif
    }

    
    {
        SwTableBox* pTmpBox = rBoxes[0];
        SwTableLine* pInsLine = pTmpBox->GetUpper();
        sal_uInt16 nInsPos = pInsLine->GetTabBoxes().GetPos( pTmpBox );

        lcl_InsTblBox( pTblNd, pDoc, pTmpBox, nInsPos );
        (*ppMergeBox) = pInsLine->GetTabBoxes()[ nInsPos ];
        pInsLine->GetTabBoxes().erase( pInsLine->GetTabBoxes().begin() + nInsPos );  
        (*ppMergeBox)->SetUpper( 0 );
        (*ppMergeBox)->ClaimFrmFmt();

        
        
        if( pLastBox && pFirstBox )
        {
            SvxBoxItem aBox( pFirstBox->GetFrmFmt()->GetBox() );
            const SvxBoxItem& rBox = pLastBox->GetFrmFmt()->GetBox();
            aBox.SetLine( rBox.GetRight(), BOX_LINE_RIGHT );
            aBox.SetLine( rBox.GetBottom(), BOX_LINE_BOTTOM );
            if( aBox.GetLeft() || aBox.GetTop() ||
                aBox.GetRight() || aBox.GetBottom() )
                (*ppMergeBox)->GetFrmFmt()->SetFmtAttr( aBox );
        }
    }

    
    if( !aPosArr.empty() )
    {
        SwTxtNode* pTxtNd = 0;
        SwPosition aInsPos( *(*ppMergeBox)->GetSttNd() );
        SwNodeIndex& rInsPosNd = aInsPos.nNode;

        SwPaM aPam( aInsPos );

        for( sal_uInt16 n = 0; n < aPosArr.size(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            aPam.GetPoint()->nNode.Assign( *rPt.pSelBox->GetSttNd()->
                                            EndOfSectionNode(), -1 );
            SwCntntNode* pCNd = aPam.GetCntntNode();
            aPam.GetPoint()->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );

            SwNodeIndex aSttNdIdx( *rPt.pSelBox->GetSttNd(), 1 );
            
            
            bool const bUndo(pDoc->GetIDocumentUndoRedo().DoesUndo());
            if( pUndo )
            {
                pDoc->GetIDocumentUndoRedo().DoUndo(false);
            }
            pDoc->AppendTxtNode( *aPam.GetPoint() );
            if( pUndo )
            {
                pDoc->GetIDocumentUndoRedo().DoUndo(bUndo);
            }
            SwNodeRange aRg( aSttNdIdx, aPam.GetPoint()->nNode );
            ++rInsPosNd;
            if( pUndo )
                pUndo->MoveBoxCntnt( pDoc, aRg, rInsPosNd );
            else
            {
                pDoc->MoveNodeRange( aRg, rInsPosNd,
                    IDocumentContentOperations::DOC_MOVEDEFAULT );
            }
            

            if( bCalcWidth )
                bCalcWidth = false;     

            
            rInsPosNd.Assign( pDoc->GetNodes(),
                            rInsPosNd.GetNode().EndOfSectionIndex() - 2 );
            pTxtNd = rInsPosNd.GetNode().GetTxtNode();
            if( pTxtNd )
                aInsPos.nContent.Assign(pTxtNd, pTxtNd->GetTxt().getLength());
        }

        
        
        OSL_ENSURE( (*ppMergeBox)->GetSttIdx()+2 <
                (*ppMergeBox)->GetSttNd()->EndOfSectionIndex(),
                    "empty box" );
        SwNodeIndex aIdx( *(*ppMergeBox)->GetSttNd()->EndOfSectionNode(), -1 );
        pDoc->GetNodes().Delete( aIdx, 1 );
    }

    
    (*ppMergeBox)->GetFrmFmt()->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));
    if( pUndo )
        pUndo->AddNewBox( (*ppMergeBox)->GetSttIdx() );
}


static bool lcl_CheckCol(_FndBox const&, bool* pPara);

static bool lcl_CheckRow( const _FndLine& rFndLine, bool* pPara )
{
    for (_FndBoxes::const_iterator it = rFndLine.GetBoxes().begin();
         it != rFndLine.GetBoxes().end(); ++it)
    {
        lcl_CheckCol(*it, pPara);
    }
    return *pPara;
}

static bool lcl_CheckCol( _FndBox const& rFndBox, bool* pPara )
{
    if (!rFndBox.GetBox()->GetSttNd())
    {
        if (rFndBox.GetLines().size() !=
            rFndBox.GetBox()->GetTabLines().size())
        {
            *pPara = false;
        }
        else
        {
            BOOST_FOREACH( _FndLine const& rFndLine, rFndBox.GetLines() )
                lcl_CheckRow( rFndLine, pPara );
        }
    }
    
    else if (rFndBox.GetBox()->GetFrmFmt()->GetProtect().IsCntntProtected())
        *pPara = false;
    return *pPara;
}


sal_uInt16 CheckMergeSel( const SwPaM& rPam )
{
    SwSelBoxes aBoxes;




    Point aPt;
    const SwCntntNode* pCntNd = rPam.GetCntntNode();
    const SwLayoutFrm *pStart = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                                        &aPt )->GetUpper();
    pCntNd = rPam.GetCntntNode(false);
    const SwLayoutFrm *pEnd = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                                    &aPt )->GetUpper();
    GetTblSel( pStart, pEnd, aBoxes, 0 );
    return CheckMergeSel( aBoxes );
}

sal_uInt16 CheckMergeSel( const SwSelBoxes& rBoxes )
{
    sal_uInt16 eRet = TBLMERGE_NOSELECTION;
    if( !rBoxes.empty() )
    {
        eRet = TBLMERGE_OK;

        _FndBox aFndBox( 0, 0 );
        _FndPara aPara( rBoxes, &aFndBox );
        const SwTableNode* pTblNd = aPara.rBoxes[0]->GetSttNd()->FindTableNode();
        ForEach_FndLineCopyCol( (SwTableLines&)pTblNd->GetTable().GetTabLines(), &aPara );
        if( !aFndBox.GetLines().empty() )
        {
            bool bMergeSelOk = true;
            _FndBox* pFndBox = &aFndBox;
            _FndLine* pFndLine = 0;
            while( pFndBox && 1 == pFndBox->GetLines().size() )
            {
                pFndLine = &pFndBox->GetLines().front();
                if( 1 == pFndLine->GetBoxes().size() )
                    pFndBox = &pFndLine->GetBoxes().front();
                else
                    pFndBox = 0;
            }
            if( pFndBox )
            {
                for (_FndLines::const_iterator it = pFndBox->GetLines().begin(),
                        end = pFndBox->GetLines().end(); it != end; ++it)
                {
                    lcl_CheckRow(*it, &bMergeSelOk);
                }
            }
            else if( pFndLine )
            {
                for (_FndBoxes::const_iterator it = pFndLine->GetBoxes().begin(),
                        end = pFndLine->GetBoxes().end(); it != end; ++it)
                {
                    lcl_CheckCol(*it, &bMergeSelOk);
                }
            }
            if( !bMergeSelOk )
                eRet = TBLMERGE_TOOCOMPLEX;
        }
        else
            eRet = TBLMERGE_NOSELECTION;
    }
    return eRet;
}

static SwTwips lcl_CalcWish( const SwLayoutFrm *pCell, long nWish,
                                                const long nAct )
{
    const SwLayoutFrm *pTmp = pCell;
    if ( !nWish )
        nWish = 1;

    const sal_Bool bRTL = pCell->IsRightToLeft();
    SwTwips nRet = bRTL ?
        nAct - pCell->Frm().Width() :
        0;

    while ( pTmp )
    {
        while ( pTmp->GetPrev() )
        {
            pTmp = (SwLayoutFrm*)pTmp->GetPrev();
            long nTmp = pTmp->GetFmt()->GetFrmSize().GetWidth();
            nRet += ( bRTL ? ( -1 ) : 1 ) * nTmp * nAct / nWish;
        }
        pTmp = pTmp->GetUpper()->GetUpper();
        if ( pTmp && !pTmp->IsCellFrm() )
            pTmp = 0;
    }
    return nRet;
}

static void lcl_FindStartEndRow( const SwLayoutFrm *&rpStart,
                             const SwLayoutFrm *&rpEnd,
                             const int bChkProtected )
{
    
    
    rpStart = (SwLayoutFrm*)rpStart->GetUpper()->Lower();
    while ( rpEnd->GetNext() )
        rpEnd = (SwLayoutFrm*)rpEnd->GetNext();

    std::deque<const SwLayoutFrm *> aSttArr, aEndArr;
    const SwLayoutFrm *pTmp;
    for( pTmp = rpStart; (FRM_CELL|FRM_ROW) & pTmp->GetType();
                pTmp = pTmp->GetUpper() )
    {
        aSttArr.push_front( pTmp );
    }
    for( pTmp = rpEnd; (FRM_CELL|FRM_ROW) & pTmp->GetType();
                pTmp = pTmp->GetUpper() )
    {
        aEndArr.push_front( pTmp );
    }

    for( sal_uInt16 n = 0; n < aEndArr.size() && n < aSttArr.size(); ++n )
        if( aSttArr[ n ] != aEndArr[ n ] )
        {
            
            if( n & 1 )                 
            {
                rpStart = aSttArr[ n ];
                rpEnd = aEndArr[ n ];
            }
            else                                
            {
                
                
                
                rpStart = aSttArr[ n+1 ];
                rpEnd = aEndArr[ n+1 ];
                if( n )
                {
                    const SwCellFrm* pCellFrm = (SwCellFrm*)aSttArr[ n-1 ];
                    const SwTableLines& rLns = pCellFrm->
                                                GetTabBox()->GetTabLines();
                    if( rLns[ 0 ] == ((SwRowFrm*)aSttArr[ n ])->GetTabLine() &&
                        rLns[ rLns.size() - 1 ] ==
                                    ((SwRowFrm*)aEndArr[ n ])->GetTabLine() )
                    {
                        rpStart = rpEnd = pCellFrm;
                        while ( rpStart->GetPrev() )
                            rpStart = (SwLayoutFrm*)rpStart->GetPrev();
                        while ( rpEnd->GetNext() )
                            rpEnd = (SwLayoutFrm*)rpEnd->GetNext();
                    }
                }
            }
            break;
        }

    if( !bChkProtected )    
        return;


    
    while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
        rpStart = (SwLayoutFrm*)rpStart->GetNext();
    while ( rpEnd->GetFmt()->GetProtect().IsCntntProtected() )
        rpEnd = (SwLayoutFrm*)rpEnd->GetPrev();
}


static void lcl_FindStartEndCol( const SwLayoutFrm *&rpStart,
                             const SwLayoutFrm *&rpEnd,
                             const int bChkProtected )
{
    
    
    
    if( !rpStart )
        return;
    const SwTabFrm *pOrg = rpStart->FindTabFrm();
    const SwTabFrm *pTab = pOrg;

    SWRECTFN( pTab )

    sal_Bool bRTL = pTab->IsRightToLeft();
    const long nTmpWish = pOrg->GetFmt()->GetFrmSize().GetWidth();
    const long nWish = ( nTmpWish > 0 ) ? nTmpWish : 1;

    while ( pTab->IsFollow() )
    {
        const SwFrm *pTmp = pTab->FindPrev();
        OSL_ENSURE( pTmp->IsTabFrm(), "Predecessor of Follow is not Master." );
        pTab = (const SwTabFrm*)pTmp;
    }

    SwTwips nSX  = 0;
    SwTwips nSX2 = 0;

    if ( pTab->GetTable()->IsNewModel() )
    {
        nSX  = (rpStart->Frm().*fnRect->fnGetLeft )();
        nSX2 = (rpStart->Frm().*fnRect->fnGetRight)();
    }
    else
    {
        const SwTwips nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
        nSX = ::lcl_CalcWish( rpStart, nWish, nPrtWidth ) + (pTab->*fnRect->fnGetPrtLeft)();
        nSX2 = nSX + (rpStart->GetFmt()->GetFrmSize().GetWidth() * nPrtWidth / nWish);
    }

    const SwLayoutFrm *pTmp = pTab->FirstCell();

    while ( pTmp &&
            (!pTmp->IsCellFrm() ||
             ( ( ! bRTL && (pTmp->Frm().*fnRect->fnGetLeft)() < nSX &&
                           (pTmp->Frm().*fnRect->fnGetRight)()< nSX2 ) ||
               (   bRTL && (pTmp->Frm().*fnRect->fnGetLeft)() > nSX &&
                           (pTmp->Frm().*fnRect->fnGetRight)()> nSX2 ) ) ) )
        pTmp = pTmp->GetNextLayoutLeaf();

    if ( pTmp )
        rpStart = pTmp;

    pTab = pOrg;

    const SwTabFrm* pLastValidTab = pTab;
    while ( pTab->GetFollow() )
    {
        //
        
        
        
        //
        if ( pTab->HasFollowFlowLine() )
        {
            pTab = pTab->GetFollow();
            const SwFrm* pTmpRow = pTab->GetFirstNonHeadlineRow();
            if ( pTmpRow && pTmpRow->GetNext() )
                pLastValidTab = pTab;
        }
        else
            pLastValidTab = pTab = pTab->GetFollow();
    }
    pTab = pLastValidTab;

    SwTwips nEX = 0;

    if ( pTab->GetTable()->IsNewModel() )
    {
        nEX = (rpEnd->Frm().*fnRect->fnGetLeft )();
    }
    else
    {
        const SwTwips nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
        nEX = ::lcl_CalcWish( rpEnd, nWish, nPrtWidth ) + (pTab->*fnRect->fnGetPrtLeft)();
    }

    const SwCntntFrm* pLastCntnt = pTab->FindLastCntnt();
    rpEnd = pLastCntnt ? pLastCntnt->GetUpper() : 0;
    
    
    if ( !pLastCntnt ) return;

    while( !rpEnd->IsCellFrm() )
        rpEnd = rpEnd->GetUpper();

    while ( (   bRTL && (rpEnd->Frm().*fnRect->fnGetLeft)() < nEX ) ||
            ( ! bRTL && (rpEnd->Frm().*fnRect->fnGetLeft)() > nEX ) )
    {
        const SwLayoutFrm* pTmpLeaf = rpEnd->GetPrevLayoutLeaf();
        if( !pTmpLeaf || !pTab->IsAnLower( pTmpLeaf ) )
            break;
        rpEnd = pTmpLeaf;
    }

    if( !bChkProtected )    
        return;

    
    
    while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
    {
        const SwLayoutFrm *pTmpLeaf = rpStart;
        pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() > nEX ) 
            pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() < nSX &&
                            (pTmpLeaf->Frm().*fnRect->fnGetRight)()< nSX2 )
            pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        const SwTabFrm *pTmpTab = rpStart->FindTabFrm();
        if ( !pTmpTab->IsAnLower( pTmpLeaf ) )
        {
            pTmpTab = pTmpTab->GetFollow();
            rpStart = pTmpTab->FirstCell();
            while ( (rpStart->Frm().*fnRect->fnGetLeft)() < nSX &&
                    (rpStart->Frm().*fnRect->fnGetRight)()< nSX2 )
                rpStart = rpStart->GetNextLayoutLeaf();
        }
        else
            rpStart = pTmpLeaf;
    }
    while ( rpEnd->GetFmt()->GetProtect().IsCntntProtected() )
    {
        const SwLayoutFrm *pTmpLeaf = rpEnd;
        pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() < nEX )
            pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() > nEX )
            pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        const SwTabFrm *pTmpTab = rpEnd->FindTabFrm();
        if ( !pTmpLeaf || !pTmpTab->IsAnLower( pTmpLeaf ) )
        {
            pTmpTab = (const SwTabFrm*)pTmpTab->FindPrev();
            OSL_ENSURE( pTmpTab->IsTabFrm(), "Predecessor of Follow not Master.");
            rpEnd = pTmpTab->FindLastCntnt()->GetUpper();
            while( !rpEnd->IsCellFrm() )
                rpEnd = rpEnd->GetUpper();
            while ( (rpEnd->Frm().*fnRect->fnGetLeft)() > nEX )
                rpEnd = rpEnd->GetPrevLayoutLeaf();
        }
        else
            rpEnd = pTmpLeaf;
    }
}


void MakeSelUnions( SwSelUnions& rUnions, const SwLayoutFrm *pStart,
                    const SwLayoutFrm *pEnd, const SwTblSearchType eSearchType )
{
    while ( pStart && !pStart->IsCellFrm() )
        pStart = pStart->GetUpper();
    while ( pEnd && !pEnd->IsCellFrm() )
        pEnd = pEnd->GetUpper();

    if ( !pStart || !pEnd )
    {
        OSL_FAIL( "MakeSelUnions with pStart or pEnd not in CellFrm" );
        return;
    }

    const SwTabFrm *pTable = pStart->FindTabFrm();
    const SwTabFrm *pEndTable = pEnd->FindTabFrm();
    if( !pTable || !pEndTable )
        return;
    bool bExchange = false;

    if ( pTable != pEndTable )
    {
        if ( !pTable->IsAnFollow( pEndTable ) )
        {
            OSL_ENSURE( pEndTable->IsAnFollow( pTable ), "Tabchain in knots." );
            bExchange = true;
        }
    }
    else
    {
        SWRECTFN( pTable )
        long nSttTop = (pStart->Frm().*fnRect->fnGetTop)();
        long nEndTop = (pEnd->Frm().*fnRect->fnGetTop)();
        if( nSttTop == nEndTop )
        {
            if( (pStart->Frm().*fnRect->fnGetLeft)() >
                (pEnd->Frm().*fnRect->fnGetLeft)() )
                bExchange = true;
        }
        else if( bVert == ( nSttTop < nEndTop ) )
            bExchange = true;
    }
    if ( bExchange )
    {
        const SwLayoutFrm *pTmp = pStart;
        pStart = pEnd;
        pEnd = pTmp;
        
        
    }

    
    
    if( nsSwTblSearchType::TBLSEARCH_ROW == ((~nsSwTblSearchType::TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndRow( pStart, pEnd, nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType );
    else if( nsSwTblSearchType::TBLSEARCH_COL == ((~nsSwTblSearchType::TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndCol( pStart, pEnd, nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType );

    if ( !pEnd ) return; 

    
    pTable = pStart->FindTabFrm();
    pEndTable = pEnd->FindTabFrm();

    const long nStSz = pStart->GetFmt()->GetFrmSize().GetWidth();
    const long nEdSz = pEnd->GetFmt()->GetFrmSize().GetWidth();
    const long nWish = std::max( 1L, pTable->GetFmt()->GetFrmSize().GetWidth() );
    while ( pTable )
    {
        SWRECTFN( pTable )
        const long nOfst = (pTable->*fnRect->fnGetPrtLeft)();
        const long nPrtWidth = (pTable->Prt().*fnRect->fnGetWidth)();
        long nSt1 = ::lcl_CalcWish( pStart, nWish, nPrtWidth ) + nOfst;
        long nEd1 = ::lcl_CalcWish( pEnd,   nWish, nPrtWidth ) + nOfst;

        if ( nSt1 <= nEd1 )
            nEd1 += (long)((nEdSz * nPrtWidth) / nWish) - 1;
        else
            nSt1 += (long)((nStSz * nPrtWidth) / nWish) - 1;

        long nSt2;
        long nEd2;
        if( pTable->IsAnLower( pStart ) )
            nSt2 = (pStart->Frm().*fnRect->fnGetTop)();
        else
            nSt2 = (pTable->Frm().*fnRect->fnGetTop)();
        if( pTable->IsAnLower( pEnd ) )
            nEd2 = (pEnd->Frm().*fnRect->fnGetBottom)();
        else
            nEd2 = (pTable->Frm().*fnRect->fnGetBottom)();
        Point aSt, aEd;
        if( nSt1 > nEd1 )
        {
            long nTmp = nSt1;
            nSt1 = nEd1;
            nEd1 = nTmp;
        }
        if( nSt2 > nEd2 )
        {
            long nTmp = nSt2;
            nSt2 = nEd2;
            nEd2 = nTmp;
        }
        if( bVert )
        {
            aSt = Point( nSt2, nSt1 );
            aEd = Point( nEd2, nEd1 );
        }
        else
        {
            aSt = Point( nSt1, nSt2 );
            aEd = Point( nEd1, nEd2 );
        }

        const Point aDiff( aEd - aSt );
        SwRect aUnion( aSt, Size( aDiff.X(), aDiff.Y() ) );
        aUnion.Justify();

        if( !(nsSwTblSearchType::TBLSEARCH_NO_UNION_CORRECT & eSearchType ))
        {
            
            
            
            
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      (const SwLayoutFrm*)pTable->Lower();

            while ( pRow && !pRow->Frm().IsOver( aUnion ) )
                pRow = (SwLayoutFrm*)pRow->GetNext();

            
            
            
            
            const SwFrm* pTmpCell = 0;
            if ( pTable->IsFollow() && pRow && pRow->IsInFollowFlowRow() )
            {
                const SwFrm* pTmpRow = pRow;
                while ( pTmpRow && pTmpRow->IsRowFrm() )
                {
                    pTmpCell = static_cast<const SwRowFrm*>(pTmpRow)->Lower();
                    pTmpRow  = static_cast<const SwCellFrm*>(pTmpCell)->Lower();
                }
                OSL_ENSURE( !pTmpCell || pTmpCell->IsCellFrm(), "Lower of rowframe != cellframe?!" );
            }

            const SwLayoutFrm* pFirst = pTmpCell ?
                                        static_cast<const SwLayoutFrm*>(pTmpCell) :
                                        pRow ?
                                        pRow->FirstCell() :
                                        0;

            while ( pFirst && !::IsFrmInTblSel( aUnion, pFirst ) )
            {
                if ( pFirst->GetNext() )
                {
                    pFirst = (const SwLayoutFrm*)pFirst->GetNext();
                    if ( pFirst->Lower() && pFirst->Lower()->IsRowFrm() )
                        pFirst = pFirst->FirstCell();
                }
                else
                    pFirst = ::lcl_FindNextCellFrm( pFirst );
            }
            const SwLayoutFrm* pLast = 0;
            const SwFrm* pLastCntnt = pTable->FindLastCntnt();
            if ( pLastCntnt )
                pLast = ::lcl_FindCellFrm( pLastCntnt->GetUpper() );

            while ( pLast && !::IsFrmInTblSel( aUnion, pLast ) )
                pLast = ::lcl_FindCellFrm( pLast->GetPrevLayoutLeaf() );

            if ( pFirst && pLast ) 
            {
                aUnion = pFirst->Frm();
                aUnion.Union( pLast->Frm() );
            }
            else
                aUnion.Width( 0 );
        }

        if( (aUnion.*fnRect->fnGetWidth)() )
        {
            SwSelUnion *pTmp = new SwSelUnion( aUnion, (SwTabFrm*)pTable );
            rUnions.push_back( pTmp );
        }

        pTable = pTable->GetFollow();
        if ( pTable != pEndTable && pEndTable->IsAnFollow( pTable ) )
            pTable = 0;
    }
}

sal_Bool CheckSplitCells( const SwCrsrShell& rShell, sal_uInt16 nDiv,
                        const SwTblSearchType eSearchType )
{
    if( !rShell.IsTableMode() )
        rShell.GetCrsr();

    return CheckSplitCells( *rShell.getShellCrsr(false), nDiv, eSearchType );
}

sal_Bool CheckSplitCells( const SwCursor& rCrsr, sal_uInt16 nDiv,
                        const SwTblSearchType eSearchType )
{
    if( 1 >= nDiv )
        return sal_False;

    sal_uInt16 nMinValue = nDiv * MINLAY;

    
    Point aPtPos, aMkPos;
    const SwShellCrsr* pShCrsr = dynamic_cast<const SwShellCrsr*>(&rCrsr);
    if( pShCrsr )
    {
        aPtPos = pShCrsr->GetPtPos();
        aMkPos = pShCrsr->GetMkPos();
    }

    const SwCntntNode* pCntNd = rCrsr.GetCntntNode();
    const SwLayoutFrm *pStart = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                                        &aPtPos )->GetUpper();
    pCntNd = rCrsr.GetCntntNode(false);
    const SwLayoutFrm *pEnd = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                &aMkPos )->GetUpper();

    SWRECTFN( pStart->GetUpper() )

    
    SwSelUnions aUnions;

    ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

    
    for ( sal_uInt16 i = 0; i < aUnions.size(); ++i )
    {
        SwSelUnion *pUnion = &aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        
        const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                  pTable->GetFirstNonHeadlineRow() :
                                  (const SwLayoutFrm*)pTable->Lower();

        while ( pRow )
        {
            if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
            {
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    OSL_ENSURE( pCell->IsCellFrm(), "Frame without cell" );
                    if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                    {
                        if( (pCell->Frm().*fnRect->fnGetWidth)() < nMinValue )
                            return sal_False;
                    }

                    if ( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }
            }
            pRow = (const SwLayoutFrm*)pRow->GetNext();
        }
    }
    return sal_True;
}






static void lcl_InsertRow( SwTableLine &rLine, SwLayoutFrm *pUpper, SwFrm *pSibling )
{
    SwRowFrm *pRow = new SwRowFrm( rLine, pUpper );
    if ( pUpper->IsTabFrm() && ((SwTabFrm*)pUpper)->IsFollow() )
    {
        SwTabFrm* pTabFrm = (SwTabFrm*)pUpper;
        pTabFrm->FindMaster()->InvalidatePos(); 

        if ( pSibling && pTabFrm->IsInHeadline( *pSibling ) )
        {
            
            pSibling = pTabFrm->GetFirstNonHeadlineRow();
        }
    }
    pRow->Paste( pUpper, pSibling );
    pRow->RegistFlys();
}


static void _FndBoxCopyCol( SwTableBox* pBox, _FndPara* pFndPara )
{
    _FndBox* pFndBox = new _FndBox( pBox, pFndPara->pFndLine );
    if( pBox->GetTabLines().size() )
    {
        _FndPara aPara( *pFndPara, pFndBox );
        ForEach_FndLineCopyCol( pFndBox->GetBox()->GetTabLines(), &aPara );
        if( pFndBox->GetLines().empty() )
        {
            delete pFndBox;
            return;
        }
    }
    else
    {
        if( pFndPara->rBoxes.find( pBox ) == pFndPara->rBoxes.end())
        {
            delete pFndBox;
            return;
        }
    }
    pFndPara->pFndLine->GetBoxes().push_back( pFndBox );
}

static void _FndLineCopyCol( SwTableLine* pLine, _FndPara* pFndPara )
{
    _FndLine* pFndLine = new _FndLine( pLine, pFndPara->pFndBox );
    _FndPara aPara( *pFndPara, pFndLine );
    for( SwTableBoxes::iterator it = pFndLine->GetLine()->GetTabBoxes().begin();
             it != pFndLine->GetLine()->GetTabBoxes().end(); ++it)
        _FndBoxCopyCol(*it, &aPara );
    if( pFndLine->GetBoxes().size() )
    {
        pFndPara->pFndBox->GetLines().push_back( pFndLine );
    }
    else
        delete pFndLine;
}

void ForEach_FndLineCopyCol(SwTableLines& rLines, _FndPara* pFndPara )
{
    for( SwTableLines::iterator it = rLines.begin(); it != rLines.end(); ++it )
        _FndLineCopyCol( *it, pFndPara );
}

void _FndBox::SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable )
{
    
    
    
    
    

    sal_uInt16 nStPos = USHRT_MAX;
    sal_uInt16 nEndPos= 0;

    for (size_t i = 0; i < rBoxes.size(); ++i)
    {
        SwTableLine *pLine = rBoxes[i]->GetUpper();
        while ( pLine->GetUpper() )
            pLine = pLine->GetUpper()->GetUpper();
        const sal_uInt16 nPos = rTable.GetTabLines().GetPos(
                    (const SwTableLine*&)pLine ) + 1;

        OSL_ENSURE( nPos != USHRT_MAX, "TableLine not found." );

        if( nStPos > nPos )
            nStPos = nPos;

        if( nEndPos < nPos )
            nEndPos = nPos;
    }
    if ( nStPos > 1 )
        pLineBefore = rTable.GetTabLines()[nStPos - 2];
    if ( nEndPos < rTable.GetTabLines().size() )
        pLineBehind = rTable.GetTabLines()[nEndPos];
}

void _FndBox::SetTableLines( const SwTable &rTable )
{
    
    
    
    
    

    if( GetLines().empty() )
        return;

    SwTableLine* pTmpLine = GetLines().front().GetLine();
    sal_uInt16 nPos = rTable.GetTabLines().GetPos( pTmpLine );
    OSL_ENSURE( USHRT_MAX != nPos, "Line steht nicht in der Tabelle" );
    if( nPos )
        pLineBefore = rTable.GetTabLines()[ nPos - 1 ];

    pTmpLine = GetLines().back().GetLine();
    nPos = rTable.GetTabLines().GetPos( pTmpLine );
    OSL_ENSURE( USHRT_MAX != nPos, "Line steht nicht in der Tabelle" );
    if( ++nPos < rTable.GetTabLines().size() )
        pLineBehind = rTable.GetTabLines()[nPos];
}

inline void UnsetFollow( SwFlowFrm *pTab )
{
    pTab->m_pPrecede = 0;
}



void _FndBox::DelFrms( SwTable &rTable,sal_Bool bAccTableDispose )
{
    
    
    
    
    

    sal_uInt16 nStPos = 0;
    sal_uInt16 nEndPos= rTable.GetTabLines().size() - 1;
    if( rTable.IsNewModel() && pLineBefore )
        rTable.CheckRowSpan( pLineBefore, true );
    if ( pLineBefore )
    {
        nStPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBefore );
        OSL_ENSURE( nStPos != USHRT_MAX, "The fox stole the line!" );
        ++nStPos;
    }
    if( rTable.IsNewModel() && pLineBehind )
        rTable.CheckRowSpan( pLineBehind, false );
    if ( pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBehind );
        OSL_ENSURE( nEndPos != USHRT_MAX, "The fox stole the line!" );
        --nEndPos;
    }

    for ( sal_uInt16 i = nStPos; i <= nEndPos; ++i)
    {
        SwFrmFmt *pFmt = rTable.GetTabLines()[i]->GetFrmFmt();
        SwIterator<SwRowFrm,SwFmt> aIter( *pFmt );
        for ( SwRowFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
        {
                if ( pFrm->GetTabLine() == rTable.GetTabLines()[i] )
                {
                    bool bDel = true;
                    SwTabFrm *pUp = !pFrm->GetPrev() && !pFrm->GetNext() ?
                                            (SwTabFrm*)pFrm->GetUpper() : 0;
                    if ( !pUp )
                    {
                        const sal_uInt16 nRepeat =
                                ((SwTabFrm*)pFrm->GetUpper())->GetTable()->GetRowsToRepeat();
                        if ( nRepeat > 0 &&
                             ((SwTabFrm*)pFrm->GetUpper())->IsFollow() )
                        {
                            if ( !pFrm->GetNext() )
                            {
                                SwRowFrm* pFirstNonHeadline =
                                    ((SwTabFrm*)pFrm->GetUpper())->GetFirstNonHeadlineRow();
                                if ( pFirstNonHeadline == pFrm )
                                {
                                    pUp = (SwTabFrm*)pFrm->GetUpper();
                                }
                            }
                        }
                    }
                    if ( pUp )
                    {
                        SwTabFrm *pFollow = pUp->GetFollow();
                        SwTabFrm *pPrev   = pUp->IsFollow() ? pUp : 0;
                        if ( pPrev )
                        {
                            SwFrm *pTmp = pPrev->FindPrev();
                            OSL_ENSURE( pTmp->IsTabFrm(),
                                    "Predecessor of Follow is no Master.");
                            pPrev = (SwTabFrm*)pTmp;
                        }
                        if ( pPrev )
                        {
                            pPrev->SetFollow( pFollow );
                            
                            
                            
                            
                            pPrev->SetFollowFlowLine( sal_False );
                        }
                        else if ( pFollow )
                            ::UnsetFollow( pFollow );

                        
                        if ( pPrev || pFollow )
                        {
                            
                            
                            {
                                SwSectionFrm* pSctFrm = pUp->FindSctFrm();
                                bool bOldSectLock = false;
                                if ( pSctFrm )
                                {
                                    bOldSectLock = pSctFrm->IsColLocked();
                                    pSctFrm->ColLock();
                                }
                                pUp->Cut();
                                if ( pSctFrm && !bOldSectLock )
                                {
                                    pSctFrm->ColUnlock();
                                }
                            }
                            delete pUp;
                            bDel = false; 
                        }
                    }
                    if ( bDel )
                    {
                        SwFrm* pTabFrm = pFrm->GetUpper();
                        if ( pTabFrm->IsTabFrm() &&
                            !pFrm->GetNext() &&
                             ((SwTabFrm*)pTabFrm)->GetFollow() )
                        {
                            
                            
                            
                            ((SwTabFrm*)pTabFrm)->SetFollowFlowLine( sal_False );
                        }
                        
                        pFrm->SetAccTableDispose( bAccTableDispose );
                        pFrm->Cut();
                        
                        pFrm->SetAccTableDispose( true );
                        delete pFrm;
                    }
                }
        }
    }
}

static bool lcl_IsLineOfTblFrm( const SwTabFrm& rTable, const SwFrm& rChk )
{
    const SwTabFrm* pTblFrm = rChk.FindTabFrm();
    if( pTblFrm->IsFollow() )
        pTblFrm = pTblFrm->FindMaster( true );
    return &rTable == pTblFrm;
}

/*
 * lcl_UpdateRepeatedHeadlines
 */
static void lcl_UpdateRepeatedHeadlines( SwTabFrm& rTabFrm, bool bCalcLowers )
{
    OSL_ENSURE( rTabFrm.IsFollow(), "lcl_UpdateRepeatedHeadlines called for non-follow tab" );

    
    SwRowFrm* pLower = 0;
    while ( 0 != ( pLower = (SwRowFrm*)rTabFrm.Lower() ) && pLower->IsRepeatedHeadline() )
    {
        pLower->Cut();
        delete pLower;
    }

    
    pLower = (SwRowFrm*)rTabFrm.Lower();
    SwTable& rTable = *rTabFrm.GetTable();
    const sal_uInt16 nRepeat = rTable.GetRowsToRepeat();
    for ( sal_uInt16 nIdx = 0; nIdx < nRepeat; ++nIdx )
    {
        SwRowFrm* pHeadline = new SwRowFrm( *rTable.GetTabLines()[ nIdx ], &rTabFrm );
        pHeadline->SetRepeatedHeadline( true );
        pHeadline->Paste( &rTabFrm, pLower );
        pHeadline->RegistFlys();
    }

    if ( bCalcLowers )
        rTabFrm.SetCalcLowers();
}

void _FndBox::MakeFrms( SwTable &rTable )
{
    
    
    sal_uInt16 nStPos = 0;
    sal_uInt16 nEndPos= rTable.GetTabLines().size() - 1;
    if ( pLineBefore )
    {
        nStPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBefore );
        OSL_ENSURE( nStPos != USHRT_MAX, "Fox stole the line!" );
        ++nStPos;

    }
    if ( pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBehind );
        OSL_ENSURE( nEndPos != USHRT_MAX, "Fox stole the line!" );
        --nEndPos;
    }
    
    SwIterator<SwTabFrm,SwFmt> aTabIter( *rTable.GetFrmFmt() );
    for ( SwTabFrm *pTable = aTabIter.First(); pTable; pTable = aTabIter.Next() )
    {
        if ( !pTable->IsFollow() )
        {
            SwRowFrm  *pSibling = 0;
            SwFrm  *pUpperFrm  = 0;
            int i;
            for ( i = rTable.GetTabLines().size()-1;
                    i >= 0 && !pSibling; --i )
            {
                SwTableLine *pLine = pLineBehind ? pLineBehind :
                                                    rTable.GetTabLines()[static_cast<sal_uInt16>(i)];
                SwIterator<SwRowFrm,SwFmt> aIter( *pLine->GetFrmFmt() );
                pSibling = aIter.First();
                while ( pSibling && (
                            pSibling->GetTabLine() != pLine ||
                            !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                            pSibling->IsRepeatedHeadline() ||
                            
                            
                            ( pLineBehind && pSibling->IsInFollowFlowRow() ) ||
                            (!pLineBehind && pSibling->IsInSplitTableRow() ) ) )
                {
                    pSibling = aIter.Next();
                }
            }
            if ( pSibling )
            {
                pUpperFrm = pSibling->GetUpper();
                if ( !pLineBehind )
                    pSibling = 0;
            }
            else

                pUpperFrm = pTable;

            for ( i = nStPos; (sal_uInt16)i <= nEndPos; ++i )
                ::lcl_InsertRow( *rTable.GetTabLines()[static_cast<sal_uInt16>(i)],
                                (SwLayoutFrm*)pUpperFrm, pSibling );
            if ( pUpperFrm->IsTabFrm() )
                ((SwTabFrm*)pUpperFrm)->SetCalcLowers();
        }
        else if ( rTable.GetRowsToRepeat() > 0 )
        {
            
            lcl_UpdateRepeatedHeadlines( *pTable, true );
        }
    }
}

void _FndBox::MakeNewFrms( SwTable &rTable, const sal_uInt16 nNumber,
                                            const sal_Bool bBehind )
{
    
    
    
    const sal_uInt16 nBfPos = pLineBefore ?
        rTable.GetTabLines().GetPos( (const SwTableLine*&)pLineBefore ) :
        USHRT_MAX;
    const sal_uInt16 nBhPos = pLineBehind ?
        rTable.GetTabLines().GetPos( (const SwTableLine*&)pLineBehind ) :
        USHRT_MAX;

    
    

    const sal_uInt16 nCnt =
        ((nBhPos != USHRT_MAX ? nBhPos : rTable.GetTabLines().size()) -
         (nBfPos != USHRT_MAX ? nBfPos + 1 : 0)) / (nNumber + 1);

    
    SwIterator<SwTabFrm,SwFmt> aTabIter( *rTable.GetFrmFmt() );
    SwTabFrm *pTable;
    for ( pTable = aTabIter.First(); pTable; pTable = aTabIter.Next() )
    {
        if( !pTable->IsFollow() )
        {
            SwRowFrm* pSibling = 0;
            SwLayoutFrm *pUpperFrm   = 0;
            if ( bBehind )
            {
                if ( pLineBehind )
                {
                    SwIterator<SwRowFrm,SwFmt> aIter( *pLineBehind->GetFrmFmt() );
                    pSibling = aIter.First();
                    while ( pSibling && (
                                
                                pSibling->GetTabLine() != pLineBehind ||
                                
                                !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                                
                                pSibling->IsRepeatedHeadline() ||
                                
                                pSibling->IsInFollowFlowRow() ) )
                    {
                          pSibling = aIter.Next();
                    }
                }
                if ( pSibling )
                    pUpperFrm = pSibling->GetUpper();
                else
                {
                    while( pTable->GetFollow() )
                        pTable = pTable->GetFollow();
                    pUpperFrm = pTable;
                }
                const sal_uInt16 nMax = nBhPos != USHRT_MAX ?
                                    nBhPos : rTable.GetTabLines().size();

                sal_uInt16 i = nBfPos != USHRT_MAX ? nBfPos + 1 + nCnt : nCnt;

                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i], pUpperFrm, pSibling );
                if ( pUpperFrm->IsTabFrm() )
                    ((SwTabFrm*)pUpperFrm)->SetCalcLowers();
            }
            else 
            {
                sal_uInt16 i;

                
                
                for ( i = 0; !pSibling; ++i )
                {
                    SwTableLine* pLine = pLineBefore ? pLineBefore : rTable.GetTabLines()[i];

                    SwIterator<SwRowFrm,SwFmt> aIter( *pLine->GetFrmFmt() );
                    pSibling = aIter.First();

                    while ( pSibling && (
                            
                            pSibling->GetTabLine() != pLine ||
                            
                            !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                            
                            pSibling->IsRepeatedHeadline() ||
                            
                            
                            
                            
                            
                            
                            ( ( !pLineBefore && pSibling->IsInFollowFlowRow() ) ||
                              (  pLineBefore && pSibling->IsInSplitTableRow() ) ) ) )
                    {
                        pSibling = aIter.Next();
                    }
                }

                pUpperFrm = pSibling->GetUpper();
                if ( pLineBefore )
                    pSibling = (SwRowFrm*) pSibling->GetNext();

                sal_uInt16 nMax = nBhPos != USHRT_MAX ?
                                    nBhPos - nCnt :
                                    rTable.GetTabLines().size() - nCnt;

                i = nBfPos != USHRT_MAX ? nBfPos + 1 : 0;
                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i],
                                pUpperFrm, pSibling );
                if ( pUpperFrm->IsTabFrm() )
                    ((SwTabFrm*)pUpperFrm)->SetCalcLowers();
            }
        }
    }

    
    
    const sal_uInt16 nRowsToRepeat = rTable.GetRowsToRepeat();
    if ( nRowsToRepeat > 0 &&
         ( ( !bBehind && ( nBfPos == USHRT_MAX || nBfPos + 1 < nRowsToRepeat ) ) ||
           (  bBehind && ( ( nBfPos == USHRT_MAX && nRowsToRepeat > 1 ) || nBfPos + 2 < nRowsToRepeat ) ) ) )
    {
        for ( pTable = aTabIter.First(); pTable; pTable = aTabIter.Next() )
        {
            if ( pTable->Lower() )
            {
                if ( pTable->IsFollow() )
                {
                    lcl_UpdateRepeatedHeadlines( *pTable, true );
                }

                OSL_ENSURE( ((SwRowFrm*)pTable->Lower())->GetTabLine() ==
                        rTable.GetTabLines()[0], "MakeNewFrms: Table corruption!" );
            }
        }
    }
}

sal_Bool _FndBox::AreLinesToRestore( const SwTable &rTable ) const
{
    

    if ( !pLineBefore && !pLineBehind && rTable.GetTabLines().size() )
        return sal_True;

    sal_uInt16 nBfPos;
    if(pLineBefore)
    {
        const SwTableLine* rLBefore = (const SwTableLine*)pLineBefore;
        nBfPos = rTable.GetTabLines().GetPos( rLBefore );
    }
    else
        nBfPos = USHRT_MAX;

    sal_uInt16 nBhPos;
    if(pLineBehind)
    {
        const SwTableLine* rLBehind = (const SwTableLine*)pLineBehind;
        nBhPos = rTable.GetTabLines().GetPos( rLBehind );
    }
    else
        nBhPos = USHRT_MAX;

    if ( nBfPos == nBhPos ) 
    {
        OSL_FAIL( "Table, erase but not on any area !?!" );
        return sal_False;
    }

    if ( rTable.GetRowsToRepeat() > 0 )
    {
        
        SwIterator<SwTabFrm,SwFmt> aIter( *rTable.GetFrmFmt() );
        for( SwTabFrm* pTable = aIter.First(); pTable; pTable = aIter.Next() )
        {
            if( pTable->IsFollow() )
            {
                
                lcl_UpdateRepeatedHeadlines( *pTable, false );
            }
        }
    }

    
    if ( nBfPos == USHRT_MAX && nBhPos == 0 )
        return sal_False;

    
    if ( nBhPos == USHRT_MAX && nBfPos == (rTable.GetTabLines().size() - 1) )
        return sal_False;

    
    if ( nBfPos != USHRT_MAX && nBhPos != USHRT_MAX && (nBfPos + 1) == nBhPos )
        return sal_False;

    
    
    return sal_True;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
