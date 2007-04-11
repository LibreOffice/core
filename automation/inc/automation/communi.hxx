/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: communi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:20:35 $
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

#ifndef _COMMUNI_HXX
#define _COMMUNI_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _VOS_THREAD_HXX_ //autogen
#include <vos/thread.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _SIMPLECM_HXX
#include <automation/simplecm.hxx>
#endif

class SvStream;
class SvMemoryStream;
//class Application;

class CommunicationManagerServerAcceptThread;
SV_DECL_PTRARR_SORT( CommunicationLinkList, CommunicationLink*, 1, 10 )

class MultiCommunicationManager : public CommunicationManager
{
public:
    MultiCommunicationManager( BOOL bUseMultiChannel = FALSE );
    virtual ~MultiCommunicationManager();
    virtual BOOL StopCommunication();       // Hält alle CommunicationLinks an
    virtual BOOL IsLinkValid( CommunicationLink* pCL );
    virtual USHORT GetCommunicationLinkCount();
    virtual CommunicationLinkRef GetCommunicationLink( USHORT nNr );
protected:
    virtual void CallConnectionOpened( CommunicationLink* pCL );
    virtual void CallConnectionClosed( CommunicationLink* pCL );
    CommunicationLinkList *ActiveLinks;
    CommunicationLinkList *InactiveLinks;       /// Hier sind die CommunicationLinks drin, die sich noch nicht selbst abgemeldet haben.
                                                /// allerdings schon ein StopCommunication gekriegt haben, bzw ein ConnectionTerminated
    virtual void DestroyingLink( CommunicationLink *pCL );  // Link trägt sich im Destruktor aus
};

class CommunicationManagerServer : public MultiCommunicationManager
{
public:
    CommunicationManagerServer( BOOL bUseMultiChannel = FALSE ):MultiCommunicationManager( bUseMultiChannel ){;}
};

class CommunicationManagerClient : public MultiCommunicationManager, public ICommunicationManagerClient
{
public:
    CommunicationManagerClient( BOOL bUseMultiChannel = FALSE );
};

class CommunicationLinkViaSocket : public SimpleCommunicationLinkViaSocket, public NAMESPACE_VOS(OThread)
{
public:
    CommunicationLinkViaSocket( CommunicationManager *pMan, NAMESPACE_VOS(OStreamSocket) *pSocket );
    virtual ~CommunicationLinkViaSocket();

    virtual BOOL IsCommunicationError();
    virtual BOOL DoTransferDataStream( SvStream *pDataStream, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );

    // Diese sind Virtuelle Links!!!!
    virtual long ConnectionClosed( void* = NULL );
    virtual long DataReceived( void* = NULL );

    virtual BOOL StopCommunication();

    void SetPutDataReceivedHdl( Link lPutDataReceived ){ mlPutDataReceived = lPutDataReceived; }
    Link GetDataReceivedLink () {Link aLink = LINK( this, CommunicationLinkViaSocket, DataReceived ); return aLink;}
    DECL_LINK( PutDataReceivedHdl, CommunicationLinkViaSocket* );

protected:
    virtual void SAL_CALL run();

    virtual BOOL ShutdownCommunication();
    ULONG nConnectionClosedEventId;
    ULONG nDataReceivedEventId;
    NAMESPACE_VOS(OMutex) aMConnectionClosed;   // Notwendig, da Event verarbeitet werden kann bevor Variable gesetzt ist
    NAMESPACE_VOS(OMutex) aMDataReceived;       // Notwendig, da Event verarbeitet werden kann bevor Variable gesetzt ist
    virtual void WaitForShutdown();

    DECL_LINK( ShutdownLink, void* );
       Timer aShutdownTimer;
    BOOL bShutdownStarted;
    BOOL bDestroying;
    Link mlPutDataReceived;
};

class CommunicationManagerServerViaSocket : public CommunicationManagerServer
{
    friend class CommunicationManagerServerAcceptThread;
public:
    using CommunicationManager::StartCommunication;

    CommunicationManagerServerViaSocket( ULONG nPort, USHORT nMaxCon, BOOL bUseMultiChannel = FALSE );
    virtual ~CommunicationManagerServerViaSocket();

    virtual BOOL StartCommunication();
    virtual BOOL StopCommunication();

protected:
    ULONG nPortToListen;
    USHORT nMaxConnections;

private:
    CommunicationManagerServerAcceptThread *pAcceptThread;
    void AddConnection( CommunicationLink *pNewConnection );
};

class CommunicationManagerServerAcceptThread: public NAMESPACE_VOS(OThread)
{
public:
    CommunicationManagerServerAcceptThread( CommunicationManagerServerViaSocket* pServer, ULONG nPort, USHORT nMaxCon = CM_UNLIMITED_CONNECTIONS );
    virtual ~CommunicationManagerServerAcceptThread();
    CommunicationLinkRef GetNewConnection(){ CommunicationLinkRef xTemp = xmNewConnection; xmNewConnection.Clear(); return xTemp; }

protected:
    virtual void SAL_CALL run();

private:
    CommunicationManagerServerViaSocket* pMyServer;
    NAMESPACE_VOS(OAcceptorSocket) *pAcceptorSocket;
    ULONG nPortToListen;
    USHORT nMaxConnections;
    ULONG nAddConnectionEventId;
    NAMESPACE_VOS(OMutex) aMAddConnection;  // Notwendig, da Event verarbeitet werden kann bevor Variable gesetzt ist
    void CallInfoMsg( InfoString aMsg ){ pMyServer->CallInfoMsg( aMsg ); }
    CM_InfoType GetInfoType(){ return pMyServer->GetInfoType(); }

    // Diese beiden werden zum Transport der Connection vom Thread zum Mainthread verwendet.
    CommunicationLinkRef xmNewConnection;
    DECL_LINK( AddConnection, void* );
};

class CommunicationManagerClientViaSocket : public CommunicationManagerClient, CommonSocketFunctions
{
public:
    using CommunicationManager::StartCommunication;

    CommunicationManagerClientViaSocket( ByteString aHost, ULONG nPort, BOOL bUseMultiChannel = FALSE );
    CommunicationManagerClientViaSocket( BOOL bUseMultiChannel = FALSE );
    virtual ~CommunicationManagerClientViaSocket();

    virtual BOOL StartCommunication(){ return StartCommunication( aHostToTalk, nPortToTalk );}
    virtual BOOL StartCommunication( ByteString aHost, ULONG nPort ){ return DoStartCommunication( this, (ICommunicationManagerClient*) this, aHost, nPort  );}

private:
    ByteString aHostToTalk;
    ULONG nPortToTalk;
protected:
    virtual CommunicationLink *CreateCommunicationLink( CommunicationManager *pCM, NAMESPACE_VOS(OConnectorSocket) *pCS ){ return new CommunicationLinkViaSocket( pCM, pCS ); }
};

#endif
