/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PolarCoordinateSystem.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:40:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "PolarCoordinateSystem.hxx"
#include "macros.hxx"
#include "servicenames_coosystems.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{

static const ::rtl::OUString lcl_aServiceNamePolar2d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.PolarCoordinateSystem2d" ));
static const ::rtl::OUString lcl_aServiceNamePolar3d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.PolarCoordinateSystem3d" ));

static const ::rtl::OUString lcl_aImplementationNamePolar2d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.PolarCoordinateSystem2d" ));
static const ::rtl::OUString lcl_aImplementationNamePolar3d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.PolarCoordinateSystem3d" ));
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
::rtl::OUString SAL_CALL PolarCoordinateSystem::getCoordinateSystemType()
    throw (RuntimeException)
{
    return CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
}

::rtl::OUString SAL_CALL PolarCoordinateSystem::getViewServiceName()
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
                             C2U( "com.sun.star.comp.chart.PolarCoordinateSystem" ))


// =================================
// ==== PolarCoordinateSystem2d ====
// =================================

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

// =================================
// ==== PolarCoordinateSystem3d ====
// =================================

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
