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
#include <AxisHelper.hxx>
#include "Chart2ModelContact.hxx"
#include <AxisIndexDefines.hxx>
#include <BaseCoordinateSystem.hxx>

#include <LinePropertiesHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <WrappedDefaultProperty.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <algorithm>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart::wrapper
{

GridWrapper::GridWrapper(tGridType eType, std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
    : m_spChart2ModelContact(std::move(spChart2ModelContact))
    , m_eType(eType)
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
{
    std::unique_lock g(m_aMutex);
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( g, lang::EventObject( xSource ) );

    clearWrappedPropertySet();
}

void SAL_CALL GridWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, xListener );
}

void SAL_CALL GridWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

// WrappedPropertySet

Reference< beans::XPropertySet > GridWrapper::getInnerPropertySet()
{
    Reference< beans::XPropertySet > xRet;
    try
    {
        rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
        rtl::Reference< BaseCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 /*nCooSysIndex*/ ) );

        sal_Int32 nDimensionIndex = 1;
        bool bSubGrid = false;
        getDimensionAndSubGridBool( m_eType, nDimensionIndex, bSubGrid );

        sal_Int32 nSubGridIndex = bSubGrid ? 0 : -1;
        xRet = AxisHelper::getGridProperties( xCooSys , nDimensionIndex, MAIN_AXIS_INDEX, nSubGridIndex );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return xRet;
}

const Sequence< beans::Property >& GridWrapper::getPropertySequence()
{
    static Sequence< Property > aPropSeq = []()
        {
            std::vector< css::beans::Property > aProperties;
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropSeq;
}

std::vector< std::unique_ptr<WrappedProperty> > GridWrapper::createWrappedProperties()
{
    std::vector< std::unique_ptr<WrappedProperty> > aWrappedProperties;

    aWrappedProperties.emplace_back( new WrappedDefaultProperty( "LineColor", "LineColor", uno::Any( sal_Int32( 0x000000) ) ) ); // black

    return aWrappedProperties;
}

OUString SAL_CALL GridWrapper::getImplementationName()
{
    return "com.sun.star.comp.chart.Grid";
}

sal_Bool SAL_CALL GridWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL GridWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartGrid",
        "com.sun.star.xml.UserDefinedAttributesSupplier",
        "com.sun.star.drawing.LineProperties",
        "com.sun.star.beans.PropertySet"
    };
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
