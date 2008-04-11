/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bridge_provider.cxx,v $
 * $Revision: 1.7 $
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

#include <stdio.h>

#include "remote_bridge.hxx"

#include <osl/diagnose.h>

#include <uno/mapping.hxx>
#include <uno/environment.h>

#include <bridges/remote/remote.h>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::bridge;

namespace remotebridges_bridge
{

    OInstanceProviderWrapper::OInstanceProviderWrapper(
        const Reference <XInstanceProvider > & rProvider,
        ORemoteBridge * pBridgeCallback ) :
        m_rProvider( rProvider ),
        m_nRef( 0 ),
        m_pBridgeCallback( pBridgeCallback )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        acquire = thisAcquire;
        release = thisRelease;
        getInstance = thisGetInstance;
    }

    OInstanceProviderWrapper::~OInstanceProviderWrapper()
    {
        g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
    }

    void OInstanceProviderWrapper::thisAcquire( remote_InstanceProvider *p )
    {
        OInstanceProviderWrapper * m = (OInstanceProviderWrapper *) p;
        osl_incrementInterlockedCount( &(m->m_nRef ) );
    }

    void OInstanceProviderWrapper::thisRelease( remote_InstanceProvider *p )
    {
        OInstanceProviderWrapper * m = ( OInstanceProviderWrapper *) p;
        if( ! osl_decrementInterlockedCount( &(m->m_nRef ) ) )
        {
            delete m;
        }
    }

    static void convertToRemoteRuntimeException ( uno_Any **ppException,
                                             const ::rtl::OUString &sMessage,
                                             const Reference< XInterface > &rContext,
                                             Mapping &map )
    {

        uno_type_any_construct( *ppException ,
                                0 ,
                                getCppuType( (RuntimeException  *)0 ).getTypeLibType() ,
                                0 );

        typelib_CompoundTypeDescription * pCompType = 0 ;
        getCppuType( (Exception*)0 ).getDescription( (typelib_TypeDescription **) &pCompType );
        if( ! ((typelib_TypeDescription *)pCompType)->bComplete )
        {
            typelib_typedescription_complete( (typelib_TypeDescription**) &pCompType );
        }

        char *pValue = (char*) (*ppException)->pData;
        rtl_uString_assign( (rtl_uString ** ) pValue  , sMessage.pData );

        *((remote_Interface**) pValue+pCompType->pMemberOffsets[1]) =
            (remote_Interface*) map.mapInterface(
                rContext.get(), getCppuType( &rContext) );

        typelib_typedescription_release( (typelib_TypeDescription *) pCompType );
    }

    void OInstanceProviderWrapper::thisGetInstance(
        remote_InstanceProvider * pProvider ,
        uno_Environment *pEnvRemote,
        remote_Interface **ppRemoteI,
        rtl_uString *pInstanceName,
        typelib_InterfaceTypeDescription *pType,
        uno_Any **ppException
        )
    {
        OInstanceProviderWrapper * m = (OInstanceProviderWrapper *)pProvider;

        OSL_ASSERT( ppRemoteI );
        if( *ppRemoteI )
        {
            (*ppRemoteI)->release( *ppRemoteI );
            *ppRemoteI = 0;
        }

        if( OUString( pType->aBase.pTypeName ) ==
            getCppuType( (Reference<XInterface>*)0).getTypeName() )
        {
            OUString sCppuName( RTL_CONSTASCII_USTRINGPARAM( CPPU_CURRENT_LANGUAGE_BINDING_NAME ) );

            uno_Environment *pEnvThis = 0;
            uno_getEnvironment( &pEnvThis ,
                                sCppuName.pData ,
                                0 );
            Mapping map( pEnvThis , pEnvRemote );
            pEnvThis->release( pEnvThis );

            try
            {
                Reference< XInterface > r = m->m_rProvider->getInstance(
                    OUString( pInstanceName ) );

                *ppRemoteI = (remote_Interface*) map.mapInterface (
                    r.get(),
                    getCppuType( (Reference< XInterface > *) 0 )
                );

                if( *ppRemoteI && m->m_pBridgeCallback )
                {
                    m->m_pBridgeCallback->objectMappedSuccesfully();
                    m->m_pBridgeCallback = 0;
                }
                *ppException = 0;
            }
            catch( ::com::sun::star::container::NoSuchElementException &e )
            {
                // NoSuchElementException not specified, so convert it to a runtime exception
                convertToRemoteRuntimeException(
                    ppException , e.Message.pData , e.Context.get(), map );
            }
            catch( ::com::sun::star::uno::RuntimeException &e )
            {
                convertToRemoteRuntimeException(
                    ppException , e.Message.pData , e.Context.get(), map );
            }

        }
    }
}
