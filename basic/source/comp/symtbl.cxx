/*************************************************************************
 *
 *  $RCSfile: symtbl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
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

#include "sbcomp.hxx"
#pragma hdrstop
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

SV_IMPL_PTRARR(SbiStrings,String*)
SV_IMPL_PTRARR(SbiSymbols,SbiSymDef*)

// Alle Symbolnamen werden im Stringpool des Symbol-Pools abgelegt, damit
// alle Symbole im gleichen Case verarbeitet werden. Beim Speichern des
// Code-Images wird der globale Stringpool mit den entsprechenden Sympools
// gespeichert. Der lokale Stringpool nimmt alle Symbole auf, die nicht
// ins Image wandern (Labels, Konstantennamen etc).

/***************************************************************************
|*
|*  SbiStringPool
|*
***************************************************************************/

SbiStringPool::SbiStringPool( SbiParser* p )
{
    pParser = p;
}

SbiStringPool::~SbiStringPool()
{}

// Suchen

const String& SbiStringPool::Find( USHORT n ) const
{
    if( !n || n > aData.Count() )
        return aEmpty;
    else
        return *aData.GetObject( n-1 );
}

// Hinzufuegen eines Strings. Der String wird Case-Insensitiv
// verglichen.

short SbiStringPool::Add( const String& rVal, BOOL bNoCase )
{
    USHORT n = aData.Count();
    for( USHORT i = 0; i < n; i++ )
    {
        String* p = aData.GetObject( i );
        if( (  bNoCase && p->Equals( rVal ) )
         || ( !bNoCase && p->EqualsIgnoreCaseAscii( rVal ) ) )
            return i+1;
    }
    const String* pNew = new String( rVal );
    aData.Insert( pNew, n++ );
    return (short) n;
}

short SbiStringPool::Add( double n, SbxDataType t )
{
    char buf[ 40 ];
    switch( t )
    {
        case SbxINTEGER: sprintf( buf, "%d", (short) n ); break;
        case SbxLONG:    sprintf( buf, "%ld", (long) n ); break;
        case SbxSINGLE:  sprintf( buf, "%.6g", (float) n ); break;
        case SbxDOUBLE:  sprintf( buf, "%.16g", n ); break;
    }
    return Add( String::CreateFromAscii( buf ) );
}

/***************************************************************************
|*
|*  SbiSymPool
|*
***************************************************************************/

SbiSymPool::SbiSymPool( SbiStringPool& r, SbiSymScope s ) : rStrings( r )
{
    pParser  = r.GetParser();
    eScope   = s;
    pParent  = NULL;
    nCur     =
    nProcId  = 0;
}

SbiSymPool::~SbiSymPool()
{}

// Inhalt loeschen

void SbiSymPool::Clear()
{
    aData.DeleteAndDestroy( 0, aData.Count() );
}

SbiSymDef* SbiSymPool::First()
{
    nCur = (USHORT) -1;
    return Next();
}

SbiSymDef* SbiSymPool::Next()
{
    if( ++nCur >= aData.Count() )
        return NULL;
    else
        return aData.GetObject( nCur );
}

// Hinzufuegen eines Symbols

SbiSymDef* SbiSymPool::AddSym( const String& rName )
{
    SbiSymDef* p = new SbiSymDef( rName );
    p->nPos    = aData.Count();
    p->nId     = rStrings.Add( rName );
    p->nProcId = nProcId;
    p->pIn     = this;
    const SbiSymDef* q = p;
    aData.Insert( q, q->nPos );
    return p;
}

SbiProcDef* SbiSymPool::AddProc( const String& rName )
{
    SbiProcDef* p = new SbiProcDef( pParser, rName );
    p->nPos    = aData.Count();
    p->nId     = rStrings.Add( rName );
    // Procs sind immer global
    p->nProcId = 0;
    p->pIn     = this;
    const SbiSymDef* q = p;
    aData.Insert( q, q->nPos );
    return p;
}

// Hinzufuegen einer extern aufgebauten Symboldefinition

