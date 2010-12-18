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
#include "vbarow.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <ooo/vba/word/WdRowHeightRule.hpp>
#include <ooo/vba/word/WdConstants.hpp>
#include <rtl/ustrbuf.hxx>
#include "wordvbahelper.hxx"
#include "vbatablehelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaRow::SwVbaRow( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nIndex ) throw ( uno::RuntimeException ) :
    SwVbaRow_BASE( rParent, rContext ), mxTextTable( xTextTable ), mnIndex( nIndex )
{
    mxTableRows = mxTextTable->getRows();
    mxRowProps.set( mxTableRows->getByIndex( mnIndex ), uno::UNO_QUERY_THROW );
}

SwVbaRow::~SwVbaRow()
{
}

uno::Any SAL_CALL SwVbaRow::getHeight() throw (css::uno::RuntimeException)
{
    if( getHeightRule() == word::WdRowHeightRule::wdRowHeightAuto )
        return uno::makeAny( sal_Int32( word::WdConstants::wdUndefined ) );

    sal_Int32 nHeight = 0;
    mxRowProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ) ) >>= nHeight;
    return uno::makeAny( (float)Millimeter::getInPoints( nHeight ) );
}

void SAL_CALL SwVbaRow::setHeight( const uno::Any& _height ) throw (css::uno::RuntimeException)
{
    float height = 0;
    _height >>= height;

    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( height );
    mxRowProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ), uno::makeAny( nHeight ) );
}

::sal_Int32 SAL_CALL SwVbaRow::getHeightRule() throw (css::uno::RuntimeException)
{
    sal_Bool isAutoHeight = sal_False;
    mxRowProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsAutoHeight") ) ) >>= isAutoHeight;
    return isAutoHeight ? word::WdRowHeightRule::wdRowHeightAuto : word::WdRowHeightRule::wdRowHeightExactly;
}

void SAL_CALL SwVbaRow::setHeightRule( ::sal_Int32 _heightrule ) throw (css::uno::RuntimeException)
{
    sal_Bool isAutoHeight = ( _heightrule == word::WdRowHeightRule::wdRowHeightAuto );
    mxRowProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsAutoHeight") ), uno::makeAny( isAutoHeight ) );
}

void SAL_CALL
SwVbaRow::Select( ) throw ( uno::RuntimeException )
{
    SelectRow( getCurrentWordDoc(mxContext), mxTextTable, mnIndex, mnIndex );
}

void SwVbaRow::SelectRow( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nStartRow, sal_Int32 nEndRow ) throw ( uno::RuntimeException )
{
    rtl::OUStringBuffer aRangeName;
    aRangeName.appendAscii("A").append(sal_Int32( nStartRow + 1 ) );
    SwVbaTableHelper aTableHelper( xTextTable );
    sal_Int32 nColCount = aTableHelper.getTabColumnsCount( nEndRow );
    // FIXME: the column count > 26
    //sal_Char cCol = 'A' + nColCount - 1;
    rtl::OUString sCol = aTableHelper.getColumnStr( nColCount - 1);
    aRangeName.appendAscii(":").append( sCol ).append( sal_Int32( nEndRow + 1 ) );

    uno::Reference< table::XCellRange > xCellRange( xTextTable, uno::UNO_QUERY_THROW );
    rtl::OUString sSelRange = aRangeName.makeStringAndClear();
    uno::Reference< table::XCellRange > xSelRange = xCellRange->getCellRangeByName( sSelRange );

    uno::Reference< view::XSelectionSupplier > xSelection( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelection->select( uno::makeAny( xSelRange ) );
}

void SAL_CALL SwVbaRow::SetHeight( float height, sal_Int32 heightrule ) throw (css::uno::RuntimeException)
{
    setHeightRule( heightrule );
    setHeight( uno::makeAny( height ) );
}

rtl::OUString&
SwVbaRow::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaRow") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaRow::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Row" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
