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

#include "comm_bas.hxx"
#include <tools/errcode.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbx.hxx>
#include <basic/sbxvar.hxx>
#include <vcl/svapp.hxx>
#include <automation/communi.hxx>
#include <basic/ttstrhlp.hxx>

// Der CommunicationManager hat folgende Elemente:
// 1) Properties:
//    Keine
// 2) Methoden:
//    CommunicationLink StartCommunication( Host, Port )
//    StopAllCommunication      // Alle Kommunikation wird abgebrochen
//    sal_Bool IsCommunicationRunning       // Läuft noch irgendwas
//    String GetMyName      Der eigene Name
//    sal_Bool IsLinkValid( CommunicationLink )     // Ist dieser Link noch gültig
//    SetCommunicationEventHandler( String )    // Diese Funktion wird aufgerufen bei jedem Event

// Der CommunicationLink hat folgende Elemente:
// 1) Properties:
//    Keine
// 2) Methoden:
//    StopCommunication     Die Kommunikation wird abgebrochen
//    String GetMyName      Der eigene Name
//    String GetHostName    Der Name des Anderen
//    Send(String )         String an den Partner schicken
//    String GetString      Ergebnis des letzten Empfangs


// Diese Implementation ist ein Beispiel fuer eine tabellengesteuerte
// Version, die sehr viele Elemente enthalten kann. Die Elemente werden
// je nach Bedarf aus der Tabelle in das Objekt uebernommen.

// Das nArgs-Feld eines Tabelleneintrags ist wie folgt verschluesselt:

#define _ARGSMASK   0x00FF  // Bis zu 255 Argumente
#define _RWMASK     0x0F00  // Maske fuer R/W-Bits
#define _TYPEMASK   0xF000  // Maske fuer den Typ des Eintrags

#define _READ       0x0100  // kann gelesen werden
#define _BWRITE     0x0200  // kann as Lvalue verwendet werden
#define _LVALUE     _BWRITE  // kann as Lvalue verwendet werden
#define _READWRITE  0x0300  // beides
#define _OPT        0x0400  // TRUE: optionaler Parameter
#define _METHOD     0x1000  // Masken-Bit fuer eine Methode
#define _PROPERTY   0x2000  // Masken-Bit fuer eine Property
#define _COLL       0x4000  // Masken-Bit fuer eine Collection
                            // Kombination von oberen Bits:
#define _FUNCTION   0x1100  // Maske fuer Function
#define _LFUNCTION  0x1300  // Maske fuer Function, die auch als Lvalue geht
#define _ROPROP     0x2100  // Maske Read Only-Property
#define _WOPROP     0x2200  // Maske Write Only-Property
#define _RWPROP     0x2300  // Maske Read/Write-Property
#define _COLLPROP   0x4100  // Maske Read-Collection-Element

#define COLLNAME    "Elements"  // Name der Collection, hier mal hart verdrahtet



CommunicationWrapper::Methods CommunicationWrapper::aManagerMethods[] = {
// Neue Kommunikation aufbauen
{ "StartCommunication", SbxEMPTY, &CommunicationWrapper::MStartCommunication, 2 | _FUNCTION },
    // Zwei Named Parameter
    { "Host", SbxSTRING, NULL, 0 },
    { "Port", SbxLONG, NULL, 0 },
// Alle Kommunikation wird abgebrochen
{ "StopAllCommunication", SbxEMPTY, &CommunicationWrapper::MStopAllCommunication, 0 | _FUNCTION },
// Läuft noch irgendwas
{ "IsCommunicationRunning", SbxBOOL, &CommunicationWrapper::MIsCommunicationRunning, 0 | _FUNCTION },
// Hostname als FQDN erfragen
{ "GetMyName", SbxSTRING, &CommunicationWrapper::MGetMyName, 0 | _FUNCTION },
// Abfragen ob der Link überhaupt noch gültig ist
{ "IsLinkValid", SbxBOOL, &CommunicationWrapper::MIsLinkValid, 1 | _FUNCTION },
    // Ein Named Parameter
    { "Link", SbxOBJECT, NULL, 0 },
// Dieser Handler wird dauernd gerufen
{ "SetCommunicationEventHandler", SbxEMPTY, &CommunicationWrapper::MSetCommunicationEventHandler, 1 | _FUNCTION },
    // Ein Named Parameter
    { "FuncName", SbxSTRING, NULL, 0 },

{ NULL, SbxNULL, NULL, -1 }};  // Tabellenende






