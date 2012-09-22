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

#include <string.h>

#include <rtl/unload.h>
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <rtl/instance.hxx>
#include <osl/mutex.hxx>
#include <boost/unordered_map.hpp>
#include "rtl/allocator.hxx"

#include <functional>
#include <list>
#include <deque>

using osl::MutexGuard;

//----------------------------------------------------------------------------

#ifndef DISABLE_DYNLOADING

static void rtl_notifyUnloadingListeners();

static sal_Bool isEqualTimeValue ( const TimeValue* time1,  const TimeValue* time2)
{
    if( time1->Seconds == time2->Seconds &&
        time1->Nanosec == time2->Nanosec)
        return sal_True;
    else
        return sal_False;
}

static sal_Bool isGreaterTimeValue(  const TimeValue* time1,  const TimeValue* time2)
{
    sal_Bool retval= sal_False;
    if ( time1->Seconds > time2->Seconds)
        retval= sal_True;
    else if ( time1->Seconds == time2->Seconds)
    {
        if( time1->Nanosec > time2->Nanosec)
            retval= sal_True;
    }
    return retval;
}

static sal_Bool isGreaterEqualTimeValue( const TimeValue* time1, const TimeValue* time2)
{
    if( isEqualTimeValue( time1, time2) )
        return sal_True;
    else if( isGreaterTimeValue( time1, time2))
        return sal_True;
    else
        return sal_False;
}

static void addTimeValue( const TimeValue* value1, const TimeValue* value2, TimeValue* result)
{
    sal_uInt64 sum;
    result->Nanosec=0;
    result->Seconds=0;

    sum= value1->Nanosec + value2->Nanosec;
    if( sum >= 1000000000 )
    {
        result->Seconds=1;
        sum -= 1000000000;
    }
    result->Nanosec= (sal_uInt32)sum;
    result->Seconds += value1->Seconds + value2->Seconds;
}


static sal_Bool hasEnoughTimePassed( const TimeValue* unusedSince, const TimeValue* timespan)
{
    sal_Bool retval= sal_False;
    TimeValue currentTime;
    if( osl_getSystemTime( &currentTime))
    {
        TimeValue addedTime;
        addTimeValue( unusedSince, timespan, &addedTime);
        if( isGreaterEqualTimeValue( &currentTime, &addedTime))
            retval= sal_True;
    }

    return retval;
}

namespace
{
    class theUnloadingMutex : public rtl::Static<osl::Mutex, theUnloadingMutex>{};
}

static osl::Mutex& getUnloadingMutex()
{
    return theUnloadingMutex::get();
}

#endif

extern "C" void rtl_moduleCount_acquire(rtl_ModuleCount * that )
{
#ifdef DISABLE_DYNLOADING
    (void) that;
#else
    rtl_StandardModuleCount* pMod= (rtl_StandardModuleCount*)that;
    osl_atomic_increment( &pMod->counter);
#endif
}

extern "C" void rtl_moduleCount_release( rtl_ModuleCount * that )
{
#ifdef DISABLE_DYNLOADING
    (void) that;
#else
    rtl_StandardModuleCount* pMod= (rtl_StandardModuleCount*)that;
    OSL_ENSURE( pMod->counter >0 , "library counter incorrect" );
    osl_atomic_decrement( &pMod->counter);
    if( pMod->counter == 0)
    {
        MutexGuard guard( getUnloadingMutex());

        if( sal_False == osl_getSystemTime( &pMod->unusedSince) )
        {
            // set the time to 0 if we could not get the time
            pMod->unusedSince.Seconds= 0;
            pMod->unusedSince.Nanosec= 0;
        }
    }
#endif
}

#ifndef DISABLE_DYNLOADING

struct hashModule
{
    size_t operator()( const oslModule& rkey) const
    {
        return (size_t)rkey;
    }
};

typedef boost::unordered_map<
    oslModule,
    std::pair<sal_uInt32, component_canUnloadFunc>,
    hashModule,
    std::equal_to<oslModule>,
    rtl::Allocator<oslModule>
