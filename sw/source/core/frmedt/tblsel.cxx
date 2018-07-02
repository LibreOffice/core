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
#include <memory>

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

struct CmpLPt
{
    Point aPos;
    const SwTableBox* pSelBox;
    bool bVert;

    CmpLPt( const Point& rPt, const SwTableBox* pBox, bool bVertical );

    bool operator<( const CmpLPt& rCmp ) const
    {
        if ( bVert )
            return X() > rCmp.X() || ( X() == rCmp.X() && Y() < rCmp.Y() );
        else
            return Y() < rCmp.Y() || ( Y() == rCmp.Y() && X() < rCmp.X() );
    }

    long X() const { return aPos.X(); }
    long Y() const { return aPos.Y(); }
};

typedef o3tl::sorted_vector<CmpLPt> MergePos;


struct Sort_CellFrame
{
    const SwCellFrame* pFrame;

    explicit Sort_CellFrame( const SwCellFrame& rCFrame )
        : pFrame( &rCFrame ) {}
};

static const SwLayoutFrame *lcl_FindCellFrame( const SwLayoutFrame *pLay )
{
    while ( pLay && !pLay->IsCellFrame() )
        pLay = pLay->GetUpper();
    return pLay;
}

static const SwLayoutFrame *lcl_FindNextCellFrame( const SwLayoutFrame *pLay )
{
    // ensure we leave the cell (sections)
    const SwLayoutFrame *pTmp = pLay;
    do {
        pTmp = pTmp->GetNextLayoutLeaf();
    } while( pLay->IsAnLower( pTmp ) );

    while( pTmp && !pTmp->IsCellFrame() )
        pTmp = pTmp->GetUpper();
    return pTmp;
}

void GetTableSelCrs( const SwCursorShell &rShell, SwSelBoxes& rBoxes )
{
    rBoxes.clear();
    if( rShell.IsTableMode() && const_cast<SwCursorShell&>(rShell).UpdateTableSelBoxes())
    {
        rBoxes.insert(rShell.GetTableCursor()->GetSelectedBoxes());
    }
}

void GetTableSelCrs( const SwTableCursor& rTableCursor, SwSelBoxes& rBoxes )
{
    rBoxes.clear();

    if (rTableCursor.IsChgd() || !rTableCursor.GetSelectedBoxesCount())
    {
        SwTableCursor* pTCursor = const_cast<SwTableCursor*>(&rTableCursor);
        pTCursor->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()->MakeTableCursors( *pTCursor );
    }

    if (rTableCursor.GetSelectedBoxesCount())
    {
        rBoxes.insert(rTableCursor.GetSelectedBoxes());
    }
}

void GetTableSel( const SwCursorShell& rShell, SwSelBoxes& rBoxes,
                const SwTableSearchType eSearchType )
{
    // get start and end cell
    if ( !rShell.IsTableMode() )
        rShell.GetCursor();

    GetTableSel( *rShell.getShellCursor(false), rBoxes, eSearchType );
}

void GetTableSel( const SwCursor& rCursor, SwSelBoxes& rBoxes,
                const SwTableSearchType eSearchType )
{
    // get start and end cell
    OSL_ENSURE( rCursor.GetContentNode() && rCursor.GetContentNode( false ),
            "Tabselection not on Cnt." );

    // Row-selection:
    // Check for complex tables. If Yes, search selected boxes via
    // the layout. Otherwise via the table structure (for macros !!)
    const SwContentNode* pContentNd = rCursor.GetNode().GetContentNode();
    const SwTableNode* pTableNd = pContentNd ? pContentNd->FindTableNode() : nullptr;
    if( pTableNd && pTableNd->GetTable().IsNewModel() )
    {
        SwTable::SearchType eSearch;
        switch( SwTableSearchType::Col & eSearchType )
        {
            case SwTableSearchType::Row: eSearch = SwTable::SEARCH_ROW; break;
            case SwTableSearchType::Col: eSearch = SwTable::SEARCH_COL; break;
            default: eSearch = SwTable::SEARCH_NONE; break;
        }
        const bool bChkP( SwTableSearchType::Protect & eSearchType );
        pTableNd->GetTable().CreateSelection( rCursor, rBoxes, eSearch, bChkP );
        return;
    }
    if( SwTableSearchType::Row == ((~SwTableSearchType::Protect ) & eSearchType ) &&
        pTableNd && !pTableNd->GetTable().IsTableComplex() )
    {
        const SwTable& rTable = pTableNd->GetTable();
        const SwTableLines& rLines = rTable.GetTabLines();

        const SwNode& rMarkNode = rCursor.GetNode( false );
        const sal_uLong nMarkSectionStart = rMarkNode.StartOfSectionIndex();
        const SwTableBox* pMarkBox = rTable.GetTableBox( nMarkSectionStart );

        OSL_ENSURE( pMarkBox, "Point in table, mark outside?" );

        const SwTableLine* pLine = pMarkBox ? pMarkBox->GetUpper() : nullptr;
        sal_uInt16 nSttPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nSttPos, "Where is my row in the table?" );
        pLine = rTable.GetTableBox( rCursor.GetNode().StartOfSectionIndex() )->GetUpper();
        sal_uInt16 nEndPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nEndPos, "Where is my row in the table?" );
        // pb: #i20193# if tableintable then nSttPos == nEndPos == USHRT_MAX
        if ( nSttPos != USHRT_MAX && nEndPos != USHRT_MAX )
        {
            if( nEndPos < nSttPos )     // exchange
            {
                sal_uInt16 nTmp = nSttPos; nSttPos = nEndPos; nEndPos = nTmp;
            }

            bool bChkProtected( SwTableSearchType::Protect & eSearchType );
            for( ; nSttPos <= nEndPos; ++nSttPos )
            {
                pLine = rLines[ nSttPos ];
                for( auto n = pLine->GetTabBoxes().size(); n ; )
                {
                    SwTableBox* pBox = pLine->GetTabBoxes()[ --n ].get();
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
        const SwShellCursor* pShCursor = dynamic_cast<const SwShellCursor*>(&rCursor);
        if( pShCursor )
        {
            aPtPos = pShCursor->GetPtPos();
            aMkPos = pShCursor->GetMkPos();
        }
        const SwContentNode *pCntNd = rCursor.GetContentNode();
        const SwLayoutFrame *pStart = pCntNd ?
            pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPtPos )->GetUpper() : nullptr;
        pCntNd = rCursor.GetContentNode(false);
        const SwLayoutFrame *pEnd = pCntNd ?
            pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aMkPos )->GetUpper() : nullptr;
        if( pStart && pEnd )
            GetTableSel( pStart, pEnd, rBoxes, nullptr, eSearchType );
    }
}

