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

#ifndef _SVX_CELLCURSOR_HXX_
#define _SVX_CELLCURSOR_HXX_

#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/XCellCursor.hpp>
#include <cppuhelper/implbase2.hxx>
#include "cellrange.hxx"

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

struct CellPos;

// -----------------------------------------------------------------------------
// CellCursor
// -----------------------------------------------------------------------------

typedef ::cppu::ImplInheritanceHelper2< CellRange, ::com::sun::star::table::XCellCursor, ::com::sun::star::table::XMergeableCellRange > CellCursorBase;

class CellCursor : public CellCursorBase
{
public:
    CellCursor( const TableModelRef& xTableModel, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom );
    virtual ~CellCursor();

    // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByName( const ::rtl::OUString& aRange ) throw (::com::sun::star::uno::RuntimeException);

    // XCellCursor
    virtual void SAL_CALL gotoStart(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoEnd(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoNext(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoPrevious(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoOffset( ::sal_Int32 nColumnOffset, ::sal_Int32 nRowOffset ) throw (::com::sun::star::uno::RuntimeException);

    // XMergeableCellRange
    virtual void SAL_CALL merge(  ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL split( ::sal_Int32 Columns, ::sal_Int32 Rows ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isMergeable(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isUnmergeable(  ) throw (::com::sun::star::uno::RuntimeException);

protected:
    bool GetMergedSelection( CellPos& rStart, CellPos& rEnd );

    void split_column( sal_Int32 nCol, sal_Int32 nColumns, std::vector< sal_Int32 >& rLeftOvers );
    void split_horizontal( sal_Int32 nColumns );
    void split_row( sal_Int32 nRow, sal_Int32 nRows, std::vector< sal_Int32 >& rLeftOvers );
    void split_vertical( sal_Int32 nRows );
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
