/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_job.cxx,v $
 * $Revision: 1.17 $
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
#include <string.h>
#include <stdio.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>

#include <uno/current_context.h>
#include <uno/current_context.hxx>
#include <uno/threadpool.h>

#include <bridges/cpp_uno/type_misc.hxx>
#include <bridges/remote/proxy.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include "urp_job.hxx"
#include "urp_bridgeimpl.hxx"
#include "urp_writer.hxx"
#include "urp_dispatch.hxx"
#include "urp_log.hxx"
#include "urp_marshal.hxx"
#include "urp_propertyobject.hxx"
#include "urp_reader.hxx"

using namespace ::std;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

using namespace bridges_urp;

extern "C" {

static void SAL_CALL doit(void * job) {
    ServerMultiJob * p = static_cast< ServerMultiJob * >(job);
    p->execute();
    delete p;
}

}

namespace bridges_urp
{
    static sal_Bool isDisposedExceptionDescriptionAvail( const Type &type )
    {
        static sal_Bool bInit;
        static sal_Bool bReturn;
        // we don't care for thread safety here, as both threads must come
        // to the same result
        if( ! bInit )
        {
            typelib_TypeDescription *pTD = 0;
            typelib_typedescriptionreference_getDescription( & pTD, type.getTypeLibType() );
            if( pTD )
            {
                bReturn = sal_True;
                typelib_typedescription_release( pTD );
            }
            else
            {
                bReturn = sal_False;
            }
            bInit = sal_True;
        }
        return bReturn;
    }

    //--------------------------------------------------------------------------------------
    static void prepareRuntimeExceptionClientSide( uno_Any **ppException , const OUString &s)
    {
        com::sun::star::lang::DisposedException exception( s , Reference< XInterface > () );
        Type type = ::getCppuType( &exception );
        if( !isDisposedExceptionDescriptionAvail( type ) )
        {
            // if it is not available (probably missing type library),
            // then we are satisfied with throwing a normal runtime exception,
            // for which cppu provides a static description
            type = getCppuType( ( RuntimeException * ) 0 );
        }
        uno_type_any_construct( *ppException , &exception , type.getTypeLibType() , 0 );
    }


    Job::Job( uno_Environment *pEnvRemote,
              remote_Context *pContext,
              sal_Sequence *pTid,
              struct urp_BridgeImpl *pBridgeImpl,
              Unmarshal *pUnmarshal )
        : m_pContext( pContext )
        , m_pUnmarshal( pUnmarshal )
        , m_pBridgeImpl( pBridgeImpl )
        , m_pTid( pTid )
        , m_counter( pEnvRemote )
    {
        if ( m_pContext )
        {
            m_pContext->aBase.acquire( &m_pContext->aBase );
        }
        if( m_pTid )
            rtl_byte_sequence_acquire( pTid );
    }

    Job::~Job()
    {
        if( m_pTid )
            rtl_byte_sequence_release( m_pTid );
        if ( m_pContext )
        {
            m_pContext->aBase.release( &m_pContext->aBase );
        }
    }




