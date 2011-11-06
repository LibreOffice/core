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


#include "vbasection.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbapagesetup.hxx"

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

uno::Any SAL_CALL SwVbaSection::Headers(  ) throw (uno::RuntimeException)
{
    return uno::Any();
}

uno::Any SAL_CALL SwVbaSection::Footers(  ) throw (uno::RuntimeException)
{
    return uno::Any();
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

