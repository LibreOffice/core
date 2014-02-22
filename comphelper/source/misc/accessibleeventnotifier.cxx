/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
        assert(nId < upper->second); 
        if (nId + 1 == upper->second)
        {
            --upper->second; 
        }
        else
        {
            IntervalMap::iterator const lower(rFreeIntervals.lower_bound(nId));
            if (lower != rFreeIntervals.end() && lower->first == nId - 1)
            {
                
                rFreeIntervals.insert(::std::make_pair(nId, lower->second));
                rFreeIntervals.erase(lower);
            }
            else 
            {
                rFreeIntervals.insert(::std::make_pair(nId, nId));
            }
        }
        
        
    }

    
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
            ++iter->second; 
        }
        else
        {
            rFreeIntervals.erase(iter); 
        }

        assert(Clients::get().end() == Clients::get().find(nFreeId));

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
    static bool implLookupClient(
            const AccessibleEventNotifier::TClientId nClient,
            ClientMap::iterator& rPos )
    {
        
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

        
        TClientId nNewClientId = generateId( );

        
        ::cppu::OInterfaceContainerHelper *const pNewListeners =
            new ::cppu::OInterfaceContainerHelper( lclMutex::get() );
            
            
            
            

        
        Clients::get().insert( ClientMap::value_type( nNewClientId, pNewListeners ) );

        
        return nNewClientId;
    }

    
    void AccessibleEventNotifier::revokeClient( const TClientId _nClient )
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            
            return;

        
        delete aClientPos->second;
        Clients::get().erase( aClientPos );
        releaseId(_nClient);
    }

    
    void AccessibleEventNotifier::revokeClientNotifyDisposing( const TClientId _nClient,
            const Reference< XInterface >& _rxEventSource ) SAL_THROW( ( ) )
    {
        ::cppu::OInterfaceContainerHelper* pListeners(0);

        {
            
            ::osl::MutexGuard aGuard( lclMutex::get() );

            ClientMap::iterator aClientPos;
            if (!implLookupClient(_nClient, aClientPos))
                
                return;

            
            pListeners = aClientPos->second;

            
            
            
            
            Clients::get().erase(aClientPos);
            releaseId(_nClient);
        }

        
        EventObject aDisposalEvent;
        aDisposalEvent.Source = _rxEventSource;

        
        pListeners->disposeAndClear( aDisposalEvent );
        delete pListeners;
    }

    
    sal_Int32 AccessibleEventNotifier::addEventListener(
        const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener ) SAL_THROW( ( ) )
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            
            return 0;

        if ( _rxListener.is() )
            aClientPos->second->addInterface( _rxListener );

        return aClientPos->second->getLength();
    }

    
    sal_Int32 AccessibleEventNotifier::removeEventListener(
        const TClientId _nClient, const Reference< XAccessibleEventListener >& _rxListener ) SAL_THROW( ( ) )
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );

        ClientMap::iterator aClientPos;
        if ( !implLookupClient( _nClient, aClientPos ) )
            
            return 0;

        if ( _rxListener.is() )
            aClientPos->second->removeInterface( _rxListener );

        return aClientPos->second->getLength();
    }

    
    void AccessibleEventNotifier::addEvent( const TClientId _nClient, const AccessibleEventObject& _rEvent ) SAL_THROW( ( ) )
    {
        Sequence< Reference< XInterface > > aListeners;

        
        {
            ::osl::MutexGuard aGuard( lclMutex::get() );

            ClientMap::iterator aClientPos;
            if ( !implLookupClient( _nClient, aClientPos ) )
                
                return;

            
            aListeners = aClientPos->second->getElements();
        }
        

            
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
                
                
            }
            ++pListeners;
        }
    }


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
