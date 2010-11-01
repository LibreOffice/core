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
#include "vbapages.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaPages::ScVbaPages( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xPages ) throw( lang::IllegalArgumentException ) : ScVbaPages_BASE( xParent, xContext, xPages )
{
}

uno::Type SAL_CALL
ScVbaPages::getElementType() throw (uno::RuntimeException)
{
    // return msforms::XPage::static_type(0);
    return uno::XInterface::static_type(0);
}

uno::Any
ScVbaPages::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

rtl::OUString&
ScVbaPages::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPages") );
    return sImplName;
}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaPages::createEnumeration() throw (uno::RuntimeException)
{
    return uno::Reference< container::XEnumeration >();
}

uno::Sequence< rtl::OUString >
ScVbaPages::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msform.Pages" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