void SbiSymPool::Add( SbiSymDef* pDef )
{
    if( pDef && pDef->pIn != this )
    {
        if( pDef->pIn )
        {
#ifndef PRODUCT
            // schon in einem anderen Pool drin!
            pParser->Error( SbERR_INTERNAL_ERROR, "Dbl Pool" );
#endif
            return;
        }

        pDef->nPos = aData.Count();
        if( !pDef->nId )
        {
            // Bei statischen Variablen muss ein eindeutiger Name
            // im Stringpool erzeugt werden (Form ProcName:VarName)
            String aName( pDef->aName );
            if( pDef->IsStatic() )
            {
                aName = pParser->aGblStrings.Find( nProcId );
                aName += ':';
                aName += pDef->aName;
            }
            pDef->nId = rStrings.Add( aName );
        }
        // Procs sind immer global
        if( !pDef->GetProcDef() )
            pDef->nProcId = nProcId;
        pDef->pIn = this;
        const SbiSymDef* q = pDef;
        aData.Insert( q, q->nPos );
    }
}

// Suchen eines Eintrags ueber den Namen. Es wird auch im Parent gesucht.

SbiSymDef* SbiSymPool::Find( const String& rName ) const
{
    for( USHORT i = 0; i < aData.Count(); i++ )
    {
        SbiSymDef* p = aData.GetObject( i );
        if( ( !p->nProcId || ( p->nProcId == nProcId ) )
         && ( p->aName.EqualsIgnoreCaseAscii( rName ) ) )
            return p;
    }
    if( pParent )
        return pParent->Find( rName );
    else
        return NULL;
}

// Suchen ueber ID-Nummer

SbiSymDef* SbiSymPool::FindId( USHORT n ) const
{
    for( USHORT i = 0; i < aData.Count(); i++ )
    {
        SbiSymDef* p = aData.GetObject( i );
        if( p->nId == n && ( !p->nProcId || ( p->nProcId == nProcId ) ) )
            return p;
    }
    if( pParent )
        return pParent->FindId( n );
    else
        return NULL;
}

// Suchen ueber Position (ab 0)

SbiSymDef* SbiSymPool::Get( USHORT n ) const
{
    if( n >= aData.Count() )
        return NULL;
    else
        return aData.GetObject( n );
}

USHORT SbiSymPool::Define( const String& rName )
{
    SbiSymDef* p = Find( rName );
    if( p )
    {   if( p->IsDefined() )
            pParser->Error( SbERR_LABEL_DEFINED, rName );
    }
    else
        p = AddSym( rName );
    return p->Define();
}

USHORT SbiSymPool::Reference( const String& rName )
{
    SbiSymDef* p = Find( rName );
    if( !p )
        p = AddSym( rName );
    //Sicherheitshalber
    pParser->aGen.GenStmnt();
    return p->Reference();
}

// Alle offenen Referenzen anmaulen

void SbiSymPool::CheckRefs()
{
    for( USHORT i = 0; i < aData.Count(); i++ )
    {
        SbiSymDef* p = aData.GetObject( i );
        if( !p->IsDefined() )
            pParser->Error( SbERR_UNDEF_LABEL, p->GetName() );
    }
}

/***************************************************************************
|*
|*  Symbol-Definitionen
|*
***************************************************************************/

SbiSymDef::SbiSymDef( const String& rName ) : aName( rName )
{
    eType    = SbxEMPTY;
    nDims    =
    nTypeId  =
    nProcId  =
    nId      =
    nPos     =
    nLen     =
    nChain   = 0;
    bAs      =
    bNew     =
    bStatic  =
    bOpt     =
    bByVal   =
    bChained = FALSE;
    pIn      =
    pPool    = NULL;
}

SbiSymDef::~SbiSymDef()
{
    delete pPool;
}

SbiProcDef* SbiSymDef::GetProcDef()
{
    return NULL;
}

SbiConstDef* SbiSymDef::GetConstDef()
{
    return NULL;
}

// Wenn der Name benoetigt wird, den aktuellen Namen
// aus dem Stringpool nehmen

const String& SbiSymDef::GetName()
{
    if( pIn )
        aName = pIn->rStrings.Find( nId );
    return aName;
}

// Eintragen eines Datentyps

void SbiSymDef::SetType( SbxDataType t )
{
    if( t == SbxVARIANT && pIn )
    {
        char ch = (char)aName.GetBuffer()[0];
        if( ch == '_' ) ch = 'Z';
        ch = toupper( ch );
        t = pIn->pParser->eDefTypes[ ch - 'A' ];
    }
    eType = t;
}

// Aufbau einer Backchain, falls noch nicht definiert
// Es wird der Wert zurueckgeliefert, der als Operand gespeichert
// werden soll.

USHORT SbiSymDef::Reference()
{
    if( !bChained )
    {
        USHORT n = nChain;
        nChain = pIn->pParser->aGen.GetOffset();
        return n;
    }
    else return nChain;
}

