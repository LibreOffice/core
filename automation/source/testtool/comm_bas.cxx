/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: comm_bas.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:30:36 $
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

#include "comm_bas.hxx"

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <basic/sbxobj.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <basic/sbx.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <basic/sbxvar.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include "communi.hxx"
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

//#include "collelem.hxx"
// Makro MEMBER()
// #include <basic/macfix.hxx>

#if defined(MAC) || defined(HPUX)
#define MEMBER(name) &##name
#elif (defined (GCC) && ( __GNUC__ >= 3 ))
#define MEMBER(name) &name
#else
#define MEMBER(name) name
#endif


// Der CommunicationManager hat folgende Elemente:
// 1) Properties:
//    Keine
// 2) Methoden:
//    CommunicationLink StartCommunication( Host, Port )
//    StopAllCommunication      // Alle Kommunikation wird abgebrochen
//    BOOL IsCommunicationRunning       // Läuft noch irgendwas
//    String GetMyName      Der eigene Name
//    BOOL IsLinkValid( CommunicationLink )     // Ist dieser Link noch gültig
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
{ "StartCommunication",             SbxEMPTY,  2 | _FUNCTION, MEMBER(CommunicationWrapper::MStartCommunication) },
    // Zwei Named Parameter
    { "Host",SbxSTRING },
    { "Port",SbxLONG },
// Alle Kommunikation wird abgebrochen
{ "StopAllCommunication",           SbxEMPTY,  0 | _FUNCTION, MEMBER(CommunicationWrapper::MStopAllCommunication) },
// Läuft noch irgendwas
{ "IsCommunicationRunning",         SbxBOOL,   0 | _FUNCTION, MEMBER(CommunicationWrapper::MIsCommunicationRunning) },
// Hostname als FQDN erfragen
{ "GetMyName",                      SbxSTRING, 0 | _FUNCTION, MEMBER(CommunicationWrapper::MGetMyName) },
// Abfragen ob der Link überhaupt noch gültig ist
{ "IsLinkValid",                    SbxBOOL,   1 | _FUNCTION, MEMBER(CommunicationWrapper::MIsLinkValid) },
    // Ein Named Parameter
    { "Link",SbxOBJECT },
// Dieser Handler wird dauernd gerufen
{ "SetCommunicationEventHandler",   SbxEMPTY,  1 | _FUNCTION, MEMBER(CommunicationWrapper::MSetCommunicationEventHandler) },
    // Ein Named Parameter
    { "FuncName",SbxSTRING },

{ NULL,     SbxNULL,            -1 }};  // Tabellenende






CommunicationWrapper::Methods CommunicationWrapper::aLinkMethods[] = {
// Die Kommunikation wird abgebrochen
{ "StopCommunication",              SbxEMPTY,  0 | _FUNCTION, MEMBER(CommunicationWrapper::LStopCommunication) },
// Der eigene Name
{ "GetMyName",                      SbxSTRING, 0 | _FUNCTION, MEMBER(CommunicationWrapper::LGetMyName) },
// Der Name des Anderen
{ "GetHostName",                    SbxSTRING, 0 | _FUNCTION, MEMBER(CommunicationWrapper::LGetHostName) },
// String an den Partner schicken
{ "Send",                           SbxEMPTY,  1 | _FUNCTION, MEMBER(CommunicationWrapper::LSend) },
    // Ein Named Parameter
    { "SendString",SbxSTRING },
// Ergebnis des letzten Empfangs
{ "GetString",                      SbxSTRING, 0 | _FUNCTION, MEMBER(CommunicationWrapper::LGetString) },

{ NULL,     SbxNULL,            -1 }};  // Tabellenende





// Konstruktor für den Manager
CommunicationWrapper::CommunicationWrapper( const String& rClass ) : SbxObject( rClass )
, pLink( NULL )
, bIsManager( TRUE )
, bCatchOpen( FALSE )
, pNewLink( NULL )
{
//  SetName( CUniString("Manager") );
    pMethods = &aManagerMethods[0];
    pManager = new CommunicationManagerClientViaSocket;
    pManager->SetConnectionOpenedHdl( LINK( this, CommunicationWrapper, Open ) );
    pManager->SetConnectionClosedHdl( LINK( this, CommunicationWrapper, Close ) );
    pManager->SetDataReceivedHdl( LINK( this, CommunicationWrapper, Data ) );
}

// Konstruktor für den Link
CommunicationWrapper::CommunicationWrapper( CommunicationLink *pThisLink ) : SbxObject( CUniString("Link") )
, pLink( pThisLink )
, bIsManager( FALSE )
, bCatchOpen( FALSE )
, pNewLink( NULL )
{
    pMethods = &aLinkMethods[0];
    pManager = (CommunicationManagerClientViaSocket*)pThisLink->GetCommunicationManager();
}