CommunicationWrapper::Methods CommunicationWrapper::aLinkMethods[] = {
// Die Kommunikation wird abgebrochen
{ "StopCommunication", SbxEMPTY, &CommunicationWrapper::LStopCommunication, 0 | _FUNCTION },
// Der eigene Name
{ "GetMyName", SbxSTRING, &CommunicationWrapper::LGetMyName, 0 | _FUNCTION },
// Der Name des Anderen
{ "GetHostName", SbxSTRING, &CommunicationWrapper::LGetHostName, 0 | _FUNCTION },
// String an den Partner schicken
{ "Send", SbxEMPTY, &CommunicationWrapper::LSend, 1 | _FUNCTION },
    // Ein Named Parameter
    { "SendString", SbxSTRING, NULL, 0 },
// Ergebnis des letzten Empfangs
{ "GetString", SbxSTRING, &CommunicationWrapper::LGetString, 0 | _FUNCTION },

{ NULL, SbxNULL, NULL, -1 }};  // Tabellenende





// Konstruktor für den Manager
CommunicationWrapper::CommunicationWrapper( const String& rClass ) : SbxObject( rClass )
, m_pLink( NULL )
, m_bIsManager( sal_True )
, m_bCatchOpen( sal_False )
, m_pNewLink( NULL )
{
    m_pMethods = &aManagerMethods[0];
    m_pManager = new CommunicationManagerClientViaSocket;
    m_pManager->SetConnectionOpenedHdl( LINK( this, CommunicationWrapper, Open ) );
    m_pManager->SetConnectionClosedHdl( LINK( this, CommunicationWrapper, Close ) );
    m_pManager->SetDataReceivedHdl( LINK( this, CommunicationWrapper, Data ) );
}

// Konstruktor für den Link
CommunicationWrapper::CommunicationWrapper( CommunicationLink *pThisLink ) : SbxObject( CUniString("Link") )
, m_pLink( pThisLink )
, m_bIsManager( sal_False )
, m_bCatchOpen( sal_False )
, m_pNewLink( NULL )
{
    m_pMethods = &aLinkMethods[0];
    m_pManager = (CommunicationManagerClientViaSocket*)pThisLink->GetCommunicationManager();
}

// Destruktor
CommunicationWrapper::~CommunicationWrapper()
{
    if ( m_bIsManager )
        delete m_pManager;
}


// Suche nach einem Element:
// Hier wird linear durch die Methodentabelle gegangen, bis eine
// passende Methode gefunden wurde.
// Wenn die Methode/Property nicht gefunden wurde, nur NULL ohne
// Fehlercode zurueckliefern, da so auch eine ganze Chain von
// Objekten nach der Methode/Property befragt werden kann.

SbxVariable* CommunicationWrapper::Find( const String& rName, SbxClassType t )
{
    // Ist das Element bereits vorhanden?
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( !pRes && t != SbxCLASS_OBJECT )
    {
        // sonst suchen
        Methods* p = m_pMethods;
        short nIndex = 0;
        sal_Bool bFound = sal_False;
        while( p->nArgs != -1 )
        {
            if( rName.CompareIgnoreCaseToAscii( p->pName ) == COMPARE_EQUAL )
            {
                bFound = sal_True; break;
            }
            nIndex += ( p->nArgs & _ARGSMASK ) + 1;
            p = m_pMethods + nIndex;
        }
        if( bFound )
        {
            // Args-Felder isolieren:
            short nAccess = ( p->nArgs & _RWMASK ) >> 8;
            short nType   = ( p->nArgs & _TYPEMASK );
            String aName( p->pName, RTL_TEXTENCODING_ASCII_US );
            SbxClassType eCT = SbxCLASS_OBJECT;
            if( nType & _PROPERTY )
                eCT = SbxCLASS_PROPERTY;
            else if( nType & _METHOD )
                eCT = SbxCLASS_METHOD;
            pRes = Make( aName, eCT, p->eType );
            // Wir setzen den Array-Index + 1, da ja noch andere
            // Standard-Properties existieren, die auch aktiviert
            // werden muessen.
            pRes->SetUserData( nIndex + 1 );
            pRes->SetFlags( nAccess );
        }
    }
    return pRes;
}

