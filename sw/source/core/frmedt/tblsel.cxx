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

// see also swtable.cxx
#define COLFUZZY 20L

// macros, determining how table boxes are merged:
//  - 1. remove empty lines, all boxes separated with blanks,
//      all lines separated with ParaBreak
//  - 2. remove all empty lines and remove all empty boxes at beginning and end,
//      all boxes separated with Blank,
//      all lines separated with ParaBreak
//  - 3. remove all empty boxes, all boxes separated with blanks,
//      all lines separated with ParaBreak

#undef      DEL_ONLY_EMPTY_LINES
#undef      DEL_EMPTY_BOXES_AT_START_AND_END
#define     DEL_ALL_EMPTY_BOXES

struct _CmpLPt
{
    Point aPos;
    const SwTableBox* pSelBox;
    sal_Bool bVert;

    _CmpLPt( const Point& rPt, const SwTableBox* pBox, sal_Bool bVertical );

    bool operator==( const _CmpLPt& rCmp ) const
    {   return X() == rCmp.X() && Y() == rCmp.Y() ? sal_True : sal_False; }

    bool operator<( const _CmpLPt& rCmp ) const
    {
        if ( bVert )
            return X() > rCmp.X() || ( X() == rCmp.X() && Y() < rCmp.Y() )
                    ? sal_True : sal_False;
        else
            return Y() < rCmp.Y() || ( Y() == rCmp.Y() && X() < rCmp.X() )
                    ? sal_True : sal_False;
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

const SwLayoutFrm *lcl_FindCellFrm( const SwLayoutFrm *pLay )
{
    while ( pLay && !pLay->IsCellFrm() )
        pLay = pLay->GetUpper();
    return pLay;
}

const SwLayoutFrm *lcl_FindNextCellFrm( const SwLayoutFrm *pLay )
{
    // ensure we leave the cell (sections)
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
        rBoxes.insert( (SwSelBoxes&)rShell.GetTableCrsr()->GetBoxes() );
}

void GetTblSelCrs( const SwTableCursor& rTblCrsr, SwSelBoxes& rBoxes )
{
    rBoxes.clear();

    if( rTblCrsr.IsChgd() || !rTblCrsr.GetBoxesCount() )
    {
        SwTableCursor* pTCrsr = (SwTableCursor*)&rTblCrsr;
        pTCrsr->GetDoc()->GetCurrentLayout()->MakeTblCrsrs( *pTCrsr );  //swmod 080218
    }

    if( rTblCrsr.GetBoxesCount() )
        rBoxes.insert( (SwSelBoxes&)rTblCrsr.GetBoxes() );
}

void GetTblSel( const SwCrsrShell& rShell, SwSelBoxes& rBoxes,
                const SwTblSearchType eSearchType )
{
    // get start and end cell
    if ( !rShell.IsTableMode() )
        rShell.GetCrsr();

    GetTblSel( *rShell.getShellCrsr(false), rBoxes, eSearchType );
}

void GetTblSel( const SwCursor& rCrsr, SwSelBoxes& rBoxes,
                const SwTblSearchType eSearchType )
{
    // get start and end cell
    OSL_ENSURE( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( sal_False ),
            "Tabselection not on Cnt." );

    // Row-selection:
    // Check for complex tables. If Yes, search selected boxes via
    // the layout. Otherwise via the table structure (for macros !!)
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

        const SwNode* pMarkNode = rCrsr.GetNode( sal_False );
        const sal_uLong nMarkSectionStart = pMarkNode->StartOfSectionIndex();
        const SwTableBox* pMarkBox = rTbl.GetTblBox( nMarkSectionStart );

        OSL_ENSURE( pMarkBox, "Point in table, mark outside?" );

        const SwTableLine* pLine = pMarkBox ? pMarkBox->GetUpper() : 0;
        sal_uInt16 nSttPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nSttPos, "Where is my row in the table?" );
        pLine = rTbl.GetTblBox( rCrsr.GetNode( sal_True )->StartOfSectionIndex() )->GetUpper();
        sal_uInt16 nEndPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nEndPos, "Where is my row in the table?" );
        // pb: #i20193# if tableintable then nSttPos == nEndPos == USHRT_MAX
        if ( nSttPos != USHRT_MAX && nEndPos != USHRT_MAX )
        {
            if( nEndPos < nSttPos )     // exchange
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
                    // check for cell protection??
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
        pCntNd = rCrsr.GetCntntNode(sal_False);
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

    sal_Bool bTblIsValid;
    // #i55421# Reduced value 10
    int nLoopMax = 10;
    sal_uInt16 i;

    do {
        bTblIsValid = sal_True;

        // First, compute tables and rectangles
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

        // Now find boxes for each entry and emit
        for( i = 0; i < aUnions.size() && bTblIsValid; ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();
            if( !pTable->IsValid() && nLoopMax )
            {
                bTblIsValid = sal_False;
                break;
            }

            // Skip any repeated headlines in the follow:
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                     (const SwLayoutFrm*)pTable->Lower();

            while( pRow && bTblIsValid )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTblIsValid = sal_False;
                    break;
                }

                if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( bTblIsValid && pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->IsValid() && nLoopMax )
                        {
                            bTblIsValid = sal_False;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                        if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwTableBox* pBox = (SwTableBox*)
                                ((SwCellFrm*)pCell)->GetTabBox();
                            // check for cell protection??
                            if( !bChkProtected ||
                                !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                                rBoxes.insert( pBox );

                            if ( pCells )
                            {
                                const Point aTopLeft( pCell->Frm().TopLeft() );
                                const Point aTopRight( pCell->Frm().TopRight() );
                                const Point aBottomLeft( pCell->Frm().BottomLeft() );
                                const Point aBottomRight( pCell->Frm().BottomRight() );

                                if ( aTopLeft.Y() < aCurrentTopLeft.Y() ||
                                     ( aTopLeft.Y() == aCurrentTopLeft.Y() &&
                                       aTopLeft.X() <  aCurrentTopLeft.X() ) )
                                {
                                    aCurrentTopLeft = aTopLeft;
                                    pCurrentTopLeftFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aTopRight.Y() < aCurrentTopRight.Y() ||
                                     ( aTopRight.Y() == aCurrentTopRight.Y() &&
                                       aTopRight.X() >  aCurrentTopRight.X() ) )
                                {
                                    aCurrentTopRight = aTopRight;
                                    pCurrentTopRightFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aBottomLeft.Y() > aCurrentBottomLeft.Y() ||
                                     ( aBottomLeft.Y() == aCurrentBottomLeft.Y() &&
                                       aBottomLeft.X() <  aCurrentBottomLeft.X() ) )
                                {
                                    aCurrentBottomLeft = aBottomLeft;
                                    pCurrentBottomLeftFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aBottomRight.Y() > aCurrentBottomRight.Y() ||
                                     ( aBottomRight.Y() == aCurrentBottomRight.Y() &&
                                       aBottomRight.X() >  aCurrentBottomRight.X() ) )
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

        // otherwise quickly "calculate" the table layout and start over
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

        // --> Make code robust, check if pStart has
        // been deleted due to the formatting of the table:
        if ( aDelCheck.HasBeenDeleted() )
        {
            OSL_FAIL( "Current box has been deleted during GetTblSel()" );
            break;
        }

        i = 0;
        rBoxes.erase( rBoxes.begin() + i, rBoxes.end() );
        --nLoopMax;

    } while( sal_True );
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
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, sal_False, sal_False );

    // if table is invisible, return
    // (layout needed for forming table selection further down, so we can't
    //  continue with invisible tables)
    // #i22135# - Also the content of the table could be
    //                          invisible - e.g. in a hidden section
    // Robust: check, if content was found (e.g. empty table cells)
    if ( !pCNd || pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() ) == NULL )
            return sal_False;

    const SwLayoutFrm *pStart = pCNd ? pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aNullPos )->GetUpper() : 0;
    OSL_ENSURE( pStart, "without frame nothing works" );

    aIdx = rEndNd;
    pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, sal_False, sal_False );

