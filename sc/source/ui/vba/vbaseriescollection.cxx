/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbaseriescollection.cxx,v $
 * $Revision: 1.4 $
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
#include "vbaseriescollection.hxx"
#include <ooo/vba/excel/XSeries.hpp>

#include "vbaglobals.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaSeriesCollection::ScVbaSeriesCollection( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext ) : SeriesCollection_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() )
{
}

// XEnumerationAccess

uno::Reference< container::XEnumeration >
ScVbaSeriesCollection::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumeration > xEnum;
    return xEnum;
}

// XElementAccess

uno::Type
ScVbaSeriesCollection::getElementType() throw (uno::RuntimeException)
{
    return excel::XSeries::static_type(0);
}

uno::Any
ScVbaSeriesCollection::createCollectionObject( const css::uno::Any& rSource )
{
    return rSource;
}

rtl::OUString&
ScVbaSeriesCollection::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaSeriesCollection") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
ScVbaSeriesCollection::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.SeriesCollection") );
    }
    return sNames;
}

