/*************************************************************************
 *
 *  $RCSfile: asyncnotification.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-17 11:25:31 $
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

#ifndef COMPHELPER_ASYNCNOTIFICATION_HXX
#include <comphelper/asyncnotification.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
/** === end UNO includes === **/

#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//........................................................................
namespace comphelper
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::awt;

    //====================================================================
    //= EventDescription
    //====================================================================
    //--------------------------------------------------------------------
    EventDescription::EventDescription( sal_uInt16 _nType )
        :m_refCount( 0 )
        ,m_nType( _nType )
    {
    }

    //--------------------------------------------------------------------
    EventDescription::EventDescription( const EventDescription& _rSource )
        :m_refCount( 0 )
    {
        *this = _rSource;
    }

    //--------------------------------------------------------------------
    EventDescription& EventDescription::operator=( const EventDescription& _rSource )
    {
        m_nType = _rSource.m_nType;
        return *this;
    }

    //--------------------------------------------------------------------
    EventDescription::~EventDescription()
    {
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL EventDescription::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL EventDescription::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
            delete this;
            return 0;
        }
        return m_refCount;
    }

    //====================================================================
    //= EventNotifierImpl
    //====================================================================
    struct EventNotifierImpl
    {
        ::osl::Mutex                    aMutex;
        ::osl::Condition                aCond;

        AsyncEventNotifier::Events      aEvents;

        IEventProcessor*                pEventProcessor;
        Reference< XComponent >         xComponent;
    };

    //====================================================================
    //= AsyncEventNotifier
    //====================================================================
    //--------------------------------------------------------------------
    AsyncEventNotifier::AsyncEventNotifier( IEventProcessor* _pProcessor )
        :m_pImpl( new EventNotifierImpl )
    {
        // observe the XComponent belonging to the event processor
        m_pImpl->pEventProcessor = _pProcessor;
        if ( m_pImpl->pEventProcessor )
            m_pImpl->xComponent = m_pImpl->pEventProcessor->getComponent();
        OSL_ENSURE( m_pImpl->xComponent.is(), "AsyncEventNotifier::AsyncEventNotifier: invalid event processor!" );

        if ( m_pImpl->xComponent.is() )
        {
            osl_incrementInterlockedCount( &m_refCount );
            {
                Reference< XEventListener > xLocalTemporary( this );
                m_pImpl->xComponent->addEventListener( xLocalTemporary );
            }
            osl_incrementInterlockedCount( &m_refCount );
        }
    }

    //--------------------------------------------------------------------
    AsyncEventNotifier::~AsyncEventNotifier()
    {
        OSL_ENSURE( m_pImpl->aEvents.empty(), "AsyncEventNotifier::~AsyncEventNotifier: did you dispose me?" );
    }

    //--------------------------------------------------------------------
    void SAL_CALL AsyncEventNotifier::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        if ( ( _rSource.Source == m_pImpl->xComponent ) && m_pImpl->xComponent.is() )
        {
            ::osl::MutexGuard aGuard( m_pImpl->aMutex );

            m_pImpl->xComponent->removeEventListener( static_cast< XEventListener* >( this ) );

            // Event-Queue loeschen
            Events aEmpty;
            m_pImpl->aEvents.swap( aEmpty );

            // reset m_pImpl->xComponent. This will cause the "run" method to terminate
            m_pImpl->xComponent.clear();

            // awake the thread
            m_pImpl->aCond.set();
            // and terminate
            terminate();
        }
    }

    //--------------------------------------------------------------------
    void AsyncEventNotifier::addEvent( const EventDescriptionRef& _rEvent )
    {
        ::osl::MutexGuard aGuard( m_pImpl->aMutex );
        m_pImpl->aEvents.push_back( _rEvent );

        // awake the thread
        m_pImpl->aCond.set();
    }

    //--------------------------------------------------------------------
    void AsyncEventNotifier::run()
    {
        implStarted( );
        if ( !m_pImpl->pEventProcessor )
            return;

        // keep us alive, in case we're disposed in the mid of the following
        Reference< XInterface > xKeepAlive( *this );
        do
        {
            ::osl::MutexGuard aGuard( m_pImpl->aMutex );
            while ( m_pImpl->aEvents.size() > 0 )
            {
                // keep a reference to the control, so it cannot be deleted
                // during processEvent
                Reference< XComponent > xComponent = m_pImpl->xComponent;

                ::rtl::Reference< EventDescription > pEvent = m_pImpl->aEvents.front();
                m_pImpl->aEvents.pop_front();
                {
                    ::comphelper::MutexRelease aReleaseOnce( m_pImpl->aMutex );

                    if ( xComponent.is() && pEvent.get() )
                        m_pImpl->pEventProcessor->processEvent( *pEvent.get() );
                }
            };

            // if we have been disposed in the above, then we're completely done
            if( !m_pImpl->xComponent.is() )
                return;

            m_pImpl->aCond.reset();
            {
                ::comphelper::MutexRelease aReleaseOnce( m_pImpl->aMutex );
                // wait until there's a new event to process
                m_pImpl->aCond.wait();
            }
        }
        while ( sal_True );
    }

    //--------------------------------------------------------------------
    void SAL_CALL AsyncEventNotifier::kill()
    {
        AsyncEventNotifier_TBASE::kill();
        implTerminated( );
    }

    //--------------------------------------------------------------------
    void SAL_CALL AsyncEventNotifier::onTerminated()
    {
        AsyncEventNotifier_TBASE::onTerminated();
        implTerminated( );
    }

    //--------------------------------------------------------------------
    void AsyncEventNotifier::implStarted()
    {
        acquire();
    }

    //--------------------------------------------------------------------
    void AsyncEventNotifier::implTerminated()
    {
        release();
    }

//........................................................................
} // namespace comphelper
//........................................................................

