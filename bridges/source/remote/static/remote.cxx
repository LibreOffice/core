/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: remote.cxx,v $
 * $Revision: 1.10 $
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
#if OSL_DEBUG_LEVEL == 0
#  ifndef NDEBUG
#    define NDEBUG
#  endif
#endif
#include <assert.h>
#include <bridges/remote/remote.hxx>
#include <bridges/remote/counter.hxx>

#if OSL_DEBUG_LEVEL > 1
static MyCounter thisCounter( "DEBUG : Remote2RemoteStub");
#endif

using namespace bridges_remote;

extern "C" {

static void SAL_CALL thisRelease( remote_Interface *pThis )
{
    Remote2RemoteStub *p = ( Remote2RemoteStub * ) pThis;
     if (! osl_decrementInterlockedCount( &(p->m_nRef) ))
    {
        p->m_pEnvRemote->pExtEnv->revokeInterface( p->m_pEnvRemote->pExtEnv, pThis );

    }
}

static void SAL_CALL thisDispatch(
    remote_Interface * pRemoteI,
    typelib_TypeDescription const * pMemberType,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException )
{
    Remote2RemoteStub *pThis = ( Remote2RemoteStub * ) pRemoteI;

    pThis->m_dispatch( pThis->m_pEnvRemote,
                       pMemberType,
                       pThis->m_sOid.pData,
                       pThis->m_pType,
                       pReturn,
                       pArgs,
                       ppException );
}

}

namespace bridges_remote {

void acquireRemote2RemoteStub( remote_Interface *pThis )
{
    Remote2RemoteStub *p = ( Remote2RemoteStub * ) pThis;
    if( 1 == osl_incrementInterlockedCount( &(p->m_nRef) ) )
    {
        p->m_pEnvRemote->pExtEnv->registerProxyInterface(
            p->m_pEnvRemote->pExtEnv,
            (void**)&pThis,
            freeRemote2RemoteStub,
            p->m_sOid.pData,
            p->m_pType );
        assert( (remote_Interface *)p == pThis );
    }
}

void freeRemote2RemoteStub(uno_ExtEnvironment *, void * stub) {
    delete static_cast< Remote2RemoteStub * >(stub);
}

Remote2RemoteStub::Remote2RemoteStub( rtl_uString *pOid,
                                      typelib_InterfaceTypeDescription *pType,
                                      uno_Environment *pEnvRemote,
                                      requestClientSideDispatcher dispatch ) :
    m_sOid( pOid ),
    m_pType( (typelib_InterfaceTypeDescription * ) pType ),
    m_nRef( 1 ),
    m_pEnvRemote( pEnvRemote ),
    m_dispatch( dispatch ),
    m_nReleaseRemote( 1 )
{
    typelib_typedescription_acquire( ( typelib_TypeDescription * ) m_pType );
    m_pEnvRemote->acquire( m_pEnvRemote );

    acquire = acquireRemote2RemoteStub;
    release = thisRelease;
    pDispatcher = thisDispatch;
#if OSL_DEBUG_LEVEL > 1
    thisCounter.acquire();
#endif
}

Remote2RemoteStub::~Remote2RemoteStub()
{

    // send a release via the connection !
    sal_Bool bNeedsRelease = sal_False;
    if( ! m_pType->aBase.bComplete )
    {
        // m_pType may be exchanged during complete, so it needs to be acquired
        // (MT : Another thread may use m_pType during e.g. dispatch ! ).
        typelib_typedescription_acquire( (typelib_TypeDescription*)m_pType );
        bNeedsRelease = sal_True;
        typelib_typedescription_complete( (typelib_TypeDescription **) &m_pType );
    }

    uno_Any any;
    uno_Any *pAny = &any;

    typelib_TypeDescription *pReleaseMethod = 0;
    typelib_typedescriptionreference_getDescription(
        &pReleaseMethod ,
        m_pType->ppAllMembers[REMOTE_RELEASE_METHOD_INDEX] );
    for( int i = 0 ; i < m_nReleaseRemote ; i ++ )
    {
        thisDispatch( this,
                      pReleaseMethod,
                      0,
                      0,
                      &pAny );
    }
    typelib_typedescription_release( pReleaseMethod );
    if( bNeedsRelease )
    {
        typelib_typedescription_release( (typelib_TypeDescription * ) m_pType );
    }

    typelib_typedescription_release( (typelib_TypeDescription * ) m_pType );
    m_pEnvRemote->release( m_pEnvRemote );
#if OSL_DEBUG_LEVEL > 1
    thisCounter.release();
#endif
}

void Remote2RemoteStub::releaseRemote()
{
    osl_incrementInterlockedCount( &m_nReleaseRemote );
}

} // end namespace bridges_remote
