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

#include "precompiled_chart2.hxx"

#include "WrappedAutomaticPositionProperties.hxx"
#include "FastPropertyIdRanges.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::rtl::OUString;

//.............................................................................
namespace chart
{
namespace wrapper
{

class WrappedAutomaticPositionProperty : public WrappedProperty
{
public:
    WrappedAutomaticPositionProperty();
    virtual ~WrappedAutomaticPositionProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException);
    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    virtual Any getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
};

WrappedAutomaticPositionProperty::WrappedAutomaticPositionProperty()
    : ::chart::WrappedProperty( C2U( "AutomaticPosition" ), rtl::OUString() )
{
}
WrappedAutomaticPositionProperty::~WrappedAutomaticPositionProperty()
{
}

void WrappedAutomaticPositionProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( xInnerPropertySet.is() )
    {
        bool bNewValue = true;
        if( ! (rOuterValue >>= bNewValue) )
            throw lang::IllegalArgumentException( C2U("Property AutomaticPosition requires value of type boolean"), 0, 0 );

        try
        {
            if( bNewValue )
            {
                Any aRelativePosition( xInnerPropertySet->getPropertyValue( C2U( "RelativePosition" ) ) );
                if( aRelativePosition.hasValue() )
                    xInnerPropertySet->setPropertyValue( C2U( "RelativePosition" ), Any() );
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

Any WrappedAutomaticPositionProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet( getPropertyDefault( Reference< beans::XPropertyState >( xInnerPropertySet, uno::UNO_QUERY ) ) );
    if( xInnerPropertySet.is() )
    {
        Any aRelativePosition( xInnerPropertySet->getPropertyValue( C2U( "RelativePosition" ) ) );
        if( !aRelativePosition.hasValue() )
            aRet <<= true;
    }
    return aRet;
}

Any WrappedAutomaticPositionProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

namespace
{
enum
{
    PROP_CHART_AUTOMATIC_POSITION = FAST_PROPERTY_ID_START_CHART_AUTOPOSITION_PROP
};

void lcl_addWrappedProperties( std::vector< WrappedProperty* >& rList )
{
    rList.push_back( new WrappedAutomaticPositionProperty() );
}

}//anonymous namespace

//-----------------------------------------------------------------------------

void WrappedAutomaticPositionProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "AutomaticPosition" ),
                  PROP_CHART_AUTOMATIC_POSITION,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

//-----------------------------------------------------------------------------

void WrappedAutomaticPositionProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList )
{
    lcl_addWrappedProperties( rList );
}

} //namespace wrapper
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
