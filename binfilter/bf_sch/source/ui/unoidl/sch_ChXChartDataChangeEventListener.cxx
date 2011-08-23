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

// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
// header for class OGuard
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include "ChXChartDataChangeEventListener.hxx"
#include "ChXChartDocument.hxx"
namespace binfilter {

#ifndef SCH_ASCII_TO_OU
#define SCH_ASCII_TO_OU( s )  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( s ) )
#endif

// using namespace osl;
using namespace rtl;
using namespace vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

ChXChartDataChangeEventListener::ChXChartDataChangeEventListener()
{
    mpXDoc = NULL;
}

void ChXChartDataChangeEventListener::SetOwner( ChXChartDocument* pXDoc ) throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    mpXDoc = pXDoc;
}

void ChXChartDataChangeEventListener::Reset() throw()
{
    OGuard aGuard( Application::GetSolarMutex() );
    mpXDoc = NULL;
}

// XChartDataChangeEventListener
void SAL_CALL ChXChartDataChangeEventListener::chartDataChanged( const chart::ChartDataChangeEvent& aEvent )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );
    if( mpXDoc )
        mpXDoc->RefreshData( aEvent );
}

// XEventListener
void SAL_CALL ChXChartDataChangeEventListener::disposing( const lang::EventObject& Source ) throw( uno::RuntimeException )
{
    Reset();
}

// XServiceInfo
::rtl::OUString SAL_CALL ChXChartDataChangeEventListener::getImplementationName() throw( uno::RuntimeException )
{
    return SCH_ASCII_TO_OU( "ChXChartDataChangeEventListener" );
}

sal_Bool SAL_CALL ChXChartDataChangeEventListener::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;	
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXChartDataChangeEventListener::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Sequence< OUString > aServSeq( 0 );
    return aServSeq;
}

}