void GetTableSel( const SwLayoutFrame* pStart, const SwLayoutFrame* pEnd,
                SwSelBoxes& rBoxes, SwCellFrames* pCells,
                const SwTableSearchType eSearchType )
{
    const SwTabFrame* pStartTab = pStart->FindTabFrame();
    if ( !pStartTab )
    {
        OSL_FAIL( "GetTableSel without start table" );
        return;
    }

    bool bChkProtected( SwTableSearchType::Protect & eSearchType );

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
        const SwCellFrame* pCurrentTopLeftFrame     = nullptr;
        const SwCellFrame* pCurrentTopRightFrame    = nullptr;
        const SwCellFrame* pCurrentBottomLeftFrame  = nullptr;
        const SwCellFrame* pCurrentBottomRightFrame  = nullptr;

        // Now find boxes for each entry and emit
        for (size_t i = 0; i < aUnions.size() && bTableIsValid; ++i)
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrame *pTable = pUnion->GetTable();

            if( !pTable->isFrameAreaDefinitionValid() && nLoopMax )
            {
                bTableIsValid = false;
                break;
            }

            // Skip any repeated headlines in the follow:
            const SwLayoutFrame* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      static_cast<const SwLayoutFrame*>(pTable->Lower());

            while( pRow && bTableIsValid )
            {
                if( !pRow->isFrameAreaDefinitionValid() && nLoopMax )
                {
                    bTableIsValid = false;
                    break;
                }

                if ( pRow->getFrameArea().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrame *pCell = pRow->FirstCell();

                    while( bTableIsValid && pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->isFrameAreaDefinitionValid() && nLoopMax )
                        {
                            bTableIsValid = false;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrame(), "Frame without Cell" );
                        if( ::IsFrameInTableSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwTableBox* pBox = const_cast<SwTableBox*>(
                                static_cast<const SwCellFrame*>(pCell)->GetTabBox());
                            // check for cell protection??
                            if( !bChkProtected ||
                                !pBox->GetFrameFormat()->GetProtect().IsContentProtected() )
                                rBoxes.insert( pBox );

                            if ( pCells )
                            {
                                const Point aTopLeft( pCell->getFrameArea().TopLeft() );
                                const Point aTopRight( pCell->getFrameArea().TopRight() );
                                const Point aBottomLeft( pCell->getFrameArea().BottomLeft() );
                                const Point aBottomRight( pCell->getFrameArea().BottomRight() );

                                if ( aTopLeft.getY() < aCurrentTopLeft.getY() ||
                                     ( aTopLeft.getY() == aCurrentTopLeft.getY() &&
                                       aTopLeft.getX() <  aCurrentTopLeft.getX() ) )
                                {
                                    aCurrentTopLeft = aTopLeft;
                                    pCurrentTopLeftFrame = static_cast<const SwCellFrame*>( pCell );
                                }

                                if ( aTopRight.getY() < aCurrentTopRight.getY() ||
                                     ( aTopRight.getY() == aCurrentTopRight.getY() &&
                                       aTopRight.getX() >  aCurrentTopRight.getX() ) )
                                {
                                    aCurrentTopRight = aTopRight;
                                    pCurrentTopRightFrame = static_cast<const SwCellFrame*>( pCell );
                                }

                                if ( aBottomLeft.getY() > aCurrentBottomLeft.getY() ||
                                     ( aBottomLeft.getY() == aCurrentBottomLeft.getY() &&
                                       aBottomLeft.getX() <  aCurrentBottomLeft.getX() ) )
                                {
                                    aCurrentBottomLeft = aBottomLeft;
                                    pCurrentBottomLeftFrame = static_cast<const SwCellFrame*>( pCell );
                                }

                                if ( aBottomRight.getY() > aCurrentBottomRight.getY() ||
                                     ( aBottomRight.getY() == aCurrentBottomRight.getY() &&
                                       aBottomRight.getX() >  aCurrentBottomRight.getX() ) )
                                {
                                    aCurrentBottomRight = aBottomRight;
                                    pCurrentBottomRightFrame = static_cast<const SwCellFrame*>( pCell );
                                }

                            }
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = static_cast<const SwLayoutFrame*>(pCell->GetNext());
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrame() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrame( pCell );
                    }
                }
                pRow = static_cast<const SwLayoutFrame*>(pRow->GetNext());
            }
        }

        if ( pCells )
        {
            pCells->clear();
            pCells->push_back( const_cast< SwCellFrame* >(pCurrentTopLeftFrame) );
            pCells->push_back( const_cast< SwCellFrame* >(pCurrentTopRightFrame) );
            pCells->push_back( const_cast< SwCellFrame* >(pCurrentBottomLeftFrame) );
            pCells->push_back( const_cast< SwCellFrame* >(pCurrentBottomRightFrame) );
        }

        if( bTableIsValid )
            break;

        SwDeletionChecker aDelCheck( pStart );

        // otherwise quickly "calculate" the table layout and start over
        SwTabFrame *pTable = aUnions.front().GetTable();
        while( pTable )
        {
            if( pTable->isFrameAreaDefinitionValid() )
            {
                pTable->InvalidatePos();
            }

            pTable->SetONECalcLowers();
            pTable->Calc(pTable->getRootFrame()->GetCurrShell()->GetOut());
            pTable->SetCompletePaint();

            if( nullptr == (pTable = pTable->GetFollow()) )
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
    if ( !pCNd || pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() ) == nullptr )
            return false;

    const SwLayoutFrame *pStart = pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aNullPos )->GetUpper();
    OSL_ENSURE( pStart, "without frame nothing works" );

    aIdx = rEndNd;
    pCNd = aIdx.GetNode().GetContentNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, false, false );

    // #i22135# - Robust: check, if content was found and if it's visible
    if ( !pCNd || pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() ) == nullptr )
    {
        return false;
    }

    const SwLayoutFrame *pEnd = pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aNullPos )->GetUpper();
    OSL_ENSURE( pEnd, "without frame nothing works" );

    bool bValidChartSel;
    // #i55421# Reduced value 10
    int nLoopMax = 10;      //JP 28.06.99: max 100 loops - Bug 67292

    do {
        bool bTableIsValid = true;
        bValidChartSel = true;

        sal_uInt16 nRowCells = USHRT_MAX;

        // First, compute tables and rectangles
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, SwTableSearchType::NoUnionCorrect );

        // find boxes for each entry and emit
        for( auto & rSelUnion : aUnions )
        {
            if (!bTableIsValid || !bValidChartSel)
                break;

            SwSelUnion *pUnion = &rSelUnion;
            const SwTabFrame *pTable = pUnion->GetTable();

            SwRectFnSet aRectFnSet(pTable);
            bool bRTL = pTable->IsRightToLeft();

            if( !pTable->isFrameAreaDefinitionValid() && nLoopMax  )
            {
                bTableIsValid = false;
                break;
            }

            std::deque< Sort_CellFrame > aCellFrames;

            // Skip any repeated headlines in the follow:
            const SwLayoutFrame* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      static_cast<const SwLayoutFrame*>(pTable->Lower());

            while( pRow && bTableIsValid && bValidChartSel )
            {
                if( !pRow->isFrameAreaDefinitionValid() && nLoopMax )
                {
                    bTableIsValid = false;
                    break;
                }

                if( pRow->getFrameArea().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrame *pCell = pRow->FirstCell();

                    while( bValidChartSel && bTableIsValid && pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->isFrameAreaDefinitionValid() && nLoopMax  )
                        {
                            bTableIsValid = false;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrame(), "Frame without Cell" );
                        const SwRect& rUnion = pUnion->GetUnion(),
                                    & rFrameRect = pCell->getFrameArea();

                        const long nUnionRight = rUnion.Right();
                        const long nUnionBottom = rUnion.Bottom();
                        const long nFrameRight = rFrameRect.Right();
                        const long nFrameBottom = rFrameRect.Bottom();

                        // ignore if FrameRect is outside the union

                        const long nXFuzzy = aRectFnSet.IsVert() ? 0 : 20;
                        const long nYFuzzy = aRectFnSet.IsVert() ? 20 : 0;

                        if( !(  rUnion.Top()  + nYFuzzy > nFrameBottom ||
                                nUnionBottom < rFrameRect.Top() + nYFuzzy ||
                                rUnion.Left() + nXFuzzy > nFrameRight ||
                                nUnionRight < rFrameRect.Left() + nXFuzzy ))
                        {
                            // ok, rUnion is _not_ completely outside of rFrameRect

                            // if not completely inside the union, then
                            // for Chart it is an invalid selection
                            if( rUnion.Left()   <= rFrameRect.Left() + nXFuzzy &&
                                rFrameRect.Left() <= nUnionRight &&
                                rUnion.Left()   <= nFrameRight &&
                                nFrameRight       <= nUnionRight + nXFuzzy &&
                                rUnion.Top()    <= rFrameRect.Top() + nYFuzzy &&
                                rFrameRect.Top()  <= nUnionBottom &&
                                rUnion.Top()    <= nFrameBottom &&
                                nFrameBottom      <= nUnionBottom+ nYFuzzy )

                                aCellFrames.emplace_back( *static_cast<const SwCellFrame*>(pCell) );
                            else
                            {
                                bValidChartSel = false;
                                break;
                            }
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = static_cast<const SwLayoutFrame*>(pCell->GetNext());
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrame() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrame( pCell );
                    }
                }
                pRow = static_cast<const SwLayoutFrame*>(pRow->GetNext());
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

            for( n = 0 ; n < aCellFrames.size(); ++n )
            {
                const Sort_CellFrame& rCF = aCellFrames[ n ];
                if( aRectFnSet.GetTop(rCF.pFrame->getFrameArea()) != nYPos )
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
                    nYPos = aRectFnSet.GetTop(rCF.pFrame->getFrameArea());
                    nHeight = aRectFnSet.GetHeight(rCF.pFrame->getFrameArea());

                    nXPos = bRTL ?
                            aRectFnSet.GetLeft(rCF.pFrame->getFrameArea()) :
                            aRectFnSet.GetRight(rCF.pFrame->getFrameArea());
                }
                else if( nXPos == ( bRTL ?
                                    aRectFnSet.GetRight(rCF.pFrame->getFrameArea()) :
                                    aRectFnSet.GetLeft(rCF.pFrame->getFrameArea()) ) &&
                         nHeight == aRectFnSet.GetHeight(rCF.pFrame->getFrameArea()) )
                {
                    nXPos += ( bRTL ? -1 : 1 ) *
                             aRectFnSet.GetWidth(rCF.pFrame->getFrameArea());
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
        SwTabFrame *pTable = aUnions.front().GetTable();

        for( SwSelUnions::size_type i = 0; i < aUnions.size(); ++i )
        {
            if( pTable->isFrameAreaDefinitionValid() )
            {
                pTable->InvalidatePos();
            }

            pTable->SetONECalcLowers();
            pTable->Calc(pTable->getRootFrame()->GetCurrShell()->GetOut());
            pTable->SetCompletePaint();

            if( nullptr == (pTable = pTable->GetFollow()) )
                break;
        }
        --nLoopMax;
    } while( true );

    OSL_ENSURE( nLoopMax, "table layout is still invalid!" );

    return bValidChartSel;
}

bool IsFrameInTableSel( const SwRect& rUnion, const SwFrame* pCell )
{
    OSL_ENSURE( pCell->IsCellFrame(), "Frame without Gazelle" );

    if( pCell->FindTabFrame()->IsVertical() )
        return   rUnion.Right() >= pCell->getFrameArea().Right() &&
                 rUnion.Left() <= pCell->getFrameArea().Left() &&
            (( rUnion.Top() <= pCell->getFrameArea().Top()+20 &&
               rUnion.Bottom() > pCell->getFrameArea().Top() ) ||
             ( rUnion.Top() >= pCell->getFrameArea().Top() &&
               rUnion.Bottom() < pCell->getFrameArea().Bottom() ));

    return
        rUnion.Top() <= pCell->getFrameArea().Top() &&
        rUnion.Bottom() >= pCell->getFrameArea().Bottom() &&

        (( rUnion.Left() <= pCell->getFrameArea().Left()+20 &&
           rUnion.Right() > pCell->getFrameArea().Left() ) ||

         ( rUnion.Left() >= pCell->getFrameArea().Left() &&
           rUnion.Right() < pCell->getFrameArea().Right() ));
}

bool GetAutoSumSel( const SwCursorShell& rShell, SwCellFrames& rBoxes )
{
    SwShellCursor* pCursor = rShell.m_pCurrentCursor;
    if ( rShell.IsTableMode() )
        pCursor = rShell.m_pTableCursor;

    const SwLayoutFrame *pStart = pCursor->GetContentNode()->getLayoutFrame( rShell.GetLayout(),
                      &pCursor->GetPtPos() )->GetUpper(),
                      *pEnd   = pCursor->GetContentNode(false)->getLayoutFrame( rShell.GetLayout(),
                      &pCursor->GetMkPos() )->GetUpper();

    const SwLayoutFrame* pSttCell = pStart;
    while( pSttCell && !pSttCell->IsCellFrame() )
        pSttCell = pSttCell->GetUpper();

    // First, compute tables and rectangles
    SwSelUnions aUnions;

    // by default, first test above and then to the left
    ::MakeSelUnions( aUnions, pStart, pEnd, SwTableSearchType::Col );

    bool bTstRow = true, bFound = false;

    // 1. check if box above contains value/formula
    for( SwSelUnions::size_type i = 0; i < aUnions.size(); ++i )
    {
        SwSelUnion *pUnion = &aUnions[i];
        const SwTabFrame *pTable = pUnion->GetTable();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrame* pRow = pTable->IsFollow() ?
                                  pTable->GetFirstNonHeadlineRow() :
                                  static_cast<const SwLayoutFrame*>(pTable->Lower());

        while( pRow )
        {
            if( pRow->getFrameArea().IsOver( pUnion->GetUnion() ) )
            {
                const SwCellFrame* pUpperCell = nullptr;
                const SwLayoutFrame *pCell = pRow->FirstCell();

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

                    OSL_ENSURE( pCell->IsCellFrame(), "Frame without cell" );
                    if( ::IsFrameInTableSel( pUnion->GetUnion(), pCell ) )
                        pUpperCell = static_cast<const SwCellFrame*>(pCell);

                    if( pCell->GetNext() )
                    {
                        pCell = static_cast<const SwLayoutFrame*>(pCell->GetNext());
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrame() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrame( pCell );
                }

                if( pUpperCell )
                    rBoxes.push_back( const_cast< SwCellFrame* >(pUpperCell) );
            }
            if( bFound )
            {
                i = aUnions.size();
                break;
            }
            pRow = static_cast<const SwLayoutFrame*>(pRow->GetNext());
        }
    }

    // 2. check if box on left contains value/formula
    if( bTstRow )
    {
        bFound = false;

        rBoxes.clear();
        aUnions.clear();
        ::MakeSelUnions( aUnions, pStart, pEnd, SwTableSearchType::Row );

        for( SwSelUnions::size_type i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrame *pTable = pUnion->GetTable();

            // Skip any repeated headlines in the follow:
            const SwLayoutFrame* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      static_cast<const SwLayoutFrame*>(pTable->Lower());

            while( pRow )
            {
                if( pRow->getFrameArea().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrame *pCell = pRow->FirstCell();

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

                        OSL_ENSURE( pCell->IsCellFrame(), "Frame without cell" );
                        if( ::IsFrameInTableSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwCellFrame* pC = const_cast<SwCellFrame*>(static_cast<const SwCellFrame*>(pCell));
                            rBoxes.push_back( pC );
                        }
                        if( pCell->GetNext() )
                        {
                            pCell = static_cast<const SwLayoutFrame*>(pCell->GetNext());
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrame() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrame( pCell );
                    }
                }
                if( !bTstRow )
                {
                    i = aUnions.size();
                    break;
                }

                pRow = static_cast<const SwLayoutFrame*>(pRow->GetNext());
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

CmpLPt::CmpLPt( const Point& rPt, const SwTableBox* pBox, bool bVertical )
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
                pDoc->GetDfltTextFormatColl(), nullptr,
                nInsPos, nCnt );
}

bool IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam )
{
    rPam.GetPoint()->nNode = *rBox.GetSttNd()->EndOfSectionNode();
    rPam.Move( fnMoveBackward, GoInContent );
    rPam.SetMark();
    rPam.GetPoint()->nNode = *rBox.GetSttNd();
    rPam.Move( fnMoveForward, GoInContent );
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
                ((RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                 (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
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
    const SwLayoutFrame *pStart = pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                        &aPt )->GetUpper();
    pCntNd = rPam.GetContentNode(false);
    const SwLayoutFrame *pEnd = pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
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

    MergePos aPosArr;      // Sort-Array with the frame positions
    long nWidth;
    SwTableBox* pLastBox = nullptr;

    SwRectFnSet aRectFnSet(pStart->GetUpper());

    for ( auto & rSelUnion : aUnions )
    {
        const SwTabFrame *pTabFrame = rSelUnion.GetTable();

        SwRect &rUnion = rSelUnion.GetUnion();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrame* pRow = pTabFrame->IsFollow() ?
                                  pTabFrame->GetFirstNonHeadlineRow() :
                                  static_cast<const SwLayoutFrame*>(pTabFrame->Lower());

        while ( pRow )
        {
            if ( pRow->getFrameArea().IsOver( rUnion ) )
            {
                const SwLayoutFrame *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    OSL_ENSURE( pCell->IsCellFrame(), "Frame without cell" );
                    // overlap in full width?
                    if( rUnion.Top() <= pCell->getFrameArea().Top() &&
                        rUnion.Bottom() >= pCell->getFrameArea().Bottom() )
                    {
                        SwTableBox* pBox = const_cast<SwTableBox*>(static_cast<const SwCellFrame*>(pCell)->GetTabBox());

                        // only overlap to the right?
                        if( ( rUnion.Left() - COLFUZZY ) <= pCell->getFrameArea().Left() &&
                            ( rUnion.Right() - COLFUZZY ) > pCell->getFrameArea().Left() )
                        {
                            if( ( rUnion.Right() + COLFUZZY ) < pCell->getFrameArea().Right() )
                            {
                                sal_uInt16 nInsPos = pBox->GetUpper()->GetBoxPos( pBox )+1;
                                lcl_InsTableBox( pTableNd, pDoc, pBox, nInsPos );
                                pBox->ClaimFrameFormat();
                                SwFormatFrameSize aNew(
                                        pBox->GetFrameFormat()->GetFrameSize() );
                                nWidth = rUnion.Right() - pCell->getFrameArea().Left();
                                nWidth = nWidth * aNew.GetWidth() /
                                         pCell->getFrameArea().Width();
                                long nTmpWidth = aNew.GetWidth() - nWidth;
                                aNew.SetWidth( nWidth );
                                pBox->GetFrameFormat()->SetFormatAttr( aNew );
                                // this box is selected
                                pLastBox = pBox;
                                rBoxes.insert( pBox );
                                aPosArr.insert(
                                    CmpLPt( aRectFnSet.GetPos(pCell->getFrameArea()),
                                    pBox, aRectFnSet.IsVert() ) );

                                pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ].get();
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
                                    CmpLPt( aRectFnSet.GetPos(pCell->getFrameArea()),
                                    pBox, aRectFnSet.IsVert() ) );
                            }
                        }
                        // overlapping on left- or right-side
                        else if( ( rUnion.Left() - COLFUZZY ) >= pCell->getFrameArea().Left() &&
                                ( rUnion.Right() + COLFUZZY ) < pCell->getFrameArea().Right() )
                        {
                            sal_uInt16 nInsPos = pBox->GetUpper()->GetBoxPos( pBox )+1;
                            lcl_InsTableBox( pTableNd, pDoc, pBox, nInsPos, 2 );
                            pBox->ClaimFrameFormat();
                            SwFormatFrameSize aNew(
                                        pBox->GetFrameFormat()->GetFrameSize() );
                            long nLeft = rUnion.Left() - pCell->getFrameArea().Left();
                            nLeft = nLeft * aNew.GetWidth() /
                                    pCell->getFrameArea().Width();
                            long nRight = pCell->getFrameArea().Right() - rUnion.Right();
                            nRight = nRight * aNew.GetWidth() /
                                     pCell->getFrameArea().Width();
                            nWidth = aNew.GetWidth() - nLeft - nRight;

                            aNew.SetWidth( nLeft );
                            pBox->GetFrameFormat()->SetFormatAttr( aNew );

                            {
                            const SfxPoolItem* pItem;
                            if( SfxItemState::SET == pBox->GetFrameFormat()->GetAttrSet()
                                        .GetItemState( RES_BOX, false, &pItem ))
                            {
                                SvxBoxItem aBox( *static_cast<const SvxBoxItem*>(pItem) );
                                aBox.SetLine( nullptr, SvxBoxItemLine::RIGHT );
                                pBox->GetFrameFormat()->SetFormatAttr( aBox );
                            }
                            }

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ].get();
                            aNew.SetWidth( nWidth );
                            pBox->ClaimFrameFormat();
                            pBox->GetFrameFormat()->SetFormatAttr( aNew );

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );

                            // this box is selected
                            pLastBox = pBox;
                            rBoxes.insert( pBox );
                            aPosArr.insert(
                                CmpLPt( aRectFnSet.GetPos(pCell->getFrameArea()),
                                pBox, aRectFnSet.IsVert() ) );

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos+1 ].get();
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrameFormat();
                            pBox->GetFrameFormat()->SetFormatAttr( aNew );

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );
                        }
                       // is right side of box part of the selected area?
                        else if( ( pCell->getFrameArea().Right() - COLFUZZY ) < rUnion.Right() &&
                                 ( pCell->getFrameArea().Right() - COLFUZZY ) > rUnion.Left() &&
                                 ( pCell->getFrameArea().Left() + COLFUZZY ) < rUnion.Left() )
                        {
                            // then we should insert a new box and adjust the widths
                            sal_uInt16 nInsPos = pBox->GetUpper()->GetBoxPos( pBox )+1;
                            lcl_InsTableBox( pTableNd, pDoc, pBox, nInsPos );

                            SwFormatFrameSize aNew(pBox->GetFrameFormat()->GetFrameSize() );
                            long nLeft = rUnion.Left() - pCell->getFrameArea().Left(),
                                nRight = pCell->getFrameArea().Right() - rUnion.Left();

                            nLeft = nLeft * aNew.GetWidth() /
                                    pCell->getFrameArea().Width();
                            nRight = nRight * aNew.GetWidth() /
                                    pCell->getFrameArea().Width();

                            aNew.SetWidth( nLeft );
                            pBox->ClaimFrameFormat()->SetFormatAttr( aNew );

                                // this box is selected
                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ].get();
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrameFormat();
                            pBox->GetFrameFormat()->SetFormatAttr( aNew );

                            pLastBox = pBox;
                            rBoxes.insert( pBox );
                            aPosArr.insert( CmpLPt( Point( rUnion.Left(),
                                                pCell->getFrameArea().Top()), pBox, aRectFnSet.IsVert() ));

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );
                        }
                    }
                    if ( pCell->GetNext() )
                    {
                        pCell = static_cast<const SwLayoutFrame*>(pCell->GetNext());
                        // --> Check if table cell is not empty
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrame() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrame( pCell );
                }
            }
            pRow = static_cast<const SwLayoutFrame*>(pRow->GetNext());
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
        nWidth = pFirstBox->GetFrameFormat()->GetFrameSize().GetWidth();
        bool bEmptyLine = sal_True;
        sal_uInt16 n, nSttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // same Y level?
            {
                if( bEmptyLine && !IsEmptyBox( *rPt.pSelBox, aPam ))
                    bEmptyLine = sal_False;
                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrameFormat()->GetFrameSize().GetWidth();
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

        nWidth = pFirstBox->GetFrameFormat()->GetFrameSize().GetWidth();
        sal_uInt16 n, nSttPos = 0, nSEndPos = 0, nESttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const CmpLPt& rPt = aPosArr[ n ];
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
                    nWidth += rPt.pSelBox->GetFrameFormat()->GetFrameSize().GetWidth();
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
                    ( aRectFnSet.IsVert() ?
                      aPosArr[ 0 ].X() :
                      aPosArr[ 0 ].Y() ) :
                  0;

        for( MergePos::size_type n = 0; n < aPosArr.size(); ++n )
        {
            const CmpLPt& rPt = aPosArr[ n ];
            if( bCalcWidth )
            {
                if( nY == ( aRectFnSet.IsVert() ? rPt.X() : rPt.Y() ) ) // same Y level?
                    nWidth += rPt.pSelBox->GetFrameFormat()->GetFrameSize().GetWidth();
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
        sal_uInt16 nInsPos = pInsLine->GetBoxPos( pTmpBox );

        lcl_InsTableBox( pTableNd, pDoc, pTmpBox, nInsPos );
        (*ppMergeBox) = pInsLine->GetTabBoxes()[ nInsPos ].release();
        pInsLine->GetTabBoxes().erase( pInsLine->GetTabBoxes().begin() + nInsPos );  // remove again
        (*ppMergeBox)->SetUpper( nullptr );
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
    (*ppMergeBox)->GetFrameFormat()->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, nWidth, 0 ));
    if( pUndo )
        pUndo->AddNewBox( (*ppMergeBox)->GetSttIdx() );
}

