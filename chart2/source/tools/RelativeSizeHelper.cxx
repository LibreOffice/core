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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "RelativeSizeHelper.hxx"
#include "macros.hxx"

#include <vector>
#include <algorithm>

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Exception;
using ::rtl::OUString;

namespace chart
{

double RelativeSizeHelper::calculate(
    double fValue,
    const Size & rOldReferenceSize,
    const Size & rNewReferenceSize )
{
    if( rOldReferenceSize.Width <= 0 ||
        rOldReferenceSize.Height <= 0 )
        return fValue;

    return min(
        static_cast< double >( rNewReferenceSize.Width )  / static_cast< double >( rOldReferenceSize.Width ),
        static_cast< double >( rNewReferenceSize.Height ) / static_cast< double >( rOldReferenceSize.Height ))
        * fValue;
}

void RelativeSizeHelper::adaptFontSizes(
    const Reference< XPropertySet > & xTargetProperties,
    const Size & rOldReferenceSize,
    const Size & rNewReferenceSize )
{
    if( ! xTargetProperties.is())
        return;

    float fFontHeight = 0;

    vector< OUString > aProperties;
    aProperties.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" )));
    aProperties.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeightAsian" )));
    aProperties.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeightComplex" )));

    for( vector< OUString >::const_iterator aIt = aProperties.begin();
         aIt != aProperties.end(); ++aIt )
    {
        try
        {
            if( xTargetProperties->getPropertyValue( *aIt ) >>= fFontHeight )
            {
                xTargetProperties->setPropertyValue(
                    *aIt,
                    makeAny( static_cast< float >(
                                 calculate( fFontHeight, rOldReferenceSize, rNewReferenceSize ))));
            }
        }
        catch( const Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

} //  namespace chart
