/*************************************************************************
 *
 *  $RCSfile: helper.cxx,v $
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
#include <rtl/alloc.h>
#include <osl/diagnose.h>

#include <bridges/remote/helper.hxx>

#include <bridges/remote/stub.hxx>
#include <bridges/remote/proxy.hxx>
#include <bridges/remote/remote.hxx>

#include <com/sun/star/uno/Sequence.hxx>

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace bridges_remote
{

void SAL_CALL remote_createStub (
    remote_Interface **ppRemoteI,
    rtl_uString *pOid ,
    typelib_TypeDescriptionReference *pTypeRef,
    uno_Environment *pEnvRemote )
{
    typelib_TypeDescription *pType = 0;
    typelib_typedescriptionreference_getDescription( &pType, pTypeRef );

    pEnvRemote->pExtEnv->getRegisteredInterface(
        pEnvRemote->pExtEnv,
        (void **)ppRemoteI,
        pOid,
        (typelib_InterfaceTypeDescription* )pType );

    if( *ppRemoteI )
    {
        if( (*ppRemoteI)->acquire == ::bridges_remote::Remote2RemoteStub::thisAcquire ) {

            // send a release to remote
            ((::bridges_remote::Remote2RemoteStub *)*ppRemoteI)->releaseRemote();
        }
        else
        {
            // Uno2RemoteStub
            // no release necessary
        }
    }
    else
    {
        remote_BridgeImpl *pImpl = ((remote_Context *)pEnvRemote->pContext)->m_pBridgeImpl;
        *ppRemoteI =
            new ::bridges_remote::Remote2RemoteStub(
                pOid,
                (typelib_InterfaceTypeDescription * ) pType,
                pEnvRemote,
                pImpl->m_sendRequest);

        // ppRemoteI may change during registration
        pEnvRemote->pExtEnv->registerProxyInterface(
            pEnvRemote->pExtEnv,
            (void **) ppRemoteI,
            ::bridges_remote::Remote2RemoteStub::thisFree,
            pOid,
            (typelib_InterfaceTypeDescription * ) pType );
    }

    typelib_typedescription_release( pType );
}

void SAL_CALL remote_sendQueryInterface(
    uno_Environment *pEnvRemote,
    remote_Interface **ppRemoteI,
    rtl_uString *pOid ,
    typelib_TypeDescriptionReference *pTypeRef
    )
{
    OSL_ASSERT( ppRemoteI );

    typelib_InterfaceTypeDescription *pType = 0;
    typelib_typedescriptionreference_getDescription( (typelib_TypeDescription ** )&pType, pTypeRef );

    if( *ppRemoteI )
    {
        (*ppRemoteI)->release( *ppRemoteI );
        (*ppRemoteI) = 0;
    }

    remote_BridgeImpl *pImpl = ((remote_Context *)pEnvRemote->pContext)->m_pBridgeImpl;

    Type type  = ::getCppuType( (Reference < XInterface > *)0 );

    // get type for queryInterface
    OUString sCompleteMethodName = type.getTypeName();
    sCompleteMethodName += OUString::createFromAscii("::queryInterface");

    typelib_InterfaceMemberTypeDescription *pMemberType = 0;
    typelib_typedescription_getByName(
        (typelib_TypeDescription **) &pMemberType,
        sCompleteMethodName.pData );

    OSL_ASSERT( pMemberType );

    uno_Any anyInterface;
    anyInterface.pType = 0;
    anyInterface.pData = 0;

    void *pReturn = &anyInterface;
    void *ppArgs[1];

    ppArgs[0] = 0;
    typelib_TypeDescriptionReference *pRef = 0;
    typelib_typedescriptionreference_new( &pRef ,
                                          pType->aBase.eTypeClass,
                                          pType->aBase.pTypeName);

    ppArgs[0] = &pRef;

    uno_Any anyException;
    uno_Any *pAnyException = &anyException;

    // do the queryInterface
    pImpl->m_sendRequest(
        pEnvRemote,
        (typelib_TypeDescription * ) pMemberType,
        pOid,
        pType,
        pReturn,
        ppArgs,
        &pAnyException );


    // now release everything
    typelib_typedescriptionreference_release( pRef );
    typelib_typedescription_release( (typelib_TypeDescription * ) pMemberType );

    if( pAnyException )
    {
        uno_any_destruct( pAnyException , 0 );
    }
    else
    {
        // set out parameter
        if( typelib_TypeClass_INTERFACE == anyInterface.pType->eTypeClass )
        {
            *ppRemoteI = *( remote_Interface ** )  anyInterface.pData;
            rtl_freeMemory( anyInterface.pData );
        }
        typelib_typedescriptionreference_release( anyInterface.pType );
    }

    typelib_typedescription_release( (typelib_TypeDescription * ) pType );
}


void SAL_CALL remote_retrieveOidFromProxy(
    remote_Interface *pRemoteI,
    rtl_uString **ppOid )
{
    if( pRemoteI->acquire == ::bridges_remote::Remote2RemoteStub::thisAcquire )
    {
        // Remote2RemoteStub
        ::bridges_remote::Remote2RemoteStub *pStub = (::bridges_remote::Remote2RemoteStub * ) pRemoteI;
        rtl_uString_newFromString( ppOid , pStub->m_sOid.pData );
    }
    else
    {
        // Uno2RemoteStub
        ::bridges_remote::Uno2RemoteStub *pStub = (::bridges_remote::Uno2RemoteStub * ) pRemoteI;
        rtl_uString_newFromString( ppOid , pStub->m_sOid.pData );
        pRemoteI->acquire( pRemoteI );
    }
}

}