static bool lcl_CheckCol(FndBox_ const&, bool* pPara);

static bool lcl_CheckRow( const FndLine_& rFndLine, bool* pPara )
{
    for (auto const& it : rFndLine.GetBoxes())
    {
        lcl_CheckCol(*it, pPara);
    }
    return *pPara;
}

static bool lcl_CheckCol( FndBox_ const& rFndBox, bool* pPara )
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

TableMergeErr CheckMergeSel( const SwPaM& rPam )
{
    SwSelBoxes aBoxes;
//JP 24.09.96:  Merge with repeating TableHeadLines does not work properly.
//              Why not use point 0,0? Then it is assured the first
//              headline is contained.

    Point aPt;
    const SwContentNode* pCntNd = rPam.GetContentNode();
    const SwLayoutFrame *pStart = pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                        &aPt )->GetUpper();
    pCntNd = rPam.GetContentNode(false);
    const SwLayoutFrame *pEnd = pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                    &aPt )->GetUpper();
    GetTableSel( pStart, pEnd, aBoxes, nullptr );
    return CheckMergeSel( aBoxes );
}

TableMergeErr CheckMergeSel( const SwSelBoxes& rBoxes )
{
    TableMergeErr eRet = TableMergeErr::NoSelection;
    if( !rBoxes.empty() )
    {
        eRet = TableMergeErr::Ok;

        FndBox_ aFndBox( nullptr, nullptr );
        FndPara aPara( rBoxes, &aFndBox );
        const SwTableNode* pTableNd = aPara.rBoxes[0]->GetSttNd()->FindTableNode();
        ForEach_FndLineCopyCol( const_cast<SwTableLines&>(pTableNd->GetTable().GetTabLines()), &aPara );
        if( !aFndBox.GetLines().empty() )
        {
            bool bMergeSelOk = true;
            FndBox_* pFndBox = &aFndBox;
            FndLine_* pFndLine = nullptr;
            while( pFndBox && 1 == pFndBox->GetLines().size() )
            {
                pFndLine = pFndBox->GetLines().front().get();
                if( 1 == pFndLine->GetBoxes().size() )
                    pFndBox = pFndLine->GetBoxes().front().get();
                else
                    pFndBox = nullptr;
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
                eRet = TableMergeErr::TooComplex;
        }
        else
            eRet = TableMergeErr::NoSelection;
    }
    return eRet;
}

static SwTwips lcl_CalcWish( const SwLayoutFrame *pCell, long nWish,
                                                const long nAct )
{
    const SwLayoutFrame *pTmp = pCell;
    if ( !nWish )
        nWish = 1;

    const bool bRTL = pCell->IsRightToLeft();
    SwTwips nRet = bRTL ?
        nAct - pCell->getFrameArea().Width() :
        0;

    while ( pTmp )
    {
        while ( pTmp->GetPrev() )
        {
            pTmp = static_cast<const SwLayoutFrame*>(pTmp->GetPrev());
            sal_Int64 nTmp = pTmp->GetFormat()->GetFrameSize().GetWidth();
            // multiply in 64-bit to avoid overflow here!
            nRet += ( bRTL ? -1 : 1 ) * nTmp * nAct / nWish;
        }
        pTmp = pTmp->GetUpper()->GetUpper();
        if ( pTmp && !pTmp->IsCellFrame() )
            pTmp = nullptr;
    }
    return nRet;
}

static void lcl_FindStartEndRow( const SwLayoutFrame *&rpStart,
                             const SwLayoutFrame *&rpEnd,
                             const bool bChkProtected )
{
    // Put Start at beginning of a row.
    // Put End at the end of its row.
    rpStart = static_cast<const SwLayoutFrame*>(rpStart->GetUpper()->Lower());
    while ( rpEnd->GetNext() )
        rpEnd = static_cast<const SwLayoutFrame*>(rpEnd->GetNext());

    std::deque<const SwLayoutFrame *> aSttArr, aEndArr;
    const SwLayoutFrame *pTmp;
    for( pTmp = rpStart; (SwFrameType::Cell|SwFrameType::Row) & pTmp->GetType();
                pTmp = pTmp->GetUpper() )
    {
        aSttArr.push_front( pTmp );
    }
    for( pTmp = rpEnd; (SwFrameType::Cell|SwFrameType::Row) & pTmp->GetType();
                pTmp = pTmp->GetUpper() )
    {
        aEndArr.push_front( pTmp );
    }

    for( std::deque<const SwLayoutFrame *>::size_type n = 0; n < aEndArr.size() && n < aSttArr.size(); ++n )
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
                    const SwCellFrame* pCellFrame = static_cast<const SwCellFrame*>(aSttArr[ n-1 ]);
                    const SwTableLines& rLns = pCellFrame->
                                                GetTabBox()->GetTabLines();
                    if( rLns[ 0 ] == static_cast<const SwRowFrame*>(aSttArr[ n ])->GetTabLine() &&
                        rLns[ rLns.size() - 1 ] ==
                                    static_cast<const SwRowFrame*>(aEndArr[ n ])->GetTabLine() )
                    {
                        rpStart = rpEnd = pCellFrame;
                        while ( rpStart->GetPrev() )
                            rpStart = static_cast<const SwLayoutFrame*>(rpStart->GetPrev());
                        while ( rpEnd->GetNext() )
                            rpEnd = static_cast<const SwLayoutFrame*>(rpEnd->GetNext());
                    }
                }
            }
            break;
        }

    if( !bChkProtected )    // protected cell ?
        return;

    // Beginning and end should not be in protected cells
    while ( rpStart->GetFormat()->GetProtect().IsContentProtected() )
        rpStart = static_cast<const SwLayoutFrame*>(rpStart->GetNext());
    while ( rpEnd->GetFormat()->GetProtect().IsContentProtected() )
        rpEnd = static_cast<const SwLayoutFrame*>(rpEnd->GetPrev());
}

