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

#include "GridWrapper.hxx"
#include "macros.hxx"
#include "AxisHelper.hxx"
#include "Chart2ModelContact.hxx"
#include "ContainerHelper.hxx"
#include "AxisIndexDefines.hxx"
#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "LineProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "WrappedDefaultProperty.hxx"

#include <algorithm>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{
static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Grid" ));

struct StaticGridWrapperPropertyArray_Initializer
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
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticGridWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticGridWrapperPropertyArray_Initializer >
{
};

} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

GridWrapper::GridWrapper(
    tGridType eType, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex ),
        m_eType( eType )
{
}

GridWrapper::~GridWrapper()
{}

void GridWrapper::getDimensionAndSubGridBool( tGridType eType, sal_Int32& rnDimensionIndex, bool& rbSubGrid )
{
    rnDimensionIndex = 1;
    rbSubGrid = false;

    switch( eType )
    {
        case X_MAJOR_GRID:
            rnDimensionIndex = 0; rbSubGrid = false; break;
        case Y_MAJOR_GRID:
            rnDimensionIndex = 1; rbSubGrid = false; break;
        case Z_MAJOR_GRID:
            rnDimensionIndex = 2; rbSubGrid = false; break;
        case X_MINOR_GRID:
            rnDimensionIndex = 0; rbSubGrid = true;  break;
        case Y_MINOR_GRID:
            rnDimensionIndex = 1; rbSubGrid = true;  break;
        case Z_MINOR_GRID:
            rnDimensionIndex = 2; rbSubGrid = true;  break;
    }
}

// ____ XComponent ____
void SAL_CALL GridWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    clearWrappedPropertySet();
}

void SAL_CALL GridWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL GridWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ================================================================================

// WrappedPropertySet

Reference< beans::XPropertySet > GridWrapper::getInnerPropertySet()
{
    Reference< beans::XPropertySet > xRet;
    try
    {
        Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        uno::Reference< XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 /*nCooSysIndex*/ ) );

        sal_Int32 nDimensionIndex = 1;
        bool bSubGrid = false;
        getDimensionAndSubGridBool( m_eType, nDimensionIndex, bSubGrid );

        sal_Int32 nSubGridIndex = bSubGrid ? 0 : -1;
        xRet.set( AxisHelper::getGridProperties( xCooSys , nDimensionIndex, MAIN_AXIS_INDEX, nSubGridIndex ) );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return xRet;
}

const Sequence< beans::Property >& GridWrapper::getPropertySequence()
{
    return *StaticGridWrapperPropertyArray::get();
}

const std::vector< WrappedProperty* > GridWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

    aWrappedProperties.push_back( new WrappedDefaultProperty( C2U("LineColor"), C2U("LineColor"), uno::makeAny( sal_Int32( 0x000000) ) ) ); // black

    return aWrappedProperties;
}

// ================================================================================

Sequence< OUString > GridWrapper::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartGrid" );
    aServices[ 1 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 2 ] = C2U( "com.sun.star.drawing.LineProperties" );
    aServices[ 3 ] = C2U( "com.sun.star.beans.PropertySet" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( GridWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
