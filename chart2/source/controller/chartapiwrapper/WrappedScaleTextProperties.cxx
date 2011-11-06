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
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

void WrappedScaleTextProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
                                 , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedScaleTextProperty( spChart2ModelContact ) );
}

} //namespace wrapper
} //namespace chart
//.............................................................................
