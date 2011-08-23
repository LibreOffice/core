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

#include <stdio.h>

#include <bf_so3/protocol.hxx>
#include <tools/debug.hxx>

#include <bf_so3/so2dll.hxx>
#include <bf_so3/ipobj.hxx>
#include "bf_so3/ipclient.hxx"
#include <bf_so3/ipenv.hxx>
#include "bf_so3/soerr.hxx"

namespace binfilter {

#define DBG_PROTLOG(FuncName,bVal)              \
{                                               \
    ByteString aTS( ByteString::CreateFromInt32( (ULONG)this ) );        			\
    aTS += "-Obj Edit Prot --- ";               \
    aTS += FuncName;                            \
    aTS += "( ";                                \
    aTS += bVal ? "TRUE" : "FALSE";             \
    aTS += " )";                                \
    DBG_TRACE( aTS.GetBuffer() );               \
}

#define DBG_PROTREC(FuncName)                   \
    DBG_TRACE( FuncName )


/************** struct ImplSvEditObjectProtocol ***************************/
class ImplSvEditObjectProtocol
{
public:
    USHORT              nRefCount;
    BOOL                bConnect:1,
                        bOpen:1,
                        bEmbed:1,
                        bPlugIn:1,
                        bIPActive:1,
                        bUIActive:1;
    BOOL                bCliConnect:1,
                        bCliOpen:1,
                        bCliEmbed:1,
                        bCliPlugIn:1,
                        bCliIPActive:1,
                        bCliUIActive:1;
    BOOL                bSvrConnect:1,
                        bSvrOpen:1,
                        bSvrEmbed:1,
                        bSvrPlugIn:1,
                        bSvrIPActive:1,
                        bSvrUIActive:1;
    BOOL                bLastActionConnect:1,
                        bLastActionOpen:1,
                        bLastActionEmbed:1,
                        bLastActionPlugIn:1,
                        bLastActionIPActive:1,
                        bLastActionUIActive:1;

    BOOL				bTopWinActive:1,
                        bDocWinActive:1,
                        bInClosed:1;

    SvEmbeddedObjectRef aObj;
    SvEmbeddedClientRef aClient;
    SvInPlaceObjectRef  aIPObj;
    SvInPlaceClientRef  aIPClient;

                        ImplSvEditObjectProtocol();
#ifdef DBG_UTIL
                        ~ImplSvEditObjectProtocol();
    void				ClassInvariant() const;
#endif

    void				MakeVisible();
                        // Protokolle
    ErrCode				EmbedProtocol();
    ErrCode				PlugInProtocol();
    ErrCode				IPProtocol();
    ErrCode				UIProtocol();

                        // Welche Protokolle werden unterstuetzt
    BOOL 				CanEmbedProtocol() const;
    BOOL 				CanPlugInProtocol() const;
    BOOL 				CanInPlaceProtocol() const;
    BOOL 				CanUIProtocol() const;

                        // Protokoll zurueckfahren
                        BOOL Reset();
                        BOOL Reset2Connect();
                        BOOL Reset2Open();
                        BOOL Reset2InPlaceActive();

