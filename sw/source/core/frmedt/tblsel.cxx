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
#include <IDocumentLayoutAccess.hxx>
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
#include <calbck.hxx>
#include <deque>

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

struct _CmpLPt
{
    Point aPos;
    const SwTableBox* pSelBox;
    bool bVert;

    _CmpLPt( const Point& rPt, const SwTableBox* pBox, bool bVertical );

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

    explicit _Sort_CellFrm( const SwCellFrm& rCFrm )
        : pFrm( &rCFrm ) {}
};

static const SwLayoutFrm *lcl_FindCellFrm( const SwLayoutFrm *pLay )
{
    while ( pLay && !pLay->IsCellFrm() )
        pLay = pLay->GetUpper();
    return pLay;
}

static const SwLayoutFrm *lcl_FindNextCellFrm( const SwLayoutFrm *pLay )
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

void GetTableSelCrs( const SwCrsrShell &rShell, SwSelBoxes& rBoxes )
{
    rBoxes.clear();
    if( rShell.IsTableMode() && const_cast<SwCrsrShell&>(rShell).UpdateTableSelBoxes())
    {
        rBoxes.insert(rShell.GetTableCrsr()->GetSelectedBoxes());
    }
}

void GetTableSelCrs( const SwTableCursor& rTableCrsr, SwSelBoxes& rBoxes )
{
    rBoxes.clear();

    if (rTableCrsr.IsChgd() || !rTableCrsr.GetSelectedBoxesCount())
    {
        SwTableCursor* pTCrsr = const_cast<SwTableCursor*>(&rTableCrsr);
        pTCrsr->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()->MakeTableCrsrs( *pTCrsr );
    }

    if (rTableCrsr.GetSelectedBoxesCount())
    {
        rBoxes.insert(rTableCrsr.GetSelectedBoxes());
    }
}

void GetTableSel( const SwCrsrShell& rShell, SwSelBoxes& rBoxes,
                const SwTableSearchType eSearchType )
{
    // get start and end cell
    if ( !rShell.IsTableMode() )
        rShell.GetCrsr();

    GetTableSel( *rShell.getShellCrsr(false), rBoxes, eSearchType );
}

void GetTableSel( const SwCursor& rCrsr, SwSelBoxes& rBoxes,
                const SwTableSearchType eSearchType )
{
    // get start and end cell
    OSL_ENSURE( rCrsr.GetContentNode() && rCrsr.GetContentNode( false ),
            "Tabselection not on Cnt." );

    // Row-selection:
    // Check for complex tables. If Yes, search selected boxes via
    // the layout. Otherwise via the table structure (for macros !!)
    const SwContentNode* pContentNd = rCrsr.GetNode().GetContentNode();
    const SwTableNode* pTableNd = pContentNd ? pContentNd->FindTableNode() : 0;
    if( pTableNd && pTableNd->GetTable().IsNewModel() )
    {
        SwTable::SearchType eSearch;
        switch( nsSwTableSearchType::TBLSEARCH_COL & eSearchType )
        {
            case nsSwTableSearchType::TBLSEARCH_ROW: eSearch = SwTable::SEARCH_ROW; break;
            case nsSwTableSearchType::TBLSEARCH_COL: eSearch = SwTable::SEARCH_COL; break;
            default: eSearch = SwTable::SEARCH_NONE; break;
        }
        const bool bChkP = 0 != ( nsSwTableSearchType::TBLSEARCH_PROTECT & eSearchType );
        pTableNd->GetTable().CreateSelection( rCrsr, rBoxes, eSearch, bChkP );
        return;
    }
    if( nsSwTableSearchType::TBLSEARCH_ROW == ((~nsSwTableSearchType::TBLSEARCH_PROTECT ) & eSearchType ) &&
        pTableNd && !pTableNd->GetTable().IsTableComplex() )
    {
        const SwTable& rTable = pTableNd->GetTable();
        const SwTableLines& rLines = rTable.GetTabLines();

        const SwNode& rMarkNode = rCrsr.GetNode( false );
        const sal_uLong nMarkSectionStart = rMarkNode.StartOfSectionIndex();
        const SwTableBox* pMarkBox = rTable.GetTableBox( nMarkSectionStart );

        OSL_ENSURE( pMarkBox, "Point in table, mark outside?" );

        const SwTableLine* pLine = pMarkBox ? pMarkBox->GetUpper() : 0;
        sal_uInt16 nSttPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nSttPos, "Where is my row in the table?" );
        pLine = rTable.GetTableBox( rCrsr.GetNode().StartOfSectionIndex() )->GetUpper();
        sal_uInt16 nEndPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nEndPos, "Where is my row in the table?" );
        // pb: #i20193# if tableintable then nSttPos == nEndPos == USHRT_MAX
        if ( nSttPos != USHRT_MAX && nEndPos != USHRT_MAX )
        {
            if( nEndPos < nSttPos )     // exchange
            {
                sal_uInt16 nTmp = nSttPos; nSttPos = nEndPos; nEndPos = nTmp;
            }

            int bChkProtected = nsSwTableSearchType::TBLSEARCH_PROTECT & eSearchType;
            for( ; nSttPos <= nEndPos; ++nSttPos )
            {
                pLine = rLines[ nSttPos ];
                for( auto n = pLine->GetTabBoxes().size(); n ; )
                {
                    SwTableBox* pBox = pLine->GetTabBoxes()[ --n ];
                    // check for cell protection??
                    if( !bChkProtected ||
                        !pBox->GetFrameFormat()->GetProtect().IsContentProtected() )
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
        const SwContentNode *pCntNd = rCrsr.GetContentNode();
        const SwLayoutFrm *pStart = pCntNd ?
            pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPtPos )->GetUpper() : 0;
        pCntNd = rCrsr.GetContentNode(false);
        const SwLayoutFrm *pEnd = pCntNd ?
            pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aMkPos )->GetUpper() : 0;
        if( pStart && pEnd )
            GetTableSel( pStart, pEnd, rBoxes, 0, eSearchType );
    }
}

void GetTableSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
                SwSelBoxes& rBoxes, SwCellFrms* pCells,
                const SwTableSearchType eSearchType )
{
    const SwTabFrm* pStartTab = pStart->FindTabFrm();
    if ( !pStartTab )
    {
        OSL_FAIL( "GetTableSel without start table" );
        return;
    }

    int bChkProtected = nsSwTableSearchType::TBLSEARCH_PROTECT & eSearchType;

    // #i55421# Reduced value 10
    int nLoopMax = 10;

    do {
        bool bTableIsValid = true;

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
        for (size_t i = 0; i < aUnions.size() && bTableIsValid; ++i)
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();
            if( !pTable->IsValid() && nLoopMax )
            {
                bTableIsValid = false;
                break;
            }

            // Skip any repeated headlines in the follow:
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      static_cast<const SwLayoutFrm*>(pTable->Lower());

            while( pRow && bTableIsValid )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTableIsValid = false;
                    break;
                }

                if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( bTableIsValid && pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->IsValid() && nLoopMax )
                        {
                            bTableIsValid = false;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                        if( ::IsFrmInTableSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwTableBox* pBox = const_cast<SwTableBox*>(
                                static_cast<const SwCellFrm*>(pCell)->GetTabBox());
                            // check for cell protection??
                            if( !bChkProtected ||
                                !pBox->GetFrameFormat()->GetProtect().IsContentProtected() )
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
                            pCell = static_cast<const SwLayoutFrm*>(pCell->GetNext());
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                pRow = static_cast<const SwLayoutFrm*>(pRow->GetNext());
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

        if( bTableIsValid )
            break;

        SwDeletionChecker aDelCheck( pStart );

        // otherwise quickly "calculate" the table layout and start over
        SwTabFrm *pTable = aUnions.front().GetTable();
        while( pTable )
        {
            if( pTable->IsValid() )
                pTable->InvalidatePos();
            pTable->SetONECalcLowers();
            pTable->Calc(pTable->getRootFrm()->GetCurrShell()->GetOut());
            pTable->SetCompletePaint();
            if( 0 == (pTable = pTable->GetFollow()) )
                break;
        }

        // --> Make code robust, check if pStart has
        // been deleted due to the formatting of the table:
        if ( aDelCheck.HasBeenDeleted() )
        {
            OSL_FAIL( "Current box has been deleted during GetTableSel()" );
            break;
        }

        rBoxes.clear();
        --nLoopMax;

    } while( true );
    OSL_ENSURE( nLoopMax, "Table layout is still invalid!" );
}

bool ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd )
{
    const SwTableNode* pTNd = rSttNd.FindTableNode();
    if( !pTNd )
        return false;

    Point aNullPos;
    SwNodeIndex aIdx( rSttNd );
    const SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, false, false );

    // if table is invisible, return
    // (layout needed for forming table selection further down, so we can't
    //  continue with invisible tables)
    // #i22135# - Also the content of the table could be
    //                          invisible - e.g. in a hidden section
    // Robust: check, if content was found (e.g. empty table cells)
    if ( !pCNd || pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() ) == NULL )
            return false;

    const SwLayoutFrm *pStart = pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aNullPos )->GetUpper();
    OSL_ENSURE( pStart, "without frame nothing works" );

    aIdx = rEndNd;
    pCNd = aIdx.GetNode().GetContentNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, false, false );

    // #i22135# - Robust: check, if content was found and if it's visible
    if ( !pCNd || pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() ) == NULL )
    {
        return false;
    }

    const SwLayoutFrm *pEnd = pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aNullPos )->GetUpper();
    OSL_ENSURE( pEnd, "without frame nothing works" );

    bool bValidChartSel;
    // #i55421# Reduced value 10
    int nLoopMax = 10;      //JP 28.06.99: max 100 loops - Bug 67292
    sal_uInt16 i = 0;

    do {
        bool bTableIsValid = true;
        bValidChartSel = true;

        sal_uInt16 nRowCells = USHRT_MAX;

        // First, compute tables and rectangles
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTableSearchType::TBLSEARCH_NO_UNION_CORRECT );

        // find boxes for each entry and emit
        for( auto & rSelUnion : aUnions )
        {
            if (!bTableIsValid || !bValidChartSel)
                break;

            SwSelUnion *pUnion = &rSelUnion;
            const SwTabFrm *pTable = pUnion->GetTable();

            SWRECTFN( pTable )
            bool bRTL = pTable->IsRightToLeft();

            if( !pTable->IsValid() && nLoopMax  )
            {
                bTableIsValid = false;
                break;
            }

            std::deque< _Sort_CellFrm > aCellFrms;

            // Skip any repeated headlines in the follow:
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      static_cast<const SwLayoutFrm*>(pTable->Lower());

            while( pRow && bTableIsValid && bValidChartSel )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTableIsValid = false;
                    break;
                }

                if( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( bValidChartSel && bTableIsValid && pCell &&
                            pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->IsValid() && nLoopMax  )
                        {
                            bTableIsValid = false;
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
                                        _Sort_CellFrm( *static_cast<const SwCellFrm*>(pCell)) );
                            else
                            {
                                bValidChartSel = false;
                                break;
                            }
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = static_cast<const SwLayoutFrm*>(pCell->GetNext());
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                pRow = static_cast<const SwLayoutFrm*>(pRow->GetNext());
            }

            if( !bValidChartSel )
                break;

            // all cells of the (part) table together. Now check if
            // they're all adjacent
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
                    // new row
                    if( n )
                    {
                        if( USHRT_MAX == nRowCells )        // 1. row change
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

        if( bTableIsValid )
            break;

        // otherwise quickly "calculate" table layout and start over
        SwTabFrm *pTable = aUnions.front().GetTable();
        for( i = 0; i < aUnions.size(); ++i )
        {
            if( pTable->IsValid() )
                pTable->InvalidatePos();
            pTable->SetONECalcLowers();
            pTable->Calc(pTable->getRootFrm()->GetCurrShell()->GetOut());
            pTable->SetCompletePaint();
            if( 0 == (pTable = pTable->GetFollow()) )
                break;
        }
        --nLoopMax;
    } while( true );

    OSL_ENSURE( nLoopMax, "table layout is still invalid!" );

    return bValidChartSel;
}

bool IsFrmInTableSel( const SwRect& rUnion, const SwFrm* pCell )
{
    OSL_ENSURE( pCell->IsCellFrm(), "Frame without Gazelle" );

    if( pCell->FindTabFrm()->IsVertical() )
        return   rUnion.Right() >= pCell->Frm().Right() &&
                 rUnion.Left() <= pCell->Frm().Left() &&
            (( rUnion.Top() <= pCell->Frm().Top()+20 &&
               rUnion.Bottom() > pCell->Frm().Top() ) ||
             ( rUnion.Top() >= pCell->Frm().Top() &&
               rUnion.Bottom() < pCell->Frm().Bottom() ));

    return
        rUnion.Top() <= pCell->Frm().Top() &&
        rUnion.Bottom() >= pCell->Frm().Bottom() &&

        (( rUnion.Left() <= pCell->Frm().Left()+20 &&
           rUnion.Right() > pCell->Frm().Left() ) ||

         ( rUnion.Left() >= pCell->Frm().Left() &&
           rUnion.Right() < pCell->Frm().Right() ));
}

