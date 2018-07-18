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

#include <swtable.hxx>
#include <tblsel.hxx>
#include <tblrwcl.hxx>
#include <ndtxt.hxx>
#include <node.hxx>
#include <UndoTable.hxx>
#include <pam.hxx>
#include <frmfmt.hxx>
#include <frmatr.hxx>
#include <cellfrm.hxx>
#include <fmtfsize.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <cstdlib>
#include <vector>
#include <set>
#include <list>
#include <memory>
#include <editeng/boxitem.hxx>
#include <editeng/protitem.hxx>
#include <swtblfmt.hxx>
#include <calbck.hxx>
#include <o3tl/make_unique.hxx>

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

/** SwBoxSelection is a small helperclass (structure) to handle selections
    of cells (boxes) between table functions

    It contains an "array" of table boxes, a rectangulare selection of table boxes.
    To be more specific, it contains a vector of box selections,
    every box selection (SwSelBoxes) contains the selected boxes inside one row.
    The member mnMergeWidth contains the width of the selected boxes
*/

class SwBoxSelection
{
public:
    std::vector<SwSelBoxes> maBoxes;
    long mnMergeWidth;
    SwBoxSelection() : mnMergeWidth(0) {}
    bool isEmpty() const { return maBoxes.empty(); }
    void push_back(const SwSelBoxes& rNew) { maBoxes.push_back(rNew); }
};

/** NewMerge(..) removes the superfluous cells after cell merge

SwTable::NewMerge(..) does some cleaning up,
it simply deletes the superfluous cells ("cell span")
and notifies the Undo about it.
The main work has been done by SwTable::PrepareMerge(..) already.

@param rBoxes
the boxes to remove

@param pUndo
the undo object to notify, maybe empty

@return true for compatibility reasons with OldMerge(..)
*/

bool SwTable::NewMerge( SwDoc* pDoc, const SwSelBoxes& rBoxes,
     const SwSelBoxes& rMerged, SwUndoTableMerge* pUndo )
{
    if( pUndo )
        pUndo->SetSelBoxes( rBoxes );
    DeleteSel( pDoc, rBoxes, &rMerged, nullptr, true, true );

    CHECK_TABLE( *this )
    return true;
}

/** lcl_CheckMinMax helps evaluating (horizontal) min/max of boxes

lcl_CheckMinMax(..) compares the left border and the right border
of a given cell with the given range and sets it accordingly.

@param rMin
will be decremented if necessary to the left border of the cell

@param rMax
will be incremented if necessary to the right border of the cell

@param rLine
the row (table line) of the interesting box

@param nCheck
the index of the box in the table box array of the given row

@param bSet
if bSet is false, rMin and rMax will be manipulated if necessary
if bSet is true, rMin and rMax will be set to the left and right border of the box

*/

static void lcl_CheckMinMax( long& rMin, long& rMax, const SwTableLine& rLine, size_t nCheck, bool bSet )
{
    ++nCheck;
    if( rLine.GetTabBoxes().size() < nCheck )
    {   // robust
        OSL_FAIL( "Box out of table line" );
        nCheck = rLine.GetTabBoxes().size();
    }

    long nNew = 0; // will be the right border of the current box
    long nWidth = 0; // the width of the current box
    for( size_t nCurrBox = 0; nCurrBox < nCheck; ++nCurrBox )
    {
        SwTableBox* pBox = rLine.GetTabBoxes()[nCurrBox].get();
        OSL_ENSURE( pBox, "Missing table box" );
        nWidth = pBox->GetFrameFormat()->GetFrameSize().GetWidth();
        nNew += nWidth;
    }
    // nNew is the right border of the wished box
    if( bSet || nNew > rMax )
        rMax = nNew;
    nNew -= nWidth; // nNew becomes the left border of the wished box
    if( bSet || nNew < rMin )
        rMin = nNew;
}

/** lcl_Box2LeftBorder(..) delivers the left (logical) border of a table box

The left logical border of a table box is the sum of the cell width before this
box.

@param rBox
is the requested table box

@return is the left logical border (long, even it cannot be negative)

*/

