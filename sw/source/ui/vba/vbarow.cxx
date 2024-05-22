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
#include <utility>
#include <vbahelper/vbahelper.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <ooo/vba/word/WdRowHeightRule.hpp>
#include <ooo/vba/word/WdConstants.hpp>
#include "vbatablehelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaRow::SwVbaRow( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,uno::Reference< text::XTextTable >  xTextTable, sal_Int32 nIndex ) :
    SwVbaRow_BASE( rParent, rContext ), mxTextTable(std::move( xTextTable )), mnIndex( nIndex )
{
    mxTableRows = mxTextTable->getRows();
    mxRowProps.set( mxTableRows->getByIndex( mnIndex ), uno::UNO_QUERY_THROW );
}

SwVbaRow::~SwVbaRow()
{
}

uno::Any SAL_CALL SwVbaRow::getHeight()
{
    if( getHeightRule() == word::WdRowHeightRule::wdRowHeightAuto )
        return uno::Any( sal_Int32( word::WdConstants::wdUndefined ) );

    sal_Int32 nHeight = 0;
    mxRowProps->getPropertyValue(u"Height"_ustr) >>= nHeight;
    return uno::Any( static_cast<float>(Millimeter::getInPoints( nHeight )) );
}

void SAL_CALL SwVbaRow::setHeight( const uno::Any& _height )
{
    float height = 0;
    _height >>= height;

    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( height );
    mxRowProps->setPropertyValue(u"Height"_ustr, uno::Any( nHeight ) );
}

::sal_Int32 SAL_CALL SwVbaRow::getHeightRule()
{
    bool isAutoHeight = false;
    mxRowProps->getPropertyValue(u"IsAutoHeight"_ustr) >>= isAutoHeight;
    return isAutoHeight ? word::WdRowHeightRule::wdRowHeightAuto : word::WdRowHeightRule::wdRowHeightExactly;
}

void SAL_CALL SwVbaRow::setHeightRule( ::sal_Int32 _heightrule )
{
    bool isAutoHeight = ( _heightrule == word::WdRowHeightRule::wdRowHeightAuto );
    mxRowProps->setPropertyValue(u"IsAutoHeight"_ustr, uno::Any( isAutoHeight ) );
}

void SAL_CALL
SwVbaRow::Select( )
{
    SelectRow( getCurrentWordDoc(mxContext), mxTextTable, mnIndex, mnIndex );
}

void SwVbaRow::SelectRow( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nStartRow, sal_Int32 nEndRow )
{
    OUString sRangeName = "A" + OUString::number(nStartRow + 1);
    SwVbaTableHelper aTableHelper( xTextTable );
    sal_Int32 nColCount = aTableHelper.getTabColumnsCount( nEndRow );
    // FIXME: the column count > 26
    //char cCol = 'A' + nColCount - 1;
    OUString sCol = SwVbaTableHelper::getColumnStr( nColCount - 1);
    sRangeName += ":" + sCol + OUString::number(nEndRow + 1);

    uno::Reference< table::XCellRange > xCellRange( xTextTable, uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xSelRange = xCellRange->getCellRangeByName( sRangeName );

    uno::Reference< view::XSelectionSupplier > xSelection( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelection->select( uno::Any( xSelRange ) );
}

void SAL_CALL SwVbaRow::SetHeight( float height, sal_Int32 heightrule )
{
    setHeightRule( heightrule );
    setHeight( uno::Any( height ) );
}

OUString
SwVbaRow::getServiceImplName()
{
    return u"SwVbaRow"_ustr;
}

uno::Sequence< OUString >
SwVbaRow::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Row"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
