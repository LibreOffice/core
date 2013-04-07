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
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include "vbaborders.hxx"
#include "vbapalette.hxx"
#include "vbarows.hxx"
#include "vbacolumns.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaTable::SwVbaTable(  const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextDocument >& rDocument, const  uno::Reference< text::XTextTable >& xTextTable) throw ( uno::RuntimeException ) : SwVbaTable_BASE( rParent, rContext ), mxTextDocument( rDocument )
{
    mxTextTable.set( xTextTable, uno::UNO_QUERY_THROW );
}

uno::Reference< word::XRange > SAL_CALL
SwVbaTable::Range(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return new SwVbaRange( mxParent, mxContext, mxTextDocument, mxTextTable->getAnchor() );
}

void SAL_CALL
SwVbaTable::Select(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< frame::XController > xController = xModel->getCurrentController();

    uno::Reference< text::XTextViewCursorSupplier > xViewCursorSupplier( xController, uno::UNO_QUERY_THROW );
    uno::Reference< view::XSelectionSupplier > xSelectionSupplier( xController, uno::UNO_QUERY_THROW );

    // set the view cursor to the start of the table.
    xSelectionSupplier->select( uno::makeAny( mxTextTable ) );

    // go to the end of the table and span the view
    uno::Reference< text::XTextViewCursor > xCursor = xViewCursorSupplier->getViewCursor();
    xCursor->gotoEnd(sal_True);

}

void SAL_CALL
SwVbaTable::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< table::XTableRows > xRows( mxTextTable->getRows() );
    xRows->removeByIndex( 0, xRows->getCount() );
}

uno::Reference< word::XRange > SAL_CALL
SwVbaTable::ConvertToText( const uno::Any& /*Separator*/, const uno::Any& /*NestedTables*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // #FIXME the helper api uses the dreaded dispatch mechanism, holding off
    // implementation while I look for alternative solution
    throw uno::RuntimeException();
}

OUString SAL_CALL
SwVbaTable::getName() throw (uno::RuntimeException)
{
    uno::Reference< container::XNamed > xNamed( mxTextTable, uno::UNO_QUERY_THROW );
    return xNamed->getName();
}

uno::Any SAL_CALL
SwVbaTable::Borders( const uno::Any& index ) throw (uno::RuntimeException)
{
    uno::Reference< table::XCellRange > aCellRange( mxTextTable, uno::UNO_QUERY_THROW );
    VbaPalette aPalette;
    uno::Reference< XCollection > xCol( new SwVbaBorders( this, mxContext, aCellRange, aPalette ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Any SAL_CALL
SwVbaTable::Rows( const uno::Any& index ) throw (uno::RuntimeException)
{
    uno::Reference< table::XTableRows > xTableRows( mxTextTable->getRows(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaRows( this, mxContext, mxTextTable, xTableRows ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Any SAL_CALL
SwVbaTable::Columns( const uno::Any& index ) throw (uno::RuntimeException)
{
    uno::Reference< table::XTableColumns > xTableColumns( mxTextTable->getColumns(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaColumns( this, mxContext, mxTextTable, xTableColumns ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

// XHelperInterface
OUString
SwVbaTable::getServiceImplName()
{
    return OUString("SwVbaTable");
}

uno::Sequence<OUString>
SwVbaTable::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.word.Table" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