static long lcl_Box2LeftBorder( const SwTableBox& rBox )
{
    if( !rBox.GetUpper() )
        return 0;
    long nLeft = 0;
    const SwTableLine &rLine = *rBox.GetUpper();
    const size_t nCount = rLine.GetTabBoxes().size();
    for( size_t nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
    {
        SwTableBox* pBox = rLine.GetTabBoxes()[nCurrBox].get();
        OSL_ENSURE( pBox, "Missing table box" );
        if( pBox == &rBox )
            return nLeft;
        nLeft += pBox->GetFrameFormat()->GetFrameSize().GetWidth();
    }
    OSL_FAIL( "Box not found in own upper?" );
    return nLeft;
}

/** lcl_LeftBorder2Box delivers the box to a given left border

It's used to find the master/follow table boxes in previous/next rows.
Don't call this function to check if there is such a box,
call it if you know there has to be such box.

@param nLeft
the left border (logical x-value) of the demanded box

@param rLine
the row (table line) to be scanned

@return a pointer to the table box inside the given row with the wished left border

*/

static SwTableBox* lcl_LeftBorder2Box( long nLeft, const SwTableLine* pLine )
{
    if( !pLine )
        return nullptr;
    long nCurrLeft = 0;
    const size_t nCount = pLine->GetTabBoxes().size();
    for( size_t nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
    {
        SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox].get();
        OSL_ENSURE( pBox, "Missing table box" );
        if( pBox->GetFrameFormat()->GetFrameSize().GetWidth() )
        {
            if( nCurrLeft == nLeft )
                return pBox;
            // HACK: It appears that rounding errors may result in positions not matching
            // exactly, so allow a little tolerance. This happens at least with merged cells
            // in the doc from fdo#38414 .
            if( std::abs( nCurrLeft - nLeft ) <= ( nLeft / 1000 ))
                return pBox;
            if( nCurrLeft >= nLeft )
            {
                SAL_WARN( "sw.core", "Possibly wrong box found" );
                return pBox;
            }
        }
        nCurrLeft += pBox->GetFrameFormat()->GetFrameSize().GetWidth();
    }
    OSL_FAIL( "Didn't find wished box" );
    return nullptr;
}

/** lcl_ChangeRowSpan corrects row span after insertion/deletion of rows

lcl_ChangeRowSpan(..) has to be called after an insertion or deletion of rows
to adjust the row spans of previous rows accordingly.
If rows are deleted, the previous rows with row spans into the deleted area
have to be decremented by the number of _overlapped_ inserted rows.
If rows are inserted, the previous rows with row span into the inserted area
have to be incremented by the number of inserted rows.
For those row spans which ends exactly above the inserted area it has to be
decided by the parameter bSingle if they have to be expanded or not.

@param rTable
the table to manipulate (has to be a new model table)

@param nDiff
the number of rows which has been inserted (nDiff > 0) or deleted (nDiff < 0)

@param nRowIdx
the index of the first row which has to be checked

@param bSingle
true if the new inserted row should not extend row spans which ends in the row above
this is for rows inserted by UI "insert row"
false if all cells of an inserted row has to be overlapped by the previous row
this is for rows inserted by "split row"
false is also needed for deleted rows

*/

static void lcl_ChangeRowSpan( const SwTable& rTable, const long nDiff,
                        sal_uInt16 nRowIdx, const bool bSingle )
{
    if( !nDiff || nRowIdx >= rTable.GetTabLines().size() )
        return;
    OSL_ENSURE( !bSingle || nDiff > 0, "Don't set bSingle when deleting lines!" );
    bool bGoOn;
    // nDistance is the distance between the current row and the critical row,
    // e.g. the deleted rows or the inserted rows.
    // If the row span is lower than the distance there is nothing to do
    // because the row span ends before the critical area.
    // When the inserted rows should not be overlapped by row spans which ends
    // exactly in the row above, the trick is to start with a distance of 1.
    long nDistance = bSingle ? 1 : 0;
    do
    {
        bGoOn = false; // will be set to true if we found a non-master cell
        // which has to be manipulated => we have to check the previous row, too.
        const SwTableLine* pLine = rTable.GetTabLines()[ nRowIdx ];
        const size_t nBoxCount = pLine->GetTabBoxes().size();
        for( size_t nCurrBox = 0; nCurrBox < nBoxCount; ++nCurrBox )
        {
            long nRowSpan = pLine->GetTabBoxes()[nCurrBox]->getRowSpan();
            long nAbsSpan = nRowSpan > 0 ? nRowSpan : -nRowSpan;
            // Check if the last overlapped cell is above or below
            // the critical area
            if( nAbsSpan > nDistance )
            {
                if( nDiff > 0 )
                {
                    if( nRowSpan > 0 )
                        nRowSpan += nDiff; // increment row span of master cell
                    else
                    {
                        nRowSpan -= nDiff; // increment row span of non-master cell
                        bGoOn = true;
                    }
                }
                else
                {
                    if( nRowSpan > 0 )
                    {   // A master cell
                         // end of row span behind the deleted area ..
                        if( nRowSpan - nDistance > -nDiff )
                            nRowSpan += nDiff;
                        else // .. or inside the deleted area
                            nRowSpan = nDistance + 1;
                    }
                    else
                    {   // Same for a non-master cell
                        if( nRowSpan + nDistance < nDiff )
                            nRowSpan -= nDiff;
                        else
                            nRowSpan = -nDistance - 1;
                        bGoOn = true; // We have to continue
                    }
                }
                pLine->GetTabBoxes()[ nCurrBox ]->setRowSpan( nRowSpan );
            }
        }
        ++nDistance;
        if( nRowIdx )
            --nRowIdx;
        else
            bGoOn = false; //robust
    } while( bGoOn );
}

/** CollectBoxSelection(..) create a rectangulare selection based on the given SwPaM
    and prepares the selected cells for merging
*/

std::unique_ptr<SwBoxSelection> SwTable::CollectBoxSelection( const SwPaM& rPam ) const
{
    OSL_ENSURE( m_bNewModel, "Don't call me for old tables" );
    if( m_aLines.empty() )
        return nullptr;
    const SwNode* pStartNd = rPam.Start()->nNode.GetNode().FindTableBoxStartNode();
    const SwNode* pEndNd = rPam.End()->nNode.GetNode().FindTableBoxStartNode();
    if( !pStartNd || !pEndNd || pStartNd == pEndNd )
        return nullptr;

    const size_t nLines = m_aLines.size();
    size_t nTop = 0;
    size_t nBottom = 0;
    long nMin = 0, nMax = 0;
    int nFound = 0;
    for( size_t nRow = 0; nFound < 2 && nRow < nLines; ++nRow )
    {
        SwTableLine* pLine = m_aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        const size_t nCols = pLine->GetTabBoxes().size();
        for( size_t nCol = 0; nCol < nCols; ++nCol )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCol].get();
            OSL_ENSURE( pBox, "Missing table box" );
            if( nFound )
            {
                if( pBox->GetSttNd() == pEndNd )
                {
                    nBottom = nRow;
                    lcl_CheckMinMax( nMin, nMax, *pLine, nCol, false );
                    ++nFound;
                    break;
                }
            }
            else if( pBox->GetSttNd() == pStartNd )
            {
                nTop = nRow;
                lcl_CheckMinMax( nMin, nMax, *pLine, nCol, true );
                ++nFound;
            }
        }
    }
    if( nFound < 2 )
        return nullptr;

    bool bOkay = true;
    long nMid = ( nMin + nMax ) / 2;

    auto pRet(o3tl::make_unique<SwBoxSelection>());
    std::vector< std::pair< SwTableBox*, long > > aNewWidthVector;
    size_t nCheckBottom = nBottom;
    long nLeftSpan = 0;
    long nRightSpan = 0;
    long nLeftSpanCnt = 0;
    long nRightSpanCnt = 0;
    for( size_t nRow = nTop; nRow <= nBottom && bOkay && nRow < nLines; ++nRow )
    {
        SwTableLine* pLine = m_aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        SwSelBoxes aBoxes;
        long nRight = 0;
        const size_t nCount = pLine->GetTabBoxes().size();
        for( size_t nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox].get();
            OSL_ENSURE( pBox, "Missing table box" );
            long nLeft = nRight;
            nRight += pBox->GetFrameFormat()->GetFrameSize().GetWidth();
            long nRowSpan = pBox->getRowSpan();
            if( nRight <= nMin )
            {
                if( nRight == nMin && nLeftSpanCnt )
                    bOkay = false;
                continue;
            }
            SwTableBox* pInnerBox = nullptr;
            SwTableBox* pLeftBox = nullptr;
            SwTableBox* pRightBox = nullptr;
            long nDiff = 0;
            long nDiff2 = 0;
            if( nLeft < nMin )
            {
                if( nRight >= nMid || nRight + nLeft >= nMin + nMin )
                {
                    if( nCurrBox )
                    {
                        aBoxes.insert(pBox);
                        pInnerBox = pBox;
                        pLeftBox = pLine->GetTabBoxes()[nCurrBox-1].get();
                        nDiff = nMin - nLeft;
                        if( nRight > nMax )
                        {
                            if( nCurrBox+1 < nCount )
                            {
                                pRightBox = pLine->GetTabBoxes()[nCurrBox+1].get();
                                nDiff2 = nRight - nMax;
                            }
                            else
                                bOkay = false;
                        }
                        else if( nRightSpanCnt && nRight == nMax )
                            bOkay = false;
                    }
                    else
                        bOkay = false;
                }
                else if( nCurrBox+1 < nCount )
                {
                    pLeftBox = pBox;
                    pInnerBox = pLine->GetTabBoxes()[nCurrBox+1].get();
                    nDiff = nMin - nRight;
                }
                else
                    bOkay = false;
            }
            else if( nRight <= nMax )
            {
                aBoxes.insert(pBox);
                if( nRow == nTop && nRowSpan < 0 )
                {
                    bOkay = false;
                    break;
                }
                if( nRowSpan > 1 && nRow + nRowSpan - 1 > nBottom )
                    nBottom = nRow + nRowSpan - 1;
                if( nRowSpan < -1 && nRow - nRowSpan - 1 > nBottom )
                    nBottom = nRow - nRowSpan - 1;
                if( nRightSpanCnt && nRight == nMax )
                    bOkay = false;
            }
            else if( nLeft < nMax )
            {
                if( nLeft <= nMid || nRight + nLeft <= nMax )
                {
                    if( nCurrBox+1 < nCount )
                    {
                        aBoxes.insert(pBox);
                        pInnerBox = pBox;
                        pRightBox = pLine->GetTabBoxes()[nCurrBox+1].get();
                        nDiff = nRight - nMax;
                    }
                    else
                        bOkay = false;
                }
                else if( nCurrBox )
                {
                    pRightBox = pBox;
                    pInnerBox = pLine->GetTabBoxes()[nCurrBox-1].get();
                    nDiff = nLeft - nMax;
                }
                else
                    bOkay = false;
            }
            else
                break;
            if( pInnerBox )
            {
                if( nRow == nBottom )
                {
                    long nTmpSpan = pInnerBox->getRowSpan();
                    if( nTmpSpan > 1 )
                        nBottom += nTmpSpan - 1;
                    else if( nTmpSpan < -1 )
                        nBottom -= nTmpSpan + 1;
                }
                SwTableBox* pOuterBox = pLeftBox;
                do
                {
                    if( pOuterBox )
                    {
                        long nOutSpan = pOuterBox->getRowSpan();
                        if( nOutSpan != 1 )
                        {
                            size_t nCheck = nRow;
                            if( nOutSpan < 0 )
                            {
                                const SwTableBox& rBox =
                                    pOuterBox->FindStartOfRowSpan( *this );
                                nOutSpan = rBox.getRowSpan();
                                const SwTableLine* pTmpL = rBox.GetUpper();
                                nCheck = GetTabLines().GetPos( pTmpL );
                                if( nCheck < nTop )
                                    bOkay = false;
                                if( pOuterBox == pLeftBox )
                                {
                                    if( !nLeftSpanCnt || nMin - nDiff != nLeftSpan )
                                        bOkay = false;
                                }
                                else
                                {
                                    if( !nRightSpanCnt || nMax + nDiff != nRightSpan )
                                        bOkay = false;
                                }
                            }
                            else
                            {
                                if( pOuterBox == pLeftBox )
                                {
                                    if( nLeftSpanCnt )
                                        bOkay = false;
                                    nLeftSpan = nMin - nDiff;
                                    nLeftSpanCnt = nOutSpan;
                                }
                                else
                                {
                                    if( nRightSpanCnt )
                                        bOkay = false;
                                    nRightSpan = nMax + nDiff;
                                    nRightSpanCnt = nOutSpan;
                                }
                            }
                            nCheck += nOutSpan - 1;
                            if( nCheck > nCheckBottom )
                                nCheckBottom = nCheck;
                        }
                        else if( ( nLeftSpanCnt && pLeftBox == pOuterBox ) ||
                            ( nRightSpanCnt && pRightBox == pOuterBox ) )
                            bOkay = false;
                        std::pair< SwTableBox*, long > aTmp;
                        aTmp.first = pInnerBox;
                        aTmp.second = -nDiff;
                        aNewWidthVector.push_back(aTmp);
                        aTmp.first = pOuterBox;
                        aTmp.second = nDiff;
                        aNewWidthVector.push_back(aTmp);
                    }
                    pOuterBox = pOuterBox == pRightBox ? nullptr : pRightBox;
                    if( nDiff2 )
                        nDiff = nDiff2;
                } while( pOuterBox );
            }
        }
        if( nLeftSpanCnt )
            --nLeftSpanCnt;
        if( nRightSpanCnt )
            --nRightSpanCnt;
        pRet->push_back(aBoxes);
    }
    if( nCheckBottom > nBottom )
        bOkay = false;
    if( bOkay )
    {
        pRet->mnMergeWidth = nMax - nMin;
        for (auto const& newWidth : aNewWidthVector)
        {
            SwFrameFormat* pFormat = newWidth.first->ClaimFrameFormat();
            long nNewWidth = pFormat->GetFrameSize().GetWidth() + newWidth.second;
            pFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, nNewWidth, 0 ) );
        }
    }
    else
        pRet.reset();

    return pRet;
}

/** lcl_InvalidateCellFrame(..) invalidates all layout representations of a given cell
    to initiate a reformatting
*/

static void lcl_InvalidateCellFrame( const SwTableBox& rBox )
{
    SwIterator<SwCellFrame,SwFormat> aIter( *rBox.GetFrameFormat() );
    for( SwCellFrame* pCell = aIter.First(); pCell; pCell = aIter.Next() )
    {
        if( pCell->GetTabBox() == &rBox )
        {
            pCell->InvalidateSize();
            SwFrame* pLower = pCell->GetLower();
            if( pLower )
                pLower->InvalidateSize_();
        }
    }
}

/** lcl_InsertPosition(..) evaluates the insert positions in every table line,
    when a selection of cells is given and returns the average cell widths
*/

static long lcl_InsertPosition( SwTable &rTable, std::vector<sal_uInt16>& rInsPos,
    const SwSelBoxes& rBoxes, bool bBehind )
{
    sal_Int32 nAddWidth = 0;
    long nCount = 0;
    for (size_t j = 0; j < rBoxes.size(); ++j)
    {
        SwTableBox *pBox = rBoxes[j];
        SwTableLine* pLine = pBox->GetUpper();
        long nWidth = rBoxes[j]->GetFrameFormat()->GetFrameSize().GetWidth();
        nAddWidth += nWidth;
        sal_uInt16 nCurrBox = pLine->GetBoxPos( pBox );
        sal_uInt16 nCurrLine = rTable.GetTabLines().GetPos( pLine );
        OSL_ENSURE( nCurrLine != USHRT_MAX, "Time to say Good-Bye.." );
        if( rInsPos[ nCurrLine ] == USHRT_MAX )
        {
            rInsPos[ nCurrLine ] = nCurrBox;
            ++nCount;
        }
        else if( ( rInsPos[ nCurrLine ] > nCurrBox ) == !bBehind )
            rInsPos[ nCurrLine ] = nCurrBox;
    }
    if( nCount )
        nAddWidth /= nCount;
    return nAddWidth;
}

