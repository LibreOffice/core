/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "ChartLegend.hxx"
#include "mapprov.hxx"
#include "defines.hxx"

// header for class OGuard
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
namespace binfilter {

using namespace ::com::sun::star;

ChartLegend::ChartLegend( ChartModel* pModel ) :
        ChXChartObject( CHMAP_LEGEND, pModel, CHOBJID_LEGEND )
{
}

ChartLegend::~ChartLegend()
{}

// XServiceInfo
::rtl::OUString SAL_CALL ChartLegend::getImplementationName()
    throw( uno::RuntimeException )
{
    return ::rtl::OUString::createFromAscii( "ChartLegend" );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChartLegend::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    uno::Sequence< ::rtl::OUString > aSeq( 6 );
    aSeq[ 0 ] = ::rtl::OUString::createFromAscii( "com.sun.star.chart.ChartLegend" );
    aSeq[ 1 ] = ::rtl::OUString::createFromAscii( "com.sun.star.style.CharacterProperties" );
    aSeq[ 2 ] = ::rtl::OUString::createFromAscii( "com.sun.star.drawing.FillProperties" );
    aSeq[ 3 ] = ::rtl::OUString::createFromAscii( "com.sun.star.drawing.LineProperties" );
    aSeq[ 4 ] = ::rtl::OUString::createFromAscii( "com.sun.star.drawing.Shape" );
    aSeq[ 5 ] = ::rtl::OUString::createFromAscii( "com.sun.star.xml.UserDefinedAttributeSupplier" );

    return aSeq;
}

// XTypeProvider
uno::Sequence< sal_Int8 > SAL_CALL ChartLegend::getImplementationId()
    throw( uno::RuntimeException )
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XShapeDescriptor
::rtl::OUString SAL_CALL ChartLegend::getShapeType()
    throw( uno::RuntimeException )
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.chart.ChartLegend" );
}

// XUnoTunnel
sal_Int64 SAL_CALL ChartLegend::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw( uno::RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                                 aIdentifier.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return ChXChartObject::getSomething( aIdentifier );
}

// helpers for XUnoTunnel 
const uno::Sequence< sal_Int8 > & ChartLegend::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}
}
