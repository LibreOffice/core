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

#include "WrappedSeriesAreaOrLineProperty.hxx"
#include "DataSeriesPointWrapper.hxx"
#include "macros.hxx"

using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
namespace wrapper
{

WrappedSeriesAreaOrLineProperty::WrappedSeriesAreaOrLineProperty(
      const rtl::OUString& rOuterName
    , const rtl::OUString& rInnerAreaTypeName
    , const rtl::OUString& rInnerLineTypeName
    , DataSeriesPointWrapper* pDataSeriesPointWrapper )
    : WrappedProperty( rOuterName, C2U( "" ) )
    , m_pDataSeriesPointWrapper( pDataSeriesPointWrapper )
    , m_aInnerAreaTypeName( rInnerAreaTypeName )
    , m_aInnerLineTypeName( rInnerLineTypeName )
{
}
WrappedSeriesAreaOrLineProperty::~WrappedSeriesAreaOrLineProperty()
{
}

//virtual
::rtl::OUString WrappedSeriesAreaOrLineProperty::getInnerName() const
{
    if( m_pDataSeriesPointWrapper && !m_pDataSeriesPointWrapper->isSupportingAreaProperties() )
        return m_aInnerLineTypeName;
    return m_aInnerAreaTypeName;
}

} //namespace wrapper
} //namespace chart
//.............................................................................