/** SwTable::NewInsertCol(..) insert new column(s) into a table

@param pDoc
the document

@param rBoxes
the selected boxes

@param nCnt
the number of columns to insert

@param bBehind
insertion behind (true) or before (false) the selected boxes

@return true, if any insertion has been done successfully

*/

bool SwTable::NewInsertCol( SwDoc* pDoc, const SwSelBoxes& rBoxes,
    sal_uInt16 nCnt, bool bBehind )
{
    if( m_aLines.empty() || !nCnt )
        return false;

    CHECK_TABLE( *this )
    long nNewBoxWidth = 0;
    std::vector< sal_uInt16 > aInsPos( m_aLines.size(), USHRT_MAX );
    { // Calculation of the insert positions and the width of the new boxes
        sal_uInt64 nTableWidth = 0;
        for( size_t i = 0; i < m_aLines[0]->GetTabBoxes().size(); ++i )
            nTableWidth += m_aLines[0]->GetTabBoxes()[i]->GetFrameFormat()->GetFrameSize().GetWidth();

        // Fill the vector of insert positions and the (average) width to insert
        sal_uInt64 nAddWidth = lcl_InsertPosition( *this, aInsPos, rBoxes, bBehind );

        // Given is the (average) width of the selected boxes, if we would
        // insert nCnt of columns the table would grow
        // So we will shrink the table first, then insert the new boxes and
        // get a table with the same width than before.
        // But we will not shrink the table by the full already calculated value,
        // we will reduce this value proportional to the old table width
        nAddWidth *= nCnt; // we have to insert nCnt boxes per line
        sal_uInt64 nResultingWidth = nAddWidth + nTableWidth;
        if( !nResultingWidth )
            return false;
        nAddWidth = (nAddWidth * nTableWidth) / nResultingWidth;
        nNewBoxWidth = long( nAddWidth / nCnt ); // Rounding
        nAddWidth = nNewBoxWidth * nCnt; // Rounding
        if( !nAddWidth || nAddWidth >= nTableWidth )
            return false;
        AdjustWidths( static_cast< long >(nTableWidth), static_cast< long >(nTableWidth - nAddWidth) );
    }

    FndBox_ aFndBox( nullptr, nullptr );
    aFndBox.SetTableLines( rBoxes, *this );
    aFndBox.DelFrames( *this );

    SwTableNode* pTableNd = GetTableNode();
    std::vector<SwTableBoxFormat*> aInsFormat( nCnt, nullptr );
    size_t nLastLine = SAL_MAX_SIZE;
    long nLastRowSpan = 1;

    for( size_t i = 0; i < m_aLines.size(); ++i )
    {
        SwTableLine* pLine = m_aLines[ i ];
        sal_uInt16 nInsPos = aInsPos[i];
        assert(nInsPos != USHRT_MAX); // didn't find insert position
        SwTableBox* pBox = pLine->GetTabBoxes()[ nInsPos ].get();
        if( bBehind )
            ++nInsPos;
        SwTableBoxFormat* pBoxFrameFormat = static_cast<SwTableBoxFormat*>(pBox->GetFrameFormat());
        ::InsTableBox( pDoc, pTableNd, pLine, pBoxFrameFormat, pBox, nInsPos, nCnt );
        long nRowSpan = pBox->getRowSpan();
        long nDiff = i - nLastLine;
        bool bNewSpan = false;
        if( nLastLine != SAL_MAX_SIZE && nDiff <= nLastRowSpan &&
            nRowSpan != nDiff - nLastRowSpan )
        {
            bNewSpan = true;
            while( nLastLine < i )
            {
                SwTableLine* pTmpLine = m_aLines[ nLastLine ];
                sal_uInt16 nTmpPos = aInsPos[nLastLine];
                if( bBehind )
                    ++nTmpPos;
                for( sal_uInt16 j = 0; j < nCnt; ++j )
                    pTmpLine->GetTabBoxes()[nTmpPos+j]->setRowSpan( nDiff );
                if( nDiff > 0 )
                    nDiff = -nDiff;
                ++nDiff;
                ++nLastLine;
            }
        }
        if( nRowSpan > 0 )
            bNewSpan = true;
        if( bNewSpan )
        {
            nLastLine = i;
            if( nRowSpan < 0 )
                nLastRowSpan = -nRowSpan;
            else
                nLastRowSpan = nRowSpan;
        }
        const SvxBoxItem& aSelBoxItem = pBoxFrameFormat->GetBox();
        SvxBoxItem* pNoRightBorder = nullptr;
        if( aSelBoxItem.GetRight() )
        {
            pNoRightBorder = new SvxBoxItem( aSelBoxItem );
            pNoRightBorder->SetLine( nullptr, SvxBoxItemLine::RIGHT );
        }
        for( sal_uInt16 j = 0; j < nCnt; ++j )
        {
            SwTableBox *pCurrBox = pLine->GetTabBoxes()[nInsPos+j].get();
            if( bNewSpan )
            {
                pCurrBox->setRowSpan( nLastRowSpan );
                SwFrameFormat* pFrameFormat = pCurrBox->ClaimFrameFormat();
                SwFormatFrameSize aFrameSz( pFrameFormat->GetFrameSize() );
                aFrameSz.SetWidth( nNewBoxWidth );
                pFrameFormat->SetFormatAttr( aFrameSz );
                if( pNoRightBorder && ( !bBehind || j+1 < nCnt ) )
                    pFrameFormat->SetFormatAttr( *pNoRightBorder );
                aInsFormat[j] = static_cast<SwTableBoxFormat*>(pFrameFormat);
            }
            else
                pCurrBox->ChgFrameFormat( aInsFormat[j] );
        }
        if( bBehind && pNoRightBorder )
        {
            SwFrameFormat* pFrameFormat = pBox->ClaimFrameFormat();
            pFrameFormat->SetFormatAttr( *pNoRightBorder );
        }
        delete pNoRightBorder;
    }

    aFndBox.MakeFrames( *this );
#if OSL_DEBUG_LEVEL > 0
    {
        const SwTableBoxes &rTabBoxes = m_aLines[0]->GetTabBoxes();
        long nNewWidth = 0;
        for( size_t i = 0; i < rTabBoxes.size(); ++i )
            nNewWidth += rTabBoxes[i]->GetFrameFormat()->GetFrameSize().GetWidth();
        OSL_ENSURE( nNewWidth > 0, "Very small" );
    }
#endif
    CHECK_TABLE( *this )

    return true;
}

/** SwTable::PrepareMerge(..) some preparation for the coming Merge(..)

For the old table model, ::GetMergeSel(..) is called only,
for the new table model, PrepareMerge does the main work.
It modifies all cells to merge (width, border, rowspan etc.) and collects
the cells which have to be deleted by Merge(..) afterwards.
If there are superfluous rows, these cells are put into the deletion list as well.

@param rPam
the selection to merge

@param rBoxes
should be empty at the beginning, at the end it is filled with boxes to delete.

@param ppMergeBox
will be set to the master cell box

@param pUndo
the undo object to record all changes
can be Null, e.g. when called by Redo(..)

@return

*/

