/*************************************************************************
 *
 *  $RCSfile: accessibleeventnotifier.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2002-12-06 12:56:46 $
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

#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::drafts::com::sun::star::accessibility;
    using namespace ::comphelper;

    //=====================================================================
    //= AccessibleEventNotifier
    //=====================================================================
    //---------------------------------------------------------------------
    ::osl::Mutex                AccessibleEventNotifier::s_aMutex;
    AccessibleEventNotifier*    AccessibleEventNotifier::s_pNotifier = NULL;

    //---------------------------------------------------------------------
    AccessibleEventNotifier::AccessibleEventNotifier( )
        :m_bTerminateRequested( sal_False )
    {
        // no events so far
        m_aEventGuard.reset();
    }

    //---------------------------------------------------------------------
    AccessibleEventNotifier::~AccessibleEventNotifier( )
    {
        OSL_ENSURE( m_aClients.empty() && m_aDisposedClients.empty(),
            "AccessibleEventNotifier::~AccessibleEventNotifier: not correctly terminated - resource leak!" );
    }

    //---------------------------------------------------------------------
    namespace
    {
        static void lcl_copyInterfaceContainer( const ::cppu::OInterfaceContainerHelper& _rSource, ::cppu::OInterfaceContainerHelper& _rDest )
        {
            _rDest.clear();
            Sequence< Reference< XInterface > > aInterfaces( _rSource.getElements() );

            const Reference< XInterface >* pInterfaces = aInterfaces.getConstArray();
            const Reference< XInterface >* pInterfacesEnd = pInterfaces + aInterfaces.getLength();
            for ( ; pInterfaces != pInterfacesEnd; ++pInterfaces )
                _rDest.addInterface( *pInterfaces );
        }
    }
    //---------------------------------------------------------------------
    void SAL_CALL AccessibleEventNotifier::run()
    {
        sal_Bool bTerminate = sal_False;

        do
        {
            // notify the events we have in the queue
            // --- <mutex_lock> -------------------------------------------
            {
                ::osl::MutexGuard aGuard( s_aMutex );

                // continue with all events we have so far
                while ( !m_aEvents.empty() )
                {
                    // the first event in the queue
                    ClientEvent aEvent = m_aEvents.front();
                    m_aEvents.pop_front();

                    // special handling for "disposing"
                    if ( aEvent.second.EventId < 0 )
                    {
                        // look up in the map for "disposed clients"
                        ClientMap::iterator aPos = m_aDisposedClients.find( aEvent.first );
                        OSL_ENSURE( m_aDisposedClients.end() != aPos,
                            "AccessibleEventNotifier::run: could not find this client!" );

                        if ( m_aDisposedClients.end() != aPos )
                        {
                            EventObject aDisposalEvent;
                            aDisposalEvent.Source = aEvent.second.Source;

                            // want to call the listeners with a released mutex
                            // thus we have to copy the container, so that we can savely use the copy while
                            // our mutex is released

                            ::cppu::OInterfaceContainerHelper aCopy( s_aMutex );
                            lcl_copyInterfaceContainer( *aPos->second, aCopy );

                            // we do not need the entry in the "disposed clients" map anymore
                            // because the "disposed" event is the _last_ one to be fired for a client
                            delete aPos->second;
                            m_aDisposedClients.erase( aPos );

                            // now do the notification, and do it with the _copy_ after releasing the mutex
                            // --- <mutex_release> ------------------------
                            {
                                MutexRelease aReleaseOnce( s_aMutex );
                                aCopy.disposeAndClear( aDisposalEvent );

                                // clear the aDisposalEvent while our mutex is _not_ acquired
                                // this ensures that we do not - by accident - release the last reference
                                // of the foreign component while our mutex is locked
                                aDisposalEvent.Source.clear();
                            }
                            // --- </mutex_release> -----------------------

                            // cleanup the thread if we do not have clients anymore
                            implCleanupNotifier( );
                        }
                    }
                    else
                    {
                        // look up the client for this event
                        ClientMap::iterator aClientPos;
                        if ( implLookupClient( aEvent.first, aClientPos ) )
                        {
                            // copy the listener sequence. We do _not_ want to call into the listeners
                            // with our mutex locked
                            Sequence< Reference< XInterface > > aListeners( aClientPos->second->getElements() );
                            // default handling: loop through all listeners, and notify them

                            const Reference< XInterface >* pListeners = aListeners.getConstArray();
                            const Reference< XInterface >* pListenersEnd = pListeners + aListeners.getLength();

                            // --- <mutex_release> ------------------------
                            {
                                // release the mutex within this block
                                MutexRelease aReleaseOnce( s_aMutex );

                                while ( pListeners != pListenersEnd )
                                {
                                    try
                                    {
                                        static_cast< XAccessibleEventListener* >( pListeners->get() )->notifyEvent( aEvent.second );
                                    }
                                    catch( const Exception& e )
                                    {
                                        e;
                                        // silent this
                                        // no assertion, because a broken access remote bridge or something like this
                                        // can cause this exception
                                    }
                                    ++pListeners;
                                }
                            }
                            // --- </mutex_release> -----------------------
                        }
                        else
                            OSL_ENSURE( sal_False, "AccessibleEventNotifier::run: invalid client id found for accessible event!" );
                    }

                    // --- <mutex_release> --------------------------------
                    {
                        MutexRelease aReleaseOnce( s_aMutex );
                        // clear the event
                        // do this with our own mutex released, as clearing the event includes releasing the reference
                        // to the css.lang.EventObject.Source - in case this release is non-trivial (i.e. the last
                        // reference to the object), we certainly do _not_ want to do this while our
                        // mutex is locked
                        aEvent = ClientEvent();
                    }
                    // --- </mutex_release> -------------------------------
                }

                // reset the condition - will be set as soon as a new event arrives
                m_aEventGuard.reset();
            }
            // --- </mutex_lock> ------------------------------------------

            // wait (sleep) 'til a new event arrives
            m_aEventGuard.wait();

            // --- <mutex_lock> -------------------------------------------
            {
                ::osl::MutexGuard aGuard( s_aMutex );
                bTerminate = m_bTerminateRequested;
            }
            // --- </mutex_lock> ------------------------------------------
        }
        while ( !bTerminate );
    }

    //---------------------------------------------------------------------
    void SAL_CALL AccessibleEventNotifier::terminate()
    {
        AccessibleEventNotifier_BASE::terminate();
        // base class does not call onTerminated - just in case we want to do any cleanup there ...
        onTerminated();
    }

    //---------------------------------------------------------------------
    void SAL_CALL AccessibleEventNotifier::onTerminated()
    {
        delete this;
    }

    //---------------------------------------------------------------------
    AccessibleEventNotifier::TClientId AccessibleEventNotifier::generateId()
    {
        TClientId nBiggestUsedId = 0;
        TClientId nFreeId = 0;

        // look through all registered clients until we find a "gap" in the ids

        // Note that the following relies on the fact the elements in the map are traveled with
        // ascending keys (aka client ids)

        for (   ClientMap::const_iterator aLookup = m_aClients.begin();
                aLookup != m_aClients.end();
                ++aLookup
            )
        {
            TClientId nCurrent = aLookup->first;
            OSL_ENSURE( nCurrent > nBiggestUsedId, "AccessibleEventNotifier::generateId: map is expected to be sorted ascending!" );

            if ( nCurrent - nBiggestUsedId > 1 )
            {   // found a "gap"
                TClientId nCandidate = nBiggestUsedId + 1;

                // ensure that the id is really free - it's possible that the id is still in the "disposed clients"
                // map
                if ( m_aDisposedClients.end() == m_aDisposedClients.find( nCandidate ) )
                {   // yep, it's really available
                    nFreeId = nCandidate;
                    break;
                }
            }

            nBiggestUsedId = nCurrent;
        }

        if ( !nFreeId )
            nFreeId = nBiggestUsedId + 1;

        OSL_ENSURE( m_aClients.end() == m_aClients.find( nFreeId ),
            "AccessibleEventNotifier::generateId: algorithm broken!" );

        return nFreeId;
    }

    //---------------------------------------------------------------------
    AccessibleEventNotifier::TClientId AccessibleEventNotifier::registerClient( )
    {
        ::osl::MutexGuard aGuard( s_aMutex );
        if ( !s_pNotifier )
        {   // the first client -> create the thread

            // create the thread object
            s_pNotifier = new AccessibleEventNotifier;

            // run the thread
            s_pNotifier->create();
                // note that the thread will start running, and the first thing it will do is stopping
                // in run, waiting for the mutex
        }

        // generate a new client id
        TClientId nNewClientId = s_pNotifier->generateId( );

        // the event listeners for the new client
        EventListeners* pNewListeners = new EventListeners( s_aMutex );
            // note that we're using our own mutex here, so the listener containers for all
            // our clients share this same mutex.
            // Shouldn't be any problem: the only situation where this is used is when the
            // thread is firing events, and there the mutex is locked, anyway.

        // add the client
        s_pNotifier->m_aClients.insert( ClientMap::value_type( nNewClientId, pNewListeners ) );

        // outta here
        return nNewClientId;
    }

    //---------------------------------------------------------------------
    sal_Bool AccessibleEventNotifier::implLookupClient( const TClientId _nClient, ClientMap::iterator& _rPos )
    {
        OSL_ENSURE( s_pNotifier, "AccessibleEventNotifier::implLookupClient: illegal call: thread not running!" );
        if ( !s_pNotifier )
            return sal_False;

        // look up this client
        _rPos = s_pNotifier->m_aClients.find( _nClient );
        OSL_ENSURE( s_pNotifier->m_aClients.end() != _rPos, "AccessibleEventNotifier::implLookupClient: invalid client id (did you register your client?)!" );

        return ( s_pNotifier->m_aClients.end() != _rPos );
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::implRemoveEventsForClient( const TClientId _nClient,
            ::std::vector< Reference< XInterface > >& _rEnsureAlive )
    {
        OSL_ENSURE( s_pNotifier, "AccessibleEventNotifier::implRemoveEventsForClient: invalid call, save your documents before it crashes!" );

        EventQueue::iterator aEventLoop = s_pNotifier->m_aEvents.begin();
        while ( aEventLoop != s_pNotifier->m_aEvents.end() )
        {
            if ( _nClient == aEventLoop->first )
            {
                // this is an event queued for the same client
                // -> remove it from the queue
                EventQueue::iterator aErasePos( aEventLoop );
                ++aEventLoop;

                // keep the object alive until we can free our own mutex
                _rEnsureAlive.push_back( aErasePos->second.Source );

                // erase the event
                s_pNotifier->m_aEvents.erase( aErasePos );
            }
            else
                ++aEventLoop;
        }
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::implCleanupNotifier( )
    {
        OSL_PRECOND( s_pNotifier, "AccessibleEventNotifier::implCleanupNotifier: invalid call!" );

        if ( s_pNotifier->m_aClients.empty() && s_pNotifier->m_aDisposedClients.empty() )
        {
            // killing me softly ....

            // tell the instance it should terminate
            s_pNotifier->m_bTerminateRequested = sal_True;

            // awake it
            // (it is sleeping currently - if it were not, it would be in the section
            // guarded by s_aMutex (see <method>run</method>), which is impossible as
            // our thread here has this mutex currently ...
            s_pNotifier->m_aEventGuard.set();

            // reset the notifier holder - thus, the thread may continue to run the few microseconds
            // it will need to finally terminate, but if in the meantime new clients
            // are registered, we will not burden this (terminating) notifier with it,
            // but create a new one.
            // Note that the instance will delete itself in onTerminated
            s_pNotifier = NULL;
        }

        OSL_POSTCOND( !s_pNotifier || !s_pNotifier->m_aClients.empty() || !s_pNotifier->m_aDisposedClients.empty(),
            "AccessibleEventNotifier::implCleanupNotifier: post condition violated!" );
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::revokeClient( const TClientId _nClient )
    {
        // below, we will destroy some AccessibleEventObject instances
        // their Source member refers a foreign component (the broadcaster), which we
        // will release with this destruction. In case that is the _last_ release, it
        // would be potentially deadly if we call it while our own mutex is locked.
        // So we ensure that all these objects are alive _until_ our mutex is released.

        ::std::vector< Reference< XInterface > > aEnsureAlive;

        // ----- <mutex_lock> ---------------------------------------------
        {
            ::osl::MutexGuard aGuard( s_aMutex );

            ClientMap::iterator aClientPos;
            if ( !implLookupClient( _nClient, aClientPos ) )
                // already asserted in implLookupClient
                return;

            // remove it from the clients map
            delete aClientPos->second;
            s_pNotifier->m_aClients.erase( aClientPos );

            // remove any other events which are pending for this client
            implRemoveEventsForClient( _nClient, aEnsureAlive );

            // cleanup the thread if we do not have clients anymore
            implCleanupNotifier( );
        }
        // ----- </mutex_lock> ---------------------------------------------

        // here, aEnsureAlive is cleared, and here it doesn't matter anymore if it's the last
        // reference to the contained components, as our mutex is not locked here ....
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::revokeClientNotifyDisposing( const TClientId _nClient,
            const Reference< XInterface >& _rxEventSource ) SAL_THROW( ( ) )
    {
        ::std::vector< Reference< XInterface > > aEnsureAlive;

        // ----- <mutex_lock> ---------------------------------------------
        {
            ::osl::MutexGuard aGuard( s_aMutex );

            ClientMap::iterator aClientPos;
            if ( !implLookupClient( _nClient, aClientPos ) )
                // already asserted in implLookupClient
                return;

            // move the client from the "regular clients" to the "disposed clients" map
            // from then on, no events for this client will be accepted anymore
        #ifdef _DEBUG
            ::std::pair< ClientMap::iterator, bool > aInsertResult =
        #endif
            s_pNotifier->m_aDisposedClients.insert( ClientMap::value_type( _nClient, aClientPos->second ) );
            OSL_ENSURE( aInsertResult.second, "AccessibleEventNotifier::revokeClientNotifyDisposing: client was already disposed!" );
                // is this asserts, then there already was an entry for _nClient in m_aDisposedClients, which means
                // somebody already called notifyDisposing with this id
            s_pNotifier->m_aClients.erase( aClientPos );

            // before we add the "disposing" event to the queue, we remove all other events for this client
            implRemoveEventsForClient( _nClient, aEnsureAlive );

            // push back a "disposing" event for this client
            AccessibleEventObject aDisposalEvent;
            aDisposalEvent.Source = _rxEventSource;
            aDisposalEvent.EventId = -1;        // this indicates "disposal"

            // add the event to the queue
            implPushBackEvent( _nClient, aDisposalEvent );
        }
        // ----- </mutex_lock> --------------------------------------------

        // here, aEnsureAlive is cleared, and here it doesn't matter anymore if it's the last
        // reference to the contained components, as our mutex is not locked here ....
    }

    //---------------------------------------------------------------------
    sal_Int32 AccessibleEventNotifier::addEventListener(
        const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener ) SAL_THROW( ( ) )
    {
        ::osl::MutexGuard aGuard( s_aMutex );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            // already asserted in implLookupClient
            return 0;

        if ( _rxListener.is() )
            aClientPos->second->addInterface( _rxListener );

        return aClientPos->second->getLength();
    }

    //---------------------------------------------------------------------
    sal_Int32 AccessibleEventNotifier::removeEventListener(
        const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener ) SAL_THROW( ( ) )
    {
        ::osl::MutexGuard aGuard( s_aMutex );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            // already asserted in implLookupClient
            return 0;

        if ( _rxListener.is() )
            aClientPos->second->removeInterface( _rxListener );

        return aClientPos->second->getLength();
    }

    //---------------------------------------------------------------------
    Sequence< Reference< XInterface > > AccessibleEventNotifier::getEventListeners( const TClientId _nClient ) SAL_THROW( ( ) )
    {
        Sequence< Reference< XInterface > > aListeners;

        ::osl::MutexGuard aGuard( s_aMutex );

        ClientMap::iterator aClientPos;
        if ( implLookupClient( _nClient, aClientPos ) )
            aListeners = aClientPos->second->getElements();

        return aListeners;
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::addEvent( const TClientId _nClient, const AccessibleEventObject& _rEvent ) SAL_THROW( ( ) )
    {
        ::osl::MutexGuard aGuard( s_aMutex );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            // already asserted in implLookupClient
            return;

        // add the event to the queue
        implPushBackEvent( _nClient, _rEvent );
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::implPushBackEvent( const TClientId _nClient, const AccessibleEventObject& _rEvent )
    {
        OSL_PRECOND( s_pNotifier, "AccessibleEventNotifier::implPushBackEvent: invalid call!" );

        // add the event to the queue
        s_pNotifier->m_aEvents.push_back( ClientEvent( _nClient, _rEvent ) );

        // wake up the thread
        s_pNotifier->m_aEventGuard.set();
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 05.12.2002 11:05:26  fs
 ************************************************************************/

