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

#ifndef INCLUDED_SVX_SOURCE_TABLE_TABLEROW_HXX
#define INCLUDED_SVX_SOURCE_TABLE_TABLEROW_HXX

#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/implbase.hxx>

#include "propertyset.hxx"
#include <celltypes.hxx>


namespace sdr::table {

typedef ::cppu::ImplInheritanceHelper< FastPropertySet, css::table::XCellRange, css::container::XNamed > TableRowBase;

class TableRow : public TableRowBase
{
    friend class TableModel;
    friend class TableRowUndo;
public:
    TableRow( TableModelRef xTableModel, sal_Int32 nRow, sal_Int32 nColumns );
    virtual ~TableRow() override;

    void dispose();
    /// @throws css::uno::RuntimeException
    void throwIfDisposed() const;

    TableRow& operator=( const TableRow& );

    void insertColumns( sal_Int32 nIndex, sal_Int32 nCount, CellVector::iterator const * pIter );
    void removeColumns( sal_Int32 nIndex, sal_Int32 nCount );
    /// Reference to the table model containing this row.
    const TableModelRef& getModel() const;

    // XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange ) override;

    // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue( ::sal_Int32 nHandle, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) override;

private:
    static rtl::Reference< FastPropertySetInfo > getStaticPropertySetInfo();

    TableModelRef mxTableModel;
    CellVector  maCells;
    sal_Int32   mnRow;
    sal_Int32   mnHeight;
    bool    mbOptimalHeight;
    bool    mbIsVisible;
    bool    mbIsStartOfNewPage;
    OUString maName;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
