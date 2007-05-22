/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BarChartType.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:44:39 $
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
#include "BarChartType.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"

using namespace ::com::sun::star;

namespace chart
{

BarChartType::BarChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{}

BarChartType::BarChartType( const BarChartType & rOther ) :
        ChartType( rOther )
{
}

BarChartType::~BarChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL BarChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new BarChartType( *this ));
}

// ____ XChartType ____
::rtl::OUString SAL_CALL BarChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_BAR;
}

uno::Sequence< ::rtl::OUString > BarChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_BAR;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartType" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( BarChartType,
                             C2U( "com.sun.star.comp.chart.BarChartType" ));

} //  namespace chart
