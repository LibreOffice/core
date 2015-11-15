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

#include "PolarCoordinateSystem.hxx"
#include "macros.hxx"
#include "servicenames_coosystems.hxx"
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

static const char lcl_aServiceNamePolar2d[] = "com.sun.star.chart2.PolarCoordinateSystem2d";
static const char lcl_aServiceNamePolar3d[] = "com.sun.star.chart2.PolarCoordinateSystem3d";

static const char lcl_aImplementationNamePolar2d[] = "com.sun.star.comp.chart2.PolarCoordinateSystem2d";
static const char lcl_aImplementationNamePolar3d[] = "com.sun.star.comp.chart2.PolarCoordinateSystem3d";

static const char CHART2_COOSYSTEM_POLAR_SERVICE_NAME[] = "com.sun.star.chart2.CoordinateSystems.Polar";

}

namespace chart
{

// explicit
PolarCoordinateSystem::PolarCoordinateSystem(
    const uno::Reference< uno::XComponentContext > & xContext,
    sal_Int32 nDimensionCount /* = 2 */,
    bool bSwapXAndYAxis /* = sal_False */ ) :
        BaseCoordinateSystem( xContext, nDimensionCount, bSwapXAndYAxis )
{}

PolarCoordinateSystem::PolarCoordinateSystem(
    const PolarCoordinateSystem & rSource ) :
        BaseCoordinateSystem( rSource )
{}

PolarCoordinateSystem::~PolarCoordinateSystem()
{}

// ____ XCoordinateSystem ____
OUString SAL_CALL PolarCoordinateSystem::getCoordinateSystemType()
    throw (RuntimeException, std::exception)
{
    return OUString(CHART2_COOSYSTEM_POLAR_SERVICE_NAME);
}

OUString SAL_CALL PolarCoordinateSystem::getViewServiceName()
    throw (RuntimeException, std::exception)
{
    return OUString(CHART2_COOSYSTEM_POLAR_VIEW_SERVICE_NAME);
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PolarCoordinateSystem::createClone()
    throw (RuntimeException, std::exception)
{
    return Reference< util::XCloneable >( new PolarCoordinateSystem( *this ));
}

// ____ XServiceInfo ____
Sequence< OUString > PolarCoordinateSystem::getSupportedServiceNames_Static()
{
    Sequence<OUString> aServices { CHART2_COOSYSTEM_POLAR_SERVICE_NAME };
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL PolarCoordinateSystem::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString PolarCoordinateSystem::getImplementationName_Static()
{
    return OUString("com.sun.star.comp.chart.PolarCoordinateSystem");
}

sal_Bool SAL_CALL PolarCoordinateSystem::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PolarCoordinateSystem::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

// ==== PolarCoordinateSystem2d ====

PolarCoordinateSystem2d::PolarCoordinateSystem2d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        PolarCoordinateSystem( xContext, 2, false )
{}

PolarCoordinateSystem2d::~PolarCoordinateSystem2d()
{}

// ____ XServiceInfo ____
Sequence< OUString > PolarCoordinateSystem2d::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
    aServices[ 1 ] = lcl_aServiceNamePolar2d;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL PolarCoordinateSystem2d::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString PolarCoordinateSystem2d::getImplementationName_Static()
{
    return OUString(lcl_aImplementationNamePolar2d) ;
}

sal_Bool SAL_CALL PolarCoordinateSystem2d::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PolarCoordinateSystem2d::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

// ==== PolarCoordinateSystem3d ====

PolarCoordinateSystem3d::PolarCoordinateSystem3d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        PolarCoordinateSystem( xContext, 3, false )
{}

PolarCoordinateSystem3d::~PolarCoordinateSystem3d()
{}

// ____ XServiceInfo ____
Sequence< OUString > PolarCoordinateSystem3d::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
    aServices[ 1 ] = lcl_aServiceNamePolar3d;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL PolarCoordinateSystem3d::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString PolarCoordinateSystem3d::getImplementationName_Static()
{
    return OUString(lcl_aImplementationNamePolar3d);
}

sal_Bool SAL_CALL PolarCoordinateSystem3d::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PolarCoordinateSystem3d::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

}  // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_PolarCoordinateSystem2d_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PolarCoordinateSystem2d(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_PolarCoordinateSystem3d_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PolarCoordinateSystem3d(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
