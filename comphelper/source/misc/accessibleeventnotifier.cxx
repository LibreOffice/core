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
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <comphelper/interfacecontainer4.hxx>

#include <map>
#include <memory>
#include <limits>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

namespace {

typedef std::pair< AccessibleEventNotifier::TClientId,
        AccessibleEventObject > ClientEvent;

typedef ::comphelper::OInterfaceContainerHelper4<XAccessibleEventListener> ListenerContainer;
typedef std::map< AccessibleEventNotifier::TClientId, ListenerContainer* > ClientMap;

/// key is the end of the interval, value is the start of the interval
typedef std::map<AccessibleEventNotifier::TClientId,
            AccessibleEventNotifier::TClientId> IntervalMap;

std::mutex& GetLocalMutex()
{
    static std::mutex MUTEX;
    return MUTEX;
}

ClientMap gaClients;

IntervalMap& GetFreeIntervals()
{
    static IntervalMap MAP =
    []()
    {
        IntervalMap map;
        map.insert(std::make_pair(
            std::numeric_limits<AccessibleEventNotifier::TClientId>::max(), 1));
        return map;
    }();
    return MAP;
}

void releaseId(AccessibleEventNotifier::TClientId const nId)
{
    IntervalMap & rFreeIntervals(GetFreeIntervals());
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
            rFreeIntervals.insert(std::make_pair(nId, lower->second));
            rFreeIntervals.erase(lower);
        }
        else // otherwise just add new 1-element interval
        {
            rFreeIntervals.insert(std::make_pair(nId, nId));
        }
    }
    // currently it's not checked whether intervals can be merged now
    // hopefully that won't be a problem in practice
}

/// generates a new client id
AccessibleEventNotifier::TClientId generateId()
{
    IntervalMap & rFreeIntervals(GetFreeIntervals());
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

    assert(gaClients.end() == gaClients.find(nFreeId));

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
        <arg>rPos</arg> has been filled with its position
*/
bool implLookupClient(
        const AccessibleEventNotifier::TClientId nClient,
        ClientMap::iterator& rPos )
{
    // look up this client
    ClientMap &rClients = gaClients;
    rPos = rClients.find( nClient );
    assert( rClients.end() != rPos &&
        "AccessibleEventNotifier::implLookupClient: invalid client id "
        "(did you register your client?)!" );

    return ( rClients.end() != rPos );
}

} // anonymous namespace

namespace comphelper {

AccessibleEventNotifier::TClientId AccessibleEventNotifier::registerClient()
{
    std::scoped_lock aGuard( GetLocalMutex() );

    // generate a new client id
    TClientId nNewClientId = generateId( );

    // the event listeners for the new client
    ListenerContainer * pNewListeners = new ListenerContainer();
    // add the client
    gaClients.emplace( nNewClientId, pNewListeners );

    // outta here
    return nNewClientId;
}

void AccessibleEventNotifier::revokeClient( const TClientId _nClient )
{
    std::scoped_lock aGuard( GetLocalMutex() );

    ClientMap::iterator aClientPos;
    if ( !implLookupClient( _nClient, aClientPos ) )
        // already asserted in implLookupClient
        return;

    // remove it from the clients map
    delete aClientPos->second;
    gaClients.erase( aClientPos );
    releaseId(_nClient);
}

void AccessibleEventNotifier::revokeClientNotifyDisposing(
    const TClientId _nClient, const Reference< XInterface >& _rxEventSource )
{
    std::unique_lock aGuard( GetLocalMutex() );

    ClientMap::iterator aClientPos;
    if (!implLookupClient(_nClient, aClientPos))
        // already asserted in implLookupClient
        return;

    // notify the listeners
    std::unique_ptr<ListenerContainer> pListeners(aClientPos->second);

    // we do not need the entry in the clients map anymore
    // (do this before actually notifying, because some client
    // implementations have re-entrance problems and call into
    // revokeClient while we are notifying from here)
    gaClients.erase(aClientPos);
    releaseId(_nClient);

    // notify the "disposing" event for this client
    EventObject aDisposalEvent;
    aDisposalEvent.Source = _rxEventSource;

    // now really do the notification
    pListeners->disposeAndClear( aGuard, aDisposalEvent );
}

sal_Int32 AccessibleEventNotifier::addEventListener(
    const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener )
{
    std::unique_lock aGuard( GetLocalMutex() );

    ClientMap::iterator aClientPos;
    if ( !implLookupClient( _nClient, aClientPos ) )
        // already asserted in implLookupClient
        return 0;

    if ( _rxListener.is() )
        aClientPos->second->addInterface( aGuard, _rxListener );

    return aClientPos->second->getLength(aGuard);
}

sal_Int32 AccessibleEventNotifier::removeEventListener(
    const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener )
{
    std::unique_lock aGuard( GetLocalMutex() );

    ClientMap::iterator aClientPos;
    if ( !implLookupClient( _nClient, aClientPos ) )
        // already asserted in implLookupClient
        return 0;

    if ( _rxListener.is() )
        aClientPos->second->removeInterface( aGuard,  _rxListener );

    return aClientPos->second->getLength(aGuard);
}

void AccessibleEventNotifier::addEvent( const TClientId _nClient, const AccessibleEventObject& _rEvent )
{
    std::vector< Reference< XAccessibleEventListener > > aListeners;

    {
        std::unique_lock aGuard( GetLocalMutex() );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            // already asserted in implLookupClient
            return;

        // since we're synchronous, again, we want to notify immediately
        aListeners = aClientPos->second->getElements(aGuard);
    }

    // default handling: loop through all listeners, and notify them
    for ( const auto& rListener : aListeners )
    {
        try
        {
            rListener->notifyEvent( _rEvent );
        }
        catch( const Exception& )
        {
            // no assertion, because a broken access remote bridge or something like this
            // can cause this exception
        }
    }
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
