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


#include "vbareplacement.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaReplacement::SwVbaReplacement( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< util::XPropertyReplace >& xPropertyReplace ) throw ( uno::RuntimeException ) :
    SwVbaReplacement_BASE( rParent, rContext ), mxPropertyReplace( xPropertyReplace )
{
}

SwVbaReplacement::~SwVbaReplacement()
{
}

::rtl::OUString SAL_CALL SwVbaReplacement::getText() throw (uno::RuntimeException)
{
    return mxPropertyReplace->getReplaceString();
}

void SAL_CALL SwVbaReplacement::setText( const ::rtl::OUString& _text ) throw (uno::RuntimeException)
{
    mxPropertyReplace->setReplaceString( _text );
}

rtl::OUString&
SwVbaReplacement::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaReplacement") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaReplacement::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Replacement" ) );
    }
    return aServiceNames;
}