bool GetAutoSumSel( const SwCrsrShell& rShell, SwCellFrms& rBoxes )
{
    SwShellCrsr* pCrsr = rShell.m_pCurCrsr;
    if ( rShell.IsTableMode() )
        pCrsr = rShell.m_pTableCrsr;

    const SwLayoutFrm *pStart = pCrsr->GetContentNode()->getLayoutFrm( rShell.GetLayout(),
                      &pCrsr->GetPtPos() )->GetUpper(),
                      *pEnd   = pCrsr->GetContentNode(false)->getLayoutFrm( rShell.GetLayout(),
                      &pCrsr->GetMkPos() )->GetUpper();

    const SwLayoutFrm* pSttCell = pStart;
    while( pSttCell && !pSttCell->IsCellFrm() )
        pSttCell = pSttCell->GetUpper();

    // First, compute tables and rectangles
    SwSelUnions aUnions;

    // by default, first test above and then to the left
    ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTableSearchType::TBLSEARCH_COL );

    bool bTstRow = true, bFound = false;
    sal_uInt16 i;

    // 1. check if box above contains value/formula
    for( i = 0; i < aUnions.size(); ++i )
    {
        SwSelUnion *pUnion = &aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                  pTable->GetFirstNonHeadlineRow() :
                                  static_cast<const SwLayoutFrm*>(pTable->Lower());

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
                        bFound = true;
                        break;
                    }

                    OSL_ENSURE( pCell->IsCellFrm(), "Frame without cell" );
                    if( ::IsFrmInTableSel( pUnion->GetUnion(), pCell ) )
                        pUpperCell = static_cast<const SwCellFrm*>(pCell);

                    if( pCell->GetNext() )
                    {
                        pCell = static_cast<const SwLayoutFrm*>(pCell->GetNext());
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
            pRow = static_cast<const SwLayoutFrm*>(pRow->GetNext());
        }
    }

    // 2. check if box on left contains value/formula
    if( bTstRow )
    {
        bFound = false;

        rBoxes.clear();
        aUnions.clear();
        ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTableSearchType::TBLSEARCH_ROW );

        for( i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            // Skip any repeated headlines in the follow:
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      static_cast<const SwLayoutFrm*>(pTable->Lower());

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
                            bTstRow = false;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame without cell" );
                        if( ::IsFrmInTableSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwCellFrm* pC = const_cast<SwCellFrm*>(static_cast<const SwCellFrm*>(pCell));
                            rBoxes.push_back( pC );
                        }
                        if( pCell->GetNext() )
                        {
                            pCell = static_cast<const SwLayoutFrm*>(pCell->GetNext());
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

                pRow = static_cast<const SwLayoutFrm*>(pRow->GetNext());
            }
        }
    }

    return bFound;
}

