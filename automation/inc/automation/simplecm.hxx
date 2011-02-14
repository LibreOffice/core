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
#ifndef _SIMPLECM_HXX
#define _SIMPLECM_HXX

#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <osl/socket.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>

#include <automation/commdefines.hxx>

// CM steht für CommunicationManager
#define CM_UNLIMITED_CONNECTIONS    0xffff

typedef USHORT CM_NameType;
#define CM_DOTTED   ( (CM_NameType) 01 )
#define CM_FQDN     ( (CM_NameType) 02 )

typedef USHORT CM_InfoType;
// nur eines dieser 3 defines darf verwendet werden
#define CM_NO_TEXT      ( (CM_InfoType) 01 )
#define CM_SHORT_TEXT   ( (CM_InfoType) 02 )
#define CM_VERBOSE_TEXT ( (CM_InfoType) 03 )

#define CM_OPEN         ( (CM_InfoType) 0x0004 )
#define CM_CLOSE        ( (CM_InfoType) 0x0008 )
#define CM_RECEIVE      ( (CM_InfoType) 0x0010 )
#define CM_SEND         ( (CM_InfoType) 0x0020 )
#define CM_ERROR        ( (CM_InfoType) 0x0040 )
#define CM_MISC         ( (CM_InfoType) 0x0080 )

#define CM_USER_1       ( (CM_InfoType) 0x0100 )
#define CM_USER_2       ( (CM_InfoType) 0x0200 )
#define CM_USER_3       ( (CM_InfoType) 0x0400 )
#define CM_USER_4       ( (CM_InfoType) 0x0800 )

#define CM_ALL          ( CM_OPEN | CM_CLOSE | CM_RECEIVE | CM_SEND | CM_ERROR | CM_MISC )
#define CM_NONE         ( 0 )

#define CByteString( constAsciiStr ) ByteString( RTL_CONSTASCII_STRINGPARAM ( constAsciiStr ) )

#define INFO_MSG( Short, Long, Type, CLink ) \
{ \
    if ( (Type & GetInfoType()) > 0 ) \
    { \
        switch ( GetInfoType() & 03 ) \
        { \
            case CM_NO_TEXT: \
                { \
                       ByteString aByteString; \
                    CallInfoMsg( InfoString( aByteString, Type, CLink ) ); \
                } \
                break; \
            case CM_SHORT_TEXT: \
                { \
                       ByteString aByteString( Short ); \
                    CallInfoMsg( InfoString( aByteString, Type, CLink ) ); \
                } \
                break; \
            case CM_VERBOSE_TEXT: \
                { \
                       ByteString aByteString( Long ); \
                    CallInfoMsg( InfoString( aByteString, Type, CLink ) ); \
                } \
                break; \
            default: \
                break; \
        } \
    } \
}\

class CommunicationLink;

SV_DECL_REF( CommunicationLink )

class InfoString : public ByteString
{
public:
    InfoString( ByteString &nMsg, CM_InfoType nIT, CommunicationLink *pCL = NULL ): ByteString( nMsg ), nInfoType( nIT ), pCommLink( pCL ) {;}
    CM_InfoType GetInfoType(){ return nInfoType; }
    CommunicationLinkRef GetCommunicationLink(){ return pCommLink; }
private:
    CM_InfoType nInfoType;
    CommunicationLinkRef pCommLink;
};

class PacketHandler;
class CommunicationManager;
class SingleCommunicationManager;
class MultiCommunicationManager;
class CommunicationManagerServerAcceptThread;
class CommunicationLink : public SvRefBase
{
protected:
    friend class CommunicationManager;
    friend class SingleCommunicationManager;
    friend class MultiCommunicationManager;
    friend class CommunicationManagerServerAcceptThread;
    // Darf nicht abgeräumt werden zwischen Empfang des Streams und ende des Callbacks

protected:  // so daß nur über Ref gelöscht werden kann
    virtual ~CommunicationLink();
    void InvalidateManager() { pMyManager = NULL; }

