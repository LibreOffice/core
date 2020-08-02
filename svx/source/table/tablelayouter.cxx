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

#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <libxml/xmlwriter.h>

#include <cell.hxx>
#include <o3tl/safeint.hxx>
#include <tablemodel.hxx>
#include "tablelayouter.hxx"
#include <svx/svdotable.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;


namespace sdr::table {


static SvxBorderLine gEmptyBorder;

const OUStringLiteral gsSize( "Size" );

TableLayouter::TableLayouter( const TableModelRef& xTableModel )
: mxTable( xTableModel )
{
}


TableLayouter::~TableLayouter()
{
    ClearBorderLayout();
}


basegfx::B2ITuple TableLayouter::getCellSize( const CellRef& xCell, const CellPos& rPos  ) const
{
    sal_Int32 width = 0;
    sal_Int32 height = 0;

    try
    {
        if( xCell.is() && !xCell->isMerged() )
        {
            CellPos aPos( rPos );

            sal_Int32 nRowCount = getRowCount();
            sal_Int32 nRowSpan = std::max( xCell->getRowSpan(), sal_Int32(1) );
            while( nRowSpan && (aPos.mnRow < nRowCount) )
            {
                if( static_cast<sal_Int32>(maRows.size()) <= aPos.mnRow )
                    break;

                height = o3tl::saturating_add(height, maRows[aPos.mnRow++].mnSize);
                nRowSpan--;
            }

            sal_Int32 nColCount = getColumnCount();
            sal_Int32 nColSpan = std::max( xCell->getColumnSpan(), sal_Int32(1) );
            while( nColSpan && (aPos.mnCol < nColCount ) )
            {
                if( static_cast<sal_Int32>(maColumns.size()) <= aPos.mnCol )
                    break;

                width = o3tl::saturating_add(width, maColumns[aPos.mnCol++].mnSize);
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


bool TableLayouter::getCellArea( const CellRef& xCell, const CellPos& rPos, basegfx::B2IRectangle& rArea ) const
{
    try
    {
        if( xCell.is() && !xCell->isMerged() && isValid(rPos) )
        {
            const basegfx::B2ITuple aCellSize( getCellSize( xCell, rPos ) );
            const bool bRTL = (mxTable->getSdrTableObj()->GetWritingMode() == WritingMode_RL_TB);

            if( (rPos.mnCol < static_cast<sal_Int32>(maColumns.size())) && (rPos.mnRow < static_cast<sal_Int32>(maRows.size()) ) )
            {
                const sal_Int32 y = maRows[rPos.mnRow].mnPos;

                sal_Int32 endy;
                if (o3tl::checked_add(y, aCellSize.getY(), endy))
                    return false;

                if(bRTL)
                {
                    ///For RTL Table Calculate the Right End of cell instead of Left
                    const sal_Int32 x = maColumns[rPos.mnCol].mnPos + maColumns[rPos.mnCol].mnSize;
                    sal_Int32 startx;
                    if (o3tl::checked_sub(x, aCellSize.getX(), startx))
                        return false;
                    rArea = basegfx::B2IRectangle(startx, y, x, endy);
                }
                else
                {
                    const sal_Int32 x = maColumns[rPos.mnCol].mnPos;
                    sal_Int32 endx;
                    if (o3tl::checked_add(x, aCellSize.getX(), endx))
                        return false;
                    rArea = basegfx::B2IRectangle(x, y, endx, endy);
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


sal_Int32 TableLayouter::getRowHeight( sal_Int32 nRow ) const
{
    if( isValidRow(nRow) )
        return maRows[nRow].mnSize;
    else
        return 0;
}


sal_Int32 TableLayouter::getColumnWidth( sal_Int32 nColumn ) const
{
    if( isValidColumn(nColumn) )
        return maColumns[nColumn].mnSize;
    else
        return 0;
}

sal_Int32 TableLayouter::calcPreferredColumnWidth( sal_Int32 nColumn, Size aSize ) const
{
    sal_Int32 nRet = 0;
    for ( sal_uInt32 nRow = 0; nRow < static_cast<sal_uInt32>(maRows.size()); ++nRow )
    {
        // Account for the space desired by spanned columns.
        // Only the last spanned cell will try to ensure sufficient space,
        // by looping through the previous columns, subtracting their portion.
        sal_Int32 nWish = 0;
        sal_Int32 nSpannedColumn = nColumn;
        bool bFindSpan = true;
        while ( bFindSpan && isValidColumn(nSpannedColumn) )
        {
            // recursive function call gets earlier portion of spanned column.
            if ( nSpannedColumn < nColumn )
                nWish -= calcPreferredColumnWidth( nSpannedColumn, aSize );

            CellRef xCell( getCell( CellPos( nSpannedColumn, nRow ) ) );
            if ( xCell.is() && !xCell->isMerged()
                 && (xCell->getColumnSpan() == 1 || nSpannedColumn < nColumn) )
            {
                nWish += xCell->calcPreferredWidth(aSize);
                bFindSpan = false;
            }
            else if ( xCell.is() && xCell->isMerged()
                      && nColumn == nSpannedColumn
                      && isValidColumn(nColumn + 1) )
            {
                xCell = getCell( CellPos( nColumn + 1, nRow ) );
                bFindSpan = xCell.is() && !xCell->isMerged();
            }
            nSpannedColumn--;
        }
        nRet = std::max( nRet, nWish );
    }
    return nRet;
}


bool TableLayouter::isEdgeVisible( sal_Int32 nEdgeX, sal_Int32 nEdgeY, bool bHorizontal ) const
{
    const BorderLineMap& rMap = bHorizontal ? maHorizontalBorders : maVerticalBorders;

    if( (nEdgeX >= 0) && (nEdgeX < sal::static_int_cast<sal_Int32>(rMap.size())) &&
        (nEdgeY >= 0) && (nEdgeY < sal::static_int_cast<sal_Int32>(rMap[nEdgeX].size())) )
    {
        return rMap[nEdgeX][nEdgeY] != nullptr;
    }
    else
    {
        OSL_FAIL( "sdr::table::TableLayouter::getBorderLine(), invalid edge!" );
    }

    return false;
}


/** returns the requested borderline in rpBorderLine or a null pointer if there is no border at this edge */
SvxBorderLine* TableLayouter::getBorderLine( sal_Int32 nEdgeX, sal_Int32 nEdgeY, bool bHorizontal )const
{
    SvxBorderLine* pLine = nullptr;

    const BorderLineMap& rMap = bHorizontal ? maHorizontalBorders : maVerticalBorders;

    if( (nEdgeX >= 0) && (nEdgeX < sal::static_int_cast<sal_Int32>(rMap.size())) &&
        (nEdgeY >= 0) && (nEdgeY < sal::static_int_cast<sal_Int32>(rMap[nEdgeX].size())) )
    {
        pLine = rMap[nEdgeX][nEdgeY];
        if( pLine == &gEmptyBorder )
            pLine = nullptr;
    }
    else
    {
        OSL_FAIL( "sdr::table::TableLayouter::getBorderLine(), invalid edge!" );
    }

    return pLine;
}

std::vector<EdgeInfo> TableLayouter::getHorizontalEdges()
{
    std::vector<EdgeInfo> aReturn;
    sal_Int32 nRowSize = sal_Int32(maRows.size());
    for (sal_Int32 i = 0; i <= nRowSize; i++)
    {
        sal_Int32 nEdgeMin = 0;
        sal_Int32 nEdgeMax = 0;
        sal_Int32 nEdge = getHorizontalEdge(i, &nEdgeMin, &nEdgeMax);
        nEdgeMin -= nEdge;
        nEdgeMax -= nEdge;
        aReturn.emplace_back(i, nEdge, nEdgeMin, nEdgeMax);
    }
    return aReturn;
}

std::vector<EdgeInfo> TableLayouter::getVerticalEdges()
{
    std::vector<EdgeInfo> aReturn;
    sal_Int32 nColumnSize = sal_Int32(maColumns.size());
    for (sal_Int32 i = 0; i <= nColumnSize; i++)
    {
        sal_Int32 nEdgeMin = 0;
        sal_Int32 nEdgeMax = 0;
        sal_Int32 nEdge = getVerticalEdge(i, &nEdgeMin, &nEdgeMax);
        nEdgeMin -= nEdge;
        nEdgeMax -= nEdge;
        aReturn.emplace_back(i, nEdge, nEdgeMin, nEdgeMax);
    }
    return aReturn;
}

sal_Int32 TableLayouter::getHorizontalEdge( int nEdgeY, sal_Int32* pnMin /*= 0*/, sal_Int32* pnMax /*= 0*/ )
{
    sal_Int32 nRet = 0;
    const sal_Int32 nRowCount = getRowCount();
    if( (nEdgeY >= 0) && (nEdgeY <= nRowCount ) )
        nRet = maRows[std::min(static_cast<sal_Int32>(nEdgeY),nRowCount-1)].mnPos;

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


sal_Int32 TableLayouter::getVerticalEdge( int nEdgeX, sal_Int32* pnMin /*= 0*/, sal_Int32* pnMax /*= 0*/ )
{
    sal_Int32 nRet = 0;

    const sal_Int32 nColCount = getColumnCount();
    if( (nEdgeX >= 0) && (nEdgeX <= nColCount ) )
        nRet = maColumns[std::min(static_cast<sal_Int32>(nEdgeX),nColCount-1)].mnPos;

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
        if( !xCell->isMerged() )
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


sal_Int32 TableLayouter::distribute( LayoutVector& rLayouts, sal_Int32 nDistribute )
{
    // break loops after 100 runs to avoid freezing office due to developer error
    sal_Int32 nSafe = 100;

    const std::size_t nCount = rLayouts.size();
    std::size_t nIndex;

    bool bConstrainsBroken = false;

    do
    {
        bConstrainsBroken = false;

        // first enforce minimum size constrains on all entities
        for( nIndex = 0; nIndex < nCount; ++nIndex )
        {
            Layout& rLayout = rLayouts[nIndex];
            if( rLayout.mnSize < rLayout.mnMinSize )
            {
                sal_Int32 nDiff(0);
                bConstrainsBroken |= o3tl::checked_sub(rLayout.mnMinSize, rLayout.mnSize, nDiff);
                bConstrainsBroken |= o3tl::checked_sub(nDistribute, nDiff, nDistribute);
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
                nCurrentWidth = o3tl::saturating_add(nCurrentWidth, rLayout.mnSize);
        }

        // now distribute over entities
        if( (nCurrentWidth != 0) && (nDistribute != 0) )
        {
            sal_Int32 nDistributed = nDistribute;
            for( nIndex = 0; nIndex < nCount; ++nIndex )
            {
                Layout& rLayout = rLayouts[nIndex];
                if( (nDistribute > 0) || (rLayout.mnSize > rLayout.mnMinSize) )
                {
                    sal_Int32 n(nDistributed); // for last entity use up rest
                    if (nIndex != (nCount-1))
                    {
                        bConstrainsBroken |= o3tl::checked_multiply(nDistribute, rLayout.mnSize, n);
                        n /= nCurrentWidth;
                    }

                    bConstrainsBroken |= o3tl::checked_add(rLayout.mnSize, n, rLayout.mnSize);
                    nDistributed -= n;

                    if( rLayout.mnSize < rLayout.mnMinSize )
                        bConstrainsBroken = true;
                }
            }
        }
    } while( bConstrainsBroken && --nSafe );

    sal_Int32 nSize = 0;
    for( nIndex = 0; nIndex < nCount; ++nIndex )
        nSize = o3tl::saturating_add(nSize, rLayouts[nIndex].mnSize);

    return nSize;
}


typedef std::vector< CellRef > MergeableCellVector;
typedef std::vector< MergeableCellVector > MergeVector;


void TableLayouter::LayoutTableWidth( tools::Rectangle& rArea, bool bFit )
{
    const sal_Int32 nColCount = getColumnCount();
    const sal_Int32 nRowCount = getRowCount();
    if( nColCount == 0 )
        return;

    MergeVector aMergedCells( nColCount );
    std::vector<sal_Int32> aOptimalColumns;

    const OUString sOptimalSize("OptimalSize");

    if( sal::static_int_cast< sal_Int32 >( maColumns.size() ) != nColCount )
        maColumns.resize( nColCount );

    Reference< XTableColumns > xCols( mxTable->getColumns(), UNO_SET_THROW );

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
                    nMinWidth = std::max( nMinWidth, xCell->getMinimumWidth() );
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
            bool bOptimal = false;
            xColSet->getPropertyValue( sOptimalSize ) >>= bOptimal;
            if( bOptimal )
            {
                aOptimalColumns.push_back(nCol);
            }
            else
            {
                xColSet->getPropertyValue( gsSize ) >>= nColWidth;
            }

            maColumns[nCol].mnSize = std::max( nColWidth, nMinWidth);

            nCurrentWidth = o3tl::saturating_add(nCurrentWidth, maColumns[nCol].mnSize);
        }
    }

    // if we have optimal sized rows, distribute what is given (left)
    if( !bFit && !aOptimalColumns.empty() && (nCurrentWidth < rArea.getWidth()) )
    {
        sal_Int32 nLeft = rArea.getWidth() - nCurrentWidth;
        sal_Int32 nDistribute = nLeft / aOptimalColumns.size();

        auto iter( aOptimalColumns.begin() );
        while( iter != aOptimalColumns.end() )
        {
            sal_Int32 nOptCol = *iter++;
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

        const sal_Int32 nOldSize = maColumns[nCol].mnSize;

        for( const CellRef& xCell : aMergedCells[nCol] )
        {
            sal_Int32 nMinWidth = xCell->getMinimumWidth();

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
        {
            nCurrentWidth = o3tl::saturating_add(nCurrentWidth, maColumns[nCol].mnSize - nOldSize);
        }
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
        nNewWidth = o3tl::saturating_add(nNewWidth, maColumns[nCol].mnSize);
        if( bFit )
        {
            Reference< XPropertySet > xColSet( xCols->getByIndex(nCol), UNO_QUERY_THROW );
            xColSet->setPropertyValue( gsSize, Any( maColumns[nCol].mnSize ) );
        }
    }

    rArea.SetSize( Size( nNewWidth, rArea.GetHeight() ) );
    updateCells( rArea );
}


void TableLayouter::LayoutTableHeight( tools::Rectangle& rArea, bool bFit )
{
    const sal_Int32 nColCount = getColumnCount();
    const sal_Int32 nRowCount = getRowCount();
    if( nRowCount == 0 )
        return;

    Reference< XTableRows > xRows( mxTable->getRows() );

    MergeVector aMergedCells( nRowCount );
    std::vector<sal_Int32> aOptimalRows;

    const OUString sOptimalSize("OptimalSize");

    // first calculate current height and initial minimum size per column,
    // merged cells will be counted later
    sal_Int32 nCurrentHeight = 0;
    sal_Int32 nCol, nRow;
    for( nRow = 0; nRow < nRowCount; ++nRow )
    {
        sal_Int32 nMinHeight = 0;

        bool bIsEmpty = true; // check if all cells in this row are merged
        bool bRowHasText = false;

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
                    bool bCellHasText = xCell->hasText();
                    if (bRowHasText == bCellHasText)
                    {
                        nMinHeight = std::max( nMinHeight, xCell->getMinimumHeight() );
                    }
                    else if ( !bRowHasText && bCellHasText )
                    {
                        bRowHasText = true;
                        nMinHeight = xCell->getMinimumHeight();
                    }
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

            bool bOptimal = false;
            xRowSet->getPropertyValue( sOptimalSize ) >>= bOptimal;
            if( bOptimal )
            {
                aOptimalRows.push_back( nRow );
            }
            else
            {
                xRowSet->getPropertyValue( gsSize ) >>= nRowHeight;
            }

            maRows[nRow].mnSize = nRowHeight;

            if( maRows[nRow].mnSize < nMinHeight )
                maRows[nRow].mnSize = nMinHeight;

            nCurrentHeight = o3tl::saturating_add(nCurrentHeight, maRows[nRow].mnSize);
        }
    }

    // if we have optimal sized rows, distribute what is given (left)
    if( !bFit && !aOptimalRows.empty() && (nCurrentHeight < rArea.getHeight()) )
    {
        sal_Int32 nLeft = rArea.getHeight() - nCurrentHeight;
        sal_Int32 nDistribute = nLeft / aOptimalRows.size();

        auto iter( aOptimalRows.begin() );
        while( iter != aOptimalRows.end() )
        {
            sal_Int32 nOptRow = *iter++;
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

        for( const CellRef& xCell : aMergedCells[nRow] )
        {
            sal_Int32 nMinHeight = xCell->getMinimumHeight();

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
            nCurrentHeight = o3tl::saturating_add(nCurrentHeight, maRows[nRow].mnSize - nOldSize);
    }

    // now scale if wanted and needed
    if( bFit && nCurrentHeight != rArea.getHeight() )
        distribute(maRows, o3tl::saturating_sub<sal_Int32>(rArea.getHeight(), nCurrentHeight));

    // last step, update left edges
    sal_Int32 nNewHeight = 0;
    for( nRow = 0; nRow < nRowCount; ++nRow )
    {
        maRows[nRow].mnPos = nNewHeight;
        nNewHeight = o3tl::saturating_add(nNewHeight, maRows[nRow].mnSize);

        if( bFit )
        {
            Reference< XPropertySet > xRowSet( xRows->getByIndex(nRow), UNO_QUERY_THROW );
            xRowSet->setPropertyValue( gsSize, Any( maRows[nRow].mnSize ) );
        }
    }

    rArea.SetSize( Size( rArea.GetWidth(), nNewHeight ) );
    updateCells( rArea );
}


/** try to fit the table into the given rectangle.
    If the rectangle is too small, it will be grown to fit the table. */
void TableLayouter::LayoutTable( tools::Rectangle& rRectangle, bool bFitWidth, bool bFitHeight )
{
    if( !mxTable.is() )
        return;

    const sal_Int32 nRowCount = mxTable->getRowCount();
    const sal_Int32 nColCount = mxTable->getColumnCount();

    if( (nRowCount != getRowCount()) || (nColCount != getColumnCount()) )
    {
        if( static_cast< sal_Int32 >( maRows.size() ) != nRowCount )
            maRows.resize( nRowCount );

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


void TableLayouter::updateCells( tools::Rectangle const & rRectangle )
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
                if( getCellArea( xCell, aPos, aCellArea ) )
                {
                    tools::Rectangle aCellRect;
                    aCellRect.SetLeft( aCellArea.getMinX() );
                    aCellRect.SetRight( aCellArea.getMaxX() );
                    aCellRect.SetTop( aCellArea.getMinY() );
                    aCellRect.SetBottom( aCellArea.getMaxY() );
                    aCellRect.Move( rRectangle.Left(), rRectangle.Top() );
                    xCell->setCellRect( aCellRect );
                }
            }
        }
    }
}


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


bool TableLayouter::HasPriority( const SvxBorderLine* pThis, const SvxBorderLine* pOther )
{
    if (!pThis || ((pThis == &gEmptyBorder) && (pOther != nullptr)))
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

void TableLayouter::SetBorder( sal_Int32 nCol, sal_Int32 nRow, bool bHorizontal, const SvxBorderLine* pLine )
{
    if (!pLine)
        pLine = &gEmptyBorder;

    BorderLineMap& rMap = bHorizontal ? maHorizontalBorders : maVerticalBorders;

    if( (nCol >= 0) && (nCol < sal::static_int_cast<sal_Int32>(rMap.size())) &&
        (nRow >= 0) && (nRow < sal::static_int_cast<sal_Int32>(rMap[nCol].size())) )
    {
        SvxBorderLine *pOld = rMap[nCol][nRow];

        if (HasPriority(pLine, pOld))
        {
            if (pOld && pOld != &gEmptyBorder)
                delete pOld;

            SvxBorderLine* pNew = (pLine != &gEmptyBorder) ?  new SvxBorderLine(*pLine) : &gEmptyBorder;

            rMap[nCol][nRow] = pNew;
        }
    }
    else
    {
        OSL_FAIL( "sdr::table::TableLayouter::SetBorder(), invalid border!" );
    }
}

void TableLayouter::ClearBorderLayout()
{
    ClearBorderLayout(maHorizontalBorders);
    ClearBorderLayout(maVerticalBorders);
}

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

                rMap[nCol][nRow] = nullptr;
            }
        }
    }
}

void TableLayouter::ResizeBorderLayout()
{
    ClearBorderLayout();
    ResizeBorderLayout(maHorizontalBorders);
    ResizeBorderLayout(maVerticalBorders);
}


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

            const SvxBoxItem* pThisAttr = xCell->GetItemSet().GetItem<SvxBoxItem>( SDRATTR_TABLE_BORDER );
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


void TableLayouter::DistributeColumns( ::tools::Rectangle& rArea,
                                       sal_Int32 nFirstCol,
                                       sal_Int32 nLastCol,
                                       const bool bOptimize,
                                       const bool bMinimize )
{
    if( !mxTable.is() )
        return;

    try
    {
        const sal_Int32 nColCount = getColumnCount();
        Reference< XTableColumns > xCols( mxTable->getColumns(), UNO_SET_THROW );
        const Size aSize(0xffffff, 0xffffff);

        //special case - optimize a single column
        if ( (bOptimize || bMinimize) && nFirstCol == nLastCol )
        {
            const sal_Int32 nWish = calcPreferredColumnWidth(nFirstCol, aSize);
            if ( nWish < getColumnWidth(nFirstCol) )
            {
                Reference< XPropertySet > xColSet( xCols->getByIndex(nFirstCol), UNO_QUERY_THROW );
                xColSet->setPropertyValue( gsSize, Any( nWish ) );

                //FitWidth automatically distributes the new excess space
                LayoutTable( rArea, /*bFitWidth=*/!bMinimize, /*bFitHeight=*/false );
            }
        }

        if( (nFirstCol < 0) || (nFirstCol>= nLastCol) || (nLastCol >= nColCount) )
            return;

        sal_Int32 nAllWidth = 0;
        float fAllWish = 0;
        sal_Int32 nUnused = 0;
        std::vector<sal_Int32> aWish(nColCount);

        for( sal_Int32 nCol = nFirstCol; nCol <= nLastCol; ++nCol )
            nAllWidth += getColumnWidth(nCol);

        const sal_Int32 nEqualWidth = nAllWidth / (nLastCol-nFirstCol+1);

        //pass 1 - collect unneeded space (from an equal width perspective)
        if ( bMinimize || bOptimize )
        {
            for( sal_Int32 nCol = nFirstCol; nCol <= nLastCol; ++nCol )
            {
                const sal_Int32 nIndex = nCol - nFirstCol;
                aWish[nIndex] = calcPreferredColumnWidth(nCol, aSize);
                fAllWish += aWish[nIndex];
                if ( aWish[nIndex] < nEqualWidth )
                    nUnused += nEqualWidth - aWish[nIndex];
            }
        }
        const sal_Int32 nDistributeExcess = nAllWidth - fAllWish;

        sal_Int32 nWidth = nEqualWidth;
        for( sal_Int32 nCol = nFirstCol; nCol <= nLastCol; ++nCol )
        {
            if ( !bMinimize && nCol == nLastCol )
                nWidth = nAllWidth; // last column gets rounding/logic errors
            else if ( (bMinimize || bOptimize) && fAllWish )
            {
                //pass 2 - first come, first served when requesting from the
                //  unneeded pool, or proportionally allocate excess.
                const sal_Int32 nIndex = nCol - nFirstCol;
                if ( aWish[nIndex] > nEqualWidth + nUnused )
                {
                    nWidth = nEqualWidth + nUnused;
                    nUnused = 0;
                }
                else
                {
                    nWidth = aWish[nIndex];
                    if ( aWish[nIndex] > nEqualWidth )
                        nUnused -= aWish[nIndex] - nEqualWidth;

                    if ( !bMinimize && nDistributeExcess > 0 )
                        nWidth += nWidth / fAllWish * nDistributeExcess;
                }
            }

            Reference< XPropertySet > xColSet( xCols->getByIndex( nCol ), UNO_QUERY_THROW );
            xColSet->setPropertyValue( gsSize, Any( nWidth ) );

            nAllWidth -= nWidth;
        }

        LayoutTable( rArea, !bMinimize, false );
    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::TableLayouter::DistributeColumns(), exception caught!");
    }
}


void TableLayouter::DistributeRows( ::tools::Rectangle& rArea,
                                    sal_Int32 nFirstRow,
                                    sal_Int32 nLastRow,
                                    const bool bOptimize,
                                    const bool bMinimize )
{
    if( !mxTable.is() )
        return;

    try
    {
        const sal_Int32 nRowCount = mxTable->getRowCount();
        Reference< XTableRows > xRows( mxTable->getRows(), UNO_SET_THROW );
        sal_Int32 nMinHeight = 0;

        //special case - minimize a single row
        if ( bMinimize && nFirstRow == nLastRow )
        {
            const sal_Int32 nWish = std::max( maRows[nFirstRow].mnMinSize, nMinHeight );
            if ( nWish < getRowHeight(nFirstRow) )
            {
                Reference< XPropertySet > xRowSet( xRows->getByIndex( nFirstRow ), UNO_QUERY_THROW );
                xRowSet->setPropertyValue( gsSize, Any( nWish ) );

                LayoutTable( rArea, /*bFitWidth=*/false, /*bFitHeight=*/!bMinimize );
            }
        }

        if( (nFirstRow < 0) || (nFirstRow>= nLastRow) || (nLastRow >= nRowCount) )
            return;

        sal_Int32 nAllHeight = 0;
        sal_Int32 nMaxHeight = 0;

        for( sal_Int32 nRow = nFirstRow; nRow <= nLastRow; ++nRow )
        {
            nMinHeight = std::max( maRows[nRow].mnMinSize, nMinHeight );
            nMaxHeight = std::max( maRows[nRow].mnSize, nMaxHeight );
            nAllHeight += maRows[nRow].mnSize;
        }

        const sal_Int32 nRows = nLastRow-nFirstRow+1;
        sal_Int32 nHeight = nAllHeight / nRows;

        if ( !bMinimize && nHeight < nMaxHeight )
        {
            if ( !bOptimize )
            {
                sal_Int32 nNeededHeight = nRows * nMaxHeight;
                rArea.AdjustBottom(nNeededHeight - nAllHeight );
                nHeight = nMaxHeight;
                nAllHeight = nRows * nMaxHeight;
            }
            else if ( nHeight < nMinHeight )
            {
                sal_Int32 nNeededHeight = nRows * nMinHeight;
                rArea.AdjustBottom(nNeededHeight - nAllHeight );
                nHeight = nMinHeight;
                nAllHeight = nRows * nMinHeight;
            }
        }

        for( sal_Int32 nRow = nFirstRow; nRow <= nLastRow; ++nRow )
        {
            if ( bMinimize )
                nHeight = maRows[nRow].mnMinSize;
            else if ( nRow == nLastRow )
                nHeight = nAllHeight; // last row get round errors

            Reference< XPropertySet > xRowSet( xRows->getByIndex( nRow ), UNO_QUERY_THROW );
            xRowSet->setPropertyValue( gsSize, Any( nHeight ) );

            nAllHeight -= nHeight;
        }

        LayoutTable( rArea, false, !bMinimize );
    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::TableLayouter::DistributeRows(), exception caught!");
    }
}

void TableLayouter::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("TableLayouter"));

    xmlTextWriterStartElement(pWriter, BAD_CAST("columns"));
    for (const auto& rColumn : maColumns)
        rColumn.dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);

    xmlTextWriterStartElement(pWriter, BAD_CAST("rows"));
    for (const auto& rRow : maRows)
        rRow.dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);

    xmlTextWriterEndElement(pWriter);
}

void TableLayouter::Layout::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("TableLayouter_Layout"));

    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pos"), BAD_CAST(OString::number(mnPos).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("size"), BAD_CAST(OString::number(mnSize).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("minSize"), BAD_CAST(OString::number(mnMinSize).getStr()));

    xmlTextWriterEndElement(pWriter);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
