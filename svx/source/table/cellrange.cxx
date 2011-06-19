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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
