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

#ifndef INCLUDED_SVX_SOURCE_TABLE_CELLCURSOR_HXX
#define INCLUDED_SVX_SOURCE_TABLE_CELLCURSOR_HXX

#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/XCellCursor.hpp>
#include <cppuhelper/implbase.hxx>
#include "cellrange.hxx"


namespace sdr { namespace table {

struct CellPos;

typedef ::cppu::ImplInheritanceHelper< CellRange, css::table::XCellCursor, css::table::XMergeableCellRange > CellCursorBase;

class CellCursor : public CellCursorBase
{
public:
    CellCursor( const TableModelRef& xTableModel, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom );
    virtual ~CellCursor() override;

    // XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange ) override;

    // XCellCursor
    virtual void SAL_CALL gotoStart(  ) override;
    virtual void SAL_CALL gotoEnd(  ) override;
    virtual void SAL_CALL gotoNext(  ) override;
    virtual void SAL_CALL gotoPrevious(  ) override;
    virtual void SAL_CALL gotoOffset( ::sal_Int32 nColumnOffset, ::sal_Int32 nRowOffset ) override;

    // XMergeableCellRange
    virtual void SAL_CALL merge(  ) override;
    virtual void SAL_CALL split( ::sal_Int32 Columns, ::sal_Int32 Rows ) override;
    virtual sal_Bool SAL_CALL isMergeable(  ) override;

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
