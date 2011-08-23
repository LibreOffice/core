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

#ifndef SC_CURSUNO_HXX
#define SC_CURSUNO_HXX

#include "cellsuno.hxx"

#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
namespace binfilter {

class ScCellCursorObj : public ScCellRangeObj,
                        public ::com::sun::star::sheet::XSheetCellCursor,
                        public ::com::sun::star::sheet::XUsedAreaCursor,
                        public ::com::sun::star::table::XCellCursor
{
public:
                            ScCellCursorObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual					~ScCellCursorObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

                            // XSheetCellCursor
    virtual void SAL_CALL	collapseToCurrentRegion() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	collapseToCurrentArray() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	collapseToMergedArea() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	expandToEntireColumns() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	expandToEntireRows() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	collapseToSize( sal_Int32 nColumns, sal_Int32 nRows )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XUsedAreaCursor
    virtual void SAL_CALL	gotoStartOfUsedArea( sal_Bool bExpand )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	gotoEndOfUsedArea( sal_Bool bExpand )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellCursor
    virtual void SAL_CALL	gotoStart() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	gotoEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	gotoNext() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	gotoPrevious() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	gotoOffset( sal_Int32 nColumnOffset, sal_Int32 nRowOffset )
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


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
