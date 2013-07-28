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

#include "GridWrapper.hxx"
#include "macros.hxx"
#include "AxisHelper.hxx"
#include "Chart2ModelContact.hxx"
#include "ContainerHelper.hxx"
#include "AxisIndexDefines.hxx"
#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "LinePropertiesHelper.hxx"
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

namespace
{
static const OUString lcl_aServiceName( "com.sun.star.comp.chart.Grid" );

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
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
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
    catch( const uno::Exception & ex )
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

    aWrappedProperties.push_back( new WrappedDefaultProperty( "LineColor", "LineColor", uno::makeAny( sal_Int32( 0x000000) ) ) ); // black

    return aWrappedProperties;
}

Sequence< OUString > GridWrapper::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = "com.sun.star.chart.ChartGrid";
    aServices[ 1 ] = "com.sun.star.xml.UserDefinedAttributesSupplier";
    aServices[ 2 ] = "com.sun.star.drawing.LineProperties";
    aServices[ 3 ] = "com.sun.star.beans.PropertySet";

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( GridWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
