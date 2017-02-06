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

#include <unotools/desktopterminationobserver.hxx>

#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

#include <list>

namespace utl
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;

    namespace
    {

        typedef ::std::list< ITerminationListener* > Listeners;

        struct ListenerAdminData
        {
            Listeners   aListeners;
            bool        bAlreadyTerminated;
            bool        bCreatedAdapter;

            ListenerAdminData() : bAlreadyTerminated( false ), bCreatedAdapter( false ) { }
        };

        ListenerAdminData& getListenerAdminData()
        {
            static ListenerAdminData s_aData;
            return s_aData;
        }

        //= OObserverImpl

        class OObserverImpl : public ::cppu::WeakImplHelper< XTerminateListener >
        {
        public:
            static void ensureObservation();

        protected:
            OObserverImpl();
            virtual ~OObserverImpl() override;

        private:
            // XTerminateListener
            virtual void SAL_CALL queryTermination( const EventObject& Event ) override;
            virtual void SAL_CALL notifyTermination( const EventObject& Event ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
        };

        OObserverImpl::OObserverImpl()
        {
        }

        OObserverImpl::~OObserverImpl()
        {
        }

        void OObserverImpl::ensureObservation()
        {
            {
                if ( getListenerAdminData().bCreatedAdapter )
                    return;
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                if ( getListenerAdminData().bCreatedAdapter )
                    return;

                getListenerAdminData().bCreatedAdapter = true;
            }

            try
            {
                Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
                xDesktop->addTerminateListener( new OObserverImpl );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "OObserverImpl::ensureObservation: caught an exception!" );
            }
        }

        void SAL_CALL OObserverImpl::queryTermination( const EventObject& /*Event*/ )
        {
            Listeners aToNotify;
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                aToNotify = getListenerAdminData().aListeners;
            }

            for ( Listeners::const_iterator listener = aToNotify.begin();
                  listener != aToNotify.end();
                  ++listener
                )
            {
                if ( !(*listener)->queryTermination() )
                    throw TerminationVetoException();
            }
        }

        void SAL_CALL OObserverImpl::notifyTermination( const EventObject& /*Event*/ )
        {
            // get the listeners
            Listeners aToNotify;
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                OSL_ENSURE( !getListenerAdminData().bAlreadyTerminated, "OObserverImpl::notifyTermination: terminated twice?" );
                aToNotify = getListenerAdminData().aListeners;
                getListenerAdminData().bAlreadyTerminated = true;
            }

            // notify the listeners
            for ( Listeners::const_iterator listener = aToNotify.begin();
                  listener != aToNotify.end();
                  ++listener
                )
            {
                (*listener)->notifyTermination();
            }

            // clear the listener container
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                getListenerAdminData().aListeners.clear();
            }
        }

        void SAL_CALL OObserverImpl::disposing( const EventObject& /*Event*/ )
        {
#if OSL_DEBUG_LEVEL > 0
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            OSL_ENSURE( getListenerAdminData().bAlreadyTerminated, "OObserverImpl::disposing: disposing without terminated?" );
#endif
            // not interested in
        }
    }

    //= DesktopTerminationObserver

    void DesktopTerminationObserver::registerTerminationListener( ITerminationListener* _pListener )
    {
        if ( !_pListener )
            return;

        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( getListenerAdminData().bAlreadyTerminated )
            {
                _pListener->notifyTermination();
                return;
            }

            getListenerAdminData().aListeners.push_back( _pListener );
        }

        OObserverImpl::ensureObservation();
    }

    void DesktopTerminationObserver::revokeTerminationListener( ITerminationListener* _pListener )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        Listeners& rListeners = getListenerAdminData().aListeners;
        for ( Listeners::iterator lookup = rListeners.begin();
              lookup != rListeners.end();
              ++lookup
              )
        {
            if ( *lookup == _pListener )
            {
                rListeners.erase( lookup );
                break;
            }
        }
    }

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
