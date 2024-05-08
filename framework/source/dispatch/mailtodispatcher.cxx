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

#include <dispatch/mailtodispatcher.hxx>
#include <services.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <utility>

namespace framework{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL MailToDispatcher::getImplementationName()
{
    return u"com.sun.star.comp.framework.MailToDispatcher"_ustr;
}

sal_Bool SAL_CALL MailToDispatcher::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL MailToDispatcher::getSupportedServiceNames()
{
    return { SERVICENAME_PROTOCOLHANDLER };
}


/**
    @short      standard ctor
    @descr      This initializes a new instance of this class with needed information for work.

    @param      rxContext
                    reference to uno servicemanager for creation of new services
*/
MailToDispatcher::MailToDispatcher( css::uno::Reference< css::uno::XComponentContext > xContext )
        : m_xContext    (std::move( xContext                     ))
{
}

/**
    @short      standard dtor
*/
MailToDispatcher::~MailToDispatcher()
{
}

/**
    @short      decide if this dispatch implementation can be used for requested URL or not
    @descr      A protocol handler is registered for a URL pattern inside configuration and will
                be asked by the generic dispatch mechanism inside framework, if he can handle this
                special URL which match his registration. He can agree by returning of a valid dispatch
                instance or disagree by returning <NULL/>.
                We don't create new dispatch instances here really - we return THIS as result to handle it
                at the same implementation.
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL MailToDispatcher::queryDispatch( const css::util::URL&  aURL    ,
                                                                                       const OUString& /*sTarget*/ ,
                                                                                             sal_Int32        /*nFlags*/  )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.startsWith("mailto:"))
        xDispatcher = this;
    return xDispatcher;
}

/**
    @short      do the same like dispatch() but for multiple requests at the same time
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL MailToDispatcher::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor )
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    auto lDispatcherRange = asNonConstRange(lDispatcher);
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcherRange[i] = queryDispatch(
                            lDescriptor[i].FeatureURL,
                            lDescriptor[i].FrameName,
                            lDescriptor[i].SearchFlags);
    }
    return lDispatcher;
}

/**
    @short      dispatch URL with arguments
    @descr      We use threadsafe internal method to do so. It returns a state value - but we ignore it.
                Because we don't support status listener notifications here. Status events are not guaranteed -
                and we call another service internally which doesn't return any notifications too.

    @param      aURL
                    mail URL which should be executed
    @param      lArguments
                    list of optional arguments for this mail request
*/
void SAL_CALL MailToDispatcher::dispatch( const css::util::URL&                                  aURL       ,
                                          const css::uno::Sequence< css::beans::PropertyValue >& /*lArguments*/ )
{
    // dispatch() is an [oneway] call ... and may our user release his reference to us immediately.
    // So we should hold us self alive till this call ends.
    css::uno::Reference< css::frame::XNotifyingDispatch > xSelfHold(this);
    implts_dispatch(aURL);
    // No notification for status listener!
}

/**
    @short      dispatch with guaranteed notifications about success
    @descr      We use threadsafe internal method to do so. Return state of this function will be used
                for notification if an optional listener is given.

    @param      aURL
                    mail URL which should be executed
    @param      lArguments
                    list of optional arguments for this mail request
    @param      xListener
                    reference to a valid listener for state events
*/
void SAL_CALL MailToDispatcher::dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                          const css::uno::Sequence< css::beans::PropertyValue >&            /*lArguments*/,
                                                          const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // This class was designed to die by reference. And if user release his reference to us immediately after calling this method
    // we can run into some problems. So we hold us self alive till this method ends.
    // Another reason: We can use this reference as source of sending event at the end too.
    css::uno::Reference< css::frame::XNotifyingDispatch > xThis(this);

    bool bState = implts_dispatch(aURL);
    if (xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        if (bState)
            aEvent.State = css::frame::DispatchResultState::SUCCESS;
        else
            aEvent.State = css::frame::DispatchResultState::FAILURE;
        aEvent.Source = xThis;

        xListener->dispatchFinished( aEvent );
    }
}

/**
    @short      threadsafe helper for dispatch calls
    @descr      We support two interfaces for the same process - dispatch URLs. That the reason for this internal
                function. It implements the real dispatch operation and returns a state value which inform caller
                about success. He can notify listener then by using this return value.

    @param      aURL
                    mail URL which should be executed

    @return     <TRUE/> if dispatch could be started successfully
                Note: Our internal used shell executor doesn't return any state value - so we must
                believe that call was successful.
                <FALSE/> if necessary resource couldn't be created or an exception was thrown.
*/
bool MailToDispatcher::implts_dispatch( const css::util::URL& aURL )
{
    bool bSuccess = false;

    css::uno::Reference< css::system::XSystemShellExecute > xSystemShellExecute = css::system::SystemShellExecute::create( m_xContext );

    try
    {
        // start mail client
        // Because there is no notification about success - we use case of
        // no detected exception as SUCCESS - FAILED otherwise.
        xSystemShellExecute->execute( aURL.Complete, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY );
        bSuccess = true;
    }
    catch (const css::lang::IllegalArgumentException&)
    {
    }
    catch (const css::system::SystemShellExecuteException&)
    {
    }

    return bSuccess;
}

/**
    @short      add/remove listener for state events
    @descr      Because we use an external process to forward such mail URLs, and this process doesn't
                return any notifications about success or failed state - we don't support such status
                listener. We have no status to send.

    @param      xListener
                    reference to a valid listener for state events
    @param      aURL
                    URL about listener will be informed, if something occurred
*/
void SAL_CALL MailToDispatcher::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                   const css::util::URL&                                     /*aURL*/      )
{
    // not supported yet
}

void SAL_CALL MailToDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                      const css::util::URL&                                     /*aURL*/      )
{
    // not supported yet
}

} //  namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_MailToDispatcher_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::MailToDispatcher(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
