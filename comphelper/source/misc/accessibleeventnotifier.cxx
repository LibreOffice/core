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
#include <limits>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

namespace
{
    typedef ::std::pair< AccessibleEventNotifier::TClientId,
            AccessibleEventObject > ClientEvent;

    typedef ::std::map< AccessibleEventNotifier::TClientId,
                ::cppu::OInterfaceContainerHelper*,
                ::std::less< AccessibleEventNotifier::TClientId > > ClientMap;

    /// key is the end of the interval, value is the start of the interval
    typedef ::std::map<AccessibleEventNotifier::TClientId,
                AccessibleEventNotifier::TClientId> IntervalMap;

    struct lclMutex
        : public rtl::Static< ::osl::Mutex, lclMutex > {};
    struct Clients
        : public rtl::Static< ClientMap, Clients > {};
    struct FreeIntervals
        : public rtl::StaticWithInit<IntervalMap, FreeIntervals> {
            IntervalMap operator() () {
                IntervalMap map;
                map.insert(::std::make_pair(
                    ::std::numeric_limits<AccessibleEventNotifier::TClientId>::max(), 1));
                return map;
            }
        };

    static void releaseId(AccessibleEventNotifier::TClientId const nId)
    {
        IntervalMap & rFreeIntervals(FreeIntervals::get());
        IntervalMap::iterator const upper(rFreeIntervals.upper_bound(nId));
        assert(upper != rFreeIntervals.end());
        assert(nId < upper->second); // second is start of the interval!
        if (nId + 1 == upper->second)
        {
            --upper->second; // add nId to existing interval
        }
        else
        {
            IntervalMap::iterator const lower(rFreeIntervals.lower_bound(nId));
            if (lower != rFreeIntervals.end() && lower->first == nId - 1)
            {
                // add nId by replacing lower with new merged entry
                rFreeIntervals.insert(::std::make_pair(nId, lower->second));
                rFreeIntervals.erase(lower);
            }
            else // otherwise just add new 1-element interval
            {
                rFreeIntervals.insert(::std::make_pair(nId, nId));
            }
        }
        // currently it's not checked whether intervals can be merged now
        // hopefully that won't be a problem in practice
    }

    /// generates a new client id
    static AccessibleEventNotifier::TClientId generateId()
    {
        IntervalMap & rFreeIntervals(FreeIntervals::get());
        assert(!rFreeIntervals.empty());
        IntervalMap::iterator const iter(rFreeIntervals.begin());
        AccessibleEventNotifier::TClientId const nFirst = iter->first;
        AccessibleEventNotifier::TClientId const nFreeId = iter->second;
        assert(nFreeId <= nFirst);
        if (nFreeId != nFirst)
        {
            ++iter->second; // remove nFreeId from interval
        }
        else
        {
            rFreeIntervals.erase(iter); // remove 1-element interval
        }

        assert(Clients::get().end() == Clients::get().find(nFreeId));

        return nFreeId;
    }

    /** looks up a client in our client map, asserts if it cannot find it or
        no event thread is present

        @precond
            to be called with our mutex locked

        @param nClient
            the id of the client to lookup
        @param rPos
            out-parameter for the position of the client in the client map

        @return
            <TRUE/> if and only if the client could be found and
            <arg>rPos</arg> has been filled with it's position
    */
    static bool implLookupClient(
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


namespace comphelper
{



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
        releaseId(_nClient);
    }


    void AccessibleEventNotifier::revokeClientNotifyDisposing( const TClientId _nClient,
            const Reference< XInterface >& _rxEventSource )
    {
        ::cppu::OInterfaceContainerHelper* pListeners(nullptr);

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
            releaseId(_nClient);
        }

        // notify the "disposing" event for this client
        EventObject aDisposalEvent;
        aDisposalEvent.Source = _rxEventSource;

        // now really do the notification
        pListeners->disposeAndClear( aDisposalEvent );
        delete pListeners;
    }


    sal_Int32 AccessibleEventNotifier::addEventListener(
        const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener )
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


    sal_Int32 AccessibleEventNotifier::removeEventListener(
        const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener )
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


    void AccessibleEventNotifier::addEvent( const TClientId _nClient, const AccessibleEventObject& _rEvent )
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


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
