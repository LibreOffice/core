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


#include "vbapane.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbaview.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaPane::SwVbaPane( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,
    const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException ) :
    SwVbaPane_BASE( rParent, rContext ), mxModel( xModel )
{
}

SwVbaPane::~SwVbaPane()
{
}

uno::Any SAL_CALL
SwVbaPane::View() throw ( css::uno::RuntimeException )
{
    return uno::makeAny( uno::Reference< word::XView >( new SwVbaView( this,  mxContext, mxModel ) ) );
}

void SAL_CALL
SwVbaPane::Close( ) throw ( css::uno::RuntimeException )
{
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CloseWin"));
    dispatchRequests( mxModel,url );
}

rtl::OUString&
SwVbaPane::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaPane") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaPane::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Pane" ) );
    }
    return aServiceNames;
}