bool HasProtectedCells( const SwSelBoxes& rBoxes )
{
    bool bRet = false;
    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        if( rBoxes[ n ]->GetFrameFormat()->GetProtect().IsContentProtected() )
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

_CmpLPt::_CmpLPt( const Point& rPt, const SwTableBox* pBox, bool bVertical )
    : aPos( rPt ), pSelBox( pBox ), bVert( bVertical )
{}

static void lcl_InsTableBox( SwTableNode* pTableNd, SwDoc* pDoc, SwTableBox* pBox,
                        sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 )
{
    OSL_ENSURE( pBox->GetSttNd(), "Box without Start-Node" );
    SwContentNode* pCNd = pDoc->GetNodes()[ pBox->GetSttIdx() + 1 ]
                                ->GetContentNode();
    if( pCNd && pCNd->IsTextNode() )
        pDoc->GetNodes().InsBoxen( pTableNd, pBox->GetUpper(),
                static_cast<SwTableBoxFormat*>(pBox->GetFrameFormat()),
                static_cast<SwTextNode*>(pCNd)->GetTextColl(),
                pCNd->GetpSwAttrSet(),
                nInsPos, nCnt );
    else
        pDoc->GetNodes().InsBoxen( pTableNd, pBox->GetUpper(),
                static_cast<SwTableBoxFormat*>(pBox->GetFrameFormat()),
                pDoc->GetDfltTextFormatColl(), 0,
                nInsPos, nCnt );
}

bool IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam )
{
    rPam.GetPoint()->nNode = *rBox.GetSttNd()->EndOfSectionNode();
    rPam.Move( fnMoveBackward, fnGoContent );
    rPam.SetMark();
    rPam.GetPoint()->nNode = *rBox.GetSttNd();
    rPam.Move( fnMoveForward, fnGoContent );
    bool bRet = *rPam.GetMark() == *rPam.GetPoint()
        && ( rBox.GetSttNd()->GetIndex() + 1 == rPam.GetPoint()->nNode.GetIndex() );

    if( bRet )
    {
        // now check for paragraph bound flies
        const SwFrameFormats& rFormats = *rPam.GetDoc()->GetSpzFrameFormats();
        sal_uLong nSttIdx = rPam.GetPoint()->nNode.GetIndex(),
              nEndIdx = rBox.GetSttNd()->EndOfSectionIndex(),
              nIdx;

        for( auto pFormat : rFormats )
        {
            const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
            const SwPosition* pAPos = rAnchor.GetContentAnchor();
            if (pAPos &&
                ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                 (FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
                nSttIdx <= ( nIdx = pAPos->nNode.GetIndex() ) &&
                nIdx < nEndIdx )
            {
                bRet = false;
                break;
            }
        }
    }
    return bRet;
}

void GetMergeSel( const SwPaM& rPam, SwSelBoxes& rBoxes,
                SwTableBox** ppMergeBox, SwUndoTableMerge* pUndo )
{
    rBoxes.clear();

    OSL_ENSURE( rPam.GetContentNode() && rPam.GetContentNode( false ),
            "Tabselection not on Cnt." );

//JP 24.09.96:  Merge with repeating TableHeadLines does not work properly.
//              Why not use point 0,0? Then it is assured the first
//              headline is contained.
    Point aPt( 0, 0 );

    const SwContentNode* pCntNd = rPam.GetContentNode();
    const SwLayoutFrm *pStart = pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                        &aPt )->GetUpper();
    pCntNd = rPam.GetContentNode(false);
    const SwLayoutFrm *pEnd = pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                        &aPt )->GetUpper();

    // First, compute tables and rectangles
    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );
    if( aUnions.empty() )
        return;

    const SwTable *pTable = aUnions.front().GetTable()->GetTable();
    SwDoc* pDoc = const_cast<SwDoc*>(pStart->GetFormat()->GetDoc());
    SwTableNode* pTableNd = const_cast<SwTableNode*>(pTable->GetTabSortBoxes()[ 0 ]->
                                        GetSttNd()->FindTableNode());

    _MergePos aPosArr;      // Sort-Array with the frame positions
    long nWidth;
    SwTableBox* pLastBox = 0;

    SWRECTFN( pStart->GetUpper() )

    for ( auto & rSelUnion : aUnions )
    {
        const SwTabFrm *pTabFrm = rSelUnion.GetTable();

        SwRect &rUnion = rSelUnion.GetUnion();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrm* pRow = pTabFrm->IsFollow() ?
                                  pTabFrm->GetFirstNonHeadlineRow() :
                                  static_cast<const SwLayoutFrm*>(pTabFrm->Lower());

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
                        SwTableBox* pBox = const_cast<SwTableBox*>(static_cast<const SwCellFrm*>(pCell)->GetTabBox());

                        // only overlap to the right?
                        if( ( rUnion.Left() - COLFUZZY ) <= pCell->Frm().Left() &&
                            ( rUnion.Right() - COLFUZZY ) > pCell->Frm().Left() )
                        {
                            if( ( rUnion.Right() + COLFUZZY ) < pCell->Frm().Right() )
                            {
                                sal_uInt16 nInsPos = pBox->GetUpper()->
                                                    GetTabBoxes().GetPos( pBox )+1;
                                lcl_InsTableBox( pTableNd, pDoc, pBox, nInsPos );
                                pBox->ClaimFrameFormat();
                                SwFormatFrmSize aNew(
                                        pBox->GetFrameFormat()->GetFrmSize() );
                                nWidth = rUnion.Right() - pCell->Frm().Left();
                                nWidth = nWidth * aNew.GetWidth() /
                                         pCell->Frm().Width();
                                long nTmpWidth = aNew.GetWidth() - nWidth;
                                aNew.SetWidth( nWidth );
                                pBox->GetFrameFormat()->SetFormatAttr( aNew );
                                // this box is selected
                                pLastBox = pBox;
                                rBoxes.insert( pBox );
                                aPosArr.insert(
                                    _CmpLPt( (pCell->Frm().*fnRect->fnGetPos)(),
                                    pBox, bVert ) );

                                pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                                aNew.SetWidth( nTmpWidth );
                                pBox->ClaimFrameFormat();
                                pBox->GetFrameFormat()->SetFormatAttr( aNew );

                                if( pUndo )
                                    pUndo->AddNewBox( pBox->GetSttIdx() );
                            }
                            else
                            {
                                // this box is selected
                                pLastBox = pBox;
                                rBoxes.insert( pBox );
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
                            lcl_InsTableBox( pTableNd, pDoc, pBox, nInsPos, 2 );
                            pBox->ClaimFrameFormat();
                            SwFormatFrmSize aNew(
                                        pBox->GetFrameFormat()->GetFrmSize() );
                            long nLeft = rUnion.Left() - pCell->Frm().Left();
                            nLeft = nLeft * aNew.GetWidth() /
                                    pCell->Frm().Width();
                            long nRight = pCell->Frm().Right() - rUnion.Right();
                            nRight = nRight * aNew.GetWidth() /
                                     pCell->Frm().Width();
                            nWidth = aNew.GetWidth() - nLeft - nRight;

                            aNew.SetWidth( nLeft );
                            pBox->GetFrameFormat()->SetFormatAttr( aNew );

                            {
                            const SfxPoolItem* pItem;
                            if( SfxItemState::SET == pBox->GetFrameFormat()->GetAttrSet()
                                        .GetItemState( RES_BOX, false, &pItem ))
                            {
                                SvxBoxItem aBox( *static_cast<const SvxBoxItem*>(pItem) );
                                aBox.SetLine( 0, SvxBoxItemLine::RIGHT );
                                pBox->GetFrameFormat()->SetFormatAttr( aBox );
                            }
                            }

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                            aNew.SetWidth( nWidth );
                            pBox->ClaimFrameFormat();
                            pBox->GetFrameFormat()->SetFormatAttr( aNew );

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
                            pBox->ClaimFrameFormat();
                            pBox->GetFrameFormat()->SetFormatAttr( aNew );

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
                            lcl_InsTableBox( pTableNd, pDoc, pBox, nInsPos );

                            SwFormatFrmSize aNew(pBox->GetFrameFormat()->GetFrmSize() );
                            long nLeft = rUnion.Left() - pCell->Frm().Left(),
                                nRight = pCell->Frm().Right() - rUnion.Left();

                            nLeft = nLeft * aNew.GetWidth() /
                                    pCell->Frm().Width();
                            nRight = nRight * aNew.GetWidth() /
                                    pCell->Frm().Width();

                            aNew.SetWidth( nLeft );
                            pBox->ClaimFrameFormat()->SetFormatAttr( aNew );

                                // this box is selected
                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrameFormat();
                            pBox->GetFrameFormat()->SetFormatAttr( aNew );

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
                        pCell = static_cast<const SwLayoutFrm*>(pCell->GetNext());
                        // --> Check if table cell is not empty
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }
            }
            pRow = static_cast<const SwLayoutFrm*>(pRow->GetNext());
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
    bool bCalcWidth = true;
    const SwTableBox* pFirstBox = aPosArr[ 0 ].pSelBox;

    // JP 27.03.98:  Optimise - if boxes on one row are empty,
    //              then do not insert blanks or carriage returns
    //Block to assure SwPaM, SwPosition are deleted from stack
    {
        SwPaM aPam( pDoc->GetNodes() );

#if defined( DEL_ONLY_EMPTY_LINES )
        nWidth = pFirstBox->GetFrameFormat()->GetFrmSize().GetWidth();
        bool bEmptyLine = sal_True;
        sal_uInt16 n, nSttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // same Y level?
            {
                if( bEmptyLine && !IsEmptyBox( *rPt.pSelBox, aPam ))
                    bEmptyLine = sal_False;
                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrameFormat()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = false;     // one line is ready

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
#elif defined( DEL_EMPTY_BOXES_AT_START_AND_END )

        nWidth = pFirstBox->GetFrameFormat()->GetFrmSize().GetWidth();
        sal_uInt16 n, nSttPos = 0, nSEndPos = 0, nESttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // same Y level?
            {
                bool bEmptyBox = IsEmptyBox( *rPt.pSelBox, aPam );
                if( bEmptyBox )
                {
                    if( nSEndPos == n )     // beginning is empty
                        nESttPos = ++nSEndPos;
                }
                else                        // end could be empty
                    nESttPos = n+1;

                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrameFormat()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = false;     // one line ready

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

        for( _MergePos::size_type n = 0; n < aPosArr.size(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( bCalcWidth )
            {
                if( nY == ( bVert ? rPt.X() : rPt.Y() ) ) // same Y level?
                    nWidth += rPt.pSelBox->GetFrameFormat()->GetFrmSize().GetWidth();
                else
                    bCalcWidth = false;     // one line ready
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

        lcl_InsTableBox( pTableNd, pDoc, pTmpBox, nInsPos );
        (*ppMergeBox) = pInsLine->GetTabBoxes()[ nInsPos ];
        pInsLine->GetTabBoxes().erase( pInsLine->GetTabBoxes().begin() + nInsPos );  // remove again
        (*ppMergeBox)->SetUpper( 0 );
        (*ppMergeBox)->ClaimFrameFormat();

        // define the border: the upper/left side of the first box,
        // the lower/right side of the last box:
        if( pLastBox && pFirstBox )
        {
            SvxBoxItem aBox( pFirstBox->GetFrameFormat()->GetBox() );
            const SvxBoxItem& rBox = pLastBox->GetFrameFormat()->GetBox();
            aBox.SetLine( rBox.GetRight(), SvxBoxItemLine::RIGHT );
            aBox.SetLine( rBox.GetBottom(), SvxBoxItemLine::BOTTOM );
            if( aBox.GetLeft() || aBox.GetTop() ||
                aBox.GetRight() || aBox.GetBottom() )
                (*ppMergeBox)->GetFrameFormat()->SetFormatAttr( aBox );
        }
    }

    //Block to delete  SwPaM, SwPosition from stack
    if( !aPosArr.empty() )
    {
        SwPosition aInsPos( *(*ppMergeBox)->GetSttNd() );
        SwNodeIndex& rInsPosNd = aInsPos.nNode;

        SwPaM aPam( aInsPos );

        for( const auto &rPt : aPosArr )
        {
            aPam.GetPoint()->nNode.Assign( *rPt.pSelBox->GetSttNd()->
                                            EndOfSectionNode(), -1 );
            SwContentNode* pCNd = aPam.GetContentNode();
            aPam.GetPoint()->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );

            SwNodeIndex aSttNdIdx( *rPt.pSelBox->GetSttNd(), 1 );
            // one node should be kept in the box (otherwise the
            // section would be deleted during a move)
            bool const bUndo(pDoc->GetIDocumentUndoRedo().DoesUndo());
            if( pUndo )
            {
                pDoc->GetIDocumentUndoRedo().DoUndo(false);
            }
            pDoc->getIDocumentContentOperations().AppendTextNode( *aPam.GetPoint() );
            if( pUndo )
            {
                pDoc->GetIDocumentUndoRedo().DoUndo(bUndo);
            }
            SwNodeRange aRg( aSttNdIdx, aPam.GetPoint()->nNode );
            ++rInsPosNd;
            if( pUndo )
                pUndo->MoveBoxContent( pDoc, aRg, rInsPosNd );
            else
            {
                pDoc->getIDocumentContentOperations().MoveNodeRange( aRg, rInsPosNd,
                    SwMoveFlags::DEFAULT );
            }
            // where is now aInsPos ??

            if( bCalcWidth )
                bCalcWidth = false;     // one line is ready

            // skip the first TextNode
            rInsPosNd.Assign( pDoc->GetNodes(),
                            rInsPosNd.GetNode().EndOfSectionIndex() - 2 );
            SwTextNode* pTextNd = rInsPosNd.GetNode().GetTextNode();
            if( pTextNd )
                aInsPos.nContent.Assign(pTextNd, pTextNd->GetText().getLength());
        }

        // the MergeBox should contain the complete text
        // now erase the initial TextNode
        OSL_ENSURE( (*ppMergeBox)->GetSttIdx()+2 <
                (*ppMergeBox)->GetSttNd()->EndOfSectionIndex(),
                    "empty box" );
        SwNodeIndex aIdx( *(*ppMergeBox)->GetSttNd()->EndOfSectionNode(), -1 );
        pDoc->GetNodes().Delete( aIdx );
    }

    // set width of the box
    (*ppMergeBox)->GetFrameFormat()->SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE, nWidth, 0 ));
    if( pUndo )
        pUndo->AddNewBox( (*ppMergeBox)->GetSttIdx() );
}

static bool lcl_CheckCol(_FndBox const&, bool* pPara);

static bool lcl_CheckRow( const _FndLine& rFndLine, bool* pPara )
{
    for (auto const& it : rFndLine.GetBoxes())
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
            for (auto const& rpFndLine : rFndBox.GetLines())
            {
                lcl_CheckRow( *rpFndLine, pPara );
            }
        }
    }
    // is box protected ??
    else if (rFndBox.GetBox()->GetFrameFormat()->GetProtect().IsContentProtected())
        *pPara = false;
    return *pPara;
}

