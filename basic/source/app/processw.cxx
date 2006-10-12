/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: processw.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 14:23:20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"


#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <sbxobj.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <sbx.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <sbxvar.hxx>
#endif

//#include <osl/thread.h>
#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif

#include "processw.hxx"

// Der Process hat folgende Elemente:
// 1) Properties:
//    Keine
// 2) Methoden:
//    SetImage( Filename )
//    BOOL Start
//    USHORT GetExitCode
//    BOOL IsRunning
//    BOOL WasGPF


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



ProcessWrapper::Methods ProcessWrapper::aProcessMethods[] = {
// Imagedatei des Executables
{ "SetImage", SbxEMPTY, &ProcessWrapper::PSetImage, 1 | _FUNCTION },
    // Zwei Named Parameter
    { "Filename", SbxSTRING, NULL, 0 },
    { "Params", SbxSTRING, NULL, _OPT },
// Programm wird gestartet
{ "Start", SbxBOOL, &ProcessWrapper::PStart, 0 | _FUNCTION },
// ExitCode des Programms(nachdem es beendet ist)
{ "GetExitCode", SbxULONG, &ProcessWrapper::PGetExitCode, 0 | _FUNCTION },
// Programm läuft noch
{ "IsRunning", SbxBOOL, &ProcessWrapper::PIsRunning, 0 | _FUNCTION },
// Programm mit GPF o.ä. abgebrochen
{ "WasGPF", SbxBOOL, &ProcessWrapper::PWasGPF, 0 | _FUNCTION },

{ NULL, SbxNULL, NULL, -1 }};  // Tabellenende



// Konstruktor für den Process
ProcessWrapper::ProcessWrapper() : SbxObject( CUniString("Process") )
{
    pProcess = new Process();
    SetName( CUniString("Process") );
    pMethods = &aProcessMethods[0];
}

// Destruktor
ProcessWrapper::~ProcessWrapper()
{
    delete pProcess;
}

// Suche nach einem Element:
// Hier wird linear durch die Methodentabelle gegangen, bis eine
// passende Methode gefunden wurde.
// Wenn die Methode/Property nicht gefunden wurde, nur NULL ohne
// Fehlercode zurueckliefern, da so auch eine ganze Chain von
// Objekten nach der Methode/Property befragt werden kann.

SbxVariable* ProcessWrapper::Find( const String& rName, SbxClassType t )
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
            if( rName.EqualsIgnoreCaseAscii( p->pName ) )
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
            String aMethodName( p->pName, RTL_TEXTENCODING_ASCII_US );
            SbxClassType eCT = SbxCLASS_OBJECT;
            if( nType & _PROPERTY )
                eCT = SbxCLASS_PROPERTY;
            else if( nType & _METHOD )
                eCT = SbxCLASS_METHOD;
            pRes = Make( aMethodName, eCT, p->eType );
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

void ProcessWrapper::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCT,
                             const SfxHint& rHint, const TypeId& rHT )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pNotifyPar = pVar->GetParameters();
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
                    if( ( !pNotifyPar && nPar )
                     || ( pNotifyPar && pNotifyPar->Count() < nPar+1 ) )
                        SetError( SbxERR_WRONG_ARGS );
                    // Alles klar, man kann den Call ausfuehren
                    else
                    {
                        (this->*(pMethods[ nIndex ].pFunc))( pVar, pNotifyPar, bWrite );
                    }
                }
            }
        }
        SbxObject::SFX_NOTIFY( rBC, rBCT, rHint, rHT );
    }
}

// Zusammenbau der Infostruktur fuer einzelne Elemente

SbxInfo* ProcessWrapper::GetInfo( short nIdx )
{
    Methods* p = &pMethods[ nIdx ];
    // Wenn mal eine Hilfedatei zur Verfuegung steht:
    // SbxInfo* pResultInfo = new SbxInfo( Hilfedateiname, p->nHelpId );
    SbxInfo* pResultInfo = new SbxInfo;
    short nPar = p->nArgs & _ARGSMASK;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        String aMethodName( p->pName, RTL_TEXTENCODING_ASCII_US );
        USHORT nInfoFlags = ( p->nArgs >> 8 ) & 0x03;
        if( p->nArgs & _OPT )
            nInfoFlags |= SBX_OPTIONAL;
        pResultInfo->AddParam( aMethodName, p->eType, nInfoFlags );
    }
    return pResultInfo;
}


////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////

// Properties und Methoden legen beim Get (bPut = FALSE) den Returnwert
// im Element 0 des Argv ab; beim Put (bPut = TRUE) wird der Wert aus
// Element 0 gespeichert.

// Die Methoden:
void ProcessWrapper::PSetImage( SbxVariable* pVar, SbxArray* pMethodePar, BOOL bWriteIt )
{ // Imagedatei des Executables
    (void) pVar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    if ( pMethodePar->Count() >= 2 )
        pProcess->SetImage(pMethodePar->Get( 1 )->GetString(), pMethodePar->Get( 2 )->GetString() );
    else
        pProcess->SetImage(pMethodePar->Get( 1 )->GetString(), String() );
}

void ProcessWrapper::PStart( SbxVariable* pVar, SbxArray* pMethodePar, BOOL bWriteIt )
{ // Programm wird gestartet
    (void) pMethodePar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    pVar->PutBool( pProcess->Start() );
}

void ProcessWrapper::PGetExitCode( SbxVariable* pVar, SbxArray* pMethodePar, BOOL bWriteIt )
{ // ExitCode des Programms(nachdem es beendet ist)
    (void) pMethodePar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    pVar->PutULong( pProcess->GetExitCode() );
}

void ProcessWrapper::PIsRunning( SbxVariable* pVar, SbxArray* pMethodePar, BOOL bWriteIt )
{ // Programm läuft noch
    (void) pMethodePar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    pVar->PutBool( pProcess->IsRunning() );
}

void ProcessWrapper::PWasGPF( SbxVariable* pVar, SbxArray* pMethodePar, BOOL bWriteIt )
{ // Programm mit GPF o.ä. abgebrochen
    (void) pMethodePar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    pVar->PutBool( pProcess->WasGPF() );
}






// Die Factory legt unser Objekte an.

SbxObject* ProcessFactory::CreateObject( const String& rClass )
{
    if( rClass.CompareIgnoreCaseToAscii( "Process" ) == COMPARE_EQUAL )
        return new ProcessWrapper();
    return NULL;
}