static void lcl_FindStartEndCol( const SwLayoutFrame *&rpStart,
                             const SwLayoutFrame *&rpEnd,
                             const bool bChkProtected )
{
    // Beginning and end vertical till the border of the table;
    // Consider the whole table, including master and follows.
    // In order to start we need the mother-tableFrame
    if( !rpStart )
        return;
    const SwTabFrame *pOrg = rpStart->FindTabFrame();
    const SwTabFrame *pTab = pOrg;

    SwRectFnSet aRectFnSet(pTab);

    bool bRTL = pTab->IsRightToLeft();
    const long nTmpWish = pOrg->GetFormat()->GetFrameSize().GetWidth();
    const long nWish = ( nTmpWish > 0 ) ? nTmpWish : 1;

    while ( pTab->IsFollow() )
    {
        const SwFrame *pTmp = pTab->FindPrev();
        OSL_ENSURE( pTmp->IsTabFrame(), "Predecessor of Follow is not Master." );
        pTab = static_cast<const SwTabFrame*>(pTmp);
    }

    SwTwips nSX  = 0;
    SwTwips nSX2 = 0;

    if ( pTab->GetTable()->IsNewModel() )
    {
        nSX  = aRectFnSet.GetLeft(rpStart->getFrameArea());
        nSX2 = aRectFnSet.GetRight(rpStart->getFrameArea());
    }
    else
    {
        const SwTwips nPrtWidth = aRectFnSet.GetWidth(pTab->getFramePrintArea());
        nSX = ::lcl_CalcWish( rpStart, nWish, nPrtWidth ) + aRectFnSet.GetPrtLeft(*pTab);
        nSX2 = nSX + (rpStart->GetFormat()->GetFrameSize().GetWidth() * nPrtWidth / nWish);
    }

    const SwLayoutFrame *pTmp = pTab->FirstCell();

    while ( pTmp &&
            (!pTmp->IsCellFrame() ||
             ( ( ! bRTL && aRectFnSet.GetLeft(pTmp->getFrameArea()) < nSX &&
                           aRectFnSet.GetRight(pTmp->getFrameArea())< nSX2 ) ||
               (   bRTL && aRectFnSet.GetLeft(pTmp->getFrameArea()) > nSX &&
                           aRectFnSet.GetRight(pTmp->getFrameArea())> nSX2 ) ) ) )
        pTmp = pTmp->GetNextLayoutLeaf();

    if ( pTmp )
        rpStart = pTmp;

    pTab = pOrg;

    const SwTabFrame* pLastValidTab = pTab;
    while ( pTab->GetFollow() )
    {

        // Check if pTab->GetFollow() is a valid follow table:
        // Only follow tables with at least on non-FollowFlowLine
        // should be considered.

        if ( pTab->HasFollowFlowLine() )
        {
            pTab = pTab->GetFollow();
            const SwFrame* pTmpRow = pTab->GetFirstNonHeadlineRow();
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
        nEX = aRectFnSet.GetLeft(rpEnd->getFrameArea());
    }
    else
    {
        const SwTwips nPrtWidth = aRectFnSet.GetWidth(pTab->getFramePrintArea());
        nEX = ::lcl_CalcWish( rpEnd, nWish, nPrtWidth ) + aRectFnSet.GetPrtLeft(*pTab);
    }

    const SwContentFrame* pLastContent = pTab->FindLastContent();
    rpEnd = pLastContent ? pLastContent->GetUpper() : nullptr;
    // --> Made code robust. If pTab does not have a lower,
    // we would crash here.
    if ( !pLastContent ) return;

    while( !rpEnd->IsCellFrame() )
        rpEnd = rpEnd->GetUpper();

    while ( (   bRTL && aRectFnSet.GetLeft(rpEnd->getFrameArea()) < nEX ) ||
            ( ! bRTL && aRectFnSet.GetLeft(rpEnd->getFrameArea()) > nEX ) )
    {
        const SwLayoutFrame* pTmpLeaf = rpEnd->GetPrevLayoutLeaf();
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
        const SwLayoutFrame *pTmpLeaf = rpStart;
        pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        while ( pTmpLeaf && aRectFnSet.GetLeft(pTmpLeaf->getFrameArea()) > nEX ) // first skip line
            pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        while ( pTmpLeaf && aRectFnSet.GetLeft(pTmpLeaf->getFrameArea()) < nSX &&
                            aRectFnSet.GetRight(pTmpLeaf->getFrameArea())< nSX2 )
            pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        const SwTabFrame *pTmpTab = rpStart->FindTabFrame();
        if ( !pTmpTab->IsAnLower( pTmpLeaf ) )
        {
            pTmpTab = pTmpTab->GetFollow();
            rpStart = pTmpTab->FirstCell();
            while ( rpStart &&
                    aRectFnSet.GetLeft(rpStart->getFrameArea()) < nSX &&
                    aRectFnSet.GetRight(rpStart->getFrameArea())< nSX2 )
                rpStart = rpStart->GetNextLayoutLeaf();
        }
        else
            rpStart = pTmpLeaf;
    }
    while ( rpEnd->GetFormat()->GetProtect().IsContentProtected() )
    {
        const SwLayoutFrame *pTmpLeaf = rpEnd;
        pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        while ( pTmpLeaf && aRectFnSet.GetLeft(pTmpLeaf->getFrameArea()) < nEX ) // skip the line for now
            pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        while ( pTmpLeaf && aRectFnSet.GetLeft(pTmpLeaf->getFrameArea()) > nEX )
            pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        const SwTabFrame *pTmpTab = rpEnd->FindTabFrame();
        if ( !pTmpLeaf || !pTmpTab->IsAnLower( pTmpLeaf ) )
        {
            pTmpTab = static_cast<const SwTabFrame*>(pTmpTab->FindPrev());
            OSL_ENSURE( pTmpTab->IsTabFrame(), "Predecessor of Follow not Master.");
            rpEnd = pTmpTab->FindLastContent()->GetUpper();
            while( !rpEnd->IsCellFrame() )
                rpEnd = rpEnd->GetUpper();
            while ( aRectFnSet.GetLeft(rpEnd->getFrameArea()) > nEX )
                rpEnd = rpEnd->GetPrevLayoutLeaf();
        }
        else
            rpEnd = pTmpLeaf;
    }
}

void MakeSelUnions( SwSelUnions& rUnions, const SwLayoutFrame *pStart,
                    const SwLayoutFrame *pEnd, const SwTableSearchType eSearchType )
{
    while ( pStart && !pStart->IsCellFrame() )
        pStart = pStart->GetUpper();
    while ( pEnd && !pEnd->IsCellFrame() )
        pEnd = pEnd->GetUpper();

    if ( !pStart || !pEnd )
    {
        OSL_FAIL( "MakeSelUnions with pStart or pEnd not in CellFrame" );
        return;
    }

    const SwTabFrame *pTable = pStart->FindTabFrame();
    const SwTabFrame *pEndTable = pEnd->FindTabFrame();
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
        SwRectFnSet aRectFnSet(pTable);
        long nSttTop = aRectFnSet.GetTop(pStart->getFrameArea());
        long nEndTop = aRectFnSet.GetTop(pEnd->getFrameArea());
        if( nSttTop == nEndTop )
        {
            if( aRectFnSet.GetLeft(pStart->getFrameArea()) >
                aRectFnSet.GetLeft(pEnd->getFrameArea()) )
                bExchange = true;
        }
        else if( aRectFnSet.IsVert() == ( nSttTop < nEndTop ) )
            bExchange = true;
    }
    if ( bExchange )
    {
        const SwLayoutFrame *pTmp = pStart;
        pStart = pEnd;
        pEnd = pTmp;
        // do no resort pTable and pEndTable, set new below
        // MA: 28. Dec. 93 Bug: 5190
    }

    // Beginning and end now nicely sorted, if required we
    // should move them
    if( SwTableSearchType::Row == ((~SwTableSearchType::Protect ) & eSearchType ) )
        ::lcl_FindStartEndRow( pStart, pEnd, bool(SwTableSearchType::Protect & eSearchType) );
    else if( SwTableSearchType::Col == ((~SwTableSearchType::Protect ) & eSearchType ) )
        ::lcl_FindStartEndCol( pStart, pEnd, bool(SwTableSearchType::Protect & eSearchType) );

    if ( !pEnd || !pStart ) return; // Made code robust.

    // retrieve again, as they have been moved
    pTable = pStart->FindTabFrame();
    pEndTable = pEnd->FindTabFrame();

    const long nStSz = pStart->GetFormat()->GetFrameSize().GetWidth();
    const long nEdSz = pEnd->GetFormat()->GetFrameSize().GetWidth();
    const long nWish = std::max( 1L, pTable->GetFormat()->GetFrameSize().GetWidth() );
    while ( pTable )
    {
        SwRectFnSet aRectFnSet(pTable);
        const long nOfst = aRectFnSet.GetPrtLeft(*pTable);
        const long nPrtWidth = aRectFnSet.GetWidth(pTable->getFramePrintArea());
        long nSt1 = ::lcl_CalcWish( pStart, nWish, nPrtWidth ) + nOfst;
        long nEd1 = ::lcl_CalcWish( pEnd,   nWish, nPrtWidth ) + nOfst;

        if ( nSt1 <= nEd1 )
            nEd1 += static_cast<long>((nEdSz * nPrtWidth) / nWish) - 1;
        else
            nSt1 += static_cast<long>((nStSz * nPrtWidth) / nWish) - 1;

        long nSt2;
        long nEd2;
        if( pTable->IsAnLower( pStart ) )
            nSt2 = aRectFnSet.GetTop(pStart->getFrameArea());
        else
            nSt2 = aRectFnSet.GetTop(pTable->getFrameArea());
        if( pTable->IsAnLower( pEnd ) )
            nEd2 = aRectFnSet.GetBottom(pEnd->getFrameArea());
        else
            nEd2 = aRectFnSet.GetBottom(pTable->getFrameArea());
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
        if( aRectFnSet.IsVert() )
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

        if( !(SwTableSearchType::NoUnionCorrect & eSearchType ))
        {
            // Unfortunately the union contains rounding errors now, therefore
            // erroneous results could occur during split/merge.
            // To prevent these we will determine the first and last row
            // within the union and use their values for a new union
            const SwLayoutFrame* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      static_cast<const SwLayoutFrame*>(pTable->Lower());

            while ( pRow && !pRow->getFrameArea().IsOver( aUnion ) )
                pRow = static_cast<const SwLayoutFrame*>(pRow->GetNext());

            // #i31976#
            // A follow flow row may contain empty cells. These are not
            // considered by FirstCell(). Therefore we have to find
            // the first cell manually:
            const SwFrame* pTmpCell = nullptr;
            if ( pTable->IsFollow() && pRow && pRow->IsInFollowFlowRow() )
            {
                const SwFrame* pTmpRow = pRow;
                while ( pTmpRow && pTmpRow->IsRowFrame() )
                {
                    pTmpCell = static_cast<const SwRowFrame*>(pTmpRow)->Lower();
                    pTmpRow  = static_cast<const SwCellFrame*>(pTmpCell)->Lower();
                }
                OSL_ENSURE( !pTmpCell || pTmpCell->IsCellFrame(), "Lower of rowframe != cellframe?!" );
            }

            const SwLayoutFrame* pFirst = pTmpCell ?
                                        static_cast<const SwLayoutFrame*>(pTmpCell) :
                                        pRow ?
                                        pRow->FirstCell() :
                                        nullptr;

            while ( pFirst && !::IsFrameInTableSel( aUnion, pFirst ) )
            {
                if ( pFirst->GetNext() )
                {
                    pFirst = static_cast<const SwLayoutFrame*>(pFirst->GetNext());
                    if ( pFirst->Lower() && pFirst->Lower()->IsRowFrame() )
                        pFirst = pFirst->FirstCell();
                }
                else
                    pFirst = ::lcl_FindNextCellFrame( pFirst );
            }
            const SwLayoutFrame* pLast = nullptr;
            const SwFrame* pLastContent = pTable->FindLastContent();
            if ( pLastContent )
                pLast = ::lcl_FindCellFrame( pLastContent->GetUpper() );

            while ( pLast && !::IsFrameInTableSel( aUnion, pLast ) )
                pLast = ::lcl_FindCellFrame( pLast->GetPrevLayoutLeaf() );

            if ( pFirst && pLast ) //Robust
            {
                aUnion = pFirst->getFrameArea();
                aUnion.Union( pLast->getFrameArea() );
            }
            else
                aUnion.Width( 0 );
        }

        if( aRectFnSet.GetWidth(aUnion) )
        {
            rUnions.emplace_back(aUnion, const_cast<SwTabFrame*>(pTable));
        }

        pTable = pTable->GetFollow();
        if ( pTable != pEndTable && pEndTable->IsAnFollow( pTable ) )
            pTable = nullptr;
    }
}

