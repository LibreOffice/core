/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: communi.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:16:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <stdio.h>
#if OSL_DEBUG_LEVEL > 1
#define DEBUGPRINTF(x) { printf(x); fflush( stdout ); }
#else
#define DEBUGPRINTF(x)
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_SOCKET_HXX_ //autogen
#include <vos/socket.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

#include "communi.hxx"


/*  Um den Destruktor protected zu machen wurde unten das delete entfernt.
    Die Methode wird ohnehin hucht benutzt.
//              delete *((AE*)pData+n);
*/

#undef  SV_IMPL_PTRARR_SORT
#define SV_IMPL_PTRARR_SORT( nm,AE )\
_SV_IMPL_SORTAR_ALG( nm,AE )\
    void nm::DeleteAndDestroy( USHORT nP, USHORT nL ) { \
        if( nL ) {\
            DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );\
            for( USHORT n=nP; n < nP + nL; n++ ) \
                DBG_ERROR("Das Element der Liste wurde nicht gelöscht"); \
            SvPtrarr::Remove( nP, nL ); \
        } \
    } \
_SV_SEEK_PTR( nm, AE )




SV_IMPL_PTRARR_SORT( CommunicationLinkList, CommunicationLink* );

NAMESPACE_VOS(OMutex) *pMPostUserEvent=NULL;        // Notwendig, da nicht threadfest


CommunicationLinkViaSocket::CommunicationLinkViaSocket( CommunicationManager *pMan, NAMESPACE_VOS(OStreamSocket) *pSocket )
: SimpleCommunicationLinkViaSocket( pMan, pSocket )
, nConnectionClosedEventId( 0 )
, nDataReceivedEventId( 0 )
, bShutdownStarted( FALSE )
, bDestroying( FALSE )
{
    if ( !pMPostUserEvent )
        pMPostUserEvent = new NAMESPACE_VOS(OMutex);
    // this is necassary to prevent the running thread from sending the close event
    // before the open event has been sent.
       StartCallback();

    create();
}

CommunicationLinkViaSocket::~CommunicationLinkViaSocket()
{
    bDestroying = TRUE;
    StopCommunication();
    while ( nConnectionClosedEventId || nDataReceivedEventId )
        GetpApp()->Yield();
    {
        NAMESPACE_VOS(OGuard) aGuard( aMConnectionClosed );
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
        NAMESPACE_VOS(OGuard) aGuard( aMDataReceived );
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

BOOL CommunicationLinkViaSocket::ShutdownCommunication()
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

        NAMESPACE_VOS(OStreamSocket) *pTempSocket = GetStreamSocket();
        SetStreamSocket( NULL );
        delete pTempSocket;

//      ConnectionClosed();     Wird am Ende des Thread gerufen

    }
    else
    {
        join();
    }

    return TRUE;
}

BOOL CommunicationLinkViaSocket::StopCommunication()
{
    if ( !bShutdownStarted )
    {
        return SimpleCommunicationLinkViaSocket::StopCommunication();
    }
    else
    {
        WaitForShutdown();
        return TRUE;
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
        bShutdownStarted = TRUE;
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

BOOL CommunicationLinkViaSocket::IsCommunicationError()
{
    return !isRunning() || SimpleCommunicationLinkViaSocket::IsCommunicationError();
}

void CommunicationLinkViaSocket::run()
{
    BOOL bWasError = FALSE;
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
            NAMESPACE_VOS(OGuard) aGuard( aMDataReceived );
            NAMESPACE_VOS(OGuard) aGuard2( *pMPostUserEvent );
            nDataReceivedEventId = GetpApp()->PostUserEvent( LINK( this, CommunicationLinkViaSocket, DataReceived ) );
        }
    }
    TimeValue sNochEins = {0, 1000000};
    while ( schedule() && bIsInsideCallback )   // solange der letzte Callback nicht beendet ist
        sleep( sNochEins );

    StartCallback();
    {
        NAMESPACE_VOS(OGuard) aGuard( aMConnectionClosed );
        NAMESPACE_VOS(OGuard) aGuard2( *pMPostUserEvent );
        nConnectionClosedEventId = GetpApp()->PostUserEvent( LINK( this, CommunicationLinkViaSocket, ConnectionClosed ) );
    }
}

BOOL CommunicationLinkViaSocket::DoTransferDataStream( SvStream *pDataStream, CMProtocol nProtocol )
{
    if ( !isRunning() )
        return FALSE;

    return SimpleCommunicationLinkViaSocket::DoTransferDataStream( pDataStream, nProtocol );
}