bool SwTable::PrepareMerge( const SwPaM& rPam, SwSelBoxes& rBoxes,
   SwSelBoxes& rMerged, SwTableBox** ppMergeBox, SwUndoTableMerge* pUndo )
{
    if( !m_bNewModel )
    {
        ::GetMergeSel( rPam, rBoxes, ppMergeBox, pUndo );
        return rBoxes.size() > 1;
    }
    CHECK_TABLE( *this )
    // We have to assert a "rectangular" box selection before we start to merge
    std::unique_ptr< SwBoxSelection > pSel( CollectBoxSelection( rPam ) );
    if( !pSel.get() || pSel->isEmpty() )
        return false;
    // Now we should have a rectangle of boxes,
    // i.e. contiguous cells in contiguous rows
    bool bMerge = false; // will be set if any content is transferred from
    // a "not already overlapped" cell into the new master cell.
    const SwSelBoxes& rFirstBoxes = pSel->maBoxes[0];
    if (rFirstBoxes.empty())
        return false;
    SwTableBox *pMergeBox = rFirstBoxes[0]; // the master cell box
    if( !pMergeBox )
        return false;
    (*ppMergeBox) = pMergeBox;
    // The new master box will get the left and the top border of the top-left
    // box of the selection and because the new master cell _is_ the top-left
    // box, the left and right border does not need to be changed.
    // The right and bottom border instead has to be derived from the right-
    // bottom box of the selection. If this is a overlapped cell,
    // the appropriate master box.
    SwTableBox* pLastBox = nullptr; // the right-bottom (master) cell
    SwDoc* pDoc = GetFrameFormat()->GetDoc();
    SwPosition aInsPos( *pMergeBox->GetSttNd()->EndOfSectionNode() );
    SwPaM aChkPam( aInsPos );
    // The number of lines in the selection rectangle: nLineCount
    const size_t nLineCount = pSel->maBoxes.size();
    // BTW: nLineCount is the rowspan of the new master cell
    long nRowSpan = static_cast<long>(nLineCount);
    // We will need the first and last line of the selection
    // to check if there any superfluous row after merging
    SwTableLine* pFirstLn = nullptr;
    SwTableLine* pLastLn = nullptr;
    // Iteration over the lines of the selection...
    for( size_t nCurrLine = 0; nCurrLine < nLineCount; ++nCurrLine )
    {
        // The selected boxes in the current line
        const SwSelBoxes& rLineBoxes = pSel->maBoxes[nCurrLine];
        size_t nColCount = rLineBoxes.size();
        // Iteration over the selected cell in the current row
        for (size_t nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol)
        {
            SwTableBox* pBox = rLineBoxes[nCurrCol];
            rMerged.insert( pBox );
            // Only the first selected cell in every row will be alive,
            // the other will be deleted => put into rBoxes
            if( nCurrCol )
                rBoxes.insert( pBox );
            else
            {
                if( nCurrLine == 1 )
                    pFirstLn = pBox->GetUpper(); // we need this line later on
                if( nCurrLine + 1 == nLineCount )
                    pLastLn = pBox->GetUpper(); // and this one, too.
            }
            // A box has to be merged if it's not the master box itself,
            // but an already overlapped cell must not be merged as well.
            bool bDoMerge = pBox != pMergeBox && pBox->getRowSpan() > 0;
            // The last box has to be in the last "column" of the selection
            // and it has to be a master cell
            if( nCurrCol+1 == nColCount && pBox->getRowSpan() > 0 )
                pLastBox = pBox;
            if( bDoMerge )
            {
                bMerge = true;
                // If the cell to merge contains only one empty paragraph,
                // we do not transfer this paragraph.
                if( !IsEmptyBox( *pBox, aChkPam ) )
                {
                    SwNodeIndex& rInsPosNd = aInsPos.nNode;
                    SwPaM aPam( aInsPos );
                    aPam.GetPoint()->nNode.Assign( *pBox->GetSttNd()->EndOfSectionNode(), -1 );
                    SwContentNode* pCNd = aPam.GetContentNode();
                    aPam.GetPoint()->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );
                    SwNodeIndex aSttNdIdx( *pBox->GetSttNd(), 1 );
                    bool const bUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
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
                    if( pUndo )
                        pUndo->MoveBoxContent( pDoc, aRg, rInsPosNd );
                    else
                    {
                        pDoc->getIDocumentContentOperations().MoveNodeRange( aRg, rInsPosNd,
                            SwMoveFlags::NO_DELFRMS );
                    }
                }
            }
            // Only the cell of the first selected column will stay alive
            // and got a new row span
            if( !nCurrCol )
                pBox->setRowSpan( nRowSpan );
        }
        if( nRowSpan > 0 ) // the master cell is done, from now on we set
            nRowSpan = -nRowSpan; // negative row spans
        ++nRowSpan; // ... -3, -2, -1
    }
    if( bMerge )
    {
        // A row containing overlapped cells is superfluous,
        // these cells can be put into rBoxes for deletion
        FindSuperfluousRows_( rBoxes, pFirstLn, pLastLn );
        // pNewFormat will be set to the new master box and the overlapped cells
        SwFrameFormat* pNewFormat = pMergeBox->ClaimFrameFormat();
        pNewFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, pSel->mnMergeWidth, 0 ) );
        for( size_t nCurrLine = 0; nCurrLine < nLineCount; ++nCurrLine )
        {
            const SwSelBoxes& rLineBoxes = pSel->maBoxes[nCurrLine];
            size_t nColCount = rLineBoxes.size();
            for (size_t nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol)
            {
                SwTableBox* pBox = rLineBoxes[nCurrCol];
                if( nCurrCol )
                {
                    // Even this box will be deleted soon,
                    // we have to correct the width to avoid side effects
                    SwFrameFormat* pFormat = pBox->ClaimFrameFormat();
                    pFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, 0, 0 ) );
                }
                else
                {
                    pBox->ChgFrameFormat( static_cast<SwTableBoxFormat*>(pNewFormat) );
                    // remove numbering from cells that will be disabled in the merge
                    if( nCurrLine )
                    {
                        SwPaM aPam( *pBox->GetSttNd(), 0 );
                        aPam.GetPoint()->nNode++;
                        SwTextNode* pNd = aPam.GetNode().GetTextNode();
                        while( pNd )
                        {
                            pNd->SetCountedInList( false );

                            aPam.GetPoint()->nNode++;
                            pNd = aPam.GetNode().GetTextNode();
                        }
                    }
                }
            }
        }
        if( pLastBox ) // Robust
        {
            // The new borders of the master cell...
            SvxBoxItem aBox( pMergeBox->GetFrameFormat()->GetBox() );
            bool bOld = aBox.GetRight() || aBox.GetBottom();
            const SvxBoxItem& rBox = pLastBox->GetFrameFormat()->GetBox();
            aBox.SetLine( rBox.GetRight(), SvxBoxItemLine::RIGHT );
            aBox.SetLine( rBox.GetBottom(), SvxBoxItemLine::BOTTOM );
            if( bOld || aBox.GetLeft() || aBox.GetTop() || aBox.GetRight() || aBox.GetBottom() )
                (*ppMergeBox)->GetFrameFormat()->SetFormatAttr( aBox );
        }

        if( pUndo )
            pUndo->AddNewBox( pMergeBox->GetSttIdx() );
    }
    return bMerge;
}

/** SwTable::FindSuperfluousRows_(..) is looking for superfluous rows, i.e. rows
    containing overlapped cells only.
*/

void SwTable::FindSuperfluousRows_( SwSelBoxes& rBoxes,
    SwTableLine* pFirstLn, SwTableLine* pLastLn )
{
    if( !pFirstLn || !pLastLn )
    {
        if( rBoxes.empty() )
            return;
        pFirstLn = rBoxes[0]->GetUpper();
        pLastLn = rBoxes.back()->GetUpper();
    }
    sal_uInt16 nFirstLn = GetTabLines().GetPos( pFirstLn );
    sal_uInt16 nLastLn = GetTabLines().GetPos( pLastLn );
    for( sal_uInt16 nRow = nFirstLn; nRow <= nLastLn; ++nRow )
    {
        SwTableLine* pLine = m_aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        const size_t nCols = pLine->GetTabBoxes().size();
        bool bSuperfl = true;
        for( size_t nCol = 0; nCol < nCols; ++nCol )
        {
            SwTableBox *pBox = pLine->GetTabBoxes()[nCol].get();
            if( pBox->getRowSpan() > 0 &&
                rBoxes.end() == rBoxes.find( pBox ) )
            {
                bSuperfl = false;
                break;
            }
        }
        if( bSuperfl )
        {
            for( size_t nCol = 0; nCol < nCols; ++nCol )
            {
                SwTableBox* pBox = pLine->GetTabBoxes()[nCol].get();
                rBoxes.insert( pBox );
            }
        }
    }
}

/** SwTableBox::FindStartOfRowSpan(..) returns the "master" cell, the cell which
    overlaps the given cell, it maybe the cell itself.
*/

SwTableBox& SwTableBox::FindStartOfRowSpan( const SwTable& rTable, sal_uInt16 nMaxStep )
{
    if( getRowSpan() > 0 || !nMaxStep )
        return *this;

    long nLeftBorder = lcl_Box2LeftBorder( *this );
    SwTableBox* pBox = this;
    const SwTableLine* pMyUpper = GetUpper();
    sal_uInt16 nLine = rTable.GetTabLines().GetPos( pMyUpper );
    if( nLine && nLine < rTable.GetTabLines().size() )
    {
        SwTableBox* pNext;
        do
        {
            pNext = lcl_LeftBorder2Box( nLeftBorder, rTable.GetTabLines()[--nLine] );
            if( pNext )
                pBox = pNext;
        } while( nLine && --nMaxStep && pNext && pBox->getRowSpan() < 1 );
    }

    return *pBox;
}

/** SwTableBox::FindEndOfRowSpan(..) returns the last overlapped cell if there is
    any. Otherwise the cell itself will returned.
*/

SwTableBox& SwTableBox::FindEndOfRowSpan( const SwTable& rTable, sal_uInt16 nMaxStep )
{
    long nAbsSpan = getRowSpan();
    if( nAbsSpan < 0 )
        nAbsSpan = -nAbsSpan;
    if( nAbsSpan == 1 || !nMaxStep )
        return *this;

    if( nMaxStep > --nAbsSpan )
        nMaxStep = static_cast<sal_uInt16>(nAbsSpan);
    const SwTableLine* pMyUpper = GetUpper();
    sal_uInt16 nLine = rTable.GetTabLines().GetPos( pMyUpper );
    nMaxStep = nLine + nMaxStep;
    if( nMaxStep >= rTable.GetTabLines().size() )
        nMaxStep = rTable.GetTabLines().size() - 1;
    long nLeftBorder = lcl_Box2LeftBorder( *this );
    SwTableBox* pBox =
        lcl_LeftBorder2Box( nLeftBorder, rTable.GetTabLines()[ nMaxStep ] );
    if ( !pBox )
        pBox = this;

    return *pBox;
}

/** lcl_getAllMergedBoxes(..) collects all overlapped boxes to a given (master) box
*/

static void lcl_getAllMergedBoxes( const SwTable& rTable, SwSelBoxes& rBoxes, SwTableBox& rBox )
{
    SwTableBox* pBox = &rBox;
    OSL_ENSURE( pBox == &rBox.FindStartOfRowSpan( rTable ), "Not a master box" );
    rBoxes.insert( pBox );
    if( pBox->getRowSpan() == 1 )
        return;
    const SwTableLine* pMyUpper = pBox->GetUpper();
    sal_uInt16 nLine = rTable.GetTabLines().GetPos( pMyUpper );
    long nLeftBorder = lcl_Box2LeftBorder( *pBox );
    sal_uInt16 nCount = rTable.GetTabLines().size();
    while( ++nLine < nCount && pBox && pBox->getRowSpan() != -1 )
    {
        pBox = lcl_LeftBorder2Box( nLeftBorder, rTable.GetTabLines()[nLine] );
        if( pBox )
            rBoxes.insert( pBox );
    }
}

/** lcl_UnMerge(..) manipulates the row span attribute of a given master cell
    and its overlapped cells to split them into several pieces.
*/

static void lcl_UnMerge( const SwTable& rTable, SwTableBox& rBox, size_t nCnt,
    bool bSameHeight )
{
    SwSelBoxes aBoxes;
    lcl_getAllMergedBoxes( rTable, aBoxes, rBox );
    size_t const nCount = aBoxes.size();
    if( nCount < 2 )
        return;
    if( nCnt > nCount )
        nCnt = nCount;
    std::unique_ptr<size_t[]> const pSplitIdx(new size_t[nCnt]);
    if( bSameHeight )
    {
        std::unique_ptr<SwTwips[]> const pHeights(new SwTwips[nCount]);
        SwTwips nHeight = 0;
        for (size_t i = 0; i < nCount; ++i)
        {
            SwTableLine* pLine = aBoxes[ i ]->GetUpper();
            SwFrameFormat *pRowFormat = pLine->GetFrameFormat();
            pHeights[ i ] = pRowFormat->GetFrameSize().GetHeight();
            nHeight += pHeights[ i ];
        }
        SwTwips nSumH = 0;
        size_t nIdx = 0;
        for (size_t i = 1; i <= nCnt; ++i)
        {
            SwTwips nSplit = ( i * nHeight ) / nCnt;
            while( nSumH < nSplit && nIdx < nCount )
                nSumH += pHeights[ nIdx++ ];
            pSplitIdx[ i - 1 ] = nIdx;
        }
    }
    else
    {
        for (size_t i = 1; i <= nCnt; ++i)
        {
            pSplitIdx[ i - 1 ] = ( i * nCount ) / nCnt;
        }
    }
    size_t nIdx = 0;
    for (size_t i = 0; i < nCnt; ++i)
    {
        size_t nNextIdx = pSplitIdx[ i ];
        aBoxes[ nIdx ]->setRowSpan( nNextIdx - nIdx );
        lcl_InvalidateCellFrame( *aBoxes[ nIdx ] );
        while( ++nIdx < nNextIdx )
            aBoxes[ nIdx ]->setRowSpan( nIdx - nNextIdx );
    }
}