bool CheckSplitCells( const SwCursorShell& rShell, sal_uInt16 nDiv,
                        const SwTableSearchType eSearchType )
{
    if( !rShell.IsTableMode() )
        rShell.GetCursor();

    return CheckSplitCells( *rShell.getShellCursor(false), nDiv, eSearchType );
}

bool CheckSplitCells( const SwCursor& rCursor, sal_uInt16 nDiv,
                        const SwTableSearchType eSearchType )
{
    if( 1 >= nDiv )
        return false;

    sal_uInt16 nMinValue = nDiv * MINLAY;

    // Get start and end cell
    Point aPtPos, aMkPos;
    const SwShellCursor* pShCursor = dynamic_cast<const SwShellCursor*>(&rCursor);
    if( pShCursor )
    {
        aPtPos = pShCursor->GetPtPos();
        aMkPos = pShCursor->GetMkPos();
    }

    const SwContentNode* pCntNd = rCursor.GetContentNode();
    const SwLayoutFrame *pStart = pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                                        &aPtPos )->GetUpper();
    pCntNd = rCursor.GetContentNode(false);
    const SwLayoutFrame *pEnd = pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                                &aMkPos )->GetUpper();

    SwRectFnSet aRectFnSet(pStart->GetUpper());

    // First, compute tables and rectangles
    SwSelUnions aUnions;

    ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

    // now search boxes for each entry and emit
    for ( const auto& rSelUnion : aUnions )
    {
        const SwTabFrame *pTable = rSelUnion.GetTable();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrame* pRow = pTable->IsFollow() ?
                                  pTable->GetFirstNonHeadlineRow() :
                                  static_cast<const SwLayoutFrame*>(pTable->Lower());

        while ( pRow )
        {
            if ( pRow->getFrameArea().IsOver( rSelUnion.GetUnion() ) )
            {
                const SwLayoutFrame *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    OSL_ENSURE( pCell->IsCellFrame(), "Frame without cell" );
                    if( ::IsFrameInTableSel( rSelUnion.GetUnion(), pCell ) )
                    {
                        if( aRectFnSet.GetWidth(pCell->getFrameArea()) < nMinValue )
                            return false;
                    }

                    if ( pCell->GetNext() )
                    {
                        pCell = static_cast<const SwLayoutFrame*>(pCell->GetNext());
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrame() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrame( pCell );
                }
            }
            pRow = static_cast<const SwLayoutFrame*>(pRow->GetNext());
        }
    }
    return true;
}

