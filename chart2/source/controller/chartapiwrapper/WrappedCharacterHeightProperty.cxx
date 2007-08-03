/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedCharacterHeightProperty.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:32:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedCharacterHeightProperty.hxx"
#include "macros.hxx"
#include "RelativeSizeHelper.hxx"
#include "ReferenceSizePropertyProvider.hxx"

// header for define DBG_ASSERT
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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

//static
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
            m_pRefSizePropProvider->setCurrentSizeAsReference();
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
