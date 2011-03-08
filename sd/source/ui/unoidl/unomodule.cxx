/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

// System - Includes -----------------------------------------------------
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include "sdmod.hxx"
#include "unomodule.hxx"
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

::rtl::OUString SAL_CALL SdUnoModule_getImplementationName() throw( uno::RuntimeException )
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.DrawingModule" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL SdUnoModule_getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSeq( 1 );
    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ModuleDispatcher"));
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SdUnoModule_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
{
    SolarMutexGuard aGuard;
    return uno::Reference< uno::XInterface >( static_cast< cppu::OWeakObject* >( new SdUnoModule( rSMgr ) ) );
}

    // XNotifyingDispatch
void SAL_CALL SdUnoModule::dispatchWithNotification( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    // there is no guarantee, that we are holded alive during this method!
    // May the outside dispatch container will be updated by a CONTEXT_CHANGED
    // asynchronous ...
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xThis(static_cast< ::com::sun::star::frame::XNotifyingDispatch* >(this));

    SolarMutexGuard aGuard;
    SdDLL::Init();
    const SfxSlot* pSlot = SD_MOD()->GetInterface()->GetSlot( aURL.Complete );

    sal_Int16 aState = ::com::sun::star::frame::DispatchResultState::DONTKNOW;
    if ( !pSlot )
        aState = ::com::sun::star::frame::DispatchResultState::FAILURE;
    else
    {
        SfxRequest aReq( pSlot, aArgs, SFX_CALLMODE_SYNCHRON, SD_MOD()->GetPool() );
        const SfxPoolItem* pResult = SD_MOD()->ExecuteSlot( aReq );
        if ( pResult )
            aState = ::com::sun::star::frame::DispatchResultState::SUCCESS;
        else
            aState = ::com::sun::star::frame::DispatchResultState::FAILURE;
    }

    if ( xListener.is() )
    {
        xListener->dispatchFinished(
            ::com::sun::star::frame::DispatchResultEvent(
                    xThis, aState, ::com::sun::star::uno::Any()));
    }
}
    // XDispatch
void SAL_CALL SdUnoModule::dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw( ::com::sun::star::uno::RuntimeException )
{
    dispatchWithNotification(aURL, aArgs, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >());
}

void SAL_CALL SdUnoModule::addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > &, const ::com::sun::star::util::URL&) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL SdUnoModule::removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > &, const ::com::sun::star::util::URL&) throw( ::com::sun::star::uno::RuntimeException )
{
}

SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL SdUnoModule::queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescripts ) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Int32 nCount = seqDescripts.getLength();
    SEQUENCE< REFERENCE< XDISPATCH > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL  ,
                                        seqDescripts[i].FrameName   ,
                                        seqDescripts[i].SearchFlags );
    }

    return lDispatcher;
}

// XDispatchProvider
REFERENCE< XDISPATCH > SAL_CALL SdUnoModule::queryDispatch( const UNOURL& aURL, const OUSTRING&, sal_Int32 ) throw( RUNTIMEEXCEPTION )
{
    SolarMutexGuard aGuard;
    SdDLL::Init();
    const SfxSlot* pSlot = SD_MOD()->GetInterface()->GetSlot( aURL.Complete );

    REFERENCE< XDISPATCH > xSlot;
    if ( pSlot )
        xSlot = this;

    return xSlot;
}

// XServiceInfo
::rtl::OUString SAL_CALL SdUnoModule::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return SdUnoModule_getImplementationName();
}

sal_Bool SAL_CALL SdUnoModule::supportsService( const ::rtl::OUString& sServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames =   getSupportedServiceNames();
    const UNOOUSTRING*          pArray          =   seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SdUnoModule::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return SdUnoModule_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
