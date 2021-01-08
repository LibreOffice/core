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

#include <CartesianCoordinateSystem.hxx>
#include <servicenames_coosystems.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace com::sun::star::uno { class XComponentContext; }

namespace
{

constexpr OUStringLiteral CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME = u"com.sun.star.chart2.CoordinateSystems.Cartesian";

}

namespace chart
{

// explicit
CartesianCoordinateSystem::CartesianCoordinateSystem(
    sal_Int32 nDimensionCount /* = 2 */ ) :
        BaseCoordinateSystem( nDimensionCount )
{}

CartesianCoordinateSystem::CartesianCoordinateSystem(
    const CartesianCoordinateSystem & rSource ) :
        BaseCoordinateSystem( rSource )
{}

CartesianCoordinateSystem::~CartesianCoordinateSystem()
{}

// ____ XCoordinateSystem ____
OUString SAL_CALL CartesianCoordinateSystem::getCoordinateSystemType()
{
    return CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME;
}

OUString SAL_CALL CartesianCoordinateSystem::getViewServiceName()
{
    return CHART2_COOSYSTEM_CARTESIAN_VIEW_SERVICE_NAME;
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL CartesianCoordinateSystem::createClone()
{
    return Reference< util::XCloneable >( new CartesianCoordinateSystem( *this ));
}

// ____ XServiceInfo ____
OUString SAL_CALL CartesianCoordinateSystem::getImplementationName()
{
    return "com.sun.star.comp.chart.CartesianCoordinateSystem";
}

sal_Bool SAL_CALL CartesianCoordinateSystem::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CartesianCoordinateSystem::getSupportedServiceNames()
{
    return { CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME };
}

// ==== CartesianCoordinateSystem2d ====

CartesianCoordinateSystem2d::CartesianCoordinateSystem2d() :
        CartesianCoordinateSystem( 2 )
{}

CartesianCoordinateSystem2d::~CartesianCoordinateSystem2d()
{}

// ____ XServiceInfo ____
OUString SAL_CALL CartesianCoordinateSystem2d::getImplementationName()
{
    return "com.sun.star.comp.chart2.CartesianCoordinateSystem2d";
}

sal_Bool SAL_CALL CartesianCoordinateSystem2d::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CartesianCoordinateSystem2d::getSupportedServiceNames()
{
    return {
        CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME,
        "com.sun.star.chart2.CartesianCoordinateSystem2d"
    };
}

// ==== CartesianCoordinateSystem3d ====

CartesianCoordinateSystem3d::CartesianCoordinateSystem3d() :
        CartesianCoordinateSystem( 3 )
{}

CartesianCoordinateSystem3d::~CartesianCoordinateSystem3d()
{}

// ____ XServiceInfo ____
OUString SAL_CALL CartesianCoordinateSystem3d::getImplementationName()
{
    return "com.sun.star.comp.chart2.CartesianCoordinateSystem3d";
}

sal_Bool SAL_CALL CartesianCoordinateSystem3d::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CartesianCoordinateSystem3d::getSupportedServiceNames()
{
    return {
        CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME,
        "com.sun.star.chart2.CartesianCoordinateSystem3d"
    };
}

}  // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_CartesianCoordinateSystem2d_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::CartesianCoordinateSystem2d);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_CartesianCoordinateSystem3d_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::CartesianCoordinateSystem3d);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