    //--------------------------------------------------------------------------------------
    sal_Bool ClientJob::extract(  )
    {
        sal_Bool bReturn = sal_True;
        //-------------------------------
        // Handle the reply, unpack data
        //-------------------------------
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
                    bReturn = m_pUnmarshal->unpack( m_pReturn , pType ) && bReturn;
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
                        bReturn = m_pUnmarshal->unpack( m_ppArgs[i] , pType ) && bReturn;
                        TYPELIB_DANGER_RELEASE( pType );
                    }
                }
            }
            else if( m_pAttributeType && m_pReturn )
            {
                typelib_TypeDescription *pType = 0;
                TYPELIB_DANGER_GET( &pType , m_pAttributeType->pAttributeTypeRef );
                bReturn = m_pUnmarshal->unpack( m_pReturn , pType ) && bReturn;
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
        }
        return bReturn;
    }

    //-------------------------------------------------------------------------------------------
    void ClientJob::initiate()
    {
        uno_threadpool_putJob( m_pBridgeImpl->m_hThreadPool, m_pTid , this, 0, sal_False);
    }

    //--------------------------------------------------------------------------------------------
    sal_Bool ClientJob::pack()
    {
        sal_Bool bSuccess = sal_True;
        MutexGuard guard( m_pBridgeImpl->m_marshalingMutex );

        if( m_pMethodType &&
            REMOTE_RELEASE_METHOD_INDEX == m_pMethodType->aBase.nPosition &&
            ! m_pBridgeImpl->m_bDisposed &&
            m_pBridgeImpl->m_pWriter->getIdentifier() != ::osl::Thread::getCurrentIdentifier() )
        {
            // all release calls are delegated to the writer thread to avoid
            // multiple synchron calls with the same thread id and reentrant
            // marshaling (in case during destruction of parameters a release is
            // invoked ).
            m_pBridgeImpl->m_pWriter->insertReleaseRemoteCall(
                m_pOid, m_pInterfaceType->aBase.pWeakRef );

            // No waiting, please
            return sal_False;
        }
        else if ( m_pMethodType &&
                  REMOTE_RELEASE_METHOD_INDEX == m_pMethodType->aBase.nPosition &&
                  m_pBridgeImpl->m_bDisposed )
        {
            // no exception for release calls !
            return sal_False;
        }
        else if( m_pBridgeImpl->m_bDisposed )
        {
            OUStringBuffer buf( 128 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "URP-Bridge: disposed" ) );
            buf.append( m_pBridgeImpl->getErrorsAsString() );
              prepareRuntimeExceptionClientSide( m_ppException , buf.makeStringAndClear() );
            return sal_False;
        }

        // build up the flag byte
        sal_Bool bType = sal_False, bOid = sal_False, bTid = sal_False;
        sal_uInt8 nFlags = 0;
        if( m_pBridgeImpl->m_lastOutType.getTypeLibType() != m_pInterfaceType->aBase.pWeakRef &&
            ! typelib_typedescriptionreference_equals(
                m_pBridgeImpl->m_lastOutType.getTypeLibType(), m_pInterfaceType->aBase.pWeakRef ) )
        {
            //new type
            nFlags = nFlags | HDRFLAG_NEWTYPE;
            bType = sal_True;
        }
        if( m_pBridgeImpl->m_lastOutOid.pData != m_pOid &&
            rtl_ustr_compare_WithLength( m_pBridgeImpl->m_lastOutOid.pData->buffer,
                                         m_pBridgeImpl->m_lastOutOid.pData->length,
                                         m_pOid->buffer,
                                         m_pOid->length ) )
        {
            //new object id
            nFlags = nFlags | HDRFLAG_NEWOID;
            bOid = sal_True;
        }
        if( m_pBridgeImpl->m_lastOutTid.getHandle() != m_pTid &&
            !(m_pBridgeImpl->m_lastOutTid == *(ByteSequence*) &(m_pTid) ) )
        {
            // new threadid
            nFlags = nFlags | HDRFLAG_NEWTID;
            bTid = sal_True;
        }

        if( m_bCallingConventionForced )
        {
            nFlags = nFlags | HDRFLAG_MOREFLAGS;
        }
#ifdef BRIDGES_URP_PROT
        sal_Int32 nLogStart = m_pBridgeImpl->m_blockMarshaler.getPos();
