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


#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>

#include <tools/gen.hxx>

#include "cell.hxx"
#include "cellrange.hxx"
#include "tablemodel.hxx"
#include "tablerow.hxx"
#include "tablerows.hxx"
#include "tablecolumn.hxx"
#include "tablecolumns.hxx"
#include "tablelayouter.hxx"
#include "svx/svdotable.hxx"
#include "editeng/borderline.hxx"
#include "editeng/boxitem.hxx"
#include "svx/svdmodel.hxx"
#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"

using ::editeng::SvxBorderLine;
using ::com::sun::star::awt::XLayoutConstrains;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

// -----------------------------------------------------------------------------

static SvxBorderLine gEmptyBorder;

// -----------------------------------------------------------------------------

TableLayouter::TableLayouter( const TableModelRef& xTableModel )
: mxTable( xTableModel )
, msSize( "Size" )
{
}

// -----------------------------------------------------------------------------

TableLayouter::~TableLayouter()
{
    ClearBorderLayout();
}

// -----------------------------------------------------------------------------

basegfx::B2ITuple TableLayouter::getCellSize( const CellPos& rPos  ) const
{
    sal_Int32 width = 0;
    sal_Int32 height = 0;

    try
    {
        CellRef xCell( getCell( rPos ) );
        if( xCell.is() && !xCell->isMerged() )
        {
            CellPos aPos( rPos );

            sal_Int32 nRowCount = getRowCount();
            sal_Int32 nRowSpan = std::max( xCell->getRowSpan(), (sal_Int32)1 );
            while( nRowSpan && (aPos.mnRow < nRowCount) )
            {
                if( ((sal_Int32)maRows.size()) <= aPos.mnRow )
                    break;

                height += maRows[aPos.mnRow++].mnSize;
                nRowSpan--;
            }

            sal_Int32 nColCount = getColumnCount();
            sal_Int32 nColSpan = std::max( xCell->getColumnSpan(), (sal_Int32)1 );
            while( nColSpan && (aPos.mnCol < nColCount ) )
            {
                if( ((sal_Int32)maColumns.size()) <= aPos.mnCol )
                    break;

                width += maColumns[aPos.mnCol++].mnSize;
                nColSpan--;
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "TableLayouter::getCellSize(), exception caught!" );
    }

    return basegfx::B2ITuple( width, height );
}

// -----------------------------------------------------------------------------

bool TableLayouter::getCellArea( const CellPos& rPos, basegfx::B2IRectangle& rArea ) const
{
    try
    {
        CellRef xCell( getCell( rPos ) );
        if( xCell.is() && !xCell->isMerged() && isValid(rPos) )
        {
            const basegfx::B2ITuple aCellSize( getCellSize( rPos ) );
            const bool bRTL = (mxTable->getSdrTableObj()->GetWritingMode() == WritingMode_RL_TB);

            if( (rPos.mnCol < ((sal_Int32)maColumns.size()) && (rPos.mnRow < ((sal_Int32)maRows.size()) ) ) )
            {
                const sal_Int32 y = maRows[rPos.mnRow].mnPos;

                if(bRTL)
                {
                    ///For RTL Table Calculate the Right End of cell instead of Left
                    const sal_Int32 x = maColumns[rPos.mnCol].mnPos + maColumns[rPos.mnCol].mnSize;
                    rArea = basegfx::B2IRectangle( x-aCellSize.getX(), y, x, y + aCellSize.getY()  );
                }
                else
                {
                    const sal_Int32 x = maColumns[rPos.mnCol].mnPos;
                    rArea = basegfx::B2IRectangle( x, y, x + aCellSize.getX(), y + aCellSize.getY()  );
                }
                return true;
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "TableLayouter::getCellSize(), exception caught!" );
    }
    return false;
}

// -----------------------------------------------------------------------------
sal_Int32 TableLayouter::getRowHeight( sal_Int32 nRow ) const
{
    if( isValidRow(nRow) )
        return maRows[nRow].mnSize;
    else
        return 0;
}

// -----------------------------------------------------------------------------
sal_Int32 TableLayouter::getColumnWidth( sal_Int32 nColumn ) const
{
    if( isValidColumn(nColumn) )
        return maColumns[nColumn].mnSize;
    else
        return 0;
}

// -----------------------------------------------------------------------------

bool TableLayouter::isEdgeVisible( sal_Int32 nEdgeX, sal_Int32 nEdgeY, bool bHorizontal ) const
{
    const BorderLineMap& rMap = bHorizontal ? maHorizontalBorders : maVerticalBorders;

    if( (nEdgeX >= 0) && (nEdgeX < sal::static_int_cast<sal_Int32>(rMap.size())) &&
        (nEdgeY >= 0) && (nEdgeY < sal::static_int_cast<sal_Int32>(rMap[nEdgeX].size())) )
    {
        return rMap[nEdgeX][nEdgeY] != 0;
    }
    else
    {
        OSL_FAIL( "sdr::table::TableLayouter::getBorderLine(), invalid edge!" );
    }

    return false;
}

// -----------------------------------------------------------------------------

/** returns the requested borderline in rpBorderLine or a null pointer if there is no border at this edge */
SvxBorderLine* TableLayouter::getBorderLine( sal_Int32 nEdgeX, sal_Int32 nEdgeY, bool bHorizontal )const
{
    SvxBorderLine* pLine = 0;

    const BorderLineMap& rMap = bHorizontal ? maHorizontalBorders : maVerticalBorders;

    if( (nEdgeX >= 0) && (nEdgeX < sal::static_int_cast<sal_Int32>(rMap.size())) &&
        (nEdgeY >= 0) && (nEdgeY < sal::static_int_cast<sal_Int32>(rMap[nEdgeX].size())) )
    {
        pLine = rMap[nEdgeX][nEdgeY];
        if( pLine == &gEmptyBorder )
            pLine = 0;
    }
    else
    {
        OSL_FAIL( "sdr::table::TableLayouter::getBorderLine(), invalid edge!" );
    }

    return pLine;
}

// -----------------------------------------------------------------------------

sal_Int32 TableLayouter::getHorizontalEdge( int nEdgeY, sal_Int32* pnMin /*= 0*/, sal_Int32* pnMax /*= 0*/ )
{
    sal_Int32 nRet = 0;
    const sal_Int32 nRowCount = getRowCount();
    if( (nEdgeY >= 0) && (nEdgeY <= nRowCount ) )
        nRet = maRows[std::min((sal_Int32)nEdgeY,nRowCount-1)].mnPos;

    if( nEdgeY == nRowCount )
        nRet += maRows[nEdgeY - 1].mnSize;

    if( pnMin )
    {
        if( (nEdgeY > 0) && (nEdgeY <= nRowCount ) )
        {
            *pnMin = maRows[nEdgeY-1].mnPos + 600; // todo
        }
        else
        {
            *pnMin = nRet;
        }
    }

    if( pnMax )
    {
        *pnMax = 0x0fffffff;
    }
    return nRet;
}

// -----------------------------------------------------------------------------

sal_Int32 TableLayouter::getVerticalEdge( int nEdgeX, sal_Int32* pnMin /*= 0*/, sal_Int32* pnMax /*= 0*/ )
{
    sal_Int32 nRet = 0;

    const sal_Int32 nColCount = getColumnCount();
    if( (nEdgeX >= 0) && (nEdgeX <= nColCount ) )
        nRet = maColumns[std::min((sal_Int32)nEdgeX,nColCount-1)].mnPos;

    const bool bRTL = (mxTable->getSdrTableObj()->GetWritingMode() == WritingMode_RL_TB);
    if( bRTL )
    {
        if( (nEdgeX >= 0) && (nEdgeX < nColCount) )
            nRet += maColumns[nEdgeX].mnSize;
    }
    else
    {
        if( nEdgeX == nColCount )
            nRet += maColumns[nEdgeX - 1].mnSize;
    }

    if( pnMin )
    {
        *pnMin = nRet;
        if( bRTL )
        {
            if( nEdgeX < nColCount )
                *pnMin = nRet - maColumns[nEdgeX].mnSize + getMinimumColumnWidth(nEdgeX);
        }
        else
        {
            if( (nEdgeX > 0) && (nEdgeX <= nColCount ) )
                *pnMin = maColumns[nEdgeX-1].mnPos + getMinimumColumnWidth( nEdgeX-1 );
        }
    }

    if( pnMax )
    {
        *pnMax = 0x0fffffff; // todo
        if( bRTL )
        {
            if( nEdgeX > 0 )
                *pnMax = nRet + maColumns[nEdgeX-1].mnSize - getMinimumColumnWidth( nEdgeX-1 );
        }
        else
        {
            if( (nEdgeX >= 0) && (nEdgeX < nColCount ) )
                *pnMax = maColumns[nEdgeX].mnPos + maColumns[nEdgeX].mnSize - getMinimumColumnWidth( nEdgeX );
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------------

static bool checkMergeOrigin( const TableModelRef& xTable, sal_Int32 nMergedX, sal_Int32 nMergedY, sal_Int32 nCellX, sal_Int32 nCellY, bool& bRunning )
{
    Reference< XMergeableCell > xCell( xTable->getCellByPosition( nCellX, nCellY ), UNO_QUERY );
    if( xCell.is() && !xCell->isMerged() )
    {
        const sal_Int32 nRight = xCell->getColumnSpan() + nCellX;
        const sal_Int32 nBottom = xCell->getRowSpan() + nCellY;
        if( (nMergedX < nRight) && (nMergedY < nBottom) )
            return true;

        bRunning = false;
    }
    return false;
}

/** returns true if the cell(nMergedX,nMergedY) is merged with other cells.
    the returned cell( rOriginX, rOriginY ) is the origin( top left cell ) of the merge.
*/
bool findMergeOrigin( const TableModelRef& xTable, sal_Int32 nMergedX, sal_Int32 nMergedY, sal_Int32& rOriginX, sal_Int32& rOriginY )
{
    rOriginX = nMergedX;
    rOriginY = nMergedY;

    if( xTable.is() ) try
    {
        // check if this cell already the origin or not merged at all
        Reference< XMergeableCell > xCell( xTable->getCellByPosition( nMergedX, nMergedY ), UNO_QUERY_THROW );
        if( !xCell.is() || !xCell->isMerged() )
            return true;

        bool bCheckVert = true;
        bool bCheckHorz = true;

        sal_Int32 nMinCol = 0;
        sal_Int32 nMinRow = 0;

        sal_Int32 nStep = 1, i;

        sal_Int32 nRow, nCol;
        do
        {
            if( bCheckVert )
            {
                nRow = nMergedY - nStep;
                if( nRow >= nMinRow )
                {
                    nCol = nMergedX;
                    for( i = 0; (i <= nStep) && (nCol >= nMinCol); i++, nCol-- )
                    {
                        if( checkMergeOrigin( xTable, nMergedX, nMergedY, nCol, nRow, bCheckVert ) )
                        {
                            rOriginX = nCol; rOriginY = nRow;
                            return true;
                        }

                        if( !bCheckVert )
                        {
                            if( nCol == nMergedX )
                            {
                                nMinRow = nRow+1;
                            }
                            else
                            {
                                bCheckVert = true;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    bCheckVert = false;
                }
            }

            if( bCheckHorz )
            {
                nCol = nMergedX - nStep;
                if( nCol >= nMinCol )
                {
                    nRow = nMergedY;
                    for( i = 0; (i < nStep) && (nRow >= nMinRow); i++, nRow-- )
                    {
                        if( checkMergeOrigin( xTable, nMergedX, nMergedY, nCol, nRow, bCheckHorz ) )
                        {
                            rOriginX = nCol; rOriginY = nRow;
                            return true;
                        }

                        if( !bCheckHorz )
                        {
                            if( nRow == nMergedY )
                            {
                                nMinCol = nCol+1;
                            }
                            else
                            {
                                bCheckHorz = true;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    bCheckHorz = false;
                }
            }
            nStep++;
        }
        while( bCheckVert || bCheckHorz );
    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::TableLayouter::findMergeOrigin(), exception caught!");
    }
    return false;
}

// -----------------------------------------------------------------------------

sal_Int32 TableLayouter::getMinimumColumnWidth( sal_Int32 nColumn )
{
    if( isValidColumn( nColumn ) )
    {
        return maColumns[nColumn].mnMinSize;
    }
    else
    {
        OSL_FAIL( "TableLayouter::getMinimumColumnWidth(), column out of range!" );
        return 0;
    }
}

// -----------------------------------------------------------------------------

sal_Int32 TableLayouter::distribute( LayoutVector& rLayouts, sal_Int32 nDistribute )
{
    // break loops after 100 runs to avoid freezing office due to developer error
    sal_Int32 nSafe = 100;

    const sal_Size nCount = rLayouts.size();
    sal_Size nIndex;

    bool bConstrainsBroken = false;

    do
    {
        // first enforce minimum size constrains on all entities
        for( nIndex = 0; nIndex < nCount; ++nIndex )
        {
            Layout& rLayout = rLayouts[nIndex];
            if( rLayout.mnSize < rLayout.mnMinSize )
            {
                nDistribute -= rLayout.mnMinSize - rLayout.mnSize;
                rLayout.mnSize = rLayout.mnMinSize;
            }
        }

        // calculate current width
        // if nDistribute is < 0 (shrinking), entities that are already
        // at minimum width are not counted
        sal_Int32 nCurrentWidth = 0;
        for( nIndex = 0; nIndex < nCount; ++nIndex )
        {
            Layout& rLayout = rLayouts[nIndex];
            if( (nDistribute > 0) || (rLayout.mnSize > rLayout.mnMinSize) )
                nCurrentWidth += rLayout.mnSize;
        }

        bConstrainsBroken = false;

        // now distribute over entities
        if( (nCurrentWidth != 0) && (nDistribute != 0) )
        {
            sal_Int32 nDistributed = nDistribute;
            for( nIndex = 0; nIndex < nCount; ++nIndex )
            {
                Layout& rLayout = rLayouts[nIndex];
                if( (nDistribute > 0) || (rLayout.mnSize > rLayout.mnMinSize) )
                {
                    sal_Int32 n;
                    if( nIndex == (nCount-1) )
                        n = nDistributed; // for last entitie, use up rest
                    else
                        n  = (nDistribute * rLayout.mnSize) / nCurrentWidth; //

                    nDistributed -= n;
                    rLayout.mnSize += n;

                    if( rLayout.mnSize < rLayout.mnMinSize )
                        bConstrainsBroken = true;
                }
            }
        }
    } while( bConstrainsBroken && --nSafe );

    sal_Int32 nSize = 0;
    for( nIndex = 0; nIndex < nCount; ++nIndex )
        nSize += rLayouts[nIndex].mnSize;

    return nSize;
}

// -----------------------------------------------------------------------------

typedef std::vector< CellRef > MergeableCellVector;
typedef std::vector< MergeableCellVector > MergeVector;
typedef std::vector< sal_Int32 > Int32Vector;

// -----------------------------------------------------------------------------

void TableLayouter::LayoutTableWidth( Rectangle& rArea, bool bFit )
{
    const sal_Int32 nColCount = getColumnCount();
    const sal_Int32 nRowCount = getRowCount();
    if( nColCount == 0 )
        return;

    MergeVector aMergedCells( nColCount );
    Int32Vector aOptimalColumns;

    const OUString sOptimalSize("OptimalSize");

    if( sal::static_int_cast< sal_Int32 >( maColumns.size() ) != nColCount )
        maColumns.resize( nColCount );

    Reference< XTableColumns > xCols( mxTable->getColumns(), UNO_QUERY_THROW );

    // first calculate current width and initial minimum width per column,
    // merged cells will be counted later
    sal_Int32 nCurrentWidth = 0;
    sal_Int32 nCol = 0, nRow = 0;
    for( nCol = 0; nCol < nColCount; nCol++ )
    {
        sal_Int32 nMinWidth = 0;

        bool bIsEmpty = true; // check if all cells in this column are merged

        for( nRow = 0; nRow < nRowCount; ++nRow )
        {
            CellRef xCell( getCell( CellPos( nCol, nRow ) ) );
            if( xCell.is() && !xCell->isMerged() )
            {
                bIsEmpty = false;

                sal_Int32 nColSpan = xCell->getColumnSpan();
                if( nColSpan > 1 )
                {
                    // merged cells will be evaluated later
                    aMergedCells[nCol+nColSpan-1].push_back( xCell );
                }
                else
                {
                    nMinWidth = std::max( nMinWidth, xCell->getMinimumSize().Width );
                }
            }
        }

        maColumns[nCol].mnMinSize = nMinWidth;

        if( bIsEmpty )
        {
            maColumns[nCol].mnSize = 0;
        }
        else
        {
            sal_Int32 nColWidth = 0;
            Reference< XPropertySet > xColSet( xCols->getByIndex( nCol ), UNO_QUERY_THROW );
            sal_Bool bOptimal = sal_False;
            xColSet->getPropertyValue( sOptimalSize ) >>= bOptimal;
            if( bOptimal )
            {
                aOptimalColumns.push_back(nCol);
            }
            else
            {
                xColSet->getPropertyValue( msSize ) >>= nColWidth;
            }

            maColumns[nCol].mnSize = nColWidth;

            if( maColumns[nCol].mnSize < nMinWidth )
                maColumns[nCol].mnSize = nMinWidth;

            nCurrentWidth += maColumns[nCol].mnSize;
        }
    }

    // if we have optimal sized rows, distribute what is given (left)
    if( !bFit && !aOptimalColumns.empty() && (nCurrentWidth < rArea.getWidth()) )
    {
        sal_Int32 nLeft = rArea.getWidth() - nCurrentWidth;
        sal_Int32 nDistribute = nLeft / aOptimalColumns.size();

        Int32Vector::iterator iter( aOptimalColumns.begin() );
        while( iter != aOptimalColumns.end() )
        {
            sal_Int32 nOptCol = (*iter++);
            if( iter == aOptimalColumns.end() )
                nDistribute = nLeft;

            maColumns[nOptCol].mnSize += nDistribute;
            nLeft -= nDistribute;
        }

        DBG_ASSERT( nLeft == 0, "svx::TableLayouter::LayoutTableWidtht(), layouting failed!" );
    }

    // now check if merged cells fit
    for( nCol = 1; nCol < nColCount; ++nCol )
    {
        bool bChanges = false;
        MergeableCellVector::iterator iter( aMergedCells[nCol].begin() );

        const sal_Int32 nOldSize = maColumns[nCol].mnSize;

        while( iter != aMergedCells[nCol].end() )
        {
            CellRef xCell( (*iter++) );
            sal_Int32 nMinWidth = xCell->getMinimumSize().Width;

            for( sal_Int32 nMCol = nCol - xCell->getColumnSpan() + 1; (nMCol > 0) && (nMCol < nCol); ++nMCol )
                nMinWidth -= maColumns[nMCol].mnSize;

            if( nMinWidth > maColumns[nCol].mnMinSize )
                maColumns[nCol].mnMinSize = nMinWidth;

            if( nMinWidth > maColumns[nCol].mnSize )
            {
                maColumns[nCol].mnSize = nMinWidth;
                bChanges = true;
            }
        }

        if( bChanges )
            nCurrentWidth += maColumns[nCol].mnSize - nOldSize;
    }

    // now scale if wanted and needed
    if( bFit && (nCurrentWidth != rArea.getWidth()) )
        distribute( maColumns, rArea.getWidth() - nCurrentWidth );

    // last step, update left edges
    sal_Int32 nNewWidth = 0;

    const bool bRTL = (mxTable->getSdrTableObj()->GetWritingMode() == WritingMode_RL_TB);
    RangeIterator<sal_Int32> coliter( 0, nColCount, !bRTL );
    while( coliter.next(nCol ) )
    {
        maColumns[nCol].mnPos = nNewWidth;
        nNewWidth += maColumns[nCol].mnSize;
        if( bFit )
        {
            Reference< XPropertySet > xColSet( xCols->getByIndex(nCol), UNO_QUERY_THROW );
            xColSet->setPropertyValue( msSize, Any( maColumns[nCol].mnSize ) );
        }
    }

    rArea.SetSize( Size( nNewWidth, rArea.GetHeight() ) );
    updateCells( rArea );
}

// -----------------------------------------------------------------------------

void TableLayouter::LayoutTableHeight( Rectangle& rArea, bool bFit )
{
    const sal_Int32 nColCount = getColumnCount();
    const sal_Int32 nRowCount = getRowCount();
    if( nRowCount == 0 )
        return;

    Reference< XTableRows > xRows( mxTable->getRows() );

    MergeVector aMergedCells( nRowCount );
    Int32Vector aOptimalRows;

    const OUString sOptimalSize("OptimalSize");

    // first calculate current height and initial minimum size per column,
    // merged cells will be counted later
    sal_Int32 nCurrentHeight = 0;
    sal_Int32 nCol, nRow;
    for( nRow = 0; nRow < nRowCount; ++nRow )
    {
        sal_Int32 nMinHeight = 0;

        bool bIsEmpty = true; // check if all cells in this row are merged

        for( nCol = 0; nCol < nColCount; ++nCol )
        {
            CellRef xCell( getCell( CellPos( nCol, nRow ) ) );
            if( xCell.is() && !xCell->isMerged() )
            {
                bIsEmpty = false;

                sal_Int32 nRowSpan = xCell->getRowSpan();
                if( nRowSpan > 1 )
                {
                    // merged cells will be evaluated later
                    aMergedCells[nRow+nRowSpan-1].push_back( xCell );
                }
                else
                {
                    nMinHeight = std::max( nMinHeight, xCell->getMinimumSize().Height );
                }
            }
        }

        maRows[nRow].mnMinSize = nMinHeight;

        if( bIsEmpty )
        {
            maRows[nRow].mnSize = 0;
        }
        else
        {
            sal_Int32 nRowHeight = 0;
            Reference< XPropertySet > xRowSet( xRows->getByIndex(nRow), UNO_QUERY_THROW );

            sal_Bool bOptimal = sal_False;
            xRowSet->getPropertyValue( sOptimalSize ) >>= bOptimal;
            if( bOptimal )
            {
                aOptimalRows.push_back( nRow );
            }
            else
            {
                xRowSet->getPropertyValue( msSize ) >>= nRowHeight;
            }

            maRows[nRow].mnSize = nRowHeight;

            if( maRows[nRow].mnSize < nMinHeight )
                maRows[nRow].mnSize = nMinHeight;

            nCurrentHeight += maRows[nRow].mnSize;
        }
    }

    // if we have optimal sized rows, distribute what is given (left)
    if( !bFit && !aOptimalRows.empty() && (nCurrentHeight < rArea.getHeight()) )
    {
        sal_Int32 nLeft = rArea.getHeight() - nCurrentHeight;
        sal_Int32 nDistribute = nLeft / aOptimalRows.size();

        Int32Vector::iterator iter( aOptimalRows.begin() );
        while( iter != aOptimalRows.end() )
        {
            sal_Int32 nOptRow = (*iter++);
            if( iter == aOptimalRows.end() )
                nDistribute = nLeft;

            maRows[nOptRow].mnSize += nDistribute;
            nLeft -= nDistribute;

        }

        DBG_ASSERT( nLeft == 0, "svx::TableLayouter::LayoutTableHeight(), layouting failed!" );
    }

    // now check if merged cells fit
    for( nRow = 1; nRow < nRowCount; ++nRow )
    {
        bool bChanges = false;
        sal_Int32 nOldSize = maRows[nRow].mnSize;

        MergeableCellVector::iterator iter( aMergedCells[nRow].begin() );
        while( iter != aMergedCells[nRow].end() )
        {
            CellRef xCell( (*iter++) );
            sal_Int32 nMinHeight = xCell->getMinimumSize().Height;

            for( sal_Int32 nMRow = nRow - xCell->getRowSpan() + 1; (nMRow > 0) && (nMRow < nRow); ++nMRow )
                nMinHeight -= maRows[nMRow].mnSize;

            if( nMinHeight > maRows[nRow].mnMinSize )
                maRows[nRow].mnMinSize = nMinHeight;

            if( nMinHeight > maRows[nRow].mnSize )
            {
                maRows[nRow].mnSize = nMinHeight;
                bChanges = true;
            }
        }
        if( bChanges )
            nCurrentHeight += maRows[nRow].mnSize - nOldSize;
    }

    // now scale if wanted and needed
    if( bFit && nCurrentHeight != rArea.getHeight() )
        distribute( maRows, rArea.getHeight() - nCurrentHeight );

    // last step, update left edges
    sal_Int32 nNewHeight = 0;
    for( nRow = 0; nRow < nRowCount; ++nRow )
    {
        maRows[nRow].mnPos = nNewHeight;
        nNewHeight += maRows[nRow].mnSize;

        if( bFit )
        {
            Reference< XPropertySet > xRowSet( xRows->getByIndex(nRow), UNO_QUERY_THROW );
            xRowSet->setPropertyValue( msSize, Any( maRows[nRow].mnSize ) );
        }
    }

    rArea.SetSize( Size( rArea.GetWidth(), nNewHeight ) );
    updateCells( rArea );
}

// -----------------------------------------------------------------------------

/** try to fit the table into the given rectangle.
    If the rectangle is to small, it will be grown to fit the table. */
void TableLayouter::LayoutTable( Rectangle& rRectangle, bool bFitWidth, bool bFitHeight )
{
    if( !mxTable.is() )
        return;

    const sal_Int32 nRowCount = mxTable->getRowCount();
    const sal_Int32 nColCount = mxTable->getColumnCount();

    if( (nRowCount != getRowCount()) || (nColCount != getColumnCount()) )
    {
        if( static_cast< sal_Int32 >( maRows.size() ) != nRowCount )
            maRows.resize( nRowCount );

        Reference< XTableRows > xRows( mxTable->getRows() );
        for( sal_Int32 nRow = 0; nRow < nRowCount; nRow++ )
            maRows[nRow].clear();

        if( static_cast< sal_Int32 >( maColumns.size() ) != nColCount )
            maColumns.resize( nColCount );

        for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ )
            maColumns[nCol].clear();
    }

    LayoutTableWidth( rRectangle, bFitWidth );
    LayoutTableHeight( rRectangle, bFitHeight );
    UpdateBorderLayout();
}

// -----------------------------------------------------------------------------

void TableLayouter::updateCells( Rectangle& rRectangle )
{
    const sal_Int32 nColCount = getColumnCount();
    const sal_Int32 nRowCount = getRowCount();

    CellPos aPos;
    for( aPos.mnRow = 0; aPos.mnRow < nRowCount; aPos.mnRow++ )
    {
        for( aPos.mnCol = 0; aPos.mnCol < nColCount; aPos.mnCol++ )
        {
            CellRef xCell( getCell( aPos ) );
            if( xCell.is() )
            {
                basegfx::B2IRectangle aCellArea;
                getCellArea( aPos, aCellArea );

                Rectangle aCellRect;
                aCellRect.Left() = aCellArea.getMinX();
                aCellRect.Right() = aCellArea.getMaxX();
                aCellRect.Top() = aCellArea.getMinY();
                aCellRect.Bottom() = aCellArea.getMaxY();
                aCellRect.Move( rRectangle.Left(), rRectangle.Top() );
                xCell->setCellRect( aCellRect );
            }
        }
    }
}

// -----------------------------------------------------------------------------

CellRef TableLayouter::getCell( const CellPos& rPos ) const
{
    CellRef xCell;
    if( mxTable.is() ) try
    {
        xCell.set( dynamic_cast< Cell* >( mxTable->getCellByPosition( rPos.mnCol, rPos.mnRow ).get() ) );
    }
    catch( Exception& )
    {
        OSL_FAIL( "sdr::table::TableLayouter::getCell(), exception caught!" );
    }
    return xCell;
}

// -----------------------------------------------------------------------------

bool TableLayouter::HasPriority( const SvxBorderLine* pThis, const SvxBorderLine* pOther )
{
    if (!pThis || ((pThis == &gEmptyBorder) && (pOther != 0)))
        return false;
    if (!pOther || (pOther == &gEmptyBorder))
        return true;

    sal_uInt16 nThisSize = pThis->GetScaledWidth();
    sal_uInt16 nOtherSize = pOther->GetScaledWidth();

    if (nThisSize > nOtherSize)
        return true;

    else if (nThisSize < nOtherSize)
    {
        return false;
    }
    else
    {
        if ( pOther->GetInWidth() && !pThis->GetInWidth() )
        {
            return true;
        }
        else if ( pThis->GetInWidth() && !pOther->GetInWidth() )
        {
            return false;
        }
        else
        {
            return true;            //! ???
        }
    }
}

// -----------------------------------------------------------------------------

void TableLayouter::SetBorder( sal_Int32 nCol, sal_Int32 nRow, bool bHorizontal, const SvxBorderLine* pLine )
{
    if( pLine == 0 )
        pLine = &gEmptyBorder;

    SvxBorderLine *pOld = bHorizontal ? maHorizontalBorders[nCol][nRow] : maVerticalBorders[nCol][nRow];

    if( HasPriority( pLine, pOld ) )
    {
        if( (pOld != 0) && (pOld != &gEmptyBorder) )
            delete pOld;

        SvxBorderLine* pNew = ( pLine != &gEmptyBorder ) ?  new SvxBorderLine(*pLine) : &gEmptyBorder;

        if( bHorizontal )
            maHorizontalBorders[nCol][nRow] = pNew;
        else
            maVerticalBorders[nCol][nRow]  = pNew;
    }
}

// -----------------------------------------------------------------------------

void TableLayouter::ClearBorderLayout()
{
    ClearBorderLayout(maHorizontalBorders);
    ClearBorderLayout(maVerticalBorders);
}

// -----------------------------------------------------------------------------

void TableLayouter::ClearBorderLayout(BorderLineMap& rMap)
{
    const sal_Int32 nColCount = rMap.size();

    for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ )
    {
        const sal_Int32 nRowCount = rMap[nCol].size();
        for( sal_Int32 nRow = 0; nRow < nRowCount; nRow++ )
        {
            SvxBorderLine* pLine = rMap[nCol][nRow];
            if( pLine )
            {
                if( pLine != &gEmptyBorder )
                    delete pLine;

                rMap[nCol][nRow] = 0;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void TableLayouter::ResizeBorderLayout()
{
    ClearBorderLayout();
    ResizeBorderLayout(maHorizontalBorders);
    ResizeBorderLayout(maVerticalBorders);
}

// -----------------------------------------------------------------------------

void TableLayouter::ResizeBorderLayout( BorderLineMap& rMap )
{
    const sal_Int32 nColCount = getColumnCount() + 1;
    const sal_Int32 nRowCount = getRowCount() + 1;

    if( sal::static_int_cast<sal_Int32>(rMap.size()) != nColCount )
        rMap.resize( nColCount );

    for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ )
    {
        if( sal::static_int_cast<sal_Int32>(rMap[nCol].size()) != nRowCount )
            rMap[nCol].resize( nRowCount );
    }
}

// -----------------------------------------------------------------------------

void TableLayouter::UpdateBorderLayout()
{
    // make sure old border layout is cleared and border maps have correct size
    ResizeBorderLayout();

    const sal_Int32 nColCount = getColumnCount();
    const sal_Int32 nRowCount = getRowCount();

    CellPos aPos;
    for( aPos.mnRow = 0; aPos.mnRow < nRowCount; aPos.mnRow++ )
    {
        for( aPos.mnCol = 0; aPos.mnCol < nColCount; aPos.mnCol++ )
        {
            CellRef xCell( getCell( aPos ) );
            if( !xCell.is() )
                continue;

            const SvxBoxItem* pThisAttr = (const SvxBoxItem*)xCell->GetItemSet().GetItem( SDRATTR_TABLE_BORDER );
            OSL_ENSURE(pThisAttr,"sdr::table::TableLayouter::UpdateBorderLayout(), no border attribute?");

            if( !pThisAttr )
                continue;

            const sal_Int32 nLastRow = xCell->getRowSpan() + aPos.mnRow;
            const sal_Int32 nLastCol = xCell->getColumnSpan() + aPos.mnCol;

            for( sal_Int32 nRow = aPos.mnRow; nRow < nLastRow; nRow++ )
            {
                SetBorder( aPos.mnCol, nRow, false, pThisAttr->GetLeft() );
                SetBorder( nLastCol, nRow, false, pThisAttr->GetRight() );
            }

            for( sal_Int32 nCol = aPos.mnCol; nCol < nLastCol; nCol++ )
            {
                SetBorder( nCol, aPos.mnRow, true, pThisAttr->GetTop() );
                SetBorder( nCol, nLastRow, true, pThisAttr->GetBottom() );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void TableLayouter::DistributeColumns( ::Rectangle& rArea, sal_Int32 nFirstCol, sal_Int32 nLastCol )
{
    if( mxTable.is() ) try
    {
        const sal_Int32 nColCount = getColumnCount();

        if( (nFirstCol < 0) || (nFirstCol>= nLastCol) || (nLastCol >= nColCount) )
            return;

        sal_Int32 nAllWidth = 0;
        for( sal_Int32 nCol = nFirstCol; nCol <= nLastCol; ++nCol )
            nAllWidth += getColumnWidth(nCol);

        sal_Int32 nWidth = nAllWidth / (nLastCol-nFirstCol+1);

        Reference< XTableColumns > xCols( mxTable->getColumns(), UNO_QUERY_THROW );

        for( sal_Int32 nCol = nFirstCol; nCol <= nLastCol; ++nCol )
        {
            if( nCol == nLastCol )
                nWidth = nAllWidth; // last column get round errors

            Reference< XPropertySet > xColSet( xCols->getByIndex( nCol ), UNO_QUERY_THROW );
            xColSet->setPropertyValue( msSize, Any( nWidth ) );

            nAllWidth -= nWidth;
        }

        LayoutTable( rArea, true, false );
    }
    catch( Exception& e )
    {
        (void)e;
        OSL_FAIL("sdr::table::TableLayouter::DistributeColumns(), exception caught!");
    }
}

// -----------------------------------------------------------------------------

void TableLayouter::DistributeRows( ::Rectangle& rArea, sal_Int32 nFirstRow, sal_Int32 nLastRow )
{
    if( mxTable.is() ) try
    {
        const sal_Int32 nRowCount = mxTable->getRowCount();

        if( (nFirstRow < 0) || (nFirstRow>= nLastRow) || (nLastRow >= nRowCount) )
            return;

        sal_Int32 nAllHeight = 0;
        sal_Int32 nMinHeight = 0;

        for( sal_Int32 nRow = nFirstRow; nRow <= nLastRow; ++nRow )
        {
            nMinHeight = std::max( maRows[nRow].mnMinSize, nMinHeight );
            nAllHeight += maRows[nRow].mnSize;
        }

        const sal_Int32 nRows = (nLastRow-nFirstRow+1);
        sal_Int32 nHeight = nAllHeight / nRows;

        if( nHeight < nMinHeight )
        {
            sal_Int32 nNeededHeight = nRows * nMinHeight;
            rArea.Bottom() += nNeededHeight - nAllHeight;
            nHeight = nMinHeight;
            nAllHeight = nRows * nMinHeight;
        }

        Reference< XTableRows > xRows( mxTable->getRows(), UNO_QUERY_THROW );
        for( sal_Int32 nRow = nFirstRow; nRow <= nLastRow; ++nRow )
        {
            if( nRow == nLastRow )
                nHeight = nAllHeight; // last row get round errors

            Reference< XPropertySet > xRowSet( xRows->getByIndex( nRow ), UNO_QUERY_THROW );
            xRowSet->setPropertyValue( msSize, Any( nHeight ) );

            nAllHeight -= nHeight;
        }

        LayoutTable( rArea, false, true );
    }
    catch( Exception& e )
    {
        (void)e;
        OSL_FAIL("sdr::table::TableLayouter::DistributeRows(), exception caught!");
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
