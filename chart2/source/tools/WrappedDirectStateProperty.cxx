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

#include "WrappedDirectStateProperty.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

WrappedDirectStateProperty::WrappedDirectStateProperty(
    const OUString& rOuterName, const OUString& rInnerName ) :
        WrappedProperty( rOuterName, rInnerName )
{}

WrappedDirectStateProperty::~WrappedDirectStateProperty()
{}

beans::PropertyState WrappedDirectStateProperty::getPropertyState(
    const Reference< beans::XPropertyState >& /* xInnerPropertyState */ ) const
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    return beans::PropertyState_DIRECT_VALUE;
}

} //  namespace chart
