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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svx/svdotable.hxx"
#include "cellcursor.hxx"
#include "tablelayouter.hxx"
#include "cell.hxx"
#include "svx/svdmodel.hxx"
#include "svdstr.hrc"
#include "svdglob.hxx"

// -----------------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::table;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

// -----------------------------------------------------------------------------
// CellCursor
// -----------------------------------------------------------------------------

CellCursor::CellCursor( const TableModelRef & xTable, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
: CellCursorBase( xTable, nLeft, nTop, nRight, nBottom )
{
}

// -----------------------------------------------------------------------------

CellCursor::~CellCursor()
{
}

// -----------------------------------------------------------------------------
// XCellCursor
// -----------------------------------------------------------------------------

Reference< XCell > SAL_CALL CellCursor::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return CellRange::getCellByPosition( nColumn, nRow );
}

// -----------------------------------------------------------------------------

Reference< XCellRange > SAL_CALL CellCursor::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return CellRange::getCellRangeByPosition( nLeft, nTop, nRight, nBottom );
}

// -----------------------------------------------------------------------------

Reference< XCellRange > SAL_CALL CellCursor::getCellRangeByName( const OUString& aRange ) throw (RuntimeException)
{
    return CellRange::getCellRangeByName( aRange );
}

// -----------------------------------------------------------------------------
// XCellCursor
// -----------------------------------------------------------------------------

void SAL_CALL CellCursor::gotoStart(  ) throw (RuntimeException)
{
    mnRight = mnLeft;
    mnBottom = mnTop;
}

// -----------------------------------------------------------------------------

void SAL_CALL CellCursor::gotoEnd(  ) throw (RuntimeException)
{
    mnLeft = mnRight;
    mnTop = mnBottom;
}

// -----------------------------------------------------------------------------

void SAL_CALL CellCursor::gotoNext(  ) throw (RuntimeException)
{
    if( mxTable.is() )
    {
        mnRight++;
        if( mnRight >= mxTable->getColumnCount() )
        {
            // if we past the last column, try skip to the row line
            mnTop++;
            if( mnTop >= mxTable->getRowCount() )
            {
                // if we past the last row, do not move cursor at all
                mnTop--;
                mnRight--;
            }
            else
            {
                // restart at the first column on the next row
                mnRight = 0;
            }
        }
    }

    mnLeft = mnRight;
    mnTop = mnBottom;
}

// -----------------------------------------------------------------------------

void SAL_CALL CellCursor::gotoPrevious(  ) throw (RuntimeException)
{
    if( mxTable.is() )
    {
        if( mnLeft > 0 )
        {
            --mnLeft;
        }
        else if( mnTop > 0 )
        {
            --mnTop;
            mnLeft = mxTable->getColumnCount() - 1;
        }
    }

    mnRight = mnLeft;
    mnBottom = mnTop;
}

// -----------------------------------------------------------------------------

void SAL_CALL CellCursor::gotoOffset( ::sal_Int32 nColumnOffset, ::sal_Int32 nRowOffset ) throw (RuntimeException)
{
    if( mxTable.is() )
    {
        const sal_Int32 nLeft = mnLeft + nColumnOffset;
        if( (nLeft >= 0) && (nLeft < mxTable->getColumnCount() ) )
            mnRight = mnLeft = nLeft;

        const sal_Int32 nTop = mnTop + nRowOffset;
        if( (nTop >= 0) && (nTop < mxTable->getRowCount()) )
            mnTop = mnBottom = nTop;
    }
}

// -----------------------------------------------------------------------------
// XMergeableCellCursor
// -----------------------------------------------------------------------------

/** returns true and the merged cell positions if a merge is valid or false if a merge is
    not valid for that range */