// Aktivierung eines Elements oder Anfordern eines Infoblocks

void CommunicationWrapper::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCT,
                             const SfxHint& rHint, const TypeId& rHT )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pPar = pVar->GetParameters();
        sal_uInt16 nIndex = (sal_uInt16) pVar->GetUserData();
        // kein Index: weiterreichen!
        if( nIndex )
        {
            sal_uLong t = pHint->GetId();
            if( t == SBX_HINT_INFOWANTED )
                pVar->SetInfo( GetInfo( (short) pVar->GetUserData() ) );
            else
            {
                sal_Bool bWrite = sal_False;
                if( t == SBX_HINT_DATACHANGED )
                    bWrite = sal_True;
                if( t == SBX_HINT_DATAWANTED || bWrite )
                {
                    // Parameter-Test fuer Methoden:
                    sal_uInt16 nPar = m_pMethods[ --nIndex ].nArgs & 0x00FF;
                    // Element 0 ist der Returnwert
                    if( ( !pPar && nPar )
                     || ( pPar && pPar->Count() != nPar+1 ) )
                        SetError( SbxERR_WRONG_ARGS );
                    // Alles klar, man kann den Call ausfuehren
                    else
                    {
                        (this->*(m_pMethods[ nIndex ].pFunc))( pVar, pPar, bWrite );
                    }
                }
            }
        }
        SbxObject::SFX_NOTIFY( rBC, rBCT, rHint, rHT );
    }
}

// Zusammenbau der Infostruktur fuer einzelne Elemente

SbxInfo* CommunicationWrapper::GetInfo( short nIdx )
{
    Methods* p = &m_pMethods[ nIdx ];
    // Wenn mal eine Hilfedatei zur Verfuegung steht:
    // SbxInfo* pInfo = new SbxInfo( Hilfedateiname, p->nHelpId );
    SbxInfo* pRetInfo = new SbxInfo;
    short nPar = p->nArgs & _ARGSMASK;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        String aName( p->pName, RTL_TEXTENCODING_ASCII_US );
        sal_uInt16 nIFlags = ( p->nArgs >> 8 ) & 0x03;
        if( p->nArgs & _OPT )
            nIFlags |= SBX_OPTIONAL;
        pRetInfo->AddParam( aName, p->eType, nIFlags );
    }
    return pRetInfo;
}


////////////////////////////////////////////////////////////////////////////

// Hilfsmethoden für den Manager

IMPL_LINK( CommunicationWrapper, Open, CommunicationLink*, pLink )
{
    if ( m_bCatchOpen )
        m_pNewLink = pLink;
    else
        Events( CUniString("Open"), pLink );
    return 1;
}

IMPL_LINK( CommunicationWrapper, Close, CommunicationLink*, pLink )
{
    Events( CUniString("Close"), pLink );
    return 1;
}

IMPL_LINK( CommunicationWrapper, Data, CommunicationLink*, pLink )
{
    Events( CUniString("Data"), pLink );
    return 1;
}

void CommunicationWrapper::Events( String aType, CommunicationLink* pLink )
{
    if ( m_aEventHandlerName.Len() )
    {
        SbxArrayRef pPar = new SbxArray( SbxVARIANT );
        pPar->Put( new SbxVariable( SbxSTRING ), 1 );
        pPar->Get( 1 )->PutString( aType );

        pPar->Put( new SbxVariable( SbxOBJECT ), 2 );
        pPar->Get( 2 )->PutObject( new CommunicationWrapper( pLink ) );

        Call( m_aEventHandlerName, pPar );
    }
    else
        delete pLink->GetServiceData();     // Stream wegschmeissen um nicht zu blockieren
}


////////////////////////////////////////////////////////////////////////////

// Properties und Methoden legen beim Get (bPut = sal_False) den Returnwert
// im Element 0 des Argv ab; beim Put (bPut = sal_True) wird der Wert aus
// Element 0 gespeichert.

// Die Methoden:

