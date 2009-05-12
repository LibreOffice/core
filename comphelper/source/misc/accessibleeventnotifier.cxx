/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibleeventnotifier.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_comphelper.hxx"
#include <comphelper/accessibleeventnotifier.hxx>
#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <comphelper/guarding.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

//=====================================================================
//= AccessibleEventNotifier
//=====================================================================
//---------------------------------------------------------------------
namespace
{
    struct lclMutex
        : public rtl::Static< ::osl::Mutex, lclMutex > {};
    struct Clients
        : public rtl::Static< AccessibleEventNotifier::ClientMap, Clients > {};
}

//.........................................................................
namespace comphelper
{
//.........................................................................

    //---------------------------------------------------------------------
    AccessibleEventNotifier::TClientId AccessibleEventNotifier::generateId()
    {
        TClientId nBiggestUsedId = 0;
        TClientId nFreeId = 0;

        // look through all registered clients until we find a "gap" in the ids

        // Note that the following relies on the fact the elements in the map are traveled with
        // ascending keys (aka client ids)
        AccessibleEventNotifier::ClientMap &rClients = Clients::get();
        for (   ClientMap::const_iterator aLookup = rClients.begin();
                aLookup != rClients.end();
                ++aLookup
            )
        {
            TClientId nCurrent = aLookup->first;
            OSL_ENSURE( nCurrent > nBiggestUsedId, "AccessibleEventNotifier::generateId: map is expected to be sorted ascending!" );

            if ( nCurrent - nBiggestUsedId > 1 )
            {   // found a "gap"
                nFreeId = nBiggestUsedId + 1;
                break;
            }

            nBiggestUsedId = nCurrent;
        }

        if ( !nFreeId )
            nFreeId = nBiggestUsedId + 1;

        OSL_ENSURE( rClients.end() == rClients.find( nFreeId ),
            "AccessibleEventNotifier::generateId: algorithm broken!" );

        return nFreeId;
    }

    //---------------------------------------------------------------------
    AccessibleEventNotifier::TClientId AccessibleEventNotifier::registerClient( )
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );

        // generate a new client id
        TClientId nNewClientId = generateId( );

        // the event listeners for the new client
        EventListeners* pNewListeners = new EventListeners( lclMutex::get() );
            // note that we're using our own mutex here, so the listener containers for all
            // our clients share this same mutex.
            // this is a reminiscense to the days where the notifier was asynchronous. Today this is
            // completely nonsense, and potentially slowing down the Office me thinks ...

        // add the client
        Clients::get().insert( ClientMap::value_type( nNewClientId, pNewListeners ) );

        // outta here
        return nNewClientId;
    }

    //---------------------------------------------------------------------
    sal_Bool AccessibleEventNotifier::implLookupClient( const TClientId _nClient, ClientMap::iterator& _rPos )
    {
        // look up this client
        AccessibleEventNotifier::ClientMap &rClients = Clients::get();
        _rPos = rClients.find( _nClient );
        OSL_ENSURE( rClients.end() != _rPos, "AccessibleEventNotifier::implLookupClient: invalid client id (did you register your client?)!" );

        return ( rClients.end() != _rPos );
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::revokeClient( const TClientId _nClient )
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            // already asserted in implLookupClient
            return;

        // remove it from the clients map
        delete aClientPos->second;
        Clients::get().erase( aClientPos );
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::revokeClientNotifyDisposing( const TClientId _nClient,
            const Reference< XInterface >& _rxEventSource ) SAL_THROW( ( ) )
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            // already asserted in implLookupClient
            return;

        // notify the "disposing" event for this client
        EventObject aDisposalEvent;
        aDisposalEvent.Source = _rxEventSource;

        // notify the listeners
        EventListeners* pListeners = aClientPos->second;

        // we do not need the entry in the clients map anymore
        // (do this before actually notifying, because some client implementations have re-entrance
        // problems and call into revokeClient while we are notifying from hereing)
        Clients::get().erase( aClientPos );

        // now really do the notification
        pListeners->disposeAndClear( aDisposalEvent );
        delete pListeners;

    }

    //---------------------------------------------------------------------
    sal_Int32 AccessibleEventNotifier::addEventListener(
        const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener ) SAL_THROW( ( ) )
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );

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
        ::osl::MutexGuard aGuard( lclMutex::get() );

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

        ::osl::MutexGuard aGuard( lclMutex::get() );

        ClientMap::iterator aClientPos;
        if ( implLookupClient( _nClient, aClientPos ) )
            aListeners = aClientPos->second->getElements();

        return aListeners;
    }

    //---------------------------------------------------------------------
    void AccessibleEventNotifier::addEvent( const TClientId _nClient, const AccessibleEventObject& _rEvent ) SAL_THROW( ( ) )
    {
        Sequence< Reference< XInterface > > aListeners;

        // --- <mutex lock> -------------------------------
        {
            ::osl::MutexGuard aGuard( lclMutex::get() );

            ClientMap::iterator aClientPos;
            if ( !implLookupClient( _nClient, aClientPos ) )
                // already asserted in implLookupClient
                return;

            // since we're synchronous, again, we want to notify immediately
            aListeners = aClientPos->second->getElements();
        }
        // --- </mutex lock> ------------------------------

            // default handling: loop through all listeners, and notify them
        const Reference< XInterface >* pListeners = aListeners.getConstArray();
        const Reference< XInterface >* pListenersEnd = pListeners + aListeners.getLength();
        while ( pListeners != pListenersEnd )
        {
            try
            {
                static_cast< XAccessibleEventListener* >( pListeners->get() )->notifyEvent( _rEvent );
            }
            catch( const Exception& )
            {
                // no assertion, because a broken access remote bridge or something like this
                // can cause this exception
            }
            ++pListeners;
        }
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

