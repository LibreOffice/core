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


#include <swtable.hxx>
#include <tblsel.hxx>
#include <tblrwcl.hxx>
#include <node.hxx>
#include <UndoTable.hxx>
#include <pam.hxx>
#include <frmfmt.hxx>
#include <frmatr.hxx>
#include <cellfrm.hxx>
#include <fmtfsize.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <vector>
#include <set>
#include <list>
#include <memory>
#include <boost/scoped_array.hpp>
#include <editeng/boxitem.hxx>
#include <editeng/protitem.hxx>
#include <swtblfmt.hxx>
#include <switerator.hxx>

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

// ---------------------------------------------------------------

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
    std::vector<const SwSelBoxes*> aBoxes;
    long mnMergeWidth;
    SwBoxSelection() : mnMergeWidth(0) {}
    bool isEmpty() const { return aBoxes.empty(); }
    void insertBoxes( const SwSelBoxes* pNew ){ aBoxes.insert( aBoxes.end(), pNew ); }
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

@return sal_True for compatibility reasons with OldMerge(..)
*/

sal_Bool SwTable::NewMerge( SwDoc* pDoc, const SwSelBoxes& rBoxes,
     const SwSelBoxes& rMerged, SwTableBox*, SwUndoTblMerge* pUndo )
{
    if( pUndo )
        pUndo->SetSelBoxes( rBoxes );
    DeleteSel( pDoc, rBoxes, &rMerged, 0, sal_True, sal_True );

    CHECK_TABLE( *this )
    return sal_True;
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

void lcl_CheckMinMax( long& rMin, long& rMax, const SwTableLine& rLine, sal_uInt16 nCheck, bool bSet )
{
    ++nCheck;
    if( rLine.GetTabBoxes().size() < nCheck )
    {   // robust
        OSL_FAIL( "Box out of table line" );
        nCheck = rLine.GetTabBoxes().size();
    }

    long nNew = 0; // will be the right border of the current box
    long nWidth = 0; // the width of the current box
    for( sal_uInt16 nCurrBox = 0; nCurrBox < nCheck; ++nCurrBox )
    {
        SwTableBox* pBox = rLine.GetTabBoxes()[nCurrBox];
        OSL_ENSURE( pBox, "Missing table box" );
        nWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
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

long lcl_Box2LeftBorder( const SwTableBox& rBox )
{
    if( !rBox.GetUpper() )
        return 0;
    long nLeft = 0;
    const SwTableLine &rLine = *rBox.GetUpper();
    sal_uInt16 nCount = rLine.GetTabBoxes().size();
    for( sal_uInt16 nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
    {
        SwTableBox* pBox = rLine.GetTabBoxes()[nCurrBox];
        OSL_ENSURE( pBox, "Missing table box" );
        if( pBox == &rBox )
            return nLeft;
        nLeft += pBox->GetFrmFmt()->GetFrmSize().GetWidth();
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

SwTableBox* lcl_LeftBorder2Box( long nLeft, const SwTableLine* pLine )
{
    if( !pLine )
        return 0;
    long nCurrLeft = 0;
    sal_uInt16 nCount = pLine->GetTabBoxes().size();
    for( sal_uInt16 nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
    {
        SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox];
        OSL_ENSURE( pBox, "Missing table box" );
        if( nCurrLeft >= nLeft && pBox->GetFrmFmt()->GetFrmSize().GetWidth() )
        {
            OSL_ENSURE( nCurrLeft == nLeft, "Wrong box found" );
            return pBox;
        }
        nCurrLeft += pBox->GetFrmFmt()->GetFrmSize().GetWidth();
    }
    OSL_FAIL( "Didn't found wished box" );
    return 0;
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

void lcl_ChangeRowSpan( const SwTable& rTable, const long nDiff,
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
        // which has to be manipulated => we have to chekc the previous row, too.
        const SwTableLine* pLine = rTable.GetTabLines()[ nRowIdx ];
        sal_uInt16 nBoxCount = pLine->GetTabBoxes().size();
        for( sal_uInt16 nCurrBox = 0; nCurrBox < nBoxCount; ++nCurrBox )
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

SwBoxSelection* SwTable::CollectBoxSelection( const SwPaM& rPam ) const
{
    OSL_ENSURE( bNewModel, "Don't call me for old tables" );
    if( aLines.empty() )
        return 0;
    const SwNode* pStartNd = rPam.Start()->nNode.GetNode().FindTableBoxStartNode();
    const SwNode* pEndNd = rPam.End()->nNode.GetNode().FindTableBoxStartNode();
    if( !pStartNd || !pEndNd || pStartNd == pEndNd )
        return 0;

    sal_uInt16 nLines = aLines.size();
    sal_uInt16 nTop = 0, nBottom = 0;
    long nMin = 0, nMax = 0;
    int nFound = 0;
    for( sal_uInt16 nRow = 0; nFound < 2 && nRow < nLines; ++nRow )
    {
        SwTableLine* pLine = aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        sal_uInt16 nCols = pLine->GetTabBoxes().size();
        for( sal_uInt16 nCol = 0; nCol < nCols; ++nCol )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCol];
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
        return 0;

    bool bOkay = true;
    long nMid = ( nMin + nMax ) / 2;

    SwBoxSelection* pRet = new SwBoxSelection();
    std::list< std::pair< SwTableBox*, long > > aNewWidthList;
    sal_uInt16 nCheckBottom = nBottom;
    long nLeftSpan = 0;
    long nRightSpan = 0;
    long nLeftSpanCnt = 0;
    long nRightSpanCnt = 0;
    for( sal_uInt16 nRow = nTop; nRow <= nBottom && bOkay; ++nRow )
    {
        SwTableLine* pLine = aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        SwSelBoxes *pBoxes = new SwSelBoxes();
        long nLeft = 0;
        long nRight = 0;
        long nRowSpan = 1;
        sal_uInt16 nCount = pLine->GetTabBoxes().size();
        for( sal_uInt16 nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox];
            OSL_ENSURE( pBox, "Missing table box" );
            nLeft = nRight;
            nRight += pBox->GetFrmFmt()->GetFrmSize().GetWidth();
            nRowSpan = pBox->getRowSpan();
            if( nRight <= nMin )
            {
                if( nRight == nMin && nLeftSpanCnt )
                    bOkay = false;
                continue;
            }
            SwTableBox* pInnerBox = 0;
            SwTableBox* pLeftBox = 0;
            SwTableBox* pRightBox = 0;
            long nDiff = 0;
            long nDiff2 = 0;
            if( nLeft < nMin )
            {
                if( nRight >= nMid || nRight + nLeft >= nMin + nMin )
                {
                    if( nCurrBox )
                    {
                        pBoxes->insert( pBox );
                        pInnerBox = pBox;
                        pLeftBox = pLine->GetTabBoxes()[nCurrBox-1];
                        nDiff = nMin - nLeft;
                        if( nRight > nMax )
                        {
                            if( nCurrBox+1 < nCount )
                            {
                                pRightBox = pLine->GetTabBoxes()[nCurrBox+1];
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
                    pInnerBox = pLine->GetTabBoxes()[nCurrBox+1];
                    nDiff = nMin - nRight;
                }
                else
                    bOkay = false;
            }
            else if( nRight <= nMax )
            {
                pBoxes->insert( pBox );
                if( nRow == nTop && nRowSpan < 0 )
                {
                    bOkay = false;
                    break;
                }
                if( nRowSpan > 1 && nRow + nRowSpan - 1 > nBottom )
                    nBottom = nRow + (sal_uInt16)nRowSpan - 1;
                if( nRowSpan < -1 && nRow - nRowSpan - 1 > nBottom )
                    nBottom = (sal_uInt16)(nRow - nRowSpan - 1);
                if( nRightSpanCnt && nRight == nMax )
                    bOkay = false;
            }
            else if( nLeft < nMax )
            {
                if( nLeft <= nMid || nRight + nLeft <= nMax )
                {
                    if( nCurrBox+1 < nCount )
                    {
                        pBoxes->insert( pBox );
                        pInnerBox = pBox;
                        pRightBox = pLine->GetTabBoxes()[nCurrBox+1];
                        nDiff = nRight - nMax;
                    }
                    else
                        bOkay = false;
                }
                else if( nCurrBox )
                {
                    pRightBox = pBox;
                    pInnerBox = pLine->GetTabBoxes()[nCurrBox-1];
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
                        nBottom += (sal_uInt16)nTmpSpan - 1;
                    else if( nTmpSpan < -1 )
                        nBottom = (sal_uInt16)( nBottom - nTmpSpan - 1 );
                }
                SwTableBox* pOuterBox = pLeftBox;
                do
                {
                    if( pOuterBox )
                    {
                        long nOutSpan = pOuterBox->getRowSpan();
                        if( nOutSpan != 1 )
                        {
                            sal_uInt16 nCheck = nRow;
                            if( nOutSpan < 0 )
                            {
                                const SwTableBox& rBox =
                                    pOuterBox->FindStartOfRowSpan( *this, USHRT_MAX );
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
                            nCheck += (sal_uInt16)nOutSpan - 1;
                            if( nCheck > nCheckBottom )
                                nCheckBottom = nCheck;
                        }
                        else if( ( nLeftSpanCnt && pLeftBox == pOuterBox ) ||
                            ( nRightSpanCnt && pRightBox == pOuterBox ) )
                            bOkay = false;
                        std::pair< SwTableBox*, long > aTmp;
                        aTmp.first = pInnerBox;
                        aTmp.second = -nDiff;
                        aNewWidthList.push_back( aTmp );
                        aTmp.first = pOuterBox;
                        aTmp.second = nDiff;
                        aNewWidthList.push_back( aTmp );
                    }
                    pOuterBox = pOuterBox == pRightBox ? 0 : pRightBox;
                    if( nDiff2 )
                        nDiff = nDiff2;
                } while( pOuterBox );
            }
        }
        if( nLeftSpanCnt )
            --nLeftSpanCnt;
        if( nRightSpanCnt )
            --nRightSpanCnt;
        pRet->insertBoxes( pBoxes );
    }
    pRet->mnMergeWidth = nMax - nMin;
    if( nCheckBottom > nBottom )
        bOkay = false;
    if( bOkay )
    {
        std::list< std::pair< SwTableBox*, long > >::iterator
            pCurr = aNewWidthList.begin();
        while( pCurr != aNewWidthList.end() )
        {
            SwFrmFmt* pFmt = pCurr->first->ClaimFrmFmt();
            long nNewWidth = pFmt->GetFrmSize().GetWidth() + pCurr->second;
            pFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nNewWidth, 0 ) );
            ++pCurr;
        }
    }
    else
    {
        delete pRet;
        pRet = 0;
    }
    return pRet;
}

/** lcl_InvalidateCellFrm(..) invalidates all layout representations of a given cell
    to initiate a reformatting
*/

void lcl_InvalidateCellFrm( const SwTableBox& rBox )
{
    SwIterator<SwCellFrm,SwFmt> aIter( *rBox.GetFrmFmt() );
    for( SwCellFrm* pCell = aIter.First(); pCell; pCell = aIter.Next() )
    {
        if( pCell->GetTabBox() == &rBox )
        {
            pCell->InvalidateSize();
            SwFrm* pLower = pCell->GetLower();
            if( pLower )
                pLower->_InvalidateSize();
        }
    }
}

/** lcl_InsertPosition(..) evaluates the insert positions in every table line,
    when a selection of cells is given and returns the average cell widths
*/

long lcl_InsertPosition( SwTable &rTable, std::vector<sal_uInt16>& rInsPos,
    const SwSelBoxes& rBoxes, sal_Bool bBehind )
{
    sal_Int32 nAddWidth = 0;
    long nCount = 0;
    for (size_t j = 0; j < rBoxes.size(); ++j)
    {
        SwTableBox *pBox = rBoxes[j];
        SwTableLine* pLine = pBox->GetUpper();
        long nWidth = rBoxes[j]->GetFrmFmt()->GetFrmSize().GetWidth();
        nAddWidth += nWidth;
        sal_uInt16 nCurrBox = pLine->GetTabBoxes().GetPos( pBox );
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

sal_Bool SwTable::NewInsertCol( SwDoc* pDoc, const SwSelBoxes& rBoxes,
    sal_uInt16 nCnt, sal_Bool bBehind )
{
    if( aLines.empty() || !nCnt )
        return sal_False;

    CHECK_TABLE( *this )
    long nNewBoxWidth = 0;
    std::vector< sal_uInt16 > aInsPos( aLines.size(), USHRT_MAX );
    { // Calculation of the insert positions and the width of the new boxes
        sal_uInt64 nTableWidth = 0;
        for( sal_uInt16 i = 0; i < aLines[0]->GetTabBoxes().size(); ++i )
            nTableWidth += aLines[0]->GetTabBoxes()[i]->GetFrmFmt()->GetFrmSize().GetWidth();

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
            return sal_False;
        nAddWidth = (nAddWidth * nTableWidth) / nResultingWidth;
        nNewBoxWidth = long( nAddWidth / nCnt ); // Rounding
        nAddWidth = nNewBoxWidth * nCnt; // Rounding
        if( !nAddWidth || nAddWidth >= nTableWidth )
            return sal_False;
        AdjustWidths( static_cast< long >(nTableWidth), static_cast< long >(nTableWidth - nAddWidth) );
    }

    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rBoxes, *this );
    aFndBox.DelFrms( *this );

    SwTableNode* pTblNd = GetTableNode();
    std::vector<SwTableBoxFmt*> aInsFormat( nCnt, 0 );
    sal_uInt16 nLastLine = USHRT_MAX;
    long nLastRowSpan = 1;

    for( sal_uInt16 i = 0; i < aLines.size(); ++i )
    {
        SwTableLine* pLine = aLines[ i ];
        sal_uInt16 nInsPos = aInsPos[i];
        assert(nInsPos != USHRT_MAX); // didn't find insert position
        SwTableBox* pBox = pLine->GetTabBoxes()[ nInsPos ];
        if( bBehind )
            ++nInsPos;
        SwTableBoxFmt* pBoxFrmFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
        ::_InsTblBox( pDoc, pTblNd, pLine, pBoxFrmFmt, pBox, nInsPos, nCnt );
        long nRowSpan = pBox->getRowSpan();
        long nDiff = i - nLastLine;
        bool bNewSpan = false;
        if( nLastLine != USHRT_MAX && nDiff <= nLastRowSpan &&
            nRowSpan != nDiff - nLastRowSpan )
        {
            bNewSpan = true;
            while( nLastLine < i )
            {
                SwTableLine* pTmpLine = aLines[ nLastLine ];
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
        const SvxBoxItem& aSelBoxItem = pBoxFrmFmt->GetBox();
        SvxBoxItem* pNoRightBorder = 0;
        if( aSelBoxItem.GetRight() )
        {
            pNoRightBorder = new SvxBoxItem( aSelBoxItem );
            pNoRightBorder->SetLine( 0, BOX_LINE_RIGHT );
        }
        for( sal_uInt16 j = 0; j < nCnt; ++j )
        {
            SwTableBox *pCurrBox = pLine->GetTabBoxes()[nInsPos+j];
            if( bNewSpan )
            {
                pCurrBox->setRowSpan( nLastRowSpan );
                SwFrmFmt* pFrmFmt = pCurrBox->ClaimFrmFmt();
                SwFmtFrmSize aFrmSz( pFrmFmt->GetFrmSize() );
                aFrmSz.SetWidth( nNewBoxWidth );
                pFrmFmt->SetFmtAttr( aFrmSz );
                if( pNoRightBorder && ( !bBehind || j+1 < nCnt ) )
                    pFrmFmt->SetFmtAttr( *pNoRightBorder );
                aInsFormat[j] = (SwTableBoxFmt*)pFrmFmt;
            }
            else
                pCurrBox->ChgFrmFmt( aInsFormat[j] );
        }
        if( bBehind && pNoRightBorder )
        {
            SwFrmFmt* pFrmFmt = pBox->ClaimFrmFmt();
            pFrmFmt->SetFmtAttr( *pNoRightBorder );
        }
        delete pNoRightBorder;
    }

    aFndBox.MakeFrms( *this );
#if OSL_DEBUG_LEVEL > 0
    {
        const SwTableBoxes &rTabBoxes = aLines[0]->GetTabBoxes();
        long nNewWidth = 0;
        for( sal_uInt16 i = 0; i < rTabBoxes.size(); ++i )
            nNewWidth += rTabBoxes[i]->GetFrmFmt()->GetFrmSize().GetWidth();
        OSL_ENSURE( nNewWidth > 0, "Very small" );
    }
#endif
    CHECK_TABLE( *this )

    return sal_True;
}

/** SwTable::PrepareMerge(..) some preparation for the coming Merge(..)

For the old table model, ::GetMergeSel(..) is called only,
for the new table model, PrepareMerge does the main work.
It modifices all cells to merge (width, border, rowspan etc.) and collects
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
   SwSelBoxes& rMerged, SwTableBox** ppMergeBox, SwUndoTblMerge* pUndo )
{
    if( !bNewModel )
    {
        ::GetMergeSel( rPam, rBoxes, ppMergeBox, pUndo );
        return rBoxes.size() > 1;
    }
    CHECK_TABLE( *this )
    // We have to assert a "rectangular" box selection before we start to merge
    std::auto_ptr< SwBoxSelection > pSel( CollectBoxSelection( rPam ) );
    if( !pSel.get() || pSel->isEmpty() )
        return false;
    // Now we should have a rectangle of boxes,
    // i.e. contiguous cells in contiguous rows
    bool bMerge = false; // will be set if any content is transferred from
    // a "not already overlapped" cell into the new master cell.
    SwTableBox *pMergeBox = (*pSel->aBoxes[0])[0]; // the master cell box
    if( !pMergeBox )
        return false;
    (*ppMergeBox) = pMergeBox;
    // The new master box will get the left and the top border of the top-left
    // box of the selection and because the new master cell _is_ the top-left
    // box, the left and right border does not need to be changed.
    // The right and bottom border instead has to be derived from the right-
    // bottom box of the selection. If this is a overlapped cell,
    // the appropriate master box.
    SwTableBox* pLastBox = 0; // the right-bottom (master) cell
    SwDoc* pDoc = GetFrmFmt()->GetDoc();
    SwPosition aInsPos( *pMergeBox->GetSttNd()->EndOfSectionNode() );
    SwPaM aChkPam( aInsPos );
    // The number of lines in the selection rectangle: nLineCount
    const sal_uInt16 nLineCount = sal_uInt16(pSel->aBoxes.size());
    // BTW: nLineCount is the rowspan of the new master cell
    long nRowSpan = nLineCount;
    // We will need the first and last line of the selection
    // to check if there any superfluous row after merging
    SwTableLine* pFirstLn = 0;
    SwTableLine* pLastLn = 0;
    // Iteration over the lines of the selection...
    for( sal_uInt16 nCurrLine = 0; nCurrLine < nLineCount; ++nCurrLine )
    {
        // The selected boxes in the current line
        const SwSelBoxes* pBoxes = pSel->aBoxes[ nCurrLine ];
        size_t nColCount = pBoxes->size();
        // Iteration over the selected cell in the current row
        for (size_t nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol)
        {
            SwTableBox* pBox = (*pBoxes)[nCurrCol];
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
                    SwCntntNode* pCNd = aPam.GetCntntNode();
                    sal_uInt16 nL = pCNd ? pCNd->Len() : 0;
                    aPam.GetPoint()->nContent.Assign( pCNd, nL );
                    SwNodeIndex aSttNdIdx( *pBox->GetSttNd(), 1 );
                    bool const bUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
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
                    if( pUndo )
                        pUndo->MoveBoxCntnt( pDoc, aRg, rInsPosNd );
                    else
                    {
                        pDoc->MoveNodeRange( aRg, rInsPosNd,
                            IDocumentContentOperations::DOC_NO_DELFRMS );
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
        _FindSuperfluousRows( rBoxes, pFirstLn, pLastLn );
        // pNewFmt will be set to the new master box and the overlapped cells
        SwFrmFmt* pNewFmt = pMergeBox->ClaimFrmFmt();
        pNewFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, pSel->mnMergeWidth, 0 ) );
        for( sal_uInt16 nCurrLine = 0; nCurrLine < nLineCount; ++nCurrLine )
        {
            const SwSelBoxes* pBoxes = pSel->aBoxes[ nCurrLine ];
            size_t nColCount = pBoxes->size();
            for (size_t nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol)
            {
                SwTableBox* pBox = (*pBoxes)[nCurrCol];
                if( nCurrCol )
                {
                    // Even this box will be deleted soon,
                    // we have to correct the width to avoid side effects
                    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
                    pFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, 0, 0 ) );
                }
                else
                    pBox->ChgFrmFmt( (SwTableBoxFmt*)pNewFmt );
            }
        }
        if( pLastBox ) // Robust
        {
            // The new borders of the master cell...
            SvxBoxItem aBox( pMergeBox->GetFrmFmt()->GetBox() );
            bool bOld = aBox.GetRight() || aBox.GetBottom();
            const SvxBoxItem& rBox = pLastBox->GetFrmFmt()->GetBox();
            aBox.SetLine( rBox.GetRight(), BOX_LINE_RIGHT );
            aBox.SetLine( rBox.GetBottom(), BOX_LINE_BOTTOM );
            if( bOld || aBox.GetLeft() || aBox.GetTop() || aBox.GetRight() || aBox.GetBottom() )
                (*ppMergeBox)->GetFrmFmt()->SetFmtAttr( aBox );
        }

        if( pUndo )
            pUndo->AddNewBox( pMergeBox->GetSttIdx() );
    }
    return bMerge;
}

/** SwTable::_FindSuperfluousRows(..) is looking for superfluous rows, i.e. rows
    containing overlapped cells only.
*/

void SwTable::_FindSuperfluousRows( SwSelBoxes& rBoxes,
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
        SwTableLine* pLine = aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        sal_uInt16 nCols = pLine->GetTabBoxes().size();
        bool bSuperfl = true;
        for( sal_uInt16 nCol = 0; nCol < nCols; ++nCol )
        {
            SwTableBox *pBox = pLine->GetTabBoxes()[nCol];
            if( pBox->getRowSpan() > 0 &&
                rBoxes.end() == rBoxes.find( pBox ) )
            {
                bSuperfl = false;
                break;
            }
        }
        if( bSuperfl )
        {
            for( sal_uInt16 nCol = 0; nCol < nCols; ++nCol )
            {
                SwTableBox* pBox = pLine->GetTabBoxes()[nCol];
                rBoxes.insert( pBox );
            }
        }
    }
}

/** SwTableBox::FindStartOfRowSpan(..) retruns the "master" cell, the cell which
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
        nMaxStep = (sal_uInt16)nAbsSpan;
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

void lcl_getAllMergedBoxes( const SwTable& rTable, SwSelBoxes& rBoxes, SwTableBox& rBox )
{
    SwTableBox* pBox = &rBox;
    OSL_ENSURE( pBox == &rBox.FindStartOfRowSpan( rTable, USHRT_MAX ), "Not a master box" );
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
    };
}

/** lcl_UnMerge(..) manipulates the row span attribute of a given master cell
    and its overlapped cells to split them into several pieces.
*/

void lcl_UnMerge( const SwTable& rTable, SwTableBox& rBox, size_t nCnt,
    sal_Bool bSameHeight )
{
    SwSelBoxes aBoxes;
    lcl_getAllMergedBoxes( rTable, aBoxes, rBox );
    size_t const nCount = aBoxes.size();
    if( nCount < 2 )
        return;
    if( nCnt > nCount )
        nCnt = nCount;
    ::boost::scoped_array<size_t> const pSplitIdx(new size_t[nCnt]);
    if( bSameHeight )
    {
        ::boost::scoped_array<SwTwips> const pHeights(new SwTwips[nCount]);
        SwTwips nHeight = 0;
        for (size_t i = 0; i < nCount; ++i)
        {
            SwTableLine* pLine = aBoxes[ i ]->GetUpper();
            SwFrmFmt *pRowFmt = pLine->GetFrmFmt();
            pHeights[ i ] = pRowFmt->GetFrmSize().GetHeight();
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
        lcl_InvalidateCellFrm( *aBoxes[ nIdx ] );
        while( ++nIdx < nNextIdx )
            aBoxes[ nIdx ]->setRowSpan( nIdx - nNextIdx );
    }
}

/** lcl_FillSelBoxes(..) puts all boxes of a given line into the selection structure
*/

void lcl_FillSelBoxes( SwSelBoxes &rBoxes, SwTableLine &rLine )
{
    sal_uInt16 nBoxCount = rLine.GetTabBoxes().size();
    for( sal_uInt16 i = 0; i < nBoxCount; ++i )
        rBoxes.insert( rLine.GetTabBoxes()[i] );
}

/** SwTable::InsertSpannedRow(..) inserts "superfluous" rows, i.e. rows containig
    overlapped cells only. This is a preparation for an upcoming split.
*/

void SwTable::InsertSpannedRow( SwDoc* pDoc, sal_uInt16 nRowIdx, sal_uInt16 nCnt )
{
    CHECK_TABLE( *this )
    OSL_ENSURE( nCnt && nRowIdx < GetTabLines().size(), "Wrong call of InsertSpannedRow" );
    SwSelBoxes aBoxes;
    SwTableLine& rLine = *GetTabLines()[ nRowIdx ];
    lcl_FillSelBoxes( aBoxes, rLine );
    SwFmtFrmSize aFSz( rLine.GetFrmFmt()->GetFrmSize() );
    if( ATT_VAR_SIZE != aFSz.GetHeightSizeType() )
    {
        SwFrmFmt* pFrmFmt = rLine.ClaimFrmFmt();
        long nNewHeight = aFSz.GetHeight() / ( nCnt + 1 );
        if( !nNewHeight )
            ++nNewHeight;
        aFSz.SetHeight( nNewHeight );
        pFrmFmt->SetFmtAttr( aFSz );
    }
    _InsertRow( pDoc, aBoxes, nCnt, sal_True );
    sal_uInt16 nBoxCount = rLine.GetTabBoxes().size();
    for( sal_uInt16 n = 0; n < nCnt; ++n )
    {
        SwTableLine *pNewLine = GetTabLines()[ nRowIdx + nCnt - n ];
        for( sal_uInt16 nCurrBox = 0; nCurrBox < nBoxCount; ++nCurrBox )
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

/******************************************************************************
When a couple of table boxes has to be split,
lcl_SophisticatedFillLineIndices delivers the information where and how many
rows have to be inserted.
Input
    rTable: the table to manipulate
    rBoxes: an array of boxes to split
    nCnt:   how many parts are wanted
Output
    rArr:   a list of pairs ( line index, number of lines to insert )

******************************************************************************/

void lcl_SophisticatedFillLineIndices( SwLineOffsetArray &rArr,
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
                std::list< SwLineOffset >::iterator pDel = pCurr;
                ++pCurr;
                aBoxes.erase( pDel );
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
                        std::list< SwLineOffset >::iterator pDel = pCurr;
                        ++pCurr;
                        aBoxes.erase( pDel );
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
    to be splitted to fulfill the requested "split same height"
*/

sal_uInt16 lcl_CalculateSplitLineHeights( SwSplitLines &rCurr, SwSplitLines &rNew,
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

    if( aBoxes.empty() )
        return 0;
    SwTwips nHeight = 0;
    SwTwips* pLines = new SwTwips[ nLast + 1 - nFirst ];
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
    delete[] pLines;
    return nFirst;
}

/** lcl_LineIndex(..) delivers the line index of the line behind or above
    the box selection.
*/

sal_uInt16 lcl_LineIndex( const SwTable& rTable, const SwSelBoxes& rBoxes,
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
                    sal_uInt16 nEndOfRowSpan = (sal_uInt16)(nPos + nRowSpan - 1);
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

sal_Bool SwTable::NewSplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt,
    sal_Bool bSameHeight )
{
    CHECK_TABLE( *this )
    ++nCnt;
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rBoxes, *this );

    if( bSameHeight && pDoc->GetCurrentViewShell() )    //swmod 071108//swmod 071225
    {
        SwSplitLines aRowLines;
        SwSplitLines aSplitLines;
        sal_uInt16 nFirst = lcl_CalculateSplitLineHeights( aRowLines, aSplitLines,
            *this, rBoxes, nCnt );
        aFndBox.DelFrms( *this );
        SwTwips nLast = 0;
        SwSplitLines::iterator pSplit = aSplitLines.begin();
        SwSplitLines::iterator pCurr = aRowLines.begin();
        while( pCurr != aRowLines.end() )
        {
            while( pSplit != aSplitLines.end() && *pSplit < *pCurr )
            {
                InsertSpannedRow( pDoc, nFirst, 1 );
                SwTableLine* pRow = GetTabLines()[ nFirst ];
                SwFrmFmt* pRowFmt = pRow->ClaimFrmFmt();
                SwFmtFrmSize aFSz( pRowFmt->GetFrmSize() );
                aFSz.SetHeightSizeType( ATT_MIN_SIZE );
                aFSz.SetHeight( *pSplit - nLast );
                pRowFmt->SetFmtAttr( aFSz );
                nLast = *pSplit;
                ++pSplit;
                ++nFirst;
            }
            if( pSplit != aSplitLines.end() && *pCurr == *pSplit )
                ++pSplit;
            SwTableLine* pRow = GetTabLines()[ nFirst ];
            SwFrmFmt* pRowFmt = pRow->ClaimFrmFmt();
            SwFmtFrmSize aFSz( pRowFmt->GetFrmSize() );
            aFSz.SetHeightSizeType( ATT_MIN_SIZE );
            aFSz.SetHeight( *pCurr - nLast );
            pRowFmt->SetFmtAttr( aFSz );
            nLast = *pCurr;
            ++pCurr;
            ++nFirst;
        }
    }
    else
    {
        aFndBox.DelFrms( *this );
        bSameHeight = sal_False;
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
    //Layout updaten
    aFndBox.MakeFrms( *this );

    return sal_True;
}

/** SwTable::InsertRow(..) inserts one or more rows before or behind the selected
    boxes.
*/

sal_Bool SwTable::InsertRow( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        sal_uInt16 nCnt, sal_Bool bBehind )
{
    bool bRet = false;
    if( IsNewModel() )
    {
        CHECK_TABLE( *this )
        sal_uInt16 nRowIdx = lcl_LineIndex( *this, rBoxes, bBehind );
        if( nRowIdx < USHRT_MAX )
        {
            _FndBox aFndBox( 0, 0 );
            aFndBox.SetTableLines( rBoxes, *this );
            aFndBox.DelFrms( *this );

            bRet = true;
            SwTableLine *pLine = GetTabLines()[ nRowIdx ];
            SwSelBoxes aLineBoxes;
            lcl_FillSelBoxes( aLineBoxes, *pLine );
            _InsertRow( pDoc, aLineBoxes, nCnt, bBehind );
            sal_uInt16 nBoxCount = pLine->GetTabBoxes().size();
            sal_uInt16 nOfs = bBehind ? 0 : 1;
            for( sal_uInt16 n = 0; n < nCnt; ++n )
            {
                SwTableLine *pNewLine = GetTabLines()[ nRowIdx+nCnt-n-nOfs];
                for( sal_uInt16 nCurrBox = 0; nCurrBox < nBoxCount; ++nCurrBox )
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
            //Layout update
            aFndBox.MakeFrms( *this );
        }
        CHECK_TABLE( *this )
    }
    else
        bRet = _InsertRow( pDoc, rBoxes, nCnt, bBehind );
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
            if( nRowSpan != 1 && pBox->GetFrmFmt()->GetFrmSize().GetWidth() )
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
                                lcl_InvalidateCellFrm( *pBox );
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

void lcl_SearchSelBox( const SwTable &rTable, SwSelBoxes& rBoxes, long nMin, long nMax,
                       SwTableLine& rLine, bool bChkProtected, bool bColumn )
{
    long nLeft = 0;
    long nRight = 0;
    long nMid = ( nMax + nMin )/ 2;
    sal_uInt16 nCount = rLine.GetTabBoxes().size();
    for( sal_uInt16 nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
    {
        SwTableBox* pBox = rLine.GetTabBoxes()[nCurrBox];
        OSL_ENSURE( pBox, "Missing table box" );
        long nWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
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
                !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() ) )
            {
                size_t const nOldCnt = rBoxes.size();
                rBoxes.insert( pBox );
                if( bColumn && nRowSpan != 1 && nOldCnt < rBoxes.size() )
                {
                    SwTableBox *pMasterBox = pBox->getRowSpan() > 0 ? pBox
                        : &pBox->FindStartOfRowSpan( rTable, USHRT_MAX );
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
    OSL_ENSURE( bNewModel, "Don't call me for old tables" );
    if( aLines.empty() )
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
    sal_uInt16 nLines = aLines.size();
    // nTop becomes the line number of the upper box
    // nBottom becomes the line number of the lower box
    sal_uInt16 nTop = 0, nBottom = 0;
    // nUpperMin becomes the left border value of the upper box
    // nUpperMax becomes the right border of the upper box
    // nLowerMin and nLowerMax the borders of the lower box
    long nUpperMin = 0, nUpperMax = 0;
    long nLowerMin = 0, nLowerMax = 0;
    // nFound will incremented if a box is found
    // 0 => no box found; 1 => the upper box has been found; 2 => both found
    int nFound = 0;
    for( sal_uInt16 nRow = 0; nFound < 2 && nRow < nLines; ++nRow )
    {
        SwTableLine* pLine = aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        sal_uInt16 nCols = pLine->GetTabBoxes().size();
        for( sal_uInt16 nCol = 0; nCol < nCols; ++nCol )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCol];
            OSL_ENSURE( pBox, "Missing table box" );
            if( pBox->GetSttNd() == pEndNd || pBox->GetSttNd() == pStartNd )
            {
                if( !bChkProtected ||
                    !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
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
        for( sal_uInt16 nRow = nTop; nRow <= nBottom; ++nRow )
        {
            SwTableLine* pLine = aLines[nRow];
            OSL_ENSURE( pLine, "Missing table line" );
            sal_uInt16 nCount = pLine->GetTabBoxes().size();
            for( sal_uInt16 nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
            {
                SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox];
                OSL_ENSURE( pBox, "Missing table box" );
                if( pBox->getRowSpan() > 0 && ( !bChkProtected ||
                    !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() ) )
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
        nTmp = nLowerMax < nUpperMax ? nLowerMax : nUpperMax;
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
        for( sal_uInt16 i = 0; i < nTop; ++i )
            lcl_SearchSelBox( *this, rBoxes, nUpperMin, nUpperMax,
                              *aLines[i], bChkProtected, bColumn );
    }

    {
        long nMin = nUpperMin < nLowerMin ? nUpperMin : nLowerMin;
        long nMax = nUpperMax < nLowerMax ? nLowerMax : nUpperMax;
        for( sal_uInt16 i = nTop; i <= nBottom; ++i )
            lcl_SearchSelBox( *this, rBoxes, nMin, nMax, *aLines[i],
                              bChkProtected, bColumn );
    }
    if( bColumn )
    {
        for( sal_uInt16 i = nBottom + 1; i < nLines; ++i )
            lcl_SearchSelBox( *this, rBoxes, nLowerMin, nLowerMax, *aLines[i],
                              bChkProtected, true );
    }
}

/** void SwTable::ExpandColumnSelection(..) adds cell to the give selection to
    assure that at least one cell of every row is part of the selection.
*/

void SwTable::ExpandColumnSelection( SwSelBoxes& rBoxes, long &rMin, long &rMax ) const
{
    OSL_ENSURE( bNewModel, "Don't call me for old tables" );
    rMin = 0;
    rMax = 0;
    if( aLines.empty() || rBoxes.empty() )
        return;

    sal_uInt16 nLineCnt = aLines.size();
    size_t const nBoxCnt = rBoxes.size();
    size_t nBox = 0;
    for( sal_uInt16 nRow = 0; nRow < nLineCnt && nBox < nBoxCnt; ++nRow )
    {
        SwTableLine* pLine = aLines[nRow];
        OSL_ENSURE( pLine, "Missing table line" );
        sal_uInt16 nCols = pLine->GetTabBoxes().size();
        for( sal_uInt16 nCol = 0; nCol < nCols; ++nCol )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCol];
            OSL_ENSURE( pBox, "Missing table box" );
            if( pBox == rBoxes[nBox] )
            {
                lcl_CheckMinMax( rMin, rMax, *pLine, nCol, nBox == 0 );
                if( ++nBox >= nBoxCnt )
                    break;
            }
        }
    }
    for( sal_uInt16 nRow = 0; nRow < nLineCnt; ++nRow )
    {
        SwTableLine* pLine = aLines[nRow];
        sal_uInt16 nCols = pLine->GetTabBoxes().size();
        long nLeft = 0;
        long nRight = 0;
        for( sal_uInt16 nCurrBox = 0; nCurrBox < nCols; ++nCurrBox )
        {
            nLeft = nRight;
            SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox];
            nRight += pBox->GetFrmFmt()->GetFrmSize().GetWidth();
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
    OSL_ENSURE( bNewModel, "Don't call me for old tables" );
    if( aLines.empty() || nMax < nMin )
        return;
    long nMid = nMin ? ( nMin + nMax ) / 2 : 0;
    const SwTwips nTabSize = GetFrmFmt()->GetFrmSize().GetWidth();
    if( nTabSize == nMax )
        nMid = nMax;
    sal_uInt16 nLineCnt = aLines.size();
    for( sal_uInt16 nRow = 0; nRow < nLineCnt; ++nRow )
    {
        SwTableLine* pLine = aLines[nRow];
        sal_uInt16 nCols = pLine->GetTabBoxes().size();
        long nLeft = 0;
        long nRight = 0;
        for( sal_uInt16 nCurrBox = 0; nCurrBox < nCols; ++nCurrBox )
        {
            nLeft = nRight;
            SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox];
            nRight += pBox->GetFrmFmt()->GetFrmSize().GetWidth();
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
                SwFrmFmt* pFrmFmt = pBox->ClaimFrmFmt();
                SwFmtFrmSize aFrmSz( pFrmFmt->GetFrmSize() );
                aFrmSz.SetWidth( nNewWidth );
                pFrmFmt->SetFmtAttr( aFrmSz );
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
                    : &pBox->FindStartOfRowSpan( *this, USHRT_MAX );
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
            sal_uInt16 nCols = rpLine->GetTabBoxes().size();
            for( sal_uInt16 nCol = 0; !bChange && nCol < nCols; ++nCol )
            {
                SwTableBox* pBox = rpLine->GetTabBoxes()[nCol];
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
                    rpLine = 0;
                }
            }
        }
    }
    else
    {
        sal_uInt16 nMaxLine = GetTabLines().size();
        while( bChange )
        {
            bChange = false;
            rpLine = GetTabLines()[ nLineIdx ];
            sal_uInt16 nCols = rpLine->GetTabBoxes().size();
            for( sal_uInt16 nCol = 0; !bChange && nCol < nCols; ++nCol )
            {
                SwTableBox* pBox = rpLine->GetTabBoxes()[nCol];
                if( pBox->getRowSpan() < 0 )
                    bChange = true;
            }
            if( bChange )
            {
                ++nLineIdx;
                if( nLineIdx >= nMaxLine )
                {
                    bChange = false;
                    rpLine = 0;
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
    sal_uInt16 nColCount = rBoxes.size();
    OSL_ENSURE( nColCount, "Empty Table Line" );
    mnRowSpans.resize( nColCount );
    for( sal_uInt16 nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
    {
        SwTableBox* pBox = rBoxes[nCurrCol];
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
        sal_uInt16 nColCount = pLine->GetTabBoxes().size();
        OSL_ENSURE( nColCount, "Empty Table Line" );
        OSL_ENSURE( nColCount == rSave.mnRowSpans.size(), "Wrong row span store" );
        if( nColCount == rSave.mnRowSpans.size() )
        {
            for( sal_uInt16 nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
            {
                SwTableBox* pBox = pLine->GetTabBoxes()[nCurrCol];
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
                                    pNext = 0;
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

SwSaveRowSpan* SwTable::CleanUpTopRowSpan( sal_uInt16 nSplitLine )
{
    SwSaveRowSpan* pRet = 0;
    if( !IsNewModel() )
        return pRet;
    pRet = new SwSaveRowSpan( GetTabLines()[0]->GetTabBoxes(), nSplitLine );
    if( pRet->mnRowSpans.empty() )
    {
        delete pRet;
        pRet = 0;
    }
    return pRet;
}

void SwTable::CleanUpBottomRowSpan( sal_uInt16 nDelLines )
{
    if( !IsNewModel() )
        return;
    sal_uInt16 nLastLine = GetTabLines().size()-1;
    SwTableLine* pLine = GetTabLines()[nLastLine];
    sal_uInt16 nColCount = pLine->GetTabBoxes().size();
    OSL_ENSURE( nColCount, "Empty Table Line" );
    for( sal_uInt16 nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
    {
        SwTableBox* pBox = pLine->GetTabBoxes()[nCurrCol];
        OSL_ENSURE( pBox, "Missing Table Box" );
        long nRowSp = pBox->getRowSpan();
        if( nRowSp < 0 )
            nRowSp = -nRowSp;
        if( nRowSp > 1 )
        {
            lcl_ChangeRowSpan( *this, -static_cast<long>(nDelLines), nLastLine, false );
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
    sal_uInt16 nLineCount = GetTabLines().size();
    const SwTwips nTabSize = GetFrmFmt()->GetFrmSize().GetWidth();
    SwTwips nLineWidth = 0;
    std::list< RowSpanCheck > aRowSpanCells;
    std::list< RowSpanCheck >::iterator aIter = aRowSpanCells.end();
    for( sal_uInt16 nCurrLine = 0; nCurrLine < nLineCount; ++nCurrLine )
    {
        SwTwips nWidth = 0;
        SwTableLine* pLine = GetTabLines()[nCurrLine];
        SAL_WARN_IF( !pLine, "sw.core", "Missing Table Line" );
        sal_uInt16 nColCount = pLine->GetTabBoxes().size();
        SAL_WARN_IF( !nColCount, "sw.core", "Empty Table Line" );
        for( sal_uInt16 nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nCurrCol];
            SAL_WARN_IF( !pBox, "sw.core", "Missing Table Box" );
            SwTwips nNewWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth() + nWidth;
            long nRowSp = pBox->getRowSpan();
            if( nRowSp < 0 )
            {
                SAL_WARN_IF( aIter == aRowSpanCells.end(),
                        "sw.core", "Missing master box");
                SAL_WARN_IF( aIter->nLeft != nWidth || aIter->nRight != nNewWidth,
                    "sw.core", "Wrong position/size of overlapped table box");
                --(aIter->nRowSpan);
                SAL_WARN_IF( aIter->nRowSpan != -nRowSp, "sw.core",
                        "Wrong row span value" );
                if( nRowSp == -1 )
                {
                    std::list< RowSpanCheck >::iterator aEraseIter = aIter;
                    ++aIter;
                    aRowSpanCells.erase( aEraseIter );
                }
                else
                    ++aIter;
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
        SAL_WARN_IF( abs(nWidth - nTabSize) > 1 /* how tolerant? */, "sw.core",
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
