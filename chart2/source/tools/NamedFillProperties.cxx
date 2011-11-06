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

#include "NamedFillProperties.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/chart2/FillBitmap.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::rtl::OUString;

namespace chart
{

void NamedFillProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    const uno::Type tCppuTypeString = ::getCppuType( reinterpret_cast< const OUString * >(0));

    // Fill Properties
    // ---------------

    //optional property:
    rOutProperties.push_back(
        Property( C2U( "FillTransparenceGradient" ),
                  PROP_FILL_TRANSPARENCE_GRADIENT,
                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    //optional property:
    rOutProperties.push_back(
        Property( C2U( "FillGradient" ),
                  PROP_FILL_GRADIENT,
                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    //optional property:
    rOutProperties.push_back(
        Property( C2U( "FillHatch" ),
                  PROP_FILL_HATCH,
                  ::getCppuType( reinterpret_cast< const drawing::Hatch * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    //optional property:
    rOutProperties.push_back(
        Property( C2U( "FillBitmapURL" ),
                  PROP_FILL_BITMAP,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void NamedFillProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
}

OUString NamedFillProperties::GetPropertyNameForHandle( sal_Int32 nHandle )
{
    //will return e.g. "FillGradientName" for PROP_FILL_GRADIENT_NAME
    switch( nHandle )
    {
        case PROP_FILL_GRADIENT_NAME:
            return C2U( "FillGradientName" );
        case PROP_FILL_HATCH_NAME:
            return C2U( "FillHatchName" );
        case PROP_FILL_BITMAP_NAME:
            return C2U( "FillBitmapName" );
        case PROP_FILL_TRANSPARENCY_GRADIENT_NAME:
            return C2U( "FillTransparenceGradientName" );
    }
    return OUString();
}

} //  namespace chart
