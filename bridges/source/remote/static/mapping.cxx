/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mapping.cxx,v $
 * $Revision: 1.5 $
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
#include <osl/diagnose.h>

#include <bridges/remote/proxy.hxx>
#include <bridges/remote/stub.hxx>
#include <bridges/remote/counter.hxx>
#include <bridges/remote/mapping.hxx>

using namespace bridges_remote;

extern "C" {

static void SAL_CALL thisAcquire( uno_Mapping *pMap )
{
    RemoteMapping *p = SAL_REINTERPRET_CAST( RemoteMapping * , pMap );
    if( 1 == osl_incrementInterlockedCount( &(p->m_nRef) ) )
    {
        if( remoteToUno == pMap->mapInterface )
        {
            uno_registerMapping( &pMap ,
                                 freeRemoteMapping,
                                 p->pEnvRemote ,
                                 p->pEnvUno ,
                                 p->m_sPurpose.pData );
        }
        else
        {
            uno_registerMapping( &pMap ,
                                 freeRemoteMapping,
                                 p->pEnvUno ,
                                 p->pEnvRemote ,
                                 p->m_sPurpose.pData );
        }

    }
}

static void SAL_CALL thisRelease( uno_Mapping *pMap )
{
    RemoteMapping *p = SAL_REINTERPRET_CAST( RemoteMapping * , pMap );
    if (! osl_decrementInterlockedCount( &(p->m_nRef) ))
    {
        uno_revokeMapping( pMap );
    }
}

}

namespace bridges_remote {

void remoteToUno( uno_Mapping *pMapping, void **ppUnoI, void *pRemoteI,
                  typelib_InterfaceTypeDescription *pTypeDescr )
{
    remote_Mapping *pRemoteMapping = ( remote_Mapping * ) pMapping;

    OSL_ASSERT( ppUnoI && pTypeDescr );
    if (*ppUnoI)
    {
        ((uno_Interface *)*ppUnoI)->release( (uno_Interface *)*ppUnoI );
        *ppUnoI = 0;
    }

    if (pRemoteI && pTypeDescr)
    {
        // get object id of interface to be wrapped
        rtl_uString * pOid = 0;
        pRemoteMapping->pEnvRemote->pExtEnv->getObjectIdentifier(
            pRemoteMapping->pEnvRemote->pExtEnv,
            &pOid,
            pRemoteI );

        OSL_ASSERT(pOid);
        if( ! pOid )
        {
            return;
        }

        // try to get any known interface from target environment
        pRemoteMapping->pEnvUno->pExtEnv->getRegisteredInterface(
            pRemoteMapping->pEnvUno->pExtEnv,
            ppUnoI,
            pOid,
            pTypeDescr);

        if ( ! *ppUnoI) // already existing interface
        {
            // try to publish a new proxy; proxy may be exchanged during registration
            *ppUnoI = new Remote2UnoProxy(
                ( remote_Interface * ) pRemoteI,
                pOid,
                pTypeDescr ,
                pRemoteMapping->pEnvUno,
                pRemoteMapping->pEnvRemote);

            pRemoteMapping->pEnvUno->pExtEnv->registerProxyInterface(
                pRemoteMapping->pEnvUno->pExtEnv,
                ppUnoI,
                freeRemote2UnoProxy,
                pOid,
                pTypeDescr );

            OSL_ASSERT( *ppUnoI );
        }
        rtl_uString_release( pOid );
    }
}

void unoToRemote( uno_Mapping *pMapping, void **ppRemoteI, void *pUnoI,
                  typelib_InterfaceTypeDescription *pTypeDescr )
{
    remote_Mapping *pRemoteMapping = ( remote_Mapping * ) pMapping;
    OSL_ASSERT( ppRemoteI && pTypeDescr );
    if (*ppRemoteI)
    {
        ((remote_Interface *)*ppRemoteI)->release( (remote_Interface *)*ppRemoteI);
        *ppRemoteI = 0;
    }
    if (pUnoI && pTypeDescr)
    {
        // get object id of interface to be wrapped
        rtl_uString * pOid = 0;
        pRemoteMapping->pEnvUno->pExtEnv->getObjectIdentifier(
            pRemoteMapping->pEnvUno->pExtEnv,
            &pOid,
            pUnoI );

        OSL_ASSERT( pOid );
        if( ! pOid )
        {
            return;
        }

        pRemoteMapping->pEnvRemote->pExtEnv->getRegisteredInterface(
            pRemoteMapping->pEnvRemote->pExtEnv,
            (void**)ppRemoteI,
            pOid,
            pTypeDescr );

        if( !*ppRemoteI )
        {
            // try to publish a new proxy;
            *ppRemoteI = new Uno2RemoteStub(
                ( uno_Interface * ) pUnoI,
                pOid,
                pTypeDescr,
                pRemoteMapping->pEnvUno,
                pRemoteMapping->pEnvRemote );

            // note : ppRemoteI may change during registration
            pRemoteMapping->pEnvRemote->pExtEnv->registerProxyInterface(
                pRemoteMapping->pEnvRemote->pExtEnv,
                (void**) ppRemoteI,
                freeUno2RemoteStub,
                pOid,
                pTypeDescr );
        }

        rtl_uString_release( pOid );
    }
}

void freeRemoteMapping(uno_Mapping * mapping) {
    delete reinterpret_cast< RemoteMapping * >(mapping);
}

RemoteMapping::RemoteMapping( uno_Environment *pEnvUno_ ,
                              uno_Environment *pEnvRemote_,
                              uno_MapInterfaceFunc func,
                              const ::rtl::OUString sPurpose) :
    m_nRef( 1 ),
    m_sPurpose( sPurpose )
{
    pEnvUno = pEnvUno_;
    pEnvRemote = pEnvRemote_;

    pEnvUno->acquire( pEnvUno );
    pEnvRemote->acquire( pEnvRemote );

    aBase.mapInterface = func;
    aBase.acquire = thisAcquire;
    aBase.release = thisRelease;
}

RemoteMapping::~RemoteMapping( )
{
    pEnvUno->release( pEnvUno );
    pEnvRemote->release( pEnvRemote );
}

}