sal_uInt16 CheckMergeSel( const SwPaM& rPam )
{
    SwSelBoxes aBoxes;
//JP 24.09.96:  Merge with repeating TableHeadLines does not work properly.
//              Why not use point 0,0? Then it is assured the first
//              headline is contained.

    Point aPt;
    const SwContentNode* pCntNd = rPam.GetContentNode();
    const SwLayoutFrm *pStart = pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                        &aPt )->GetUpper();
    pCntNd = rPam.GetContentNode(false);
    const SwLayoutFrm *pEnd = pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                    &aPt )->GetUpper();
    GetTableSel( pStart, pEnd, aBoxes, 0 );
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
        const SwTableNode* pTableNd = aPara.rBoxes[0]->GetSttNd()->FindTableNode();
        ForEach_FndLineCopyCol( (SwTableLines&)pTableNd->GetTable().GetTabLines(), &aPara );
        if( !aFndBox.GetLines().empty() )
        {
            bool bMergeSelOk = true;
            _FndBox* pFndBox = &aFndBox;
            _FndLine* pFndLine = 0;
            while( pFndBox && 1 == pFndBox->GetLines().size() )
            {
                pFndLine = pFndBox->GetLines().front().get();
                if( 1 == pFndLine->GetBoxes().size() )
                    pFndBox = pFndLine->GetBoxes().front().get();
                else
                    pFndBox = 0;
            }
            if( pFndBox )
            {
                for (auto const& it : pFndBox->GetLines())
                {
                    lcl_CheckRow(*it, &bMergeSelOk);
                }
            }
            else if( pFndLine )
            {
                for (auto const& it : pFndLine->GetBoxes())
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

    const bool bRTL = pCell->IsRightToLeft();
    SwTwips nRet = bRTL ?
        nAct - pCell->Frm().Width() :
        0;

    while ( pTmp )
    {
        while ( pTmp->GetPrev() )
        {
            pTmp = static_cast<const SwLayoutFrm*>(pTmp->GetPrev());
            sal_Int64 nTmp = pTmp->GetFormat()->GetFrmSize().GetWidth();
            // multiply in 64-bit to avoid overflow here!
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
    // Put Start at beginning of a row.
    // Put End at the end of its row.
    rpStart = static_cast<const SwLayoutFrm*>(rpStart->GetUpper()->Lower());
    while ( rpEnd->GetNext() )
        rpEnd = static_cast<const SwLayoutFrm*>(rpEnd->GetNext());

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

    for( std::deque<const SwLayoutFrm *>::size_type n = 0; n < aEndArr.size() && n < aSttArr.size(); ++n )
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
                    const SwCellFrm* pCellFrm = static_cast<const SwCellFrm*>(aSttArr[ n-1 ]);
                    const SwTableLines& rLns = pCellFrm->
                                                GetTabBox()->GetTabLines();
                    if( rLns[ 0 ] == static_cast<const SwRowFrm*>(aSttArr[ n ])->GetTabLine() &&
                        rLns[ rLns.size() - 1 ] ==
                                    static_cast<const SwRowFrm*>(aEndArr[ n ])->GetTabLine() )
                    {
                        rpStart = rpEnd = pCellFrm;
                        while ( rpStart->GetPrev() )
                            rpStart = static_cast<const SwLayoutFrm*>(rpStart->GetPrev());
                        while ( rpEnd->GetNext() )
                            rpEnd = static_cast<const SwLayoutFrm*>(rpEnd->GetNext());
                    }
                }
            }
            break;
        }

    if( !bChkProtected )    // protected cell ?
        return;

    // Beginning and end should not be in protected cells
    while ( rpStart->GetFormat()->GetProtect().IsContentProtected() )
        rpStart = static_cast<const SwLayoutFrm*>(rpStart->GetNext());
    while ( rpEnd->GetFormat()->GetProtect().IsContentProtected() )
        rpEnd = static_cast<const SwLayoutFrm*>(rpEnd->GetPrev());
}