/** lcl_FillSelBoxes(..) puts all boxes of a given line into the selection structure
*/

static void lcl_FillSelBoxes( SwSelBoxes &rBoxes, SwTableLine &rLine )
{
    const size_t nBoxCount = rLine.GetTabBoxes().size();
    for( size_t i = 0; i < nBoxCount; ++i )
        rBoxes.insert( rLine.GetTabBoxes()[i].get() );
}

/** SwTable::InsertSpannedRow(..) inserts "superfluous" rows, i.e. rows containing
    overlapped cells only. This is a preparation for an upcoming split.
*/

void SwTable::InsertSpannedRow( SwDoc* pDoc, sal_uInt16 nRowIdx, sal_uInt16 nCnt )
{
    CHECK_TABLE( *this )
    OSL_ENSURE( nCnt && nRowIdx < GetTabLines().size(), "Wrong call of InsertSpannedRow" );
    SwSelBoxes aBoxes;
    SwTableLine& rLine = *GetTabLines()[ nRowIdx ];
    lcl_FillSelBoxes( aBoxes, rLine );
    SwFormatFrameSize aFSz( rLine.GetFrameFormat()->GetFrameSize() );
    if( ATT_VAR_SIZE != aFSz.GetHeightSizeType() )
    {
        SwFrameFormat* pFrameFormat = rLine.ClaimFrameFormat();
        long nNewHeight = aFSz.GetHeight() / ( nCnt + 1 );
        if( !nNewHeight )
            ++nNewHeight;
        aFSz.SetHeight( nNewHeight );
        pFrameFormat->SetFormatAttr( aFSz );
    }
    InsertRow_( pDoc, aBoxes, nCnt, true );
    const size_t nBoxCount = rLine.GetTabBoxes().size();
    for( sal_uInt16 n = 0; n < nCnt; ++n )
    {
        SwTableLine *pNewLine = GetTabLines()[ nRowIdx + nCnt - n ];
        for( size_t nCurrBox = 0; nCurrBox < nBoxCount; ++nCurrBox )
        {
            long nRowSpan = rLine.GetTabBoxes()[nCurrBox]->getRowSpan();
            if( nRowSpan > 0 )
                nRowSpan = - nRowSpan;
            pNewLine->GetTabBoxes()[ nCurrBox ]->setRowSpan( nRowSpan - n );
        }
    }
    lcl_ChangeRowSpan( *this, nCnt, nRowIdx, false );
    CHECK_TABLE( *this )
}

typedef std::pair< sal_uInt16, sal_uInt16 > SwLineOffset;
typedef std::list< SwLineOffset > SwLineOffsetArray;

/*
* When a couple of table boxes has to be split,
* lcl_SophisticatedFillLineIndices delivers the information where and how many
* rows have to be inserted.
* Input
*     rTable: the table to manipulate
*     rBoxes: an array of boxes to split
*     nCnt:   how many parts are wanted
* Output
*     rArr:   a list of pairs ( line index, number of lines to insert )
*/
static void lcl_SophisticatedFillLineIndices( SwLineOffsetArray &rArr,
    const SwTable& rTable, const SwSelBoxes& rBoxes, sal_uInt16 nCnt )
{
    std::list< SwLineOffset > aBoxes;
    SwLineOffset aLnOfs( USHRT_MAX, USHRT_MAX );
    for (size_t i = 0; i < rBoxes.size(); ++i)
    {   // Collect all end line indices and the row spans
        const SwTableBox &rBox = rBoxes[ i ]->FindStartOfRowSpan( rTable );
        OSL_ENSURE( rBox.getRowSpan() > 0, "Didn't I say 'StartOfRowSpan' ??" );
        if( nCnt > rBox.getRowSpan() )
        {
            const SwTableLine *pLine = rBox.GetUpper();
            const sal_uInt16 nEnd = sal_uInt16( rBox.getRowSpan() +
                rTable.GetTabLines().GetPos(  pLine ) );
            // The next if statement is a small optimization
            if( aLnOfs.first != nEnd || aLnOfs.second != rBox.getRowSpan() )
            {
                aLnOfs.first = nEnd; // ok, this is the line behind the box
                aLnOfs.second = sal_uInt16( rBox.getRowSpan() ); // the row span
                aBoxes.insert( aBoxes.end(), aLnOfs );
            }
        }
    }
    // As I said, I noted the line index _behind_ the last line of the boxes
    // in the resulting array the index has to be _on_ the line
    // nSum is to evaluate the wished value
    sal_uInt16 nSum = 1;
    while( !aBoxes.empty() )
    {
        // I. step:
        // Looking for the "smallest" line end with the smallest row span
        std::list< SwLineOffset >::iterator pCurr = aBoxes.begin();
        aLnOfs = *pCurr; // the line end and row span of the first box
        while( ++pCurr != aBoxes.end() )
        {
            if( aLnOfs.first > pCurr->first )
            {   // Found a smaller line end
                aLnOfs.first = pCurr->first;
                aLnOfs.second = pCurr->second; // row span
            }
            else if( aLnOfs.first == pCurr->first &&
                     aLnOfs.second < pCurr->second )
                aLnOfs.second = pCurr->second; // Found a smaller row span
        }
        OSL_ENSURE( aLnOfs.second < nCnt, "Clean-up failed" );
        aLnOfs.second = nCnt - aLnOfs.second; // the number of rows to insert
        rArr.insert( rArr.end(),
            SwLineOffset( aLnOfs.first - nSum, aLnOfs.second ) );
        // the correction has to be incremented because in the following
        // loops the line ends were manipulated
        nSum = nSum + aLnOfs.second;

        pCurr = aBoxes.begin();
        while( pCurr != aBoxes.end() )
        {
            if( pCurr->first == aLnOfs.first )
            {   // These boxes can be removed because the last insertion
                // of rows will expand their row span above the needed value
                pCurr = aBoxes.erase(pCurr);
            }
            else
            {
                bool bBefore = ( pCurr->first - pCurr->second < aLnOfs.first );
                // Manipulation of the end line indices as if the rows are
                // already inserted
                pCurr->first = pCurr->first + aLnOfs.second;
                if( bBefore )
                {   // If the insertion is inside the box,
                    // its row span has to be incremented
                    pCurr->second = pCurr->second + aLnOfs.second;
                    if( pCurr->second >= nCnt )
                    {   // if the row span is bigger than the split factor
                        // this box is done
                        pCurr = aBoxes.erase(pCurr);
                    }
                    else
                        ++pCurr;
                }
                else
                    ++pCurr;
            }
        }
    }
}

typedef std::set< SwTwips > SwSplitLines;

/** lcl_CalculateSplitLineHeights(..) delivers all y-positions where table rows have
    to be split to fulfill the requested "split same height"
*/

static sal_uInt16 lcl_CalculateSplitLineHeights( SwSplitLines &rCurr, SwSplitLines &rNew,
    const SwTable& rTable, const SwSelBoxes& rBoxes, sal_uInt16 nCnt )
{
    if( nCnt < 2 )
        return 0;
    std::list< SwLineOffset > aBoxes;
    SwLineOffset aLnOfs( USHRT_MAX, USHRT_MAX );
    sal_uInt16 nFirst = USHRT_MAX; // becomes the index of the first line
    sal_uInt16 nLast = 0; // becomes the index of the last line of the splitting
    for (size_t i = 0; i < rBoxes.size(); ++i)
    {   // Collect all pairs (start+end) of line indices to split
        const SwTableBox &rBox = rBoxes[ i ]->FindStartOfRowSpan( rTable );
        OSL_ENSURE( rBox.getRowSpan() > 0, "Didn't I say 'StartOfRowSpan' ??" );
        const SwTableLine *pLine = rBox.GetUpper();
        const sal_uInt16 nStart = rTable.GetTabLines().GetPos( pLine );
        const sal_uInt16 nEnd = sal_uInt16( rBox.getRowSpan() + nStart - 1 );
        // The next if statement is a small optimization
        if( aLnOfs.first != nStart || aLnOfs.second != nEnd )
        {
            aLnOfs.first = nStart;
            aLnOfs.second = nEnd;
            aBoxes.insert( aBoxes.end(), aLnOfs );
            if( nStart < nFirst )
                nFirst = nStart;
            if( nEnd > nLast )
                nLast = nEnd;
        }
    }

    if (nFirst == USHRT_MAX)
    {
        assert(aBoxes.empty());
        return 0;
    }

    SwTwips nHeight = 0;
    std::unique_ptr<SwTwips[]> pLines(new SwTwips[ nLast + 1 - nFirst ]);
    for( sal_uInt16 i = nFirst; i <= nLast; ++i )
    {
        bool bLayoutAvailable = false;
        nHeight += rTable.GetTabLines()[ i ]->GetTableLineHeight( bLayoutAvailable );
        rCurr.insert( rCurr.end(), nHeight );
        pLines[ i - nFirst ] = nHeight;
    }
    std::list< SwLineOffset >::iterator pSplit = aBoxes.begin();
    while( pSplit != aBoxes.end() )
    {
        SwTwips nBase = pSplit->first <= nFirst ? 0 :
                        pLines[ pSplit->first - nFirst - 1 ];
        SwTwips nDiff = pLines[ pSplit->second - nFirst ] - nBase;
        for( sal_uInt16 i = 1; i < nCnt; ++i )
        {
            SwTwips nSplit = nBase + ( i * nDiff ) / nCnt;
            rNew.insert( nSplit );
        }
        ++pSplit;
    }
    return nFirst;
}

/** lcl_LineIndex(..) delivers the line index of the line behind or above
    the box selection.
*/