                        // Einzelschritte der Protokolle
                        void Connected( BOOL bOpen );
                        void Opened( BOOL bOpen );
                        void Embedded( BOOL bEmbed );
                        void PlugIn( BOOL bPlugIn );
                        void InPlaceActivate( BOOL bActivate );
                        void UIActivate( BOOL bActivate );
                        void TopWinActivate( BOOL bActivate );
                        void SetTopUIActiveClient( BOOL bTopWinAct, BOOL bUIAct );
                        void DocWinActivate( BOOL bActivate );
};

//=========================================================================
ImplSvEditObjectProtocol::ImplSvEditObjectProtocol()
{
    nRefCount  = 0;
    bConnect  = bCliConnect  = bSvrConnect  = bLastActionConnect  = FALSE;
    bOpen     = bCliOpen     = bSvrOpen     = bLastActionOpen     = FALSE;
    bEmbed    = bCliEmbed    = bSvrEmbed    = bLastActionEmbed    = FALSE;
    bPlugIn   = bCliPlugIn   = bSvrPlugIn   = bLastActionPlugIn   = FALSE;
    bIPActive = bCliIPActive = bSvrIPActive = bLastActionIPActive = FALSE;
    bUIActive = bCliUIActive = bSvrUIActive = bLastActionUIActive = FALSE;
    bTopWinActive	= FALSE;
    bDocWinActive	= FALSE;
    bInClosed		= FALSE;
}

//=========================================================================
#ifdef DBG_UTIL
ImplSvEditObjectProtocol::~ImplSvEditObjectProtocol()
{
}
#endif

//=========================================================================
#define IS_CONNECT()		\
    (bConnect || bCliConnect || bSvrConnect )
#define IS_ALL_CONNECT()	\
    (bConnect && bCliConnect && bSvrConnect )
#define IS_OPEN()			\
    (bOpen || bCliOpen || bSvrOpen )
#define IS_ALL_OPEN()		\
    (bOpen && bCliOpen && bSvrOpen )
#define IS_EMBED()			\
    (bEmbed || bCliEmbed || bSvrEmbed )
#define IS_ALL_EMBED()		\
    (bEmbed && bCliEmbed && bSvrEmbed )
#define IS_PLUGIN() 		\
    (bPlugIn || bCliPlugIn || bSvrPlugIn )
#define IS_ALL_PLUGIN() 	\
    (bPlugIn && bCliPlugIn && bSvrPlugIn )
#define IS_IPACTIVE()		\
    (bIPActive || bCliIPActive || bSvrIPActive )
#define IS_ALL_IPACTIVE()	\
    (bIPActive && bCliIPActive && bSvrIPActive )
#define IS_UIACTIVE()		\
    (bUIActive || bCliUIActive || bSvrUIActive )
#define IS_ALL_UIACTIVE()		\
    (bUIActive && bCliUIActive && bSvrUIActive )

#ifdef DBG_UTIL
void ImplSvEditObjectProtocol::ClassInvariant() const
{
    DBG_ASSERT( IS_ALL_CONNECT() ||
                (!IS_OPEN() && !IS_EMBED() && !IS_PLUGIN()
                && !IS_IPACTIVE() && !IS_UIACTIVE()),
                "not full connect, with higher status" );
    DBG_ASSERT( IS_ALL_OPEN() ||
                (!IS_EMBED() && !IS_PLUGIN() && !IS_IPACTIVE() && !IS_UIACTIVE()),
                "not full open, with higher status" );
    DBG_ASSERT( IS_ALL_IPACTIVE() || !IS_UIACTIVE(),
                "not full ipactive, with higher status" );
    DBG_ASSERT( !IS_EMBED() ||
                (IS_EMBED() && !IS_PLUGIN() && !IS_IPACTIVE() && !IS_UIACTIVE()),
                "embed and active or plugin" );
    DBG_ASSERT( !IS_PLUGIN() ||
                (IS_PLUGIN() && !IS_EMBED() && !IS_IPACTIVE() && !IS_UIACTIVE()),
                "plugin, and active or embed" );
    DBG_ASSERT( !IS_IPACTIVE() ||
                (IS_IPACTIVE() && !IS_EMBED() && !IS_PLUGIN() ),
                "active and embed or plugin" );

    DBG_ASSERT( !IS_UIACTIVE() ||
                (IS_ALL_OPEN() && IS_ALL_CONNECT()
                && IS_ALL_IPACTIVE()),
                "uiactive, without full lower status" );
    DBG_ASSERT( !IS_IPACTIVE() ||
                (IS_ALL_OPEN() && IS_ALL_CONNECT()),
                "ipactive, without full lower status" );
    DBG_ASSERT( !IS_OPEN() || IS_ALL_CONNECT(),
                "open, without full lower status" );
    DBG_ASSERT( !IS_EMBED() ||
                (IS_ALL_OPEN() && IS_ALL_CONNECT()),
                "embed, without full lower status" );
    DBG_ASSERT( !IS_PLUGIN() ||
                (IS_ALL_OPEN() && IS_ALL_CONNECT()),
                "plugin, without full lower status" );
}
#define CLASS_INVARIANT ClassInvariant();
#else
#define CLASS_INVARIANT
#endif

void SvEditObjectProtocol::Imp_DeleteDefault()
{
    delete SOAPP->pIEOPDflt;
    SOAPP->pIEOPDflt = NULL;
}

/************** class SvEditObjectProtocol **********************************/
/************************************************************************
|*
|*    SvEditObjectProtocol::SvEditObjectProtocol()
|*    SvEditObjectProtocol::operator = ()
|*    SvEditObjectProtocol::~SvEditObjectProtocol()
|*    SvEditObjectProtocol::Release()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.08.94
|*    Letzte Aenderung  MM 15.08.94
|*
*************************************************************************/
SvEditObjectProtocol::SvEditObjectProtocol()
{
/*
    if( !SOAPP->pIEOPDflt )
    {
        SOAPP->pIEOPDflt = new ImplSvEditObjectProtocol();
        SOAPP->pIEOPDflt->nRefCount = 1; // nie freigeben
    }
    pImp = SOAPP->pIEOPDflt;
    pImp->nRefCount++;
*/
    pImp = new ImplSvEditObjectProtocol();
    pImp->nRefCount = 1;
}

SvEditObjectProtocol::SvEditObjectProtocol( const SvEditObjectProtocol & rObj )
{
    pImp = rObj.pImp;
    pImp->nRefCount++;
}

SvEditObjectProtocol & SvEditObjectProtocol::operator = ( const SvEditObjectProtocol & rObj )
{
    if( rObj.pImp == pImp )
        return *this;
    rObj.pImp->nRefCount++;
    Release();
    pImp = rObj.pImp;
    return *this;
}

SvEditObjectProtocol::~SvEditObjectProtocol()
{
    Release();
}

BOOL SvEditObjectProtocol::Release()
{
    if( 1 == pImp->nRefCount )
    {
        Reset();
        delete pImp;
        return TRUE;
    }
    else
        --pImp->nRefCount;
    return FALSE;
}

/************************************************************************
|*
|*    SvEditObjectProtocol::GetObj()
|*    SvEditObjectProtocol::GetClient()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.08.94
|*    Letzte Aenderung  MM 15.08.94
|*
*************************************************************************/
SvEmbeddedObject * SvEditObjectProtocol::GetObj() const
{
    return pImp->aObj;
}
SvInPlaceObject * SvEditObjectProtocol::GetIPObj() const
{
    return pImp->aIPObj;
}
SvEmbeddedClient * SvEditObjectProtocol::GetClient() const
{
    return pImp->aClient;
}
SvInPlaceClient * SvEditObjectProtocol::GetIPClient() const
{
    return pImp->aIPClient;
}

/************************************************************************
|*
|*    SvEditObjectProtocol::IsConnect()
|*    SvEditObjectProtocol::IsOpen()
|*    SvEditObjectProtocol::IsEmbed()
|*    SvEditObjectProtocol::IsInPlaceActive()
|*    SvEditObjectProtocol::IsUIActive()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.08.94
|*    Letzte Aenderung  MM 15.08.94
|*
*************************************************************************/
BOOL SvEditObjectProtocol::IsConnect() const
{
    return pImp->bConnect;
}
BOOL SvEditObjectProtocol::IsOpen() const
{
    return pImp->bOpen;
}
BOOL SvEditObjectProtocol::IsEmbed() const
{
    return pImp->bEmbed;
}
BOOL SvEditObjectProtocol::IsPlugIn() const
{
    return pImp->bPlugIn;
}
BOOL SvEditObjectProtocol::IsInPlaceActive() const
{
    return pImp->bIPActive;
}
BOOL SvEditObjectProtocol::IsUIActive() const
{
    return pImp->bUIActive;
}

//=========================================================================
void ImplSvEditObjectProtocol::MakeVisible()
/*	[Beschreibung]

    Zeigt das Objekt an, wenn das Objekt IP, Embed oder PlugIn-aktiv ist.

    [Querverweise]

*/
{
    if( bIPActive || bEmbed || bPlugIn )
        aClient->MakeVisible();
}

//=========================================================================
ErrCode ImplSvEditObjectProtocol::EmbedProtocol()
{
    CLASS_INVARIANT

    if( !aClient.Is() || !aObj.Is() )
        return ERRCODE_SO_GENERALERROR;

    ErrCode nRet = ERRCODE_NONE;
    if( !bIPActive && !bEmbed && !bPlugIn && aObj->Owner() )
    {
        if( !bOpen )
        {
            nRet = aObj->DoOpen( TRUE );
            if( ERRCODE_TOERROR( nRet ) )
                // Abbrechen, wenn nicht geoeffnet werden kann
                return nRet;
        }
        else
            // in den offen-Status zurueck
            Reset2Open();

        if( !bIPActive && !bEmbed && !bPlugIn )
        { // der Status in dem wir nach Embed wechseln koennen
            nRet = aObj->DoEmbed( TRUE );
        }
    }

    MakeVisible();
    // IPProtocol ist TRUE, wenn eines der drei Protokolle
    // gefahren werden konnte
    if( !bIPActive && !bEmbed && !bPlugIn && !ERRCODE_TOERROR( nRet ) )
        nRet = ERRCODE_SO_NOT_INPLACEACTIVE;
    return nRet;
}

ErrCode SvEditObjectProtocol::EmbedProtocol()
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    return pImp->EmbedProtocol();
}

