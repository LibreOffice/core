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

#include "WrappedProperty.hxx"
#include "macros.hxx"
#include <com/sun/star/drawing/LineStyle.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