    // #i22135# - Robust: check, if content was found and if it's visible
    if ( !pCNd || pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() ) == NULL )
    {
        return sal_False;
    }

    const SwLayoutFrm *pEnd = pCNd ? pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aNullPos )->GetUpper() : 0;
    OSL_ENSURE( pEnd, "without frame nothing works" );


    sal_Bool bTblIsValid, bValidChartSel;
    // #i55421# Reduced value 10
    int nLoopMax = 10;      //JP 28.06.99: max 100 loops - Bug 67292
    sal_uInt16 i = 0;

    do {
        bTblIsValid = sal_True;
        bValidChartSel = sal_True;

        sal_uInt16 nRowCells = USHRT_MAX;

        // First, compute tables and rectangles
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_NO_UNION_CORRECT );

        // find boxes for each entry and emit
        for( i = 0; i < aUnions.size() && bTblIsValid &&
                                    bValidChartSel; ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            SWRECTFN( pTable )
            sal_Bool bRTL = pTable->IsRightToLeft();

            if( !pTable->IsValid() && nLoopMax  )
            {
                bTblIsValid = sal_False;
                break;
            }

            _Sort_CellFrms aCellFrms;

            // Skip any repeated headlines in the follow:
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      (const SwLayoutFrm*)pTable->Lower();

            while( pRow && bTblIsValid && bValidChartSel )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTblIsValid = sal_False;
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
                            bTblIsValid = sal_False;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                        const SwRect& rUnion = pUnion->GetUnion(),
                                    & rFrmRect = pCell->Frm();

                        const long nUnionRight = rUnion.Right();
                        const long nUnionBottom = rUnion.Bottom();
                        const long nFrmRight = rFrmRect.Right();
                        const long nFrmBottom = rFrmRect.Bottom();

                        // ignore if FrmRect is outside the union

                        const long nXFuzzy = bVert ? 0 : 20;
                        const long nYFuzzy = bVert ? 20 : 0;

                        if( !(  rUnion.Top()  + nYFuzzy > nFrmBottom ||
                                nUnionBottom < rFrmRect.Top() + nYFuzzy ||
                                rUnion.Left() + nXFuzzy > nFrmRight ||
                                nUnionRight < rFrmRect.Left() + nXFuzzy ))
                        {
                            // ok, rUnion is _not_ completely outside of rFrmRect

                            // if not completely inside the union, then
                            // for Chart it is an invalid selection
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
                                bValidChartSel = sal_False;
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

            // all cells of the (part) table together. Now check if
            // they're all adjacent
            size_t n, nCellCnt = 0;
            long nYPos = LONG_MAX;
            long nXPos = 0;
            long nHeight = 0;

            for( n = 0 ; n < aCellFrms.size(); ++n )
            {
                const _Sort_CellFrm& rCF = aCellFrms[ n ];
                if( (rCF.pFrm->Frm().*fnRect->fnGetTop)() != nYPos )
                {
                    // new row
                    if( n )
                    {
                        if( USHRT_MAX == nRowCells )        // 1. row change
                            nRowCells = nCellCnt;
                        else if( nRowCells != nCellCnt )
                        {
                            bValidChartSel = sal_False;
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
                    bValidChartSel = sal_False;
                    break;
                }
            }
            if( bValidChartSel )
            {
                if( USHRT_MAX == nRowCells )
                    nRowCells = nCellCnt;
                else if( nRowCells != nCellCnt )
                    bValidChartSel = sal_False;
            }
        }

        if( bTblIsValid )
            break;

        // otherwise quickly "calculate" table layout and start over
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
    } while( sal_True );

    OSL_ENSURE( nLoopMax, "table layout is still invalid!" );

    return bValidChartSel;
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
    SwShellCrsr* pCrsr = rShell.pCurCrsr;
    if ( rShell.IsTableMode() )
        pCrsr = rShell.pTblCrsr;

    const SwLayoutFrm *pStart = pCrsr->GetCntntNode()->getLayoutFrm( rShell.GetLayout(),
                      &pCrsr->GetPtPos() )->GetUpper(),
                      *pEnd   = pCrsr->GetCntntNode(sal_False)->getLayoutFrm( rShell.GetLayout(),
                      &pCrsr->GetMkPos() )->GetUpper();

    const SwLayoutFrm* pSttCell = pStart;
    while( pSttCell && !pSttCell->IsCellFrm() )
        pSttCell = pSttCell->GetUpper();

    // First, compute tables and rectangles
    SwSelUnions aUnions;

    // by default, first test above and then to the left
    ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_COL );

    sal_Bool bTstRow = sal_True, bFound = sal_False;
    sal_uInt16 i;

    // 1. check if box above contains value/formula
    for( i = 0; i < aUnions.size(); ++i )
    {
        SwSelUnion *pUnion = &aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        // Skip any repeated headlines in the follow:
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

                        // all boxes together, do not check the
                        // row, if a formula or value was found
                        bTstRow = 0 == nWhichId || USHRT_MAX == nWhichId;
                        bFound = sal_True;
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


    // 2. check if box on left contains value/formula
    if( bTstRow )
    {
        bFound = sal_False;

        rBoxes.clear();
        aUnions.clear();
        ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_ROW );

        for( i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            // Skip any repeated headlines in the follow:
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

                            // all boxes together, do not check the
                            // row if a formula or value was found
                            bFound = 0 != nWhichId && USHRT_MAX != nWhichId;
                            bTstRow = sal_False;
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

    return bFound;
}

sal_Bool HasProtectedCells( const SwSelBoxes& rBoxes )
{
    sal_Bool bRet = sal_False;
    for( sal_uInt16 n = 0, nCnt = rBoxes.size(); n < nCnt; ++n )
        if( rBoxes[ n ]->GetFrmFmt()->GetProtect().IsCntntProtected() )
        {
            bRet = sal_True;
            break;
        }
    return bRet;
}


_CmpLPt::_CmpLPt( const Point& rPt, const SwTableBox* pBox, sal_Bool bVertical )
    : aPos( rPt ), pSelBox( pBox ), bVert( bVertical )
{}

void lcl_InsTblBox( SwTableNode* pTblNd, SwDoc* pDoc, SwTableBox* pBox,
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
        // now check for paragraph bound flyes
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

    OSL_ENSURE( rPam.GetCntntNode() && rPam.GetCntntNode( sal_False ),
            "Tabselection not on Cnt." );

//JP 24.09.96:  Merge with repeating TableHeadLines does not work properly.
//              Why not use point 0,0? Then it is assured the first
//              headline is contained.
    Point aPt( 0, 0 );

    const SwCntntNode* pCntNd = rPam.GetCntntNode();
    const SwLayoutFrm *pStart = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                                        &aPt )->GetUpper();
    pCntNd = rPam.GetCntntNode(sal_False);
    const SwLayoutFrm *pEnd = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                                        &aPt )->GetUpper();

    // First, compute tables and rectangles
    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );
    if( aUnions.empty() )
        return;

    const SwTable *pTable = aUnions.front().GetTable()->GetTable();
    SwDoc* pDoc = (SwDoc*)pStart->GetFmt()->GetDoc();
    SwTableNode* pTblNd = (SwTableNode*)pTable->GetTabSortBoxes()[ 0 ]->
                                        GetSttNd()->FindTableNode();

    _MergePos aPosArr;      // Sort-Array with the frame positions
    long nWidth;
    SwTableBox* pLastBox = 0;

    SWRECTFN( pStart->GetUpper() )

    for ( sal_uInt16 i = 0; i < aUnions.size(); ++i )
    {
        const SwTabFrm *pTabFrm = aUnions[i].GetTable();

        SwRect &rUnion = aUnions[i].GetUnion();

        // Skip any repeated headlines in the follow:
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
                    // overlap in full width?
                    if( rUnion.Top() <= pCell->Frm().Top() &&
                        rUnion.Bottom() >= pCell->Frm().Bottom() )
                    {
                        SwTableBox* pBox =(SwTableBox*)((SwCellFrm*)pCell)->GetTabBox();

                        // only overlap to the right?
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
                                // this box is selected
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
                                // this box is selected
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
                        // overlapping on left- or right-side
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
                                        .GetItemState( RES_BOX, sal_False, &pItem ))
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

                            // this box is selected
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
                       // is right side of box part of the selected area?
                        else if( ( pCell->Frm().Right() - COLFUZZY ) < rUnion.Right() &&
                                 ( pCell->Frm().Right() - COLFUZZY ) > rUnion.Left() &&
                                 ( pCell->Frm().Left() + COLFUZZY ) < rUnion.Left() )
                        {
                            // then we should insert a new box and adjust the widths
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

                                // this box is selected
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
                        // --> Check if table cell is not empty
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

    // no SSelection / no boxes found
    if( 1 >= rBoxes.size() )
        return;

    // now search all horizontally adjacent boxes and connect
    // their contents with blanks. All vertically adjacent will be tied
    // together as paragraphs

    // 1. Solution: map array and all on same Y-level
    //      are separated with blanks
    //      all others are separated with paragraphs
    sal_Bool bCalcWidth = sal_True;
    const SwTableBox* pFirstBox = aPosArr[ 0 ].pSelBox;

    // JP 27.03.98:  Optimise - if boxes on one row are empty,
    //              then do not insert blanks or carriage returns
    //Block to assure SwPaM, SwPosition are deleted from stack
    {
        SwPaM aPam( pDoc->GetNodes() );

#if defined( DEL_ONLY_EMPTY_LINES )
        nWidth = pFirstBox->GetFrmFmt()->GetFrmSize().GetWidth();
        sal_Bool bEmptyLine = sal_True;
        sal_uInt16 n, nSttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // same Y level?
            {
                if( bEmptyLine && !IsEmptyBox( *rPt.pSelBox, aPam ))
                    bEmptyLine = sal_False;
                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = sal_False;     // one line is ready

                if( bEmptyLine && nSttPos < n )
                {
                    // now complete line is empty and should not
                    // be filled with blanks and be inserted as paragraph
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
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // same Y level?
            {
                sal_Bool bEmptyBox = IsEmptyBox( *rPt.pSelBox, aPam );
                if( bEmptyBox )
                {
                    if( nSEndPos == n )     // beginning is empty
                        nESttPos = ++nSEndPos;
                }
                else                        // end could be empty
                    nESttPos = n+1;

                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = sal_False;     // one line ready

                // first those at the beginning
                if( nSttPos < nSEndPos )
                {
                    // now the beginning of the line is empty and should
                    // not be filled with blanks
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
                    // now the beginning of the line is empty and should
                    // not be filled with blanks
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

        // first those at the beginning
        if( nSttPos < nSEndPos )
        {
            // now the beginning of the line is empty and should
            // not be filled with blanks
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
            // now the beginning of the line is empty and should
            // not be filled with blanks
            if( pUndo )
                for( sal_uInt16 i = nESttPos; i < n; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

            sal_uInt16 nCnt = n - nESttPos;
            aPosArr.Remove( nESttPos, nCnt );
        }
#else
// DEL_ALL_EMPTY_BOXES

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
                if( nY == ( bVert ? rPt.X() : rPt.Y() ) ) // same Y level?
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
                else
                    bCalcWidth = sal_False;     // one line ready
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

    // first create new box
    {
        SwTableBox* pTmpBox = rBoxes[0];
        SwTableLine* pInsLine = pTmpBox->GetUpper();
        sal_uInt16 nInsPos = pInsLine->GetTabBoxes().GetPos( pTmpBox );

        lcl_InsTblBox( pTblNd, pDoc, pTmpBox, nInsPos );
        (*ppMergeBox) = pInsLine->GetTabBoxes()[ nInsPos ];
        pInsLine->GetTabBoxes().erase( pInsLine->GetTabBoxes().begin() + nInsPos );  // remove again
        (*ppMergeBox)->SetUpper( 0 );
        (*ppMergeBox)->ClaimFrmFmt();

        // define the border: the upper/left side of the first box,
        // the lower/right side of the last box:
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

    //Block to delete  SwPaM, SwPosition from stack
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
            sal_uInt16 nL = pCNd ? pCNd->Len() : 0;
            aPam.GetPoint()->nContent.Assign( pCNd, nL );

            SwNodeIndex aSttNdIdx( *rPt.pSelBox->GetSttNd(), 1 );
            // one node should be kept in the box (otherwise the
            // section would be deleted during a move)
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
            rInsPosNd++;
            if( pUndo )
                pUndo->MoveBoxCntnt( pDoc, aRg, rInsPosNd );
            else
            {
                pDoc->MoveNodeRange( aRg, rInsPosNd,
                    IDocumentContentOperations::DOC_MOVEDEFAULT );
            }
            // where is now aInsPos ??

            if( bCalcWidth )
                bCalcWidth = sal_False;     // one line is ready

            // skip the first TextNode
            rInsPosNd.Assign( pDoc->GetNodes(),
                            rInsPosNd.GetNode().EndOfSectionIndex() - 2 );
            pTxtNd = rInsPosNd.GetNode().GetTxtNode();
            if( pTxtNd )
                aInsPos.nContent.Assign( pTxtNd, pTxtNd->GetTxt().Len() );
        }

        // the MergeBox should contain the complete text
        // now erase the initial TextNode
        OSL_ENSURE( (*ppMergeBox)->GetSttIdx()+2 <
                (*ppMergeBox)->GetSttNd()->EndOfSectionIndex(),
                    "empty box" );
        SwNodeIndex aIdx( *(*ppMergeBox)->GetSttNd()->EndOfSectionNode(), -1 );
        pDoc->GetNodes().Delete( aIdx, 1 );
    }

    // set width of the box
    (*ppMergeBox)->GetFrmFmt()->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));
    if( pUndo )
        pUndo->AddNewBox( (*ppMergeBox)->GetSttIdx() );
}


static sal_Bool lcl_CheckCol(_FndBox const&, sal_Bool* pPara);

static sal_Bool lcl_CheckRow( const _FndLine& rFndLine, sal_Bool* pPara )
{
    for (_FndBoxes::const_iterator it = rFndLine.GetBoxes().begin();
         it != rFndLine.GetBoxes().end(); ++it)
    {
        lcl_CheckCol(*it, pPara);
    }
    return *pPara;
}

static sal_Bool lcl_CheckCol( _FndBox const& rFndBox, sal_Bool* pPara )
{
    if (!rFndBox.GetBox()->GetSttNd())
    {
        if (rFndBox.GetLines().size() !=
            rFndBox.GetBox()->GetTabLines().size())
        {
            *pPara = sal_False;
        }
        else
            BOOST_FOREACH( _FndLine const& rFndLine, rFndBox.GetLines() )
                lcl_CheckRow( rFndLine, pPara );
    }
    // is box protected ??
    else if (rFndBox.GetBox()->GetFrmFmt()->GetProtect().IsCntntProtected())
        *pPara = sal_False;
    return *pPara;
}


sal_uInt16 CheckMergeSel( const SwPaM& rPam )
{
    SwSelBoxes aBoxes;
//JP 24.09.96:  Merge with repeating TableHeadLines does not work properly.
//              Why not use point 0,0? Then it is assured the first
//              headline is contained.

    Point aPt;
    const SwCntntNode* pCntNd = rPam.GetCntntNode();
    const SwLayoutFrm *pStart = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                                        &aPt )->GetUpper();
    pCntNd = rPam.GetCntntNode(sal_False);
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
            sal_Bool bMergeSelOk = sal_True;
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
                BOOST_FOREACH( _FndLine& rFndLine, pFndBox->GetLines() )
                    lcl_CheckRow( rFndLine, &bMergeSelOk );
            else if( pFndLine )
                for (_FndBoxes::const_iterator it = pFndLine->GetBoxes().begin();
                     it != pFndLine->GetBoxes().end(); ++it)
                    lcl_CheckCol(*it, &bMergeSelOk);
            if( !bMergeSelOk )
                eRet = TBLMERGE_TOOCOMPLEX;
        }
        else
            eRet = TBLMERGE_NOSELECTION;
    }
    return eRet;
}

SwTwips lcl_CalcWish( const SwLayoutFrm *pCell, long nWish,
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

void lcl_FindStartEndRow( const SwLayoutFrm *&rpStart,
                             const SwLayoutFrm *&rpEnd,
                             const int bChkProtected )
{
    // Put Start at beginning of a row.
    // Put End at the end of its row.
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
            // first unequal line or box - all odds are
            if( n & 1 )                 // 1, 3, 5, ... are boxes
            {
                rpStart = aSttArr[ n ];
                rpEnd = aEndArr[ n ];
            }
            else                                // 0, 2, 4, ... are lines
            {
                // check if start & end line are the first & last Line of the
                // box. If not return these cells.
                // Else the hole line with all Boxes has to be deleted.
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

    if( !bChkProtected )    // protected cell ?
        return;


    // Beginning and end should not be in protected cells
    while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
        rpStart = (SwLayoutFrm*)rpStart->GetNext();
    while ( rpEnd->GetFmt()->GetProtect().IsCntntProtected() )
        rpEnd = (SwLayoutFrm*)rpEnd->GetPrev();
}


void lcl_FindStartEndCol( const SwLayoutFrm *&rpStart,
                             const SwLayoutFrm *&rpEnd,
                             const int bChkProtected )
{
    // Beginning and end vertical till the border of the table;
    // Consider the whole table, including master and follows.
    // In order to start we need the mother-tableFrm
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
        // Check if pTab->GetFollow() is a valid follow table:
        // Only follow tables with at least on non-FollowFlowLine
        // should be considered.
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
    // --> Made code robust. If pTab does not have a lower,
    // we would crash here.
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

    if( !bChkProtected )    // check for protected cell ?
        return;

    // Beginning and end should not be in protected cells.
    // If necessary we should search backwards again
    while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
    {
        const SwLayoutFrm *pTmpLeaf = rpStart;
        pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() > nEX ) // first skip line
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
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() < nEX )//erstmal die Zeile ueberspr.
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
    sal_Bool bExchange = sal_False;

    if ( pTable != pEndTable )
    {
        if ( !pTable->IsAnFollow( pEndTable ) )
        {
            OSL_ENSURE( pEndTable->IsAnFollow( pTable ), "Tabchain in knots." );
            bExchange = sal_True;
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
                bExchange = sal_True;
        }
        else if( bVert == ( nSttTop < nEndTop ) )
            bExchange = sal_True;
    }
    if ( bExchange )
    {
        const SwLayoutFrm *pTmp = pStart;
        pStart = pEnd;
        pEnd = pTmp;
        // do no resort pTable and pEndTable, set new below
        // MA: 28. Dec. 93 Bug: 5190
    }

    // Beginning and end now nicely sorted, if required we
    // should move them
    if( nsSwTblSearchType::TBLSEARCH_ROW == ((~nsSwTblSearchType::TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndRow( pStart, pEnd, nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType );
    else if( nsSwTblSearchType::TBLSEARCH_COL == ((~nsSwTblSearchType::TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndCol( pStart, pEnd, nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType );

    if ( !pEnd ) return; // Made code robust.

    // retrieve again, as they have been moved
    pTable = pStart->FindTabFrm();
    pEndTable = pEnd->FindTabFrm();

    const long nStSz = pStart->GetFmt()->GetFrmSize().GetWidth();
    const long nEdSz = pEnd->GetFmt()->GetFrmSize().GetWidth();
    const long nWish = Max( 1L, pTable->GetFmt()->GetFrmSize().GetWidth() );
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
            // Unfortunately the union contains rounding errors now, therefore
            // erroneous results could occur during split/merge.
            // To prevent these we will determine the first and last row
            // within the union and use their values for a new union
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      (const SwLayoutFrm*)pTable->Lower();

            while ( pRow && !pRow->Frm().IsOver( aUnion ) )
                pRow = (SwLayoutFrm*)pRow->GetNext();

            // #i31976#
            // A follow flow row may contain emtpy cells. These are not
            // considered by FirstCell(). Therefore we have to find
            // the first cell manually:
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

            if ( pFirst && pLast ) //Robust
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

    // Get start and end cell
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
    pCntNd = rCrsr.GetCntntNode(sal_False);
    const SwLayoutFrm *pEnd = pCntNd->getLayoutFrm( pCntNd->GetDoc()->GetCurrentLayout(),
                                &aMkPos )->GetUpper();

    SWRECTFN( pStart->GetUpper() )

    // First, compute tables and rectangles
    SwSelUnions aUnions;

    ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

    // now search boxes for each entry and emit
    for ( sal_uInt16 i = 0; i < aUnions.size(); ++i )
    {
        SwSelUnion *pUnion = &aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        // Skip any repeated headlines in the follow:
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

// -------------------------------------------------------------------
// These Classes copy the current table selections (rBoxes),
// into a new structure, retaining the table structure
// new: SS for targeted erasing/restoring of the layout

void lcl_InsertRow( SwTableLine &rLine, SwLayoutFrm *pUpper, SwFrm *pSibling )
{
    SwRowFrm *pRow = new SwRowFrm( rLine, pUpper );
    if ( pUpper->IsTabFrm() && ((SwTabFrm*)pUpper)->IsFollow() )
    {
        SwTabFrm* pTabFrm = (SwTabFrm*)pUpper;
        pTabFrm->FindMaster()->InvalidatePos(); //can absorb the line

        if ( pSibling && pTabFrm->IsInHeadline( *pSibling ) )
        {
            // Skip any repeated headlines in the follow:
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
    // Set pointers to lines before and after the area to process.
    // If the first/last lines are contained in the area, then the pointers
    // are 0. We first search for the positions of the first/last affected
    // lines in array of the SwTable. In order to use 0 for 'no line'
    // we adjust the positions by 1.

    sal_uInt16 nStPos = USHRT_MAX;
    sal_uInt16 nEndPos= 0;

    for ( sal_uInt16 i = 0; i < rBoxes.size(); ++i )
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
    // Set pointers to lines before and after the area to process.
    // If the first/last lines are contained in the area, then the pointers
    // are 0. The positions of the first/last affected lines in the array
    // of the SwTable are in FndBox. In order to use 0 for 'no line'
    // we adjust the positions by 1.

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

void _FndBox::DelFrms( SwTable &rTable )
{
    // All lines between pLineBefore and pLineBehind should be cut
    // from the layout and erased.
    // If this creates empty Follows we should destroy these.
    // If a master is destroyed, the follow should become master.
    // Always a TabFrm should remain.

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
                    sal_Bool bDel = sal_True;
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
                            // #i60340# Do not transfer the
                            // flag from pUp to pPrev. pUp may still have the
                            // flag set although there is not more follow flow
                            // line associated with pUp.
                            pPrev->SetFollowFlowLine( sal_False );
                        }
                        else if ( pFollow )
                            ::UnsetFollow( pFollow );

                        // A TableFrm should always remain!
                        if ( pPrev || pFollow )
                        {
                            // OD 26.08.2003 #i18103# - if table is in a section,
                            // lock the section, to avoid its delete.
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
                            bDel = sal_False; // Row goes to /dev/null.
                        }
                    }
                    if ( bDel )
                    {
                        SwFrm* pTabFrm = pFrm->GetUpper();
                        if ( pTabFrm->IsTabFrm() &&
                            !pFrm->GetNext() &&
                             ((SwTabFrm*)pTabFrm)->GetFollow() )
                        {
                            // We do not delete the follow flow line,
                            // this will be done automatically in the
                            // next turn.
                            ((SwTabFrm*)pTabFrm)->SetFollowFlowLine( sal_False );
                        }

                        pFrm->Cut();
                        delete pFrm;
                    }
                }
        }
    }
}

sal_Bool lcl_IsLineOfTblFrm( const SwTabFrm& rTable, const SwFrm& rChk )
{
    const SwTabFrm* pTblFrm = rChk.FindTabFrm();
    if( pTblFrm->IsFollow() )
        pTblFrm = pTblFrm->FindMaster( true );
    return &rTable == pTblFrm;
}

/*
 * lcl_UpdateRepeatedHeadlines
 */
void lcl_UpdateRepeatedHeadlines( SwTabFrm& rTabFrm, bool bCalcLowers )
{
    OSL_ENSURE( rTabFrm.IsFollow(), "lcl_UpdateRepeatedHeadlines called for non-follow tab" );

    // Delete remaining headlines:
    SwRowFrm* pLower = 0;
    while ( 0 != ( pLower = (SwRowFrm*)rTabFrm.Lower() ) && pLower->IsRepeatedHeadline() )
    {
        pLower->Cut();
        delete pLower;
    }

    // Insert fresh set of headlines:
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
    // All lines between pLineBefore and pLineBehind should be re-generated in layout.
    // And this for all instances of a table (for example in header/footer).
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
    // now big insert operation for all tables.
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
                            // #i53647# If !pLineBehind,
                            // IsInSplitTableRow() should be checked.
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
// ???? or is this the last Follow of the table ????
                pUpperFrm = pTable;

            for ( i = nStPos; (sal_uInt16)i <= nEndPos; ++i )
                ::lcl_InsertRow( *rTable.GetTabLines()[static_cast<sal_uInt16>(i)],
                                (SwLayoutFrm*)pUpperFrm, pSibling );
            if ( pUpperFrm->IsTabFrm() )
                ((SwTabFrm*)pUpperFrm)->SetCalcLowers();
        }
        else if ( rTable.GetRowsToRepeat() > 0 )
        {
            // Insert new headlines:
            lcl_UpdateRepeatedHeadlines( *pTable, true );
        }
    }
}

void _FndBox::MakeNewFrms( SwTable &rTable, const sal_uInt16 nNumber,
                                            const sal_Bool bBehind )
{
    // Create Frms for newly inserted lines
    // bBehind == sal_True:  before  pLineBehind
    //         == sal_False: after   pLineBefore
    const sal_uInt16 nBfPos = pLineBefore ?
        rTable.GetTabLines().GetPos( (const SwTableLine*&)pLineBefore ) :
        USHRT_MAX;
    const sal_uInt16 nBhPos = pLineBehind ?
        rTable.GetTabLines().GetPos( (const SwTableLine*&)pLineBehind ) :
        USHRT_MAX;

    //nNumber: how often did we insert
    //nCnt:    how many were inserted nNumber times

    const sal_uInt16 nCnt =
        ((nBhPos != USHRT_MAX ? nBhPos : rTable.GetTabLines().size()) -
         (nBfPos != USHRT_MAX ? nBfPos + 1 : 0)) / (nNumber + 1);

    // search the Master-TabFrm
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
                                // only consider row frames associated with pLineBehind:
                                pSibling->GetTabLine() != pLineBehind ||
                                // only consider row frames that are in pTables Master-Follow chain:
                                !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                                // only consider row frames that are not repeated headlines:
                                pSibling->IsRepeatedHeadline() ||
                                // only consider row frames that are not follow flow rows
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
            else // insert before
            {
                sal_uInt16 i;

                // We are looking for the frame that is behind the row frame
                // that should be inserted.
                for ( i = 0; !pSibling; ++i )
                {
                    SwTableLine* pLine = pLineBefore ? pLineBefore : rTable.GetTabLines()[i];

                    SwIterator<SwRowFrm,SwFmt> aIter( *pLine->GetFrmFmt() );
                    pSibling = aIter.First();

                    while ( pSibling && (
                            // only consider row frames associated with pLineBefore:
                            pSibling->GetTabLine() != pLine ||
                            // only consider row frames that are in pTables Master-Follow chain:
                            !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                            // only consider row frames that are not repeated headlines:
                            pSibling->IsRepeatedHeadline() ||
                            // 1. case: pLineBefore == 0:
                            // only consider row frames that are not follow flow rows
                            // 2. case: pLineBefore != 0:
                            // only consider row frames that are not split table rows
                            // #i37476# If !pLineBefore,
                            // check IsInFollowFlowRow instead of IsInSplitTableRow.
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

    // If necessary headlines should be processed. In order to
    // not to fragment good code, we interate once more.
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
    // Should we call MakeFrms here?

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

    if ( nBfPos == nBhPos ) // Should never occur.
    {
        OSL_FAIL( "Table, erase but not on any area !?!" );
        return sal_False;
    }

    if ( rTable.GetRowsToRepeat() > 0 )
    {
        // oops: should the repeated headline have been deleted??
        SwIterator<SwTabFrm,SwFmt> aIter( *rTable.GetFrmFmt() );
        for( SwTabFrm* pTable = aIter.First(); pTable; pTable = aIter.Next() )
        {
            if( pTable->IsFollow() )
            {
                // Insert new headlines:
                lcl_UpdateRepeatedHeadlines( *pTable, false );
            }
        }
    }

    // Some adjacent lines at the beginning of the table have been deleted:
    if ( nBfPos == USHRT_MAX && nBhPos == 0 )
        return sal_False;

    // Some adjacent lines at the end of the table have been deleted:
    if ( nBhPos == USHRT_MAX && nBfPos == (rTable.GetTabLines().size() - 1) )
        return sal_False;

    // Some adjacent lines in the middle of the table have been deleted:
    if ( nBfPos != USHRT_MAX && nBhPos != USHRT_MAX && (nBfPos + 1) == nBhPos )
        return sal_False;

    // The structure of the deleted lines is more complex due to split lines.
    // A call of MakeFrms() is necessary.
    return sal_True;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
