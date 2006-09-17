/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: desktopterminationobserver.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:27:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#ifndef UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX
#include <unotools/desktopterminationobserver.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

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
                OSL_ENSURE( sal_False, "OObserverImpl::ensureObservation: caught an exception!" );
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
        if ( getListenerAdminData().bAlreadyTerminated )
        {
            _pListener->notifyTermination();
            return;
        }

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

