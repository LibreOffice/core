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

#include "NamedLineProperties.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::rtl::OUString;

namespace chart
{

// @deprecated !!
void NamedLineProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    const uno::Type tCppuTypeString = ::getCppuType( reinterpret_cast< const OUString * >(0));

    // Line Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "LineDash" ),
                  PROP_LINE_DASH,
                  ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    if( bIncludeLineEnds )
    {
        rOutProperties.push_back(
            Property( C2U( "LineStartName" ),
                      PROP_LINE_START_NAME,
                      tCppuTypeString,
                      beans::PropertyAttribute::BOUND
                      | beans::PropertyAttribute::MAYBEDEFAULT
                      | beans::PropertyAttribute::MAYBEVOID ));

        rOutProperties.push_back(
            Property( C2U( "LineEndName" ),
                      PROP_LINE_END_NAME,
                      tCppuTypeString,
                      beans::PropertyAttribute::BOUND
                      | beans::PropertyAttribute::MAYBEDEFAULT
                      | beans::PropertyAttribute::MAYBEVOID ));
    }
}

void NamedLineProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap,
    bool bIncludeLineEnds /* = false */ )
{
}

OUString NamedLineProperties::GetPropertyNameForHandle( sal_Int32 nHandle )
{
    //will return e.g. "LineDashName" for PROP_LINE_DASH_NAME
    switch( nHandle )
    {
        case PROP_LINE_DASH_NAME:
            return C2U( "LineDashName" );
        case PROP_LINE_START_NAME:
        case PROP_LINE_END_NAME:
            break;
    }
    return OUString();
}

} //  namespace chart