    PacketHandler* pPacketHandler;

public:
    CommunicationLink( CommunicationManager *pMan );

    virtual BOOL StopCommunication()=0;
    virtual BOOL IsCommunicationError()=0;
    CommunicationManager* GetCommunicationManager(){ return pMyManager; }

//  Der Name oder die IP-Adresse oder sonstwas um den Communikationspartner zu identifizieren
    virtual ByteString GetCommunicationPartner( CM_NameType eType )=0;

//  Der Name oder die IP-Adresse oder sonstwas um den Communikationspartner zu identifizieren
    virtual ByteString GetMyName( CM_NameType eType )=0;

//  Liefert einen neuen Stream zum Versenden von Daten.
    virtual SvStream* GetBestCommunicationStream()=0;

    /** will call virtual function DoTransferDataStream to do actual work
        Purpos is to allow housekeeping
    **/
    BOOL TransferDataStream( SvStream *pDataStream, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );

    // Liefert die ID, die vom Sender angegeben wurde.
    // Dadurch lassen sich virtuelle Kommunikationen über einen physikalischen Link realisiren.
    // Da die Kommunikation zu älteren Versionen kompatibel bleiben muß, muß der Empfänger raten,
    // die neue oder die alte verwendet wird, da sich der Kopf eines Auftrages dann ändert.
    UINT16 GetProtocol(){ return nServiceProtocol; }

    // Der Stream wird hier übergeben. Der Aufrufer ist für dessen Löschung zuständig
    // Die Methode MUSS gerufen werden, da sonst keine weiteren Daten empfangen werden.
    SvStream* GetServiceData(){ SvStream *pTemp = pServiceData; pServiceData = NULL; return pTemp; }

    /// Ermöglicht das Auslösen des nächsten Callbacks. Wird auch Implizit gerufen.
    void FinishCallback(){ bIsInsideCallback = FALSE; }

    /// Syncrones Empfangen der Daten. Nur für Kommandozeile, sonst leer implementiert
    virtual BOOL ReceiveDataStream(){ return FALSE; }

    /// Statistics
    DateTime GetStart() { return aStart; }
    ULONG GetTotalBytes() { return nTotalBytes; }
    DateTime GetLastAccess() { return aLastAccess; }
    const ByteString& GetApplication() { return maApplication; }
    virtual void SetApplication( const ByteString& aApp );

protected:
    void CallInfoMsg( InfoString aMsg );
    CM_InfoType GetInfoType();
    CommunicationManager *pMyManager;
// Diese Methoden werden im Main Kontext gerufen und an den Manager weitergereicht.
    virtual DECL_LINK( ConnectionClosed, void* = NULL );
    virtual DECL_LINK( DataReceived, void* = NULL );

    virtual BOOL DoTransferDataStream( SvStream *pDataStream, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );

    SvStream *pServiceData;
    UINT16 nServiceProtocol;
    UINT16 nServiceHeaderType;

    /// Verhindert das vorzeitige Auslösen des nächsten Callbacks.
    void StartCallback(){ bIsInsideCallback = TRUE; }
    BOOL bIsInsideCallback;

    virtual BOOL SendHandshake( HandshakeType aHandshakeType, SvStream* pData = NULL)=0;

    virtual BOOL ShutdownCommunication() = 0;   /// Really stop the Communication

    /// Statistics
    DateTime aStart;
    ULONG nTotalBytes;
    DateTime aLastAccess;

private:
    ByteString maApplication;

#if OSL_DEBUG_LEVEL > 1
public:
    // misc (debuging) purposes
    BOOL bFlag;
    ULONG nSomething;
#endif

};

SV_IMPL_REF( CommunicationLink );

