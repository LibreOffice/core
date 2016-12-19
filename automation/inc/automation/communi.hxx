/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _COMMUNI_HXX
#define _COMMUNI_HXX

#include <svl/svarray.hxx>
#include <vos/thread.hxx>
#include <vos/mutex.hxx>
#include <vcl/timer.hxx>
#include <automation/simplecm.hxx>
#include <automation/automationdllapi.h>

class SvStream;
class SvMemoryStream;
//class Application;

class CommunicationManagerServerAcceptThread;
SV_DECL_PTRARR_SORT( CommunicationLinkList, CommunicationLink*, 1, 10 )

class AUTOMATION_DLLPUBLIC MultiCommunicationManager : public CommunicationManager
{
public:
    MultiCommunicationManager( sal_Bool bUseMultiChannel = sal_False );
    virtual ~MultiCommunicationManager();
    virtual sal_Bool StopCommunication();       // Hält alle CommunicationLinks an
    virtual sal_Bool IsLinkValid( CommunicationLink* pCL );
    virtual sal_uInt16 GetCommunicationLinkCount();
    virtual CommunicationLinkRef GetCommunicationLink( sal_uInt16 nNr );

    void DoQuickShutdown( sal_Bool bQuickShutdown = sal_True) { bGracefullShutdown = !bQuickShutdown; }

protected:
    virtual void CallConnectionOpened( CommunicationLink* pCL );
    virtual void CallConnectionClosed( CommunicationLink* pCL );
    CommunicationLinkList *ActiveLinks;
    CommunicationLinkList *InactiveLinks;       /// Hier sind die CommunicationLinks drin, die sich noch nicht selbst abgemeldet haben.
                                                /// allerdings schon ein StopCommunication gekriegt haben, bzw ein ConnectionTerminated
    virtual void DestroyingLink( CommunicationLink *pCL );  // Link trägt sich im Destruktor aus

    sal_Bool bGracefullShutdown;
};

class AUTOMATION_DLLPUBLIC CommunicationManagerServer : public MultiCommunicationManager
{
public:
    CommunicationManagerServer( sal_Bool bUseMultiChannel = sal_False ):MultiCommunicationManager( bUseMultiChannel ){;}
};

class AUTOMATION_DLLPUBLIC CommunicationManagerClient : public MultiCommunicationManager, public ICommunicationManagerClient
{
public:
    CommunicationManagerClient( sal_Bool bUseMultiChannel = sal_False );
};

class AUTOMATION_DLLPUBLIC CommunicationLinkViaSocket : public SimpleCommunicationLinkViaSocket, public vos::OThread
{
public:
    CommunicationLinkViaSocket( CommunicationManager *pMan, vos::OStreamSocket *pSocket );
    virtual ~CommunicationLinkViaSocket();

    virtual sal_Bool IsCommunicationError();
    virtual sal_Bool DoTransferDataStream( SvStream *pDataStream, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );

    // Diese sind Virtuelle Links!!!!
    virtual long ConnectionClosed( void* = NULL );
    virtual long DataReceived( void* = NULL );

    virtual sal_Bool StopCommunication();

    void SetPutDataReceivedHdl( Link lPutDataReceived ){ mlPutDataReceived = lPutDataReceived; }
    Link GetDataReceivedLink () {Link aLink = LINK( this, CommunicationLinkViaSocket, DataReceived ); return aLink;}
    DECL_LINK( PutDataReceivedHdl, CommunicationLinkViaSocket* );

protected:
    virtual void SAL_CALL run();

    virtual sal_Bool ShutdownCommunication();
    sal_uLong nConnectionClosedEventId;
    sal_uLong nDataReceivedEventId;
    vos::OMutex aMConnectionClosed; // Notwendig, da Event verarbeitet werden kann bevor Variable gesetzt ist
    vos::OMutex aMDataReceived;     // Notwendig, da Event verarbeitet werden kann bevor Variable gesetzt ist
    virtual void WaitForShutdown();

    DECL_LINK( ShutdownLink, void* );
       Timer aShutdownTimer;
    sal_Bool bShutdownStarted;
    sal_Bool bDestroying;
    Link mlPutDataReceived;
};

class AUTOMATION_DLLPUBLIC CommunicationManagerServerViaSocket : public CommunicationManagerServer
{
    friend class CommunicationManagerServerAcceptThread;
public:
    using CommunicationManager::StartCommunication;

    CommunicationManagerServerViaSocket( sal_uLong nPort, sal_uInt16 nMaxCon, sal_Bool bUseMultiChannel = sal_False );
    virtual ~CommunicationManagerServerViaSocket();

    virtual sal_Bool StartCommunication();
    virtual sal_Bool StopCommunication();

protected:
    sal_uLong nPortToListen;
    sal_uInt16 nMaxConnections;

private:
    CommunicationManagerServerAcceptThread *pAcceptThread;
    void AddConnection( CommunicationLink *pNewConnection );
};

class AUTOMATION_DLLPUBLIC CommunicationManagerServerAcceptThread: public vos::OThread
{
public:
    CommunicationManagerServerAcceptThread( CommunicationManagerServerViaSocket* pServer, sal_uLong nPort, sal_uInt16 nMaxCon = CM_UNLIMITED_CONNECTIONS );
    virtual ~CommunicationManagerServerAcceptThread();
    CommunicationLinkRef GetNewConnection(){ CommunicationLinkRef xTemp = xmNewConnection; xmNewConnection.Clear(); return xTemp; }

protected:
    virtual void SAL_CALL run();

private:
    CommunicationManagerServerViaSocket* pMyServer;
    vos::OAcceptorSocket *pAcceptorSocket;
    sal_uLong nPortToListen;
    sal_uInt16 nMaxConnections;
    sal_uLong nAddConnectionEventId;
    vos::OMutex aMAddConnection;    // Notwendig, da Event verarbeitet werden kann bevor Variable gesetzt ist
    void CallInfoMsg( InfoString aMsg ){ pMyServer->CallInfoMsg( aMsg ); }
    CM_InfoType GetInfoType(){ return pMyServer->GetInfoType(); }

    // Diese beiden werden zum Transport der Connection vom Thread zum Mainthread verwendet.
    CommunicationLinkRef xmNewConnection;
    DECL_LINK( AddConnection, void* );
};

class AUTOMATION_DLLPUBLIC CommunicationManagerClientViaSocket : public CommunicationManagerClient, CommonSocketFunctions
{
public:
    using CommunicationManager::StartCommunication;

    CommunicationManagerClientViaSocket( ByteString aHost, sal_uLong nPort, sal_Bool bUseMultiChannel = sal_False );
    CommunicationManagerClientViaSocket( sal_Bool bUseMultiChannel = sal_False );
    virtual ~CommunicationManagerClientViaSocket();

    virtual sal_Bool StartCommunication(){ return StartCommunication( aHostToTalk, nPortToTalk );}
    virtual sal_Bool StartCommunication( ByteString aHost, sal_uLong nPort ){ return DoStartCommunication( this, (ICommunicationManagerClient*) this, aHost, nPort  );}

private:
    ByteString aHostToTalk;
    sal_uLong nPortToTalk;
protected:
    virtual CommunicationLink *CreateCommunicationLink( CommunicationManager *pCM, vos::OConnectorSocket *pCS ){ return new CommunicationLinkViaSocket( pCM, pCS ); }
};

#endif
