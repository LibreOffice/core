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

#include "vbapane.hxx"
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "vbarange.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaPane::ScVbaPane(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< sheet::XViewPane > xViewPane ) throw (uno::RuntimeException) :
    ScVbaPane_BASE( xParent, xContext ),
    m_xModel( xModel, uno::UNO_SET_THROW ),
    m_xViewPane( xViewPane, uno::UNO_SET_THROW )
{
}

sal_Int32 SAL_CALL
ScVbaPane::getScrollColumn() throw (uno::RuntimeException)
{
    return ( m_xViewPane->getFirstVisibleColumn() + 1 );
}

void SAL_CALL
ScVbaPane::setScrollColumn( sal_Int32 _scrollcolumn ) throw (uno::RuntimeException)
{
    if( _scrollcolumn < 1 )
    {
        throw uno::RuntimeException( OUString( "Column number should not less than 1" ),
                uno::Reference< uno::XInterface >() );
    }
    m_xViewPane->setFirstVisibleColumn( _scrollcolumn - 1 );
}

sal_Int32 SAL_CALL
ScVbaPane::getScrollRow() throw (uno::RuntimeException)
{
    return ( m_xViewPane->getFirstVisibleRow() + 1 );
}

void SAL_CALL
ScVbaPane::setScrollRow( sal_Int32 _scrollrow ) throw (uno::RuntimeException)
{
    if( _scrollrow < 1 )
    {
        throw uno::RuntimeException( OUString( "Row number should not less than 1" ),
                uno::Reference< uno::XInterface >() );
    }
    m_xViewPane->setFirstVisibleRow( _scrollrow - 1 );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaPane::getVisibleRange() throw (uno::RuntimeException)
{
    // TODO: Excel includes partly visible rows/columns, Calc does not
    table::CellRangeAddress aRangeAddr = m_xViewPane->getVisibleRange();
    uno::Reference< sheet::XSpreadsheetDocument > xDoc( m_xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xSheetsIA( xDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetsIA->getByIndex( aRangeAddr.Sheet ), uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xRange( xSheet->getCellRangeByPosition( aRangeAddr.StartColumn, aRangeAddr.StartRow, aRangeAddr.EndColumn, aRangeAddr.EndRow ), uno::UNO_SET_THROW );
    // TODO: getParent() returns the window, Range needs the worksheet
    return new ScVbaRange( getParent(), mxContext, xRange );
}

//Method
void SAL_CALL
ScVbaPane::SmallScroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft ) throw (uno::RuntimeException)
{
    OUString messageBuffer;
    sal_Int32 downRows = 0;
    sal_Int32 rightCols = 0;
    table::CellRangeAddress visibleRange = m_xViewPane->getVisibleRange();

    if( Down.hasValue() )
    {
        sal_Int32 down = 0;
        if( Down >>= down )
            downRows += down;
        else
            messageBuffer += OUString( "Error getting parameter: Down\n" );
    }
    if( Up.hasValue() )
    {
        sal_Int32 up = 0;
        if( Up >>= up )
            downRows -= up;
        else
            messageBuffer += OUString( "Error getting parameter: Up\n" );
    }
    if( ToRight.hasValue() )
    {
        sal_Int32 right = 0;
        if( ToRight >>= right )
            rightCols += right;
        else
            messageBuffer += OUString( "Error getting parameter: ToRight\n" );
    }
    if( ToLeft.hasValue() )
    {
        sal_Int32 left = 0;
        if( ToLeft >>= left )
            rightCols -= left;
        else
            messageBuffer += OUString( "Error getting parameter: ToLeft\n" );
    }
    if( !messageBuffer.isEmpty() )
        throw(uno::RuntimeException( messageBuffer, uno::Reference< uno::XInterface >() ) );

    sal_Int32 newStartRow = visibleRange.StartRow + downRows;
    if( newStartRow < 0 )
        newStartRow = 0;
    sal_Int32 newStartCol = visibleRange.StartColumn + rightCols;
    if( newStartCol < 0 )
        newStartCol = 0;
    m_xViewPane->setFirstVisibleRow( newStartRow );
    m_xViewPane->setFirstVisibleColumn( newStartCol );
}

void SAL_CALL
ScVbaPane::LargeScroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft ) throw (uno::RuntimeException)
{
    OUString messageBuffer;
    table::CellRangeAddress visibleRange = m_xViewPane->getVisibleRange();

    sal_Int32 vertPageSize = 1 + visibleRange.EndRow - visibleRange.StartRow;
    sal_Int32 horizPageSize = 1 + visibleRange.EndColumn - visibleRange.StartColumn;
    sal_Int32 downPages = 0;
    sal_Int32 acrossPages = 0;
    if( Down.hasValue() )
    {
        sal_Int32 down = 0;
        if( Down >>= down )
            downPages += down;
        else
            messageBuffer += OUString( "Error getting parameter: Down\n" );
    }
    if( Up.hasValue() )
    {
        sal_Int32 up = 0;
        if( Up >>= up )
            downPages -= up;
        else
            messageBuffer += OUString( "Error getting parameter: Up\n" );
    }
    if( ToRight.hasValue() )
    {
        sal_Int32 right = 0;
        if( ToRight >>= right )
            acrossPages += right;
        else
            messageBuffer += OUString( "Error getting parameter: ToRight\n" );
    }
    if( ToLeft.hasValue() )
    {
        sal_Int32 left = 0;
        if( ToLeft >>= left )
            acrossPages -= left;
        else
            messageBuffer += OUString( "Error getting parameter: ToLeft\n" );
    }
    if( !messageBuffer.isEmpty() )
        throw(uno::RuntimeException( messageBuffer, uno::Reference< uno::XInterface >() ) );

    sal_Int32 newStartRow = visibleRange.StartRow + (downPages * vertPageSize );
    if( newStartRow < 0 )
        newStartRow = 0;
    sal_Int32 newStartCol = visibleRange.StartColumn + (acrossPages * horizPageSize );
    if( newStartCol < 0 )
        newStartCol = 0;
    m_xViewPane->setFirstVisibleRow( newStartRow );
    m_xViewPane->setFirstVisibleColumn( newStartCol );
}

// XHelperInterface

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaPane, "ooo.vba.excel.Pane" )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
