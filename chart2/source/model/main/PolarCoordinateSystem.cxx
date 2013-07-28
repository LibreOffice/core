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

using namespace ::com::sun::star;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

static const OUString lcl_aServiceNamePolar2d( "com.sun.star.chart2.PolarCoordinateSystem2d" );
static const OUString lcl_aServiceNamePolar3d( "com.sun.star.chart2.PolarCoordinateSystem3d" );

static const OUString lcl_aImplementationNamePolar2d( "com.sun.star.comp.chart2.PolarCoordinateSystem2d" );
static const OUString lcl_aImplementationNamePolar3d( "com.sun.star.comp.chart2.PolarCoordinateSystem3d" );
}

namespace chart
{

// explicit
PolarCoordinateSystem::PolarCoordinateSystem(
    const uno::Reference< uno::XComponentContext > & xContext,
    sal_Int32 nDimensionCount /* = 2 */,
    sal_Bool bSwapXAndYAxis /* = sal_False */ ) :
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
    throw (RuntimeException)
{
    return CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
}

OUString SAL_CALL PolarCoordinateSystem::getViewServiceName()
    throw (RuntimeException)
{
    return CHART2_COOSYSTEM_POLAR_VIEW_SERVICE_NAME;
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PolarCoordinateSystem::createClone()
    throw (RuntimeException)
{
    return Reference< util::XCloneable >( new PolarCoordinateSystem( *this ));
}

// ____ XServiceInfo ____
Sequence< OUString > PolarCoordinateSystem::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PolarCoordinateSystem,
                             OUString("com.sun.star.comp.chart.PolarCoordinateSystem") )

// ==== PolarCoordinateSystem2d ====

PolarCoordinateSystem2d::PolarCoordinateSystem2d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        PolarCoordinateSystem( xContext, 2, sal_False )
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
APPHELPER_XSERVICEINFO_IMPL( PolarCoordinateSystem2d, lcl_aImplementationNamePolar2d )

// ==== PolarCoordinateSystem3d ====

PolarCoordinateSystem3d::PolarCoordinateSystem3d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        PolarCoordinateSystem( xContext, 3, sal_False )
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
APPHELPER_XSERVICEINFO_IMPL( PolarCoordinateSystem3d, lcl_aImplementationNamePolar3d )

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