// These Classes copy the current table selections (rBoxes),
// into a new structure, retaining the table structure
// new: SS for targeted erasing/restoring of the layout

static void lcl_InsertRow( SwTableLine const &rLine, SwLayoutFrame *pUpper, SwFrame *pSibling )
{
    SwRowFrame *pRow = new SwRowFrame( rLine, pUpper );
    if ( pUpper->IsTabFrame() && static_cast<SwTabFrame*>(pUpper)->IsFollow() )
    {
        SwTabFrame* pTabFrame = static_cast<SwTabFrame*>(pUpper);
        pTabFrame->FindMaster()->InvalidatePos(); //can absorb the line

        if ( pSibling && pTabFrame->IsInHeadline( *pSibling ) )
        {
            // Skip any repeated headlines in the follow:
            pSibling = pTabFrame->GetFirstNonHeadlineRow();
        }
    }
    pRow->Paste( pUpper, pSibling );
    pRow->RegistFlys();
}

static void FndBoxCopyCol( SwTableBox* pBox, FndPara* pFndPara )
{
    std::unique_ptr<FndBox_> pFndBox(new FndBox_( pBox, pFndPara->pFndLine ));
    if( pBox->GetTabLines().size() )
    {
        FndPara aPara( *pFndPara, pFndBox.get() );
        ForEach_FndLineCopyCol( pFndBox->GetBox()->GetTabLines(), &aPara );
        if( pFndBox->GetLines().empty() )
        {
            return;
        }
    }
    else
    {
        if( pFndPara->rBoxes.find( pBox ) == pFndPara->rBoxes.end())
        {
            return;
        }
    }
    pFndPara->pFndLine->GetBoxes().push_back( std::move(pFndBox) );
}

