/*************************************************************************
 *
 *  $RCSfile: unload.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-06-11 15:57:04 $
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
#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <hash_map>
#include <list>
#include <queue>

using namespace ::std;
using namespace ::rtl;
using namespace ::osl;

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
    sal_Bool retval;
    if ( time1->Seconds > time2->Seconds)
        retval= sal_True;
    else if ( time1->Seconds == time2->Seconds)
    {
        if( time1->Nanosec > time2->Nanosec)
            retval= sal_True;
    }
    else
        retval= sal_False;
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

static Mutex* getUnloadingMutex()
{
    static Mutex* pMutex= NULL;

    if( ! pMutex)
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pMutex)
        {
            static Mutex aMutex;
            pMutex= &aMutex;
        }
    }
    return pMutex;
}

extern "C" void rtl_moduleCount_acquire(rtl_ModuleCount * that )
{
    rtl_StandardModuleCount* pMod= (rtl_StandardModuleCount*)that;
    osl_incrementInterlockedCount( &pMod->counter);
}

extern "C" void rtl_moduleCount_release( rtl_ModuleCount * that )
{
    rtl_StandardModuleCount* pMod= (rtl_StandardModuleCount*)that;
    OSL_ENSURE( pMod->counter >0 , "library counter incorrect" );
    osl_decrementInterlockedCount( &pMod->counter);
    if( pMod->counter == 0)
    {
        MutexGuard guard( getUnloadingMutex());

        if( sal_False == osl_getSystemTime( &pMod->unusedSince) )
        {// set the time to 0 if we could not get the time
            pMod->unusedSince.Seconds= 0;
            pMod->unusedSince.Nanosec= 0;
        }
    }
}


struct hashModule
{
    size_t operator()( const oslModule& rkey) const
    {
        return (size_t)rkey;
    }
};

struct equalModule
{
    bool operator()(const oslModule& m1, const oslModule& s2) const
    {
        return m1 == s2;
    }
};

typedef hash_map<oslModule,
        pair<sal_uInt32,component_canUnloadFunc>,
        hashModule, equalModule>
        ModuleMap;
typedef ModuleMap::iterator Mod_IT;
ModuleMap g_moduleMap;


extern "C" sal_Bool rtl_moduleCount_canUnload( rtl_StandardModuleCount * that, TimeValue * libUnused)
{
    sal_Bool retVal= sal_False;
    if( that->counter== 0)
    {
        MutexGuard guard( getUnloadingMutex());
        if( libUnused && that->counter == 0)
        {

            rtl_copyMemory( libUnused, &that->unusedSince,  sizeof( TimeValue));
        }
    }
    return that->counter == 0;
}


extern "C" sal_Bool SAL_CALL rtl_registerModuleForUnloading( oslModule module)
{
    MutexGuard guard( getUnloadingMutex());
    sal_Bool ret= sal_True;
    // If the module has been registered before, then find it and increment
    // its reference cout
    Mod_IT it= g_moduleMap.find( module);
    if( it != g_moduleMap.end())
    {
        //module already registered, increment ref count
        it->second.first++;
    }
    else
    {
        //Test if the module supports unloading, that is, it exports
        // component_canUnload
        OUString name(RTL_CONSTASCII_USTRINGPARAM( COMPONENT_CANUNLOAD));
        component_canUnloadFunc  pFunc= ( component_canUnloadFunc)osl_getSymbol( module, name.pData);
        if( pFunc)
        {
            //register module for the first time, set ref count to 1
            g_moduleMap[module]= make_pair((sal_uInt32)1, pFunc);
        }
        else
            ret= sal_False;
    }
    return ret;
}

extern "C" void SAL_CALL rtl_unregisterModuleForUnloading( oslModule module)
{
    MutexGuard guard( getUnloadingMutex());
    Mod_IT it= g_moduleMap.find( module);
    if( it != g_moduleMap.end() )
    {
        // The module is registered, decrement ref count.
        it->second.first --;

        // If the refcount == 0 then remove the module from the map
        if( it->second.first == 0)
            g_moduleMap.erase( it);
    }

}

extern "C" void SAL_CALL rtl_unloadUnusedModules( TimeValue* libUnused)
{
    MutexGuard guard( getUnloadingMutex());
    list<oslModule> unloadedModulesList;
    Mod_IT it_e= g_moduleMap.end();

    // notify all listeners
    rtl_notifyUnloadingListeners();
    // prepare default TimeValue if argumetn is NULL
    TimeValue nullTime={0,0};
    TimeValue* pLibUnused= libUnused? libUnused : &nullTime;
    for( Mod_IT it= g_moduleMap.begin(); it != it_e; it++)
    {
        //can the module be unloaded?
        component_canUnloadFunc func= it->second.second;
        TimeValue unusedSince= {0, 0};
        if( func( &unusedSince) )
        {   // module can be unloaded if it has not been used at least for the time
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
    for( list<oslModule>::iterator un_it= unloadedModulesList.begin();
            un_it != unloadedModulesList.end(); un_it++)
    {
        g_moduleMap.erase( *un_it);
    }
}


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

struct equalListener
{
    bool operator()(const sal_Int32& m1, const sal_Int32& s2) const
    {
        return m1 == s2;
    }
};

typedef hash_map<sal_Int32,
        pair<rtl_unloadingListenerFunc, void*>,
        hashListener, equalListener>
        ListenerMap;
typedef ListenerMap::iterator Lis_IT;
ListenerMap g_listenerMap;


// This queue contains cookies which have been passed out by rtl_addUnloadingListener and
// which have been regainded by rtl_removeUnloadingListener. When rtl_addUnloadingListener
// is called then a cookie has to be returned. First we look into the set if there is one
// availabe. Otherwise a new cookie will be provided.
// not a new value is returned.
static queue<sal_Int32> g_regainedCookies;

static sal_Int32 getCookie()
{
    static sal_Int32 cookieValue= 1;

    sal_Int32 retval;
    if( g_regainedCookies.empty() )
        retval= cookieValue++;
    else
    {
        retval= g_regainedCookies.front();
        g_regainedCookies.pop();
    }
    return retval;
}

static inline void recycleCookie( sal_Int32 i)
{
    g_regainedCookies.push( i);
}


// calling the function twice with the same arguments will return tow different cookies.
// The listener will then notified twice.
extern "C"
sal_Int32 SAL_CALL rtl_addUnloadingListener( rtl_unloadingListenerFunc callback, void* _this)
{
    MutexGuard guard( getUnloadingMutex());
    sal_Int32 cookie= getCookie();
    g_listenerMap[ cookie]= make_pair( callback, _this);
    return cookie;
}


extern "C"
void SAL_CALL rtl_removeUnloadingListener( sal_Int32 cookie )
{
    MutexGuard guard( getUnloadingMutex());
    size_t removedElements= g_listenerMap.erase( cookie);
    if( removedElements )
        recycleCookie( cookie);
}


static void rtl_notifyUnloadingListeners()
{
    for( Lis_IT it= g_listenerMap.begin(); it != g_listenerMap.end(); it++)
    {
        rtl_unloadingListenerFunc callbackFunc= it->second.first;
        callbackFunc( it->second.second);
    }
}
