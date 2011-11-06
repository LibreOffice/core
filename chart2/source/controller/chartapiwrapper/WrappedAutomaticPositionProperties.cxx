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
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

void WrappedAutomaticPositionProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList )
{
    lcl_addWrappedProperties( rList );
}

} //namespace wrapper
} //namespace chart
//.............................................................................
