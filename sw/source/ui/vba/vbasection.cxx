/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "vbasection.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbapagesetup.hxx"
#include "vbaheadersfooters.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaSection::SwVbaSection( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& xProps ) throw ( uno::RuntimeException ) :
    SwVbaSection_BASE( rParent, rContext ), mxModel( xModel ), mxPageProps( xProps )
{
}

SwVbaSection::~SwVbaSection()
{
}

::sal_Bool SAL_CALL SwVbaSection::getProtectedForForms() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaSection::setProtectedForForms( ::sal_Bool /*_protectedforforms*/ ) throw (uno::RuntimeException)
{
}

uno::Any SAL_CALL SwVbaSection::Headers( const uno::Any& index ) throw (uno::RuntimeException)
{
    uno::Reference< XCollection > xCol( new SwVbaHeadersFooters( this, mxContext, mxModel, mxPageProps, sal_True ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Any SAL_CALL SwVbaSection::Footers( const uno::Any& index ) throw (uno::RuntimeException)
{
    uno::Reference< XCollection > xCol( new SwVbaHeadersFooters( this, mxContext, mxModel, mxPageProps, sal_False ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Any SAL_CALL
SwVbaSection::PageSetup( ) throw (uno::RuntimeException)
{
    return uno::makeAny( uno::Reference< word::XPageSetup >( new SwVbaPageSetup( this, mxContext, mxModel, mxPageProps ) ) );
}

rtl::OUString&
SwVbaSection::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaSection") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaSection::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Section" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
