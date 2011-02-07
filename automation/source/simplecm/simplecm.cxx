/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_automation.hxx"


#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <tools/solar.h>
#include <automation/simplecm.hxx>

#include <automation/commdefines.hxx>
#include "packethandler.hxx"
#include "tcpio.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
void debug_printf( const char *chars )
{
    static BOOL bPrint = (getenv("DEBUG") != NULL);
    if ( bPrint )
    {
        printf( "%c\n", chars );
        fflush( stdout );
    }
}
#endif

CommunicationLink::CommunicationLink( CommunicationManager *pMan )
: pMyManager(pMan)
, pServiceData(NULL)
, nServiceProtocol( 0 )
, bIsInsideCallback( FALSE )
, nTotalBytes( 0 )
, maApplication("Undefined")
#if OSL_DEBUG_LEVEL > 1
, bFlag( FALSE )
, nSomething( 0 )
#endif
{
}

CommunicationLink::~CommunicationLink()
{
#if OSL_DEBUG_LEVEL > 1
    if ( !bFlag )    // bFlag will be set if deletion is expected else we can set a breakpoint
        bFlag = FALSE;
#endif
    if ( pMyManager )
        pMyManager->DestroyingLink( this );
}

void CommunicationLink::CallInfoMsg( InfoString aMsg )
{
    if ( pMyManager )
        pMyManager->InfoMsg( aMsg );
};

CM_InfoType CommunicationLink::GetInfoType()
{
    if ( pMyManager )
        return pMyManager->GetInfoType();
    else
        return CM_NO_TEXT;
}

IMPL_LINK( CommunicationLink, ConnectionClosed, void*, EMPTYARG )
{
    if ( pMyManager )
        pMyManager->CallConnectionClosed( this );
    return 1;
}

IMPL_LINK( CommunicationLink, DataReceived, void*, EMPTYARG )
{
    if ( pMyManager )
        pMyManager->CallDataReceived( this );
    return 1;
}

BOOL CommunicationLink::DoTransferDataStream( SvStream *pDataStream, CMProtocol nProtocol )
{
    INFO_MSG( CByteString("S :").Append( GetCommunicationPartner( CM_FQDN ) ),
        CByteString("Daten Senden:").Append( GetCommunicationPartner( CM_FQDN ) ),
        CM_SEND, this );
    BOOL bWasError = FALSE;

    UINT32 nBuffer;
    nBuffer = pDataStream->SeekRel(0) +1;
    bWasError = pPacketHandler->TransferData( ((SvMemoryStream*)pDataStream)->GetData(), nBuffer, nProtocol ) != C_ERROR_NONE;

    if ( bWasError )
    {
        INFO_MSG( CByteString("Send Failed:").Append( GetCommunicationPartner( CM_FQDN ) ),
            CByteString( "Socket wird wegen Fehlers beim Senden geschlossen: ").Append( GetCommunicationPartner( CM_FQDN ) ),
            CM_ERROR, this );
        ShutdownCommunication();
    }
    return !bWasError;
}

BOOL CommunicationLink::TransferDataStream( SvStream *pDataStream, CMProtocol nProtocol )
{
    aLastAccess = DateTime();
    nTotalBytes += pDataStream->Seek( STREAM_SEEK_TO_END );
    return DoTransferDataStream( pDataStream, nProtocol );
}

void CommunicationLink::SetApplication( const ByteString& aApp )
{
    maApplication = aApp;
}


SimpleCommunicationLinkViaSocket::SimpleCommunicationLinkViaSocket( CommunicationManager *pMan, osl::StreamSocket* pSocket )
: CommunicationLink( pMan )
, aCommunicationPartner()
, aMyName()
, pStreamSocket( pSocket )
, pReceiveStream( NULL )
, bIsRequestShutdownPending( FALSE )
{
    pTCPIO = new TCPIO( pStreamSocket );
    pPacketHandler = new PacketHandler( (ITransmiter*) pTCPIO, pTCPIO, pMyManager->IsMultiChannel() );
}

SimpleCommunicationLinkViaSocket::~SimpleCommunicationLinkViaSocket()
{
    delete pPacketHandler;
    pPacketHandler = NULL;
    delete pTCPIO;
    pTCPIO = NULL;
    delete pStreamSocket;
    pStreamSocket = NULL;
}

void SimpleCommunicationLinkViaSocket::SetStreamSocket( osl::StreamSocket* pSocket )
{
    if ( pTCPIO )
        pTCPIO->SetStreamSocket( pSocket );
    pStreamSocket = pSocket;
}

