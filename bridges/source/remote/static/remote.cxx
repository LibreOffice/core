/*************************************************************************
 *
 *  $RCSfile: remote.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-19 14:22:25 $
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
#include <assert.h>

#include <bridges/remote/remote.hxx>
#include <bridges/remote/counter.hxx>

#ifdef DEBUG
static MyCounter thisCounter( "DEBUG : Remote2RemoteStub");
#endif

namespace bridges_remote {

Remote2RemoteStub::Remote2RemoteStub( rtl_uString *pOid,
                                      typelib_InterfaceTypeDescription *pType,
                                      uno_Environment *pEnvRemote,
                                      requestClientSideDispatcher dispatch ) :
    m_pType( (typelib_InterfaceTypeDescription * ) pType ),
    m_pEnvRemote( pEnvRemote ),
    m_sOid( pOid ),
    m_nRef( 1 ),
    m_dispatch( dispatch )
{
    typelib_typedescription_acquire( ( typelib_TypeDescription * ) m_pType );
    m_pEnvRemote->acquire( m_pEnvRemote );

    acquire = thisAcquire;
    release = thisRelease;
    pDispatcher = thisDispatch;
#ifdef DEBUG
    thisCounter.acquire();
#endif
}

Remote2RemoteStub::~Remote2RemoteStub()
{

    // send a release via the connection !
    releaseRemote();

    typelib_typedescription_release( (typelib_TypeDescription * ) m_pType );
    m_pEnvRemote->release( m_pEnvRemote );
#ifdef DEBUG
    thisCounter.release();
#endif
}


void Remote2RemoteStub::thisFree( uno_ExtEnvironment *pEnvUno , void *pThis )
{
    delete (Remote2RemoteStub *) pThis;
}
void Remote2RemoteStub::releaseRemote()
{
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
    thisDispatch( this,
                  pReleaseMethod,
                  0,
                  0,
                  &pAny );

    typelib_typedescription_release( pReleaseMethod );
    if( bNeedsRelease )
    {
        typelib_typedescription_release( (typelib_TypeDescription * ) m_pType );
    }
}

void Remote2RemoteStub::thisAcquire( remote_Interface *pThis )
{
    Remote2RemoteStub *p = ( Remote2RemoteStub * ) pThis;
    if( 1 == osl_incrementInterlockedCount( &(p->m_nRef) ) )
    {
        p->m_pEnvRemote->pExtEnv->registerProxyInterface(
            p->m_pEnvRemote->pExtEnv,
            (void**)&pThis,
            Remote2RemoteStub::thisFree,
            p->m_sOid.pData,
            p->m_pType );
        assert( (remote_Interface *)p == pThis );
    }
}

void Remote2RemoteStub::thisRelease( remote_Interface *pThis )
{
    Remote2RemoteStub *p = ( Remote2RemoteStub * ) pThis;
     if (! osl_decrementInterlockedCount( &(p->m_nRef) ))
    {
        p->m_pEnvRemote->pExtEnv->revokeInterface( p->m_pEnvRemote->pExtEnv, pThis );

    }
}

void Remote2RemoteStub::thisDispatch(
    remote_Interface * pRemoteI,
    typelib_TypeDescription * pMemberType,
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

} // end namespace bridges_remote