//=========================================================================
ErrCode ImplSvEditObjectProtocol::PlugInProtocol()
/*	[Beschreibung]

    Unterst"utzt der Client das PlugIn-Protokol, dann wird dieses
    ausgefuehrt. Ansonsten wird das
    <SvEditObjectProtocol::EmbedProtocol()> aktiviert.

    [R"uckgabewert]

    BOOL	Befindet sich das Protokol im Status Embed, InPlaceActive
            oder PlugIn wird TRUE, sondt FALSE zur"uckgegeben.

    [Querverweise]

    <SvEmbeddedClient::CanPlugIn()>
*/
{
    CLASS_INVARIANT

    if( !aClient.Is() || !aObj.Is() )
        return ERRCODE_SO_GENERALERROR;

    ErrCode nRet = ERRCODE_NONE;

    if( !bIPActive && !bEmbed && !bPlugIn && aObj->Owner() )
    {
        BOOL bCanPlugIn = aClient->CanPlugIn();
        if( bCanPlugIn )
        {
            if( !bOpen )
            {
                nRet = aObj->DoOpen( TRUE );
                if( ERRCODE_TOERROR( nRet ) )
                    // Abbrechen, wenn nicht geoeffnet werden kann
                    return nRet;
            }
            else
                // in den offen-Status zurueck
                Reset2Open();

            if( !bIPActive && !bEmbed && !bPlugIn )
            { // der Status in dem wir nach PlugIn wechseln koennen
                nRet = aObj->DoPlugIn( TRUE );
            }
        }
        else
            return EmbedProtocol();
    }

    //MakeVisible();
    // IPProtocol ist TRUE, wenn eines der drei Protokolle
    // gefahren werden konnte
    if( !bIPActive && !bEmbed && !bPlugIn && !ERRCODE_TOERROR( nRet ) )
        nRet = ERRCODE_SO_NOT_INPLACEACTIVE;
    return nRet;
}

ErrCode SvEditObjectProtocol::PlugInProtocol()
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    return pImp->PlugInProtocol();
}

/************************************************************************
|*    SvEditObjectProtocol::InPlaceProtocol()
|*
|*    Beschreibung
*************************************************************************/
ErrCode ImplSvEditObjectProtocol::IPProtocol()
{
    CLASS_INVARIANT

    if( !aIPClient.Is() || !aIPObj.Is() )
        return PlugInProtocol();

    ErrCode nRet = ERRCODE_NONE;
    if( !bIPActive && !bEmbed && !bPlugIn && aObj->Owner() )
    {
        // auf beiden Seiten IP-Service
        BOOL bIP = aIPClient->CanInPlaceActivate();
        nRet = aIPObj->DoOpen( TRUE );
        if( ERRCODE_TOERROR( nRet ) )
            // Abbrechen, wenn nicht geoeffnet werden kann
            return nRet;

        if( bIP )
        {
            if( !bIPActive )
                nRet = aIPObj->DoInPlaceActivate( TRUE );
            //MakeVisible();
        }
        else
            // auf PlugIn Protokoll umsteigen
            nRet = PlugInProtocol();
    }
    /*
    else
        MakeVisible();
        */

    CLASS_INVARIANT

    // IPProtocol ist TRUE, wenn eines der drei Protokolle
    // gefahren werden konnte
    if( !bIPActive && !bEmbed && !bPlugIn && !ERRCODE_TOERROR( nRet ) )
        nRet = ERRCODE_SO_NOT_INPLACEACTIVE;
    return nRet;
}