// Manager
void CommunicationWrapper::MStartCommunication( SbxVariable* pVar, SbxArray* pPar, sal_Bool /*bWrite*/ )
{ //    CommunicationLink StartCommunication( Host, Port )
    m_bCatchOpen = sal_True;
    if ( m_pManager->StartCommunication( ByteString( pPar->Get( 1 )->GetString(), RTL_TEXTENCODING_UTF8 ), pPar->Get( 2 )->GetULong() ) )
    {
        while ( !m_pNewLink )
            GetpApp()->Reschedule();
        m_bCatchOpen = sal_False;
        CommunicationWrapper *pNewLinkWrapper = new CommunicationWrapper( m_pNewLink );
        m_pNewLink = NULL;
        pVar->PutObject( pNewLinkWrapper );
    }

}

void CommunicationWrapper::MStopAllCommunication( SbxVariable* /*pVar*/, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    StopAllCommunication        // Alle Kommunikation wird abgebrochen
    m_pManager->StopCommunication();
}

void CommunicationWrapper::MIsCommunicationRunning( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    sal_Bool IsCommunicationRunning     // Läuft noch irgendwas
    pVar->PutBool( m_pManager->IsCommunicationRunning() );
}

void CommunicationWrapper::MGetMyName( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    String GetMyName        Der eigene Name
    pVar->PutString( UniString( m_pManager->GetMyName( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::MIsLinkValid( SbxVariable* pVar, SbxArray* pPar, sal_Bool /*bWrite*/ )
{ //    sal_Bool IsLinkValid( CommunicationLink )       // Ist dieser Link noch gültig
    CommunicationWrapper *pWrapper = (CommunicationWrapper*)(pPar->Get( 1 )->GetObject());
    pVar->PutBool( m_pManager->IsLinkValid( pWrapper->GetCommunicationLink() ) );
}

void CommunicationWrapper::MSetCommunicationEventHandler( SbxVariable* /*pVar*/, SbxArray* pPar, sal_Bool /*bWrite*/ )
{ //    SetCommunicationEventHandler( String )  // Diese Funktion wird aufgerufen bei jedem Event
    m_aEventHandlerName = pPar->Get( 1 )->GetString();
}





//      Link
void CommunicationWrapper::LStopCommunication( SbxVariable* /*pVar*/, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    StopCommunication       Die Kommunikation wird abgebrochen
    m_pLink->StopCommunication();
}

void CommunicationWrapper::LGetMyName( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    String GetMyName        Der eigene Name
    pVar->PutString( UniString( m_pLink->GetMyName( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::LGetHostName( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    String GetHostName  Der Name des Anderen
    pVar->PutString( UniString( m_pLink->GetCommunicationPartner( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::LSend( SbxVariable* /*pVar*/, SbxArray* pPar, sal_Bool /*bWrite*/ )
{ //    Send(String )           String an den Partner schicken
    SvStream *pSendStream = m_pLink->GetBestCommunicationStream();
    String aSendString = pPar->Get( 1 )->GetString();
    pSendStream->WriteByteString( aSendString, RTL_TEXTENCODING_UTF8 );
    m_pLink->TransferDataStream( pSendStream );
    delete pSendStream;
}

void CommunicationWrapper::LGetString( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    String GetString        Ergebnis des letzten Empfangs
    SvStream *pReceiveStream = m_pLink->GetServiceData();
    if ( pReceiveStream )
    {
        sal_uLong nLength = pReceiveStream->Seek( STREAM_SEEK_TO_END );
        pReceiveStream->Seek( STREAM_SEEK_TO_BEGIN );
        char *pBuffer = new char[nLength];
        pReceiveStream->Read( pBuffer, nLength );
        String aReceive(
            pBuffer, sal::static_int_cast< xub_StrLen >( nLength ),
            RTL_TEXTENCODING_UTF8 );
        delete [] pBuffer;
        pVar->PutString( aReceive );
        delete pReceiveStream;
    }
    else
        pVar->PutString( UniString() );
}



// Die Factory legt unser Objekte an.

SbxObject* CommunicationFactory::CreateObject( const String& rClass )
{
    if( rClass.CompareIgnoreCaseToAscii( "CommunicationManager" ) == COMPARE_EQUAL )
        return new CommunicationWrapper( rClass );
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
