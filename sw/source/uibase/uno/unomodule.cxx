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

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include <swmodule.hxx>
#include <swdll.hxx>
#include "unomodule.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/frame.hxx>
#include <vcl/svapp.hxx>

using namespace css;

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Writer_WriterModule_get_implementation(uno::XComponentContext* /*pCtx*/,
                                                         uno::Sequence<uno::Any> const& /*rSeq*/)
{
    SolarMutexGuard aGuard;
    return cppu::acquire(new SwUnoModule);
}

    // XNotifyingDispatch
void SAL_CALL SwUnoModule::dispatchWithNotification( const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs, const uno::Reference< frame::XDispatchResultListener >& xListener )
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
        SfxAllItemSet aInternalSet( SfxGetpApp()->GetPool() );

        css::uno::Reference<css::frame::XDesktop2> xDesktop = css::frame::Desktop::create(::comphelper::getProcessComponentContext());
        css::uno::Reference<css::frame::XFrame> xCurrentFrame = xDesktop->getCurrentFrame();
        if (xCurrentFrame.is()) // an empty set is no problem ... but an empty frame reference can be a problem !
            aInternalSet.Put(SfxUnoFrameItem(SID_FILLFRAME, xCurrentFrame));

        aReq.SetInternalArgs_Impl(aInternalSet);
        const SfxPoolItemHolder& rResult(SW_MOD()->ExecuteSlot(aReq));
        if (rResult)
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
void SAL_CALL SwUnoModule::dispatch( const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs )
{
    dispatchWithNotification(aURL, aArgs, uno::Reference< frame::XDispatchResultListener >());
}

void SAL_CALL SwUnoModule::addStatusListener(
    const uno::Reference< frame::XStatusListener > & /*xControl*/,
    const util::URL& /*aURL*/)
{
}

void SAL_CALL SwUnoModule::removeStatusListener(
    const uno::Reference< frame::XStatusListener > & /*xControl*/,
    const util::URL& /*aURL*/)
{
}

uno::Sequence< uno::Reference< frame::XDispatch > > SAL_CALL SwUnoModule::queryDispatches(
    const uno::Sequence< frame::DispatchDescriptor >& seqDescripts )
{
    sal_Int32 nCount = seqDescripts.getLength();
    uno::Sequence< uno::Reference< frame::XDispatch > > lDispatcher( nCount );

    std::transform(seqDescripts.begin(), seqDescripts.end(), lDispatcher.getArray(),
        [this](const frame::DispatchDescriptor& rDescr) -> uno::Reference< frame::XDispatch > {
            return queryDispatch( rDescr.FeatureURL, rDescr.FrameName, rDescr.SearchFlags ); });

    return lDispatcher;
}

// XDispatchProvider
uno::Reference< frame::XDispatch > SAL_CALL SwUnoModule::queryDispatch(
    const util::URL& aURL, const OUString& /*sTargetFrameName*/,
    sal_Int32 /*eSearchFlags*/    )
{
    uno::Reference< frame::XDispatch > xReturn;

    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    const SfxSlot* pSlot = SW_MOD()->GetInterface()->GetSlot( aURL.Complete );
    if ( pSlot )
        xReturn = this;

    return xReturn;
}

// XServiceInfo
OUString SAL_CALL SwUnoModule::getImplementationName(  )
{
    return u"com.sun.star.comp.Writer.WriterModule"_ustr;
}

sal_Bool SAL_CALL SwUnoModule::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString > SAL_CALL SwUnoModule::getSupportedServiceNames(  )
{
    uno::Sequence<OUString> aSeq { u"com.sun.star.text.ModuleDispatcher"_ustr };
    return aSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
