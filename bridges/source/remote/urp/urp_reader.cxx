/*************************************************************************
 *
 *  $RCSfile: urp_reader.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:29:09 $
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
#include <rtl/ustrbuf.hxx>

#include <bridges/remote/connection.h>
#include <bridges/remote/counter.hxx>
#include <bridges/remote/context.h>
#include <bridges/remote/helper.hxx>

#include <uno/environment.h>

#include "urp_reader.hxx"
#include "urp_writer.hxx"
#include "urp_dispatch.hxx"
#include "urp_job.hxx"
#include "urp_bridgeimpl.hxx"
#include "urp_log.hxx"
#include "urp_propertyobject.hxx"

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

#if OSL_DEBUG_LEVEL > 1
static MyCounter thisCounter( "DEBUG : ReaderThread" );
#endif

namespace bridges_urp
{

    /**
     * This callback is used to ensure, that the release call is sent for the correct type.
     *
     ***/
    void SAL_CALL urp_releaseRemoteCallback (
        remote_Interface *pRemoteI,rtl_uString *pOid,
        typelib_TypeDescriptionReference *pTypeRef,
        uno_Environment *pEnvRemote )
    {
        remote_Context *pContext = (remote_Context *) pEnvRemote->pContext;
        urp_BridgeImpl *pImpl = (urp_BridgeImpl*) ( pContext->m_pBridgeImpl );

        pImpl->m_pWriter->insertReleaseRemoteCall( pOid , pTypeRef );
    }


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

