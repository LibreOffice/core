/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedProperty.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:07:08 $
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

#include "WrappedProperty.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
//.............................................................................

WrappedProperty::WrappedProperty( const OUString& rOuterName, const OUString& rInnerName)
                         : m_aOuterName( rOuterName )
                         , m_aInnerName( rInnerName )
{
}
WrappedProperty::~WrappedProperty()
{
}

const OUString& WrappedProperty::getOuterName() const
{
    return m_aOuterName;
}

//virtual
::rtl::OUString WrappedProperty::getInnerName() const
{
    return m_aInnerName;
}

Any WrappedProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    return rInnerValue;
}
Any WrappedProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    return rOuterValue;
}

void WrappedProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(xInnerPropertySet.is())
        xInnerPropertySet->setPropertyValue( this->getInnerName(), this->convertOuterToInnerValue( rOuterValue ) );
}

Any WrappedProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    if( xInnerPropertySet.is() )
    {
        aRet = xInnerPropertySet->getPropertyValue( this->getInnerName() );
        aRet = this->convertInnerToOuterValue( aRet );
    }
    return aRet;
}

void WrappedProperty::setPropertyToDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    if( xInnerPropertyState.is() && this->getInnerName().getLength() )
        xInnerPropertyState->setPropertyToDefault(this->getInnerName());
    else
    {
        Reference< beans::XPropertySet > xInnerProp( xInnerPropertyState, uno::UNO_QUERY );
        setPropertyValue( getPropertyDefault( xInnerPropertyState ), xInnerProp );
    }
}

Any WrappedProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    if( xInnerPropertyState.is() )
    {
        aRet = xInnerPropertyState->getPropertyDefault( this->getInnerName() );
        aRet = this->convertInnerToOuterValue( aRet );
    }
    return aRet;
}

beans::PropertyState WrappedProperty::getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    beans::PropertyState aState = beans::PropertyState_DIRECT_VALUE;
    rtl::OUString aInnerName( this->getInnerName() );
    if( xInnerPropertyState.is() && aInnerName.getLength() )
        aState = xInnerPropertyState->getPropertyState( aInnerName );
    else
    {
        try
        {
            Reference< beans::XPropertySet > xInnerProp( xInnerPropertyState, uno::UNO_QUERY );
            uno::Any aValue = this->getPropertyValue( xInnerProp );
            if( !aValue.hasValue() )
                aState = beans::PropertyState_DEFAULT_VALUE;
            else
            {
                uno::Any aDefault = this->getPropertyDefault( xInnerPropertyState );
                if( aValue == aDefault )
                    aState = beans::PropertyState_DEFAULT_VALUE;
            }
        }
        catch( beans::UnknownPropertyException& ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    return aState;
}

//.............................................................................
} //namespace chart
//.............................................................................
