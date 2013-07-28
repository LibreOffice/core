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

#include "WrappedScaleTextProperties.hxx"
#include "FastPropertyIdRanges.hxx"
#include "macros.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace chart
{
namespace wrapper
{

class WrappedScaleTextProperty : public WrappedProperty
{
public:
    WrappedScaleTextProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedScaleTextProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException);
    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    virtual Any getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

private:
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
};

WrappedScaleTextProperty::WrappedScaleTextProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
    : ::chart::WrappedProperty( "ScaleText" , OUString() )
    , m_spChart2ModelContact( spChart2ModelContact )
{
}

WrappedScaleTextProperty::~WrappedScaleTextProperty()
{
}

void WrappedScaleTextProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    static const OUString aRefSizeName( "ReferencePageSize" );

    if( xInnerPropertySet.is() )
    {
        bool bNewValue = false;
        if( ! (rOuterValue >>= bNewValue) )
        {
            if( rOuterValue.hasValue() )
                throw lang::IllegalArgumentException( "Property ScaleText requires value of type boolean", 0, 0 );
        }

        try
        {
            if( bNewValue )
            {
                awt::Size aRefSize( m_spChart2ModelContact->GetPageSize() );
                xInnerPropertySet->setPropertyValue( aRefSizeName, uno::makeAny( aRefSize ) );
            }
            else
                xInnerPropertySet->setPropertyValue( aRefSizeName, Any() );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

Any WrappedScaleTextProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    static const OUString aRefSizeName( "ReferencePageSize" );

    Any aRet( getPropertyDefault( Reference< beans::XPropertyState >( xInnerPropertySet, uno::UNO_QUERY ) ) );
    if( xInnerPropertySet.is() )
    {
        if( xInnerPropertySet->getPropertyValue( aRefSizeName ).hasValue() )
            aRet <<= true;
        else
            aRet <<= false;
    }

    return aRet;
}

Any WrappedScaleTextProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
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
    PROP_CHART_SCALE_TEXT = FAST_PROPERTY_ID_START_SCALE_TEXT_PROP
};

}//anonymous namespace

void WrappedScaleTextProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "ScaleText",
                  PROP_CHART_SCALE_TEXT,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void WrappedScaleTextProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
                                 , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedScaleTextProperty( spChart2ModelContact ) );
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