static sal_uInt16 lcl_LineIndex( const SwTable& rTable, const SwSelBoxes& rBoxes,
                      bool bBehind )
{
    sal_uInt16 nDirect = USHRT_MAX;
    sal_uInt16 nSpan = USHRT_MAX;
    for (size_t i = 0; i < rBoxes.size(); ++i)
    {
        SwTableBox *pBox = rBoxes[i];
        const SwTableLine* pLine = rBoxes[i]->GetUpper();
        sal_uInt16 nPos = rTable.GetTabLines().GetPos( pLine );
        if( USHRT_MAX != nPos )
        {
            if( bBehind )
            {
                if( nPos > nDirect || nDirect == USHRT_MAX )
                    nDirect = nPos;
                long nRowSpan = pBox->getRowSpan();
                if( nRowSpan < 2 )
                    nSpan = 0;
                else if( nSpan )
                {
                    sal_uInt16 nEndOfRowSpan = static_cast<sal_uInt16>(nPos + nRowSpan - 1);
                    if( nEndOfRowSpan > nSpan || nSpan == USHRT_MAX )
                        nSpan = nEndOfRowSpan;
                }
            }
            else if( nPos < nDirect )
                nDirect = nPos;
        }
    }
    if( nSpan && nSpan < USHRT_MAX )
        return nSpan;
    return nDirect;
}

/** SwTable::NewSplitRow(..) splits all selected boxes horizontally.
*/

bool SwTable::NewSplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt,
                           bool bSameHeight )
{
    CHECK_TABLE( *this )
    ++nCnt;
    FndBox_ aFndBox( nullptr, nullptr );
    aFndBox.SetTableLines( rBoxes, *this );

    if( bSameHeight && pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        SwSplitLines aRowLines;
        SwSplitLines aSplitLines;
        sal_uInt16 nFirst = lcl_CalculateSplitLineHeights( aRowLines, aSplitLines,
            *this, rBoxes, nCnt );
        aFndBox.DelFrames( *this );
        SwTwips nLast = 0;
        SwSplitLines::iterator pSplit = aSplitLines.begin();
        SwSplitLines::iterator pCurr = aRowLines.begin();
        while( pCurr != aRowLines.end() )
        {
            while( pSplit != aSplitLines.end() && *pSplit < *pCurr )
            {
                InsertSpannedRow( pDoc, nFirst, 1 );
                SwTableLine* pRow = GetTabLines()[ nFirst ];
                SwFrameFormat* pRowFormat = pRow->ClaimFrameFormat();
                SwFormatFrameSize aFSz( pRowFormat->GetFrameSize() );
                aFSz.SetHeightSizeType( ATT_MIN_SIZE );
                aFSz.SetHeight( *pSplit - nLast );
                pRowFormat->SetFormatAttr( aFSz );
                nLast = *pSplit;
                ++pSplit;
                ++nFirst;
            }
            if( pSplit != aSplitLines.end() && *pCurr == *pSplit )
                ++pSplit;
            SwTableLine* pRow = GetTabLines()[ nFirst ];
            SwFrameFormat* pRowFormat = pRow->ClaimFrameFormat();
            SwFormatFrameSize aFSz( pRowFormat->GetFrameSize() );
            aFSz.SetHeightSizeType( ATT_MIN_SIZE );
            aFSz.SetHeight( *pCurr - nLast );
            pRowFormat->SetFormatAttr( aFSz );
            nLast = *pCurr;
            ++pCurr;
            ++nFirst;
        }
    }
    else
    {
        aFndBox.DelFrames( *this );
        bSameHeight = false;
    }
    if( !bSameHeight )
    {
        SwLineOffsetArray aLineOffs;
        lcl_SophisticatedFillLineIndices( aLineOffs, *this, rBoxes, nCnt );
        SwLineOffsetArray::reverse_iterator pCurr( aLineOffs.rbegin() );
        while( pCurr != aLineOffs.rend() )
        {
            InsertSpannedRow( pDoc, pCurr->first, pCurr->second );
            ++pCurr;
        }
    }

    std::set<size_t> aIndices;
    for (size_t i = 0; i < rBoxes.size(); ++i)
    {
        OSL_ENSURE( rBoxes[i]->getRowSpan() != 1, "Forgot to split?" );
        if( rBoxes[i]->getRowSpan() > 1 )
            aIndices.insert( i );
    }

    std::set<size_t>::iterator pCurrBox = aIndices.begin();
    while( pCurrBox != aIndices.end() )
        lcl_UnMerge( *this, *rBoxes[*pCurrBox++], nCnt, bSameHeight );

    CHECK_TABLE( *this )
    // update the layout
    aFndBox.MakeFrames( *this );

    return true;
}

/** SwTable::InsertRow(..) inserts one or more rows before or behind the selected
    boxes.
*/

bool SwTable::InsertRow( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        sal_uInt16 nCnt, bool bBehind )
{
    bool bRet = false;
    if( IsNewModel() )
    {
        CHECK_TABLE( *this )
        sal_uInt16 nRowIdx = lcl_LineIndex( *this, rBoxes, bBehind );
        if( nRowIdx < USHRT_MAX )
        {
            FndBox_ aFndBox( nullptr, nullptr );
            aFndBox.SetTableLines( rBoxes, *this );
            aFndBox.DelFrames( *this );

            bRet = true;
            SwTableLine *pLine = GetTabLines()[ nRowIdx ];
            SwSelBoxes aLineBoxes;
            lcl_FillSelBoxes( aLineBoxes, *pLine );
            InsertRow_( pDoc, aLineBoxes, nCnt, bBehind );
            const size_t nBoxCount = pLine->GetTabBoxes().size();
            sal_uInt16 nOfs = bBehind ? 0 : 1;
            for( sal_uInt16 n = 0; n < nCnt; ++n )
            {
                SwTableLine *pNewLine = GetTabLines()[ nRowIdx+nCnt-n-nOfs];
                for( size_t nCurrBox = 0; nCurrBox < nBoxCount; ++nCurrBox )
                {
                    long nRowSpan = pLine->GetTabBoxes()[nCurrBox]->getRowSpan();
                    if( bBehind )
                    {
                        if( nRowSpan == 1 || nRowSpan == -1 )
                            nRowSpan = n + 1;
                        else if( nRowSpan > 1 )
                            nRowSpan = - nRowSpan;
                    }
                    else
                    {
                        if( nRowSpan > 0 )
                            nRowSpan = n + 1;
                        else
                            --nRowSpan;
                    }
                    pNewLine->GetTabBoxes()[ nCurrBox ]->setRowSpan( nRowSpan - n );
                }
            }
            if( bBehind )
                ++nRowIdx;
            if( nRowIdx )
                lcl_ChangeRowSpan( *this, nCnt, --nRowIdx, true );
            // update the layout
            aFndBox.MakeFrames( *this );
        }
        CHECK_TABLE( *this )
    }
    else
        bRet = InsertRow_( pDoc, rBoxes, nCnt, bBehind );
    return bRet;
}

/** SwTable::PrepareDelBoxes(..) adjusts the row span attributes for an upcoming
    deletion of table cells and invalidates the layout of these cells.
*/

void SwTable::PrepareDelBoxes( const SwSelBoxes& rBoxes )
{
    if( IsNewModel() )
    {
        for (size_t i = 0; i < rBoxes.size(); ++i)
        {
            SwTableBox* pBox = rBoxes[i];
            long nRowSpan = pBox->getRowSpan();
            if( nRowSpan != 1 && pBox->GetFrameFormat()->GetFrameSize().GetWidth() )
            {
                long nLeft = lcl_Box2LeftBorder( *pBox );
                SwTableLine *pLine = pBox->GetUpper();
                sal_uInt16 nLinePos = GetTabLines().GetPos( pLine);
                OSL_ENSURE( nLinePos < USHRT_MAX, "Box/table mismatch" );
                if( nRowSpan > 1 )
                {
                    if( ++nLinePos < GetTabLines().size() )
                    {
                        pLine = GetTabLines()[ nLinePos ];
                        pBox = lcl_LeftBorder2Box( nLeft, pLine );
                        OSL_ENSURE( pBox, "RowSpan irritation I" );
                        if( pBox )
                            pBox->setRowSpan( --nRowSpan );
                    }
                }
                else if( nLinePos > 0 )
                {
                    do
                    {
                        pLine = GetTabLines()[ --nLinePos ];
                        pBox = lcl_LeftBorder2Box( nLeft, pLine );
                        OSL_ENSURE( pBox, "RowSpan irritation II" );
                        if( pBox )
                        {
                            nRowSpan = pBox->getRowSpan();
                            if( nRowSpan > 1 )
                            {
                                lcl_InvalidateCellFrame( *pBox );
                                --nRowSpan;
                            }
                            else
                                ++nRowSpan;
                            pBox->setRowSpan( nRowSpan );
                        }
                        else
                            nRowSpan = 1;
                    }
                    while( nRowSpan < 0 && nLinePos > 0 );
                }
            }
        }
    }
}

/** lcl_SearchSelBox(..) adds cells of a given table row to the selection structure
    if it overlaps with the given x-position range
*/

