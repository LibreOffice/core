/*************************************************************************
 *
 *  $RCSfile: mapping.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:50 $
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
#include <osl/diagnose.h>

#include <bridges/remote/proxy.hxx>
#include <bridges/remote/stub.hxx>
#include <bridges/remote/counter.hxx>
#include <bridges/remote/mapping.hxx>

namespace bridges_remote {

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

void RemoteMapping::thisFree( uno_Mapping * p )
{
    delete ( RemoteMapping * ) p;
}

RemoteMapping::~RemoteMapping( )
{
    pEnvUno->release( pEnvUno );
    pEnvRemote->release( pEnvRemote );
}

void RemoteMapping::thisAcquire( uno_Mapping *pMap )
{
    RemoteMapping *p = SAL_REINTERPRET_CAST( RemoteMapping * , pMap );
    if( 1 == osl_incrementInterlockedCount( &(p->m_nRef) ) )
    {
        if( RemoteMapping::remoteToUno  == pMap->mapInterface )
        {
            uno_registerMapping( &pMap ,
                                 RemoteMapping::thisFree,
                                 p->pEnvRemote ,
                                 p->pEnvUno ,
                                 p->m_sPurpose.pData );
        }
        else
        {
            uno_registerMapping( &pMap ,
                                 RemoteMapping::thisFree,
                                 p->pEnvUno ,
                                 p->pEnvRemote ,
                                 p->m_sPurpose.pData );
        }

    }
}

void RemoteMapping::thisRelease( uno_Mapping *pMap )
{
    RemoteMapping *p = SAL_REINTERPRET_CAST( RemoteMapping * , pMap );
    if (! osl_decrementInterlockedCount( &(p->m_nRef) ))
    {
        uno_revokeMapping( pMap );
    }
}


void RemoteMapping::remoteToUno(    uno_Mapping *pMapping,
                                    void **ppUnoI,
                                    void *pRemoteI,
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
                Remote2UnoProxy::thisFree,
                pOid,
                pTypeDescr );

            OSL_ASSERT( *ppUnoI );
        }
        rtl_uString_release( pOid );
    }
}


void RemoteMapping::unoToRemote(    uno_Mapping *pMapping,
                                    void **ppRemoteI,
                                    void *pUnoI,
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
                Uno2RemoteStub::thisFree,
                pOid,
                pTypeDescr );
        }

        rtl_uString_release( pOid );
    }
}

}
