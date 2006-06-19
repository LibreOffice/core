/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: remote.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:51:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
