/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "sbcomp.hxx"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// All symbol names are laid down int the symbol-pool's stringpool, so that
// all symbols are handled in the same case. On saving the code-image, the
// global stringpool with the respective symbols is also saved.
// The local stringpool holds all the symbols that don't move to the image
// (labels, constant names etc.).

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

const rtl::OUString& SbiStringPool::Find( sal_uInt32 n ) const
{
    if( n == 0 || n > aData.size() )
        return aEmpty; //hack, returning a reference to a simulation of null
    else
        return aData[n - 1];
}

short SbiStringPool::Add( const rtl::OUString& rVal, sal_Bool bNoCase )
{
    sal_uInt32 n = aData.size();
    for( sal_uInt32 i = 0; i < n; ++i )
    {
        rtl::OUString& p = aData[i];
        if( (  bNoCase && p == rVal )
            || ( !bNoCase && p.equalsIgnoreAsciiCase( rVal ) ) )
            return i+1;
    }

    aData.push_back(rVal);
    return (short) ++n;
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
    return Add( rtl::OUString::createFromAscii( buf ) );
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


SbiSymDef* SbiSymPool::First()
{
    nCur = (sal_uInt16) -1;
    return Next();
}

SbiSymDef* SbiSymPool::Next()
{
    if( ++nCur >= aData.size() )
        return NULL;
    else
        return aData[ nCur ];
}


SbiSymDef* SbiSymPool::AddSym( const String& rName )
{
    SbiSymDef* p = new SbiSymDef( rName );
    p->nPos    = aData.size();
    p->nId     = rStrings.Add( rName );
    p->nProcId = nProcId;
    p->pIn     = this;
    aData.insert( aData.begin() + p->nPos, p );
    return p;
}

SbiProcDef* SbiSymPool::AddProc( const String& rName )
{
    SbiProcDef* p = new SbiProcDef( pParser, rName );
    p->nPos    = aData.size();
    p->nId     = rStrings.Add( rName );
    // procs are always local
    p->nProcId = 0;
    p->pIn     = this;
    aData.insert( aData.begin() + p->nPos, p );
    return p;
}

// adding an externally constructed symbol definition

void SbiSymPool::Add( SbiSymDef* pDef )
{
    if( pDef && pDef->pIn != this )
    {
        if( pDef->pIn )
        {
#ifdef DBG_UTIL

            pParser->Error( SbERR_INTERNAL_ERROR, "Dbl Pool" );
#endif
            return;
        }

        pDef->nPos = aData.size();
        if( !pDef->nId )
        {
            // A unique name must be created in the string pool
            // for static variables (Form ProcName:VarName)
            String aName( pDef->aName );
            if( pDef->IsStatic() )
            {
                aName = pParser->aGblStrings.Find( nProcId );
                aName += ':';
                aName += pDef->aName;
            }
            pDef->nId = rStrings.Add( aName );
        }

        if( !pDef->GetProcDef() )
            pDef->nProcId = nProcId;
        pDef->pIn = this;
        aData.insert( aData.begin() + pDef->nPos, pDef );
    }
}


SbiSymDef* SbiSymPool::Find( const String& rName ) const
{
    sal_uInt16 nCount = aData.size();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        SbiSymDef* p = aData[ nCount - i - 1 ];
        if( ( !p->nProcId || ( p->nProcId == nProcId ) )
         && ( p->aName.EqualsIgnoreCaseAscii( rName ) ) )
            return p;
    }
    if( pParent )
        return pParent->Find( rName );
    else
        return NULL;
}


SbiSymDef* SbiSymPool::FindId( sal_uInt16 n ) const
{
    for( sal_uInt16 i = 0; i < aData.size(); i++ )
    {
        SbiSymDef* p = aData[ i ];
        if( p->nId == n && ( !p->nProcId || ( p->nProcId == nProcId ) ) )
            return p;
    }
    if( pParent )
        return pParent->FindId( n );
    else
        return NULL;
}

// find via position (from 0)

