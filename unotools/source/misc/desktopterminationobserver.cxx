/*************************************************************************
 *
 *  $RCSfile: desktopterminationobserver.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-06 13:13:51 $
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
        void SAL_CALL OObserverImpl::queryTermination( const EventObject& Event ) throw (TerminationVetoException, RuntimeException)
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
        void SAL_CALL OObserverImpl::notifyTermination( const EventObject& Event ) throw (RuntimeException)
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
        void SAL_CALL OObserverImpl::disposing( const EventObject& Event ) throw (RuntimeException)
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