static void lcl_FindStartEndCol( const SwLayoutFrm *&rpStart,
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

    bool bRTL = pTab->IsRightToLeft();
    const long nTmpWish = pOrg->GetFormat()->GetFrmSize().GetWidth();
    const long nWish = ( nTmpWish > 0 ) ? nTmpWish : 1;

    while ( pTab->IsFollow() )
    {
        const SwFrm *pTmp = pTab->FindPrev();
        OSL_ENSURE( pTmp->IsTabFrm(), "Predecessor of Follow is not Master." );
        pTab = static_cast<const SwTabFrm*>(pTmp);
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
        nSX2 = nSX + (rpStart->GetFormat()->GetFrmSize().GetWidth() * nPrtWidth / nWish);
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

        // Check if pTab->GetFollow() is a valid follow table:
        // Only follow tables with at least on non-FollowFlowLine
        // should be considered.

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

    const SwContentFrm* pLastContent = pTab->FindLastContent();
    rpEnd = pLastContent ? pLastContent->GetUpper() : 0;
    // --> Made code robust. If pTab does not have a lower,
    // we would crash here.
    if ( !pLastContent ) return;

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
    while ( rpStart->GetFormat()->GetProtect().IsContentProtected() )
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
            while ( rpStart &&
                    (rpStart->Frm().*fnRect->fnGetLeft)() < nSX &&
                    (rpStart->Frm().*fnRect->fnGetRight)()< nSX2 )
                rpStart = rpStart->GetNextLayoutLeaf();
        }
        else
            rpStart = pTmpLeaf;
    }
    while ( rpEnd->GetFormat()->GetProtect().IsContentProtected() )
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
            pTmpTab = static_cast<const SwTabFrm*>(pTmpTab->FindPrev());
            OSL_ENSURE( pTmpTab->IsTabFrm(), "Predecessor of Follow not Master.");
            rpEnd = pTmpTab->FindLastContent()->GetUpper();
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
                    const SwLayoutFrm *pEnd, const SwTableSearchType eSearchType )
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
        // do no resort pTable and pEndTable, set new below
        // MA: 28. Dec. 93 Bug: 5190
    }

    // Beginning and end now nicely sorted, if required we
    // should move them
    if( nsSwTableSearchType::TBLSEARCH_ROW == ((~nsSwTableSearchType::TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndRow( pStart, pEnd, nsSwTableSearchType::TBLSEARCH_PROTECT & eSearchType );
    else if( nsSwTableSearchType::TBLSEARCH_COL == ((~nsSwTableSearchType::TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndCol( pStart, pEnd, nsSwTableSearchType::TBLSEARCH_PROTECT & eSearchType );

    if ( !pEnd || !pStart ) return; // Made code robust.

    // retrieve again, as they have been moved
    pTable = pStart->FindTabFrm();
    pEndTable = pEnd->FindTabFrm();

    const long nStSz = pStart->GetFormat()->GetFrmSize().GetWidth();
    const long nEdSz = pEnd->GetFormat()->GetFrmSize().GetWidth();
    const long nWish = std::max( 1L, pTable->GetFormat()->GetFrmSize().GetWidth() );
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

        if( !(nsSwTableSearchType::TBLSEARCH_NO_UNION_CORRECT & eSearchType ))
        {
            // Unfortunately the union contains rounding errors now, therefore
            // erroneous results could occur during split/merge.
            // To prevent these we will determine the first and last row
            // within the union and use their values for a new union
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      static_cast<const SwLayoutFrm*>(pTable->Lower());

            while ( pRow && !pRow->Frm().IsOver( aUnion ) )
                pRow = static_cast<const SwLayoutFrm*>(pRow->GetNext());

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

            while ( pFirst && !::IsFrmInTableSel( aUnion, pFirst ) )
            {
                if ( pFirst->GetNext() )
                {
                    pFirst = static_cast<const SwLayoutFrm*>(pFirst->GetNext());
                    if ( pFirst->Lower() && pFirst->Lower()->IsRowFrm() )
                        pFirst = pFirst->FirstCell();
                }
                else
                    pFirst = ::lcl_FindNextCellFrm( pFirst );
            }
            const SwLayoutFrm* pLast = 0;
            const SwFrm* pLastContent = pTable->FindLastContent();
            if ( pLastContent )
                pLast = ::lcl_FindCellFrm( pLastContent->GetUpper() );

            while ( pLast && !::IsFrmInTableSel( aUnion, pLast ) )
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
            rUnions.push_back(SwSelUnion(aUnion, const_cast<SwTabFrm*>(pTable)));
        }

        pTable = pTable->GetFollow();
        if ( pTable != pEndTable && pEndTable->IsAnFollow( pTable ) )
            pTable = 0;
    }
}

bool CheckSplitCells( const SwCrsrShell& rShell, sal_uInt16 nDiv,
                        const SwTableSearchType eSearchType )
{
    if( !rShell.IsTableMode() )
        rShell.GetCrsr();

    return CheckSplitCells( *rShell.getShellCrsr(false), nDiv, eSearchType );
}

bool CheckSplitCells( const SwCursor& rCrsr, sal_uInt16 nDiv,
                        const SwTableSearchType eSearchType )
{
    if( 1 >= nDiv )
        return false;

    sal_uInt16 nMinValue = nDiv * MINLAY;

    // Get start and end cell
    Point aPtPos, aMkPos;
    const SwShellCrsr* pShCrsr = dynamic_cast<const SwShellCrsr*>(&rCrsr);
    if( pShCrsr )
    {
        aPtPos = pShCrsr->GetPtPos();
        aMkPos = pShCrsr->GetMkPos();
    }

    const SwContentNode* pCntNd = rCrsr.GetContentNode();
    const SwLayoutFrm *pStart = pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                        &aPtPos )->GetUpper();
    pCntNd = rCrsr.GetContentNode(false);
    const SwLayoutFrm *pEnd = pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                &aMkPos )->GetUpper();

    SWRECTFN( pStart->GetUpper() )

    // First, compute tables and rectangles
    SwSelUnions aUnions;

    ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

    // now search boxes for each entry and emit
    for ( auto rSelUnion : aUnions )
    {
        const SwTabFrm *pTable = rSelUnion.GetTable();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                  pTable->GetFirstNonHeadlineRow() :
                                  static_cast<const SwLayoutFrm*>(pTable->Lower());

        while ( pRow )
        {
            if ( pRow->Frm().IsOver( rSelUnion.GetUnion() ) )
            {
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    OSL_ENSURE( pCell->IsCellFrm(), "Frame without cell" );
                    if( ::IsFrmInTableSel( rSelUnion.GetUnion(), pCell ) )
                    {
                        if( (pCell->Frm().*fnRect->fnGetWidth)() < nMinValue )
                            return false;
                    }

                    if ( pCell->GetNext() )
                    {
                        pCell = static_cast<const SwLayoutFrm*>(pCell->GetNext());
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }
            }
            pRow = static_cast<const SwLayoutFrm*>(pRow->GetNext());
        }
    }
    return true;
}

// These Classes copy the current table selections (rBoxes),
// into a new structure, retaining the table structure
// new: SS for targeted erasing/restoring of the layout

static void lcl_InsertRow( SwTableLine &rLine, SwLayoutFrm *pUpper, SwFrm *pSibling )
{
    SwRowFrm *pRow = new SwRowFrm( rLine, pUpper );
    if ( pUpper->IsTabFrm() && static_cast<SwTabFrm*>(pUpper)->IsFollow() )
    {
        SwTabFrm* pTabFrm = static_cast<SwTabFrm*>(pUpper);
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
    pFndPara->pFndLine->GetBoxes().push_back(std::unique_ptr<_FndBox>(pFndBox));
}

static void _FndLineCopyCol( SwTableLine* pLine, _FndPara* pFndPara )
{
    std::unique_ptr<_FndLine> pFndLine(new _FndLine(pLine, pFndPara->pFndBox));
    _FndPara aPara(*pFndPara, pFndLine.get());
    for( SwTableBoxes::iterator it = pFndLine->GetLine()->GetTabBoxes().begin();
             it != pFndLine->GetLine()->GetTabBoxes().end(); ++it)
        _FndBoxCopyCol(*it, &aPara );
    if( pFndLine->GetBoxes().size() )
    {
        pFndPara->pFndBox->GetLines().push_back( std::move(pFndLine) );
    }
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

    for (size_t i = 0; i < rBoxes.size(); ++i)
    {
        SwTableLine *pLine = rBoxes[i]->GetUpper();
        while ( pLine->GetUpper() )
            pLine = pLine->GetUpper()->GetUpper();
        const sal_uInt16 nPos = rTable.GetTabLines().GetPos(
                    const_cast<const SwTableLine*&>(pLine) ) + 1;

        OSL_ENSURE( nPos != USHRT_MAX, "TableLine not found." );

        if( nStPos > nPos )
            nStPos = nPos;

        if( nEndPos < nPos )
            nEndPos = nPos;
    }
    if (USHRT_MAX != nStPos && nStPos > 1)
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

    SwTableLine* pTmpLine = GetLines().front()->GetLine();
    sal_uInt16 nPos = rTable.GetTabLines().GetPos( pTmpLine );
    OSL_ENSURE( USHRT_MAX != nPos, "Line steht nicht in der Tabelle" );
    if( nPos )
        pLineBefore = rTable.GetTabLines()[ nPos - 1 ];

    pTmpLine = GetLines().back()->GetLine();
    nPos = rTable.GetTabLines().GetPos( pTmpLine );
    OSL_ENSURE( USHRT_MAX != nPos, "Line steht nicht in der Tabelle" );
    if( ++nPos < rTable.GetTabLines().size() )
        pLineBehind = rTable.GetTabLines()[nPos];
}

inline void UnsetFollow( SwFlowFrm *pTab )
{
    pTab->m_pPrecede = 0;
}

//When bAccTableDispose is FALSE,the acc table should not be disposed.
//void _FndBox::DelFrms( SwTable &rTable )
void _FndBox::DelFrms( SwTable &rTable, bool bAccTableDispose )
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
                        const_cast<const SwTableLine*&>(pLineBefore) );
        OSL_ENSURE( nStPos != USHRT_MAX, "The fox stole the line!" );
        ++nStPos;
    }
    if( rTable.IsNewModel() && pLineBehind )
        rTable.CheckRowSpan( pLineBehind, false );
    if ( pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        const_cast<const SwTableLine*&>(pLineBehind) );
        OSL_ENSURE( nEndPos != USHRT_MAX, "The fox stole the line!" );
        if (nEndPos != 0)
            --nEndPos;
    }

    for ( sal_uInt16 i = nStPos; i <= nEndPos; ++i)
    {
        SwFrameFormat *pFormat = rTable.GetTabLines()[i]->GetFrameFormat();
        SwIterator<SwRowFrm,SwFormat> aIter( *pFormat );
        for ( SwRowFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
        {
                if ( pFrm->GetTabLine() == rTable.GetTabLines()[i] )
                {
                    bool bDel = true;
                    SwTabFrm *pUp = !pFrm->GetPrev() && !pFrm->GetNext() ?
                                            static_cast<SwTabFrm*>(pFrm->GetUpper()) : 0;
                    if ( !pUp )
                    {
                        const sal_uInt16 nRepeat =
                                static_cast<SwTabFrm*>(pFrm->GetUpper())->GetTable()->GetRowsToRepeat();
                        if ( nRepeat > 0 &&
                             static_cast<SwTabFrm*>(pFrm->GetUpper())->IsFollow() )
                        {
                            if ( !pFrm->GetNext() )
                            {
                                SwRowFrm* pFirstNonHeadline =
                                    static_cast<SwTabFrm*>(pFrm->GetUpper())->GetFirstNonHeadlineRow();
                                if ( pFirstNonHeadline == pFrm )
                                {
                                    pUp = static_cast<SwTabFrm*>(pFrm->GetUpper());
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
                            pPrev = static_cast<SwTabFrm*>(pTmp);
                        }
                        if ( pPrev )
                        {
                            pPrev->SetFollow( pFollow );
                            // #i60340# Do not transfer the
                            // flag from pUp to pPrev. pUp may still have the
                            // flag set although there is not more follow flow
                            // line associated with pUp.
                            pPrev->SetFollowFlowLine( false );
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
                            SwFrm::DestroyFrm(pUp);
                            bDel = false; // Row goes to /dev/null.
                        }
                    }
                    if ( bDel )
                    {
                        SwFrm* pTabFrm = pFrm->GetUpper();
                        if ( pTabFrm->IsTabFrm() &&
                            !pFrm->GetNext() &&
                             static_cast<SwTabFrm*>(pTabFrm)->GetFollow() )
                        {
                            // We do not delete the follow flow line,
                            // this will be done automatically in the
                            // next turn.
                            static_cast<SwTabFrm*>(pTabFrm)->SetFollowFlowLine( false );
                        }
                        //Set acc table dispose state
                        pFrm->SetAccTableDispose( bAccTableDispose );
                        pFrm->Cut();
                        //Set acc table dispose state to default value.
                        pFrm->SetAccTableDispose( true );
                        SwFrm::DestroyFrm(pFrm);
                    }
                }
        }
    }
}

static bool lcl_IsLineOfTableFrm( const SwTabFrm& rTable, const SwFrm& rChk )
{
    const SwTabFrm* pTableFrm = rChk.FindTabFrm();
    if( pTableFrm->IsFollow() )
        pTableFrm = pTableFrm->FindMaster( true );
    return &rTable == pTableFrm;
}

static void lcl_UpdateRepeatedHeadlines( SwTabFrm& rTabFrm, bool bCalcLowers )
{
    OSL_ENSURE( rTabFrm.IsFollow(), "lcl_UpdateRepeatedHeadlines called for non-follow tab" );

    // Delete remaining headlines:
    SwRowFrm* pLower = 0;
    while ( 0 != ( pLower = static_cast<SwRowFrm*>(rTabFrm.Lower()) ) && pLower->IsRepeatedHeadline() )
    {
        pLower->Cut();
        SwFrm::DestroyFrm(pLower);
    }

    // Insert fresh set of headlines:
    pLower = static_cast<SwRowFrm*>(rTabFrm.Lower());
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
                        const_cast<const SwTableLine*&>(pLineBefore) );
        OSL_ENSURE( nStPos != USHRT_MAX, "Fox stole the line!" );
        ++nStPos;

    }
    if ( pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        const_cast<const SwTableLine*&>(pLineBehind) );
        OSL_ENSURE( nEndPos != USHRT_MAX, "Fox stole the line!" );
        --nEndPos;
    }
    // now big insert operation for all tables.
    SwIterator<SwTabFrm,SwFormat> aTabIter( *rTable.GetFrameFormat() );
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
                SwIterator<SwRowFrm,SwFormat> aIter( *pLine->GetFrameFormat() );
                pSibling = aIter.First();
                while ( pSibling && (
                            pSibling->GetTabLine() != pLine ||
                            !lcl_IsLineOfTableFrm( *pTable, *pSibling ) ||
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

            for ( sal_uInt16 j = nStPos; j <= nEndPos; ++j )
                ::lcl_InsertRow( *rTable.GetTabLines()[j],
                                static_cast<SwLayoutFrm*>(pUpperFrm), pSibling );
            if ( pUpperFrm->IsTabFrm() )
                static_cast<SwTabFrm*>(pUpperFrm)->SetCalcLowers();
        }
        else if ( rTable.GetRowsToRepeat() > 0 )
        {
            // Insert new headlines:
            lcl_UpdateRepeatedHeadlines( *pTable, true );
        }
    }
}