SbiSymDef* SbiSymPool::Get( sal_uInt16 n ) const
{
    if( n >= aData.size() )
        return NULL;
    else
        return aData[ n ];
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
    // to be sure
    pParser->aGen.GenStmnt();
    return p->Reference();
}


void SbiSymPool::CheckRefs()
{
    for( sal_uInt16 i = 0; i < aData.size(); i++ )
    {
        SbiSymDef* p = aData[ i ];
        if( !p->IsDefined() )
            pParser->Error( SbERR_UNDEF_LABEL, p->GetName() );
    }
}

/***************************************************************************
|*
|*  symbol definitions
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
    bWithBrackets =
    bByVal   =
    bChained =
    bGlobal  = false;
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


const String& SbiSymDef::GetName()
{
    if( pIn )
        aName = pIn->rStrings.Find( nId );
    return aName;
}


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

// construct a backchain, if not yet defined
// the value that shall be stored as an operand is returned

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


sal_uInt32 SbiSymDef::Define()
{
    sal_uInt32 n = pIn->pParser->aGen.GetPC();
    pIn->pParser->aGen.GenStmnt();
    if( nChain ) pIn->pParser->aGen.BackChain( nChain );
    nChain = n;
    bChained = true;
    return nChain;
}

// A symbol definition may have its own pool. This is the caseDies ist
// for objects and procedures (local variable)

SbiSymPool& SbiSymDef::GetPool()
{
    if( !pPool )
        pPool = new SbiSymPool( pIn->pParser->aGblStrings, SbLOCAL );   // is dumped
    return *pPool;
}

SbiSymScope SbiSymDef::GetScope() const
{
    return pIn ? pIn->GetScope() : SbLOCAL;
}


// The procedure definition has three pools:
// 1) aParams: is filled by the definition. Contains the
//    parameters' names, like they're used inside the body.
//    The first element is the return value.
// 2) pPool: all local variables
// 3) aLabels: labels

SbiProcDef::SbiProcDef( SbiParser* pParser, const String& rName,
                        sal_Bool bProcDecl )
         : SbiSymDef( rName )
         , aParams( pParser->aGblStrings, SbPARAM )  // is dumped
         , aLabels( pParser->aLclStrings, SbLOCAL )  // is not dumped
         , mbProcDecl( bProcDecl )
{
    aParams.SetParent( &pParser->aPublics );
    pPool = new SbiSymPool( pParser->aGblStrings, SbLOCAL );
    pPool->SetParent( &aParams );
    nLine1  =
    nLine2  = 0;
    mePropMode = PROPERTY_MODE_NONE;
    bPublic = true;
    bCdecl  = sal_False;
    bStatic = sal_False;
    // For return values the first element of the parameter
    // list is always defined with name and type of the proc
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

// match with a forward-declaration
// if the match is OK, pOld is replaced by this in the pool
// pOld is deleted in any case!

void SbiProcDef::Match( SbiProcDef* pOld )
{
    SbiSymDef* po, *pn=NULL;
    // parameter 0 is the function name
    sal_uInt16 i;
    for( i = 1; i < aParams.GetSize(); i++ )
    {
        po = pOld->aParams.Get( i );
        pn = aParams.Get( i );
        // no type matching - that is done during running
        // but is it maybe called with too little parameters?
        if( !po && !pn->IsOptional() && !pn->IsParamArray() )
            break;
        po = pOld->aParams.Next();
    }

    if( pn && i < aParams.GetSize() && pOld->pIn )
    {
        // mark the whole line
        pOld->pIn->GetParser()->SetCol1( 0 );
        pOld->pIn->GetParser()->Error( SbERR_BAD_DECLARATION, aName );
    }
    if( !pIn && pOld->pIn )
    {
        // Replace old entry with the new one
        pOld->pIn->aData[ pOld->nPos ] = this;
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
                OSL_FAIL( "Illegal PropertyMode PROPERTY_MODE_NONE" );
                break;
        }
        aCompleteProcName += aName;
        aName = aCompleteProcName;
    }
}



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

SbiSymbols::~SbiSymbols()
{
    for( const_iterator it = begin(); it != end(); ++it )
        delete *it;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
