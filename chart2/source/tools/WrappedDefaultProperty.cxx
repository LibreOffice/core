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

#include "WrappedDefaultProperty.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

WrappedDefaultProperty::WrappedDefaultProperty(
    const OUString& rOuterName, const OUString& rInnerName,
    const uno::Any& rNewOuterDefault ) :
        WrappedProperty( rOuterName, rInnerName ),
        m_aOuterDefaultValue( rNewOuterDefault )
{}

WrappedDefaultProperty::~WrappedDefaultProperty()
{}

void WrappedDefaultProperty::setPropertyToDefault(
    const Reference< beans::XPropertyState >& xInnerPropertyState ) const
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    Reference< beans::XPropertySet > xInnerPropSet( xInnerPropertyState, uno::UNO_QUERY );
    if( xInnerPropSet.is())
        this->setPropertyValue( m_aOuterDefaultValue, xInnerPropSet );
}

uno::Any WrappedDefaultProperty::getPropertyDefault(
    const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    return m_aOuterDefaultValue;
}

beans::PropertyState WrappedDefaultProperty::getPropertyState(
    const Reference< beans::XPropertyState >& xInnerPropertyState ) const
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    beans::PropertyState aState = beans::PropertyState_DIRECT_VALUE;
    try
    {
        Reference< beans::XPropertySet > xInnerProp( xInnerPropertyState, uno::UNO_QUERY_THROW );
        uno::Any aValue = this->getPropertyValue( xInnerProp );
        if( m_aOuterDefaultValue == this->convertInnerToOuterValue( aValue ))
            aState = beans::PropertyState_DEFAULT_VALUE;
    }
    catch( beans::UnknownPropertyException& ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return aState;
}

} //  namespace chart
