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


#include <memory>
#include <parser.hxx>

#include <osl/diagnose.h>

#include <stdio.h>
#include <rtl/character.hxx>
#include <basic/sberrors.hxx>
#include <utility>

// All symbol names are laid down int the symbol-pool's stringpool, so that
// all symbols are handled in the same case. On saving the code-image, the
// global stringpool with the respective symbols is also saved.
// The local stringpool holds all the symbols that don't move to the image
// (labels, constant names etc.).

SbiStringPool::SbiStringPool( )
{}

SbiStringPool::~SbiStringPool()
{}

OUString SbiStringPool::Find( sal_uInt32 n ) const
{
    if( n == 0 || n > aData.size() )
        return OUString();
    else
        return aData[n - 1];
}

short SbiStringPool::Add( const OUString& rVal )
{
    sal_uInt32 n = aData.size();
    for( sal_uInt32 i = 0; i < n; ++i )
    {
        OUString& p = aData[i];
        if( p == rVal )
            return i+1;
    }

    aData.push_back(rVal);
    return static_cast<short>(++n);
}

short SbiStringPool::Add(double n, SbxDataType t)
{
    size_t size = 0;
    const size_t aBufLength = 40;
    char buf[aBufLength]{};

    // tdf#143707 - add the type character after the null termination of the string in order to
    // keep compatibility. After the type character has been added, the buffer contains the value
    // of the double n, the string termination symbol, and the type character.
    switch( t )
    {
        // tdf#142460 - properly handle boolean values in string pool
        case SbxBOOL:
            size = snprintf(buf, sizeof(buf), "%d", static_cast<short>(n)) + 1;
            buf[size++] = 'b';
            break;
        // tdf#131296 - store numeric value including its type character
        // See GetSuffixType in basic/source/comp/scanner.cxx for type characters
        case SbxINTEGER:
            size = snprintf(buf, sizeof(buf), "%d", static_cast<short>(n)) + 1;
            buf[size++] = '%';
            break;
        case SbxLONG:
            size = snprintf(buf, sizeof(buf), "%" SAL_PRIdINT32, static_cast<sal_Int32>(n)) + 1;
            buf[size++] = '&';
            break;
        case SbxSINGLE:
            size = snprintf(buf, sizeof(buf), "%.6g", static_cast<float>(n)) + 1;
            buf[size++] = '!';
            break;
        case SbxDOUBLE:
            size = snprintf(buf, sizeof(buf), "%.16g", n) + 1;
            buf[size++] = '#';
            break;
        case SbxCURRENCY:
            size = snprintf(buf, sizeof(buf), "%.16g", n) + 1;
            buf[size++] = '@';
            break;
        default: assert(false); break; // should not happen
    }

    // tdf#143707 - add the content of the buffer to the string pool including its calculated length
    return Add(OUString::fromUtf8(std::string_view(buf, size)));
}

SbiSymPool::SbiSymPool( SbiStringPool& r, SbiSymScope s, SbiParser* pP ) :
    rStrings(r),
    pParent(nullptr),
    pParser(pP),
    eScope(s),
    nProcId(0),
    nCur(0)
{
}

SbiSymPool::~SbiSymPool()
{}


SbiSymDef* SbiSymPool::First()
{
    nCur = sal_uInt16(-1);
    return Next();
}

SbiSymDef* SbiSymPool::Next()
{
    if (m_Data.size() <= ++nCur)
        return nullptr;
    else
        return m_Data[ nCur ].get();
}


SbiSymDef* SbiSymPool::AddSym( const OUString& rName )
{
    SbiSymDef* p = new SbiSymDef( rName );
    p->nPos    = m_Data.size();
    p->nId     = rStrings.Add( rName );
    p->nProcId = nProcId;
    p->pIn     = this;
    m_Data.insert( m_Data.begin() + p->nPos, std::unique_ptr<SbiSymDef>(p) );
    return p;
}

SbiProcDef* SbiSymPool::AddProc( const OUString& rName )
{
    SbiProcDef* p = new SbiProcDef( pParser, rName );
    p->nPos    = m_Data.size();
    p->nId     = rStrings.Add( rName );
    // procs are always local
    p->nProcId = 0;
    p->pIn     = this;
    m_Data.insert( m_Data.begin() + p->nPos, std::unique_ptr<SbiProcDef>(p) );
    return p;
}

