/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cursuno.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 11:54:42 $
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

#ifndef SC_CURSUNO_HXX
#define SC_CURSUNO_HXX

#ifndef SC_CELLSUNO_HXX
#include "cellsuno.hxx"
#endif

#ifndef _COM_SUN_STAR_TABLE_XCELLCURSOR_HPP_
#include <com/sun/star/table/XCellCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCELLCURSOR_HPP_
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XUSEDAREACURSOR_HPP_
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#endif

class ScCellCursorObj : public ScCellRangeObj,
                        public com::sun::star::sheet::XSheetCellCursor,
                        public com::sun::star::sheet::XUsedAreaCursor,
                        public com::sun::star::table::XCellCursor
{
public:
                            ScCellCursorObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellCursorObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw();
    virtual void SAL_CALL   release() throw();

                            // XSheetCellCursor
    virtual void SAL_CALL   collapseToCurrentRegion() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   collapseToCurrentArray() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   collapseToMergedArea() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   expandToEntireColumns() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   expandToEntireRows() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   collapseToSize( sal_Int32 nColumns, sal_Int32 nRows )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XUsedAreaCursor
    virtual void SAL_CALL   gotoStartOfUsedArea( sal_Bool bExpand )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   gotoEndOfUsedArea( sal_Bool bExpand )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellCursor
    virtual void SAL_CALL   gotoStart() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   gotoEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   gotoNext() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   gotoPrevious() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   gotoOffset( sal_Int32 nColumnOffset, sal_Int32 nRowOffset )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSheetCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(::com::sun::star::uno::RuntimeException);

                            // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::uno::RuntimeException);
    using ScCellRangeObj::getCellRangeByName;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByName( const ::rtl::OUString& aRange )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


#endif