bool CellCursor::GetMergedSelection( CellPos& rStart, CellPos& rEnd )
{
    rStart.mnCol = mnLeft; rStart.mnRow = mnTop;
    rEnd.mnCol = mnRight; rEnd.mnRow = mnBottom;

    // single cell merge is never valid
    if( mxTable.is() && ((mnLeft != mnRight) || (mnTop != mnBottom)) ) try
    {
        CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( mnLeft, mnTop ).get() ) );

        // check if first cell is merged
        if( xCell.is() && xCell->isMerged() )
            findMergeOrigin( mxTable, mnLeft, mnTop, rStart.mnCol, rStart.mnRow );

        // check if last cell is merged
        xCell.set( dynamic_cast< Cell* >( mxTable->getCellByPosition( mnRight, mnBottom ).get() ) );
        if( xCell.is() )
        {
            if( xCell->isMerged() )
            {
                findMergeOrigin( mxTable, mnRight, mnBottom, rEnd.mnCol, rEnd.mnRow );
                // merge not possible if selection is only one cell and all its merges
                if( rEnd == rStart )
                    return false;
                xCell.set( dynamic_cast< Cell* >( mxTable->getCellByPosition( rEnd.mnCol, rEnd.mnRow ).get() ) );
            }
        }
        if( xCell.is() )
        {
            rEnd.mnCol += xCell->getColumnSpan()-1;
            rEnd.mnRow += xCell->getRowSpan()-1;
        }

        // now check if everything is inside the given bounds
        sal_Int32 nRow, nCol;
        for( nRow = rStart.mnRow; nRow <= rEnd.mnRow; nRow++ )
        {
            for( nCol = rStart.mnCol; nCol <= rEnd.mnCol; nCol++ )
            {
                xCell.set( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                if( !xCell.is() )
                    continue;

                if( xCell->isMerged() )
                {
                    sal_Int32 nOriginCol, nOriginRow;
                    if( findMergeOrigin( mxTable, nCol, nRow, nOriginCol, nOriginRow ) )
                    {
                        if( (nOriginCol < rStart.mnCol) || (nOriginRow < rStart.mnRow) )
                            return false;

                        xCell.set( dynamic_cast< Cell* >( mxTable->getCellByPosition( nOriginCol, nOriginRow ).get() ) );
                        if( xCell.is() )
                        {
                            nOriginCol += xCell->getColumnSpan()-1;
                            nOriginRow += xCell->getRowSpan()-1;

                            if( (nOriginCol > rEnd.mnCol) || (nOriginRow > rEnd.mnRow) )
                                return false;
                        }
                    }
                }
                else if( ((nCol + xCell->getColumnSpan() - 1) > rEnd.mnCol) || ((nRow + xCell->getRowSpan() - 1 ) > rEnd.mnRow) )
                {
                    return false;
                }
            }
        }
        return true;
    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::SvmxTableController::GetMergedSelection(), exception caught!");
    }
    return false;
}

// -----------------------------------------------------------------------------

void SAL_CALL CellCursor::merge(  ) throw (NoSupportException, RuntimeException)
{
    CellPos aStart, aEnd;
    if( !GetMergedSelection( aStart, aEnd ) )
        throw NoSupportException();

    if( !mxTable.is() || (mxTable->getSdrTableObj() == 0) )
        throw DisposedException();

    SdrModel* pModel = mxTable->getSdrTableObj()->GetModel();
    const bool bUndo = pModel && mxTable->getSdrTableObj()->IsInserted() && pModel->IsUndoEnabled();

    if( bUndo )
        pModel->BegUndo( ImpGetResStr(STR_TABLE_MERGE) );

    try
    {
        mxTable->merge( aStart.mnCol, aStart.mnRow, aEnd.mnCol - aStart.mnCol + 1, aEnd.mnRow - aStart.mnRow + 1 );
        mxTable->optimize();
        mxTable->setModified(sal_True);
    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::CellCursor::merge(), exception caught!");
    }

    if( bUndo )
        pModel->EndUndo();

    if( pModel )
        pModel->SetChanged();
}

// -----------------------------------------------------------------------------

