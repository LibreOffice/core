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


#include "vbaaxistitle.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaAxisTitle::ScVbaAxisTitle( const uno::Reference< XHelperInterface >& xParent,  const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& _xTitleShape ) :  AxisTitleBase( xParent, xContext, _xTitleShape )
{
}

rtl::OUString&
ScVbaAxisTitle::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaAxisTitle") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaAxisTitle::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        uno::Sequence< rtl::OUString > BaseServiceNames = AxisTitleBase::getServiceNames();
        aServiceNames.realloc( BaseServiceNames.getLength() + 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.AxisTitle" ) );
        for ( sal_Int32 index = 1; index < (BaseServiceNames.getLength() + 1); ++index )
            aServiceNames[ index ] = BaseServiceNames[ index ];
    }
    return aServiceNames;
}


