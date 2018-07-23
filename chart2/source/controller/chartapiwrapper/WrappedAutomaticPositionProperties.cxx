/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "WrappedAutomaticPositionProperties.hxx"
#include <FastPropertyIdRanges.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;

namespace chart
{
namespace wrapper
{

class WrappedAutomaticPositionProperty : public WrappedProperty
{
public:
    WrappedAutomaticPositionProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;
    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;
    virtual Any getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const override;
};

WrappedAutomaticPositionProperty::WrappedAutomaticPositionProperty()
    : ::chart::WrappedProperty( "AutomaticPosition" , OUString() )
{
}

void WrappedAutomaticPositionProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    if( xInnerPropertySet.is() )
    {
        bool bNewValue = true;
        if( ! (rOuterValue >>= bNewValue) )
            throw lang::IllegalArgumentException( "Property AutomaticPosition requires value of type boolean", nullptr, 0 );

        try
        {
            if( bNewValue )
            {
                Any aRelativePosition( xInnerPropertySet->getPropertyValue( "RelativePosition" ) );
                if( aRelativePosition.hasValue() )
                    xInnerPropertySet->setPropertyValue( "RelativePosition", Any() );
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

Any WrappedAutomaticPositionProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet( getPropertyDefault( Reference< beans::XPropertyState >( xInnerPropertySet, uno::UNO_QUERY ) ) );
    if( xInnerPropertySet.is() )
    {
        Any aRelativePosition( xInnerPropertySet->getPropertyValue( "RelativePosition" ) );
        if( !aRelativePosition.hasValue() )
            aRet <<= true;
    }
    return aRet;
}

Any WrappedAutomaticPositionProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
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

}//anonymous namespace

void WrappedAutomaticPositionProperties::addProperties( std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "AutomaticPosition",
                  PROP_CHART_AUTOMATIC_POSITION,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

void WrappedAutomaticPositionProperties::addWrappedProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList )
{
    rList.emplace_back( new WrappedAutomaticPositionProperty() );
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