// adding an externally constructed symbol definition

void SbiSymPool::Add( SbiSymDef* pDef )
{
    if( !(pDef && pDef->pIn != this) )
        return;

    if( pDef->pIn )
    {
#ifdef DBG_UTIL

        pParser->Error( ERRCODE_BASIC_INTERNAL_ERROR, u"Dbl Pool"_ustr );
#endif
        return;
    }

    pDef->nPos = m_Data.size();
    if( !pDef->nId )
    {
        // A unique name must be created in the string pool
        // for static variables (Form ProcName:VarName)
        OUString aName( pDef->aName );
        if( pDef->IsStatic() )
        {
            aName = pParser->aGblStrings.Find( nProcId )
                  + ":"
                  + pDef->aName;
        }
        pDef->nId = rStrings.Add( aName );
    }

    if( !pDef->GetProcDef() )
    {
        pDef->nProcId = nProcId;
    }
    pDef->pIn = this;
    m_Data.insert( m_Data.begin() + pDef->nPos, std::unique_ptr<SbiSymDef>(pDef) );
}


SbiSymDef* SbiSymPool::Find( const OUString& rName, bool bSearchInParents )
{
    sal_uInt16 nCount = m_Data.size();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        SbiSymDef &r = *m_Data[ nCount - i - 1 ];
        if( ( !r.nProcId || ( r.nProcId == nProcId)) &&
            ( r.aName.equalsIgnoreAsciiCase(rName)))
        {
            return &r;
        }
    }
    if( bSearchInParents && pParent )
    {
        return pParent->Find( rName );
    }
    else
    {
        return nullptr;
    }
}


// find via position (from 0)

SbiSymDef* SbiSymPool::Get( sal_uInt16 n )
{
    if (m_Data.size() <= n)
    {
        return nullptr;
    }
    else
    {
        return m_Data[ n ].get();
    }
}

sal_uInt32 SbiSymPool::Define( const OUString& rName )
{
    SbiSymDef* p = Find( rName );
    if( p )
    {
        if( p->IsDefined() )
        {
            pParser->Error( ERRCODE_BASIC_LABEL_DEFINED, rName );
        }
    }
    else
    {
        p = AddSym( rName );
    }
    return p->Define();
}

sal_uInt32 SbiSymPool::Reference( const OUString& rName )
{
    SbiSymDef* p = Find( rName );
    if( !p )
    {
        p = AddSym( rName );
    }
    // to be sure
    pParser->aGen.GenStmnt();
    return p->Reference();
}


void SbiSymPool::CheckRefs()
{
    for (std::unique_ptr<SbiSymDef> & r : m_Data)
    {
        if( !r->IsDefined() )
        {
            pParser->Error( ERRCODE_BASIC_UNDEF_LABEL, r->GetName() );
        }
    }
}

SbiSymDef::SbiSymDef( OUString _aName ) :
    aName(std::move(_aName)),
    eType(SbxEMPTY),
    pIn(nullptr),
    nLen(0),
    nDims(0),
    nId(0),
    nTypeId(0),
    nProcId(0),
    nPos(0),
    nChain(0),
    bNew(false),
    bChained(false),
    bByVal(false),
    bOpt(false),
    bStatic(false),
    bAs(false),
    bGlobal(false),
    bParamArray(false),
    bWithEvents(false),
    bWithBrackets(false),
    nDefaultId(0),
    nFixedStringLength(-1)
{
}

SbiSymDef::~SbiSymDef()
{
}

SbiProcDef* SbiSymDef::GetProcDef()
{
    return nullptr;
}

SbiConstDef* SbiSymDef::GetConstDef()
{
    return nullptr;
}


const OUString& SbiSymDef::GetName()
{
    if( pIn )
    {
        aName = pIn->rStrings.Find( nId );
    }
    return aName;
}


