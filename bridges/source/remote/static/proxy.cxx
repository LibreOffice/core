/*************************************************************************
 *
 *  $RCSfile: proxy.cxx,v $
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
#include <assert.h>
#ifdef SOLARIS
#include <alloca.h>
#else
#include <malloc.h>
#endif

#include <bridges/remote/proxy.hxx>
#include <bridges/remote/context.h>

#include <uno/data.h>
#include <uno/mapping.hxx>
#include <uno/environment.h>

#include <com/sun/star/uno/Any.hxx>

#include <bridges/remote/bridgeimpl.hxx>

#include "remote_types.hxx"

#ifdef DEBUG
#include <bridges/remote/counter.hxx>
static MyCounter thisCounter( "DEBUG : Remote2UnoProxy");
#endif

using namespace ::bridges_remote;
using namespace ::com::sun::star::uno;

namespace bridges_remote {

Remote2UnoProxy::Remote2UnoProxy( remote_Interface *pRemoteI,
                                  rtl_uString *pOid,
                                  typelib_InterfaceTypeDescription *pType,
                                  uno_Environment *pEnvUno,
                                  uno_Environment *pEnvRemote ) :
    m_pType( pType ),
    m_pRemoteI( pRemoteI ),
    m_pEnvUno( pEnvUno ),
    m_pEnvRemote( pEnvRemote ),
    m_sOid( pOid ),
    m_nRef( 1 ),
    m_mapRemote2Uno( pEnvRemote, pEnvUno ),
    m_mapUno2Remote( pEnvUno , pEnvRemote )
{
    typelib_typedescription_acquire( (typelib_TypeDescription * ) m_pType );
    m_pEnvUno->acquire( m_pEnvUno );
    m_pEnvRemote->acquire( m_pEnvRemote );

    acquire = thisAcquire;
    release = thisRelease;
    pDispatcher = ( uno_DispatchMethod ) thisDispatch;

    m_pEnvRemote->pExtEnv->registerInterface(
        m_pEnvRemote->pExtEnv,
        (void**)&m_pRemoteI,
        m_sOid.pData,
        m_pType );
    m_pRemoteI->acquire( m_pRemoteI );

#ifdef DEBUG
    thisCounter.acquire();
#endif
}

Remote2UnoProxy::~Remote2UnoProxy()
{
    // revoke external ref (oid)
    m_pEnvRemote->pExtEnv->revokeInterface( m_pEnvRemote->pExtEnv , m_pRemoteI );

    typelib_typedescription_release( (typelib_TypeDescription * )m_pType );
    m_pRemoteI->release( m_pRemoteI );
    m_pEnvUno->release( m_pEnvUno );
    m_pEnvRemote->release( m_pEnvRemote );
#ifdef DEBUG
    thisCounter.release();
#endif
}

void Remote2UnoProxy::thisFree( uno_ExtEnvironment *pEnvUno , void *pThis )
{
    delete (Remote2UnoProxy*) pThis;
}

void Remote2UnoProxy::thisAcquire( uno_Interface *pThis )
{
    Remote2UnoProxy *p = ( Remote2UnoProxy * ) pThis;
    if( 1 == osl_incrementInterlockedCount( &(p->m_nRef) ) )
    {
        p->m_pEnvUno->pExtEnv->registerProxyInterface(
            p->m_pEnvUno->pExtEnv,
            (void**)&pThis,
            Remote2UnoProxy::thisFree,
            p->m_sOid.pData,
            p->m_pType );
        assert( (uno_Interface *)p == pThis );
    }
}

void Remote2UnoProxy::thisRelease( uno_Interface *pThis )
{
    Remote2UnoProxy *p = ( Remote2UnoProxy * ) pThis;
    if ( 0 == osl_decrementInterlockedCount( &(p->m_nRef) ))
    {
        p->m_pEnvUno->pExtEnv->revokeInterface( p->m_pEnvUno->pExtEnv, p );
    }
}

void SAL_CALL remote_release( void *pRemoteI )
{
    ((remote_Interface * )pRemoteI)->release( (remote_Interface * ) pRemoteI );
}

void Remote2UnoProxy::thisDispatch(
    uno_Interface * pUnoI,
    typelib_TypeDescription * pType,
    void * pReturn,
    void * ppArgs[],
    uno_Any ** ppException )
{
    Remote2UnoProxy *p = ( Remote2UnoProxy * ) pUnoI;
    RemoteThreadCounter counter( p->m_pEnvRemote );

    typelib_InterfaceMethodTypeDescription *pMethodType = 0;
    typelib_InterfaceAttributeTypeDescription *pAttributeType = 0;
    typelib_TypeDescription *pReturnType = 0;
    typelib_TypeDescription **ppArgType = 0;
    sal_Int32 nArgCount = 0;
    sal_Bool *pbIsIn = 0;
    sal_Bool *pbIsOut = 0;
    sal_Bool *pbConversionNeeded = 0;
    sal_Bool bConversionNeededForReturn = 0;

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
            pbConversionNeeded = ( sal_Bool *) alloca( sizeof( sal_Bool ) );

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
        pbConversionNeeded = ( sal_Bool *) alloca( sizeof( sal_Bool ) * nArgCount );
        sal_Int32 i;
        for( i = 0 ; i < nArgCount ; i ++ )
        {
            ppArgType[i] = 0;
            TYPELIB_DANGER_GET( & (ppArgType[i]) ,  pMethodType->pParams[i].pTypeRef );
            pbIsIn[i]  = pMethodType->pParams[i].bIn;
            pbIsOut[i] = pMethodType->pParams[i].bOut;
            pbConversionNeeded[i] = remote_relatesToInterface( ppArgType[i] );
        }
    }

    void *pRemoteReturn = 0;
    if( pReturnType )
    {
        if( bConversionNeededForReturn )
        {
            pRemoteReturn = alloca( pReturnType->nSize );
        }
        else
        {
            pRemoteReturn = pReturn;
        }
    }

    void ** ppRemoteArgs = 0;
    if( nArgCount )
    {
        ppRemoteArgs = (void**) alloca( sizeof( void * ) * nArgCount );
    }

    sal_Int32 i;
    for( i = 0 ; i < nArgCount ; i ++ )
    {
        if( pbConversionNeeded[i] )
        {
            ppRemoteArgs[i] = alloca( ppArgType[i]->nSize );

            if( pbIsIn[i] ) {
                uno_copyAndConvertData(
                    ppRemoteArgs[i],
                    ppArgs[i],
                    ppArgType[i],
                    p->m_mapUno2Remote.get() );
            }
        }
        else
        {
            ppRemoteArgs[i] = ppArgs[i];
        }
    }

    uno_Any any;
    uno_Any *pAny = &any;

    p->m_pRemoteI->pDispatcher( p->m_pRemoteI,
                                pType,
                                pRemoteReturn,
                                ppRemoteArgs,
                                &pAny );

    if( ! pAny )
    {
        if( pReturn && bConversionNeededForReturn )
        {
            uno_copyAndConvertData(
                pReturn ,
                pRemoteReturn,
                pReturnType,
                p->m_mapRemote2Uno.get() );
            uno_destructData( pRemoteReturn , pReturnType , remote_release );
        }

        sal_Int32 i;
        for( i = 0 ; i < nArgCount ; i ++ )
        {
            if( pbConversionNeeded[i] )
            {
                if( pbIsIn[i] ) {
                    if( pbIsOut[i] ) {
                        uno_destructData( ppArgs[i] ,
                                          ppArgType[i] ,
                                          0 );
                        uno_copyAndConvertData( ppArgs[i] ,
                                                ppRemoteArgs[i],
                                                ppArgType[i],
                                                p->m_mapRemote2Uno.get()  );
                    }
                }
                else // pure out
                {
                    uno_copyAndConvertData( ppArgs[i] ,
                                            ppRemoteArgs[i],
                                            ppArgType[i],
                                            p->m_mapRemote2Uno.get() );
                }
                uno_destructData( ppRemoteArgs[i],
                                  ppArgType[i],
                                  remote_release );
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
        getCppuType( (::com::sun::star::uno::Any*) 0 ).getDescription( &pAnyType );
        uno_copyAndConvertData( *ppException ,
                                pAny ,
                                pAnyType,
                                p->m_mapRemote2Uno.get() );
        uno_destructData( pAny , pAnyType , remote_release );
        typelib_typedescription_release( pAnyType );

        // destruct remote in parameters ( out parameters have not been constructed )
        for( i = 0 ; i < nArgCount ; i ++ )
        {
            if( pbConversionNeeded[i] &&  pbIsIn[i] )
            {
                uno_destructData( ppRemoteArgs[i],
                                  ppArgType[i],
                                  remote_release );
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

} // end namespace bridge_remote