#endif
        // Short request headers can only handle 14-bit method IDs, so
        // unconditionally use a long header for method IDs that are too large:
        if( nFlags || m_nMethodIndex >= 0xC000 )
        {
            // the flag byte is needed + request
            nFlags = nFlags | HDRFLAG_LONGHEADER | HDRFLAG_REQUEST; //

            // as long as we do not have customized calls, no MOREFLAGS must be set
            if( m_nMethodIndex >= 0x100 )
            {
                nFlags = nFlags | HDRFLAG_LONGMETHODID;
            }
            m_pBridgeImpl->m_blockMarshaler.packInt8( &nFlags  );

            if( nFlags & HDRFLAG_MOREFLAGS )
            {
                sal_uInt8 nMoreFlags = 0;
                if( ! m_bOneway )
                {
                    nMoreFlags = HDRFLAG_SYNCHRONOUS |HDRFLAG_MUSTREPLY;
                }
                m_pBridgeImpl->m_blockMarshaler.packInt8( &nMoreFlags  );
            }
            if( nFlags & HDRFLAG_LONGMETHODID )
            {
                sal_uInt16 nMethod = (sal_uInt16 ) m_nMethodIndex;
                m_pBridgeImpl->m_blockMarshaler.packInt16( &nMethod  );
            }
            else
            {
                sal_uInt8 nMethod = (sal_uInt8) m_nMethodIndex;
                m_pBridgeImpl->m_blockMarshaler.packInt8( &nMethod );
            }
        }
        else
        {
            // no flag byte needed, simply marshal the method index
            if( m_nMethodIndex >= 64 )
            {
                sal_uInt16 nMethod = ( sal_uInt16 ) m_nMethodIndex;
                nMethod = nMethod | 0x4000;
                m_pBridgeImpl->m_blockMarshaler.packInt16( &nMethod );
            }
            else
            {
                sal_uInt8 nMethod = (sal_uInt8 ) m_nMethodIndex;
                m_pBridgeImpl->m_blockMarshaler.packInt8( &nMethod );
            }
        }

        // marshal type,oid,tid
        if( bType )
        {
            m_pBridgeImpl->m_lastOutType = m_pInterfaceType->aBase.pWeakRef;
            m_pBridgeImpl->m_blockMarshaler.packType( &(m_pBridgeImpl->m_lastOutType) );
        }
        if( bOid )
        {
            m_pBridgeImpl->m_lastOutOid = *(OUString *)&m_pOid;
            m_pBridgeImpl->m_blockMarshaler.packOid( m_pBridgeImpl->m_lastOutOid );
        }
        if( bTid )
        {
            m_pBridgeImpl->m_lastOutTid = *(ByteSequence*)&(m_pTid);
            m_pBridgeImpl->m_blockMarshaler.packTid( m_pBridgeImpl->m_lastOutTid );
        }

        if ( m_pBridgeImpl->m_properties.bCurrentContext
             && m_nMethodIndex != REMOTE_RELEASE_METHOD_INDEX
             && m_pContext != 0 )
        {
            void * pCc = 0;
            rtl::OUString aEnvName( RTL_CONSTASCII_USTRINGPARAM( "urp" ) );
            bSuccess = bSuccess && uno_getCurrentContext(
                &pCc, aEnvName.pData, m_pContext );
            typelib_TypeDescription * pType = 0;
            TYPELIB_DANGER_GET(
                &pType, XCurrentContext::static_type().getTypeLibType() );
            bSuccess = bSuccess && m_pBridgeImpl->m_blockMarshaler.pack(
                &pCc, pType );
            TYPELIB_DANGER_RELEASE( pType );
            if ( pCc )
            {
                remote_Interface * p = static_cast< remote_Interface * >( pCc );
                p->release( p );
            }
        }

        // marshal arguments !
#ifdef BRIDGES_URP_PROT
        sal_Int32 nLogHeader = m_pBridgeImpl->m_blockMarshaler.getPos();
#endif
        if( m_pMethodType )
        {
            sal_Int32 i;
            for( i = 0 ; i < m_pMethodType->nParams ; i ++ )
            {
                if( m_pMethodType->pParams[i].bIn )
                {
                      typelib_TypeDescription *pType = 0;
                      TYPELIB_DANGER_GET( &pType , m_pMethodType->pParams[i].pTypeRef );
                    if( pType )
                    {
                        bSuccess =
                            bSuccess && m_pBridgeImpl->m_blockMarshaler.pack( m_ppArgs[i] , pType );
                        TYPELIB_DANGER_RELEASE( pType );
                    }
                    else
                    {
                        bSuccess = sal_False;
                        OUStringBuffer buffer( 128 );
                        buffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "no typedescription available for type" ) );
                        buffer.append( m_pMethodType->pParams[i].pTypeRef->pTypeName );
                        m_pBridgeImpl->addError( buffer.makeStringAndClear() );
                    }
                }
            }
        }
        else if( m_pAttributeType && m_pReturn )
        {
            // nothing to do !
        }
        else if( m_pAttributeType && m_ppArgs )
        {
              typelib_TypeDescription *pType = 0;
              TYPELIB_DANGER_GET( &pType , m_pAttributeType->pAttributeTypeRef );
            if( pType )
            {
                bSuccess = bSuccess && m_pBridgeImpl->m_blockMarshaler.pack( m_ppArgs[0] , pType );
                TYPELIB_DANGER_RELEASE( pType );
            }
            else
            {
                bSuccess = sal_False;
                OUStringBuffer buffer( 128 );
                buffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "no typedescription available for type" ) );
                buffer.append( m_pAttributeType->pAttributeTypeRef->pTypeName );
                m_pBridgeImpl->addError( buffer.makeStringAndClear() );
            }
        }
        else
        {
            OSL_ASSERT( 0 );
        }