// Definition eines Symbols.
// Hier wird der Backchain aufgeloest, falls vorhanden

USHORT SbiSymDef::Define()
{
    USHORT n = pIn->pParser->aGen.GetPC();
    pIn->pParser->aGen.GenStmnt();
    if( nChain ) pIn->pParser->aGen.BackChain( nChain );
    nChain = n;
    bChained = TRUE;
    return nChain;
}

// Eine Symboldefinition kann einen eigenen Pool haben. Dies ist
// der Fall bei Objekten und Prozeduren (lokale Variable)

SbiSymPool& SbiSymDef::GetPool()
{
    if( !pPool )
        pPool = new SbiSymPool( pIn->pParser->aGblStrings, SbLOCAL );   // wird gedumpt
    return *pPool;
}

SbiSymScope SbiSymDef::GetScope() const
{
    return pIn ? pIn->GetScope() : SbLOCAL;
}

////////////////////////////////////////////////////////////////////////////

// Die Prozedur-Definition hat drei Pools:
// 1) aParams: wird durch die Definition gefuellt. Enthaelt die Namen
//    der Parameter, wie sie innerhalb des Rumpfes verwendet werden.
//    Das erste Element ist der Returnwert.
// 2) pPool: saemtliche lokale Variable
// 3) aLabels: Labels

SbiProcDef::SbiProcDef( SbiParser* pParser, const String& rName )
         : SbiSymDef( rName ),
           aParams( pParser->aGblStrings, SbPARAM ), // wird gedumpt
           aLabels( pParser->aLclStrings, SbLOCAL )  // wird nicht gedumpt
{
    aParams.SetParent( &pParser->aPublics );
    pPool = new SbiSymPool( pParser->aGblStrings, SbLOCAL ); // Locals
    pPool->SetParent( &aParams );
    nLine1  =
    nLine2  = 0;
    bPublic = TRUE;
    bCdecl  = FALSE;
    // Fuer Returnwerte ist das erste Element der Parameterliste
    // immer mit dem Namen und dem Typ der Proc definiert
    aParams.AddSym( aName );
}

SbiProcDef::~SbiProcDef()
{}

SbiProcDef* SbiProcDef::GetProcDef()
{
    return this;
}

void SbiProcDef::SetType( SbxDataType t )
{
    SbiSymDef::SetType( t );
    aParams.Get( 0 )->SetType( eType );
}

// Match mit einer Forward-Deklaration
// Falls der Match OK ist, wird pOld durch this im Pool ersetzt
// pOld wird immer geloescht!

void SbiProcDef::Match( SbiProcDef* pOld )
{
    SbiSymDef* po, *pn;
    // Parameter 0 ist der Funktionsname
    USHORT i;
    for( i = 1; i < aParams.GetSize(); i++ )
    {
        po = pOld->aParams.Get( i );
        pn = aParams.Get( i );
        // Kein Typabgleich; das wird beim Laufen erledigt
        // aber ist sie evtl. mit zu wenigen Parametern aufgerufen
        // worden?
        if( !po && !pn->IsOptional() )
            break;
        po = pOld->aParams.Next();
    }
    // Wurden zu viele Parameter angegeben?
    if( pn && i < aParams.GetSize() && pOld->pIn )
    {
        // Die ganze Zeile markieren
        pOld->pIn->GetParser()->SetCol1( 0 );
        pOld->pIn->GetParser()->Error( SbERR_BAD_DECLARATION, aName );
    }
    if( !pIn && pOld->pIn )
    {
        // Alten Eintrag durch neuen ersetzen
        SbiSymDef** pData = (SbiSymDef**) pOld->pIn->aData.GetData();
        pData[ pOld->nPos ] = this;
        nPos = pOld->nPos;
        nId  = pOld->nId;
        pIn  = pOld->pIn;
    }
    delete pOld;
}

//////////////////////////////////////////////////////////////////////////

SbiConstDef::SbiConstDef( const String& rName )
           : SbiSymDef( rName )
{
    nVal = 0; eType = SbxINTEGER;
}

void SbiConstDef::Set( double n, SbxDataType t )
{
    aVal.Erase(); nVal = n; eType = t;
}

void SbiConstDef::Set( const String& n )
{
    aVal = n; nVal = 0; eType = SbxSTRING;
}

SbiConstDef::~SbiConstDef()
{}

SbiConstDef* SbiConstDef::GetConstDef()
{
    return this;
}