// Destruktor
CommunicationWrapper::~CommunicationWrapper()
{
    if ( bIsManager )
        delete pManager;
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
        Methods* p = pMethods;
        short nIndex = 0;
        BOOL bFound = FALSE;
        while( p->nArgs != -1 )
        {
            if( rName.CompareIgnoreCaseToAscii( p->pName ) == COMPARE_EQUAL )
            {
                bFound = TRUE; break;
            }
            nIndex += ( p->nArgs & _ARGSMASK ) + 1;
            p = pMethods + nIndex;
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
        USHORT nIndex = (USHORT) pVar->GetUserData();
        // kein Index: weiterreichen!
        if( nIndex )
        {
            ULONG t = pHint->GetId();
            if( t == SBX_HINT_INFOWANTED )
                pVar->SetInfo( GetInfo( (short) pVar->GetUserData() ) );
            else
            {
                BOOL bWrite = FALSE;
                if( t == SBX_HINT_DATACHANGED )
                    bWrite = TRUE;
                if( t == SBX_HINT_DATAWANTED || bWrite )
                {
                    // Parameter-Test fuer Methoden:
                    USHORT nPar = pMethods[ --nIndex ].nArgs & 0x00FF;
                    // Element 0 ist der Returnwert
                    if( ( !pPar && nPar )
                     || ( pPar && pPar->Count() != nPar+1 ) )
                        SetError( SbxERR_WRONG_ARGS );
                    // Alles klar, man kann den Call ausfuehren
                    else
                    {
                        (this->*(pMethods[ nIndex ].pFunc))( pVar, pPar, bWrite );
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
    Methods* p = &pMethods[ nIdx ];
    // Wenn mal eine Hilfedatei zur Verfuegung steht:
    // SbxInfo* pInfo = new SbxInfo( Hilfedateiname, p->nHelpId );
    SbxInfo* pInfo = new SbxInfo;
    short nPar = p->nArgs & _ARGSMASK;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        String aName( p->pName, RTL_TEXTENCODING_ASCII_US );
        USHORT nFlags = ( p->nArgs >> 8 ) & 0x03;
        if( p->nArgs & _OPT )
            nFlags |= SBX_OPTIONAL;
        pInfo->AddParam( aName, p->eType, nFlags );
    }
    return pInfo;
}


////////////////////////////////////////////////////////////////////////////

// Hilfsmethoden für den Manager

IMPL_LINK( CommunicationWrapper, Open, CommunicationLink*, pLink )
{
    if ( bCatchOpen )
        pNewLink = pLink;
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
    if ( aEventHandlerName.Len() )
    {
        SbxArrayRef pPar = new SbxArray( SbxVARIANT );
        pPar->Put( new SbxVariable( SbxSTRING ), 1 );
        pPar->Get( 1 )->PutString( aType );

        pPar->Put( new SbxVariable( SbxOBJECT ), 2 );
        pPar->Get( 2 )->PutObject( new CommunicationWrapper( pLink ) );

        Call( aEventHandlerName, pPar );
    }
    else
        delete pLink->GetServiceData();     // Stream wegschmeissen um nicht zu blockieren
}


////////////////////////////////////////////////////////////////////////////

// Properties und Methoden legen beim Get (bPut = FALSE) den Returnwert
// im Element 0 des Argv ab; beim Put (bPut = TRUE) wird der Wert aus
// Element 0 gespeichert.

// Die Methoden:

// Manager
void CommunicationWrapper::MStartCommunication( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    CommunicationLink StartCommunication( Host, Port )
    bCatchOpen = TRUE;
    if ( pManager->StartCommunication( ByteString( pPar->Get( 1 )->GetString(), RTL_TEXTENCODING_UTF8 ), pPar->Get( 2 )->GetULong() ) )
    {
        while ( !pNewLink )
            GetpApp()->Reschedule();
        bCatchOpen = FALSE;
        CommunicationWrapper *pNewLinkWrapper = new CommunicationWrapper( pNewLink );
        pNewLink = NULL;
        pVar->PutObject( pNewLinkWrapper );
    }

}

void CommunicationWrapper::MStopAllCommunication( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    StopAllCommunication        // Alle Kommunikation wird abgebrochen
    pManager->StopCommunication();
}

void CommunicationWrapper::MIsCommunicationRunning( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    BOOL IsCommunicationRunning     // Läuft noch irgendwas
    pVar->PutBool( pManager->IsCommunicationRunning() );
}

void CommunicationWrapper::MGetMyName( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    String GetMyName        Der eigene Name
    pVar->PutString( UniString( pManager->GetMyName( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::MIsLinkValid( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    BOOL IsLinkValid( CommunicationLink )       // Ist dieser Link noch gültig
    CommunicationWrapper *pWrapper = (CommunicationWrapper*)(pPar->Get( 1 )->GetObject());
    pVar->PutBool( pManager->IsLinkValid( pWrapper->GetCommunicationLink() ) );
}

void CommunicationWrapper::MSetCommunicationEventHandler( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    SetCommunicationEventHandler( String )  // Diese Funktion wird aufgerufen bei jedem Event
    aEventHandlerName = pPar->Get( 1 )->GetString();
}





//      Link
void CommunicationWrapper::LStopCommunication( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    StopCommunication       Die Kommunikation wird abgebrochen
    pLink->StopCommunication();
}

void CommunicationWrapper::LGetMyName( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    String GetMyName        Der eigene Name
    pVar->PutString( UniString( pLink->GetMyName( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::LGetHostName( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    String GetHostName  Der Name des Anderen
    pVar->PutString( UniString( pLink->GetCommunicationPartner( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::LSend( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    Send(String )           String an den Partner schicken
    SvStream *pSendStream = pLink->GetBestCommunicationStream();
    String aSendString = pPar->Get( 1 )->GetString();
    pSendStream->WriteByteString( aSendString, RTL_TEXTENCODING_UTF8 );
    pLink->TransferDataStream( pSendStream );
    delete pSendStream;
}

void CommunicationWrapper::LGetString( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite )
{ //    String GetString        Ergebnis des letzten Empfangs
    SvStream *pReceiveStream = pLink->GetServiceData();
    if ( pReceiveStream )
    {
        ULONG nLength = pReceiveStream->Seek( STREAM_SEEK_TO_END );
        pReceiveStream->Seek( STREAM_SEEK_TO_BEGIN );
        char *pBuffer = new char[nLength];
        pReceiveStream->Read( pBuffer, nLength );
        String aReceive( pBuffer, nLength );
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

