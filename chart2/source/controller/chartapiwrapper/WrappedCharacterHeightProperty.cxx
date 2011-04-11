/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    OSL_FAIL("should not be used: WrappedCharacterHeightProperty_Base::convertInnerToOuterValue - check if you miss data");
    return rInnerValue;
}
Any WrappedCharacterHeightProperty_Base::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    OSL_FAIL("should not be used: WrappedCharacterHeightProperty_Base::convertOuterToInnerValue - check if you miss data");
    return rOuterValue;
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
