/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PolarCoordinateSystem.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:03:19 $
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
#include "PolarCoordinateSystem.hxx"
#include "PolarTransformation.hxx"
#include "macros.hxx"
#include "servicenames_coosystems.hxx"

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

PolarCoordinateSystem::PolarCoordinateSystem( sal_Int32 nDim ) :
        m_aTransformationToCartesian(
            new PolarTransformation( PolarTransformation::TRANSFORM_FROM, nDim ) ),
        m_aTransformationFromCartesian(
            new PolarTransformation( PolarTransformation::TRANSFORM_TO, nDim ) ),
        m_nDim( nDim )
{
}

PolarCoordinateSystem::~PolarCoordinateSystem()
{}

// ____ XCoordinateSystem ____
::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation > SAL_CALL
    PolarCoordinateSystem::getTransformationToCartesian()
    throw (RuntimeException)
{
    return m_aTransformationToCartesian;
}

::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation > SAL_CALL
    PolarCoordinateSystem::getTransformationFromCartesian()
    throw (RuntimeException)
{
    return m_aTransformationFromCartesian;
}

sal_Int32 SAL_CALL PolarCoordinateSystem::getDimension() throw (RuntimeException)
{
    return m_nDim;
}

::rtl::OUString SAL_CALL PolarCoordinateSystem::getCoordinateSystemType() throw (RuntimeException)
{
    return CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
}

::rtl::OUString SAL_CALL PolarCoordinateSystem::getViewServiceName() throw (RuntimeException)
{
    return CHART2_COOSYSTEM_POLAR_VIEW_SERVICE_NAME;
}

// ____ XServiceInfo ____

Sequence< OUString > PolarCoordinateSystem::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices;
//     Sequence< OUString > aServices( 1 );
//     aServices[ 0 ] = C2U( "com.sun.star.chart2.CoordinateSystem" );
    return aServices;
}

APPHELPER_XSERVICEINFO_IMPL( PolarCoordinateSystem,
                             C2U( "com.sun.star.comp.chart.PolarCoordinateSystem" ));

}  // namespace chart