void _FndBox::MakeNewFrms( SwTable &rTable, const sal_uInt16 nNumber,
                                            const bool bBehind )
{
    // Create Frms for newly inserted lines
    // bBehind == true:  before  pLineBehind
    //         == false: after   pLineBefore
    const sal_uInt16 nBfPos = pLineBefore ?
        rTable.GetTabLines().GetPos( const_cast<const SwTableLine*&>(pLineBefore) ) :
        USHRT_MAX;
    const sal_uInt16 nBhPos = pLineBehind ?
        rTable.GetTabLines().GetPos( const_cast<const SwTableLine*&>(pLineBehind) ) :
        USHRT_MAX;

    //nNumber: how often did we insert
    //nCnt:    how many were inserted nNumber times

    const sal_uInt16 nCnt =
        ((nBhPos != USHRT_MAX ? nBhPos : rTable.GetTabLines().size()) -
         (nBfPos != USHRT_MAX ? nBfPos + 1 : 0)) / (nNumber + 1);

    // search the Master-TabFrm
    SwIterator<SwTabFrm,SwFormat> aTabIter( *rTable.GetFrameFormat() );
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
                    SwIterator<SwRowFrm,SwFormat> aIter( *pLineBehind->GetFrameFormat() );
                    pSibling = aIter.First();
                    while ( pSibling && (
                                // only consider row frames associated with pLineBehind:
                                pSibling->GetTabLine() != pLineBehind ||
                                // only consider row frames that are in pTables Master-Follow chain:
                                !lcl_IsLineOfTableFrm( *pTable, *pSibling ) ||
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
                    static_cast<SwTabFrm*>(pUpperFrm)->SetCalcLowers();
            }
            else // insert before
            {
                sal_uInt16 i;

                // We are looking for the frame that is behind the row frame
                // that should be inserted.
                for ( i = 0; !pSibling; ++i )
                {
                    SwTableLine* pLine = pLineBefore ? pLineBefore : rTable.GetTabLines()[i];

                    SwIterator<SwRowFrm,SwFormat> aIter( *pLine->GetFrameFormat() );
                    pSibling = aIter.First();

                    while ( pSibling && (
                            // only consider row frames associated with pLineBefore:
                            pSibling->GetTabLine() != pLine ||
                            // only consider row frames that are in pTables Master-Follow chain:
                            !lcl_IsLineOfTableFrm( *pTable, *pSibling ) ||
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
                    pSibling = static_cast<SwRowFrm*>( pSibling->GetNext() );

                sal_uInt16 nMax = nBhPos != USHRT_MAX ?
                                    nBhPos - nCnt :
                                    rTable.GetTabLines().size() - nCnt;

                i = nBfPos != USHRT_MAX ? nBfPos + 1 : 0;
                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i],
                                pUpperFrm, pSibling );
                if ( pUpperFrm->IsTabFrm() )
                    static_cast<SwTabFrm*>(pUpperFrm)->SetCalcLowers();
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

                OSL_ENSURE( static_cast<SwRowFrm*>(pTable->Lower())->GetTabLine() ==
                        rTable.GetTabLines()[0], "MakeNewFrms: Table corruption!" );
            }
        }
    }
}

