/*************************************************************************
 *
 *  $RCSfile: urp_reader.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-20 16:44:05 $
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

#include <osl/diagnose.h>

#include <bridges/remote/connection.h>
#include <bridges/remote/counter.hxx>
#include <bridges/remote/context.h>
#include <bridges/remote/helper.hxx>

#include <uno/environment.h>

#include "urp_reader.hxx"
#include "urp_dispatch.hxx"
#include "urp_job.hxx"
#include "urp_bridgeimpl.hxx"
#include "urp_log.hxx"
#include "urp_propertyobject.hxx"

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

#ifdef DEBUG
static MyCounter thisCounter( "DEBUG : ReaderThread" );
#endif

namespace bridges_urp
{
    struct MessageFlags
    {
        sal_uInt16 nMethodId;
        sal_Bool bRequest;
        sal_Bool bType;
        sal_Bool bOid;
        sal_Bool bTid;
        sal_Bool bException;
        sal_Bool bMustReply;
        sal_Bool bSynchronous;
        sal_Bool bMoreFlags;
        sal_Bool bIgnoreCache;
        sal_Bool bBridgePropertyCall;
        ///--------------------------
        inline MessageFlags()
            {
                bTid = sal_False;
                bOid = sal_False;
                bType = sal_False;
                bException = sal_False;
                bMoreFlags = sal_False;
                bIgnoreCache = sal_False;
                bBridgePropertyCall = sal_False;
            }
        //---------------------------
    }; // end struct MessageFlags


    inline sal_Bool getMemberTypeDescription(
        typelib_InterfaceAttributeTypeDescription **ppAttributeType,
        typelib_InterfaceMethodTypeDescription **ppMethodType,
        sal_Bool *pbIsSetter,
        sal_uInt16 nMethodId ,
        const Type &typeInterface)
    {
        if( typeInterface.getTypeClass() != typelib_TypeClass_INTERFACE )
        {
            OSL_ENSURE( 0 , "type is not an interface" );
            return sal_False;
        }

        typelib_InterfaceTypeDescription *pInterfaceType = 0;
        TYPELIB_DANGER_GET(
            (typelib_TypeDescription **)&pInterfaceType ,   typeInterface.getTypeLibType() );
        if( ! pInterfaceType )
        {
            OString o = OUStringToOString( typeInterface.getTypeName() , RTL_TEXTENCODING_ASCII_US );
            OSL_ENSURE( !"urp: unknown type " , o.getStr() );
            return sal_False;
        }

        if( ! pInterfaceType->aBase.bComplete )
        {
            typelib_typedescription_complete( (typelib_TypeDescription **) &pInterfaceType );
        }

        if( nMethodId < 0 || nMethodId > pInterfaceType->nAllMembers *2 )
        {
            // ( m_nMethodId > m_pInterfaceType->nAllMembers *2) is an essential condition
            // for the vtable index to be correct
            OSL_ENSURE( 0 , "vtable index out of range" );
            return sal_False;
        }

        // TODO : check the range of m_nMethodId
        sal_Int32 nMemberIndex = pInterfaceType->pMapFunctionIndexToMemberIndex[ nMethodId ];

        if( !( pInterfaceType->nAllMembers > nMemberIndex && nMemberIndex >= 0 ) )
        {
            OSL_ENSURE( 0 , "vtable index out of range" );
            return sal_False;
        }

        typelib_InterfaceMemberTypeDescription *pMemberType = 0;
        typelib_typedescriptionreference_getDescription(
            (typelib_TypeDescription **) &pMemberType,pInterfaceType->ppAllMembers[nMemberIndex]);

        if(! pMemberType )
        {
            OSL_ENSURE( 0 , "unknown method type description" );
            return sal_False;
        }

        if( typelib_TypeClass_INTERFACE_ATTRIBUTE == pMemberType->aBase.eTypeClass )
        {
            *ppAttributeType = (typelib_InterfaceAttributeTypeDescription *) pMemberType;
            *pbIsSetter = ! (
                pInterfaceType->pMapMemberIndexToFunctionIndex[nMemberIndex] == nMethodId );
        }
        else
        {
            *ppMethodType = (typelib_InterfaceMethodTypeDescription *) pMemberType;
        }

        TYPELIB_DANGER_RELEASE( (typelib_TypeDescription * )pInterfaceType );
        return sal_True;
    }

OReaderThread::OReaderThread( remote_Connection *pConnection,
                              uno_Environment *pEnvRemote,
                              OWriterThread * pWriterThread ) :
    m_pConnection( pConnection ),
    m_pEnvRemote( pEnvRemote ),
    m_pWriterThread( pWriterThread ),
    m_bDestroyMyself( sal_False ),
    m_pBridgeImpl((struct urp_BridgeImpl*)
                  ((remote_Context *)pEnvRemote->pContext)->m_pBridgeImpl ),
    m_unmarshal( m_pBridgeImpl, m_pEnvRemote, ::bridges_remote::remote_createStub )
{
    m_pConnection->acquire( m_pConnection );
#ifdef DEBUG
    thisCounter.acquire();
#endif
}


OReaderThread::~OReaderThread( )
{
#ifdef DEBUG
    thisCounter.release();
#endif
}

// may only be called in the callstack of this thread !!!!!
// run() -> dispose() -> destroyYourself()
void OReaderThread::destroyYourself()
{
    m_bDestroyMyself = sal_True;
    m_pConnection->release( m_pConnection );
    m_pConnection = 0;
}

void OReaderThread::onTerminated()
{
    if( m_bDestroyMyself )
    {
        delete this;
    }
}


void OReaderThread::disposeEnvironment()
{
    struct remote_Context *pContext =
        ( struct remote_Context * ) m_pEnvRemote->pContext;
    if( ! pContext->m_pBridgeImpl->m_bDisposed )
    {
        // NOTE : This method may only be called by the run-method.
        //        The remote-environment does not dispose, until the reader thread has gone.
        //        So it is completly OK to hold the environment weakly, and only make a
        //        hard reference during dispose call.
        m_pEnvRemote->acquire( m_pEnvRemote );
        m_pEnvRemote->dispose( m_pEnvRemote );
        m_pEnvRemote->release( m_pEnvRemote );
    }
}

inline sal_Bool OReaderThread::readBlock( sal_Int32 *pnMessageCount )
{
    m_unmarshal.setSize( 8 );
    if( 8 != m_pConnection->read( m_pConnection , m_unmarshal.getBuffer(), 8 ) )
    {
        return sal_False;
    }

    sal_Int32 nSize;
    m_unmarshal.unpackInt32( &nSize );
    m_unmarshal.unpackInt32( pnMessageCount );

    if( nSize < 0 )
    {
        // buffer too big
        // no exception can be thrown, because there is no thread id, which could be
        // used. -> terminate !
        OSL_ENSURE( 0 , "urp bridge: invalid message size, terminating connection." );
        disposeEnvironment();
        return sal_False;
    }

    if( 0 == nSize )
    {
        return sal_False;
    }

    // allocate the necessary memory
    if( ! m_unmarshal.setSize( nSize ) )
    {
        OSL_ENSURE( 0 , "urp bridge: messages size too large, terminating connection" );
        return sal_False;
    }

    sal_Int32 nRead = m_pConnection->read( m_pConnection , m_unmarshal.getBuffer() , nSize );

    if( nSize != nRead )
    {
        // couldn't get the asked amount of bytes, quit
        // should only occur, when the environment has already been disposed
        OSL_ENSURE( m_pBridgeImpl->m_bDisposed , "urp bridge: couldn't read complete message, terminating connection." );
        return sal_False;
    }
    return sal_True;
}

inline sal_Bool OReaderThread::readFlags( struct MessageFlags *pFlags )
{
    sal_uInt8 nBitField;
    if( ! m_unmarshal.unpackInt8( &nBitField ) )
    {
        return sal_False;
    }

    if( HDRFLAG_LONGHEADER & nBitField )
    {
        // this is a long header, interpret the byte as bitfield
        pFlags->bTid     = (HDRFLAG_NEWTID & nBitField );
        pFlags->bRequest = (HDRFLAG_REQUEST & nBitField);

        if( pFlags->bRequest )
        {
            // request
            pFlags->bType = ( HDRFLAG_NEWTYPE & nBitField );
            pFlags->bOid  = ( HDRFLAG_NEWOID & nBitField );
            pFlags->bIgnoreCache = ( HDRFLAG_IGNORECACHE & nBitField );
            pFlags->bMoreFlags = ( HDRFLAG_MOREFLAGS & nBitField );

            if( pFlags->bMoreFlags )
            {
                // another byte with flags
                sal_Int8 moreFlags;
                if( ! m_unmarshal.unpackInt8( &moreFlags ) )
                {
                    return sal_False;
                }
                pFlags->bSynchronous = ( HDRFLAG_SYNCHRONOUS & moreFlags );
                pFlags->bMustReply = ( HDRFLAG_MUSTREPLY & moreFlags );
                OSL_ENSURE( pFlags->bSynchronous && pFlags->bMustReply ||
                            ! pFlags->bSynchronous && !pFlags->bMustReply,
                            "urp-bridge : customized calls currently not supported !");
            }

            if( HDRFLAG_LONGMETHODID & nBitField )
            {
                // methodid as unsigned short
                if( ! m_unmarshal.unpackInt16( &(pFlags->nMethodId )) )
                {
                    return sal_False;
                }
            }
            else
            {
                sal_uInt8 id;
                if( ! m_unmarshal.unpackInt8( &id ) )
                {
                    return sal_False;
                }
                pFlags->nMethodId = (sal_uInt16) id;
            }
        }
        else
        {
            // reply
            pFlags->bRequest = sal_False;
            pFlags->bException = ( HDRFLAG_EXCEPTION & nBitField );
        }
    }
    else
    {
        // short request
        pFlags->bRequest = sal_True;
        if( 0x40 & nBitField )
        {
            sal_uInt8 lower;
            if( ! m_unmarshal.unpackInt8( &lower  ) )
            {
                return sal_False;
            }
            pFlags->nMethodId = ( nBitField & 0x3f ) << 8 | lower;
        }
        else
        {
            pFlags->nMethodId = ( nBitField & 0x3f );
        }
    }
    return sal_True;
}

void OReaderThread::run()
{
    sal_Bool bContinue = sal_True;

    // This vars are needed to hold oid,tid and type information, which should not be cached.
    Type lastTypeNoCache;
    OUString lastOidNoCache;
    ByteSequence lastTidNoCache;

    while( bContinue )
    {
        sal_Int32 nMessageCount;
        if( ! readBlock( &nMessageCount ) )
        {
            disposeEnvironment();
            break;
        }

        ServerMultiJob *pMultiJob = 0;
        remote_Interface *pLastRemoteI = 0;
        while( ! m_unmarshal.finished()  )
        {
#ifdef BRIDGES_URP_PROT
            sal_uInt32 nLogStart = m_unmarshal.getPos();
            sal_Bool bIsOneWay = sal_False;
            OUString sMemberName;
#endif
            MessageFlags flags;

            if( ! readFlags( &flags ) )
            {
                OSL_ENSURE ( 0 , "urp-bridge : incomplete message, skipping block" );
                break;
            }

            // use these ** to access the ids fast ( avoid acquire/release calls )
            sal_Sequence **ppLastTid = flags.bIgnoreCache ?
                (sal_Sequence **) &lastTidNoCache :
                (sal_Sequence **) &(m_pBridgeImpl->m_lastInTid);
            rtl_uString **ppLastOid = flags.bIgnoreCache ?
                (rtl_uString ** ) &lastOidNoCache :
                (rtl_uString ** ) &(m_pBridgeImpl->m_lastInOid);
            typelib_TypeDescriptionReference **ppLastType =
                flags.bIgnoreCache ?
                (typelib_TypeDescriptionReference ** ) &lastTidNoCache :
                (typelib_TypeDescriptionReference ** ) &(m_pBridgeImpl->m_lastInType);

            // get new type
            if( flags.bType )
            {
                typelib_TypeDescriptionReference *pTypeRef = 0;
                if( m_unmarshal.unpackType( &pTypeRef ) )
                {
                    // release the old type
                    typelib_typedescriptionreference_release( *ppLastType );
                    // set the new type
                    *ppLastType = pTypeRef;

                    // no release on pTypeRef necessary (will be released by type dtor)
                }
                else
                {
                    typelib_typedescriptionreference_release( pTypeRef );
                    OSL_ENSURE( 0 , "urp-bridge : error during unpacking interface type, terminating connection" );
                    disposeEnvironment();
                    return;
                }
                if( m_pBridgeImpl->m_lastInType.getTypeClass() != typelib_TypeClass_INTERFACE )
                {
                    OSL_ENSURE( 0 , "urp-bridge : not an interface type" );
                    disposeEnvironment();
                    return;
                }
            }
            if( flags.bOid )
            {
                rtl_uString *pOid = 0;
                if( m_unmarshal.unpackOid( &pOid ) )
                {
                    rtl_uString_release( *ppLastOid );
                    *ppLastOid = pOid;
                }
                else
                {
                    rtl_uString_release( pOid );
                    OSL_ENSURE( 0 , "urp-bridge : error during unpacking cached data, terminating connection" );
                    disposeEnvironment();
                    return;
                }
            }

            if( flags.bTid )
            {
                sal_Sequence *pSeq = 0;
                if( m_unmarshal.unpackTid( &pSeq ) )
                {
                    rtl_byte_sequence_release( *ppLastTid );
                    *ppLastTid = pSeq;
                }
                else
                {
                    rtl_byte_sequence_release( pSeq );

                    OSL_ENSURE( 0 , "urp-bridge : error during unpacking cached data, terminating connection" );
                    disposeEnvironment();
                    return;
                }
            }

            // do the job
            if( flags.bRequest )
            {
                //--------------------------
                // handle request
                //--------------------------
                // get the membertypedescription
                typelib_InterfaceMethodTypeDescription *pMethodType = 0;
                typelib_InterfaceAttributeTypeDescription *pAttributeType = 0;
                sal_Bool bIsSetter = sal_False;

                if( getMemberTypeDescription(
                    &pAttributeType, &pMethodType, &bIsSetter,
                    flags.nMethodId, *ppLastType ) )
                {
                    if( ! pLastRemoteI || flags.bOid || flags.bType )
                    {
                        // a new interface must be retrieved

                        // retrieve the interface NOW from the environment
                        // (avoid race conditions : oneway followed by release )
                        typelib_InterfaceTypeDescription *pInterfaceType = 0;

                        TYPELIB_DANGER_GET(
                            (typelib_TypeDescription ** ) &pInterfaceType ,
                            *ppLastType );
                        if( !pInterfaceType )
                        {
                            delete pMultiJob;
                            pMultiJob = 0;
                            disposeEnvironment();
                            pLastRemoteI = 0; // stubs are released during dispose eitherway
                            bContinue = sal_False;
                            break;
                        }
                        m_pEnvRemote->pExtEnv->getRegisteredInterface(
                            m_pEnvRemote->pExtEnv, ( void **  ) &pLastRemoteI,
                            *ppLastOid, pInterfaceType );
                        TYPELIB_DANGER_RELEASE( (typelib_TypeDescription * )pInterfaceType );

                        if( !pLastRemoteI &&
                            REMOTE_RELEASE_METHOD_INDEX != flags.nMethodId &&
                            OUString::createFromAscii( g_NameOfUrpProtocolPropertiesObject ).equals( *ppLastOid ) )
                        {
                            // check for bridge internal propertyobject
                            pLastRemoteI = m_pBridgeImpl->m_pPropertyObject;
                            pLastRemoteI->acquire( pLastRemoteI );
                            flags.bBridgePropertyCall = sal_True;
                        }

                        // NOTE : Instance provider is called in the executing thread
                        //        Otherwise, instance provider may block the bridge
                    }

                    if( pMultiJob && ! flags.bTid && pMethodType && pMethodType->bOneWay && ! pMultiJob->isFull())
                    {
                        // add to the existing multijob, nothing to do here
                    }
                    else
                    {
                        // create a new multijob
                        if( pMultiJob )
                        {
                            // there exists an old one, start it first.
                            pMultiJob->initiate();
                        }

                        pMultiJob = new ServerMultiJob(
                            m_pEnvRemote, *ppLastTid,
                            m_pBridgeImpl, &m_unmarshal , nMessageCount );
                    }

                    pMultiJob->setIgnoreCache( flags.bIgnoreCache );
                    pMultiJob->setType( *ppLastType );
                    if( pMethodType )
                        pMultiJob->setMethodType( pMethodType , REMOTE_RELEASE_METHOD_INDEX == flags.nMethodId);
                    else if( pAttributeType )
                        pMultiJob->setAttributeType( pAttributeType, bIsSetter  );

                    if( pLastRemoteI )
                        pMultiJob->setInterface( pLastRemoteI );
                    else
                        pMultiJob->setOid( *ppLastOid );
                }
                else
                {
                    delete pMultiJob;
                    pMultiJob = 0;
                    pLastRemoteI = 0; // stubs are released during dispose eitherway
                    disposeEnvironment();
                    bContinue = sal_False;
                    break;
                }
#ifdef BRIDGES_URP_PROT
                bIsOneWay = pMethodType && pMethodType->bOneWay;
                sMemberName = pMethodType ?
                    pMethodType->aBase.pMemberName :
                    pAttributeType->aBase.pMemberName;
                sal_uInt32 nLogHeader = m_unmarshal.getPos();
#endif
                if( ! pMultiJob->extract(  ) )
                {
                    // severe error during extracting, dispose
                    delete pMultiJob;
                    pMultiJob = 0;
                    pLastRemoteI = 0; // stubs are released during dispose eitherway
                    disposeEnvironment();
                    bContinue = sal_False;
                    break;
                }

#ifdef BRIDGES_URP_PROT
                urp_logServingRequest(
                    m_pBridgeImpl, m_unmarshal.getPos() - nLogStart,
                    m_unmarshal.getPos() - nLogHeader,
                    !bIsOneWay,
                    sMemberName );
#endif
                if ( flags.bBridgePropertyCall )
                {
                    // call to the bridge internal object.
                    // these calls MUST be executed within the dispatcher thread in order
                    // to synchronize properly with protocol changes
                    // NOTE : Threadid is not preserved for this call.

                    // lock the marshaling  NOW !
                    {
                        MutexGuard guard( m_pBridgeImpl->m_marshalingMutex );

                        pMultiJob->execute();

                        if( m_pBridgeImpl->m_pPropertyObject->changesHaveBeenCommited() )
                        {
                            Properties props;
                            props = m_pBridgeImpl->m_pPropertyObject->getCommitedChanges();

                            // This call modified the protocol, apply the changes NOW !
                            m_pBridgeImpl->applyProtocolChanges( props );
                        }
                    }
                    delete pMultiJob;
                    pMultiJob = 0;
                }
            }
            else
            {
                //--------------------------
                // handle reply
                //--------------------------
                if( pMultiJob )
                {
                    pMultiJob->initiate();
                    pMultiJob = 0;
                }
                if( pLastRemoteI )
                {
                    pLastRemoteI->release( pLastRemoteI );
                    pLastRemoteI = 0;
                }
                ClientJob *pClientJob =
                    m_pBridgeImpl->m_clientJobContainer.remove( *( ByteSequence * )ppLastTid );
                OSL_ASSERT( pClientJob );
                pClientJob->m_bExceptionOccured = flags.bException;

                pClientJob->setUnmarshal( &m_unmarshal );
#ifdef BRIDGES_URP_PROT
                sMemberName = pClientJob->m_pMethodType ?
                    pClientJob->m_pMethodType->aBase.pMemberName :
                    pClientJob->m_pAttributeType->aBase.pMemberName;
                sal_uInt32 nLogHeader = m_unmarshal.getPos();
#endif
                if( ! pClientJob->extract(  ) )
                {
                    // severe error during extracting, dispose
                    pLastRemoteI = 0; // stubs are released during dispose eitherway
                    disposeEnvironment();
                    bContinue = sal_False;
                    break;
                }
#ifdef BRIDGES_URP_PROT
                urp_logGettingReply(
                    m_pBridgeImpl, m_unmarshal.getPos() - nLogStart,
                    m_unmarshal.getPos() - nLogHeader, sMemberName );
#endif
                sal_Bool bBridgePropertyCallAndWaitingForReply =
                    m_pBridgeImpl->m_pPropertyObject->waitingForCommitChangeReply() &&
                    pClientJob->isBridgePropertyCall();

                pClientJob->initiate();

                if( bBridgePropertyCallAndWaitingForReply )
                {
                    // NOTE : This must be the reply for commit change. The new properties
                    //        are now applied by the clientJob thread, but the reader thread
                    //        must wait for it, because the next message on the wire already
                    //        uses the new protocol settings.
                    // waiting for the commit change reply
                    m_pBridgeImpl->m_pPropertyObject->waitUntilChangesAreCommitted();
                }
            }
        }  // end while( !m_unmarshal.finished() )

        if( pLastRemoteI )
            pLastRemoteI->release( pLastRemoteI );

        if( pMultiJob )
        {
            pMultiJob->initiate();
        }
    }

    if( m_pConnection )
    {
        m_pConnection->release( m_pConnection );
        m_pConnection = 0;
    }
}
}
