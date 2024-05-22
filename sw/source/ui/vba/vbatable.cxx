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

#include "vbatable.hxx"
#include "vbarange.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <utility>
#include "vbaborders.hxx"
#include "vbapalette.hxx"
#include "vbarows.hxx"
#include "vbacolumns.hxx"
#include "vbaapplication.hxx"

#include <tools/UnitConversion.hxx>

#include <sal/log.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaTable::SwVbaTable(  const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< text::XTextDocument > xDocument, const  uno::Reference< text::XTextTable >& xTextTable) : SwVbaTable_BASE( rParent, rContext ), mxTextDocument(std::move( xDocument ))
{
    mxTextTable.set( xTextTable, uno::UNO_SET_THROW );
}

uno::Reference< word::XRange > SAL_CALL
SwVbaTable::Range(  )
{
    return new SwVbaRange( mxParent, mxContext, mxTextDocument, mxTextTable->getAnchor() );
}

void SAL_CALL
SwVbaTable::Select(  )
{
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< frame::XController > xController = xModel->getCurrentController();

    uno::Reference< text::XTextViewCursorSupplier > xViewCursorSupplier( xController, uno::UNO_QUERY_THROW );
    uno::Reference< view::XSelectionSupplier > xSelectionSupplier( xController, uno::UNO_QUERY_THROW );

    // set the view cursor to the start of the table.
    xSelectionSupplier->select( uno::Any( mxTextTable ) );

    // go to the end of the table and span the view
    uno::Reference< text::XTextViewCursor > xCursor = xViewCursorSupplier->getViewCursor();
    xCursor->gotoEnd(true);

}

void SAL_CALL
SwVbaTable::Delete(  )
{
    uno::Reference< table::XTableRows > xRows( mxTextTable->getRows() );
    xRows->removeByIndex( 0, xRows->getCount() );
}

OUString SAL_CALL
SwVbaTable::getName(  )
{
    uno::Reference< container::XNamed > xNamed( mxTextTable, uno::UNO_QUERY_THROW );
    return xNamed->getName();
}

uno::Any SAL_CALL
SwVbaTable::Borders( const uno::Any& index )
{
    uno::Reference< table::XCellRange > aCellRange( mxTextTable, uno::UNO_QUERY_THROW );
    VbaPalette aPalette;
    uno::Reference< XCollection > xCol( new SwVbaBorders( this, mxContext, aCellRange, aPalette ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

double SAL_CALL
SwVbaTable::getBottomPadding()
{
    uno::Reference< beans::XPropertySet > xPropertySet( mxTextTable, uno::UNO_QUERY_THROW);
    table::TableBorderDistances aTableBorderDistances;
    xPropertySet->getPropertyValue(u"TableBorderDistances"_ustr) >>= aTableBorderDistances;
    return convertMm100ToPoint(aTableBorderDistances.BottomDistance);
}

void SAL_CALL
SwVbaTable::setBottomPadding( double fValue )
{
    uno::Reference< beans::XPropertySet > xPropertySet( mxTextTable, uno::UNO_QUERY_THROW);
    table::TableBorderDistances aTableBorderDistances;
    aTableBorderDistances.IsBottomDistanceValid = true;
    aTableBorderDistances.BottomDistance = convertPointToMm100(fValue);
    xPropertySet->setPropertyValue( u"TableBorderDistances"_ustr, uno::Any( aTableBorderDistances ) );
}

double SAL_CALL
SwVbaTable::getLeftPadding()
{
    uno::Reference< beans::XPropertySet > xPropertySet( mxTextTable, uno::UNO_QUERY_THROW);
    table::TableBorderDistances aTableBorderDistances;
    xPropertySet->getPropertyValue(u"TableBorderDistances"_ustr) >>= aTableBorderDistances;
    return convertMm100ToPoint(aTableBorderDistances.LeftDistance);
}

void SAL_CALL
SwVbaTable::setLeftPadding( double fValue )
{
    uno::Reference< beans::XPropertySet > xPropertySet( mxTextTable, uno::UNO_QUERY_THROW);
    table::TableBorderDistances aTableBorderDistances;
    aTableBorderDistances.IsLeftDistanceValid = true;
    aTableBorderDistances.LeftDistance = convertPointToMm100(fValue);
    xPropertySet->setPropertyValue( u"TableBorderDistances"_ustr, uno::Any( aTableBorderDistances ) );
}

double SAL_CALL
SwVbaTable::getRightPadding()
{
    uno::Reference< beans::XPropertySet > xPropertySet( mxTextTable, uno::UNO_QUERY_THROW);
    table::TableBorderDistances aTableBorderDistances;
    xPropertySet->getPropertyValue(u"TableBorderDistances"_ustr) >>= aTableBorderDistances;
    return convertMm100ToPoint(aTableBorderDistances.RightDistance);
}

void SAL_CALL
SwVbaTable::setRightPadding( double fValue )
{
    uno::Reference< beans::XPropertySet > xPropertySet( mxTextTable, uno::UNO_QUERY_THROW);
    table::TableBorderDistances aTableBorderDistances;
    aTableBorderDistances.IsRightDistanceValid = true;
    aTableBorderDistances.RightDistance = convertPointToMm100(fValue);
    xPropertySet->setPropertyValue( u"TableBorderDistances"_ustr, uno::Any( aTableBorderDistances ) );
}

double SAL_CALL
SwVbaTable::getTopPadding()
{
    uno::Reference< beans::XPropertySet > xPropertySet( mxTextTable, uno::UNO_QUERY_THROW);
    table::TableBorderDistances aTableBorderDistances;
    xPropertySet->getPropertyValue(u"TableBorderDistances"_ustr) >>= aTableBorderDistances;
    return convertMm100ToPoint(aTableBorderDistances.TopDistance);
}

void SAL_CALL
SwVbaTable::setTopPadding( double fValue )
{
    uno::Reference< beans::XPropertySet > xPropertySet( mxTextTable, uno::UNO_QUERY_THROW);
    table::TableBorderDistances aTableBorderDistances;
    aTableBorderDistances.IsTopDistanceValid = true;
    aTableBorderDistances.TopDistance = convertPointToMm100(fValue);
    xPropertySet->setPropertyValue( u"TableBorderDistances"_ustr, uno::Any( aTableBorderDistances ) );
}

uno::Any SAL_CALL
SwVbaTable::Rows( const uno::Any& index )
{
    uno::Reference< table::XTableRows > xTableRows( mxTextTable->getRows(), uno::UNO_SET_THROW );
    uno::Reference< XCollection > xCol( new SwVbaRows( this, mxContext, mxTextTable, xTableRows ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaTable::Columns( const uno::Any& index )
{
    uno::Reference< table::XTableColumns > xTableColumns( mxTextTable->getColumns(), uno::UNO_SET_THROW );
    uno::Reference< XCollection > xCol( new SwVbaColumns( this, mxContext, mxTextTable, xTableColumns ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

// XHelperInterface
OUString
SwVbaTable::getServiceImplName()
{
    return u"SwVbaTable"_ustr;
}

uno::Sequence<OUString>
SwVbaTable::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Table"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
