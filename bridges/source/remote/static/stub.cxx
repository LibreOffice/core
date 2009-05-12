/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: stub.cxx,v $
 * $Revision: 1.11 $
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
#include <sal/alloca.h>
#include <osl/diagnose.h>

#include <uno/data.h>
#include <uno/mapping.hxx>

#include <bridges/remote/stub.hxx>
#include <bridges/remote/proxy.hxx>
#include <bridges/remote/context.h>
#include <bridges/remote/bridgeimpl.hxx>

#include "remote_types.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <bridges/remote/counter.hxx>
static MyCounter thisCounter( "DEBUG : Uno2RemoteStub");
#endif

using namespace ::com::sun::star::uno;
using namespace bridges_remote;

extern "C" {

void SAL_CALL thisRelease( remote_Interface *pThis )
{
    Uno2RemoteStub *p = ( Uno2RemoteStub * ) pThis;
    if (! osl_decrementInterlockedCount( &(p->m_nRef) ))
    {
        p->m_pEnvRemote->pExtEnv->revokeInterface( p->m_pEnvRemote->pExtEnv, pThis );
    }
}

void SAL_CALL thisDispatch(
    remote_Interface * pRemoteI,
    typelib_TypeDescription const * pType,
    void * pReturn,
    void * ppArgs[],
    uno_Any ** ppException )
{
    Uno2RemoteStub *p = ( Uno2RemoteStub * ) pRemoteI;

    RemoteThreadCounter counter( p->m_pEnvRemote );

    typelib_InterfaceMethodTypeDescription *pMethodType = 0;
    typelib_InterfaceAttributeTypeDescription *pAttributeType = 0;
    typelib_TypeDescription *pReturnType = 0;
    typelib_TypeDescription **ppArgType = 0;
    sal_Int32 nArgCount = 0;
    sal_Bool *pbIsIn = 0;
    sal_Bool *pbIsOut = 0;
    sal_Bool bConversionNeededForReturn = sal_False;
    sal_Bool *pbConversionNeeded = 0;

    sal_Int32 i;
    //--------------------------------
    // Collect all needed types !
    //--------------------------------
    if( typelib_TypeClass_INTERFACE_ATTRIBUTE == pType->eTypeClass )
    {
        pAttributeType = ( typelib_InterfaceAttributeTypeDescription * ) pType;
        if( pReturn )
        {
            TYPELIB_DANGER_GET( &pReturnType , pAttributeType->pAttributeTypeRef );
            bConversionNeededForReturn = remote_relatesToInterface( pReturnType );
        }
        else
        {
            nArgCount = 1;
            ppArgType = (typelib_TypeDescription **) alloca( sizeof( void * ) );
            pbIsIn  = ( sal_Bool * ) alloca( sizeof( sal_Bool ) );
            pbIsOut = ( sal_Bool * ) alloca( sizeof( sal_Bool ) );
            pbConversionNeeded = ( sal_Bool * ) alloca( sizeof( sal_Bool ) );
            pbIsIn[0]  = sal_True;
            pbIsOut[0] = sal_False;
            ppArgType[0] = 0;
            TYPELIB_DANGER_GET( &( ppArgType[0] ) , pAttributeType->pAttributeTypeRef );
            pbConversionNeeded[0] = remote_relatesToInterface( ppArgType[0] );
        }
    }
    if( typelib_TypeClass_INTERFACE_METHOD == pType->eTypeClass )
    {
        pMethodType = ( typelib_InterfaceMethodTypeDescription * ) pType;
        TYPELIB_DANGER_GET( &pReturnType , pMethodType->pReturnTypeRef );
        bConversionNeededForReturn = remote_relatesToInterface( pReturnType );
        nArgCount = pMethodType->nParams;
        ppArgType = (typelib_TypeDescription **) alloca( sizeof( void * ) * nArgCount );
        pbIsIn  = (sal_Bool * ) alloca( sizeof( sal_Bool ) * nArgCount );
        pbIsOut = (sal_Bool * ) alloca( sizeof( sal_Bool ) * nArgCount );
        pbConversionNeeded = ( sal_Bool * ) alloca( sizeof( sal_Bool ) * nArgCount );

        for( i = 0 ; i < nArgCount ; i ++ )
        {
            ppArgType[i] = 0;
            TYPELIB_DANGER_GET( & (ppArgType[i]) ,  pMethodType->pParams[i].pTypeRef );
            pbIsIn[i]  = pMethodType->pParams[i].bIn;
            pbIsOut[i] = pMethodType->pParams[i].bOut;
            pbConversionNeeded[i] = remote_relatesToInterface( ppArgType[i] );
        }
    }

    // create Mapping

    void *pUnoReturn = 0;
    void **ppUnoArgs = 0;

    if( pReturnType )
    {
        if( bConversionNeededForReturn )
        {
            pUnoReturn = alloca( pReturnType->nSize );
        }
        else
        {
            pUnoReturn = pReturn;
        }
    }

    ppUnoArgs = (void **) alloca( nArgCount * sizeof( void * ) );
    for( i = 0 ; i < nArgCount ; i ++ )
    {
        if( pbConversionNeeded[i] )
        {
            ppUnoArgs[i] = alloca( ppArgType[i]->nSize );
            if( pbIsIn[i] )
            {
                uno_copyAndConvertData(
                    ppUnoArgs[i],
                    ppArgs[i],
                    ppArgType[i],
                    p->m_mapRemote2Uno.get()
                    );
            }
        }
        else
        {
            ppUnoArgs[i] = ppArgs[i];
        }
    }

    // do the call
    uno_Any any;
    uno_Any *pAny = &any;

    p->m_pUnoI->pDispatcher( p->m_pUnoI,
                             pType,
                             pUnoReturn,
                             ppUnoArgs,
                             &pAny);

    if( ! pAny )
    {
        // ------------------
        // No Exception
        // ------------------

        // Map return value
        if( pReturnType && bConversionNeededForReturn )
        {
            uno_copyAndConvertData(
                pReturn ,
                pUnoReturn,
                pReturnType,
                p->m_mapUno2Remote.get() );
            uno_destructData( pUnoReturn , pReturnType, 0 );
        }

        // map arguments
        for( i = 0 ; i < nArgCount ; i ++ )
        {
            if( pbConversionNeeded[i] )
            {
                if( pbIsIn[i] ) {
                    if( pbIsOut[i] ) {
                        uno_destructData(
                            ppArgs[i] ,
                            ppArgType[i] ,
                            remote_release );
                        uno_copyAndConvertData( ppArgs[i] ,
                                                ppUnoArgs[i],
                                                ppArgType[i],
                                                p->m_mapUno2Remote.get()  );
                    }
                }
                else // pure out
                {
                    uno_copyAndConvertData( ppArgs[i] ,
                                            ppUnoArgs[i],
                                            ppArgType[i],
                                            p->m_mapUno2Remote.get() );
                }
                uno_destructData( ppUnoArgs[i],
                                  ppArgType[i],
                                  0 );
            }
        }
        *ppException = 0;
    }
    else
    {
        // -----------------------
        // an exception occured
        // -----------------------
        typelib_TypeDescription *pAnyType = 0;
        getCppuType( (Any*) 0 ).getDescription( &pAnyType );
        uno_copyAndConvertData( *ppException ,
                                pAny ,
                                pAnyType,
                                p->m_mapUno2Remote.get() );
        uno_destructData( pAny , pAnyType , 0 );
        typelib_typedescription_release( pAnyType );

        // destruct uno in parameters ( out parameters have not been constructed )
        for( i = 0 ; i < nArgCount ; i ++ )
        {
            if( pbConversionNeeded[i] && pbIsIn[i] )
            {
                uno_destructData( ppUnoArgs[i],
                                  ppArgType[i],
                                  0 );
            }
        }
    }

    //--------------------------
    // release all acquired types
    //--------------------------
    if( pReturnType )
    {
        TYPELIB_DANGER_RELEASE( pReturnType );
    }
    for( i = 0 ; i < nArgCount ; i ++ )
    {
        TYPELIB_DANGER_RELEASE( ppArgType[ i] );
    }
}

}

