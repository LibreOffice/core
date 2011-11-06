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

#include "WrappedCharacterHeightProperty.hxx"
#include "macros.hxx"
#include "RelativeSizeHelper.hxx"
#include "ReferenceSizePropertyProvider.hxx"

// header for define DBG_ASSERT
#include <tools/debug.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;

//.............................................................................
//.............................................................................

//.............................................................................
//.............................................................................

namespace chart
{
namespace wrapper
{
WrappedCharacterHeightProperty_Base::WrappedCharacterHeightProperty_Base(
                            const OUString& rOuterEqualsInnerName
                            , ReferenceSizePropertyProvider* pRefSizePropProvider )
                            : WrappedProperty( rOuterEqualsInnerName, rOuterEqualsInnerName )
                           , m_pRefSizePropProvider( pRefSizePropProvider )
{
}
WrappedCharacterHeightProperty_Base::~WrappedCharacterHeightProperty_Base()
{
}

void WrappedCharacterHeightProperty::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , ReferenceSizePropertyProvider* pRefSizePropProvider  )
{
    rList.push_back( new WrappedCharacterHeightProperty( pRefSizePropProvider ) );
    rList.push_back( new WrappedAsianCharacterHeightProperty( pRefSizePropProvider ) );
    rList.push_back( new WrappedComplexCharacterHeightProperty( pRefSizePropProvider ) );
}

void WrappedCharacterHeightProperty_Base::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(xInnerPropertySet.is())
    {
        if( m_pRefSizePropProvider )
            m_pRefSizePropProvider->updateReferenceSize();
        xInnerPropertySet->setPropertyValue( m_aInnerName, rOuterValue );
    }
}

Any WrappedCharacterHeightProperty_Base::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    if( xInnerPropertySet.is() )
    {
        aRet = xInnerPropertySet->getPropertyValue( m_aInnerName );
        float fHeight = 0;
        if( aRet >>= fHeight )
        {
            if( m_pRefSizePropProvider )
            {
                awt::Size aReferenceSize;
                if( m_pRefSizePropProvider->getReferenceSize() >>= aReferenceSize )
                {
                    awt::Size aCurrentSize = m_pRefSizePropProvider->getCurrentSizeForReference();
                    aRet <<= static_cast< float >(
                            RelativeSizeHelper::calculate( fHeight, aReferenceSize, aCurrentSize ));
                }
            }
        }
    }
    return aRet;
}

Any WrappedCharacterHeightProperty_Base::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    if( xInnerPropertyState.is() )
    {
        aRet = xInnerPropertyState->getPropertyDefault( m_aInnerName );
    }
    return aRet;
}

beans::PropertyState WrappedCharacterHeightProperty_Base::getPropertyState( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    return beans::PropertyState_DIRECT_VALUE;
}

Any WrappedCharacterHeightProperty_Base::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    OSL_ASSERT("should not be used: WrappedCharacterHeightProperty_Base::convertInnerToOuterValue - check if you miss data");
    return rInnerValue;
}
Any WrappedCharacterHeightProperty_Base::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    OSL_ASSERT("should not be used: WrappedCharacterHeightProperty_Base::convertOuterToInnerValue - check if you miss data");
    return rOuterValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

WrappedCharacterHeightProperty::WrappedCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider )
        : WrappedCharacterHeightProperty_Base( C2U( "CharHeight" ), pRefSizePropProvider )
{
}
WrappedCharacterHeightProperty::~WrappedCharacterHeightProperty()
{
}

//-----------------------------------------------------------------------------

WrappedAsianCharacterHeightProperty::WrappedAsianCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider )
        : WrappedCharacterHeightProperty_Base( C2U( "CharHeightAsian" ), pRefSizePropProvider )
{
}
WrappedAsianCharacterHeightProperty::~WrappedAsianCharacterHeightProperty()
{
}

//-----------------------------------------------------------------------------

WrappedComplexCharacterHeightProperty::WrappedComplexCharacterHeightProperty( ReferenceSizePropertyProvider* pRefSizePropProvider )
        : WrappedCharacterHeightProperty_Base( C2U( "CharHeightComplex" ), pRefSizePropProvider )
{
}
WrappedComplexCharacterHeightProperty::~WrappedComplexCharacterHeightProperty()
{
}

} //namespace wrapper
} //namespace chart
//.............................................................................
