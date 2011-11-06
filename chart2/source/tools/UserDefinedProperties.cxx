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
#include "UserDefinedProperties.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void UserDefinedProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "ChartUserDefinedAttributes" ),
                  PROP_XML_USERDEF_CHART,
                  ::getCppuType( reinterpret_cast< const uno::Reference< container::XNameContainer > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "TextUserDefinedAttributes" ),
                  PROP_XML_USERDEF_TEXT,
                  ::getCppuType( reinterpret_cast< const uno::Reference< container::XNameContainer > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "ParaUserDefinedAttributes" ),
                  PROP_XML_USERDEF_PARA,
                  ::getCppuType( reinterpret_cast< const uno::Reference< container::XNameContainer > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    // UserDefinedAttributeSupplier
    // ----------------------------
    rOutProperties.push_back(
        Property( C2U( "UserDefinedAttributes" ),
                  PROP_XML_USERDEF,
                  ::getCppuType( reinterpret_cast< const uno::Reference< container::XNameContainer > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
}

} //  namespace chart