void CellCursor::split_column( sal_Int32 nCol, sal_Int32 nColumns, std::vector< sal_Int32 >& rLeftOvers )
{
    const sal_Int32 nRowCount = mxTable->getRowCount();

    sal_Int32 nNewCols = 0, nRow;

    // first check how many columns we need to add
    for( nRow = mnTop; nRow <= mnBottom; ++nRow )
    {
        CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
        if( xCell.is() && !xCell->isMerged() )
            nNewCols = std::max( nNewCols, nColumns - xCell->getColumnSpan() + 1 - rLeftOvers[nRow] );
    }

    if( nNewCols > 0 )
    {
        const OUString sWidth( RTL_CONSTASCII_USTRINGPARAM("Width") );
        Reference< XTableColumns > xCols( mxTable->getColumns(), UNO_QUERY_THROW );
        Reference< XPropertySet > xRefColumn( xCols->getByIndex( nCol ), UNO_QUERY_THROW );
        sal_Int32 nWidth = 0;
        xRefColumn->getPropertyValue( sWidth ) >>= nWidth;
        const sal_Int32 nNewWidth = nWidth / (nNewCols + 1);

        // reference column gets new width + rounding errors
        xRefColumn->setPropertyValue( sWidth, Any( nWidth - (nNewWidth * nNewCols) ) );

        xCols->insertByIndex( nCol + 1, nNewCols );
        mnRight += nNewCols;

        // distribute new width
        for( sal_Int32 nNewCol = nCol + nNewCols; nNewCol > nCol; --nNewCol )
        {
            Reference< XPropertySet > xNewCol( xCols->getByIndex( nNewCol ), UNO_QUERY_THROW );
            xNewCol->setPropertyValue( sWidth, Any( nNewWidth ) );
        }
    }

    for( nRow = 0; nRow < nRowCount; ++nRow )
    {
        CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
        if( !xCell.is() || xCell->isMerged() )
        {
            if( nNewCols > 0 )
            {
                // merged cells are ignored, but newly added columns will be added to leftovers
                xCell.set( dynamic_cast< Cell* >(mxTable->getCellByPosition( nCol+1, nRow ).get() ) );
                if( !xCell.is() || !xCell->isMerged() )
                rLeftOvers[nRow] += nNewCols;
            }
        }
        else
        {
            sal_Int32 nRowSpan = xCell->getRowSpan() - 1;
            sal_Int32 nColSpan = xCell->getColumnSpan() - 1;

            if( (nRow >= mnTop) && (nRow <= mnBottom) )
            {
                sal_Int32 nCellsAvailable = 1 + nColSpan + rLeftOvers[nRow];
                if( nColSpan == 0 )
                    nCellsAvailable += nNewCols;

                DBG_ASSERT( nCellsAvailable > nColumns, "sdr::table::CellCursor::split_column(), somethings wrong" );

                sal_Int32 nSplitSpan = (nCellsAvailable / (nColumns + 1)) - 1;

                sal_Int32 nSplitCol = nCol;
                sal_Int32 nSplits = nColumns + 1;
                while( nSplits-- )
                {
                    // last split eats rounding cells
                    if( nSplits == 0 )
                        nSplitSpan = nCellsAvailable - ((nSplitSpan+1) * nColumns) - 1;

                    mxTable->merge( nSplitCol, nRow, nSplitSpan + 1, nRowSpan + 1);
                    if( nSplits > 0 )
                        nSplitCol += nSplitSpan + 1;
                }

                do
                {
                    rLeftOvers[nRow++] = 0;
                }
                while( nRowSpan-- );
                --nRow;
            }
            else
            {
                // cope with outside cells, merge if needed
                if( nColSpan < (rLeftOvers[nRow] + nNewCols) )
                    mxTable->merge( nCol, nRow, (rLeftOvers[nRow] + nNewCols) + 1, nRowSpan + 1 );

                do
                {
                    rLeftOvers[nRow++] = 0; // consumed
                }
                while( nRowSpan-- );
                --nRow;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void CellCursor::split_horizontal( sal_Int32 nColumns )
{
    const sal_Int32 nRowCount = mxTable->getRowCount();

    std::vector< sal_Int32 > aLeftOvers( nRowCount );

    for( sal_Int32 nCol = mnRight; nCol >= mnLeft; --nCol )
        split_column( nCol, nColumns, aLeftOvers );
}

// -----------------------------------------------------------------------------

void CellCursor::split_row( sal_Int32 nRow, sal_Int32 nRows, std::vector< sal_Int32 >& rLeftOvers )
{
    const sal_Int32 nColCount = mxTable->getColumnCount();

    sal_Int32 nNewRows = 0, nCol;

    // first check how many columns we need to add
    for( nCol = mnLeft; nCol <= mnRight; ++nCol )
    {
        CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
        if( xCell.is() && !xCell->isMerged() )
            nNewRows = std::max( nNewRows, nRows - xCell->getRowSpan() + 1 - rLeftOvers[nCol] );
    }

    if( nNewRows > 0 )
    {
        const OUString sHeight( RTL_CONSTASCII_USTRINGPARAM("Height") );
        Reference< XTableRows > xRows( mxTable->getRows(), UNO_QUERY_THROW );
        Reference< XPropertySet > xRefRow( xRows->getByIndex( nRow ), UNO_QUERY_THROW );
        sal_Int32 nHeight = 0;
        xRefRow->getPropertyValue( sHeight ) >>= nHeight;
        const sal_Int32 nNewHeight = nHeight / (nNewRows + 1);

        // reference row gets new height + rounding errors
        xRefRow->setPropertyValue( sHeight, Any( nHeight - (nNewHeight * nNewRows) ) );

        xRows->insertByIndex( nRow + 1, nNewRows );
        mnBottom += nNewRows;

        // distribute new width
        for( sal_Int32 nNewRow = nRow + nNewRows; nNewRow > nRow; --nNewRow )
        {
            Reference< XPropertySet > xNewRow( xRows->getByIndex( nNewRow ), UNO_QUERY_THROW );
            xNewRow->setPropertyValue( sHeight, Any( nNewHeight ) );
        }
    }

    for( nCol = 0; nCol < nColCount; ++nCol )
    {
        CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
        if( !xCell.is() || xCell->isMerged() )
        {
            if( nNewRows )
            {
                // merged cells are ignored, but newly added columns will be added to leftovers
                xCell.set( dynamic_cast< Cell* >(mxTable->getCellByPosition( nCol, nRow+1 ).get() ) );
                if( !xCell.is() || !xCell->isMerged() )
                    rLeftOvers[nCol] += nNewRows;
            }
        }
        else
        {
            sal_Int32 nRowSpan = xCell->getRowSpan() - 1;
            sal_Int32 nColSpan = xCell->getColumnSpan() - 1;

            if( (nCol >= mnLeft) && (nCol <= mnRight) )
            {
                sal_Int32 nCellsAvailable = 1 + nRowSpan + rLeftOvers[nCol];
                if( nRowSpan == 0 )
                    nCellsAvailable += nNewRows;

                DBG_ASSERT( nCellsAvailable > nRows, "sdr::table::CellCursor::split_row(), somethings wrong" );

                sal_Int32 nSplitSpan = (nCellsAvailable / (nRows + 1)) - 1;

                sal_Int32 nSplitRow = nRow;
                sal_Int32 nSplits = nRows + 1;
                while( nSplits-- )
                {
                    // last split eats rounding cells
                    if( nSplits == 0 )
                        nSplitSpan = nCellsAvailable - ((nSplitSpan+1) * nRows) - 1;

                    mxTable->merge( nCol, nSplitRow, nColSpan + 1, nSplitSpan + 1 );
                    if( nSplits > 0 )
                        nSplitRow += nSplitSpan + 1;
                }

                do
                {
                    rLeftOvers[nCol++] = 0;
                }
                while( nColSpan-- );
                --nCol;
            }
            else
            {
                // cope with outside cells, merge if needed
                if( nRowSpan < (rLeftOvers[nCol] + nNewRows) )
                    mxTable->merge( nCol, nRow, nColSpan + 1, (rLeftOvers[nCol] + nNewRows) + 1 );

                do
                {
                    rLeftOvers[nCol++] = 0; // consumed
                }
                while( nColSpan-- );
                --nCol;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void CellCursor::split_vertical( sal_Int32 nRows )
{
    const sal_Int32 nColCount = mxTable->getColumnCount();

    std::vector< sal_Int32 > aLeftOvers( nColCount );

    for( sal_Int32 nRow = mnBottom; nRow >= mnTop; --nRow )
        split_row( nRow, nRows, aLeftOvers );
}

// -----------------------------------------------------------------------------

void SAL_CALL CellCursor::split( sal_Int32 nColumns, sal_Int32 nRows ) throw (NoSupportException, IllegalArgumentException, RuntimeException)
{
    if( (nColumns < 0) || (nRows < 0) )
        throw IllegalArgumentException();

    if( !mxTable.is() || (mxTable->getSdrTableObj() == 0) )
        throw DisposedException();

    SdrModel* pModel = mxTable->getSdrTableObj()->GetModel();
    const bool bUndo = pModel && mxTable->getSdrTableObj()->IsInserted() && pModel->IsUndoEnabled();
    if( bUndo )
        pModel->BegUndo( ImpGetResStr(STR_TABLE_SPLIT) );

    try
    {
        if( nColumns > 0 )
            split_horizontal( nColumns );

        if( nRows > 0 )
            split_vertical( nRows );

        if( nColumns > 0 ||nRows > 0 )
            mxTable->setModified(sal_True);
    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::CellCursor::split(), exception caught!");
        throw NoSupportException();
    }

    if( bUndo )
        pModel->EndUndo();

    if( pModel )
        pModel->SetChanged();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL CellCursor::isMergeable(  ) throw (RuntimeException)
{
    CellPos aStart, aEnd;
    return GetMergedSelection( aStart, aEnd ) ? sal_True : sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL CellCursor::isUnmergeable(  ) throw (RuntimeException)
{
    // this is true if there is at least one merged cell in the current range
    for( sal_Int32 nRow = mnTop; nRow <= mnBottom; nRow++ )
    {
        for( sal_Int32 nCol = mnLeft; nCol <= mnRight; nCol++ )
        {
            CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
            if( xCell.is() && ( (xCell->getRowSpan() > 1) || (xCell->getColumnSpan() > 1) ) )
                return sal_True;
        }
    }
    return sal_False;
}

// -----------------------------------------------------------------------------

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