static void FndLineCopyCol( SwTableLine* pLine, FndPara* pFndPara )
{
    std::unique_ptr<FndLine_> pFndLine(new FndLine_(pLine, pFndPara->pFndBox));
    FndPara aPara(*pFndPara, pFndLine.get());
    for( std::unique_ptr<SwTableBox> const & pBox : pFndLine->GetLine()->GetTabBoxes())
        FndBoxCopyCol(pBox.get(), &aPara );
    if( pFndLine->GetBoxes().size() )
    {
        pFndPara->pFndBox->GetLines().push_back( std::move(pFndLine) );
    }
}

void ForEach_FndLineCopyCol(SwTableLines& rLines, FndPara* pFndPara )
{
    for( SwTableLines::iterator it = rLines.begin(); it != rLines.end(); ++it )
        FndLineCopyCol( *it, pFndPara );
}

void FndBox_::SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable )
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
        m_pLineBefore = rTable.GetTabLines()[nStPos - 2];
    if ( nEndPos < rTable.GetTabLines().size() )
        m_pLineBehind = rTable.GetTabLines()[nEndPos];
}

void FndBox_::SetTableLines( const SwTable &rTable )
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
    OSL_ENSURE( USHRT_MAX != nPos, "Line is not in table" );
    if( nPos )
        m_pLineBefore = rTable.GetTabLines()[ nPos - 1 ];

    pTmpLine = GetLines().back()->GetLine();
    nPos = rTable.GetTabLines().GetPos( pTmpLine );
    OSL_ENSURE( USHRT_MAX != nPos, "Line is not in the table" );
    if( ++nPos < rTable.GetTabLines().size() )
        m_pLineBehind = rTable.GetTabLines()[nPos];
}

inline void UnsetFollow( SwFlowFrame *pTab )
{
    pTab->m_pPrecede = nullptr;
}

void FndBox_::DelFrames( SwTable &rTable )
{
    // All lines between pLineBefore and pLineBehind should be cut
    // from the layout and erased.
    // If this creates empty Follows we should destroy these.
    // If a master is destroyed, the follow should become master.
    // Always a TabFrame should remain.

    sal_uInt16 nStPos = 0;
    sal_uInt16 nEndPos= rTable.GetTabLines().size() - 1;
    if( rTable.IsNewModel() && m_pLineBefore )
        rTable.CheckRowSpan( m_pLineBefore, true );
    if ( m_pLineBefore )
    {
        nStPos = rTable.GetTabLines().GetPos(
                        const_cast<const SwTableLine*&>(m_pLineBefore) );
        OSL_ENSURE( nStPos != USHRT_MAX, "The fox stole the line!" );
        ++nStPos;
    }
    if( rTable.IsNewModel() && m_pLineBehind )
        rTable.CheckRowSpan( m_pLineBehind, false );
    if ( m_pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        const_cast<const SwTableLine*&>(m_pLineBehind) );
        OSL_ENSURE( nEndPos != USHRT_MAX, "The fox stole the line!" );
        if (nEndPos != 0)
            --nEndPos;
    }

    for ( sal_uInt16 i = nStPos; i <= nEndPos; ++i)
    {
        SwFrameFormat *pFormat = rTable.GetTabLines()[i]->GetFrameFormat();
        SwIterator<SwRowFrame,SwFormat> aIter( *pFormat );
        for ( SwRowFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
        {
                if ( pFrame->GetTabLine() == rTable.GetTabLines()[i] )
                {
                    bool bDel = true;
                    SwTabFrame *pUp = !pFrame->GetPrev() && !pFrame->GetNext() ?
                                            static_cast<SwTabFrame*>(pFrame->GetUpper()) : nullptr;
                    if ( !pUp )
                    {
                        const sal_uInt16 nRepeat =
                                static_cast<SwTabFrame*>(pFrame->GetUpper())->GetTable()->GetRowsToRepeat();
                        if ( nRepeat > 0 &&
                             static_cast<SwTabFrame*>(pFrame->GetUpper())->IsFollow() )
                        {
                            if ( !pFrame->GetNext() )
                            {
                                SwRowFrame* pFirstNonHeadline =
                                    static_cast<SwTabFrame*>(pFrame->GetUpper())->GetFirstNonHeadlineRow();
                                if ( pFirstNonHeadline == pFrame )
                                {
                                    pUp = static_cast<SwTabFrame*>(pFrame->GetUpper());
                                }
                            }
                        }
                    }
                    if ( pUp )
                    {
                        SwTabFrame *pFollow = pUp->GetFollow();
                        SwTabFrame *pPrev   = pUp->IsFollow() ? pUp : nullptr;
                        if ( pPrev )
                        {
                            SwFrame *pTmp = pPrev->FindPrev();
                            OSL_ENSURE( pTmp->IsTabFrame(),
                                    "Predecessor of Follow is no Master.");
                            pPrev = static_cast<SwTabFrame*>(pTmp);
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

                        // A TableFrame should always remain!
                        if ( pPrev || pFollow )
                        {
                            // OD 26.08.2003 #i18103# - if table is in a section,
                            // lock the section, to avoid its delete.
                            {
                                SwSectionFrame* pSctFrame = pUp->FindSctFrame();
                                bool bOldSectLock = false;
                                if ( pSctFrame )
                                {
                                    bOldSectLock = pSctFrame->IsColLocked();
                                    pSctFrame->ColLock();
                                }
                                pUp->Cut();
                                if ( pSctFrame && !bOldSectLock )
                                {
                                    pSctFrame->ColUnlock();
                                }
                            }
                            SwFrame::DestroyFrame(pUp);
                            bDel = false; // Row goes to /dev/null.
                        }
                    }
                    if ( bDel )
                    {
                        SwFrame* pTabFrame = pFrame->GetUpper();
                        if ( pTabFrame->IsTabFrame() &&
                            !pFrame->GetNext() &&
                             static_cast<SwTabFrame*>(pTabFrame)->GetFollow() )
                        {
                            // We do not delete the follow flow line,
                            // this will be done automatically in the
                            // next turn.
                            static_cast<SwTabFrame*>(pTabFrame)->SetFollowFlowLine( false );
                        }
                        pFrame->Cut();
                        SwFrame::DestroyFrame(pFrame);
                    }
                }
        }
    }
}

static bool lcl_IsLineOfTableFrame( const SwTabFrame& rTable, const SwFrame& rChk )
{
    const SwTabFrame* pTableFrame = rChk.FindTabFrame();
    if( pTableFrame->IsFollow() )
        pTableFrame = pTableFrame->FindMaster( true );
    return &rTable == pTableFrame;
}

static void lcl_UpdateRepeatedHeadlines( SwTabFrame& rTabFrame, bool bCalcLowers )
{
    OSL_ENSURE( rTabFrame.IsFollow(), "lcl_UpdateRepeatedHeadlines called for non-follow tab" );

    // Delete remaining headlines:
    SwRowFrame* pLower = nullptr;
    while ( nullptr != ( pLower = static_cast<SwRowFrame*>(rTabFrame.Lower()) ) && pLower->IsRepeatedHeadline() )
    {
        pLower->Cut();
        SwFrame::DestroyFrame(pLower);
    }

    // Insert fresh set of headlines:
    pLower = static_cast<SwRowFrame*>(rTabFrame.Lower());
    SwTable& rTable = *rTabFrame.GetTable();
    const sal_uInt16 nRepeat = rTable.GetRowsToRepeat();
    for ( sal_uInt16 nIdx = 0; nIdx < nRepeat; ++nIdx )
    {
        SwRowFrame* pHeadline = new SwRowFrame( *rTable.GetTabLines()[ nIdx ], &rTabFrame );
        pHeadline->SetRepeatedHeadline( true );
        pHeadline->Paste( &rTabFrame, pLower );
        pHeadline->RegistFlys();
    }

    if ( bCalcLowers )
        rTabFrame.SetCalcLowers();
}

void FndBox_::MakeFrames( SwTable &rTable )
{
    // All lines between pLineBefore and pLineBehind should be re-generated in layout.
    // And this for all instances of a table (for example in header/footer).
    sal_uInt16 nStPos = 0;
    sal_uInt16 nEndPos= rTable.GetTabLines().size() - 1;
    if ( m_pLineBefore )
    {
        nStPos = rTable.GetTabLines().GetPos(
                        const_cast<const SwTableLine*&>(m_pLineBefore) );
        OSL_ENSURE( nStPos != USHRT_MAX, "Fox stole the line!" );
        ++nStPos;

    }
    if ( m_pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        const_cast<const SwTableLine*&>(m_pLineBehind) );
        OSL_ENSURE( nEndPos != USHRT_MAX, "Fox stole the line!" );
        --nEndPos;
    }
    // now big insert operation for all tables.
    SwIterator<SwTabFrame,SwFormat> aTabIter( *rTable.GetFrameFormat() );
    for ( SwTabFrame *pTable = aTabIter.First(); pTable; pTable = aTabIter.Next() )
    {
        if ( !pTable->IsFollow() )
        {
            SwRowFrame  *pSibling = nullptr;
            SwFrame  *pUpperFrame  = nullptr;
            int i;
            for ( i = rTable.GetTabLines().size()-1;
                    i >= 0 && !pSibling; --i )
            {
                SwTableLine *pLine = m_pLineBehind ? m_pLineBehind :
                                                    rTable.GetTabLines()[static_cast<sal_uInt16>(i)];
                SwIterator<SwRowFrame,SwFormat> aIter( *pLine->GetFrameFormat() );
                pSibling = aIter.First();
                while ( pSibling && (
                            pSibling->GetTabLine() != pLine ||
                            !lcl_IsLineOfTableFrame( *pTable, *pSibling ) ||
                            pSibling->IsRepeatedHeadline() ||
                            // #i53647# If !pLineBehind,
                            // IsInSplitTableRow() should be checked.
                            ( m_pLineBehind && pSibling->IsInFollowFlowRow() ) ||
                            (!m_pLineBehind && pSibling->IsInSplitTableRow() ) ) )
                {
                    pSibling = aIter.Next();
                }
            }
            if ( pSibling )
            {
                pUpperFrame = pSibling->GetUpper();
                if ( !m_pLineBehind )
                    pSibling = nullptr;
            }
            else
// ???? or is this the last Follow of the table ????
                pUpperFrame = pTable;

            for ( sal_uInt16 j = nStPos; j <= nEndPos; ++j )
                ::lcl_InsertRow( *rTable.GetTabLines()[j],
                                static_cast<SwLayoutFrame*>(pUpperFrame), pSibling );
            if ( pUpperFrame->IsTabFrame() )
                static_cast<SwTabFrame*>(pUpperFrame)->SetCalcLowers();
        }
        else if ( rTable.GetRowsToRepeat() > 0 )
        {
            // Insert new headlines:
            lcl_UpdateRepeatedHeadlines( *pTable, true );
        }
    }
}