class CommonSocketFunctions;
class CommunicationManager
{
    friend class CommunicationLink;
    friend class CommonSocketFunctions;
public:
    CommunicationManager( BOOL bUseMultiChannel = FALSE );
    virtual ~CommunicationManager();

    virtual BOOL StartCommunication()=0;
    virtual BOOL StartCommunication( String aApp, String aParams );
    virtual BOOL StartCommunication( ByteString aHost, ULONG nPort );
    virtual BOOL StopCommunication()=0;     // Hält alle CommunicationLinks an
    virtual BOOL IsCommunicationRunning() { return bIsCommunicationRunning; }
//  virtual BOOL IsCommunicationError();

//  Der Name oder die IP-Adresse oder sonstwas um den Communikationspartner zu identifizieren
    virtual ByteString GetMyName( CM_NameType eType );

    virtual BOOL IsLinkValid( CommunicationLink* pCL )=0;   // Notwendig für call im Destruktor

    virtual USHORT GetCommunicationLinkCount()=0;
    virtual CommunicationLinkRef GetCommunicationLink( USHORT nNr )=0;

    // Liefert den letzten neuen Link oder NULL wenn dieser schon wieder geschlossen ist.
    CommunicationLinkRef GetLastNewLink() { return xLastNewLink; }

    void SetConnectionOpenedHdl( Link lConnectionOpened ){ mlConnectionOpened = lConnectionOpened; }
    void SetConnectionClosedHdl( Link lConnectionClosed ){ mlConnectionClosed = lConnectionClosed; }
    void SetDataReceivedHdl( Link lDataReceived ){ mlDataReceived = lDataReceived; }
    void SetInfoMsgHdl( Link lInfoMsg ){ mlInfoMsg = lInfoMsg; }

    void SetInfoType( CM_InfoType nIT ){ nInfoType = nIT; }
    CM_InfoType GetInfoType(){ return nInfoType; }

    BOOL IsMultiChannel(){ return bIsMultiChannel; }
    void SetApplication( const ByteString& aApp, BOOL bRunningLinks = FALSE );
    const ByteString& GetApplication() { return maApplication; }

protected:
    // Diese Methoden werden innerhalb gerufen. Sie erledigen eventuelles Housekeeping
    // und rufen dann die entsprechende Methode
    virtual void CallConnectionOpened( CommunicationLink* pCL );
    virtual void CallConnectionClosed( CommunicationLink* pCL );
    void CallDataReceived( CommunicationLink* pCL );
    void CallInfoMsg( InfoString aMsg );

    CM_InfoType nInfoType;

    //  Diese Routinen rufen den Link oder sind überladen
    virtual void ConnectionOpened( CommunicationLink* pCL ){ mlConnectionOpened.Call( pCL ); }
    virtual void ConnectionClosed( CommunicationLink* pCL ){ mlConnectionClosed.Call( pCL ); }
    virtual void DataReceived( CommunicationLink* pCL ){ mlDataReceived.Call( pCL ); }
    virtual void InfoMsg( InfoString aMsg ){ mlInfoMsg.Call( &aMsg ); }

    BOOL bIsCommunicationRunning;

    virtual void DestroyingLink( CommunicationLink *pCL )=0;    // Link trägt sich im Destruktor aus

private:
    ByteString maApplication;
    Link mlConnectionOpened;
    Link mlConnectionClosed;
    Link mlDataReceived;
    Link mlInfoMsg;
    CommunicationLinkRef xLastNewLink;

    BOOL bIsMultiChannel;
};

class SingleCommunicationManager : public CommunicationManager
{
public:
    SingleCommunicationManager( BOOL bUseMultiChannel = FALSE );
    virtual ~SingleCommunicationManager();
    virtual BOOL StopCommunication();       // Hält alle CommunicationLinks an
    virtual BOOL IsLinkValid( CommunicationLink* pCL );
    virtual USHORT GetCommunicationLinkCount();
    virtual CommunicationLinkRef GetCommunicationLink( USHORT nNr );

protected:
    virtual void CallConnectionOpened( CommunicationLink* pCL );
    virtual void CallConnectionClosed( CommunicationLink* pCL );
    CommunicationLinkRef xActiveLink;
    CommunicationLink *pInactiveLink;
    virtual void DestroyingLink( CommunicationLink *pCL );  // Link trägt sich im Destruktor aus
};