ErrCode SvEditObjectProtocol::IPProtocol()
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    return pImp->IPProtocol();
}

/************************************************************************
|*    SvEditObjectProtocol::UIProtocol()
|*
|*    Beschreibung
*************************************************************************/
ErrCode ImplSvEditObjectProtocol::UIProtocol()
{
    CLASS_INVARIANT

    if( !aIPClient.Is() || !aIPObj.Is() )
        return PlugInProtocol();

    ErrCode nRet = ERRCODE_NONE;
    if( !bUIActive && !bEmbed && !bPlugIn && aObj->Owner() )
    {
        nRet = IPProtocol();
        if( !bUIActive && bIPActive )
            nRet = aIPObj->DoUIActivate( TRUE );
    }
    else
        MakeVisible();

    CLASS_INVARIANT
    // UIProtocol ist TRUE, wenn eine der drei Protokolle
    // gefahren werden konnte
    if( !bIPActive && !bEmbed && !bPlugIn && !ERRCODE_TOERROR( nRet ) )
        nRet = ERRCODE_SO_NOT_INPLACEACTIVE;
    return nRet;
}

ErrCode SvEditObjectProtocol::UIProtocol()
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    return pImp->UIProtocol();
}

/************************************************************************
|*    SvEditObjectProtocol::Reset()
|*
|*    Beschreibung
*************************************************************************/
BOOL ImplSvEditObjectProtocol::Reset()
{
    CLASS_INVARIANT
    DBG_PROTREC( "Reset" );

    if( bInClosed || (aObj.Is() && aObj->IsInClose()) )
    {
        if( bConnect )
            Reset2Connect(); // bis auf Connect runter
        if( bCliConnect || bSvrConnect )
            // falls nur Client oder nur Server Disconnect bekam
            Connected( FALSE );
    }
    else if( bConnect )
        Connected( FALSE );

    DBG_ASSERT( !IS_CONNECT() && !IS_OPEN()
                && !IS_EMBED() && !IS_PLUGIN()
                && !IS_UIACTIVE() && !IS_IPACTIVE(),
                "cannot Reset()" );
    CLASS_INVARIANT
    return !bConnect;
}

BOOL SvEditObjectProtocol::Reset()
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    return pImp->Reset();
}

/************************************************************************
|*    SvEditObjectProtocol::Reset2Connect()
|*
|*    Beschreibung
*************************************************************************/
BOOL ImplSvEditObjectProtocol::Reset2Connect()
{
    CLASS_INVARIANT
    DBG_PROTREC( "Reset2Connect" );
    if( bOpen )
    {
        Reset2Open();
        if( bOpen )
            aObj->DoOpen( FALSE );
    }
    if( bCliOpen || bSvrOpen )
        // falls nur Client oder nur Server Close bekam
        Opened( FALSE );
    DBG_ASSERT( !IS_OPEN() && !IS_EMBED() && !IS_PLUGIN()
                && !IS_UIACTIVE() && !IS_IPACTIVE(),
                "cannot Reset2Connect()" );
    CLASS_INVARIANT
    return bConnect;
}

BOOL SvEditObjectProtocol::Reset2Connect()
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    return pImp->Reset2Connect();
}

/************************************************************************
|*    SvEditObjectProtocol::Reset2Open()
|*
|*    Beschreibung
*************************************************************************/
BOOL ImplSvEditObjectProtocol::Reset2Open()
{
    CLASS_INVARIANT
    DBG_PROTREC( "Reset2Open" );
    if( bEmbed )
        aObj->DoEmbed( FALSE );
    else if( bPlugIn )
        aObj->DoPlugIn( FALSE );
    else if( bIPActive )
    {
        Reset2InPlaceActive();
        if( bIPActive )
            aIPObj->DoInPlaceActivate( FALSE );
    }
    if( bCliEmbed || bSvrEmbed )
        // falls nur Client oder nur Server Embed bekam
        Embedded( FALSE );
    if( bCliPlugIn || bSvrPlugIn )
        // falls nur Client oder nur Server PlugIn bekam
        PlugIn( FALSE );
    if( bCliIPActive || bSvrIPActive )
        // falls nur Client oder nur Server IPActive bekam
        InPlaceActivate( FALSE );
    DBG_ASSERT( !IS_EMBED() && !IS_PLUGIN()
                && !IS_UIACTIVE() && !IS_IPACTIVE(),
                "cannot Reset2Open()" );
    CLASS_INVARIANT
    return bOpen;
}

BOOL SvEditObjectProtocol::Reset2Open()
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    return pImp->Reset2Open();
}

//=========================================================================
BOOL SvEditObjectProtocol::Reset2Embed()
{
    return IsEmbed();
}

//=========================================================================
BOOL SvEditObjectProtocol::Reset2PlugIn()
/*	[Beschreibung]

    Ist das Protocol im PlugIn Status, dann wird TRUE zur"uckgegeben.

    [R"uckgabewert]

    BOOL

    [Querverweise]
*/
{
    return IsPlugIn();
}

/************************************************************************
|*    SvEditObjectProtocol::Reset2InPlaceActive()
|*
|*    Beschreibung
*************************************************************************/
BOOL ImplSvEditObjectProtocol::Reset2InPlaceActive()
{
    CLASS_INVARIANT
    DBG_PROTREC( "Reset2InPlaceActive" );
    if( bUIActive && aIPObj.Is() )
        aIPObj->DoUIActivate( FALSE );
    if( bCliUIActive || bSvrUIActive )
        // falls nur Client oder nur Server UIActive bekam
        UIActivate( FALSE );
    DBG_ASSERT( !IS_UIACTIVE(),
                "cannot Reset2InPlaceActive()" );
    CLASS_INVARIANT
    return bIPActive;
}

