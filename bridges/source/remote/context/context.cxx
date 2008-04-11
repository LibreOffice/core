/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: context.cxx,v $
 * $Revision: 1.12 $
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
#include "precompiled_bridges.hxx"
#include <stdio.h>
#include <string.h>
#include <list>
#include <hash_map>
#include <utility>

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <osl/mutex.hxx>

#include "rtl/ustring.hxx"

#include <bridges/remote/context.h>
#include <bridges/remote/remote.h>
#include <bridges/remote/connection.h>
#include <bridges/remote/counter.hxx>

using namespace ::std;
using namespace ::osl;
using namespace ::rtl;

namespace {

extern "C" typedef void * (SAL_CALL * MemAlloc)(sal_Size);

}

namespace remote_context
{

class remote_ContextImpl :
    public remote_Context
{
public:
    remote_ContextImpl( remote_Connection *pConnection,
                        rtl_uString *pIdStr,
                        rtl_uString *pDescription,
                        rtl_uString *pProtocol,
                        remote_InstanceProvider *pProvider );
    ~remote_ContextImpl();

    static void SAL_CALL thisAcquire( uno_Context * );
    static void SAL_CALL thisRelease( uno_Context * );
    static void * SAL_CALL thisQuery( uno_Context * , rtl_uString * );
    static void SAL_CALL thisAddDisposingListener( remote_Context * , remote_DisposingListener * );
    static void SAL_CALL thisRemoveDisposingListener( remote_Context *, remote_DisposingListener *);
    static void SAL_CALL thisDispose( remote_Context *);
public:
    oslInterlockedCount m_nRef;
    sal_Bool m_bDisposed;
    list < remote_DisposingListener * > m_lstListener;
    Mutex m_mutex;
};




struct equalOUString_Impl
{
    sal_Bool operator()(const OUString & s1, const OUString & s2) const
        { return s1 == s2; }
};

struct hashOUString_Impl
{
    size_t operator()(const OUString & rName) const
        { return rName.hashCode(); }
};

typedef hash_map
<
    OUString,
    void *,
    hashOUString_Impl,
    equalOUString_Impl
>
ContextMap;

#if OSL_DEBUG_LEVEL > 1
static MyCounter thisCounter( "DEBUG : Context" );
#endif

class ContextAdmin;

ContextAdmin *g_pTheContext = 0;



class ContextAdmin
{
public:
    static ContextAdmin *getInstance();

    // listener administration
    void addContextListener( remote_contextListenerFunc listener , void *pObject );
    void removeContextListener( remote_contextListenerFunc listener , void *pObject );

    void fire( sal_Int32 nRemoteContextMode,
               rtl_uString *sName,
               rtl_uString *sDescription );

    // context administration
    uno_Context *createAndRegisterContext(
        remote_Connection *pConnection,
        rtl_uString *pIdStr,
        rtl_uString *pDescription,
        rtl_uString *pProtocol,
        remote_InstanceProvider *pInstanceProvider );

    void revokeContext( uno_Context *pRemoteContext );

    uno_Context *get( rtl_uString *pHost );

    rtl_uString ** getConnectionList(
        sal_Int32 *pnStringCount, MemAlloc memAlloc );

private:
    ::osl::Mutex          m_mutex;

    ContextMap m_mapContext;

    typedef std::list< std::pair< remote_contextListenerFunc, void * > > List;
    List m_lstListener;
};

ContextAdmin *ContextAdmin::getInstance()
{
    if( ! g_pTheContext ) {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! g_pTheContext ) {
            //TODO  This memory is leaked; see #i63473# for when this should be
            // changed again:
            g_pTheContext = new ContextAdmin;
        }
    }
    return g_pTheContext;
}

void ContextAdmin::addContextListener( remote_contextListenerFunc listener , void  *pObject )
{
    ::osl::MutexGuard guard( m_mutex );

    m_lstListener.push_back( std::make_pair( listener, pObject ) );
}

void ContextAdmin::removeContextListener( remote_contextListenerFunc listener , void *pObject )
{
    ::osl::MutexGuard guard( m_mutex );

    for (List::iterator ii(m_lstListener.begin()); ii != m_lstListener.end();
         ++ii)
    {
        if (ii->first == listener && ii->second == pObject) {
            m_lstListener.erase( ii );
            break;
        }
    }
}

void ContextAdmin::fire(
    sal_Int32 nRemoteContextMode,
    rtl_uString *pName,
    rtl_uString *sDescription )
{
    List lst;
    {
        ::osl::MutexGuard guard( m_mutex );
        lst = m_lstListener;
    }
    for (List::iterator i(lst.begin()); i != lst.end(); ++i) {
        (i->first)(i->second, nRemoteContextMode, pName, sDescription);
    }
}

