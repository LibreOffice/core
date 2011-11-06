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