> ModuleMap;

typedef ModuleMap::iterator Mod_IT;

static ModuleMap& getModuleMap()
{
    static ModuleMap * g_pMap= NULL;
    if (!g_pMap)
    {
        MutexGuard guard( getUnloadingMutex() );
        if (!g_pMap)
        {
            static ModuleMap g_aModuleMap;
            g_pMap= &g_aModuleMap;
        }
    }
    return *g_pMap;
}

#endif

extern "C" sal_Bool rtl_moduleCount_canUnload( rtl_StandardModuleCount * that, TimeValue * libUnused)
{
#ifdef DISABLE_DYNLOADING
    (void) that;
    (void) libUnused;
    return sal_False;
#else
    if (that->counter == 0)
    {
        MutexGuard guard( getUnloadingMutex());
        if (libUnused && (that->counter == 0))
        {
            memcpy(libUnused, &that->unusedSince, sizeof(TimeValue));
        }
    }
    return (that->counter == 0);
#endif
}


extern "C" sal_Bool SAL_CALL rtl_registerModuleForUnloading( oslModule module)
{
#ifdef DISABLE_DYNLOADING
    (void) module;
    return sal_False;
#else
    MutexGuard guard( getUnloadingMutex());
    ModuleMap& moduleMap= getModuleMap();
    sal_Bool ret= sal_True;

    // If the module has been registered before, then find it and increment
    // its reference cout
    Mod_IT it= moduleMap.find( module);
    if( it != moduleMap.end())
    {
        //module already registered, increment ref count
        it->second.first++;
    }
    else
    {
        // Test if the module supports unloading (exports component_canUnload)
        rtl::OUString name(RTL_CONSTASCII_USTRINGPARAM( COMPONENT_CANUNLOAD));
        component_canUnloadFunc pFunc=
            (component_canUnloadFunc)osl_getFunctionSymbol( module, name.pData);
        if (pFunc)
        {
            //register module for the first time, set ref count to 1
            moduleMap[module]= std::make_pair((sal_uInt32)1, pFunc);
        }
        else
            ret= sal_False;
    }
    return ret;
#endif
}

extern "C" void SAL_CALL rtl_unregisterModuleForUnloading( oslModule module)
{
#ifdef DISABLE_DYNLOADING
    (void) module;
#else
    MutexGuard guard( getUnloadingMutex());

    ModuleMap& moduleMap= getModuleMap();
    Mod_IT it= moduleMap.find( module);
    if( it != moduleMap.end() )
    {
        // The module is registered, decrement ref count.
        it->second.first --;

        // If the refcount == 0 then remove the module from the map
        if( it->second.first == 0)
            moduleMap.erase( it);
    }
#endif
}

extern "C" void SAL_CALL rtl_unloadUnusedModules( TimeValue* libUnused)
{
#ifdef DISABLE_DYNLOADING
    (void) libUnused;
#else
    MutexGuard guard( getUnloadingMutex());

    typedef std::list< oslModule, rtl::Allocator<oslModule> > list_type;
    list_type unloadedModulesList;

    ModuleMap& moduleMap= getModuleMap();
    Mod_IT it_e= moduleMap.end();

    // notify all listeners
    rtl_notifyUnloadingListeners();

    // prepare default TimeValue if argumetn is NULL
    TimeValue nullTime={0,0};
    TimeValue* pLibUnused= libUnused? libUnused : &nullTime;

    Mod_IT it= moduleMap.begin();
    for (; it != it_e; ++it)
    {
        //can the module be unloaded?
        component_canUnloadFunc func= it->second.second;
        TimeValue unusedSince= {0, 0};

        if( func( &unusedSince) )
        {
            // module can be unloaded if it has not been used at least for the time
            // specified by the argument libUnused
            if( hasEnoughTimePassed( &unusedSince, pLibUnused))
            {
                // get the reference count and unload the module as many times
                sal_uInt32 refCount= it->second.first;

                for ( sal_uInt32 i=0; i < refCount; i++)
                    osl_unloadModule( it->first);

                // mark the module for later removal
                unloadedModulesList.push_front( it->first);
            }
        }
    }

    // remove all entries containing invalid (unloaded) modules
    list_type::const_iterator un_it= unloadedModulesList.begin();
    for (; un_it != unloadedModulesList.end(); ++un_it)
    {
        moduleMap.erase( *un_it);
    }
#endif
}

