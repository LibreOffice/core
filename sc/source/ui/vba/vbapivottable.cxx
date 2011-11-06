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


#include "vbapivottable.hxx"
#include "vbapivotcache.hxx"


using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaPivotTable::ScVbaPivotTable( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XDataPilotTable >& xTable ) : PivotTableImpl_BASE( uno::Reference< XHelperInterface >(),  xContext), m_xTable( xTable )
{
}

uno::Reference< excel::XPivotCache >
ScVbaPivotTable::PivotCache() throw (uno::RuntimeException)
{
    // #FIXME with a quick example failed to determine what the parent
    // should be, leaving as null at the moment
    return new ScVbaPivotCache( uno::Reference< XHelperInterface >(), mxContext, m_xTable );
}

rtl::OUString&
ScVbaPivotTable::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPivotTable") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaPivotTable::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.PivotTable" ) );
    }
    return aServiceNames;
}
