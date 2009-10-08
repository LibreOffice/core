/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbacharts.cxx,v $
 * $Revision: 1.3 $
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
#include "vbacharts.hxx"
#include <basic/sberrors.hxx>
#include <com/sun/star/table/XTableChartsSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;


ScVbaCharts::ScVbaCharts( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext ) : Charts_BASE(_xParent, _xContext, uno::Reference< container::XIndexAccess >() )
{
    // #TODO #FIXME surely this is wrong, you should never use the
    // currently documement ( it could be anything )
    xComponent.set( getCurrentDocument(), uno::UNO_QUERY_THROW );
    xSpreadsheetDocument.set( xComponent, uno::UNO_QUERY_THROW );
}

uno::Any SAL_CALL
ScVbaCharts::Add() throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    // Not implemented in the helperapi ( see ChartsImpl.java )
    if ( true )
        throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_BAD_METHOD, rtl::OUString() );
    return aNULL();
}

uno::Reference< excel::XChart > SAL_CALL
ScVbaCharts::getActiveChart() throw (script::BasicErrorException, uno::RuntimeException)
{
    return xActiveChart;
}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaCharts::createEnumeration() throw (uno::RuntimeException)
{
    // #FIXME not implemented
    if ( true )
        throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_BAD_METHOD, rtl::OUString() );
    return uno::Reference< container::XEnumeration >();
}

// #FIXME #TODO this method shouldn't appear in this class directly
// a XIndexAccess/XNameAccess wrapper should be passed to the base class instead
::sal_Int32 SAL_CALL
ScVbaCharts::getCount() throw (uno::RuntimeException)
{
    sal_Int32 ncount = 0;
    try
    {
        uno::Reference< sheet::XSpreadsheets > xSpreadsheets( xSpreadsheetDocument->getSheets() );
        uno::Sequence< rtl::OUString > SheetNames = xSpreadsheets->getElementNames();
        sal_Int32 nLen = SheetNames.getLength();
        for (sal_Int32 i = 0; i < nLen; i++)
        {
            uno::Reference< table::XTableChartsSupplier > xTableChartsSupplier( xSpreadsheets->getByName(SheetNames[i]), uno::UNO_QUERY);
            if ( xTableChartsSupplier.is() )
            {
                uno::Reference< table::XTableCharts > xTableCharts = xTableChartsSupplier->getCharts();
                ncount =+ xTableCharts->getElementNames().getLength();
            }
        }
    }
    catch (uno::Exception& )
    {
        throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return ncount;
}

uno::Any
ScVbaCharts::createCollectionObject( const uno::Any& aSource )
{
    if ( true )
        throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_BAD_METHOD, rtl::OUString() );
    // #TODO implementation please
    return aSource;
}

rtl::OUString&
ScVbaCharts::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCharts") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
ScVbaCharts::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Charts") );
    }
    return sNames;
}

