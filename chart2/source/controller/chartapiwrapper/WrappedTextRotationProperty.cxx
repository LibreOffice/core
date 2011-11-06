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

#include "WrappedTextRotationProperty.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/XPropertyState.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
//.............................................................................

WrappedTextRotationProperty::WrappedTextRotationProperty( bool bDirectState )
    : ::chart::WrappedProperty( C2U( "TextRotation" ), C2U( "TextRotation" ) )
    , m_bDirectState( bDirectState )
{
}
WrappedTextRotationProperty::~WrappedTextRotationProperty()
{
}

beans::PropertyState WrappedTextRotationProperty::getPropertyState( const uno::Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    if( m_bDirectState )
        return beans::PropertyState_DIRECT_VALUE;
    return WrappedProperty::getPropertyState( xInnerPropertyState );
}

Any WrappedTextRotationProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    Any aRet;
    double fVal = 0;
    if( rInnerValue >>= fVal )
    {
        sal_Int32 n100thDegrees = static_cast< sal_Int32 >( fVal * 100.0 );
        aRet <<= n100thDegrees;
    }
    return aRet;
}
Any WrappedTextRotationProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    Any aRet;
    sal_Int32 nVal = 0;
    if( rOuterValue >>= nVal )
    {
        double fDoubleDegrees = ( static_cast< double >( nVal ) / 100.0 );
        aRet <<= fDoubleDegrees;
    }
    return aRet;
}

//.............................................................................
} //namespace chart
//.............................................................................
