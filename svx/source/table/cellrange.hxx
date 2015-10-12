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

#ifndef INCLUDED_SVX_SOURCE_TABLE_CELLRANGE_HXX
#define INCLUDED_SVX_SOURCE_TABLE_CELLRANGE_HXX

#include <com/sun/star/table/XCellRange.hpp>
#include <cppuhelper/implbase1.hxx>

#include "tablemodel.hxx"



namespace sdr { namespace table {

class CellRange : public ::cppu::WeakAggImplHelper1< ::com::sun::star::table::XCellRange >, public ICellRange
{
public:
    CellRange( const TableModelRef & xTable, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom );
    virtual ~CellRange();

    // ICellRange
    virtual sal_Int32 getLeft() override;
    virtual sal_Int32 getTop() override;
    virtual sal_Int32 getRight() override;
    virtual sal_Int32 getBottom() override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTable > getTable() override;

    // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    TableModelRef mxTable;
    sal_Int32 mnLeft;
    sal_Int32 mnTop;
    sal_Int32 mnRight;
    sal_Int32 mnBottom;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