void FndBox_::MakeNewFrames( SwTable &rTable, const sal_uInt16 nNumber,
                                            const bool bBehind )
{
    // Create Frames for newly inserted lines
    // bBehind == true:  before  pLineBehind
    //         == false: after   pLineBefore
    const sal_uInt16 nBfPos = m_pLineBefore ?
        rTable.GetTabLines().GetPos( const_cast<const SwTableLine*&>(m_pLineBefore) ) :
        USHRT_MAX;
    const sal_uInt16 nBhPos = m_pLineBehind ?
        rTable.GetTabLines().GetPos( const_cast<const SwTableLine*&>(m_pLineBehind) ) :
        USHRT_MAX;

    //nNumber: how often did we insert
    //nCnt:    how many were inserted nNumber times

    const sal_uInt16 nCnt =
        ((nBhPos != USHRT_MAX ? nBhPos : rTable.GetTabLines().size()) -
         (nBfPos != USHRT_MAX ? nBfPos + 1 : 0)) / (nNumber + 1);

    // search the Master-TabFrame
    SwIterator<SwTabFrame,SwFormat> aTabIter( *rTable.GetFrameFormat() );
    SwTabFrame *pTable;
    for ( pTable = aTabIter.First(); pTable; pTable = aTabIter.Next() )
    {
        if( !pTable->IsFollow() )
        {
            SwRowFrame* pSibling = nullptr;
            SwLayoutFrame *pUpperFrame   = nullptr;
            if ( bBehind )
            {
                if ( m_pLineBehind )
                {
                    SwIterator<SwRowFrame,SwFormat> aIter( *m_pLineBehind->GetFrameFormat() );
                    pSibling = aIter.First();
                    while ( pSibling && (
                                // only consider row frames associated with pLineBehind:
                                pSibling->GetTabLine() != m_pLineBehind ||
                                // only consider row frames that are in pTables Master-Follow chain:
                                !lcl_IsLineOfTableFrame( *pTable, *pSibling ) ||
                                // only consider row frames that are not repeated headlines:
                                pSibling->IsRepeatedHeadline() ||
                                // only consider row frames that are not follow flow rows
                                pSibling->IsInFollowFlowRow() ) )
                    {
                          pSibling = aIter.Next();
                    }
                }
                if ( pSibling )
                    pUpperFrame = pSibling->GetUpper();
                else
                {
                    while( pTable->GetFollow() )
                        pTable = pTable->GetFollow();
                    pUpperFrame = pTable;
                }
                const sal_uInt16 nMax = nBhPos != USHRT_MAX ?
                                    nBhPos : rTable.GetTabLines().size();

                sal_uInt16 i = nBfPos != USHRT_MAX ? nBfPos + 1 + nCnt : nCnt;

                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i], pUpperFrame, pSibling );
                if ( pUpperFrame->IsTabFrame() )
                    static_cast<SwTabFrame*>(pUpperFrame)->SetCalcLowers();
            }
            else // insert before
            {
                sal_uInt16 i;

                // We are looking for the frame that is behind the row frame
                // that should be inserted.
                for ( i = 0; !pSibling; ++i )
                {
                    SwTableLine* pLine = m_pLineBefore ? m_pLineBefore : rTable.GetTabLines()[i];

                    SwIterator<SwRowFrame,SwFormat> aIter( *pLine->GetFrameFormat() );
                    pSibling = aIter.First();

                    while ( pSibling && (
                            // only consider row frames associated with pLineBefore:
                            pSibling->GetTabLine() != pLine ||
                            // only consider row frames that are in pTables Master-Follow chain:
                            !lcl_IsLineOfTableFrame( *pTable, *pSibling ) ||
                            // only consider row frames that are not repeated headlines:
                            pSibling->IsRepeatedHeadline() ||
                            // 1. case: pLineBefore == 0:
                            // only consider row frames that are not follow flow rows
                            // 2. case: pLineBefore != 0:
                            // only consider row frames that are not split table rows
                            // #i37476# If !pLineBefore,
                            // check IsInFollowFlowRow instead of IsInSplitTableRow.
                            ( ( !m_pLineBefore && pSibling->IsInFollowFlowRow() ) ||
                              (  m_pLineBefore && pSibling->IsInSplitTableRow() ) ) ) )
                    {
                        pSibling = aIter.Next();
                    }
                }

                pUpperFrame = pSibling->GetUpper();
                if ( m_pLineBefore )
                    pSibling = static_cast<SwRowFrame*>( pSibling->GetNext() );

                sal_uInt16 nMax = nBhPos != USHRT_MAX ?
                                    nBhPos - nCnt :
                                    rTable.GetTabLines().size() - nCnt;

                i = nBfPos != USHRT_MAX ? nBfPos + 1 : 0;
                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i],
                                pUpperFrame, pSibling );
                if ( pUpperFrame->IsTabFrame() )
                    static_cast<SwTabFrame*>(pUpperFrame)->SetCalcLowers();
            }
        }
    }

    // If necessary headlines should be processed. In order to
    // not to fragment good code, we iterate once more.
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

                OSL_ENSURE( static_cast<SwRowFrame*>(pTable->Lower())->GetTabLine() ==
                        rTable.GetTabLines()[0], "MakeNewFrames: Table corruption!" );
            }
        }
    }
}

bool FndBox_::AreLinesToRestore( const SwTable &rTable ) const
{
    // Should we call MakeFrames here?

    if ( !m_pLineBefore && !m_pLineBehind && rTable.GetTabLines().size() )
        return true;

    sal_uInt16 nBfPos;
    if(m_pLineBefore)
    {
        const SwTableLine* rLBefore = const_cast<const SwTableLine*>(m_pLineBefore);
        nBfPos = rTable.GetTabLines().GetPos( rLBefore );
    }
    else
        nBfPos = USHRT_MAX;

    sal_uInt16 nBhPos;
    if(m_pLineBehind)
    {
        const SwTableLine* rLBehind = const_cast<const SwTableLine*>(m_pLineBehind);
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
        SwIterator<SwTabFrame,SwFormat> aIter( *rTable.GetFrameFormat() );
        for( SwTabFrame* pTable = aIter.First(); pTable; pTable = aIter.Next() )
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
    // A call of MakeFrames() is necessary.
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
