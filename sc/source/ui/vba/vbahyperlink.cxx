/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <vbahelper/helperdecl.hxx>
#include "vbahyperlink.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaHyperlink::ScVbaHyperlink( uno::Sequence< uno::Any> const & args,
    uno::Reference< uno::XComponentContext> const & xContext ) throw ( lang::IllegalArgumentException ) :  HyperlinkImpl_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext ), mxCell( getXSomethingFromArgs< table::XCell >( args, 1, false ) )
{
    mxCell = getXSomethingFromArgs< table::XCell >( args, 1, false );
    uno::Reference< text::XTextFieldsSupplier > xTextFields( mxCell, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndex( xTextFields->getTextFields(), uno::UNO_QUERY_THROW );
    mxTextField.set( xIndex->getByIndex(0), uno::UNO_QUERY_THROW );
}

ScVbaHyperlink::~ScVbaHyperlink()
{
}

::rtl::OUString
ScVbaHyperlink::getAddress() throw (css::uno::RuntimeException)
{
    rtl::OUString aAddress;
    uno::Any aValue = mxTextField->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) );
    aValue >>= aAddress;
    return aAddress;
}

void
ScVbaHyperlink::setAddress( const ::rtl::OUString & rAddress ) throw (css::uno::RuntimeException)
{
    uno::Any aValue;
    aValue <<= rAddress;
    mxTextField->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ), aValue );
}

::rtl::OUString
ScVbaHyperlink::getName() throw (css::uno::RuntimeException)
{
    // it seems this attribute is same as TextToDisplay
    return getTextToDisplay();
}

void
ScVbaHyperlink::setName( const ::rtl::OUString &  rName ) throw (css::uno::RuntimeException)
{
    setTextToDisplay( rName );
}

::rtl::OUString
ScVbaHyperlink::getTextToDisplay() throw (css::uno::RuntimeException)
{
    rtl::OUString aTextToDisplay;
    uno::Any aValue = mxTextField->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Representation" ) ) );
    aValue >>= aTextToDisplay;
    return aTextToDisplay;
}

void
ScVbaHyperlink::setTextToDisplay( const ::rtl::OUString & rTextToDisplay ) throw (css::uno::RuntimeException)
{
    uno::Any aValue;
    aValue <<= rTextToDisplay;
    mxTextField->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Representation" ) ), aValue );
}

css::uno::Reference< ov::excel::XRange > SAL_CALL ScVbaHyperlink::Range() throw (css::uno::RuntimeException)
{
    uno::Reference< table::XCellRange > xRange( mxCell,uno::UNO_QUERY_THROW );
    // FIXME: need to pass current worksheet as the parent of XRange.
    return uno::Reference< excel::XRange >( new ScVbaRange( uno::Reference< XHelperInterface > (), mxContext, xRange ) );
}

rtl::OUString&
ScVbaHyperlink::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaHyperlink") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaHyperlink::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Hyperlink" ) );
    }
    return aServiceNames;
}

namespace hyperlink
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaHyperlink, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaHyperlink",
    "ooo.vba.excel.Hyperlink" );
}
