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


#include "vbapivotcache.hxx"


using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaPivotCache::ScVbaPivotCache( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,  const uno::Reference< sheet::XDataPilotTable >& xTable ) : PivotCacheImpl_BASE( xParent, xContext ), m_xTable( xTable )
{
}

void SAL_CALL
ScVbaPivotCache::Refresh() throw (css::uno::RuntimeException)
{
    m_xTable->refresh();
}
rtl::OUString&
ScVbaPivotCache::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPivotCache") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaPivotCache::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.PivotCache" ) );
    }
    return aServiceNames;
}

