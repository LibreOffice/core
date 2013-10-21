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

#include <comphelper/accessibleeventnotifier.hxx>
#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/guarding.hxx>

#include <map>

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
    typedef ::std::pair< AccessibleEventNotifier::TClientId,
            AccessibleEventObject > ClientEvent;

    typedef ::std::map< AccessibleEventNotifier::TClientId,
                ::cppu::OInterfaceContainerHelper*,
                ::std::less< AccessibleEventNotifier::TClientId > > ClientMap;

    struct lclMutex
        : public rtl::Static< ::osl::Mutex, lclMutex > {};
    struct Clients
        : public rtl::Static< ClientMap, Clients > {};

    /// generates a new client id
    static AccessibleEventNotifier::TClientId generateId()
    {
        AccessibleEventNotifier::TClientId nBiggestUsedId = 0;
        AccessibleEventNotifier::TClientId nFreeId = 0;

        // look through all registered clients until we find a "gap" in the ids

        // Note that the following relies on the fact the elements in the map
        // are traveled with ascending keys (aka client ids)
        ClientMap &rClients = Clients::get();
        for (   ClientMap::const_iterator aLookup = rClients.begin();
                aLookup != rClients.end();
                ++aLookup
            )
        {
            AccessibleEventNotifier::TClientId nCurrent = aLookup->first;
            OSL_ENSURE( nCurrent > nBiggestUsedId,
                "AccessibleEventNotifier::generateId: "
                "map is expected to be sorted ascending!" );

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

    /** looks up a client in our client map, asserts if it cannot find it or
        no event thread is present

        @precond
            to be called with our mutex locked

        @param nClient
            the id of the client to loopup
        @param rPos
            out-parameter for the position of the client in the client map

        @return
            <TRUE/> if and only if the client could be found and
            <arg>rPos</arg> has been filled with it's position
    */
    static sal_Bool implLookupClient(
            const AccessibleEventNotifier::TClientId nClient,
            ClientMap::iterator& rPos )
    {
        // look up this client
        ClientMap &rClients = Clients::get();
        rPos = rClients.find( nClient );
        OSL_ENSURE( rClients.end() != rPos,
            "AccessibleEventNotifier::implLookupClient: invalid client id "
            "(did you register your client?)!" );

        return ( rClients.end() != rPos );
    }
}

//.........................................................................
namespace comphelper
{
//.........................................................................

    //---------------------------------------------------------------------
    AccessibleEventNotifier::TClientId AccessibleEventNotifier::registerClient( )
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );

        // generate a new client id
        TClientId nNewClientId = generateId( );

        // the event listeners for the new client
        ::cppu::OInterfaceContainerHelper *const pNewListeners =
            new ::cppu::OInterfaceContainerHelper( lclMutex::get() );
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
        ::cppu::OInterfaceContainerHelper* pListeners(0);

        {
            // rhbz#1001768 drop the mutex before calling disposeAndClear
            ::osl::MutexGuard aGuard( lclMutex::get() );

            ClientMap::iterator aClientPos;
            if (!implLookupClient(_nClient, aClientPos))
                // already asserted in implLookupClient
                return;

            // notify the listeners
            pListeners = aClientPos->second;

            // we do not need the entry in the clients map anymore
            // (do this before actually notifying, because some client
            // implementations have re-entrance problems and call into
            // revokeClient while we are notifying from here)
            Clients::get().erase(aClientPos);
        }

        // notify the "disposing" event for this client
        EventObject aDisposalEvent;
        aDisposalEvent.Source = _rxEventSource;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