namespace bridges_remote {

void acquireUno2RemoteStub( remote_Interface *pThis )
{
    Uno2RemoteStub *p = ( Uno2RemoteStub * ) pThis;
    if( 1 == osl_incrementInterlockedCount( &(p->m_nRef) ) )
    {

        p->m_pEnvRemote->pExtEnv->registerProxyInterface(
            p->m_pEnvRemote->pExtEnv,
            (void**)&pThis,
            freeUno2RemoteStub,
            p->m_sOid.pData,
            p->m_pType );

        OSL_ASSERT( (remote_Interface*) p == pThis );
    }
}

void freeUno2RemoteStub(uno_ExtEnvironment *, void * stub) {
    delete static_cast< Uno2RemoteStub * >(stub);
}

Uno2RemoteStub::Uno2RemoteStub( uno_Interface *pUnoI,
                                rtl_uString *pOid,
                                typelib_InterfaceTypeDescription *pType,
                                uno_Environment *pEnvUno,
                                uno_Environment *pEnvRemote ) :
    m_sOid( pOid ),
    m_pType(  pType ),
    m_pUnoI( pUnoI ),
    m_nRef( 1 ),
    m_pEnvUno( pEnvUno ),
    m_pEnvRemote( pEnvRemote ),
    m_mapRemote2Uno( pEnvRemote, pEnvUno ),
    m_mapUno2Remote( pEnvUno, pEnvRemote )
{
    typelib_typedescription_acquire( (typelib_TypeDescription * )m_pType );
    m_pEnvUno->acquire( m_pEnvUno );
    m_pEnvRemote->acquire( m_pEnvRemote );

    acquire = acquireUno2RemoteStub;
    release = thisRelease;
    pDispatcher = thisDispatch;

    m_pEnvUno->pExtEnv->registerInterface( m_pEnvUno->pExtEnv,
                                           (void **)&m_pUnoI,
                                           m_sOid.pData,
                                           m_pType );
    m_pUnoI->acquire( m_pUnoI );
#if OSL_DEBUG_LEVEL > 1
    thisCounter.acquire();
#endif
}

Uno2RemoteStub::~Uno2RemoteStub()
{
    m_pEnvUno->pExtEnv->revokeInterface( m_pEnvUno->pExtEnv , m_pUnoI );

    typelib_typedescription_release( (typelib_TypeDescription * )m_pType );
    m_pUnoI->release( m_pUnoI );
    m_pEnvUno->release( m_pEnvUno );
    m_pEnvRemote->release( m_pEnvRemote );
#if OSL_DEBUG_LEVEL > 1
    thisCounter.release();
#endif
}

} // end namespace bridges_remote
