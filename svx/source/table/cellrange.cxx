/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "cellrange.hxx"

// -----------------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

// -----------------------------------------------------------------------------
// CellRange
// -----------------------------------------------------------------------------

CellRange::CellRange( const TableModelRef & xTable, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
: mxTable( xTable )
, mnLeft(nLeft)
, mnTop(nTop)
, mnRight(nRight)
, mnBottom(nBottom)
{
}

// -----------------------------------------------------------------------------

CellRange::~CellRange()
{
}

// -----------------------------------------------------------------------------
// ICellRange
// -----------------------------------------------------------------------------

sal_Int32 CellRange::getLeft()
{
    return mnLeft;
}

sal_Int32 CellRange::getTop()
{
    return mnTop;
}

sal_Int32 CellRange::getRight()
{
    return mnRight;
}

sal_Int32 CellRange::getBottom()
{
    return mnBottom;
}

Reference< XTable > CellRange::getTable()
{
    return mxTable.get();
}

// -----------------------------------------------------------------------------
// XCellRange
// -----------------------------------------------------------------------------

Reference< XCell > SAL_CALL CellRange::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return mxTable->getCellByPosition( mnLeft + nColumn, mnTop + nRow );
}

// -----------------------------------------------------------------------------

Reference< XCellRange > SAL_CALL CellRange::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw (IndexOutOfBoundsException, RuntimeException)
{
    if( (nLeft >= 0 ) && (nTop >= 0) && (nRight >= nLeft) && (nBottom >= nTop)  )
    {
        nLeft += mnLeft;
        nTop += mnTop;
        nRight += mnLeft;
        nBottom += mnTop;

        const sal_Int32 nMaxColumns = (mnRight == -1) ? mxTable->getColumnCount() : mnLeft;
        const sal_Int32 nMaxRows = (mnBottom == -1) ? mxTable->getRowCount() : mnBottom;
        if( (nLeft < nMaxColumns) && (nRight < nMaxColumns) && (nTop < nMaxRows) && (nBottom < nMaxRows) )
        {
            return mxTable->getCellRangeByPosition( nLeft, nTop, nRight, nBottom );
        }
    }
    throw IndexOutOfBoundsException();
}

// -----------------------------------------------------------------------------

Reference< XCellRange > SAL_CALL CellRange::getCellRangeByName( const OUString& /*aRange*/ ) throw (RuntimeException)
{
    return Reference< XCellRange >();
}

// -----------------------------------------------------------------------------

} }