uno_Context *ContextAdmin::createAndRegisterContext( remote_Connection *pConnection,
                                                     rtl_uString *pIdStr,
                                                     rtl_uString *pDescription,
                                                     rtl_uString *pProtocol,
                                                     remote_InstanceProvider *pInstanceProvider )
{
    ::osl::MutexGuard guard( m_mutex );

    uno_Context *pContext = get( pIdStr );
    if( pContext )
    {
        pContext->release( pContext );
        return 0;
    }

    remote_ContextImpl *p = new remote_ContextImpl( pConnection,
                                                    pIdStr,
                                                    pDescription,
                                                    pProtocol,
                                                    pInstanceProvider );

    p->aBase.acquire( (uno_Context*) p );

    m_mapContext[ OUString( pIdStr) ] = (void*) p;

    fire( REMOTE_CONTEXT_CREATE , pIdStr , pDescription );
    return ( uno_Context * )p;
}


void ContextAdmin::revokeContext( uno_Context *pRemoteContext )
{
    ::osl::MutexGuard guard( m_mutex );

    remote_ContextImpl *p = ( remote_ContextImpl * ) pRemoteContext;

    ContextMap::iterator ii = m_mapContext.find(  p->m_pName );
    OSL_ASSERT( ii != m_mapContext.end() );
    m_mapContext.erase( ii );

    fire( REMOTE_CONTEXT_DESTROY , p->m_pName , p->m_pDescription );

}

uno_Context *ContextAdmin::get( rtl_uString *pHost )
{
    ::osl::MutexGuard guard( m_mutex );

    ContextMap::iterator ii = m_mapContext.find( OUString( (rtl_uString*)pHost ) );
    if( ii == m_mapContext.end() )
    {
        return 0;
    }

    uno_Context *p =  ( uno_Context * ) (*ii).second;
    p->acquire( p );
    return p;
}


rtl_uString ** ContextAdmin::getConnectionList(
    sal_Int32 *pnStringCount, MemAlloc memAlloc )
{
    ::osl::MutexGuard guard( m_mutex );

    *pnStringCount = m_mapContext.size();
    rtl_uString **ppReturn = ( rtl_uString ** )
        memAlloc( sizeof( rtl_uString * ) * m_mapContext.size() );
    memset( ppReturn , 0 , sizeof( rtl_uString * ) * m_mapContext.size() );

    sal_Int32 i = 0;
    for( ContextMap::iterator ii = m_mapContext.begin() ;
         ii != m_mapContext.end();
         ++ii, i++ )
    {
        rtl_uString_assign( &( ppReturn[i] ), (*ii).first.pData  );
    }

    return ppReturn;
}



/*****************************
 * remote_ContextImpl implementation
 ****************************/



remote_ContextImpl::remote_ContextImpl( remote_Connection *pConnection ,
                                        rtl_uString *pIdStr,
                                        rtl_uString *pDescription,
                                        rtl_uString *pProtocol,
                                        remote_InstanceProvider *pProvider ) :
    m_nRef( 0 ),
    m_bDisposed( sal_False )
{
    m_pConnection = pConnection;
    m_pConnection->acquire( m_pConnection );

    m_pInstanceProvider = pProvider;
    if( m_pInstanceProvider )
    {
        m_pInstanceProvider->acquire( pProvider );
    }

    m_pName = pIdStr;
    rtl_uString_acquire( m_pName );

    m_pDescription = pDescription;
    rtl_uString_acquire( m_pDescription );

    m_pProtocol = pProtocol;
    rtl_uString_acquire( pProtocol );

    aBase.acquire = thisAcquire;
    aBase.release = thisRelease;
    addDisposingListener = thisAddDisposingListener;
    removeDisposingListener = thisRemoveDisposingListener;
    dispose = thisDispose;
#if OSL_DEBUG_LEVEL > 1
    thisCounter.acquire();
#endif
}

remote_ContextImpl::~remote_ContextImpl()
{
    // disposed must have been called
    OSL_ASSERT( m_bDisposed );

    rtl_uString_release( m_pName );
    rtl_uString_release( m_pDescription );
    rtl_uString_release( m_pProtocol );
#if OSL_DEBUG_LEVEL > 1
    thisCounter.release();
#endif

}