bool _FndBox::AreLinesToRestore( const SwTable &rTable ) const
{
    // Should we call MakeFrms here?

    if ( !pLineBefore && !pLineBehind && rTable.GetTabLines().size() )
        return true;

    sal_uInt16 nBfPos;
    if(pLineBefore)
    {
        const SwTableLine* rLBefore = const_cast<const SwTableLine*>(pLineBefore);
        nBfPos = rTable.GetTabLines().GetPos( rLBefore );
    }
    else
        nBfPos = USHRT_MAX;

    sal_uInt16 nBhPos;
    if(pLineBehind)
    {
        const SwTableLine* rLBehind = const_cast<const SwTableLine*>(pLineBehind);
        nBhPos = rTable.GetTabLines().GetPos( rLBehind );
    }
    else
        nBhPos = USHRT_MAX;

    if ( nBfPos == nBhPos ) // Should never occur.
    {
        OSL_FAIL( "Table, erase but not on any area !?!" );
        return false;
    }

    if ( rTable.GetRowsToRepeat() > 0 )
    {
        // oops: should the repeated headline have been deleted??
        SwIterator<SwTabFrm,SwFormat> aIter( *rTable.GetFrameFormat() );
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
        return false;

    // Some adjacent lines at the end of the table have been deleted:
    if ( nBhPos == USHRT_MAX && nBfPos == (rTable.GetTabLines().size() - 1) )
        return false;

    // Some adjacent lines in the middle of the table have been deleted:
    if ( nBfPos != USHRT_MAX && nBhPos != USHRT_MAX && (nBfPos + 1) == nBhPos )
        return false;

    // The structure of the deleted lines is more complex due to split lines.
    // A call of MakeFrms() is necessary.
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
