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
