/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cellcursor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:59:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

    void merge( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan );

    void split_column( sal_Int32 nCol, sal_Int32 nColumns, std::vector< sal_Int32 >& rLeftOvers );
    void split_horizontal( sal_Int32 nColumns );
    void split_row( sal_Int32 nRow, sal_Int32 nRows, std::vector< sal_Int32 >& rLeftOvers );
    void split_vertical( sal_Int32 nRows );
};

} }

#endif
