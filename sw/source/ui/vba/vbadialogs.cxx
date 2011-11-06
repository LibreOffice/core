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


#include <ooo/vba/word/XDialog.hpp>
#include "vbadialogs.hxx"
#include "vbadialog.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

uno::Any
SwVbaDialogs::Item( const uno::Any &aItem ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    aItem >>= nIndex;
    uno::Reference< word::XDialog > aDialog( new SwVbaDialog( uno::Reference< XHelperInterface >( Application(),uno::UNO_QUERY_THROW ), mxContext, m_xModel, nIndex ) );
    return uno::Any( aDialog );
}

rtl::OUString&
SwVbaDialogs::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaDialogs") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaDialogs::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Dialogs" ) );
    }
    return aServiceNames;
}

