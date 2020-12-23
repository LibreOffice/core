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

#include <PolarCoordinateSystem.hxx>
#include <servicenames_coosystems.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

const OUStringLiteral CHART2_COOSYSTEM_POLAR_SERVICE_NAME
    = u"com.sun.star.chart2.CoordinateSystems.Polar";

}

namespace chart
{

// explicit
PolarCoordinateSystem::PolarCoordinateSystem( sal_Int32 nDimensionCount /* = 2 */ ) :
        BaseCoordinateSystem( nDimensionCount )
{}

PolarCoordinateSystem::PolarCoordinateSystem(
    const PolarCoordinateSystem & rSource ) :
        BaseCoordinateSystem( rSource )
{}

PolarCoordinateSystem::~PolarCoordinateSystem()
{}

// ____ XCoordinateSystem ____
OUString SAL_CALL PolarCoordinateSystem::getCoordinateSystemType()
{
    return CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
}

OUString SAL_CALL PolarCoordinateSystem::getViewServiceName()
{
    return CHART2_COOSYSTEM_POLAR_VIEW_SERVICE_NAME;
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PolarCoordinateSystem::createClone()
{
    return Reference< util::XCloneable >( new PolarCoordinateSystem( *this ));
}

// ____ XServiceInfo ____
OUString SAL_CALL PolarCoordinateSystem::getImplementationName()
{
    return "com.sun.star.comp.chart.PolarCoordinateSystem";
}

sal_Bool SAL_CALL PolarCoordinateSystem::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PolarCoordinateSystem::getSupportedServiceNames()
{
    return { CHART2_COOSYSTEM_POLAR_SERVICE_NAME };
}

// ==== PolarCoordinateSystem2d ====

PolarCoordinateSystem2d::PolarCoordinateSystem2d() :
        PolarCoordinateSystem( 2 )
{}

PolarCoordinateSystem2d::~PolarCoordinateSystem2d()
{}

// ____ XServiceInfo ____
OUString SAL_CALL PolarCoordinateSystem2d::getImplementationName()
{
    return "com.sun.star.comp.chart2.PolarCoordinateSystem2d" ;
}

sal_Bool SAL_CALL PolarCoordinateSystem2d::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PolarCoordinateSystem2d::getSupportedServiceNames()
{
    return {
        CHART2_COOSYSTEM_POLAR_SERVICE_NAME,
        "com.sun.star.chart2.PolarCoordinateSystem2d" };
}

// ==== PolarCoordinateSystem3d ====

PolarCoordinateSystem3d::PolarCoordinateSystem3d() :
        PolarCoordinateSystem( 3 )
{}

PolarCoordinateSystem3d::~PolarCoordinateSystem3d()
{}

// ____ XServiceInfo ____
OUString SAL_CALL PolarCoordinateSystem3d::getImplementationName()
{
    return "com.sun.star.comp.chart2.PolarCoordinateSystem3d";
}

sal_Bool SAL_CALL PolarCoordinateSystem3d::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PolarCoordinateSystem3d::getSupportedServiceNames()
{
    return {
        CHART2_COOSYSTEM_POLAR_SERVICE_NAME,
        "com.sun.star.chart2.PolarCoordinateSystem3d" };
}

}  // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_PolarCoordinateSystem2d_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PolarCoordinateSystem2d);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_PolarCoordinateSystem3d_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PolarCoordinateSystem3d);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