BOOL SvEditObjectProtocol::Reset2InPlaceActive()
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    return pImp->Reset2InPlaceActive();
}

/************************************************************************
|*    SvEditObjectProtocol::Reset2UIActive()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvEditObjectProtocol::Reset2UIActive()
{
    return IsUIActive();
}

/************************************************************************
|*    SvEditObjectProtocol::Connected()
|*
|*    Beschreibung
*************************************************************************/
void ImplSvEditObjectProtocol::Connected( BOOL bConnectP )
{
    CLASS_INVARIANT
    DBG_PROTREC( "Connected" );
    if( bCliConnect == bConnectP && bSvrConnect == bConnectP )
    {
        DBG_ASSERT( bConnect == bConnectP, "connect assert" );
        return; // nichts zu tun
    }

    if( !aClient.Is() || !aObj.Is() )
        return; //keine Kommunikationspartner

    bLastActionConnect = bConnectP;
    if( !bConnectP )
        Reset2Connect();

    if( bLastActionConnect != bConnectP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    DBG_ASSERT( !bOpen, "connect assert failed" );
    bConnect = bConnectP; // vor der Aktion den Status setzen
    if( bLastActionConnect && !bCliConnect )
    { // Ich darf verbinden und zuerst an den Client
        DBG_ASSERT( bConnect && bConnectP && bLastActionConnect && !bCliConnect,
                    "connect assert failed" );
        DBG_ASSERT( aClient.Is(), "connect assert failed" );
        bCliConnect = TRUE;
        DBG_PROTLOG( "Cli - Connected", bConnectP )
        aClient->Connected( TRUE );
    }

    if( bLastActionConnect != bConnectP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    // nach dem ClientConnect darf alles passieren, bis auf loeschen von this
    if( (bLastActionConnect && !bSvrConnect) || (!bLastActionConnect && bSvrConnect) )
    { // Object verbinden Ich darf verbinden
        DBG_ASSERT( bConnect && bConnectP && bLastActionConnect && !bSvrConnect
                    || !bConnect && !bConnectP && !bLastActionConnect && bSvrConnect,
                    "connect assert failed" );
        bSvrConnect = bConnect;
        DBG_PROTLOG( "Obj - Connected", bConnectP )
        aObj->Connect( bConnect );
    }

    if( bLastActionConnect != bConnectP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    if( !bLastActionConnect && bCliConnect )
    { // Ich darf die Verbindund abbrechen. Zuletzt an den Client
        DBG_ASSERT( !bConnect && !bConnectP && !bLastActionConnect && bCliConnect,
                    "connect assert failed" );
        DBG_ASSERT( aClient.Is(), "connect assert failed" );
        bCliConnect = FALSE;
        DBG_PROTLOG( "Cli - Connected", bConnectP )
        aClient->Connected( FALSE );
    }

    if( !bLastActionConnect )
    {
         aObj.Clear();
         aIPObj.Clear();
         aClient.Clear();
         aIPClient.Clear();
    }
    CLASS_INVARIANT
}

/************************************************************************
|*    SvEditObjectProtocol::Opened()
|*
|*    Beschreibung
*************************************************************************/
void ImplSvEditObjectProtocol::Opened( BOOL bOpenP )
{
    CLASS_INVARIANT
    DBG_PROTREC( "Opened" );

    // Letzte Aktion war 'runterfahren, aktuelle Aktion is hochfahren
    // und auf dieser Seite noch nicht komplett 'runtergefahren
    if( bOpenP && !bLastActionOpen && (bCliOpen || bSvrOpen))
        return;

    if( bCliOpen == bOpenP && bSvrOpen == bOpenP )
    {
        DBG_ASSERT( bOpen == bOpenP, "open assert" );
        return; // nichts zu tun
    }
    bLastActionOpen = bOpenP;

    if( bOpenP )
        Connected( bOpenP );
    else
        Reset2Open();

    if( bLastActionOpen != bOpenP )
        return; // irgendeiner hat Rekursiv protokoll geaendert

    DBG_ASSERT( !bEmbed && !bPlugIn && !bIPActive, "open assert failed" );
    bOpen = bOpenP; // vor der Aktion den Status setzen
    if( bLastActionOpen && !bCliOpen )
    { // Ich darf oeffnen und zuerst an den Client
        DBG_ASSERT( bOpen && bOpenP && bLastActionOpen && !bCliOpen,
                    "open assert failed" );
        bCliOpen = TRUE;
        DBG_PROTLOG( "Cli - Opened", bOpenP )
        aClient->Opened( TRUE );
    }

    if( bLastActionOpen != bOpenP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    // nach dem ClientOpen darf alles passieren, bis auf loeschen von this
    if( (bLastActionOpen && !bSvrOpen) || (!bLastActionOpen && bSvrOpen) )
    { // Object oeffnen
        DBG_ASSERT( bOpen && bOpenP && bLastActionOpen && !bSvrOpen
                    || !bOpen && !bOpenP && !bLastActionOpen && bSvrOpen,
                    "open assert failed" );
        bSvrOpen = bOpen;
        DBG_PROTLOG( "Svr - Opened", bOpenP )
        aObj->Open( bOpen );
    }

    if( bLastActionOpen != bOpenP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    if( !bLastActionOpen && bCliOpen )
    { // Ich darf schliessen. Zuletzt an den Client
        DBG_ASSERT( !bOpen && !bOpenP && !bLastActionOpen && bCliOpen,
                    "open assert failed" );
        DBG_ASSERT( aClient.Is(), "open assert failed" );
        bCliOpen = FALSE;
        DBG_PROTLOG( "Cli - Opened", bOpenP )
        aClient->Opened( FALSE );
    }
    CLASS_INVARIANT
}

void SvEditObjectProtocol::Opened( BOOL bOpen )
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    pImp->Opened( bOpen );
}

/************************************************************************
|*    SvEditObjectProtocol::Embedded()
|*
|*    Beschreibung
*************************************************************************/
void ImplSvEditObjectProtocol::Embedded( BOOL bEmbedP )
{
    CLASS_INVARIANT
    DBG_PROTREC( "Embedded" );
    if( bCliEmbed == bEmbedP && bSvrEmbed == bEmbedP )
    {
        DBG_ASSERT( bEmbed == bEmbedP, "Embedded assert" );
        return; // nichts zu tun
    }
    bLastActionEmbed = bEmbedP;

    if( bEmbedP )
        Opened( bEmbedP );
/*  else gibt nicht  ueber embed
        Reset2Embed();
*/

    if( bLastActionEmbed != bEmbedP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    DBG_ASSERT( !bIPActive, "embed assert failed" );
    bEmbed = bEmbedP; // vor der Aktion den Status setzen
    if( bLastActionEmbed && !bCliEmbed )
    { // Ich darf oeffnen und zuerst an den Client
        DBG_ASSERT( bEmbed && bEmbedP && bLastActionEmbed && !bCliEmbed,
                    "embed assert failed" );
        bCliEmbed = TRUE;
        DBG_PROTLOG( "Cli - Embedded", bEmbedP )
        aClient->Embedded( TRUE );
    }

    if( bLastActionEmbed != bEmbedP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    // nach dem ClientEmbed darf alles passieren, bis auf loeschen von this
    if( (bLastActionEmbed && !bSvrEmbed) || (!bLastActionEmbed && bSvrEmbed) )
    { // Object oeffnen
        DBG_ASSERT( bEmbed && bEmbedP && bLastActionEmbed && !bSvrEmbed
                    || !bEmbed && !bEmbedP && !bLastActionEmbed && bSvrEmbed,
                    "embed assert failed" );
        bSvrEmbed = bEmbed;
        DBG_PROTLOG( "Svr - Embedded", bEmbedP )
        aObj->Embed( bEmbed );
        if( bEmbed && aObj->GetDocumentName().Len() )
            aObj->DocumentNameChanged( aObj->GetDocumentName() );
    }

    if( bLastActionEmbed != bEmbedP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    if( !bLastActionEmbed && bCliEmbed )
    { // Ich darf schliessen. Zuletzt an den Client
        DBG_ASSERT( !bEmbed && !bEmbedP && !bLastActionEmbed && bCliEmbed,
                    "embed assert failed" );
        bCliEmbed = FALSE;
        DBG_PROTLOG( "Cli - Embedded", bEmbedP )
        aClient->Embedded( FALSE );
    }
    CLASS_INVARIANT
}

void SvEditObjectProtocol::Embedded( BOOL bEmbed )
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    pImp->Embedded( bEmbed );
}

//=========================================================================
void ImplSvEditObjectProtocol::PlugIn
(
    BOOL bPlugInP
)
/*	[Beschreibung]

    [R"uckgabewert]

    [Querverweise]
*/
{
    CLASS_INVARIANT
    DBG_PROTREC( "PlugIn" );
    if( bCliPlugIn == bPlugInP && bSvrPlugIn == bPlugInP )
    {
        DBG_ASSERT( bPlugIn == bPlugInP, "PlugIn assert" );
        return; // nichts zu tun
    }
    bLastActionPlugIn = bPlugInP;

    if( bPlugInP )
        Opened( bPlugInP );
/*  else gibt nicht  ueber PlugIn
        Reset2PlugIn();
*/

    if( bLastActionPlugIn != bPlugInP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    DBG_ASSERT( !bIPActive, "PlugIn assert failed" );
    bPlugIn = bPlugInP; // vor der Aktion den Status setzen
    if( bLastActionPlugIn && !bCliPlugIn )
    { // Ich darf oeffnen und zuerst an den Client
        DBG_ASSERT( bPlugIn && bPlugInP && bLastActionPlugIn && !bCliPlugIn,
                    "PlugIn assert failed" );
        bCliPlugIn = TRUE;
        DBG_PROTLOG( "Cli - PlugIn", bPlugInP )
        aClient->PlugIn( TRUE );
    }

    if( bLastActionPlugIn != bPlugInP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    // nach dem ClientPlugIn darf alles passieren, bis auf loeschen von this
    if( (bLastActionPlugIn && !bSvrPlugIn) || (!bLastActionPlugIn && bSvrPlugIn) )
    { // Object oeffnen
        DBG_ASSERT( bPlugIn && bPlugInP && bLastActionPlugIn && !bSvrPlugIn
                    || !bPlugIn && !bPlugInP && !bLastActionPlugIn && bSvrPlugIn,
                    "PlugIn assert failed" );
        bSvrPlugIn = bPlugIn;
        DBG_PROTLOG( "Svr - PlugIn", bPlugInP )
        aObj->PlugIn( bPlugIn );
        if( bPlugIn && aObj->GetDocumentName().Len() )
            aObj->DocumentNameChanged( aObj->GetDocumentName() );
    }

    if( bLastActionPlugIn != bPlugInP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    if( !bLastActionPlugIn && bCliPlugIn )
    { // Ich darf schliessen. Zuletzt an den Client
        DBG_ASSERT( !bPlugIn && !bPlugInP && !bLastActionPlugIn && bCliPlugIn,
                    "PlugIn assert failed" );
        bCliPlugIn = FALSE;
        DBG_PROTLOG( "Cli - PlugIn", bPlugInP )
        aClient->PlugIn( FALSE );
    }
    CLASS_INVARIANT
}

//=========================================================================
void SvEditObjectProtocol::PlugIn( BOOL bPlugIn )
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    pImp->PlugIn( bPlugIn );
}

/************************************************************************
|*    SvEditObjectProtocol::InPlaceActivate();
|*
|*    Beschreibung
*************************************************************************/
void ImplSvEditObjectProtocol::InPlaceActivate( BOOL bIPActiveP )
{
    CLASS_INVARIANT
    DBG_PROTREC( "InPlaceActivate" );
    if( bCliIPActive == bIPActiveP && bSvrIPActive == bIPActiveP )
    {
        DBG_ASSERT( bIPActive == bIPActiveP, "IPActive assert" );
        return; // nichts zu tun
    }
    bLastActionIPActive = bIPActiveP;
    if( bIPActiveP )
        Opened( bIPActiveP );
    else
        Reset2InPlaceActive();

    if( bLastActionIPActive != bIPActiveP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    DBG_ASSERT( !bEmbed && !bPlugIn, "inplace assert failed" );
    bIPActive = bIPActiveP; // vor der Aktion den Status setzen
    if( bLastActionIPActive && !bCliIPActive )
    { // Ich darf oeffnen und zuerst an den Client
        DBG_ASSERT( bIPActive && bIPActiveP && bLastActionIPActive && !bCliIPActive,
                    "inplace assert failed" );
        DBG_ASSERT( aIPClient.Is(), "inplace assert failed" );
        bCliIPActive = TRUE;
        DBG_PROTLOG( "Cli - InPlaceActivate", bIPActiveP )
        if( aIPClient->Owner() )
            SvInPlaceClient::GetIPActiveClientList().Insert( aIPClient, LIST_APPEND );
        aIPClient->InPlaceActivate( TRUE );
    }

    if( bLastActionIPActive != bIPActiveP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    // nach dem ClientIPActive darf alles passieren, bis auf loeschen von this
    if( (bLastActionIPActive && !bSvrIPActive) || (!bLastActionIPActive && bSvrIPActive) )
    { // Object oeffnen
        DBG_ASSERT( bIPActive && bIPActiveP && bLastActionIPActive && !bSvrIPActive
                    || !bIPActive && !bIPActiveP && !bLastActionIPActive && bSvrIPActive,
                    "inplace assert failed" );
        DBG_ASSERT( aIPObj.Is(), "inplace assert failed" );
        bSvrIPActive = bIPActive;
        DBG_PROTLOG( "Svr - InPlaceActivate", bIPActiveP )
        if( aIPObj->Owner() )
        {
            if( bIPActive )
                SvInPlaceObject::GetIPActiveObjectList().Insert( aIPObj, LIST_APPEND );
            else
                SvInPlaceObject::GetIPActiveObjectList().Remove( aIPObj );
        }
        if( bIPActive )
        {
            aIPObj->InPlaceActivate( bIPActive );
            if( aIPObj.Is() && bIPActive )
                TopWinActivate( bIPActive );
            if( aIPObj.Is() && bIPActive )
                DocWinActivate( bIPActive );
        }
        else
        {
            DocWinActivate( bIPActive );
            TopWinActivate( bIPActive );
            aIPObj->InPlaceActivate( bIPActive );
        }
    }

    if( bLastActionIPActive != bIPActiveP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    if( !bLastActionIPActive && bCliIPActive )
    { // Ich darf schliessen. Zuletzt an den Client
        DBG_ASSERT( !bIPActive && !bIPActiveP && !bLastActionIPActive && bCliIPActive,
                    "open assert failed" );
        bCliIPActive = FALSE;
        DBG_ASSERT( aIPClient.Is(), "inplace assert failed" );
        DBG_PROTLOG( "Cli - InPlaceActivate", bIPActiveP )
        if( aIPClient->Owner() )
            SvInPlaceClient::GetIPActiveClientList().Remove( aIPClient );
        aIPClient->InPlaceActivate( FALSE );
    }
    CLASS_INVARIANT
}

void SvEditObjectProtocol::InPlaceActivate( BOOL bIPActive )
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    pImp->InPlaceActivate( bIPActive );
}

/************************************************************************
|*    SvEditObjectProtocol::UIActivate();
|*
|*    Beschreibung
*************************************************************************/
void ImplSvEditObjectProtocol::UIActivate( BOOL bUIActiveP )
{
    CLASS_INVARIANT
    DBG_PROTREC( "UIActivate" );
    if( bCliUIActive == bUIActiveP && bSvrUIActive == bUIActiveP )
    {
        DBG_ASSERT( bUIActive == bUIActiveP, "UIActive assert" );
        return; // nichts zu tun
    }
    bLastActionUIActive = bUIActiveP;
    if( bUIActiveP )
        InPlaceActivate( bUIActiveP );
/* gibt nichts ueber UIActive
    else
        Reset2UIActive();
*/
    DBG_PROTLOG( "UIActivate", bUIActiveP )
    DBG_ASSERT( !bEmbed && !bPlugIn, "ui assert failed" );
    bUIActive = bUIActiveP; // vor der Aktion den Status setzen
    if( bLastActionUIActive && !bCliUIActive )
    { // Ich darf ui aktivieren und zuerst an den Client
        DBG_ASSERT( bUIActive && bUIActiveP && bLastActionUIActive && !bCliUIActive,
                    "ui assert failed" );
        DBG_ASSERT( aIPClient.Is(), "inplace assert failed" );
        SvContainerEnvironment * pFrm = aIPClient->GetEnv();

        // falls es einen Parent gibt, diesen UI-Deaktivieren
        SvContainerEnvironment * pParentFrm = pFrm->GetParent();
        if( pParentFrm && pParentFrm->GetIPClient() )
            pParentFrm->GetIPClient()->GetProtocol().Reset2InPlaceActive();

        // Alle UI-Aktiven Clients im Dokument suchen und UI-Deaktivieren
        SvInPlaceClientList * pCList = SOAPP->pIPActiveClientList;
        if( pCList )
        {
            ULONG i = 0;
            while( i < pCList->Count() )
            {
                SvInPlaceClient * pCl = pCList->GetObject( i );
                SvContainerEnvironment * pClEnv = pCl->GetEnv();
                if( pCl->Owner() && pCl != &aIPClient
                  && pCl->GetProtocol().IsUIActive()
                  && pClEnv->GetTopWin() == pFrm->GetTopWin()
                  && pClEnv->GetDocWin() == pFrm->GetDocWin() )
                {
                    pCl->GetProtocol().Reset2InPlaceActive();
                    // wer weis wie die Liste veraendert wurde
                    i = 0;
                }
                else
                    i++;
            }
        }


        if( bLastActionUIActive && !bCliUIActive )
        { // wer weis was DoInPlaceActivate getan hat
            bCliUIActive = TRUE;
            DBG_PROTLOG( "Cli - UIActivate", bUIActiveP )
            aIPClient->UIActivate( TRUE );
            if( aIPObj.Is() && aIPObj->Owner() )
            { // Tools anordnen anstossen, aber nur wenn selbst
                aIPObj->GetIPEnv()->DoTopWinResize();
            }
        }
    }

    if( bLastActionUIActive != bUIActiveP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    // nach dem ClientUIActive darf alles passieren, bis auf loeschen von this
    if( (bLastActionUIActive && !bSvrUIActive) || (!bLastActionUIActive && bSvrUIActive) )
    { // Object oeffnen
        DBG_ASSERT( bUIActive && bUIActiveP && bLastActionUIActive && !bSvrUIActive
                    || !bUIActive && !bUIActiveP && !bLastActionUIActive && bSvrUIActive,
                    "ui assert failed" );
        DBG_ASSERT( aIPObj.Is(), "inplace assert failed" );
        bSvrUIActive = bUIActive;
        DBG_PROTLOG( "Obj - UIActivate", bUIActiveP )
        if( aIPClient->Owner() )
        {
            if( aIPClient->GetEnv()->GetDocWin() )
                SetTopUIActiveClient( bDocWinActive, bUIActive );
            else
                SetTopUIActiveClient( bTopWinActive, bUIActive );
        }
        aIPObj->UIActivate( bUIActive );
    }

    if( bLastActionUIActive != bUIActiveP )
        return; // irgend einer hat rekursiv das Protokoll geaendert

    if( !bLastActionUIActive && bCliUIActive )
    { // Ich darf schliessen. Zuletzt an den Client
        DBG_ASSERT( !bUIActive && !bUIActiveP && !bLastActionUIActive && bCliUIActive,
                    "open assert failed" );
        bCliUIActive = FALSE;
        DBG_ASSERT( aIPClient.Is(), "inplace assert failed" );
        DBG_PROTLOG( "Cli - UIActivate", bUIActiveP )
        aIPClient->UIActivate( FALSE );
    }

    /*
    if( bUIActive )
    {
        if( aIPClient->Owner() )
            // Object anordnen anstossen
            aIPObj->GetIPEnv()->DoRectsChanged();
    }
    */
    CLASS_INVARIANT
}

void SvEditObjectProtocol::UIActivate( BOOL bUIActive )
{
    // Schutz gegen Zuweisungsoperator und delete
    SvEditObjectProtocol aThis( *this );
    pImp->UIActivate( bUIActive );
}

void ImplSvEditObjectProtocol::TopWinActivate( BOOL bActive )
{
    CLASS_INVARIANT
    if( aIPObj.Is() && bActive != bTopWinActive )
    {
        bTopWinActive = bActive;
        SvContainerEnvironment * pEnv = aIPClient->GetEnv();
        if( aIPClient.Is() && pEnv && !pEnv->GetDocWin() )
            SetTopUIActiveClient( bTopWinActive, bUIActive );
        aIPObj->TopWinActivate( bActive );
    }
    CLASS_INVARIANT
}

/************************************************************************
|*    ImpSvEditObjectProtocol::SetTopUIActiveClient()
|*
|*    Beschreibung
*************************************************************************/
void ImplSvEditObjectProtocol::SetTopUIActiveClient( BOOL /*bTopWinActive*/,
                                                    BOOL /*bUIActive*/ )
{
}

void ImplSvEditObjectProtocol::DocWinActivate( BOOL bActive )
{
    CLASS_INVARIANT
    if( aIPObj.Is() && aIPClient.Is() && bActive != bDocWinActive )
    {
        SvContainerEnvironment * pEnv = aIPClient->GetEnv();
        if( !aIPClient->Owner() || (pEnv && pEnv->GetDocWin()) )
        {// eventuell kann das DocWin nicht geholt werden (Send in Send)
            bDocWinActive = bActive;
            SetTopUIActiveClient( bDocWinActive, bUIActive );
            aIPObj->DocWinActivate( bActive );
        }
    }
    CLASS_INVARIANT
}

/************************************************************************
|*    SvEditObjectProtocol::SetInClosed()
|*
|*    Beschreibung
*************************************************************************/
void SvEditObjectProtocol::SetInClosed( BOOL bInClosed )
{
    DBG_ASSERT( bInClosed != pImp->bInClosed, "bInClosed == pImp->bInClosed" );
    pImp->bInClosed = bInClosed;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
