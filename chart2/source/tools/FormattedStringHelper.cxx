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

#include "FormattedStringHelper.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using rtl::OUString;

Sequence< Reference< chart2::XFormattedString > >
            FormattedStringHelper::createFormattedStringSequence(
                     const Reference< uno::XComponentContext > & xContext
                    , const OUString & rString
                    , const Reference< beans::XPropertySet > & xTextProperties ) throw()
{
    Reference< XFormattedString > xFormStr;
    try
    {
        if( xContext.is() )
        {
            xFormStr.set(
                xContext->getServiceManager()->createInstanceWithContext(
                    C2U("com.sun.star.chart2.FormattedString"), xContext ),
                uno::UNO_QUERY_THROW );

            xFormStr->setString( rString );

            // set character properties
            comphelper::copyProperties(
                xTextProperties, Reference< beans::XPropertySet >( xFormStr, uno::UNO_QUERY ) );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return Sequence< Reference< XFormattedString > >( & xFormStr, 1 );
}

//.............................................................................
} //namespace chart
//.............................................................................