static void lcl_SearchSelBox( const SwTable &rTable, SwSelBoxes& rBoxes, long nMin, long nMax,
                       SwTableLine& rLine, bool bChkProtected, bool bColumn )
{
    long nLeft = 0;
    long nRight = 0;
    long nMid = ( nMax + nMin )/ 2;
    const size_t nCount = rLine.GetTabBoxes().size();
    for( size_t nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
    {
        SwTableBox* pBox = rLine.GetTabBoxes()[nCurrBox].get();
        OSL_ENSURE( pBox, "Missing table box" );
        long nWidth = pBox->GetFrameFormat()->GetFrameSize().GetWidth();
        nRight += nWidth;
        if( nRight > nMin )
        {
            bool bAdd = false;
            if( nRight <= nMax )
                bAdd = nLeft >= nMin || nRight >= nMid ||
                       nRight - nMin > nMin - nLeft;
            else
                bAdd = nLeft <= nMid || nRight - nMax < nMax - nLeft;
            long nRowSpan = pBox->getRowSpan();
            if( bAdd &&
                ( !bChkProtected ||
                !pBox->GetFrameFormat()->GetProtect().IsContentProtected() ) )
            {
                size_t const nOldCnt = rBoxes.size();
                rBoxes.insert( pBox );
                if( bColumn && nRowSpan != 1 && nOldCnt < rBoxes.size() )
                {
                    SwTableBox *pMasterBox = pBox->getRowSpan() > 0 ? pBox
                        : &pBox->FindStartOfRowSpan( rTable );
                    lcl_getAllMergedBoxes( rTable, rBoxes, *pMasterBox );
                }
            }
        }
        if( nRight >= nMax )
            break;
        nLeft = nRight;
    }
}

/** void SwTable::CreateSelection(..) fills the selection structure with table cells
    for a given SwPaM, ie. start and end position inside a table
*/

void SwTable::CreateSelection(  const SwPaM& rPam, SwSelBoxes& rBoxes,
    const SearchType eSearch, bool bChkProtected ) const
{
    OSL_ENSURE( m_bNewModel, "Don't call me for old tables" );
    if( m_aLines.empty() )
        return;
    const SwNode* pStartNd = rPam.GetPoint()->nNode.GetNode().FindTableBoxStartNode();
    const SwNode* pEndNd = rPam.GetMark()->nNode.GetNode().FindTableBoxStartNode();
    if( !pStartNd || !pEndNd )
        return;
    CreateSelection( pStartNd, pEndNd, rBoxes, eSearch, bChkProtected );
}

/** void SwTable::CreateSelection(..) fills the selection structure with table cells
    for given start and end nodes inside a table
*/
void SwTable::CreateSelection( const SwNode* pStartNd, const SwNode* pEndNd,
    SwSelBoxes& rBoxes, const SearchType eSearch, bool bChkProtected ) const
{
    rBoxes.clear();
    // Looking for start and end of the selection given by SwNode-pointer
    const size_t nLines = m_aLines.size();
    // nTop becomes the line number of the upper box
    // nBottom becomes the line number of the lower box
    size_t nTop = 0;
    size_t nBottom = 0;
    // nUpperMin becomes the left border value of the upper box
    // nUpperMax becomes the right border of the upper box
    // nLowerMin and nLowerMax the borders of the lower box
    long nUpperMin = 0, nUpperMax = 0;
    long nLowerMin = 0, nLowerMax = 0;
    // nFound will incremented if a box is found
    // 0 => no box found; 1 => the upper box has been found; 2 => both found
    int nFound = 0;
    for( size_t nRow = 0; nFound < 2 && nRow < nLines; ++nRow )
    {
        SwTableLine* pLine = m_aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        const size_t nCols = pLine->GetTabBoxes().size();
        for( size_t nCol = 0; nCol < nCols; ++nCol )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCol].get();
            OSL_ENSURE( pBox, "Missing table box" );
            if( pBox->GetSttNd() == pEndNd || pBox->GetSttNd() == pStartNd )
            {
                if( !bChkProtected ||
                    !pBox->GetFrameFormat()->GetProtect().IsContentProtected() )
                    rBoxes.insert( pBox );
                if( nFound )
                {
                    nBottom = nRow;
                    lcl_CheckMinMax( nLowerMin, nLowerMax, *pLine, nCol, true );
                    ++nFound;
                    break;
                }
                else
                {
                    nTop = nRow;
                    lcl_CheckMinMax( nUpperMin, nUpperMax, *pLine, nCol, true );
                    ++nFound;
                     // If start and end node are identical, we're nearly done..
                    if( pEndNd == pStartNd )
                    {
                        nBottom = nTop;
                        nLowerMin = nUpperMin;
                        nLowerMax = nUpperMax;
                        ++nFound;
                    }
                }
            }
        }
    }
    if( nFound < 2 )
        return; // At least one node was not a part of the given table
    if( eSearch == SEARCH_ROW )
    {
        // Selection of a row is quiet easy:
        // every (unprotected) box between start and end line
        // with a positive row span will be collected
        for( size_t nRow = nTop; nRow <= nBottom; ++nRow )
        {
            SwTableLine* pLine = m_aLines[nRow];
            OSL_ENSURE( pLine, "Missing table line" );
            const size_t nCount = pLine->GetTabBoxes().size();
            for( size_t nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
            {
                SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox].get();
                OSL_ENSURE( pBox, "Missing table box" );
                if( pBox->getRowSpan() > 0 && ( !bChkProtected ||
                    !pBox->GetFrameFormat()->GetProtect().IsContentProtected() ) )
                    rBoxes.insert( pBox );
            }
        }
        return;
    }
    bool bCombine = nTop == nBottom;
    if( !bCombine )
    {
        long nMinWidth = nUpperMax - nUpperMin;
        long nTmp = nLowerMax - nLowerMin;
        if( nMinWidth > nTmp )
            nMinWidth = nTmp;
        nTmp = std::min(nLowerMax, nUpperMax);
        nTmp -= ( nLowerMin < nUpperMin ) ? nUpperMin : nLowerMin;
        // If the overlapping between upper and lower box is less than half
        // of the width (of the smaller cell), bCombine is set,
        // e.g. if upper and lower cell are in different columns
        bCombine = ( nTmp + nTmp < nMinWidth );
    }
    if( bCombine )
    {
        if( nUpperMin < nLowerMin )
            nLowerMin = nUpperMin;
        else
            nUpperMin = nLowerMin;
        if( nUpperMax > nLowerMax )
            nLowerMax = nUpperMax;
        else
            nUpperMax = nLowerMax;
    }
    const bool bColumn = eSearch == SEARCH_COL;
    if( bColumn )
    {
        for( size_t i = 0; i < nTop; ++i )
            lcl_SearchSelBox( *this, rBoxes, nUpperMin, nUpperMax,
                              *m_aLines[i], bChkProtected, bColumn );
    }

    {
        long nMin = std::min(nUpperMin, nLowerMin);
        long nMax = nUpperMax < nLowerMax ? nLowerMax : nUpperMax;
        for( size_t i = nTop; i <= nBottom; ++i )
            lcl_SearchSelBox( *this, rBoxes, nMin, nMax, *m_aLines[i],
                              bChkProtected, bColumn );
    }
    if( bColumn )
    {
        for( size_t i = nBottom + 1; i < nLines; ++i )
            lcl_SearchSelBox( *this, rBoxes, nLowerMin, nLowerMax, *m_aLines[i],
                              bChkProtected, true );
    }
}

/** void SwTable::ExpandColumnSelection(..) adds cell to the give selection to
    assure that at least one cell of every row is part of the selection.
*/

void SwTable::ExpandColumnSelection( SwSelBoxes& rBoxes, long &rMin, long &rMax ) const
{
    OSL_ENSURE( m_bNewModel, "Don't call me for old tables" );
    rMin = 0;
    rMax = 0;
    if( m_aLines.empty() || rBoxes.empty() )
        return;

    const size_t nLineCnt = m_aLines.size();
    const size_t nBoxCnt = rBoxes.size();
    size_t nBox = 0;
    for( size_t nRow = 0; nRow < nLineCnt && nBox < nBoxCnt; ++nRow )
    {
        SwTableLine* pLine = m_aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        const size_t nCols = pLine->GetTabBoxes().size();
        for( size_t nCol = 0; nCol < nCols; ++nCol )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCol].get();
            OSL_ENSURE( pBox, "Missing table box" );
            if( pBox == rBoxes[nBox] )
            {
                lcl_CheckMinMax( rMin, rMax, *pLine, nCol, nBox == 0 );
                if( ++nBox >= nBoxCnt )
                    break;
            }
        }
    }
    for( size_t nRow = 0; nRow < nLineCnt; ++nRow )
    {
        SwTableLine* pLine = m_aLines[nRow];
        const size_t nCols = pLine->GetTabBoxes().size();
        long nRight = 0;
        for( size_t nCurrBox = 0; nCurrBox < nCols; ++nCurrBox )
        {
            long nLeft = nRight;
            SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox].get();
            nRight += pBox->GetFrameFormat()->GetFrameSize().GetWidth();
            if( nLeft >= rMin && nRight <= rMax )
                rBoxes.insert( pBox );
        }
    }
}

/** SwTable::PrepareDeleteCol(..) adjusts the widths of the neighbour cells of
    a cell selection for an upcoming (column) deletion
*/
void SwTable::PrepareDeleteCol( long nMin, long nMax )
{
    OSL_ENSURE( m_bNewModel, "Don't call me for old tables" );
    if( m_aLines.empty() || nMax < nMin )
        return;
    long nMid = nMin ? ( nMin + nMax ) / 2 : 0;
    const SwTwips nTabSize = GetFrameFormat()->GetFrameSize().GetWidth();
    if( nTabSize == nMax )
        nMid = nMax;
    const size_t nLineCnt = m_aLines.size();
    for( size_t nRow = 0; nRow < nLineCnt; ++nRow )
    {
        SwTableLine* pLine = m_aLines[nRow];
        const size_t nCols = pLine->GetTabBoxes().size();
        long nRight = 0;
        for( size_t nCurrBox = 0; nCurrBox < nCols; ++nCurrBox )
        {
            long nLeft = nRight;
            SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox].get();
            nRight += pBox->GetFrameFormat()->GetFrameSize().GetWidth();
            if( nRight < nMin )
                continue;
            if( nLeft > nMax )
                break;
            long nNewWidth = -1;
            if( nLeft < nMin )
            {
                if( nRight <= nMax )
                    nNewWidth = nMid - nLeft;
            }
            else if( nRight > nMax )
                nNewWidth = nRight - nMid;
            else
                nNewWidth = 0;
            if( nNewWidth >= 0 )
            {
                SwFrameFormat* pFrameFormat = pBox->ClaimFrameFormat();
                SwFormatFrameSize aFrameSz( pFrameFormat->GetFrameSize() );
                aFrameSz.SetWidth( nNewWidth );
                pFrameFormat->SetFormatAttr( aFrameSz );
            }
        }
    }
}

/** SwTable::ExpandSelection(..) adds all boxes to the box selections which are
    overlapped by it.
*/

