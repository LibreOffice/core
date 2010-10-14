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

::sal_Int32 SAL_CALL SwVbaCell::getWidth() throw (css::uno::RuntimeException)
{
    SwVbaTableHelper aTableHelper( mxTextTable );
    return aTableHelper.GetColWidth( mnColumn, mnRow, sal_True );
}

void SAL_CALL SwVbaCell::setWidth( ::sal_Int32 _width ) throw (css::uno::RuntimeException)
{
    SwVbaTableHelper aTableHelper( mxTextTable );
    aTableHelper.SetColWidth( _width, mnColumn, mnRow, sal_True );
}

uno::Any SAL_CALL SwVbaCell::getHeight() throw (css::uno::RuntimeException)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnRow ) );
    return xRow->getHeight();
}

void SAL_CALL SwVbaCell::setHeight( const uno::Any& _height ) throw (css::uno::RuntimeException)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnRow ) );
    xRow->setHeight( _height );
}

::sal_Int32 SAL_CALL SwVbaCell::getHeightRule() throw (css::uno::RuntimeException)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnRow ) );
    return xRow->getHeightRule();
}

void SAL_CALL SwVbaCell::setHeightRule( ::sal_Int32 _heightrule ) throw (css::uno::RuntimeException)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnRow ) );
    xRow->setHeightRule( _heightrule );
}

void SAL_CALL SwVbaCell::SetWidth( float width, sal_Int32 /*rulestyle*/ ) throw (css::uno::RuntimeException)
{
    // FIXME: handle the argument: rulestyle
    setWidth( width );
}

void SAL_CALL SwVbaCell::SetHeight( float height, sal_Int32 heightrule ) throw (css::uno::RuntimeException)
{
    // FIXME: handle the argument: heightrule
    setHeightRule( heightrule );
    setHeight( uno::makeAny( height ) );
}

rtl::OUString&
SwVbaCell::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaCell") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaCell::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Cell" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
