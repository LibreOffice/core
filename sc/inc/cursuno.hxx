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

#ifndef INCLUDED_SC_INC_CURSUNO_HXX
#define INCLUDED_SC_INC_CURSUNO_HXX

#include "cellsuno.hxx"
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>

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
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

                            // XSheetCellCursor
    virtual void SAL_CALL   collapseToCurrentRegion() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   collapseToCurrentArray()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   collapseToMergedArea() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   expandToEntireColumns() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   expandToEntireRows() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   collapseToSize( sal_Int32 nColumns, sal_Int32 nRows )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XUsedAreaCursor
    virtual void SAL_CALL   gotoStartOfUsedArea( sal_Bool bExpand )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   gotoEndOfUsedArea( sal_Bool bExpand )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XCellCursor
    virtual void SAL_CALL   gotoStart() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   gotoEnd() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   gotoNext() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   gotoPrevious() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   gotoOffset( sal_Int32 nColumnOffset, sal_Int32 nRowOffset )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XSheetCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) override;
    using ScCellRangeObj::getCellRangeByName;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByName( const OUString& aRange )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
