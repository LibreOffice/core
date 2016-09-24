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
#include "vbacell.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbatable.hxx"
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <rtl/ustrbuf.hxx>
#include "wordvbahelper.hxx"
#include "vbatablehelper.hxx"
#include "vbarow.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaCell::SwVbaCell( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nColumn, sal_Int32 nRow ) throw ( uno::RuntimeException ) :
    SwVbaCell_BASE( rParent, rContext ), mxTextTable( xTextTable ), mnColumn( nColumn ), mnRow( nRow )
{
}

SwVbaCell::~SwVbaCell()
{
}

::sal_Int32 SAL_CALL SwVbaCell::getWidth() throw (css::uno::RuntimeException, std::exception)
{
    SwVbaTableHelper aTableHelper( mxTextTable );
    return aTableHelper.GetColWidth( mnColumn, mnRow, true );
}

void SAL_CALL SwVbaCell::setWidth( ::sal_Int32 _width ) throw (css::uno::RuntimeException, std::exception)
{
    SwVbaTableHelper aTableHelper( mxTextTable );
    aTableHelper.SetColWidth( _width, mnColumn, mnRow, true );
}

uno::Any SAL_CALL SwVbaCell::getHeight() throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnRow ) );
    return xRow->getHeight();
}

void SAL_CALL SwVbaCell::setHeight( const uno::Any& _height ) throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnRow ) );
    xRow->setHeight( _height );
}

::sal_Int32 SAL_CALL SwVbaCell::getHeightRule() throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnRow ) );
    return xRow->getHeightRule();
}

void SAL_CALL SwVbaCell::setHeightRule( ::sal_Int32 _heightrule ) throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnRow ) );
    xRow->setHeightRule( _heightrule );
}

void SAL_CALL SwVbaCell::SetWidth( float width, sal_Int32 /*rulestyle*/ ) throw (css::uno::RuntimeException, std::exception)
{
    // FIXME: handle the argument: rulestyle
    setWidth( static_cast<sal_Int32>(width) );
}

void SAL_CALL SwVbaCell::SetHeight( float height, sal_Int32 heightrule ) throw (css::uno::RuntimeException, std::exception)
{
    // FIXME: handle the argument: heightrule
    setHeightRule( heightrule );
    setHeight( uno::makeAny( height ) );
}

OUString
SwVbaCell::getServiceImplName()
{
    return OUString("SwVbaCell");
}

uno::Sequence< OUString >
SwVbaCell::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Cell";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