/// Dies ist ein virtueller Link!!!
long CommunicationLinkViaSocket::ConnectionClosed( void* EMPTYARG )
{
    {
        NAMESPACE_VOS(OGuard) aGuard( aMConnectionClosed );
        nConnectionClosedEventId = 0;   // Achtung!! alles andere muß oben gemacht werden.
    }
    ShutdownCommunication();
    return CommunicationLink::ConnectionClosed( );
}

/// Dies ist ein virtueller Link!!!
long CommunicationLinkViaSocket::DataReceived( void* EMPTYARG )
{
    {
        NAMESPACE_VOS(OGuard) aGuard( aMDataReceived );
        nDataReceivedEventId = 0;   // Achtung!! alles andere muß oben gemacht werden.
    }
    return CommunicationLink::DataReceived( );
}




MultiCommunicationManager::MultiCommunicationManager( BOOL bUseMultiChannel )
: CommunicationManager( bUseMultiChannel )
{
    ActiveLinks = new CommunicationLinkList;
    InactiveLinks = new CommunicationLinkList;
}

MultiCommunicationManager::~MultiCommunicationManager()
{
    StopCommunication();

    Timer aTimeout;
    aTimeout.SetTimeout( 40000 );
    aTimeout.Start();
    USHORT nLinkCount = 0;
    USHORT nNewLinkCount = 0;
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

    // Alles weghauen, was nicht rechtzeitig auf die Bäume gekommen ist
    // Was bei StopCommunication übrig geblieben ist, da es sich asynchron austragen wollte
    int i = ActiveLinks->Count();
    while ( i )
    {
        CommunicationLinkRef rTempLink = ActiveLinks->GetObject(i-1);
        ActiveLinks->Remove( i-1 );
        rTempLink->InvalidateManager();
        rTempLink->ReleaseReference();
        i--;
    }
    delete ActiveLinks;

    /// Die Links zwischen ConnectionClosed und Destruktor.
    /// Hier NICHT gerefcounted, da sie sich sonst im Kreis festhaten würden,
    /// da die Links sich erst in ihrem Destruktor austragen
    i = InactiveLinks->Count();
    while ( i )
    {
        CommunicationLinkRef rTempLink = InactiveLinks->GetObject(i-1);
        InactiveLinks->Remove( i-1 );
        rTempLink->InvalidateManager();
        i--;
    }
    delete InactiveLinks;
}

BOOL MultiCommunicationManager::StopCommunication()
{
    // Alle Verbindungen abbrechen
    // ConnectionClosed entfernt die Links aus der Liste. Je nach Implementation syncron
    // oder asyncron. Daher Von oben nach unten Abräumen, so daß sich nichts verschiebt.
    int i = ActiveLinks->Count();
    int nFail = 0;
    while ( i )
    {
        if ( !ActiveLinks->GetObject(i-1)->StopCommunication() )
            nFail++;    // Hochzählen, da Verbindung sich nicht (sofort) beenden lässt.
        i--;
    }

    return nFail == 0;
}

BOOL MultiCommunicationManager::IsLinkValid( CommunicationLink* pCL )
{
    if ( ActiveLinks->Seek_Entry( pCL ) )
        return TRUE;
    else
        return FALSE;
}

USHORT MultiCommunicationManager::GetCommunicationLinkCount()
{
    return ActiveLinks->Count();
}

CommunicationLinkRef MultiCommunicationManager::GetCommunicationLink( USHORT nNr )
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

    USHORT nPos;
    if ( ActiveLinks->Seek_Entry( pCL, &nPos ) )
    {
        InactiveLinks->C40_PTR_INSERT(CommunicationLink, pCL);  // Ohne Reference
        ActiveLinks->Remove( nPos );
    }
    pCL->ReleaseReference();

    bIsCommunicationRunning = ActiveLinks->Count() > 0;
//  delete pCL;
#ifdef DBG_UTIL
        rHold->bFlag = TRUE;
#endif
}

void MultiCommunicationManager::DestroyingLink( CommunicationLink *pCL )
{
    USHORT nPos;
    if ( InactiveLinks->Seek_Entry( pCL, &nPos ) )
        InactiveLinks->Remove( nPos );
    pCL->InvalidateManager();
}



CommunicationManagerClient::CommunicationManagerClient( BOOL bUseMultiChannel )
: MultiCommunicationManager( bUseMultiChannel )
{
    aApplication = ByteString("Something inside ");
    aApplication.Append( ByteString( DirEntry( Application::GetAppFileName() ).GetName(), gsl_getSystemTextEncoding() ) );
}



CommunicationManagerServerViaSocket::CommunicationManagerServerViaSocket( ULONG nPort, USHORT nMaxCon, BOOL bUseMultiChannel )
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

BOOL CommunicationManagerServerViaSocket::StartCommunication()
{
    if ( !pAcceptThread )
        pAcceptThread = new CommunicationManagerServerAcceptThread( this, nPortToListen, nMaxConnections );
    return TRUE;
}


BOOL CommunicationManagerServerViaSocket::StopCommunication()
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


CommunicationManagerServerAcceptThread::CommunicationManagerServerAcceptThread( CommunicationManagerServerViaSocket* pServer, ULONG nPort, USHORT nMaxCon )
: pMyServer( pServer )
, pAcceptorSocket( NULL )
, nPortToListen( nPort )
, nMaxConnections( nMaxCon )
, nAddConnectionEventId( 0 )
, xNewConnection( NULL )
{
    if ( !pMPostUserEvent )
        pMPostUserEvent = new NAMESPACE_VOS(OMutex);
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
        NAMESPACE_VOS(OGuard) aGuard( aMAddConnection );
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

    pAcceptorSocket = new NAMESPACE_VOS(OAcceptorSocket)();
    NAMESPACE_VOS(OInetSocketAddr) Addr;
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


    NAMESPACE_VOS(OStreamSocket) *pStreamSocket = NULL;

    while ( schedule() )
    {
        pStreamSocket = new NAMESPACE_VOS(OStreamSocket);
        switch ( pAcceptorSocket->acceptConnection( *pStreamSocket ) )
        {
        case NAMESPACE_VOS(ISocketTypes::TResult_Ok):
            {
                pStreamSocket->setTcpNoDelay( 1 );

                TimeValue sNochEins = {0, 100};
                while ( schedule() && xNewConnection.Is() ) // Solange die letzte Connection nicht abgeholt wurde warten wir
                    sleep( sNochEins );
                xNewConnection = new CommunicationLinkViaSocket( pMyServer, pStreamSocket );
                xNewConnection->StartCallback();
                {
                    NAMESPACE_VOS(OGuard) aGuard( aMAddConnection );
                    NAMESPACE_VOS(OGuard) aGuard2( *pMPostUserEvent );
                    nAddConnectionEventId = GetpApp()->PostUserEvent( LINK( this, CommunicationManagerServerAcceptThread, AddConnection ) );
                }
            }
            break;
        case NAMESPACE_VOS(ISocketTypes::TResult_TimedOut):
            delete pStreamSocket;
            pStreamSocket = NULL;
            break;
        case NAMESPACE_VOS(ISocketTypes::TResult_Error):
            delete pStreamSocket;
            pStreamSocket = NULL;
            break;

        case NAMESPACE_VOS(ISocketTypes::TResult_Interrupted):
        case NAMESPACE_VOS(ISocketTypes::TResult_InProgress):
            break;  // -Wall not handled...
        }
    }
}


IMPL_LINK( CommunicationManagerServerAcceptThread, AddConnection, void*, EMPTYARG )
{
    {
        NAMESPACE_VOS(OGuard) aGuard( aMAddConnection );
        nAddConnectionEventId = 0;
    }
    pMyServer->AddConnection( xNewConnection );
    xNewConnection.Clear();
    return 1;
}


#define GETSET(aVar, KeyName, Dafault)                 \
    aVar = aConf.ReadKey(KeyName,"No Entry");          \
    if ( aVar == "No Entry" )                          \
    {                                                  \
        aVar = Dafault;                                \
        aConf.WriteKey(KeyName, aVar);                 \
    }


CommunicationManagerClientViaSocket::CommunicationManagerClientViaSocket( ByteString aHost, ULONG nPort, BOOL bUseMultiChannel )
: CommunicationManagerClient( bUseMultiChannel )
, aHostToTalk( aHost )
, nPortToTalk( nPort )
{
}

CommunicationManagerClientViaSocket::CommunicationManagerClientViaSocket( BOOL bUseMultiChannel )
: CommunicationManagerClient( bUseMultiChannel )
, aHostToTalk( "" )
, nPortToTalk( 0 )
{
}

CommunicationManagerClientViaSocket::~CommunicationManagerClientViaSocket()
{
}


