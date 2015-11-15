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

#include "CartesianCoordinateSystem.hxx"
#include "macros.hxx"
#include "servicenames_coosystems.hxx"
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

static const char lcl_aServiceNameCartesian2d[] = "com.sun.star.chart2.CartesianCoordinateSystem2d";
static const char lcl_aServiceNameCartesian3d[] = "com.sun.star.chart2.CartesianCoordinateSystem3d";

static const char lcl_aImplementationNameCartesian2d[] = "com.sun.star.comp.chart2.CartesianCoordinateSystem2d";
static const char lcl_aImplementationNameCartesian3d[] = "com.sun.star.comp.chart2.CartesianCoordinateSystem3d";

static const char CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME[] = "com.sun.star.chart2.CoordinateSystems.Cartesian";

}

namespace chart
{

// explicit
CartesianCoordinateSystem::CartesianCoordinateSystem(
    const uno::Reference< uno::XComponentContext > & xContext,
    sal_Int32 nDimensionCount /* = 2 */,
    bool bSwapXAndYAxis /* = sal_False */ ) :
        BaseCoordinateSystem( xContext, nDimensionCount, bSwapXAndYAxis )
{}

CartesianCoordinateSystem::CartesianCoordinateSystem(
    const CartesianCoordinateSystem & rSource ) :
        BaseCoordinateSystem( rSource )
{}

CartesianCoordinateSystem::~CartesianCoordinateSystem()
{}

// ____ XCoordinateSystem ____
OUString SAL_CALL CartesianCoordinateSystem::getCoordinateSystemType()
    throw (RuntimeException, std::exception)
{
    return OUString(CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME);
}

OUString SAL_CALL CartesianCoordinateSystem::getViewServiceName()
    throw (RuntimeException, std::exception)
{
    return OUString(CHART2_COOSYSTEM_CARTESIAN_VIEW_SERVICE_NAME);
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL CartesianCoordinateSystem::createClone()
    throw (RuntimeException, std::exception)
{
    return Reference< util::XCloneable >( new CartesianCoordinateSystem( *this ));
}

// ____ XServiceInfo ____
Sequence< OUString > CartesianCoordinateSystem::getSupportedServiceNames_Static()
{
    Sequence<OUString> aServices { CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME };
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL CartesianCoordinateSystem::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString CartesianCoordinateSystem::getImplementationName_Static()
{
    return OUString("com.sun.star.comp.chart.CartesianCoordinateSystem");
}

sal_Bool SAL_CALL CartesianCoordinateSystem::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CartesianCoordinateSystem::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

// ==== CartesianCoordinateSystem2d ====

CartesianCoordinateSystem2d::CartesianCoordinateSystem2d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        CartesianCoordinateSystem( xContext, 2, false )
{}

CartesianCoordinateSystem2d::~CartesianCoordinateSystem2d()
{}

// ____ XServiceInfo ____
Sequence< OUString > CartesianCoordinateSystem2d::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME;
    aServices[ 1 ] = lcl_aServiceNameCartesian2d;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL CartesianCoordinateSystem2d::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString CartesianCoordinateSystem2d::getImplementationName_Static()
{
    return OUString(lcl_aImplementationNameCartesian2d);
}

sal_Bool SAL_CALL CartesianCoordinateSystem2d::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CartesianCoordinateSystem2d::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

// ==== CartesianCoordinateSystem3d ====

CartesianCoordinateSystem3d::CartesianCoordinateSystem3d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        CartesianCoordinateSystem( xContext, 3, false )
{}

CartesianCoordinateSystem3d::~CartesianCoordinateSystem3d()
{}

// ____ XServiceInfo ____
Sequence< OUString > CartesianCoordinateSystem3d::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME;
    aServices[ 1 ] = lcl_aServiceNameCartesian3d;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL CartesianCoordinateSystem3d::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString CartesianCoordinateSystem3d::getImplementationName_Static()
{
    return OUString(lcl_aImplementationNameCartesian3d);
}

sal_Bool SAL_CALL CartesianCoordinateSystem3d::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CartesianCoordinateSystem3d::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

}  // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_CartesianCoordinateSystem2d_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::CartesianCoordinateSystem2d(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_CartesianCoordinateSystem3d_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::CartesianCoordinateSystem3d(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
