/*************************************************************************
 *
 *  $RCSfile: mailtodispatcher.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2002-05-02 11:40:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_DISPATCH_MAILTODISPATCHER_HXX_
#include <dispatch/mailtodispatcher.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

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

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_4(MailToDispatcher                                ,
                    OWeakObject                                     ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo       ),
                    DIRECT_INTERFACE(css::frame::XNotifyingDispatch),
                    DIRECT_INTERFACE(css::frame::XDispatch         ))

DEFINE_XTYPEPROVIDER_4(MailToDispatcher              ,
                       css::lang::XTypeProvider      ,
                       css::lang::XServiceInfo       ,
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
                <FALSE/> if neccessary ressource couldn't be created or an exception was thrown.

    @modified   30.04.2002 14:49, as96863
*/
sal_Bool MailToDispatcher::implts_dispatch( const css::util::URL&                                  aURL       ,
                                            const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    sal_Bool bSuccess = sal_False;

    // don't accept other protocols
    // Normaly we shouldn't be used for other URLs then "mailto" ...
    // but ...
    if (aURL.Protocol.compareToAscii("mailto:",7)==0)
    {
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
                ::rtl::OUString sURL( aURL.Complete );
                xSystemShellExecute->execute( sURL, ::rtl::OUString(), css::system::SystemShellExecuteFlags::DEFAULTS );
                bSuccess = sal_True;
            }
            catch (css::lang::IllegalArgumentException&)
            {
            }
            catch (css::system::SystemShellExecuteException&)
            {
            }
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
                    URL about listener will be informed, if something occured

    @modified   30.04.2002 14:49, as96863
*/
void SAL_CALL MailToDispatcher::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                   const css::util::URL&                                     aURL      ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

//_________________________________________________________________________________________________________________

void SAL_CALL MailToDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                      const css::util::URL&                                     aURL      ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

} //  namespace framework
