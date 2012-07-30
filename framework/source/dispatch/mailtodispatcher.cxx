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


//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <dispatch/mailtodispatcher.hxx>
#include <threadhelp/readguard.hxx>
#include <general.h>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

#define PROTOCOL_VALUE      "mailto:"
#define PROTOCOL_LENGTH     7

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_5(MailToDispatcher                                ,
                    OWeakObject                                     ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo       ),
                    DIRECT_INTERFACE(css::frame::XDispatchProvider ),
                    DIRECT_INTERFACE(css::frame::XNotifyingDispatch),
                    DIRECT_INTERFACE(css::frame::XDispatch         ))

DEFINE_XTYPEPROVIDER_5(MailToDispatcher              ,
                       css::lang::XTypeProvider      ,
                       css::lang::XServiceInfo       ,
                       css::frame::XDispatchProvider ,
                       css::frame::XNotifyingDispatch,
                       css::frame::XDispatch         )

DEFINE_XSERVICEINFO_MULTISERVICE(MailToDispatcher                   ,
                                 ::cppu::OWeakObject                ,
                                 SERVICENAME_PROTOCOLHANDLER        ,
                                 IMPLEMENTATIONNAME_MAILTODISPATCHER)

DEFINE_INIT_SERVICE(MailToDispatcher,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//_________________________________________________________________________________________________________________

/**
    @short      standard ctor
    @descr      These initialize a new instance of ths class with needed informations for work.

    @param      xFactory
                    reference to uno servicemanager for creation of new services

    @modified   30.04.2002 14:10, as96863
*/
MailToDispatcher::MailToDispatcher( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        , OWeakObject   (                               )
        // Init member
        , m_xFactory    ( xFactory                      )
{
}

//_________________________________________________________________________________________________________________

/**
    @short      standard dtor
    @descr      -

    @modified   30.04.2002 14:10, as96863
*/
MailToDispatcher::~MailToDispatcher()
{
    m_xFactory = NULL;
}

//_________________________________________________________________________________________________________________

/**
    @short      decide if this dispatch implementation can be used for requested URL or not
    @descr      A protocol handler is registerd for an URL pattern inside configuration and will
                be asked by the generic dispatch mechanism inside framework, if he can handle this
                special URL wich match his registration. He can agree by returning of a valid dispatch
                instance or disagree by returning <NULL/>.
                We don't create new dispatch instances here realy - we return THIS as result to handle it
                at the same implementation.

    @modified   02.05.2002 15:25, as96863
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL MailToDispatcher::queryDispatch( const css::util::URL&  aURL    ,
                                                                                       const ::rtl::OUString& /*sTarget*/ ,
                                                                                             sal_Int32        /*nFlags*/  ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.compareToAscii(PROTOCOL_VALUE,PROTOCOL_LENGTH)==0)
        xDispatcher = this;
    return xDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      do the same like dispatch() but for multiple requests at the same time
    @descr      -

    @modified   02.05.2002 15:27, as96863
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

    @modified   30.04.2002 14:15, as96863
*/
void SAL_CALL MailToDispatcher::dispatch( const css::util::URL&                                  aURL       ,
                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    // dispatch() is an [oneway] call ... and may our user release his reference to us immediatly.
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

    @modified   30.04.2002 14:49, as96863
*/
void SAL_CALL MailToDispatcher::dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                          const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                          const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw( css::uno::RuntimeException )
{
    // This class was designed to die by reference. And if user release his reference to us immediatly after calling this method
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

    @modified   30.04.2002 14:49, as96863
*/
sal_Bool MailToDispatcher::implts_dispatch( const css::util::URL&                                  aURL       ,
                                            const css::uno::Sequence< css::beans::PropertyValue >& /*lArguments*/ ) throw( css::uno::RuntimeException )
{
    sal_Bool bSuccess = sal_False;

    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory;
    /* SAFE */{
        ReadGuard aReadLock( m_aLock );
        xFactory = m_xFactory;
    /* SAFE */}

    css::uno::Reference< css::system::XSystemShellExecute > xSystemShellExecute( xFactory->createInstance(SERVICENAME_SYSTEMSHELLEXECUTE), css::uno::UNO_QUERY );
    if (xSystemShellExecute.is())
    {
        try
        {
            // start mail client
            // Because there is no notofocation about success - we use case of
            // no detected exception as SUCCESS - FAILED otherwhise.
            xSystemShellExecute->execute( aURL.Complete, ::rtl::OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY );
            bSuccess = sal_True;
        }
        catch (const css::lang::IllegalArgumentException&)
        {
        }
        catch (const css::system::SystemShellExecuteException&)
        {
        }
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

    @modified   30.04.2002 14:49, as96863
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
