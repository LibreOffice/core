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

uno::Any SAL_CALL SwVbaRow::getHeight() throw (css::uno::RuntimeException, std::exception)
{
    if( getHeightRule() == word::WdRowHeightRule::wdRowHeightAuto )
        return uno::makeAny( sal_Int32( word::WdConstants::wdUndefined ) );

    sal_Int32 nHeight = 0;
    mxRowProps->getPropertyValue("Height") >>= nHeight;
    return uno::makeAny( (float)Millimeter::getInPoints( nHeight ) );
}

void SAL_CALL SwVbaRow::setHeight( const uno::Any& _height ) throw (css::uno::RuntimeException, std::exception)
{
    float height = 0;
    _height >>= height;

    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( height );
    mxRowProps->setPropertyValue("Height", uno::makeAny( nHeight ) );
}

::sal_Int32 SAL_CALL SwVbaRow::getHeightRule() throw (css::uno::RuntimeException, std::exception)
{
    sal_Bool isAutoHeight = sal_False;
    mxRowProps->getPropertyValue("IsAutoHeight") >>= isAutoHeight;
    return isAutoHeight ? word::WdRowHeightRule::wdRowHeightAuto : word::WdRowHeightRule::wdRowHeightExactly;
}

void SAL_CALL SwVbaRow::setHeightRule( ::sal_Int32 _heightrule ) throw (css::uno::RuntimeException, std::exception)
{
    sal_Bool isAutoHeight = ( _heightrule == word::WdRowHeightRule::wdRowHeightAuto );
    mxRowProps->setPropertyValue("IsAutoHeight", uno::makeAny( isAutoHeight ) );
}

void SAL_CALL
SwVbaRow::Select( ) throw ( uno::RuntimeException, std::exception )
{
    SelectRow( getCurrentWordDoc(mxContext), mxTextTable, mnIndex, mnIndex );
}

void SwVbaRow::SelectRow( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nStartRow, sal_Int32 nEndRow ) throw ( uno::RuntimeException )
{
    OUStringBuffer aRangeName;
    aRangeName.append('A').append(sal_Int32( nStartRow + 1 ) );
    SwVbaTableHelper aTableHelper( xTextTable );
    sal_Int32 nColCount = aTableHelper.getTabColumnsCount( nEndRow );
    // FIXME: the column count > 26
    //sal_Char cCol = 'A' + nColCount - 1;
    OUString sCol = aTableHelper.getColumnStr( nColCount - 1);
    aRangeName.append(':').append( sCol ).append( sal_Int32( nEndRow + 1 ) );

    uno::Reference< table::XCellRange > xCellRange( xTextTable, uno::UNO_QUERY_THROW );
    OUString sSelRange = aRangeName.makeStringAndClear();
    uno::Reference< table::XCellRange > xSelRange = xCellRange->getCellRangeByName( sSelRange );

    uno::Reference< view::XSelectionSupplier > xSelection( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelection->select( uno::makeAny( xSelRange ) );
}

void SAL_CALL SwVbaRow::SetHeight( float height, sal_Int32 heightrule ) throw (css::uno::RuntimeException, std::exception)
{
    setHeightRule( heightrule );
    setHeight( uno::makeAny( height ) );
}

OUString
SwVbaRow::getServiceImplName()
{
    return OUString("SwVbaRow");
}

uno::Sequence< OUString >
SwVbaRow::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Row";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
