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
#include "precompiled_unotools.hxx"
#include <unotools/desktopterminationobserver.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
/** === end UNO includes === **/
#include <cppuhelper/implbase1.hxx>
#include <comphelper/processfactory.hxx>

#include <list>

//........................................................................
namespace utl
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;

    namespace
    {
        //................................................................
        typedef ::std::list< ITerminationListener* > Listeners;

        struct ListenerAdminData
        {
            Listeners   aListeners;
            bool        bAlreadyTerminated;
            bool        bCreatedAdapter;

            ListenerAdminData() : bAlreadyTerminated( false ), bCreatedAdapter( false ) { }
        };

        //................................................................
        ListenerAdminData& getListenerAdminData()
        {
            static ListenerAdminData s_aData;
            return s_aData;
        }

        //================================================================
        //= OObserverImpl
        //================================================================
        class OObserverImpl : public ::cppu::WeakImplHelper1< XTerminateListener >
        {
        public:
            static void ensureObservation();

        protected:
            OObserverImpl();
            ~OObserverImpl();

        private:
            // XTerminateListener
            virtual void SAL_CALL queryTermination( const EventObject& Event ) throw (TerminationVetoException, RuntimeException);
            virtual void SAL_CALL notifyTermination( const EventObject& Event ) throw (RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
        };

        //--------------------------------------------------------------------
        OObserverImpl::OObserverImpl()
        {
        }

        //--------------------------------------------------------------------
        OObserverImpl::~OObserverImpl()
        {
        }

        //--------------------------------------------------------------------
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
                Reference< XDesktop > xDesktop;
                xDesktop = xDesktop.query( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ) );
                OSL_ENSURE( xDesktop.is(), "OObserverImpl::ensureObservation: could not ensureObservation the desktop!" );
                if ( xDesktop.is() )
                    xDesktop->addTerminateListener( new OObserverImpl );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "OObserverImpl::ensureObservation: caught an exception!" );
            }
        }

        //--------------------------------------------------------------------
        void SAL_CALL OObserverImpl::queryTermination( const EventObject& /*Event*/ ) throw (TerminationVetoException, RuntimeException)
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

        //--------------------------------------------------------------------
        void SAL_CALL OObserverImpl::notifyTermination( const EventObject& /*Event*/ ) throw (RuntimeException)
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

        //--------------------------------------------------------------------
        void SAL_CALL OObserverImpl::disposing( const EventObject& /*Event*/ ) throw (RuntimeException)
        {
#if OSL_DEBUG_LEVEL > 0
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            OSL_ENSURE( getListenerAdminData().bAlreadyTerminated, "OObserverImpl::disposing: disposing without terminated?" );
#endif
            // not interested in
        }
    }

    //====================================================================
    //= DesktopTerminationObserver
    //====================================================================
    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
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

//........................................................................
} // namespace utl
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