BOOL SimpleCommunicationLinkViaSocket::StopCommunication()
{
    CommunicationLinkRef rHold(this);       // avoid deleting this link before the end of the method
    if ( !IsCommunicationError() )  // Meaning that the Communication is still runnung
    {
#if OSL_DEBUG_LEVEL > 1
        debug_printf("Sending REQUEST_ShutdownLink\n");
#endif
        SendHandshake( CH_REQUEST_ShutdownLink );
    }
    WaitForShutdown();
    return TRUE;
}

BOOL SimpleCommunicationLinkViaSocket::IsCommunicationError()
{
    return !pStreamSocket;
}

ByteString SimpleCommunicationLinkViaSocket::GetCommunicationPartner( CM_NameType eType )
{
    if ( pStreamSocket )
    {
        switch ( eType )
        {
            case CM_DOTTED:
                {
                    rtl::OUString aDotted;
                    osl::SocketAddr* pPeerAdr = new osl::SocketAddr;
                    pStreamSocket->getPeerAddr( *pPeerAdr );
                    osl_getDottedInetAddrOfSocketAddr( pPeerAdr->getHandle(), &aDotted.pData);
                    delete pPeerAdr;
                    return ByteString( UniString(aDotted), RTL_TEXTENCODING_UTF8 );
                }
                //break;
            case CM_FQDN:
                {
                    if ( !aCommunicationPartner.Len() )
                    {
                        rtl::OUString aFQDN( pStreamSocket->getPeerHost());
                        aCommunicationPartner = ByteString( UniString(aFQDN), RTL_TEXTENCODING_UTF8 );
                    }
                    return aCommunicationPartner;
                }
                //break;
        }
    }
    return CByteString( "Unknown" );
}

ByteString SimpleCommunicationLinkViaSocket::GetMyName( CM_NameType eType )
{
    if ( pStreamSocket )
    {
        switch ( eType )
        {
            case CM_DOTTED:
                {
                    rtl::OUString aDotted;
                    osl::SocketAddr* pPeerAdr = new osl::SocketAddr;
                    pStreamSocket->getPeerAddr( *pPeerAdr );
                    osl_getDottedInetAddrOfSocketAddr( pPeerAdr->getHandle(), &aDotted.pData);
                    delete pPeerAdr;
                    return ByteString( UniString(aDotted), RTL_TEXTENCODING_UTF8 );
                }
                //break;
            case CM_FQDN:
                {
                    if ( !aMyName.Len() )
                    {
                        rtl::OUString aFQDN(pStreamSocket->getLocalHost());
                        aMyName = ByteString( UniString(aFQDN), RTL_TEXTENCODING_UTF8 );
                    }
                    return aMyName;
                }
                //break;
        }
    }
    return CByteString( "Error" );
}

SvStream* SimpleCommunicationLinkViaSocket::GetBestCommunicationStream()
{
    SvStream* pStream = new SvMemoryStream;
//  pStream->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    return pStream;
}

#define READ_SOCKET( pBuffer, nLength )\
    if ( !bWasError )\
        {bWasError |= pTCPIO->ReceiveBytes( pBuffer, nLength ) != C_ERROR_NONE;}

#define READ_SOCKET_LEN( pBuffer, nLength, nTotal )\
    READ_SOCKET( pBuffer, nLength );\
    if ( !bWasError )\
        {nTotal += nLength;}

BOOL SimpleCommunicationLinkViaSocket::DoReceiveDataStream()
{
    BOOL bWasError = FALSE;
    void* pBuffer = NULL;
    comm_UINT32 nLen;
    bWasError = pPacketHandler->ReceiveData( pBuffer, nLen ) != C_ERROR_NONE;
    if ( !bWasError )
    {
        pReceiveStream = GetBestCommunicationStream();
        DBG_ASSERT( pReceiveStream->IsA() == ID_MEMORYSTREAM, "CommunicationStream is not an SvMemoryStream. Communication has to be reimplemented here!");
        if ( pReceiveStream->IsA() == ID_MEMORYSTREAM )
            ((SvMemoryStream*)pReceiveStream)->SetBuffer( pBuffer, nLen, TRUE, nLen );
        DBG_ASSERT( pReceiveStream, "Datastream is NULL");
    }

    return !bWasError;
}