#ifdef BRIDGES_URP_PROT
        urp_logCall( m_pBridgeImpl, m_pBridgeImpl->m_blockMarshaler.getPos() - nLogStart,
                     m_pBridgeImpl->m_blockMarshaler.getPos() - nLogHeader, ! m_bOneway,
                     m_pMethodType ? m_pMethodType->aBase.pMemberName :
                                               m_pAttributeType->aBase.pMemberName );
#endif

        if( bSuccess )
        {
            if( ! m_bOneway )
            {
                uno_threadpool_attach( m_pBridgeImpl->m_hThreadPool );
                m_pBridgeImpl->m_clientJobContainer.add( *(ByteSequence*)&(m_pTid), this );
            }

            m_pBridgeImpl->m_nMarshaledMessages ++;
            //---------------------------
            // Inform the writer thread, that there is some work to do
            //---------------------------
            m_pBridgeImpl->m_pWriter->touch( ! m_bOneway );

            if( m_bOneway )
            {
                *m_ppException = 0;
            }
        }
        else
        {
            // Something went wrong during packing, which means, that the caches may not be in sync
            // anymore. So we have no other choice than to dispose the environment.
            m_pEnvRemote->dispose( m_pEnvRemote );
            OUStringBuffer buf( 128 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "Error during marshaling " )  );
            if( m_pMethodType )
            {
                buf.append( m_pMethodType->aBase.aBase.pTypeName );
            }
            else if( m_pAttributeType )
            {
                buf.append( m_pAttributeType->aBase.aBase.pTypeName );
            }
            buf.appendAscii( "\n" );
            buf.append( m_pBridgeImpl->getErrorsAsString() );
            prepareRuntimeExceptionClientSide( m_ppException , buf.makeStringAndClear() );
        }
        return bSuccess;
        // release the guard
    }

    //------------------------------------------------------------------------------------
    void ClientJob::wait()
    {
        //---------------------------
        // Wait for the reply
        //---------------------------
        void * pDisposeReason = 0;

        uno_threadpool_enter(m_pBridgeImpl->m_hThreadPool, &pDisposeReason );

        if( ! pDisposeReason )
        {
            // thread has been disposed !
            // avoid leak due continous calling on a disposed reference. The
            // reply may or may not be within the container. If the reader thread
            // got into problems during unmarshaling the reply for this request,
            // it won't be in the container anymore, but it is eiterway safe to call
            // the method
            ClientJob *pJob =
                m_pBridgeImpl->m_clientJobContainer.remove( *(ByteSequence*) &m_pTid );
            if( pJob != this )
            {
                // this is not our job, it is probably one of the callstack below, so
                // push it back
                m_pBridgeImpl->m_clientJobContainer.add( *(ByteSequence*) &m_pTid , pJob );
            }

            OUStringBuffer sMessage( 256 );
            sMessage.appendAscii( RTL_CONSTASCII_STRINGPARAM( "URP_Bridge : disposed\n" ) );
            sMessage.append( m_pBridgeImpl->getErrorsAsString() );
            prepareRuntimeExceptionClientSide( m_ppException, sMessage.makeStringAndClear() );
            m_bExceptionOccured = sal_True;
        }
        else
        {
            OSL_ASSERT( pDisposeReason == (void*)this );
        }
        if( !m_bExceptionOccured )
        {
            *m_ppException = 0;
        }
        uno_threadpool_detach( m_pBridgeImpl->m_hThreadPool );
    }

    //------------------------------------------------------------------------------------
    // ServerMultiJob
    //------------------------------------------------------------------------------------
    ServerMultiJob::ServerMultiJob(
        uno_Environment *pEnvRemote,
        remote_Context *pContext,
        sal_Sequence *pTid,
        struct urp_BridgeImpl *pBridgeImpl,
        Unmarshal *pUnmarshal,
        sal_Int32 nMaxMessages )
        : Job( pEnvRemote, pContext, pTid, pBridgeImpl, pUnmarshal )
        , m_pEnvRemote( pEnvRemote )
        , m_nCalls( 0 )
        , m_nMaxMessages( nMaxMessages )
        , m_nCurrentMemPosition( 0 )
    {
        m_pEnvRemote->acquire( m_pEnvRemote );
        m_nCurrentMemSize = MULTIJOB_STANDARD_MEMORY_SIZE + m_nMaxMessages * (
            MULTIJOB_PER_CALL_MEMORY_SIZE + sizeof(ServerJobEntry) + sizeof(MemberTypeInfo) );
        m_pCurrentMem = ( sal_Int8 * ) rtl_allocateMemory( m_nCurrentMemSize );
        m_aEntries = ( ServerJobEntry * ) getHeap( m_nMaxMessages * sizeof( ServerJobEntry ) );
        m_aTypeInfo = ( MemberTypeInfo * ) getHeap( m_nMaxMessages * sizeof( MemberTypeInfo ) );
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

    //-------------------------------------------------------------------------------------
    void ServerMultiJob::execute()
    {
        Reference< XCurrentContext > xOldCc;
        bool bHasOldCc = false;
        for( sal_Int32 i = 0; i < m_nCalls ; i ++ )
        {
            struct MemberTypeInfo * const pMTI = &( m_aTypeInfo[i] );
            struct ServerJobEntry * const pSJE = &( m_aEntries[i] );

            if ( pSJE->m_bHasCurrentContext )
            {
                if ( !bHasOldCc )
                {
                    xOldCc = com::sun::star::uno::getCurrentContext();
                    bHasOldCc = true;
                }
                rtl::OUString aEnvName( RTL_CONSTASCII_USTRINGPARAM( "urp" ) );
                if ( !uno_setCurrentContext(
                         pSJE->m_pCurrentContext, aEnvName.pData, m_pContext ) )
                {
                    throw RuntimeException(
                        rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "fatal: uno_setCurrentContext failed" ) ),
                        Reference< XInterface >() );
                }
                if ( pSJE->m_pCurrentContext )
                {
                    pSJE->m_pCurrentContext->release( pSJE->m_pCurrentContext );
                }
            }

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
                    pSJE->m_pException = &(pSJE->m_exception);

                    pRemoteC->m_pInstanceProvider->getInstance(
                        pRemoteC->m_pInstanceProvider,
                        m_pEnvRemote,
                        &(pSJE->m_pRemoteI),
                        pSJE->m_pOid,
                        pMTI->m_pInterfaceType,
                        &(pSJE->m_pException));
                }
                else
                {
                    prepareRuntimeException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "urp: No instance provider set")),i);
                }
            }

            if( pSJE->m_pException )
            {
                // errors during extracting, do nothing
            }
            else if( ! pSJE->m_pRemoteI )
            {
                // May only occur during the queryInterface call on the initial object !!!
                // construct the return value
                 uno_type_any_construct( (uno_Any*) pSJE->m_pReturn , 0 , 0 , 0 );
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
            if( pSJE->m_pRemoteI )
            {
                /**
                   Do the release here, in case of ForceSynchronous=1, calls
                   originated by the destructor of an UNO object must be sent BEFORE the
                   release returns ( otherwise we don't own the thread anymore ! )
                */
                pSJE->m_pRemoteI->release( pSJE->m_pRemoteI );
                pSJE->m_pRemoteI = 0;
            }

            // now destruct parameters and marshal replies
            // Note : when call is synchron => m_nCalls == 1
              if( pMTI->m_bIsOneway )
            {
                // Oneway call, destruct in parameters
                for( sal_Int32 j = 0 ; j < pMTI->m_pMethodType->nParams ; j ++ )
                {
                    // usually all parameters must be in parameters, but to be robust ...
                    if( pMTI->m_pbIsIn[j] && !cppu_isSimpleType( pMTI->m_ppArgType[j] ) )
                    {
                        uno_destructData( pSJE->m_ppArgs[j] , pMTI->m_ppArgType[j] , 0 );
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
                sal_uInt8 nFlags = HDRFLAG_LONGHEADER;
                ByteSequence tid = m_pTid;
                if( !( tid == m_pBridgeImpl->m_lastOutTid ) || pSJE->m_bIgnoreCache )
                {
                    // new threadid
                    nFlags = nFlags | HDRFLAG_NEWTID;
                    bTid = sal_True;
                }

                if( pSJE->m_pException )
                {
                    nFlags = nFlags | HDRFLAG_EXCEPTION;
                }
#ifdef BRIDGES_URP_PROT
                sal_Int32 nLogStart = m_pBridgeImpl->m_blockMarshaler.getPos();
#endif
                m_pBridgeImpl->m_blockMarshaler.packInt8( &nFlags );

                if( bTid )
                {
                    if( ! pSJE->m_bIgnoreCache )
                    {
                        m_pBridgeImpl->m_lastOutTid = tid;
                    }
                    m_pBridgeImpl->m_blockMarshaler.packTid( tid , pSJE->m_bIgnoreCache );
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
                    for( sal_Int32 j = 0 ; j < pMTI->m_nArgCount ; j ++ )
                    {
                        if( pMTI->m_pbIsIn[j] && ! cppu_isSimpleType( pMTI->m_ppArgType[j] ))
                        {
                            uno_destructData( pSJE->m_ppArgs[j] , pMTI->m_ppArgType[j] ,
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
                    for( sal_Int32 j = 0 ; j < pMTI->m_nArgCount ; j ++ )
                    {
                        if( pMTI->m_pbIsOut[j] )
                        {
                            m_pBridgeImpl->m_blockMarshaler.pack(
                                pSJE->m_ppArgs[j] , pMTI->m_ppArgType[j] );
                        }
                        if( ! cppu_isSimpleType( pMTI->m_ppArgType[j] ) )
                        {
                            uno_destructData( pSJE->m_ppArgs[j], pMTI->m_ppArgType[j] ,
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

                m_pBridgeImpl->m_nMarshaledMessages ++;
                // put it on the wire
                m_pBridgeImpl->m_pWriter->touch( sal_True );
            } // MutexGuard marshalingMutex
        }
        if ( bHasOldCc )
        {
            if ( !com::sun::star::uno::setCurrentContext( xOldCc ) )
            {
                throw RuntimeException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "fatal: com::sun::star::uno::setCurrentContext"
                            " failed" ) ),
                    Reference< XInterface >() );
            }
        }
    }

    //-------------------------------------------------------------------------------------
    void ServerMultiJob::prepareRuntimeException( const OUString & sMessage , sal_Int32 nCall )
    {
        // -------------------------------
        // Construct the DisposedException
        // -------------------------------
        com::sun::star::lang::DisposedException exception( sMessage , Reference< XInterface > () );
        Type type = getCppuType( &exception );
        if( !isDisposedExceptionDescriptionAvail( type ) )
        {
            // if it is not available (probably missing type library),
            // then we are satisfied with throwing a normal runtime exception,
            // for which cppu provides a static description
            type = getCppuType( ( RuntimeException * ) 0 );
        }

        m_aEntries[nCall].m_pException = &(m_aEntries[nCall].m_exception);
        uno_type_any_construct( m_aEntries[nCall].m_pException , &exception , type.getTypeLibType() , 0 );
    }

    //-------------------------------------------------------------------------------------
    void ServerMultiJob::initiate()
    {
        uno_threadpool_putJob(
            m_pBridgeImpl->m_hThreadPool,
            m_pTid,
            this,
            doit,
            m_aTypeInfo[0].m_bIsOneway );
    }


    //-------------------------------------------------------------------------------------
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