#ifndef DISABLE_DYNLOADING

// ==============================================================================
// Unloading Listener Administration
//===============================================================================
struct hashListener
{
    size_t operator()( const sal_Int32& rkey) const
    {
        return (size_t)rkey;
    }
};

typedef boost::unordered_map<
    sal_Int32,
    std::pair<rtl_unloadingListenerFunc, void*>,
    hashListener,
    std::equal_to<sal_Int32>,
    rtl::Allocator<sal_Int32>
> ListenerMap;

typedef ListenerMap::iterator Lis_IT;

static ListenerMap& getListenerMap()
{
    static ListenerMap * g_pListeners= NULL;
    if (!g_pListeners)
    {
        MutexGuard guard( getUnloadingMutex() );
        if (!g_pListeners)
        {
            static ListenerMap g_aListenerMap;
            g_pListeners= &g_aListenerMap;
        }
    }
    return *g_pListeners;
}


// This queue contains cookies which have been passed out by rtl_addUnloadingListener and
// which have been regainded by rtl_removeUnloadingListener. When rtl_addUnloadingListener
// is called then a cookie has to be returned. First we look into the set if there is one
// availabe. Otherwise a new cookie will be provided.
// not a new value is returned.

typedef std::deque<
    sal_Int32,
    rtl::Allocator<sal_Int32>
> queue_type;

static queue_type& getCookieQueue()
{
    static queue_type * g_pCookies= NULL;
    if (!g_pCookies)
    {
        MutexGuard guard( getUnloadingMutex() );
        if (!g_pCookies)
        {
            static queue_type g_aCookieQueue;
            g_pCookies= &g_aCookieQueue;
        }
    }
    return *g_pCookies;
}

static sal_Int32 getCookie()
{
    static sal_Int32 cookieValue= 1;

    sal_Int32 retval;
    queue_type& regainedCookies= getCookieQueue();
    if( regainedCookies.empty() )
        retval= cookieValue++;
    else
    {
        retval= regainedCookies.front();
        regainedCookies.pop_front();
    }
    return retval;
}

static inline void recycleCookie( sal_Int32 i)
{
    getCookieQueue().push_back(i);
}


#endif

// calling the function twice with the same arguments will return tow different cookies.
// The listener will then notified twice.

extern "C"
sal_Int32 SAL_CALL rtl_addUnloadingListener( rtl_unloadingListenerFunc callback, void* _this)
{
#ifdef DISABLE_DYNLOADING
    (void) callback;
    (void) _this;
    return 0;
#else
    MutexGuard guard( getUnloadingMutex());

    sal_Int32 cookie= getCookie();
    ListenerMap& listenerMap= getListenerMap();
    listenerMap[ cookie]= std::make_pair( callback, _this);
    return cookie;
#endif
}


extern "C"
void SAL_CALL rtl_removeUnloadingListener( sal_Int32 cookie )
{
#ifdef DISABLE_DYNLOADING
    (void) cookie;
#else
    MutexGuard guard( getUnloadingMutex());

    ListenerMap& listenerMap= getListenerMap();
    size_t removedElements= listenerMap.erase( cookie);
    if( removedElements )
        recycleCookie( cookie);
#endif
}

#ifndef DISABLE_DYNLOADING

static void rtl_notifyUnloadingListeners()
{
    ListenerMap& listenerMap= getListenerMap();
    for( Lis_IT it= listenerMap.begin(); it != listenerMap.end(); ++it)
    {
        rtl_unloadingListenerFunc callbackFunc= it->second.first;
        callbackFunc( it->second.second);
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
