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
#include <threadhelp/readguard.hxx>
#include <general.h>
#include <services.h>

#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include <vcl/svapp.hxx>

namespace framework{

//_________________________________________________________________________________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_MULTISERVICE_2(MailToDispatcher                   ,
                                 ::cppu::OWeakObject                ,
                                 SERVICENAME_PROTOCOLHANDLER        ,
                                 IMPLEMENTATIONNAME_MAILTODISPATCHER)

DEFINE_INIT_SERVICE(MailToDispatcher,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                        */
                    }
                   )

//_________________________________________________________________________________________________________________

/**
    @short      standard ctor
    @descr      These initialize a new instance of ths class with needed information for work.

    @param      rxContext
                    reference to uno servicemanager for creation of new services
*/
MailToDispatcher::MailToDispatcher( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        // Init member
        , m_xContext    ( rxContext                     )
{
}

//_________________________________________________________________________________________________________________

/**
    @short      standard dtor
    @descr      -
*/
MailToDispatcher::~MailToDispatcher()
{
    m_xContext = NULL;
}

//_________________________________________________________________________________________________________________

/**
    @short      decide if this dispatch implementation can be used for requested URL or not
    @descr      A protocol handler is registerd for an URL pattern inside configuration and will
                be asked by the generic dispatch mechanism inside framework, if he can handle this
                special URL which match his registration. He can agree by returning of a valid dispatch
                instance or disagree by returning <NULL/>.
                We don't create new dispatch instances here realy - we return THIS as result to handle it
                at the same implementation.
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL MailToDispatcher::queryDispatch( const css::util::URL&  aURL    ,
                                                                                       const OUString& /*sTarget*/ ,
                                                                                             sal_Int32        /*nFlags*/  ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.startsWith("mailto:"))
        xDispatcher = this;
    return xDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      do the same like dispatch() but for multiple requests at the same time
    @descr      -
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL MailToDispatcher::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException )
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = this->queryDispatch(
                            lDescriptor[i].FeatureURL,
                            lDescriptor[i].FrameName,
                            lDescriptor[i].SearchFlags);
    }
    return lDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      dispatch URL with arguments
    @descr      We use threadsafe internal method to do so. It returns a state value - but we ignore it.
                Because we doesn't support status listener notifications here. Status events are not guaranteed -
                and we call another service internaly which doesn't return any notifications too.

    @param      aURL
                    mail URL which should be executed
    @param      lArguments
                    list of optional arguments for this mail request
*/
void SAL_CALL MailToDispatcher::dispatch( const css::util::URL&                                  aURL       ,
                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    // dispatch() is an [oneway] call ... and may our user release his reference to us immediately.
    // So we should hold us self alive till this call ends.
    css::uno::Reference< css::frame::XNotifyingDispatch > xSelfHold(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    implts_dispatch(aURL,lArguments);
    // No notification for status listener!
}

//_________________________________________________________________________________________________________________

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
                                                          const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                          const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw( css::uno::RuntimeException )
{
    // This class was designed to die by reference. And if user release his reference to us immediately after calling this method
    // we can run into some problems. So we hold us self alive till this method ends.
    // Another reason: We can use this reference as source of sending event at the end too.
    css::uno::Reference< css::frame::XNotifyingDispatch > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    sal_Bool bState = implts_dispatch(aURL,lArguments);
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

//_________________________________________________________________________________________________________________

/**
    @short      threadsafe helper for dispatch calls
    @descr      We support two interfaces for the same process - dispatch URLs. That the reason for this internal
                function. It implements the real dispatch operation and returns a state value which inform caller
                about success. He can notify listener then by using this return value.

    @param      aURL
                    mail URL which should be executed
    @param      lArguments
                    list of optional arguments for this mail request

    @return     <TRUE/> if dispatch could be started successfully
                Note: Our internal used shell executor doesn't return any state value - so we must
                belive that call was successfully.
                <FALSE/> if neccessary resource couldn't be created or an exception was thrown.
*/
sal_Bool MailToDispatcher::implts_dispatch( const css::util::URL&                                  aURL       ,
                                            const css::uno::Sequence< css::beans::PropertyValue >& /*lArguments*/ ) throw( css::uno::RuntimeException )
{
    sal_Bool bSuccess = sal_False;

    css::uno::Reference< css::uno::XComponentContext > xContext;
    /* SAFE */{
        ReadGuard aReadLock( m_aLock );
        xContext = m_xContext;
    /* SAFE */}

    css::uno::Reference< css::system::XSystemShellExecute > xSystemShellExecute = css::system::SystemShellExecute::create( xContext );

    try
    {
        // start mail client
        // Because there is no notofocation about success - we use case of
        // no detected exception as SUCCESS - FAILED otherwise.
        xSystemShellExecute->execute( aURL.Complete, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY );
        bSuccess = sal_True;
    }
    catch (const css::lang::IllegalArgumentException&)
    {
    }
    catch (const css::system::SystemShellExecuteException&)
    {
    }

    return bSuccess;
}

//_________________________________________________________________________________________________________________

/**
    @short      add/remove listener for state events
    @descr      Because we use an external process to forward such mail URLs, and this process doesn't
                return any notifications about success or failed state - we doesn't support such status
                listener. We have no status to send.

    @param      xListener
                    reference to a valid listener for state events
    @param      aURL
                    URL about listener will be informed, if something occurred
*/
void SAL_CALL MailToDispatcher::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                   const css::util::URL&                                     /*aURL*/      ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

//_________________________________________________________________________________________________________________

void SAL_CALL MailToDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                      const css::util::URL&                                     /*aURL*/      ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

} //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
