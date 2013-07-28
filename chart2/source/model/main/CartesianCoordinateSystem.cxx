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

using namespace ::com::sun::star;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

static const OUString lcl_aServiceNameCartesian2d( "com.sun.star.chart2.CartesianCoordinateSystem2d" );
static const OUString lcl_aServiceNameCartesian3d( "com.sun.star.chart2.CartesianCoordinateSystem3d" );

static const OUString lcl_aImplementationNameCartesian2d( "com.sun.star.comp.chart2.CartesianCoordinateSystem2d" );
static const OUString lcl_aImplementationNameCartesian3d( "com.sun.star.comp.chart2.CartesianCoordinateSystem3d" );
}

namespace chart
{

// explicit
CartesianCoordinateSystem::CartesianCoordinateSystem(
    const uno::Reference< uno::XComponentContext > & xContext,
    sal_Int32 nDimensionCount /* = 2 */,
    sal_Bool bSwapXAndYAxis /* = sal_False */ ) :
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
    throw (RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME;
}

OUString SAL_CALL CartesianCoordinateSystem::getViewServiceName()
    throw (RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN_VIEW_SERVICE_NAME;
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL CartesianCoordinateSystem::createClone()
    throw (RuntimeException)
{
    return Reference< util::XCloneable >( new CartesianCoordinateSystem( *this ));
}

// ____ XServiceInfo ____
Sequence< OUString > CartesianCoordinateSystem::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( CartesianCoordinateSystem,
                             OUString("com.sun.star.comp.chart.CartesianCoordinateSystem"))

// ==== CartesianCoordinateSystem2d ====

CartesianCoordinateSystem2d::CartesianCoordinateSystem2d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        CartesianCoordinateSystem( xContext, 2, sal_False )
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
APPHELPER_XSERVICEINFO_IMPL( CartesianCoordinateSystem2d, lcl_aImplementationNameCartesian2d )

// ==== CartesianCoordinateSystem3d ====

CartesianCoordinateSystem3d::CartesianCoordinateSystem3d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        CartesianCoordinateSystem( xContext, 3, sal_False )
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
APPHELPER_XSERVICEINFO_IMPL( CartesianCoordinateSystem3d, lcl_aImplementationNameCartesian3d )

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
