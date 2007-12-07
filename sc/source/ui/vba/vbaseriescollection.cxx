/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaseriescollection.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:01:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "vbaseriescollection.hxx"
#include <org/openoffice/excel/XSeries.hpp>

#include "vbaglobals.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;

ScVbaSeriesCollection::ScVbaSeriesCollection( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext ) : SeriesCollection_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() )
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
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.SeriesCollection") );
    }
    return sNames;
}

