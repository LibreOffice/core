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

#include "WrappedScaleTextProperties.hxx"
#include "FastPropertyIdRanges.hxx"
#include "macros.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>

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
    : ::chart::WrappedProperty( C2U( "ScaleText" ), rtl::OUString() )
    , m_spChart2ModelContact( spChart2ModelContact )
{
}

WrappedScaleTextProperty::~WrappedScaleTextProperty()
{
}

void WrappedScaleTextProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    static const OUString aRefSizeName( RTL_CONSTASCII_USTRINGPARAM("ReferencePageSize") );

    if( xInnerPropertySet.is() )
    {
        bool bNewValue = false;
        if( ! (rOuterValue >>= bNewValue) )
        {
            if( rOuterValue.hasValue() )
                throw lang::IllegalArgumentException( C2U("Property ScaleText requires value of type boolean"), 0, 0 );
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
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

Any WrappedScaleTextProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    static const OUString aRefSizeName( RTL_CONSTASCII_USTRINGPARAM("ReferencePageSize") );

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

//-----------------------------------------------------------------------------
void WrappedScaleTextProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "ScaleText" ),
                  PROP_CHART_SCALE_TEXT,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

//-----------------------------------------------------------------------------

void WrappedScaleTextProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
                                 , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedScaleTextProperty( spChart2ModelContact ) );
}

} //namespace wrapper
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