void SbiSymDef::SetType( SbxDataType t )
{
    if( t == SbxVARIANT && pIn )
    {
        //See if there have been any deftype statements to set the default type
        //of a variable based on its starting letter
        sal_Unicode cu = aName[0];
        if( cu < 256 )
        {
            unsigned char ch = static_cast<unsigned char>(cu);
            if( ch == '_' )
            {
                ch = 'Z';
            }
            int ch2 = rtl::toAsciiUpperCase( ch );
            int nIndex = ch2 - 'A';
            if (nIndex >= 0 && nIndex < N_DEF_TYPES)
                t = pIn->pParser->eDefTypes[nIndex];
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
    if( nChain )
    {
        pIn->pParser->aGen.BackChain( nChain );
    }
    nChain = n;
    bChained = true;
    return nChain;
}

// A symbol definition may have its own pool. This is the case
// for objects and procedures (local variable)

SbiSymPool& SbiSymDef::GetPool()
{
    if( !pPool )
    {
        pPool = std::make_unique<SbiSymPool>( pIn->pParser->aGblStrings, SbLOCAL, pIn->pParser );// is dumped
    }
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

SbiProcDef::SbiProcDef( SbiParser* pParser, const OUString& rName,
                        bool bProcDecl )
         : SbiSymDef( rName )
         , aParams( pParser->aGblStrings, SbPARAM, pParser )  // is dumped
         , aLabels( pParser->aLclStrings, SbLOCAL, pParser )  // is not dumped
         , mbProcDecl( bProcDecl )
{
    aParams.SetParent( &pParser->aPublics );
    pPool = std::make_unique<SbiSymPool>( pParser->aGblStrings, SbLOCAL, pParser );
    pPool->SetParent( &aParams );
    nLine1  =
    nLine2  = 0;
    mePropMode = PropertyMode::NONE;
    bPublic = true;
    bCdecl  = false;
    bStatic = false;
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
    SbiSymDef *pn=nullptr;
    // parameter 0 is the function name
    sal_uInt16 i;
    for( i = 1; i < aParams.GetSize(); i++ )
    {
        SbiSymDef* po = pOld->aParams.Get( i );
        pn = aParams.Get( i );
        // no type matching - that is done during running
        // but is it maybe called with too little parameters?
        if( !po && !pn->IsOptional() && !pn->IsParamArray() )
        {
            break;
        }
        pOld->aParams.Next();
    }

    if( pn && i < aParams.GetSize() && pOld->pIn )
    {
        // mark the whole line
        pOld->pIn->GetParser()->SetCol1( 0 );
        pOld->pIn->GetParser()->Error( ERRCODE_BASIC_BAD_DECLARATION, aName );
    }

    if( !pIn && pOld->pIn )
    {
        // Replace old entry with the new one
        nPos = pOld->nPos;
        nId  = pOld->nId;
        pIn  = pOld->pIn;

        // don't delete pOld twice, if it's stored in m_Data
        if (pOld == pIn->m_Data[nPos].get())
            pOld = nullptr;
        pIn->m_Data[nPos].reset(this);
    }
    delete pOld;
}

void SbiProcDef::setPropertyMode( PropertyMode ePropMode )
{
    mePropMode = ePropMode;
    if( mePropMode == PropertyMode::NONE )
        return;

    // Prop name = original scanned procedure name
    maPropName = aName;

    // CompleteProcName includes "Property xxx "
    // to avoid conflicts with other symbols
    OUString aCompleteProcName = u"Property "_ustr;
    switch( mePropMode )
    {
    case PropertyMode::Get:  aCompleteProcName += "Get "; break;
    case PropertyMode::Let:  aCompleteProcName += "Let "; break;
    case PropertyMode::Set:  aCompleteProcName += "Set "; break;
    case PropertyMode::NONE: OSL_FAIL( "Illegal PropertyMode PropertyMode::NONE" ); break;
    }
    aCompleteProcName += aName;
    aName = aCompleteProcName;
}


SbiConstDef::SbiConstDef( const OUString& rName )
           : SbiSymDef( rName )
{
    nVal = 0; eType = SbxINTEGER;
}

void SbiConstDef::Set( double n, SbxDataType t )
{
    aVal.clear(); nVal = n; eType = t;
}

void SbiConstDef::Set( const OUString& n )
{
    aVal = n; nVal = 0; eType = SbxSTRING;
}

SbiConstDef::~SbiConstDef()
{}

SbiConstDef* SbiConstDef::GetConstDef()
{
    return this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
