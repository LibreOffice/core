/*************************************************************************
 *
 *  $RCSfile: urp_job.cxx,v $
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
#include <string.h>
#include <stdio.h>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <rtl/alloc.h>

#include <uno/threadpool.h>

#include <bridges/cpp_uno/type_misc.hxx>
#include <bridges/remote/proxy.hxx>

#include <com/sun/star/uno/Any.hxx>

#include "urp_job.hxx"
#include "urp_bridgeimpl.hxx"
#include "urp_writer.hxx"
#include "urp_dispatch.hxx"
#include "urp_log.hxx"
#include "urp_marshal.hxx"

using namespace ::std;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;


namespace bridges_urp
{

    Job::Job(
        uno_Environment *pEnvRemote,
        sal_Sequence *pTid,
        struct urp_BridgeImpl *pBridgeImpl,
        Unmarshal *pUnmarshal )
        : m_pTid( pTid )
        , m_counter( pEnvRemote )
        , m_pBridgeImpl( pBridgeImpl )
        , m_pUnmarshal( pUnmarshal )
    {
        if( m_pTid )
            rtl_byte_sequence_acquire( pTid );
    }

    Job::~Job()
    {
        if( m_pTid )
            rtl_byte_sequence_release( m_pTid );
    }

    // static method
    void Job::doit( void *pThis )
    {
        Job *pJob = ( Job * ) pThis;
        pJob->execute();
        delete pJob;
    }


    //--------------------------------------------------------------------------------------
    sal_Bool ClientJob::extract(  )
    {
        sal_Bool bReturn = sal_True;

        //--------------------------
        // Handle the reply, unpack data
        //--------------------------
        if( m_bExceptionOccured )
        {
            bReturn = m_pUnmarshal->unpackAny( *m_ppException );
        }
         else
        {
            //---------------------------------
            // alles ist gut
            //---------------------------------
            if( m_pMethodType )
            {
                if( m_pMethodType->pReturnTypeRef->eTypeClass != typelib_TypeClass_VOID )
                {
                    typelib_TypeDescription *pType = 0;
                    TYPELIB_DANGER_GET( &pType , m_pMethodType->pReturnTypeRef );
                    m_pUnmarshal->unpack( m_pReturn , pType );
                    TYPELIB_DANGER_RELEASE( pType );
                }

                // out parameters
                sal_Int32 i;
                for( i = 0 ; i < m_pMethodType->nParams ; i ++ )
                {
                    if( m_pMethodType->pParams[i].bOut )
                    {
                        typelib_TypeDescription *pType = 0;
                        TYPELIB_DANGER_GET( &pType , m_pMethodType->pParams[i].pTypeRef );
                        if( m_pMethodType->pParams[i].bIn )
                        {
                            uno_destructData( m_ppArgs[i] , pType , ::bridges_remote::remote_release );
                        }
                        m_pUnmarshal->unpack( m_ppArgs[i] , pType );
                        TYPELIB_DANGER_RELEASE( pType );
                    }
                }
            }
            else if( m_pAttributeType && m_pReturn )
            {
                typelib_TypeDescription *pType = 0;
                TYPELIB_DANGER_GET( &pType , m_pAttributeType->pAttributeTypeRef );
                m_pUnmarshal->unpack( m_pReturn , pType );
                TYPELIB_DANGER_RELEASE( pType );
            }
            else if( m_pAttributeType && m_ppArgs )
            {
                // nothing to do
            }
            else
            {
                OSL_ASSERT( 0 );
            }
            *m_ppException = 0;
        }
        return sal_True;
    }

    void ClientJob::initiate()
    {
        uno_threadpool_putReply( m_pTid , (sal_Int8*) this );
    }



    //------------------------------------------------------------------------------------
    ServerMultiJob::ServerMultiJob(
        uno_Environment *pEnvRemote,
        sal_Sequence *pTid,
        struct urp_BridgeImpl *pBridgeImpl,
        Unmarshal *pUnmarshal )
        : Job( pEnvRemote, pTid, pBridgeImpl , pUnmarshal )
        , m_pEnvRemote( pEnvRemote )
        , m_pCurrentMem( (sal_Int8*)rtl_allocateMemory( g_nInitialMemorySize ) )
        , m_nCurrentMemPosition( 0 )
        , m_nCalls( 0 )
    {
        m_pEnvRemote->acquire( m_pEnvRemote );
    }

    ServerMultiJob::~ServerMultiJob()
    {
        sal_Int32 i;
        for( i = 0 ; i < m_nCalls ; i ++ )
        {
            struct MemberTypeInfo *const pMTI = &( m_aTypeInfo[i] );
            struct ServerJobEntry *const pSJE = &( m_aEntries[i] );

              if( pSJE->m_pRemoteI )
                  pSJE->m_pRemoteI->release( pSJE->m_pRemoteI );

            if( pSJE->m_pOid )
                rtl_uString_release( pSJE->m_pOid );

            if( pSJE->m_pInterfaceTypeRef )
                typelib_typedescriptionreference_release( pSJE->m_pInterfaceTypeRef );

            if( pMTI->m_pInterfaceType )
                TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pMTI->m_pInterfaceType );

            for( sal_Int32 iArgs = 0 ; iArgs < pMTI->m_nArgCount ; iArgs ++ )
            {
                if( pMTI->m_ppArgType[iArgs] )
                    TYPELIB_DANGER_RELEASE( pMTI->m_ppArgType [iArgs] );
            }
            if( pMTI->m_pReturnType )
                TYPELIB_DANGER_RELEASE( pMTI->m_pReturnType );

            if( pMTI->m_pMethodType )
                typelib_typedescription_release( (typelib_TypeDescription*)pMTI->m_pMethodType );
            if( pMTI->m_pAttributeType )
                typelib_typedescription_release( (typelib_TypeDescription*)pMTI->m_pAttributeType );
        }

        rtl_freeMemory( m_pCurrentMem );
        for( list< sal_Int8 *>::iterator ii = m_lstMem.begin() ; ii != m_lstMem.end() ; ++ii )
            rtl_freeMemory( *ii );

        if( m_pEnvRemote )
            m_pEnvRemote->release( m_pEnvRemote );

    }

    void ServerMultiJob::execute()
    {
        for( sal_Int32 i = 0; i < m_nCalls ; i ++ )
        {
            struct MemberTypeInfo * const pMTI = &( m_aTypeInfo[i] );
            struct ServerJobEntry * const pSJE = &( m_aEntries[i] );

            if( ! pSJE->m_pRemoteI )
            {
                // -------------------
                // Initial object  ?
                // ------------------
                // be robust : Sending a release on a not constructed object
                //             provokes an segfault. Make sure, the call
                //             is not a release call.
                remote_Context *pRemoteC = ((remote_Context*)m_pEnvRemote->pContext);
                if( ! pMTI->m_bIsReleaseCall && pRemoteC->m_pInstanceProvider )
                {
                    pRemoteC->m_pInstanceProvider->getInstance(
                        pRemoteC->m_pInstanceProvider,
                        m_pEnvRemote,
                        &(pSJE->m_pRemoteI),
                        pSJE->m_pOid,
                        pMTI->m_pInterfaceType );
                }

                if( ! pSJE->m_pRemoteI )
                {
                    // Requested object could not be located in this environment,
                    // throw a runtimeexception
                    OUString sMessage( RTL_CONSTASCII_USTRINGPARAM( "urp-bridge: unknown object identifier : " ) );
                    sMessage += pSJE->m_pOid;

                    // prepare the exception
                    prepareRuntimeException( sMessage , i );
                    // no interface to call on, but unmarshaling must continue to update cache
                }
            }

            if( pSJE->m_pException )
            {
                // errors during extracting, do nothing
            }
            else
            {
                pSJE->m_pException = &(pSJE->m_exception );

                if( pMTI->m_bIsReleaseCall )
                {
                    pSJE->m_pRemoteI->release( pSJE->m_pRemoteI );
                    pSJE->m_pException = 0;
                }
                else
                {
                    pSJE->m_pRemoteI->pDispatcher(
                        pSJE->m_pRemoteI,
                        pMTI->m_pMethodType ? (typelib_TypeDescription*) pMTI->m_pMethodType :
                                              (typelib_TypeDescription*) pMTI->m_pAttributeType,
                        pSJE->m_pReturn,
                        pSJE->m_ppArgs,
                        &(pSJE->m_pException) );
                }
            }

            // now destruct parameters and marshal replies
            // Note : when call is synchron => m_nCalls == 1
            if( pMTI->m_pMethodType && pMTI->m_pMethodType->bOneWay )
            {
                // Oneway call, destruct in parameters
                for( sal_Int32 i = 0 ; i < pMTI->m_pMethodType->nParams ; i ++ )
                {
                    // usually all parameters must be in parameters, but to be robust ...
                    if( pMTI->m_pbIsIn[i] && !cppu_isSimpleType( pMTI->m_ppArgType[i] ) )
                    {
                        uno_destructData( pSJE->m_ppArgs[i] , pMTI->m_ppArgType[i] , 0 );
                    }
                }

                if( pSJE->m_pException )
                {
                    uno_any_destruct( pSJE->m_pException, ::bridges_remote::remote_release );
                }

            }
            else
            {
                // synchron, get the mutex to marshal reply and send immeadiatly
                MutexGuard guard( m_pBridgeImpl->m_marshalingMutex );

                sal_Bool bTid = sal_False;
                sal_uInt8 nFlags = HDRFLAG_LONG;
                ByteSequence tid = m_pTid;
                if( !( tid == m_pBridgeImpl->m_lastOutTid ) )
                {
                    // new threadid
                    nFlags = nFlags | HDRFLAG_NEWTID;
                    bTid = sal_True;
                }

                if( pSJE->m_pException )
                {
                    nFlags = nFlags | HDRFLAG_EXCEPTIONOCCURED;
                }
#ifdef BRIDGES_URP_PROT
                sal_Int32 nLogStart = m_pBridgeImpl->m_blockMarshaler.getPos();
#endif
                m_pBridgeImpl->m_blockMarshaler.packInt8( &nFlags );

                if( bTid )
                {

                    m_pBridgeImpl->m_lastOutTid = tid;
                    m_pBridgeImpl->m_blockMarshaler.packTid( tid );
                }

#ifdef BRIDGES_URP_PROT
                sal_Int32 nLogHeader = m_pBridgeImpl->m_blockMarshaler.getPos();
#endif

                if( pSJE->m_pException )
                {
                    //--------------------
                    // an exception was thrown
                    //--------------------
                    m_pBridgeImpl->m_blockMarshaler.packAny( &(pSJE->m_exception) );
                    uno_any_destruct( &(pSJE->m_exception) , ::bridges_remote::remote_release );

                    // destroy in parameters
                    for( sal_Int32 i = 0 ; i < pMTI->m_nArgCount ; i ++ )
                    {
                        if( pMTI->m_pbIsIn[i] && ! cppu_isSimpleType( pMTI->m_ppArgType[i] ))
                        {
                            uno_destructData( pSJE->m_ppArgs[i] , pMTI->m_ppArgType[i] ,
                                              ::bridges_remote::remote_release );
                        }
                    }
                }
                else
                {
                    //---------------------------
                    // alles ist gut ...
                    //--------------------------
                    if( pMTI->m_pReturnType )
                    {
                        m_pBridgeImpl->m_blockMarshaler.pack(
                            pSJE->m_pReturn, pMTI->m_pReturnType );
                        if( ! cppu_isSimpleType( pMTI->m_pReturnType ) )
                        {
                            uno_destructData( pSJE->m_pReturn , pMTI->m_pReturnType ,
                                              ::bridges_remote::remote_release );
                        }
                    }
                    for( sal_Int32 i = 0 ; i < pMTI->m_nArgCount ; i ++ )
                    {
                        if( pMTI->m_pbIsOut[i] )
                        {
                            m_pBridgeImpl->m_blockMarshaler.pack(
                                pSJE->m_ppArgs[i] , pMTI->m_ppArgType[i] );
                        }
                        if( ! cppu_isSimpleType( pMTI->m_ppArgType[i] ) )
                        {
                            uno_destructData( pSJE->m_ppArgs[i], pMTI->m_ppArgType[i] ,
                                              ::bridges_remote::remote_release );
                        }
                    }
                }

#ifdef BRIDGES_URP_PROT
                 {
                     typelib_InterfaceMemberTypeDescription *pMemberType =
                         pMTI->m_pMethodType ?
                         (typelib_InterfaceMemberTypeDescription*)pMTI->m_pMethodType :
                         (typelib_InterfaceMemberTypeDescription*)pMTI->m_pAttributeType;

                     urp_logReplying( m_pBridgeImpl ,
                                      m_pBridgeImpl->m_blockMarshaler.getPos() - nLogStart,
                                      m_pBridgeImpl->m_blockMarshaler.getPos() - nLogHeader,
                                      pMemberType->pMemberName );
                 }
#endif

                if( pSJE->m_pRemoteI )
                {
                    pSJE->m_pRemoteI->release( pSJE->m_pRemoteI );
                    pSJE->m_pRemoteI = 0;
                }

                // put it on the wire
                m_pBridgeImpl->m_pWriter->touch( sal_True );
            } // MutexGuard marshalingMutex
        }
    }

    void ServerMultiJob::prepareRuntimeException( const OUString & sMessage , sal_Int32 nCall )
    {
        // -------------------------------
        // Construct the RuntimeException
        // -------------------------------

        typelib_TypeDescription *pType = 0;
        getCppuType( (RuntimeException * ) 0 ).getDescription(&pType );

        OSL_ENSURE( pType , "urp-bridge: couldn't get RuntimeException type" );

        m_aEntries[nCall].m_pException = &(m_aEntries[nCall].m_exception);
        uno_any_construct( m_aEntries[nCall].m_pException , 0 , pType , 0 );

        typelib_typedescription_release( pType );
    }

    void ServerMultiJob::initiate()
    {
        uno_threadpool_putRequest(
            m_pTid, this, doit,
            m_aTypeInfo[0].m_pMethodType && m_aTypeInfo[0].m_pMethodType->bOneWay );
    }


    // this method is called by the dispatcher thread to unmarshal a request
    sal_Bool ServerMultiJob::extract()
    {
        sal_Bool bContinue = sal_True;
        struct MemberTypeInfo * const pMTI = &(m_aTypeInfo[m_nCalls]);
        struct ServerJobEntry * const pSJE = &(m_aEntries[m_nCalls]);

        pSJE->m_pException = 0;
        pSJE->m_ppArgs = 0;
        pSJE->m_pReturn = 0;
        pMTI->m_pReturnType = 0;

        if( pMTI->m_nArgCount )
        {
            pMTI->m_ppArgType =
                ( typelib_TypeDescription ** ) getHeap( sizeof(void*) * pMTI->m_nArgCount );
            pSJE->m_ppArgs  = (void**) getHeap( sizeof( void * ) * pMTI->m_nArgCount );
            pMTI->m_pbIsIn  = (sal_Bool *) getHeap( sizeof( sal_Bool ) * pMTI->m_nArgCount );
            pMTI->m_pbIsOut = (sal_Bool *) getHeap( sizeof( sal_Bool ) * pMTI->m_nArgCount );
        }
        if( pMTI->m_pMethodType &&
            pMTI->m_pMethodType->pReturnTypeRef->eTypeClass != typelib_TypeClass_VOID )
        {
            TYPELIB_DANGER_GET( &(pMTI->m_pReturnType), pMTI->m_pMethodType->pReturnTypeRef );
        }
        else if( pMTI->m_pAttributeType && ! pMTI->m_nArgCount )
        {
            TYPELIB_DANGER_GET( &(pMTI->m_pReturnType) , pMTI->m_pAttributeType->pAttributeTypeRef );
        }

        // normal method
        if( pMTI->m_pMethodType )
        {
            for( sal_Int32 i = 0 ; i < pMTI->m_nArgCount ; i ++ )
            {
                pMTI->m_ppArgType[i] = 0;
                TYPELIB_DANGER_GET( & ( pMTI->m_ppArgType[i] ) , pMTI->m_pMethodType->pParams[i].pTypeRef);
                pMTI->m_pbIsIn[i]  = pMTI->m_pMethodType->pParams[i].bIn;
                pMTI->m_pbIsOut[i] = pMTI->m_pMethodType->pParams[i].bOut;

                pSJE->m_ppArgs[i] = getHeap( pMTI->m_ppArgType[i]->nSize );
                if( pMTI->m_pbIsIn[i] )
                {
                    // everything needs to be constructed
                    bContinue = m_pUnmarshal->unpack(
                        pSJE->m_ppArgs[i], pMTI->m_ppArgType[i] ) && bContinue;
                }
            }
        }
        else if( pMTI->m_nArgCount )
        {
            // set attribut
            pMTI->m_ppArgType[0] = 0;
            pMTI->m_pbIsIn[0]  = sal_True;
            pMTI->m_pbIsOut[0] = sal_False;
            TYPELIB_DANGER_GET(
                & ( pMTI->m_ppArgType[0] ) , pMTI->m_pAttributeType->pAttributeTypeRef );
            pSJE->m_ppArgs[0] = getHeap( pMTI->m_ppArgType[0]->nSize );
            bContinue = m_pUnmarshal->unpack(
                pSJE->m_ppArgs[0], pMTI->m_ppArgType[0] ) && bContinue;
        }

        if( pMTI->m_pReturnType )
        {
            pSJE->m_pReturn = getHeap( pMTI->m_pReturnType->nSize );
        }

        m_nCalls ++;
        return bContinue;
    }
}