void SwTable::ExpandSelection( SwSelBoxes& rBoxes ) const
{
    for (size_t i = 0; i < rBoxes.size(); ++i)
    {
        SwTableBox *pBox = rBoxes[i];
        long nRowSpan = pBox->getRowSpan();
        if( nRowSpan != 1 )
        {
            SwTableBox *pMasterBox = nRowSpan > 0 ? pBox
                    : &pBox->FindStartOfRowSpan( *this );
            lcl_getAllMergedBoxes( *this, rBoxes, *pMasterBox );
        }
    }
}

/** SwTable::CheckRowSpan(..) looks for the next line without an overlapping to
    the previous line.
*/

void SwTable::CheckRowSpan( SwTableLine* &rpLine, bool bUp ) const
{
    OSL_ENSURE( IsNewModel(), "Don't call me for old tables" );
    sal_uInt16 nLineIdx = GetTabLines().GetPos( rpLine );
    OSL_ENSURE( nLineIdx < GetTabLines().size(), "Start line out of range" );
    bool bChange = true;
    if( bUp )
    {
        while( bChange )
        {
            bChange = false;
            rpLine = GetTabLines()[ nLineIdx ];
            const size_t nCols = rpLine->GetTabBoxes().size();
            for( size_t nCol = 0; !bChange && nCol < nCols; ++nCol )
            {
                SwTableBox* pBox = rpLine->GetTabBoxes()[nCol].get();
                if( pBox->getRowSpan() > 1 || pBox->getRowSpan() < -1 )
                    bChange = true;
            }
            if( bChange )
            {
                if( nLineIdx )
                    --nLineIdx;
                else
                {
                    bChange = false;
                    rpLine = nullptr;
                }
            }
        }
    }
    else
    {
        const size_t nMaxLine = GetTabLines().size();
        while( bChange )
        {
            bChange = false;
            rpLine = GetTabLines()[ nLineIdx ];
            const size_t nCols = rpLine->GetTabBoxes().size();
            for( size_t nCol = 0; !bChange && nCol < nCols; ++nCol )
            {
                SwTableBox* pBox = rpLine->GetTabBoxes()[nCol].get();
                if( pBox->getRowSpan() < 0 )
                    bChange = true;
            }
            if( bChange )
            {
                ++nLineIdx;
                if( nLineIdx >= nMaxLine )
                {
                    bChange = false;
                    rpLine = nullptr;
                }
            }
        }
    }
}

// This structure corrects the row span attributes for a top line of a table
// In a top line no negative row span is allowed, so these have to be corrected.
// If there has been at least one correction, all values are stored
// and can be used by undo of table split
SwSaveRowSpan::SwSaveRowSpan( SwTableBoxes& rBoxes, sal_uInt16 nSplitLn )
    : mnSplitLine( nSplitLn )
{
    bool bDontSave = true; // nothing changed, nothing to save
    const size_t nColCount = rBoxes.size();
    OSL_ENSURE( nColCount, "Empty Table Line" );
    mnRowSpans.resize( nColCount );
    for( size_t nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
    {
        SwTableBox* pBox = rBoxes[nCurrCol].get();
        OSL_ENSURE( pBox, "Missing Table Box" );
        long nRowSp = pBox->getRowSpan();
        mnRowSpans[ nCurrCol ] = nRowSp;
        if( nRowSp < 0 )
        {
            bDontSave = false;
            nRowSp = -nRowSp;
            pBox->setRowSpan( nRowSp ); // correction needed
        }
    }
    if( bDontSave )
        mnRowSpans.clear();
}

// This function is called by undo of table split to restore the old row span
// values at the split line
void SwTable::RestoreRowSpan( const SwSaveRowSpan& rSave )
{
    if( !IsNewModel() ) // for new model only
        return;
    sal_uInt16 nLineCount = GetTabLines().size();
    OSL_ENSURE( rSave.mnSplitLine < nLineCount, "Restore behind last line?" );
    if( rSave.mnSplitLine < nLineCount )
    {
        SwTableLine* pLine = GetTabLines()[rSave.mnSplitLine];
        const size_t nColCount = pLine->GetTabBoxes().size();
        OSL_ENSURE( nColCount, "Empty Table Line" );
        OSL_ENSURE( nColCount == rSave.mnRowSpans.size(), "Wrong row span store" );
        if( nColCount == rSave.mnRowSpans.size() )
        {
            for( size_t nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
            {
                SwTableBox* pBox = pLine->GetTabBoxes()[nCurrCol].get();
                OSL_ENSURE( pBox, "Missing Table Box" );
                long nRowSp = pBox->getRowSpan();
                if( nRowSp != rSave.mnRowSpans[ nCurrCol ] )
                {
                    OSL_ENSURE( -nRowSp == rSave.mnRowSpans[ nCurrCol ], "Pardon me?!" );
                    OSL_ENSURE( rSave.mnRowSpans[ nCurrCol ] < 0, "Pardon me?!" );
                    pBox->setRowSpan( -nRowSp );

                    sal_uInt16 nLine = rSave.mnSplitLine;
                    if( nLine )
                    {
                        long nLeftBorder = lcl_Box2LeftBorder( *pBox );
                        SwTableBox* pNext;
                        do
                        {
                            pNext = lcl_LeftBorder2Box( nLeftBorder, GetTabLines()[--nLine] );
                            if( pNext )
                            {
                                pBox = pNext;
                                long nNewSpan = pBox->getRowSpan();
                                if( pBox->getRowSpan() < 1 )
                                    nNewSpan -= nRowSp;
                                else
                                {
                                    nNewSpan += nRowSp;
                                    pNext = nullptr;
                                }
                                pBox->setRowSpan( nNewSpan );
                            }
                        } while( nLine && pNext );
                    }
                }
            }
        }
    }
}

std::unique_ptr<SwSaveRowSpan> SwTable::CleanUpTopRowSpan( sal_uInt16 nSplitLine )
{
    if( !IsNewModel() )
        return nullptr;
    std::unique_ptr<SwSaveRowSpan> pRet(new SwSaveRowSpan( GetTabLines()[0]->GetTabBoxes(), nSplitLine ));
    if( pRet->mnRowSpans.empty() )
        return nullptr;
    return pRet;
}

void SwTable::CleanUpBottomRowSpan( sal_uInt16 nDelLines )
{
    if( !IsNewModel() )
        return;
    const size_t nLastLine = GetTabLines().size()-1;
    SwTableLine* pLine = GetTabLines()[nLastLine];
    const size_t nColCount = pLine->GetTabBoxes().size();
    OSL_ENSURE( nColCount, "Empty Table Line" );
    for( size_t nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
    {
        SwTableBox* pBox = pLine->GetTabBoxes()[nCurrCol].get();
        OSL_ENSURE( pBox, "Missing Table Box" );
        long nRowSp = pBox->getRowSpan();
        if( nRowSp < 0 )
            nRowSp = -nRowSp;
        if( nRowSp > 1 )
        {
            lcl_ChangeRowSpan( *this, -static_cast<long>(nDelLines),
                               static_cast<sal_uInt16>(nLastLine), false );
            break;
        }
    }
}

#ifdef DBG_UTIL

struct RowSpanCheck
{
    long nRowSpan;
    SwTwips nLeft;
    SwTwips nRight;
};

void SwTable::CheckConsistency() const
{
    if( !IsNewModel() )
        return;
    const size_t nLineCount = GetTabLines().size();
    const SwTwips nTabSize = GetFrameFormat()->GetFrameSize().GetWidth();
    SwTwips nLineWidth = 0;
    std::list< RowSpanCheck > aRowSpanCells;
    std::list< RowSpanCheck >::iterator aIter = aRowSpanCells.end();
    for( size_t nCurrLine = 0; nCurrLine < nLineCount; ++nCurrLine )
    {
        SwTwips nWidth = 0;
        SwTableLine* pLine = GetTabLines()[nCurrLine];
        SAL_WARN_IF( !pLine, "sw.core", "Missing Table Line" );
        const size_t nColCount = pLine->GetTabBoxes().size();
        SAL_WARN_IF( !nColCount, "sw.core", "Empty Table Line" );
        for( size_t nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCurrCol].get();
            SAL_WARN_IF( !pBox, "sw.core", "Missing Table Box" );
            SwTwips nNewWidth = pBox->GetFrameFormat()->GetFrameSize().GetWidth() + nWidth;
            long nRowSp = pBox->getRowSpan();
            if( nRowSp < 0 )
            {
                SAL_WARN_IF( aIter == aRowSpanCells.end(),
                        "sw.core", "Missing master box");
                if (aIter != aRowSpanCells.end())
                {
                    SAL_WARN_IF( aIter->nLeft != nWidth || aIter->nRight != nNewWidth,
                        "sw.core", "Wrong position/size of overlapped table box");
                    --(aIter->nRowSpan);
                    SAL_WARN_IF( aIter->nRowSpan != -nRowSp, "sw.core",
                            "Wrong row span value" );
                    if( nRowSp == -1 )
                    {
                        aIter = aRowSpanCells.erase(aIter);
                    }
                    else
                        ++aIter;
                }
            }
            else if( nRowSp != 1 )
            {
                SAL_WARN_IF( !nRowSp, "sw.core", "Zero row span?!" );
                RowSpanCheck aEntry;
                aEntry.nLeft = nWidth;
                aEntry.nRight = nNewWidth;
                aEntry.nRowSpan = nRowSp;
                aRowSpanCells.insert( aIter, aEntry );
            }
            nWidth = nNewWidth;
        }
        if( !nCurrLine )
            nLineWidth = nWidth;
        SAL_WARN_IF( nWidth != nLineWidth, "sw.core",
                "Different Line Widths: first: " << nLineWidth
                << " current [" << nCurrLine << "]: " <<  nWidth);
        SAL_WARN_IF( std::abs(nWidth - nTabSize) > 1 /* how tolerant? */, "sw.core",
                "Line width differs from table width: " << nTabSize
                << " current [" << nCurrLine << "]: " << nWidth);
        SAL_WARN_IF( nWidth < 0 || nWidth > USHRT_MAX, "sw.core",
                "Width out of range [" << nCurrLine << "]: " << nWidth);
        SAL_WARN_IF( aIter != aRowSpanCells.end(), "sw.core",
                "Missing overlapped box" );
        aIter = aRowSpanCells.begin();
    }
    bool bEmpty = aRowSpanCells.empty();
    SAL_WARN_IF( !bEmpty, "sw.core", "Open row span detected" );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
