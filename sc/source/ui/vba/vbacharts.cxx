/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "vbacharts.hxx"
#include <basic/sberrors.hxx>
#include <com/sun/star/table/XTableChartsSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;


ScVbaCharts::ScVbaCharts( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const uno::Reference< frame::XModel >& xModel ) : Charts_BASE(_xParent, _xContext, uno::Reference< container::XIndexAccess >())
{
    xComponent.set( xModel, uno::UNO_QUERY_THROW );
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
                ncount += xTableCharts->getElementNames().getLength();
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