void remote_ContextImpl::thisAddDisposingListener( remote_Context *pRemoteC ,
                                                   remote_DisposingListener *pListener )
{
    remote_ContextImpl *pImpl = (remote_ContextImpl * ) pRemoteC;

    ::osl::MutexGuard guard( pImpl->m_mutex );

    pListener->acquire( pListener );
    pImpl->m_lstListener.push_back( pListener );
}

void remote_ContextImpl::thisRemoveDisposingListener( remote_Context *pRemoteC,
                                                      remote_DisposingListener *pListener)
{
    remote_ContextImpl *pImpl = (remote_ContextImpl * ) pRemoteC;
    MutexGuard guard( pImpl->m_mutex );

    for( list< remote_DisposingListener * >::iterator ii = pImpl->m_lstListener.begin() ;
         ii != pImpl->m_lstListener.end();
         ++ii )
    {
        if( (*ii) == pListener  )
        {
            pImpl->m_lstListener.erase( ii );
            pListener->release( pListener );
            break;
        }
    }
}

void remote_ContextImpl::thisDispose( remote_Context *pRemoteC )
{
    remote_ContextImpl *pImpl = ( remote_ContextImpl * )pRemoteC;

    MutexGuard guard( pImpl->m_mutex );
    if( ! pImpl->m_bDisposed )
    {
        pImpl->m_bDisposed = sal_True;
        ContextAdmin::getInstance()->revokeContext( (uno_Context * )  pRemoteC );

        if( pImpl->m_pInstanceProvider )
        {
            pImpl->m_pInstanceProvider->release( pImpl->m_pInstanceProvider );
            pImpl->m_pInstanceProvider = 0;
        }

        pImpl->m_pConnection->release( pImpl->m_pConnection );
        pImpl->m_pConnection = 0;

        list< remote_DisposingListener * > lst = pImpl->m_lstListener;
        pImpl->m_lstListener.clear();

        for( list < remote_DisposingListener * >::iterator ii = lst.begin();
             ii != lst.end();
             ++ii )
        {
            (*ii)->disposing( (*ii) , pImpl->m_pName );
            (*ii)->release( (*ii) );
        }

    }
}



void remote_ContextImpl::thisAcquire( uno_Context *pRemoteC )
{
    remote_ContextImpl *p = SAL_REINTERPRET_CAST(remote_ContextImpl * ,pRemoteC );
    osl_incrementInterlockedCount( &(p->m_nRef) );
}

void remote_ContextImpl::thisRelease( uno_Context *pRemoteC )
{
    remote_ContextImpl *p = SAL_REINTERPRET_CAST( remote_ContextImpl * , pRemoteC );
    if (! osl_decrementInterlockedCount( &(p->m_nRef) ))
    {
        // enshure, that this piece of code is not reentered
        osl_incrementInterlockedCount( &(p->m_nRef) );

        // dispose, if necessary
        p->dispose( p );

        // restore the counter
        osl_decrementInterlockedCount( &(p->m_nRef) );

        if( 0 == p->m_nRef )
        {
            delete p;
        }
        else
        {
            // reanimated, but disposed !
        }
    }
}

void *remote_ContextImpl::thisQuery( uno_Context * , rtl_uString * )
{
    return 0;
}


} // end namespace remote_context


using namespace remote_context;

//-----------------------
//
// C-Interface
//
//-----------------------
extern "C" remote_Context * SAL_CALL
remote_getContext( rtl_uString *pIdString )
{
    return (remote_Context *) ContextAdmin::getInstance()->get(  pIdString );
}



extern "C" remote_Context * SAL_CALL
remote_createContext( remote_Connection *pConnection,
                      rtl_uString *pIdStr,
                      rtl_uString *pDescription,
                      rtl_uString *pProtocol,
                      remote_InstanceProvider *pProvider )
{
    remote_ContextImpl *p = (remote_ContextImpl * )
        ContextAdmin::getInstance()->createAndRegisterContext(
            pConnection ,
            pIdStr ,
            pDescription,
            pProtocol,
            pProvider );

    return (remote_Context * )p;
}


extern "C" void SAL_CALL
remote_addContextListener( remote_contextListenerFunc listener,  void *pObject )
{
    ContextAdmin::getInstance()->addContextListener( listener , pObject );
}

extern "C" void SAL_CALL
remote_removeContextListener( remote_contextListenerFunc listener , void *pObject )
{
    ContextAdmin::getInstance()->removeContextListener( listener , pObject );
}

extern "C" rtl_uString ** SAL_CALL
remote_getContextList( sal_Int32 *pnStringCount, MemAlloc memAlloc )
{
    return ContextAdmin::getInstance()->getConnectionList( pnStringCount , memAlloc );
}
