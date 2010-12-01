/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbacolumn.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbatable.hxx"
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <rtl/ustrbuf.hxx>
#include "wordvbahelper.hxx"
#include "vbatablehelper.hxx"

#define RELATIVE_TABLE_WIDTH 10000

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaColumn::SwVbaColumn( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nIndex ) throw ( uno::RuntimeException ) :
    SwVbaColumn_BASE( rParent, rContext ), mxTextTable( xTextTable ), mnIndex( nIndex )
{
    mxTableColumns = mxTextTable->getColumns();
}

SwVbaColumn::~SwVbaColumn()
{
}

void SwVbaColumn::calculateAbsoluteColumnWidth( sal_Int32 nTableWidth, const css::uno::Sequence< css::text::TableColumnSeparator >& aSeparators, double* pAbsWidth )
{
    const text::TableColumnSeparator* pArray = aSeparators.getConstArray();
    sal_Int32 nSepCount = aSeparators.getLength();
    for( sal_Int32 i = 0; i <= nSepCount; i++ )
    {
        sal_Int32 nRelColWidth = 0;
        if( i == 0 )
        {
            if( nSepCount != 0 )
            {
                nRelColWidth = pArray[0].Position;
            }
            else
            {
                nRelColWidth = RELATIVE_TABLE_WIDTH;
            }
        }
        else
        {
            if( i == nSepCount )
            {
                nRelColWidth = RELATIVE_TABLE_WIDTH - pArray[i-1].Position;
            }
            else
            {
                nRelColWidth = pArray[i].Position - pArray[i-1].Position;
            }
        }
        pAbsWidth[i] = ( (double)nRelColWidth / RELATIVE_TABLE_WIDTH ) * (double) nTableWidth;
    }
}

void SwVbaColumn::calculateRelativeColumnWidth( const double* pAbsWidth, double* pRelWidth, sal_Int32 nCount )
{
    double tableWidth = 0.0;
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        tableWidth += pAbsWidth[i];
    }

    pRelWidth[ nCount - 1 ] = tableWidth;
    for( sal_Int32 i = 0; i < nCount - 1; i++ )
    {
        if( i == 0 )
        {
            pRelWidth[i] = ( pAbsWidth[i] * RELATIVE_TABLE_WIDTH ) / tableWidth;
        }
        else
        {
            pRelWidth[i] = pRelWidth[i-1] + ( pAbsWidth[i] * RELATIVE_TABLE_WIDTH ) / tableWidth;
        }
    }
}

sal_Int32 SAL_CALL
SwVbaColumn::getWidth( ) throw ( css::uno::RuntimeException )
{
   /* uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    uno::Sequence< text::TableColumnSeparator > aSeparators;
    xTableProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TableColumnSeparators") ) ) >>= aSeparators;
    sal_Int32 nTableWidth = SwVbaTable::getTableWidth( xTableProps );
    sal_Int32 nColCont = aSeparators.getLength() + 1;
    double dAbsColWidth[ nColCont ];
    calculateAbsoluteColumnWidth( nTableWidth, aSeparators, dAbsColWidth );
    return Millimeter::getInPoints( (sal_Int32)( dAbsColWidth[ mnIndex ] ) );
   */
    SwVbaTableHelper aTableHelper( mxTextTable );
    return aTableHelper.GetColWidth( mnIndex );
}

void SAL_CALL
SwVbaColumn::setWidth( sal_Int32 _width ) throw ( css::uno::RuntimeException )
{
/*    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    uno::Sequence< text::TableColumnSeparator > aSeparators;
    xTableProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TableColumnSeparators") ) ) >>= aSeparators;
    sal_Int32 nTableWidth = SwVbaTable::getTableWidth( xTableProps );
    sal_Int32 nColCont = aSeparators.getLength() + 1;
    double dAbsColWidth[ nColCont ];
    calculateAbsoluteColumnWidth( nTableWidth, aSeparators, dAbsColWidth );
    dAbsColWidth[ mnIndex ] = Millimeter::getInHundredthsOfOneMillimeter( _width );
    double tableWidth = 0.0;
    for( sal_Int32 i = 0; i < nColCont; i++ )
    {
        tableWidth += dAbsColWidth[i];
    }

    double dRelColWidth[ nColCont ];
    calculateRelativeColumnWidth( dAbsColWidth, dRelColWidth, nColCont);

    text::TableColumnSeparator* pArray = aSeparators.getArray();
    for( sal_Int32 i = 0; i < nColCont - 1; i++ )
    {
        pArray[i].Position = (sal_Int16)(dRelColWidth[i]);
    }

    xTableProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TableColumnSeparators") ), uno::makeAny( aSeparators ) );
    SwVbaTable::setTableWidth( xTableProps, (sal_Int32)( tableWidth ) );
    */

    SwVbaTableHelper aTableHelper( mxTextTable );
    aTableHelper.SetColWidth( _width, mnIndex );
}

void SAL_CALL
SwVbaColumn::Select( ) throw ( uno::RuntimeException )
{
    SelectColumn( getCurrentWordDoc(mxContext), mxTextTable, mnIndex, mnIndex );
}

void SwVbaColumn::SelectColumn( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nStartColumn, sal_Int32 nEndColumn ) throw ( uno::RuntimeException )
{
    rtl::OUStringBuffer aRangeName;
    rtl::OUString sStartCol = SwVbaTableHelper::getColumnStr( nStartColumn );
    aRangeName.append(sStartCol).append(sal_Int32( 1 ) );
    rtl::OUString sEndCol = SwVbaTableHelper::getColumnStr( nEndColumn );
    sal_Int32 nRowCount = xTextTable->getRows()->getCount();
    aRangeName.appendAscii(":").append( sEndCol ).append( sal_Int32( nRowCount ) );

    uno::Reference< table::XCellRange > xCellRange( xTextTable, uno::UNO_QUERY_THROW );
    rtl::OUString sSelRange = aRangeName.makeStringAndClear();
    uno::Reference< table::XCellRange > xSelRange = xCellRange->getCellRangeByName( sSelRange );

    uno::Reference< view::XSelectionSupplier > xSelection( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelection->select( uno::makeAny( xSelRange ) );
}

rtl::OUString&
SwVbaColumn::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaColumn") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaColumn::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Column" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