void SimpleCommunicationLinkViaSocket::SetApplication( const ByteString& aApp )
{
    CommunicationLink::SetApplication( aApp );
    SvStream* pData = GetBestCommunicationStream();
    *pData << aApp;
    SendHandshake( CH_SetApplication, pData );
    delete pData;
}

void SimpleCommunicationLinkViaSocket::SetNewPacketAsCurrent()
{
    pServiceData = pReceiveStream;
    nServiceProtocol = pPacketHandler->GetReceiveProtocol();
    nServiceHeaderType = pPacketHandler->GetReceiveHeaderType();
}

BOOL SimpleCommunicationLinkViaSocket::SendHandshake( HandshakeType aHandshakeType, SvStream* pData )
{
    BOOL bWasError;

    if ( pData )
    {
        UINT32 nBuffer;
        nBuffer = pData->Seek( STREAM_SEEK_TO_END );
        bWasError = !pPacketHandler->SendHandshake( aHandshakeType, ((SvMemoryStream*)pData)->GetData(), nBuffer );
    }
    else
        bWasError = !pPacketHandler->SendHandshake( aHandshakeType );


    if ( bWasError )
    {
        INFO_MSG( CByteString("Send Failed:").Append( GetCommunicationPartner( CM_FQDN ) ),
            CByteString( "Socket wird wegen Fehlers beim Senden geschlossen: ").Append( GetCommunicationPartner( CM_FQDN ) ),
            CM_ERROR, this );
        ShutdownCommunication();
    }
    else
    {   // set new status
        switch ( aHandshakeType )
        {
            case CH_REQUEST_HandshakeAlive:
                break;
            case CH_RESPONSE_HandshakeAlive:
                break;
            case CH_REQUEST_ShutdownLink:
                bIsRequestShutdownPending = TRUE;
                break;
            case CH_ShutdownLink:
                break;
            case CH_SUPPORT_OPTIONS:
                break;
            case CH_SetApplication:
                break;
            default:
                DBG_ERROR("Unknown HandshakeType");
        }
    }
    return !bWasError;
}

SimpleCommunicationLinkViaSocketWithReceiveCallbacks::SimpleCommunicationLinkViaSocketWithReceiveCallbacks( CommunicationManager *pMan, osl::StreamSocket* pSocket )
: SimpleCommunicationLinkViaSocket( pMan, pSocket )
{
}

SimpleCommunicationLinkViaSocketWithReceiveCallbacks::~SimpleCommunicationLinkViaSocketWithReceiveCallbacks()
{
    if ( pMyManager && pMyManager->IsLinkValid( this ) && !bIsRequestShutdownPending )
        StopCommunication();
}

bool SimpleCommunicationLinkViaSocket::IsReceiveReady()
{
    if ( !IsCommunicationError() )
    {
        TimeValue aTime = {30, 0};   // 30 seconds
        return pStreamSocket->isRecvReady( &aTime );
    }

    return false;
}

void SimpleCommunicationLinkViaSocketWithReceiveCallbacks::WaitForShutdown()
{
    CommunicationLinkRef rHold(this);       // avoid deleting this link before the end of the method

    while( pMyManager && !IsCommunicationError() && IsReceiveReady())
        ReceiveDataStream();
}

BOOL SimpleCommunicationLinkViaSocketWithReceiveCallbacks::ReceiveDataStream()
{
    if ( DoReceiveDataStream() )
    {
        SetNewPacketAsCurrent();
        StartCallback();
        DataReceived();
        return TRUE;
    }
    else
    {
        StartCallback();
        ShutdownCommunication();
        return FALSE;
    }
}

BOOL SimpleCommunicationLinkViaSocketWithReceiveCallbacks::ShutdownCommunication()
{
    if ( GetStreamSocket() )
        GetStreamSocket()->shutdown();

    if ( GetStreamSocket() )
        GetStreamSocket()->close();

    osl::StreamSocket* pTempSocket = GetStreamSocket();
    SetStreamSocket( NULL );
    delete pTempSocket;

    ConnectionClosed();

    return TRUE;
}



CommunicationManager::CommunicationManager( BOOL bUseMultiChannel )
: nInfoType( CM_NONE )
, bIsCommunicationRunning( FALSE )
, maApplication("Unknown")
, bIsMultiChannel( bUseMultiChannel )
{
}

CommunicationManager::~CommunicationManager()
{
    xLastNewLink.Clear();
}

BOOL CommunicationManager::StartCommunication( String aApp, String aParams )
{
    (void) aApp; /* avoid warning about unused parameter */
    (void) aParams; /* avoid warning about unused parameter */
    return FALSE;
}

BOOL CommunicationManager::StartCommunication( ByteString aHost, ULONG nPort )
{
    (void) aHost; /* avoid warning about unused parameter */
    (void) nPort; /* avoid warning about unused parameter */
    return FALSE;
}

ByteString CommunicationManager::GetMyName( CM_NameType )
{
    rtl::OUString aHostname( osl::SocketAddr::getLocalHostname());
    return ByteString( UniString(aHostname), RTL_TEXTENCODING_UTF8 );
}

void CommunicationManager::CallConnectionOpened( CommunicationLink* pCL )
{
    pCL->StartCallback();       // Sollte bereits vor dem Aufruf gerufen werden
    pCL->aStart = DateTime();
    pCL->aLastAccess = pCL->aStart;
    bIsCommunicationRunning = TRUE;
    pCL->SetApplication( GetApplication() );

    xLastNewLink = pCL;

    INFO_MSG( CByteString("C+:").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
        CByteString("Verbindung aufgebaut: ").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
        CM_OPEN, pCL );
    ConnectionOpened( pCL );
    pCL->FinishCallback();
}

void CommunicationManager::CallConnectionClosed( CommunicationLink* pCL )
{
    pCL->StartCallback();       // Sollte bereits vor dem Aufruf gerufen werden
    pCL->aLastAccess = DateTime();

    INFO_MSG( CByteString("C-:").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
        CByteString("Verbindung abgebrochen: ").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
        CM_CLOSE, pCL );
    ConnectionClosed( pCL );

    if ( xLastNewLink == pCL )
        xLastNewLink.Clear();

    pCL->FinishCallback();
//  delete pCL;
}

void CommunicationManager::CallDataReceived( CommunicationLink* pCL )
{
    pCL->StartCallback();       // Sollte bereits vor dem Aufruf gerufen werden
    pCL->aLastAccess = DateTime();
    CommunicationLinkRef rHold(pCL);    // Hält den Zeiger bis zum Ende des calls

    // should be impossible but happens for mysterious reasons
    if ( !pCL->pServiceData )
    {
        DBG_ERROR( "Datastream is NULL" );
        pCL->FinishCallback();
        return;
    }


    if ( CH_Handshake == pCL->nServiceHeaderType )
    {
        SvStream *pData = pCL->GetServiceData();
        USHORT nType;
        pData->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );    // Unfortulately it is written this way :((
        *pData >> nType;
        pData->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        switch ( nType )
        {
            case CH_REQUEST_HandshakeAlive:
                {
                    pCL->SendHandshake( CH_RESPONSE_HandshakeAlive );
                }
                break;
            case CH_REQUEST_ShutdownLink:
                {
#if OSL_DEBUG_LEVEL > 1
                    debug_printf("Sending ShutdownLink\n");
#endif
                    pCL->SendHandshake( CH_ShutdownLink );
                }
                break;
            case CH_ShutdownLink:
                {
#if OSL_DEBUG_LEVEL > 1
                    debug_printf("Executing ShutdownLink\n");
#endif
                    pCL->ShutdownCommunication();
                }
                break;
            case CH_SetApplication:
                {
                    ByteString aApplication;
                    *pData >> aApplication;
                    pCL->CommunicationLink::SetApplication( aApplication );
#if OSL_DEBUG_LEVEL > 1
                    debug_printf( "Setting Application to " );
                    debug_printf( aApplication.GetBuffer() );
                    debug_printf( "\n" );
#endif
                }
                break;

#if OSL_DEBUG_LEVEL > 1
            default:
                {
                    debug_printf("Unknown Handshake received\n");
                }
#endif
        }
        delete pData;
    }
    else
    {
        if ( pCL->pServiceData )
        {
            pCL->nTotalBytes += pCL->pServiceData->Seek( STREAM_SEEK_TO_END );
            pCL->pServiceData->Seek( STREAM_SEEK_TO_BEGIN );
        }

        INFO_MSG( CByteString("D :").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
            CByteString("Daten Empfangen:").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
            CM_RECEIVE, pCL );
        DataReceived( pCL );
    }
    delete pCL->GetServiceData();
    pCL->FinishCallback();
}

void CommunicationManager::CallInfoMsg( InfoString aMsg )
{
    // Hier wird es wohl kein Housekeeping geben
    InfoMsg( aMsg );
}

void CommunicationManager::SetApplication( const ByteString& aApp, BOOL bRunningLinks )
{
    maApplication = aApp;
    if ( bRunningLinks )
    {
        USHORT i;
        for ( i = 0 ; i < GetCommunicationLinkCount() ; i++ )
            GetCommunicationLink( i )->SetApplication( aApp );
    }
}



SingleCommunicationManager::SingleCommunicationManager( BOOL bUseMultiChannel )
: CommunicationManager( bUseMultiChannel )
{
    xActiveLink = NULL;
    pInactiveLink = NULL;
}

SingleCommunicationManager::~SingleCommunicationManager()
{
    StopCommunication();
    if ( pInactiveLink )
        pInactiveLink->InvalidateManager();
}

BOOL SingleCommunicationManager::StopCommunication()
{
    if ( xActiveLink.Is() )
    {
        BOOL bSuccess = xActiveLink->StopCommunication();
        if ( pInactiveLink )
            pInactiveLink->InvalidateManager();
        pInactiveLink = xActiveLink;
        xActiveLink.Clear();
        return bSuccess;
    }
    return TRUE;
}

BOOL SingleCommunicationManager::IsLinkValid( CommunicationLink* pCL )
{
    return &xActiveLink == pCL;
}

USHORT SingleCommunicationManager::GetCommunicationLinkCount()
{
    return IsCommunicationRunning()?1:0;
}

CommunicationLinkRef SingleCommunicationManager::GetCommunicationLink( USHORT )
{
    return xActiveLink;
}

void SingleCommunicationManager::CallConnectionOpened( CommunicationLink* pCL )
{
    DBG_ASSERT( !xActiveLink.Is(), "Es ist bereits ein CommunicationLink aktiv");
    if ( xActiveLink.Is() )
    {
        if ( pInactiveLink )
            pInactiveLink->InvalidateManager();
        pInactiveLink = xActiveLink;
        xActiveLink->StopCommunication();   // Den alten Link brutal abwürgen
    }
    xActiveLink = pCL;
    CommunicationManager::CallConnectionOpened( pCL );
}

void SingleCommunicationManager::CallConnectionClosed( CommunicationLink* pCL )
{
    CommunicationManager::CallConnectionClosed( pCL );

    DBG_ASSERT( pCL == xActiveLink, "SingleCommunicationManager::CallConnectionClosed mit fremdem Link");
    if ( pInactiveLink )
        pInactiveLink->InvalidateManager();
    pInactiveLink = xActiveLink;
    xActiveLink.Clear();
    bIsCommunicationRunning = FALSE;
}

void SingleCommunicationManager::DestroyingLink( CommunicationLink *pCL )
{
    pInactiveLink = NULL;
    pCL->InvalidateManager();
}


SingleCommunicationManagerClientViaSocket::SingleCommunicationManagerClientViaSocket( ByteString aHost, ULONG nPort, BOOL bUseMultiChannel )
: SingleCommunicationManager( bUseMultiChannel )
, aHostToTalk( aHost )
, nPortToTalk( nPort )
{
}


SingleCommunicationManagerClientViaSocket::SingleCommunicationManagerClientViaSocket( BOOL bUseMultiChannel )
: SingleCommunicationManager( bUseMultiChannel )
, aHostToTalk()
, nPortToTalk( 0 )
{
}


BOOL CommonSocketFunctions::DoStartCommunication( CommunicationManager *pCM, ICommunicationManagerClient *pCMC, ByteString aHost, ULONG nPort )
{
    osl::SocketAddr Addr( rtl::OUString( UniString( aHost, RTL_TEXTENCODING_UTF8 ) ), nPort );
    osl::ConnectorSocket *pConnSocket;


    TimeValue aTV;
    aTV.Seconds = 10;       // Warte 10 Sekunden
    aTV.Nanosec = 0;
    do
    {
        pConnSocket = new osl::ConnectorSocket();
        pConnSocket->setOption( osl_Socket_OptionTcpNoDelay, 1 );
        if ( pConnSocket->connect( Addr, &aTV ) == osl_Socket_Ok )
        {
            pConnSocket->setOption( osl_Socket_OptionTcpNoDelay, 1 );
            pCM->CallConnectionOpened( CreateCommunicationLink( pCM, pConnSocket ) );
            return TRUE;
        }
        else
            delete pConnSocket;

    } while ( pCMC->RetryConnect() );

    return FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
