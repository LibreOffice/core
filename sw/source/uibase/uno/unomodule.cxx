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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include "swmodule.hxx"
#include "swdll.hxx"
#include "unofreg.hxx"
#include "unomodule.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace css;

OUString SAL_CALL SwUnoModule_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Writer.WriterModule" );
}

uno::Sequence< OUString > SAL_CALL SwUnoModule_getSupportedServiceNames() throw()
{
    uno::Sequence< OUString > aSeq( 1 );
    aSeq[0] = "com.sun.star.text.ModuleDispatcher";
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SwUnoModule_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
    throw (css::uno::Exception)
{
    SolarMutexGuard aGuard;
    return uno::Reference< uno::XInterface >( dynamic_cast< frame::XDispatch * >(new SwUnoModule( rSMgr )), uno::UNO_QUERY );
}

    // XNotifyingDispatch
void SAL_CALL SwUnoModule::dispatchWithNotification( const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs, const uno::Reference< frame::XDispatchResultListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    // there is no guarantee, that we are holded alive during this method!
    // May the outside dispatch container will be updated by a CONTEXT_CHANGED
    // asynchronous ...
    uno::Reference< uno::XInterface > xThis(static_cast< frame::XNotifyingDispatch* >(this));

    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    const SfxSlot* pSlot = SW_MOD()->GetInterface()->GetSlot( aURL.Complete );

    sal_Int16 aState = frame::DispatchResultState::DONTKNOW;
    if ( !pSlot )
        aState = frame::DispatchResultState::FAILURE;
    else
    {
        SfxRequest aReq( pSlot, aArgs, SfxCallMode::SYNCHRON, SW_MOD()->GetPool() );
        const SfxPoolItem* pResult = SW_MOD()->ExecuteSlot( aReq );
        if ( pResult )
            aState = frame::DispatchResultState::SUCCESS;
        else
            aState = frame::DispatchResultState::FAILURE;
    }

    if ( xListener.is() )
    {
        xListener->dispatchFinished(
            frame::DispatchResultEvent(
                    xThis, aState, uno::Any()));
    }
}

    // XDispatch
void SAL_CALL SwUnoModule::dispatch( const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs ) throw( uno::RuntimeException, std::exception )
{
    dispatchWithNotification(aURL, aArgs, uno::Reference< frame::XDispatchResultListener >());
}

void SAL_CALL SwUnoModule::addStatusListener(
    const uno::Reference< frame::XStatusListener > & /*xControl*/,
    const util::URL& /*aURL*/)
    throw( uno::RuntimeException, std::exception )
{
}

void SAL_CALL SwUnoModule::removeStatusListener(
    const uno::Reference< frame::XStatusListener > & /*xControl*/,
    const util::URL& /*aURL*/) throw( uno::RuntimeException, std::exception )
{
}

uno::Sequence< uno::Reference< frame::XDispatch > > SAL_CALL SwUnoModule::queryDispatches(
    const uno::Sequence< frame::DispatchDescriptor >& seqDescripts ) throw( uno::RuntimeException, std::exception )
{
    sal_Int32 nCount = seqDescripts.getLength();
    uno::Sequence< uno::Reference< frame::XDispatch > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL  ,
                                        seqDescripts[i].FrameName   ,
                                        seqDescripts[i].SearchFlags );
    }

    return lDispatcher;
}

// XDispatchProvider
uno::Reference< frame::XDispatch > SAL_CALL SwUnoModule::queryDispatch(
    const util::URL& aURL, const OUString& /*sTargetFrameName*/,
    sal_Int32 /*eSearchFlags*/    ) throw( uno::RuntimeException, std::exception )
{
    uno::Reference< frame::XDispatch > xReturn;

    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    const SfxSlot* pSlot = SW_MOD()->GetInterface()->GetSlot( aURL.Complete );
    if ( pSlot )
        xReturn.set(static_cast< frame::XDispatch* >(this), uno::UNO_QUERY);

    return xReturn;
}

// XServiceInfo
OUString SAL_CALL SwUnoModule::getImplementationName(  ) throw(uno::RuntimeException, std::exception)
{
    return SwUnoModule_getImplementationName();
}

sal_Bool SAL_CALL SwUnoModule::supportsService( const OUString& sServiceName ) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString > SAL_CALL SwUnoModule::getSupportedServiceNames(  ) throw(uno::RuntimeException, std::exception)
{
    return SwUnoModule_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
