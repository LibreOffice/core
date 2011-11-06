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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "AreaWrapper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include "Chart2ModelContact.hxx"
#include "WrappedDirectStateProperty.hxx"
#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>

#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"

#include <algorithm>

using namespace ::com::sun::star;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Area" ));

struct StaticAreaWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        //::chart::NamedProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticAreaWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticAreaWrapperPropertyArray_Initializer >
{
};


} // anonymous namespace

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

AreaWrapper::AreaWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex )
{
}

AreaWrapper::~AreaWrapper()
{}

// ____ XShape ____
awt::Point SAL_CALL AreaWrapper::getPosition()
    throw (uno::RuntimeException)
{
    return awt::Point(0,0);
}

void SAL_CALL AreaWrapper::setPosition( const awt::Point& /*aPosition*/ )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "trying to set position of chart area" );
}

awt::Size SAL_CALL AreaWrapper::getSize()
    throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->GetPageSize();
}

void SAL_CALL AreaWrapper::setSize( const awt::Size& /*aSize*/ )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "trying to set size of chart area" );
}

// ____ XShapeDescriptor (base of XShape) ____
::rtl::OUString SAL_CALL AreaWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return C2U( "com.sun.star.chart.ChartArea" );
}

// ____ XComponent ____
void SAL_CALL AreaWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    // /--
    MutexGuard aGuard( GetMutex());
    clearWrappedPropertySet();
    // \--
}

void SAL_CALL AreaWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL AreaWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ================================================================================

// WrappedPropertySet
Reference< beans::XPropertySet > AreaWrapper::getInnerPropertySet()
{
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( xChartDoc.is() )
        return xChartDoc->getPageBackground();
    OSL_ENSURE(false,"AreaWrapper::getInnerPropertySet() is NULL");
    return 0;
}

const Sequence< beans::Property >& AreaWrapper::getPropertySequence()
{
    return *StaticAreaWrapperPropertyArray::get();
}

const std::vector< WrappedProperty* > AreaWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

    aWrappedProperties.push_back( new WrappedDirectStateProperty( C2U("LineStyle"), C2U("LineStyle") ) );

    return aWrappedProperties;
}

// ================================================================================

Sequence< ::rtl::OUString > AreaWrapper::getSupportedServiceNames_Static()
{
    Sequence< ::rtl::OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 1 ] = C2U( "com.sun.star.beans.PropertySet" );
    aServices[ 2 ] = C2U( "com.sun.star.drawing.FillProperties" );
    aServices[ 3 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( AreaWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart
