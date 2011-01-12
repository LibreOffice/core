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
#include <stdio.h>
#if OSL_DEBUG_LEVEL > 1
#define DEBUGPRINTF(x) { printf(x); fflush( stdout ); }
#else
#define DEBUGPRINTF(x)
#endif
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vos/socket.hxx>
#include <tools/stream.hxx>
#include <vcl/timer.hxx>
#include <tools/fsys.hxx>

#include <automation/communi.hxx>


/*  Um den Destruktor protected zu machen wurde unten das delete entfernt.
    Die Methode wird ohnehin hucht benutzt.
//              delete *((AE*)pData+n);
*/

#undef  SV_IMPL_PTRARR_SORT
#define SV_IMPL_PTRARR_SORT( nm,AE )\
_SV_IMPL_SORTAR_ALG( nm,AE )\
    void nm::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL ) { \
        if( nL ) {\
            DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );\
            for( sal_uInt16 n=nP; n < nP + nL; n++ ) \
                DBG_ERROR("Das Element der Liste wurde nicht gelöscht"); \
            SvPtrarr::Remove( nP, nL ); \
        } \
    } \
_SV_SEEK_PTR( nm, AE )




SV_IMPL_PTRARR_SORT( CommunicationLinkList, CommunicationLink* );

vos::OMutex *pMPostUserEvent=NULL;      // Notwendig, da nicht threadfest

CommunicationLinkViaSocket::CommunicationLinkViaSocket( CommunicationManager *pMan, vos::OStreamSocket *pSocket )
: SimpleCommunicationLinkViaSocket( pMan, pSocket )
, nConnectionClosedEventId( 0 )
, nDataReceivedEventId( 0 )
, bShutdownStarted( sal_False )
, bDestroying( sal_False )
{
    SetPutDataReceivedHdl(LINK( this, CommunicationLinkViaSocket, PutDataReceivedHdl ));
    if ( !pMPostUserEvent )
        pMPostUserEvent = new vos::OMutex;
    // this is necassary to prevent the running thread from sending the close event
    // before the open event has been sent.
       StartCallback();

    create();
}

CommunicationLinkViaSocket::~CommunicationLinkViaSocket()
{
    bDestroying = sal_True;
    StopCommunication();
    while ( nConnectionClosedEventId || nDataReceivedEventId )
        GetpApp()->Yield();
    {
        vos::OGuard aGuard( aMConnectionClosed );
        if ( nConnectionClosedEventId )
        {
            GetpApp()->RemoveUserEvent( nConnectionClosedEventId );
            nConnectionClosedEventId = 0;
            INFO_MSG( CByteString("Event gelöscht"),
                CByteString( "ConnectionClosedEvent aus Queue gelöscht"),
                CM_MISC, NULL );
        }
    }
    {
        vos::OGuard aGuard( aMDataReceived );
        if ( nDataReceivedEventId )
        {
            GetpApp()->RemoveUserEvent( nDataReceivedEventId );
            nDataReceivedEventId = 0;
            delete GetServiceData();
            INFO_MSG( CByteString("Event gelöscht"),
                CByteString( "DataReceivedEvent aus Queue gelöscht"),
                CM_MISC, NULL );
        }
    }
}

sal_Bool CommunicationLinkViaSocket::ShutdownCommunication()
{
    if ( isRunning() )
    {

        terminate();
        if ( GetStreamSocket() )
            GetStreamSocket()->shutdown();

        if ( GetStreamSocket() )    // Mal wieder nach oben verschoben, da sonst nicht vom Read runtergesprungen wird.
            GetStreamSocket()->close();

        resume();   // So daß das run auch die Schleife verlassen kann

        join();

        vos::OStreamSocket *pTempSocket = GetStreamSocket();
        SetStreamSocket( NULL );
        delete pTempSocket;

//      ConnectionClosed();     Wird am Ende des Thread gerufen

    }
    else
    {
        join();
    }

    return sal_True;
}

sal_Bool CommunicationLinkViaSocket::StopCommunication()
{
    if ( !bShutdownStarted )
    {
        return SimpleCommunicationLinkViaSocket::StopCommunication();
    }
    else
    {
        WaitForShutdown();
        return sal_True;
    }
}


IMPL_LINK( CommunicationLinkViaSocket, ShutdownLink, void*, EMPTYARG )
{
    if ( !IsCommunicationError() )
        ShutdownCommunication();
    return 0;
}


void CommunicationLinkViaSocket::WaitForShutdown()
{
    if ( !bShutdownStarted )
    {
        aShutdownTimer.SetTimeout( 30000 );     // Should be 30 Seconds
        aShutdownTimer.SetTimeoutHdl( LINK( this, CommunicationLinkViaSocket, ShutdownLink ) );
        aShutdownTimer.Start();
        bShutdownStarted = sal_True;
    }
    if ( bDestroying )
    {
        while ( pMyManager && aShutdownTimer.IsActive() )
        {
            if ( IsCommunicationError() )
                return;
            GetpApp()->Yield();
        }
        ShutdownCommunication();
    }
}

sal_Bool CommunicationLinkViaSocket::IsCommunicationError()
{
    return !isRunning() || SimpleCommunicationLinkViaSocket::IsCommunicationError();
}

void CommunicationLinkViaSocket::run()
{
    sal_Bool bWasError = sal_False;
    while ( schedule() && !bWasError && GetStreamSocket() )
    {
        if ( bWasError |= !DoReceiveDataStream() )
            continue;

        TimeValue sNochEins = {0, 1000000};
        while ( schedule() && bIsInsideCallback )   // solange der letzte Callback nicht beendet ist
            sleep( sNochEins );
        SetNewPacketAsCurrent();
        StartCallback();
        {
            vos::OGuard aGuard( aMDataReceived );
            vos::OGuard aGuard2( *pMPostUserEvent );
            mlPutDataReceived.Call(this);
        }
    }
    TimeValue sNochEins = {0, 1000000};
    while ( schedule() && bIsInsideCallback )   // solange der letzte Callback nicht beendet ist
        sleep( sNochEins );

    StartCallback();
    {
        vos::OGuard aGuard( aMConnectionClosed );
        vos::OGuard aGuard2( *pMPostUserEvent );
        nConnectionClosedEventId = GetpApp()->PostUserEvent( LINK( this, CommunicationLinkViaSocket, ConnectionClosed ) );
    }
}

sal_Bool CommunicationLinkViaSocket::DoTransferDataStream( SvStream *pDataStream, CMProtocol nProtocol )
{
    if ( !isRunning() )
        return sal_False;

    return SimpleCommunicationLinkViaSocket::DoTransferDataStream( pDataStream, nProtocol );
}

/// Dies ist ein virtueller Link!!!
long CommunicationLinkViaSocket::ConnectionClosed( void* EMPTYARG )
{
    {
        vos::OGuard aGuard( aMConnectionClosed );
        nConnectionClosedEventId = 0;   // Achtung!! alles andere muß oben gemacht werden.
    }
    ShutdownCommunication();
    return CommunicationLink::ConnectionClosed( );
}

/// Dies ist ein virtueller Link!!!
long CommunicationLinkViaSocket::DataReceived( void* EMPTYARG )
{
    {
        vos::OGuard aGuard( aMDataReceived );
        nDataReceivedEventId = 0;   // Achtung!! alles andere muß oben gemacht werden.
    }
    return CommunicationLink::DataReceived( );
}

IMPL_LINK( CommunicationLinkViaSocket, PutDataReceivedHdl, CommunicationLinkViaSocket*, EMPTYARG )
{
    nDataReceivedEventId = GetpApp()->PostUserEvent( LINK( this, CommunicationLink, DataReceived ) );
    return 0;
}



MultiCommunicationManager::MultiCommunicationManager( sal_Bool bUseMultiChannel )
: CommunicationManager( bUseMultiChannel )
, bGracefullShutdown( sal_True )
{
    ActiveLinks = new CommunicationLinkList;
    InactiveLinks = new CommunicationLinkList;
}

MultiCommunicationManager::~MultiCommunicationManager()
{
    StopCommunication();

    if ( bGracefullShutdown )   // first try to collect all callbacks for closing channels
    {
        Timer aTimeout;
        aTimeout.SetTimeout( 40000 );
        aTimeout.Start();
        sal_uInt16 nLinkCount = 0;
        sal_uInt16 nNewLinkCount = 0;
        while ( aTimeout.IsActive() )
        {
            GetpApp()->Yield();
            nNewLinkCount = GetCommunicationLinkCount();
            if ( nNewLinkCount == 0 )
                aTimeout.Stop();
            if ( nNewLinkCount != nLinkCount )
            {
                aTimeout.Start();
                nLinkCount = nNewLinkCount;
            }
        }
    }

    // Alles weghauen, was nicht rechtzeitig auf die Bäume gekommen ist
    // Was bei StopCommunication übrig geblieben ist, da es sich asynchron austragen wollte
    sal_uInt16 i = ActiveLinks->Count();
    while ( i-- )
    {
        CommunicationLinkRef rTempLink = ActiveLinks->GetObject( i );
        ActiveLinks->Remove( i );
        rTempLink->InvalidateManager();
        rTempLink->ReleaseReference();
    }
    delete ActiveLinks;

    /// Die Links zwischen ConnectionClosed und Destruktor.
    /// Hier NICHT gerefcounted, da sie sich sonst im Kreis festhaten würden,
    /// da die Links sich erst in ihrem Destruktor austragen
    i = InactiveLinks->Count();
    while ( i-- )
    {
        CommunicationLinkRef rTempLink = InactiveLinks->GetObject( i );
        InactiveLinks->Remove( i );
        rTempLink->InvalidateManager();
    }
    delete InactiveLinks;
}

sal_Bool MultiCommunicationManager::StopCommunication()
{
    // Alle Verbindungen abbrechen
    // ConnectionClosed entfernt die Links aus der Liste. Je nach Implementation syncron
    // oder asyncron. Daher Von oben nach unten Abräumen, so daß sich nichts verschiebt.
    sal_uInt16 i = ActiveLinks->Count();
    int nFail = 0;
    while ( i )
    {
        if ( !ActiveLinks->GetObject(i-1)->StopCommunication() )
            nFail++;    // Hochzählen, da Verbindung sich nicht (sofort) beenden lässt.
        i--;
    }

    return nFail == 0;
}

sal_Bool MultiCommunicationManager::IsLinkValid( CommunicationLink* pCL )
{
    if ( ActiveLinks->Seek_Entry( pCL ) )
        return sal_True;
    else
        return sal_False;
}

sal_uInt16 MultiCommunicationManager::GetCommunicationLinkCount()
{
    return ActiveLinks->Count();
}

CommunicationLinkRef MultiCommunicationManager::GetCommunicationLink( sal_uInt16 nNr )
{
    return ActiveLinks->GetObject( nNr );
}

void MultiCommunicationManager::CallConnectionOpened( CommunicationLink* pCL )
{
    CommunicationLinkRef rHold(pCL);    // Hält den Zeiger bis zum Ende des calls
    ActiveLinks->C40_PTR_INSERT(CommunicationLink, pCL);
    rHold->AddRef();

    CommunicationManager::CallConnectionOpened( pCL );
}

void MultiCommunicationManager::CallConnectionClosed( CommunicationLink* pCL )
{
    CommunicationLinkRef rHold(pCL);    // Hält denm Zeiger bis zum Ende des calls

    CommunicationManager::CallConnectionClosed( pCL );

    sal_uInt16 nPos;
    if ( ActiveLinks->Seek_Entry( pCL, &nPos ) )
    {
        InactiveLinks->C40_PTR_INSERT(CommunicationLink, pCL);  // Ohne Reference
        ActiveLinks->Remove( nPos );
    }
    pCL->ReleaseReference();

    bIsCommunicationRunning = ActiveLinks->Count() > 0;
//  delete pCL;
#if OSL_DEBUG_LEVEL > 1
        rHold->bFlag = sal_True;
#endif
}

void MultiCommunicationManager::DestroyingLink( CommunicationLink *pCL )
{
    sal_uInt16 nPos;
    if ( InactiveLinks->Seek_Entry( pCL, &nPos ) )
        InactiveLinks->Remove( nPos );
    pCL->InvalidateManager();
}



CommunicationManagerClient::CommunicationManagerClient( sal_Bool bUseMultiChannel )
: MultiCommunicationManager( bUseMultiChannel )
{
    ByteString aApplication("Something inside ");
    aApplication.Append( ByteString( DirEntry( Application::GetAppFileName() ).GetName(), gsl_getSystemTextEncoding() ) );
    SetApplication( aApplication );
}



CommunicationManagerServerViaSocket::CommunicationManagerServerViaSocket( sal_uLong nPort, sal_uInt16 nMaxCon, sal_Bool bUseMultiChannel )
: CommunicationManagerServer( bUseMultiChannel )
, nPortToListen( nPort )
, nMaxConnections( nMaxCon )
, pAcceptThread( NULL )
{
}

CommunicationManagerServerViaSocket::~CommunicationManagerServerViaSocket()
{
    StopCommunication();
}

sal_Bool CommunicationManagerServerViaSocket::StartCommunication()
{
    if ( !pAcceptThread )
        pAcceptThread = new CommunicationManagerServerAcceptThread( this, nPortToListen, nMaxConnections );
    return sal_True;
}


sal_Bool CommunicationManagerServerViaSocket::StopCommunication()
{
    // Erst den Acceptor anhalten
    delete pAcceptThread;
    pAcceptThread = NULL;

    // Dann alle Verbindungen kappen
    return CommunicationManagerServer::StopCommunication();
}


void CommunicationManagerServerViaSocket::AddConnection( CommunicationLink *pNewConnection )
{
    CallConnectionOpened( pNewConnection );
}


CommunicationManagerServerAcceptThread::CommunicationManagerServerAcceptThread( CommunicationManagerServerViaSocket* pServer, sal_uLong nPort, sal_uInt16 nMaxCon )
: pMyServer( pServer )
, pAcceptorSocket( NULL )
, nPortToListen( nPort )
, nMaxConnections( nMaxCon )
, nAddConnectionEventId( 0 )
, xmNewConnection( NULL )
{
    if ( !pMPostUserEvent )
        pMPostUserEvent = new vos::OMutex;
    create();
}


CommunicationManagerServerAcceptThread::~CommunicationManagerServerAcceptThread()
{
#ifndef aUNX        // Weil das Accept nicht abgebrochen werden kann, so terminiert wenigstens das Prog
    // #62855# pl: gilt auch bei anderen Unixen
    // die richtige Loesung waere natuerlich, etwas auf die pipe zu schreiben,
    // was der thread als Abbruchbedingung erkennt
    // oder wenigstens ein kill anstatt join
    terminate();
    if ( pAcceptorSocket )
        pAcceptorSocket->close();   // Dann das Accept unterbrechen

    join();     // Warten bis fertig

    if ( pAcceptorSocket )
    {
        delete pAcceptorSocket;
        pAcceptorSocket = NULL;
    }
#else
    DEBUGPRINTF ("Destructor CommunicationManagerServerAcceptThread Übersprungen!!!! (wegen Solaris BUG)\n");
#endif
    {
        vos::OGuard aGuard( aMAddConnection );
        if ( nAddConnectionEventId )
        {
            GetpApp()->RemoveUserEvent( nAddConnectionEventId );
            nAddConnectionEventId = 0;
            CommunicationLinkRef xNewConnection = GetNewConnection();
            INFO_MSG( CByteString("Event gelöscht"),
                CByteString( "AddConnectionEvent aus Queue gelöscht"),
                CM_MISC, xNewConnection );
            xNewConnection->InvalidateManager();
            xNewConnection.Clear(); // sollte das Objekt hier löschen
        }
    }
}

void CommunicationManagerServerAcceptThread::run()
{
    if ( !nPortToListen )
        return;

    pAcceptorSocket = new vos::OAcceptorSocket();
    vos::OInetSocketAddr Addr;
    Addr.setPort( nPortToListen );
    pAcceptorSocket->setReuseAddr( 1 );
    if ( !pAcceptorSocket->bind( Addr ) )
    {
        return;
    }
    if ( !pAcceptorSocket->listen( nMaxConnections ) )
    {
        return;
    }


    vos::OStreamSocket *pStreamSocket = NULL;

    while ( schedule() )
    {
        pStreamSocket = new vos::OStreamSocket;
        switch ( pAcceptorSocket->acceptConnection( *pStreamSocket ) )
        {
        case vos::ISocketTypes::TResult_Ok:
            {
                pStreamSocket->setTcpNoDelay( 1 );

                TimeValue sNochEins = {0, 100};
                while ( schedule() && xmNewConnection.Is() )    // Solange die letzte Connection nicht abgeholt wurde warten wir
                    sleep( sNochEins );
                xmNewConnection = new CommunicationLinkViaSocket( pMyServer, pStreamSocket );
                xmNewConnection->StartCallback();
                {
                    vos::OGuard aGuard( aMAddConnection );
                    vos::OGuard aGuard2( *pMPostUserEvent );
                    nAddConnectionEventId = GetpApp()->PostUserEvent( LINK( this, CommunicationManagerServerAcceptThread, AddConnection ) );
                }
            }
            break;
        case vos::ISocketTypes::TResult_TimedOut:
            delete pStreamSocket;
            pStreamSocket = NULL;
            break;
        case vos::ISocketTypes::TResult_Error:
            delete pStreamSocket;
            pStreamSocket = NULL;
            break;

        case vos::ISocketTypes::TResult_Interrupted:
        case vos::ISocketTypes::TResult_InProgress:
            break;  // -Wall not handled...
        }
    }
}


IMPL_LINK( CommunicationManagerServerAcceptThread, AddConnection, void*, EMPTYARG )
{
    {
        vos::OGuard aGuard( aMAddConnection );
        nAddConnectionEventId = 0;
    }
    pMyServer->AddConnection( xmNewConnection );
    xmNewConnection.Clear();
    return 1;
}


#define GETSET(aVar, KeyName, Dafault)                 \
    aVar = aConf.ReadKey(KeyName,"No Entry");          \
    if ( aVar == "No Entry" )                          \
    {                                                  \
        aVar = Dafault;                                \
        aConf.WriteKey(KeyName, aVar);                 \
    }


CommunicationManagerClientViaSocket::CommunicationManagerClientViaSocket( ByteString aHost, sal_uLong nPort, sal_Bool bUseMultiChannel )
: CommunicationManagerClient( bUseMultiChannel )
, aHostToTalk( aHost )
, nPortToTalk( nPort )
{
}

CommunicationManagerClientViaSocket::CommunicationManagerClientViaSocket( sal_Bool bUseMultiChannel )
: CommunicationManagerClient( bUseMultiChannel )
, aHostToTalk( "" )
, nPortToTalk( 0 )
{
}

CommunicationManagerClientViaSocket::~CommunicationManagerClientViaSocket()
{
}


