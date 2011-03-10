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

    MutexGuard aGuard( GetMutex());
    clearWrappedPropertySet();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