class ICommunicationManagerClient
{
    friend class CommonSocketFunctions;
protected:
    virtual BOOL RetryConnect() { return FALSE; }   // Kann dann eventuell die Applikation starten
};

class TCPIO;
class SimpleCommunicationLinkViaSocket : public CommunicationLink
{
public:
    virtual BOOL IsCommunicationError();
    virtual BOOL StopCommunication();

    virtual ByteString GetCommunicationPartner( CM_NameType eType );
    virtual ByteString GetMyName( CM_NameType eType );
    virtual SvStream* GetBestCommunicationStream();
    virtual void SetApplication( const ByteString& aApp );

private:
    ByteString aCommunicationPartner;
    ByteString aMyName;

    TCPIO* pTCPIO;
    osl::StreamSocket* pStreamSocket;

protected:
    SimpleCommunicationLinkViaSocket( CommunicationManager *pMan, osl::StreamSocket* pSocket );
    virtual ~SimpleCommunicationLinkViaSocket();

    osl::StreamSocket* GetStreamSocket() { return pStreamSocket; }
    void SetStreamSocket( osl::StreamSocket* pSocket );

    SvStream *pReceiveStream;
    BOOL DoReceiveDataStream();             /// Recieve DataPacket from Socket
    virtual BOOL SendHandshake( HandshakeType aHandshakeType, SvStream* pData = NULL);
    bool IsReceiveReady();
    BOOL bIsRequestShutdownPending;
    virtual void WaitForShutdown()=0;
    void SetNewPacketAsCurrent();
};

class SimpleCommunicationLinkViaSocketWithReceiveCallbacks : public SimpleCommunicationLinkViaSocket
{
public:
    SimpleCommunicationLinkViaSocketWithReceiveCallbacks( CommunicationManager *pMan, osl::StreamSocket* pSocket );
    ~SimpleCommunicationLinkViaSocketWithReceiveCallbacks();
    virtual BOOL ReceiveDataStream();
protected:
    virtual BOOL ShutdownCommunication();   /// Really stop the Communication
    virtual void WaitForShutdown();
};

class CommonSocketFunctions
{
public:
    BOOL DoStartCommunication( CommunicationManager *pCM, ICommunicationManagerClient *pCMC, ByteString aHost, ULONG nPort );
protected:
    virtual CommunicationLink *CreateCommunicationLink( CommunicationManager *pCM, osl::ConnectorSocket* pCS )=0;
};

class SingleCommunicationManagerClientViaSocket : public SingleCommunicationManager, public ICommunicationManagerClient, CommonSocketFunctions
{
public:
    using CommunicationManager::StartCommunication;

    SingleCommunicationManagerClientViaSocket( ByteString aHost, ULONG nPort, BOOL bUseMultiChannel = FALSE );
    SingleCommunicationManagerClientViaSocket( BOOL bUseMultiChannel = FALSE );
    virtual BOOL StartCommunication(){ return DoStartCommunication( this, (ICommunicationManagerClient*) this, aHostToTalk, nPortToTalk );}
    virtual BOOL StartCommunication( ByteString aHost, ULONG nPort ){ return DoStartCommunication( this, (ICommunicationManagerClient*) this, aHost, nPort );}
private:
    ByteString aHostToTalk;
    ULONG nPortToTalk;
protected:
    virtual CommunicationLink *CreateCommunicationLink( CommunicationManager *pCM, osl::ConnectorSocket* pCS ){ return new SimpleCommunicationLinkViaSocketWithReceiveCallbacks( pCM, pCS ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