inline sal_Bool OReaderThread::getMemberTypeDescription(
    typelib_InterfaceAttributeTypeDescription **ppAttributeType,
    typelib_InterfaceMethodTypeDescription **ppMethodType,
    sal_Bool *pbIsSetter,
    sal_uInt16 nMethodId ,
    typelib_TypeDescriptionReference * pITypeRef )
{
    if( pITypeRef->eTypeClass != typelib_TypeClass_INTERFACE )
    {
        OUStringBuffer sMessage;
        sMessage.appendAscii( "interface type is not of typeclass interface (" );
        sMessage.append( (sal_Int32) pITypeRef->eTypeClass );
        m_pBridgeImpl->addError( sMessage.makeStringAndClear() );
        OSL_ENSURE( 0 , "type is not an interface" );
        return sal_False;
    }

    typelib_InterfaceTypeDescription *pInterfaceType = 0;
    TYPELIB_DANGER_GET(
        (typelib_TypeDescription **)&pInterfaceType , pITypeRef );
    if( ! pInterfaceType )
    {
        OUStringBuffer sMessage;
        sMessage.appendAscii( "No typedescription can be retrieved for type " );
        sMessage.append( OUString( pITypeRef->pTypeName ) );
        m_pBridgeImpl->addError( sMessage.makeStringAndClear() );
        OSL_ENSURE( 0 , "urp: unknown type " );
        return sal_False;
    }

    if( ! pInterfaceType->aBase.bComplete )
    {
        typelib_typedescription_complete( (typelib_TypeDescription **) &pInterfaceType );
    }

    if ( nMethodId >= pInterfaceType->nMapFunctionIndexToMemberIndex )
    {
        OUStringBuffer sMessage;
        sMessage.appendAscii( "vtable out of range for type " );
        sMessage.append( OUString( pITypeRef->pTypeName ) );
        sMessage.appendAscii( " (" );
        sMessage.append( (sal_Int32) nMethodId );
        sMessage.appendAscii( " )" );
        m_pBridgeImpl->addError( sMessage.makeStringAndClear() );

        OSL_ENSURE( 0 , "vtable index out of range" );
        return sal_False;
    }

    sal_Int32 nMemberIndex = pInterfaceType->pMapFunctionIndexToMemberIndex[ nMethodId ];

    if( !( pInterfaceType->nAllMembers > nMemberIndex && nMemberIndex >= 0 ) )
    {
        OUStringBuffer sMessage;
        sMessage.appendAscii( "vtable out of range for type " );
        sMessage.append( OUString( pITypeRef->pTypeName ) );
        sMessage.appendAscii( " (" );
        sMessage.append( (sal_Int32) nMethodId );
        sMessage.appendAscii( " )" );
        m_pBridgeImpl->addError( sMessage.makeStringAndClear() );

        OSL_ENSURE( 0 , "vtable index out of range" );
        return sal_False;
    }

    typelib_InterfaceMemberTypeDescription *pMemberType = 0;
    typelib_typedescriptionreference_getDescription(
        (typelib_TypeDescription **) &pMemberType,pInterfaceType->ppAllMembers[nMemberIndex]);

    if(! pMemberType )
    {
        OUStringBuffer sMessage;
        sMessage.appendAscii( "unknown method type description for type" );
        sMessage.append( OUString( pITypeRef->pTypeName ) );
        sMessage.appendAscii( " (" );
        sMessage.append( (sal_Int32) nMethodId );
        sMessage.appendAscii( " )" );
        m_pBridgeImpl->addError( sMessage.makeStringAndClear() );

        OSL_ENSURE( 0 , "unknown method type description" );
        return sal_False;
    }

    if( typelib_TypeClass_INTERFACE_ATTRIBUTE == pMemberType->aBase.eTypeClass )
    {
        // Note: pMapMemberIndexToFunctionIndex always contains the function
        // index of the attribute getter! setter function index is getter index
        // + 1.
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
    m_bContinue( sal_True ),
    m_pBridgeImpl((struct urp_BridgeImpl*)
                  ((remote_Context *)pEnvRemote->pContext)->m_pBridgeImpl ),
    m_unmarshal( m_pBridgeImpl, m_pEnvRemote, ::bridges_remote::remote_createStub )
{
    m_pEnvRemote->acquireWeak( m_pEnvRemote );
    m_pConnection->acquire( m_pConnection );
#if OSL_DEBUG_LEVEL > 1
    thisCounter.acquire();
#endif
}


OReaderThread::~OReaderThread( )
{
    m_pEnvRemote->releaseWeak( m_pEnvRemote );
#if OSL_DEBUG_LEVEL > 1
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
    m_bContinue = sal_False;
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
    m_bContinue = sal_False;
    if( ! pContext->m_pBridgeImpl->m_bDisposed )
    {
        uno_Environment *pEnvRemote = 0;
        m_pEnvRemote->harden( &pEnvRemote , m_pEnvRemote );
        if( pEnvRemote )
        {
            pEnvRemote->dispose( m_pEnvRemote );
            pEnvRemote->release( m_pEnvRemote );
        }
        else
        {
            // environment has been disposed eitherway !
        }
    }
}

inline sal_Bool OReaderThread::readBlock( sal_Int32 *pnMessageCount )
{
    m_unmarshal.setSize( 8 );
    if( 8 != m_pConnection->read( m_pConnection , m_unmarshal.getBuffer(), 8 ) )
    {
        OUString s( RTL_CONSTASCII_USTRINGPARAM( "Unexpected connection closure" ) );
        m_pBridgeImpl->addError( s );
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
        OUStringBuffer s;
        s.appendAscii( "Packet-size too big (" );
        s.append( (sal_Int64) (sal_uInt32 ) nSize );
        s.append( sal_Unicode( ')' ) );
        m_pBridgeImpl->addError( s.makeStringAndClear() );
        OSL_ENSURE( 0 , "urp bridge: Packet-size too big" );
        disposeEnvironment();
        return sal_False;
    }

    if( 0 == nSize )
    {
        // normal termination !
        return sal_False;
    }

    // allocate the necessary memory
    if( ! m_unmarshal.setSize( nSize ) )
    {
        OUStringBuffer s;
        s.appendAscii( "Packet-size too big, couln't allocate necessary memory (" );
        s.append( (sal_Int64) (sal_uInt32 ) nSize );
        s.append( sal_Unicode( ')' ) );
        m_pBridgeImpl->addError( s.makeStringAndClear() );
        OSL_ENSURE( 0 , "urp bridge: messages size too large, terminating connection" );
        return sal_False;
    }

    sal_Int32 nRead = m_pConnection->read( m_pConnection , m_unmarshal.getBuffer() , nSize );

    if( nSize != nRead )
    {
        OUStringBuffer s;
        s.appendAscii( "Unexpected connection closure, inconsistent packet (" );
        s.append( (sal_Int64) (sal_uInt32 ) nSize );
        s.appendAscii( " asked, " );
        s.append( (sal_Int64) (sal_uInt32 ) nRead );
        s.appendAscii( " got )" );
        m_pBridgeImpl->addError( s.makeStringAndClear() );
        // couldn't get the asked amount of bytes, quit
        // should only occur, when the environment has already been disposed
        OSL_ENSURE( m_pBridgeImpl->m_bDisposed , "urp bridge: inconsistent packet, terminating connection." );
        return sal_False;
    }
    return sal_True;
}

inline sal_Bool OReaderThread::readFlags( struct MessageFlags *pFlags )
{
    sal_uInt8 nBitField;
    if( ! m_unmarshal.unpackInt8( &nBitField ) )
    {
        m_pBridgeImpl->addError( "Unexpected end of message header (1)" );
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
                    m_pBridgeImpl->addError( "Unexpected end of message header (2)" );
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
                    m_pBridgeImpl->addError( "Unexpected end of message header (3)" );
                    return sal_False;
                }
            }
            else
            {
                sal_uInt8 id;
                if( ! m_unmarshal.unpackInt8( &id ) )
                {
                    m_pBridgeImpl->addError( "Unexpected end of message header (4)" );
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
                m_pBridgeImpl->addError( "Unexpected end of message header (5)" );
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
    // This vars are needed to hold oid,tid and type information, which should not be cached.
    Type lastTypeNoCache;
    OUString lastOidNoCache;
    ByteSequence lastTidNoCache;

    while( m_bContinue )
    {
        sal_Int32 nMessageCount;
        if( ! readBlock( &nMessageCount ) )
        {
            disposeEnvironment();
            break;
        }

        uno_Environment *pEnvRemote = 0;
           m_pEnvRemote->harden( &pEnvRemote , m_pEnvRemote );
        if( !pEnvRemote )
        {
            // environment has been disposed already, quit here
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
                m_pBridgeImpl->addError( "incomplete message, skipping block" );
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
                    m_pBridgeImpl->addError( "error during unpacking (maybe cached) interface type" );
                    OSL_ENSURE( 0 , "urp-bridge : error during unpacking interface type, terminating connection" );
                    disposeEnvironment();
                    break;
                }
                if( m_pBridgeImpl->m_lastInType.getTypeClass() != TypeClass_INTERFACE )
                {
                    OUStringBuffer sMessage;
                    sMessage.appendAscii( "interface type is not of typeclass interface (" );
                    sMessage.append( (sal_Int32) m_pBridgeImpl->m_lastInType.getTypeClass() );
                    m_pBridgeImpl->addError( sMessage.makeStringAndClear() );
                    OSL_ENSURE( 0 , "urp-bridge : not an interface type" );
                    disposeEnvironment();
                    break;
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
                    m_pBridgeImpl->addError( "error during unpacking (maybe cached) oid" );
                    OSL_ENSURE( 0 , "urp-bridge : error during unpacking cached data, terminating connection" );
                    disposeEnvironment();
                    break;
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

                    m_pBridgeImpl->addError( "error during unpacking (maybe cached) tid" );
                    OSL_ENSURE( 0 , "urp-bridge : error during unpacking cached data, terminating connection" );
                    disposeEnvironment();
                    break;
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
                            OUStringBuffer sMessage;
                            sMessage.appendAscii( "Couldn't retrieve type description for type " );
                            sMessage.append( OUString( (*ppLastType)->pTypeName ) );
                            m_pBridgeImpl->addError( sMessage.makeStringAndClear() );
                            delete pMultiJob;
                            pMultiJob = 0;
                            disposeEnvironment();
                            pLastRemoteI = 0; // stubs are released during dispose eitherway
                            break;
                        }
                        pEnvRemote->pExtEnv->getRegisteredInterface(
                            pEnvRemote->pExtEnv, ( void **  ) &pLastRemoteI,
                            *ppLastOid, pInterfaceType );
                        TYPELIB_DANGER_RELEASE( (typelib_TypeDescription * )pInterfaceType );

                        if( !pLastRemoteI &&
                            REMOTE_RELEASE_METHOD_INDEX != flags.nMethodId &&
                            0 == rtl_ustr_ascii_compare_WithLength(
                                (*ppLastOid)->buffer, (*ppLastOid)->length, g_NameOfUrpProtocolPropertiesObject ) )
                        {
                            // check for bridge internal propertyobject
                            pLastRemoteI = m_pBridgeImpl->m_pPropertyObject;
                            pLastRemoteI->acquire( pLastRemoteI );
                            flags.bBridgePropertyCall = sal_True;
                        }

                        // NOTE : Instance provider is called in the executing thread
                        //        Otherwise, instance provider may block the bridge
                    }

                    sal_Bool bCallIsOneway = sal_False;
                    if( flags.bMoreFlags )
                    {
                        // flags override the default !
                        bCallIsOneway = ! flags.bSynchronous;
                    }
                    else if( pMethodType && pMethodType->bOneWay )
                    {
                        bCallIsOneway = sal_True;
                    }

                    if( pMultiJob && ! flags.bTid && bCallIsOneway && ! pMultiJob->isFull())
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
                            pEnvRemote, *ppLastTid,
                            m_pBridgeImpl, &m_unmarshal , nMessageCount );
                    }

                    pMultiJob->setIgnoreCache( flags.bIgnoreCache );
                    pMultiJob->setType( *ppLastType );
                    if( pMethodType )
                    {
                        pMultiJob->setMethodType( pMethodType ,
                                                  REMOTE_RELEASE_METHOD_INDEX == flags.nMethodId,
                                                  bCallIsOneway );
                    }
                    else if( pAttributeType )
                    {
                        pMultiJob->setAttributeType( pAttributeType, bIsSetter, bCallIsOneway );
                    }
                    else
                    {
                        OSL_ASSERT( 0 );
                    }

                    if( pLastRemoteI )
                        pMultiJob->setInterface( pLastRemoteI );
                    else
                        pMultiJob->setOid( *ppLastOid );
                } /* getMemberTypeDescription */
                else
                {
                    delete pMultiJob;
                    pMultiJob = 0;
                    pLastRemoteI = 0; // stubs are released during dispose eitherway
                    disposeEnvironment();
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

                // Bridge MUST be already disposed, otherwise we got a wrong threadid
                // from remote !
                OSL_ASSERT( pClientJob || m_pBridgeImpl->m_bDisposed );
                if( ! pClientJob )
                {
                    OUStringBuffer error( 128 );
                    error.appendAscii( "ThreadID " );
                    OString o = byteSequence2HumanReadableString( *(ByteSequence* )ppLastTid );
                    error.appendAscii( o.getStr(), o.getLength() );
                    error.appendAscii( " unknown, so couldn't unmarshal reply" );
                    m_pBridgeImpl->addError( error.makeStringAndClear() );
                    pLastRemoteI = 0;
                    disposeEnvironment();
                    break;
                }

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
            pMultiJob->initiate();

           if( pEnvRemote )
               pEnvRemote->release( pEnvRemote );
    }

    if( m_pConnection )
    {
        m_pConnection->release( m_pConnection );
        m_pConnection = 0;
    }
}
}
