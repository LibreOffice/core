/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CartesianCoordinateSystem.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:31:53 $
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
#include "CartesianCoordinateSystem.hxx"
#include "macros.hxx"
#include "servicenames_coosystems.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{

static const ::rtl::OUString lcl_aServiceNameCartesian2d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.CartesianCoordinateSystem2d" ));
static const ::rtl::OUString lcl_aServiceNameCartesian3d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.CartesianCoordinateSystem3d" ));

static const ::rtl::OUString lcl_aImplementationNameCartesian2d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.CartesianCoordinateSystem2d" ));
static const ::rtl::OUString lcl_aImplementationNameCartesian3d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.CartesianCoordinateSystem3d" ));
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
::rtl::OUString SAL_CALL CartesianCoordinateSystem::getCoordinateSystemType()
    throw (RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME;
}

::rtl::OUString SAL_CALL CartesianCoordinateSystem::getViewServiceName()
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
                             C2U( "com.sun.star.comp.chart.CartesianCoordinateSystem" ))


// =================================
// ==== CartesianCoordinateSystem2d ====
// =================================

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

// =================================
// ==== CartesianCoordinateSystem3d ====
// =================================

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
