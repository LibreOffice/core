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
#include "precompiled_basic.hxx"

#include "sbcomp.hxx"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

const String& SbiStringPool::Find( sal_uInt16 n ) const
{
    if( !n || n > aData.Count() )
        return aEmpty;
    else
        return *aData.GetObject( n-1 );
}

// Hinzufuegen eines Strings. Der String wird Case-Insensitiv
// verglichen.

short SbiStringPool::Add( const String& rVal, sal_Bool bNoCase )
{
    sal_uInt16 n = aData.Count();
    for( sal_uInt16 i = 0; i < n; i++ )
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
        case SbxINTEGER: snprintf( buf, sizeof(buf), "%d", (short) n ); break;
        case SbxLONG:    snprintf( buf, sizeof(buf), "%ld", (long) n ); break;
        case SbxSINGLE:  snprintf( buf, sizeof(buf), "%.6g", (float) n ); break;
        case SbxDOUBLE:  snprintf( buf, sizeof(buf), "%.16g", n ); break;
        default: break;
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
    nCur = (sal_uInt16) -1;
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
#ifdef DBG_UTIL
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
    sal_uInt16 nCount = aData.Count();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        SbiSymDef* p = aData.GetObject( nCount - i - 1 );
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

SbiSymDef* SbiSymPool::FindId( sal_uInt16 n ) const
{
    for( sal_uInt16 i = 0; i < aData.Count(); i++ )
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

SbiSymDef* SbiSymPool::Get( sal_uInt16 n ) const
{
    if( n >= aData.Count() )
        return NULL;
    else
        return aData.GetObject( n );
}

sal_uInt32 SbiSymPool::Define( const String& rName )
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

sal_uInt32 SbiSymPool::Reference( const String& rName )
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
    for( sal_uInt16 i = 0; i < aData.Count(); i++ )
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
    nDims    = 0;
    nTypeId  = 0;
    nProcId  = 0;
    nId      = 0;
    nPos     = 0;
    nLen     = 0;
    nChain   = 0;
    bAs      =
    bNew     =
    bStatic  =
    bOpt     =
    bParamArray =
    bWithEvents =
    bByVal   =
    bChained =
    bGlobal  = sal_False;
    pIn      =
    pPool    = NULL;
    nDefaultId = 0;
    nFixedStringLength = -1;
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
        sal_Unicode cu = aName.GetBuffer()[0];
        if( cu < 256 )
        {
            char ch = (char)aName.GetBuffer()[0];
            if( ch == '_' ) ch = 'Z';
            int ch2 = toupper( ch );
            unsigned char c = (unsigned char)ch2;
            if( c > 0 && c < 128 )
                t = pIn->pParser->eDefTypes[ ch2 - 'A' ];
        }
    }
    eType = t;
}

// Aufbau einer Backchain, falls noch nicht definiert
// Es wird der Wert zurueckgeliefert, der als Operand gespeichert
// werden soll.

sal_uInt32 SbiSymDef::Reference()
{
    if( !bChained )
    {
        sal_uInt32 n = nChain;
        nChain = pIn->pParser->aGen.GetOffset();
        return n;
    }
    else return nChain;
}

// Definition eines Symbols.
// Hier wird der Backchain aufgeloest, falls vorhanden

sal_uInt32 SbiSymDef::Define()
{
    sal_uInt32 n = pIn->pParser->aGen.GetPC();
    pIn->pParser->aGen.GenStmnt();
    if( nChain ) pIn->pParser->aGen.BackChain( nChain );
    nChain = n;
    bChained = sal_True;
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

SbiProcDef::SbiProcDef( SbiParser* pParser, const String& rName,
                        sal_Bool bProcDecl )
         : SbiSymDef( rName )
         , aParams( pParser->aGblStrings, SbPARAM )  // wird gedumpt
         , aLabels( pParser->aLclStrings, SbLOCAL )  // wird nicht gedumpt
         , mbProcDecl( bProcDecl )
{
    aParams.SetParent( &pParser->aPublics );
    pPool = new SbiSymPool( pParser->aGblStrings, SbLOCAL ); // Locals
    pPool->SetParent( &aParams );
    nLine1  =
    nLine2  = 0;
    mePropMode = PROPERTY_MODE_NONE;
    bPublic = sal_True;
    bCdecl  = sal_False;
    bStatic = sal_False;
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
    SbiSymDef* po, *pn=NULL;
    // Parameter 0 ist der Funktionsname
    sal_uInt16 i;
    for( i = 1; i < aParams.GetSize(); i++ )
    {
        po = pOld->aParams.Get( i );
        pn = aParams.Get( i );
        // Kein Typabgleich; das wird beim Laufen erledigt
        // aber ist sie evtl. mit zu wenigen Parametern aufgerufen
        // worden?
        if( !po && !pn->IsOptional() && !pn->IsParamArray() )
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

void SbiProcDef::setPropertyMode( PropertyMode ePropMode )
{
    mePropMode = ePropMode;
    if( mePropMode != PROPERTY_MODE_NONE )
    {
        // Prop name = original scanned procedure name
        maPropName = aName;

        // CompleteProcName includes "Property xxx "
        // to avoid conflicts with other symbols
        String aCompleteProcName;
        aCompleteProcName.AppendAscii( "Property " );
        switch( mePropMode )
        {
            case PROPERTY_MODE_GET:     aCompleteProcName.AppendAscii( "Get " ); break;
            case PROPERTY_MODE_LET:     aCompleteProcName.AppendAscii( "Let " ); break;
            case PROPERTY_MODE_SET:     aCompleteProcName.AppendAscii( "Set " ); break;
            case PROPERTY_MODE_NONE:
                DBG_ERROR( "Illegal PropertyMode PROPERTY_MODE_NONE" );
                break;
        }
        aCompleteProcName += aName;
        aName = aCompleteProcName;
    }
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

