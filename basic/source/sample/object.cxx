/*************************************************************************
 *
 *  $RCSfile: object.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <svtools/sbxobj.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#pragma hdrstop
#include "object.hxx"
#include "collelem.hxx"
// Makro MEMBER()
#include "macfix.hxx"

// Das Sample-Objekt hat folgende Elemente:
// 1) Properties:
//    Name      der Name
//    Value     ein double-Wert, beide bereits als Default drin
// 2) Methoden:
//    Create    Erzeugen eines neuen Unterelements
//    Display   Ausgabe eines Textes
//    Square    Argument * Argument
//    Event     Aufruf eines Basic-Eventhandlers
// 3) Unterobjekte:
//    Per Create() kann ein neues Unterelement eingerichtet werden,
//    das indiziert werden kann, falls mehrere Objekte gleichen Namens
//    existieren.
// Diese Implementation ist ein Beispiel fuer eine tabellengesteuerte
// Version, die sehr viele Elemente enthalten kann. Die Elemente werden
// je nach Bedarf aus der Tabelle in das Objekt uebernommen.
// Die Collection findet sich in COLLECTN.*, die in der Collection
// enthaltenen Objekte in COLLELEM.*

// Das Sample-Objekt wird in ..\app\mybasic.cxx wie folgt in StarBASIC
// eingebaut:

// MyBasic::MyBasic() : StarBASIC()
// {
//      AddFactory( new SampleObjectFac() );
// }

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

SampleObject::Methods SampleObject::aMethods[] = {
// Eine Sample-Methode (der Returnwert ist SbxNULL)
{ "Display",    SbxEMPTY,       1 | _FUNCTION, MEMBER(SampleObject::Display) },
    // Ein Named Parameter
    { "message",SbxSTRING },
// Eine Sample-Funktion
{ "Square",     SbxDOUBLE,      1 | _FUNCTION, MEMBER(SampleObject::Square)  },
    // Ein Named Parameter
    { "value",  SbxDOUBLE },
//  Basic-Callback
{ "Event",      SbxEMPTY,       1 | _FUNCTION, MEMBER(SampleObject::Event)   },
    // Ein Named Parameter
    { "event",  SbxSTRING },
//  Element erzeugen
{ "Create",     SbxEMPTY,       1 | _FUNCTION, MEMBER(SampleObject::Create)  },
    // Ein Named Parameter
    { "name",  SbxSTRING },

{ NULL,     SbxNULL,            -1 }};  // Tabellenende

SampleObject::SampleObject( const String& rClass ) : SbxObject( rClass )
{
    SetName( String( RTL_CONSTASCII_USTRINGPARAM("Sample") ) );
    PutDouble( 1.0 );   // Startwert fuer Value
}

// Suche nach einem Element:
// Hier wird linear durch die Methodentabelle gegangen, bis eine
// passende Methode gefunden wurde.
// Wenn die Methode/Property nicht gefunden wurde, nur NULL ohne
// Fehlercode zurueckliefern, da so auch eine ganze Chain von
// Objekten nach der Methode/Property befragt werden kann.

SbxVariable* SampleObject::Find( const String& rName, SbxClassType t )
{
    // Ist das Element bereits vorhanden?
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( !pRes && t != SbxCLASS_OBJECT )
    {
        // sonst suchen
        Methods* p = aMethods;
        short nIndex = 0;
        BOOL bFound = FALSE;
        while( p->nArgs != -1 )
        {
            if( rName.EqualsIgnoreCaseAscii( p->pName ) )
            {
                bFound = TRUE; break;
            }
            nIndex += ( p->nArgs & _ARGSMASK ) + 1;
            p = aMethods + nIndex;
        }
        if( bFound )
        {
            // Args-Felder isolieren:
            short nAccess = ( p->nArgs & _RWMASK ) >> 8;
            short nType   = ( p->nArgs & _TYPEMASK );
            String aName = String::CreateFromAscii( p->pName );
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

void SampleObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCT,
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
                    USHORT nPar = aMethods[ --nIndex ].nArgs & 0x00FF;
                    // Element 0 ist der Returnwert
                    if( ( !pPar && nPar )
                     || ( pPar->Count() != nPar+1 ) )
                        SetError( SbxERR_WRONG_ARGS );
                    // Alles klar, man kann den Call ausfuehren
                    else
                    {
                        (this->*(aMethods[ nIndex ].pFunc))( pVar, pPar, bWrite );
                    }
                }
            }
        }
        SbxObject::SFX_NOTIFY( rBC, rBCT, rHint, rHT );
    }
}

// Zusammenbau der Infostruktur fuer einzelne Elemente

SbxInfo* SampleObject::GetInfo( short nIdx )
{
    Methods* p = &aMethods[ nIdx ];
    // Wenn mal eine Hilfedatei zur Verfuegung steht:
    // SbxInfo* pInfo = new SbxInfo( Hilfedateiname, p->nHelpId );
    SbxInfo* pInfo = new SbxInfo;
    short nPar = p->nArgs & _ARGSMASK;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        String aName = String::CreateFromAscii( p->pName );
        USHORT nFlags = ( p->nArgs >> 8 ) & 0x03;
        if( p->nArgs & _OPT )
            nFlags |= SBX_OPTIONAL;
        pInfo->AddParam( aName, p->eType, nFlags );
    }
    return pInfo;
}

////////////////////////////////////////////////////////////////////////////

// Properties und Methoden legen beim Get (bPut = FALSE) den Returnwert
// im Element 0 des Argv ab; beim Put (bPut = TRUE) wird der Wert aus
// Element 0 gespeichert.

// Die Methoden:

void SampleObject::Display( SbxVariable*, SbxArray* pPar, BOOL )
{
    // GetString() loest u.U. auch einen Error aus!
    String s( pPar->Get( 1 )->GetString() );
    if( !IsError() )
        InfoBox( NULL, s ).Execute();
}

void SampleObject::Square( SbxVariable* pVar, SbxArray* pPar, BOOL )
{
    double n = pPar->Get( 1 )->GetDouble();
    pVar->PutDouble( n * n );
}

// Callback nach BASIC:

void SampleObject::Event( SbxVariable*, SbxArray* pPar, BOOL )
{
    Call( pPar->Get( 1 )->GetString(), NULL );
}

// Neues Element anlegen

void SampleObject::Create( SbxVariable* pVar, SbxArray* pPar, BOOL )
{
    pVar->PutObject(
        MakeObject( pPar->Get( 1 )->GetString(), String( RTL_CONSTASCII_USTRINGPARAM("SampleElement") ) ) );
}

// Die Factory legt unsere beiden Objekte an.

SbxObject* SampleObjectFac::CreateObject( const String& rClass )
{
    if( rClass.EqualsIgnoreCaseAscii( "SampleObject" ) )
        return new SampleObject( rClass );
    if( rClass.EqualsIgnoreCaseAscii( "SampleElement" ) )
        return new SampleElement( rClass );
    return NULL;
}

