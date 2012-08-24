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

#include <tools/stream.hxx>

#include <basic/sbx.hxx>
#include "sbxres.hxx"

TYPEINIT1(SbxCollection,SbxObject)
TYPEINIT1(SbxStdCollection,SbxCollection)

static const char* pCount;
static const char* pAdd;
static const char* pItem;
static const char* pRemove;
static sal_uInt16 nCountHash = 0, nAddHash, nItemHash, nRemoveHash;


SbxCollection::SbxCollection( const XubString& rClass )
             : SbxObject( rClass )
{
    if( !nCountHash )
    {
        pCount  = GetSbxRes( STRING_COUNTPROP );
        pAdd    = GetSbxRes( STRING_ADDMETH );
        pItem   = GetSbxRes( STRING_ITEMMETH );
        pRemove = GetSbxRes( STRING_REMOVEMETH );
        nCountHash  = MakeHashCode( rtl::OUString::createFromAscii( pCount ) );
        nAddHash    = MakeHashCode( rtl::OUString::createFromAscii( pAdd ) );
        nItemHash   = MakeHashCode( rtl::OUString::createFromAscii( pItem ) );
        nRemoveHash = MakeHashCode( rtl::OUString::createFromAscii( pRemove ) );
    }
    Initialize();
    // For Access on itself
    StartListening( GetBroadcaster(), sal_True );
}

SbxCollection::SbxCollection( const SbxCollection& rColl )
    : SvRefBase( rColl ), SbxObject( rColl )
{}

SbxCollection& SbxCollection::operator=( const SbxCollection& r )
{
    if( &r != this )
        SbxObject::operator=( r );
    return *this;
}

SbxCollection::~SbxCollection()
{}

void SbxCollection::Clear()
{
    SbxObject::Clear();
    Initialize();
}

void SbxCollection::Initialize()
{
    SetType( SbxOBJECT );
    SetFlag( SBX_FIXED );
    ResetFlag( SBX_WRITE );
    SbxVariable* p;
    p = Make( rtl::OUString::createFromAscii( pCount ), SbxCLASS_PROPERTY, SbxINTEGER );
    p->ResetFlag( SBX_WRITE );
    p->SetFlag( SBX_DONTSTORE );
    p = Make( rtl::OUString::createFromAscii( pAdd ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
    p = Make( rtl::OUString::createFromAscii( pItem ), SbxCLASS_METHOD, SbxOBJECT );
    p->SetFlag( SBX_DONTSTORE );
    p = Make( rtl::OUString::createFromAscii( pRemove ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
}

SbxVariable* SbxCollection::FindUserData( sal_uInt32 nData )
{
    if( GetParameters() )
    {
        SbxObject* pObj = (SbxObject*) GetObject();
        return pObj ? pObj->FindUserData( nData ) : NULL;
    }
    else
        return SbxObject::FindUserData( nData );
}

SbxVariable* SbxCollection::Find( const rtl::OUString& rName, SbxClassType t )
{
    if( GetParameters() )
    {
        SbxObject* pObj = (SbxObject*) GetObject();
        return pObj ? pObj->Find( rName, t ) : NULL;
    }
    else
        return SbxObject::Find( rName, t );
}

void SbxCollection::SFX_NOTIFY( SfxBroadcaster& rCst, const TypeId& rId1,
                                const SfxHint& rHint, const TypeId& rId2 )
{
    const SbxHint* p = PTR_CAST(SbxHint,&rHint);
    if( p )
    {
        sal_uIntPtr nId = p->GetId();
        bool bRead  = ( nId == SBX_HINT_DATAWANTED );
        bool bWrite = ( nId == SBX_HINT_DATACHANGED );
        SbxVariable* pVar = p->GetVar();
        SbxArray* pArg = pVar->GetParameters();
        if( bRead || bWrite )
        {
            XubString aVarName( pVar->GetName() );
            if( pVar == this )
                CollItem( pArg );
            else if( pVar->GetHashCode() == nCountHash
                  && aVarName.EqualsIgnoreCaseAscii( pCount ) )
                pVar->PutLong( pObjs->Count() );
            else if( pVar->GetHashCode() == nAddHash
                  && aVarName.EqualsIgnoreCaseAscii( pAdd ) )
                CollAdd( pArg );
            else if( pVar->GetHashCode() == nItemHash
                  && aVarName.EqualsIgnoreCaseAscii( pItem ) )
                CollItem( pArg );
            else if( pVar->GetHashCode() == nRemoveHash
                  && aVarName.EqualsIgnoreCaseAscii( pRemove ) )
                CollRemove( pArg );
            else
                SbxObject::SFX_NOTIFY( rCst, rId1, rHint, rId2 );
            return;
        }
    }
    SbxObject::SFX_NOTIFY( rCst, rId1, rHint, rId2 );
}

// Default: argument is object

void SbxCollection::CollAdd( SbxArray* pPar_ )
{
    if( pPar_->Count() != 2 )
        SetError( SbxERR_WRONG_ARGS );
    else
    {
        SbxBase* pObj = pPar_->Get( 1 )->GetObject();
        if( !pObj || !( pObj->ISA(SbxObject) ) )
            SetError( SbxERR_NOTIMP );
        else
            Insert( (SbxObject*) pObj );
    }
}

// Default: index from 1 or object name

void SbxCollection::CollItem( SbxArray* pPar_ )
{
    if( pPar_->Count() != 2 )
        SetError( SbxERR_WRONG_ARGS );
    else
    {
        SbxVariable* pRes = NULL;
        SbxVariable* p = pPar_->Get( 1 );
        if( p->GetType() == SbxSTRING )
            pRes = Find( p->GetString(), SbxCLASS_OBJECT );
        else
        {
            short n = p->GetInteger();
            if( n >= 1 && n <= (short) pObjs->Count() )
                pRes = pObjs->Get( (sal_uInt16) n - 1 );
        }
        if( !pRes )
            SetError( SbxERR_BAD_INDEX );
        pPar_->Get( 0 )->PutObject( pRes );
    }
}

// Default: index from 1

void SbxCollection::CollRemove( SbxArray* pPar_ )
{
    if( pPar_->Count() != 2 )
        SetError( SbxERR_WRONG_ARGS );
    else
    {
        short n = pPar_->Get( 1 )->GetInteger();
        if( n < 1 || n > (short) pObjs->Count() )
            SetError( SbxERR_BAD_INDEX );
        else
            Remove( pObjs->Get( (sal_uInt16) n - 1 ) );
    }
}

sal_Bool SbxCollection::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    sal_Bool bRes = SbxObject::LoadData( rStrm, nVer );
    Initialize();
    return bRes;
}


SbxStdCollection::SbxStdCollection
                    ( const XubString& rClass, const XubString& rElem, sal_Bool b )
                  : SbxCollection( rClass ), aElemClass( rElem ),
                    bAddRemoveOk( b )
{}

SbxStdCollection::SbxStdCollection( const SbxStdCollection& r )
                  : SvRefBase( r ), SbxCollection( r ),
                    aElemClass( r.aElemClass ), bAddRemoveOk( r.bAddRemoveOk )
{}

SbxStdCollection& SbxStdCollection::operator=( const SbxStdCollection& r )
{
    if( &r != this )
    {
        if( !r.aElemClass.EqualsIgnoreCaseAscii( aElemClass ) )
            SetError( SbxERR_CONVERSION );
        else
            SbxCollection::operator=( r );
    }
    return *this;
}

SbxStdCollection::~SbxStdCollection()
{}

// Default: Error, if wrong object

void SbxStdCollection::Insert( SbxVariable* p )
{
    SbxObject* pObj = PTR_CAST(SbxObject,p);
    if( pObj && !pObj->IsClass( aElemClass ) )
        SetError( SbxERR_BAD_ACTION );
    else
        SbxCollection::Insert( p );
}

void SbxStdCollection::CollAdd( SbxArray* pPar_ )
{
    if( !bAddRemoveOk )
        SetError( SbxERR_BAD_ACTION );
    else
        SbxCollection::CollAdd( pPar_ );
}

void SbxStdCollection::CollRemove( SbxArray* pPar_ )
{
    if( !bAddRemoveOk )
        SetError( SbxERR_BAD_ACTION );
    else
        SbxCollection::CollRemove( pPar_ );
}

sal_Bool SbxStdCollection::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    sal_Bool bRes = SbxCollection::LoadData( rStrm, nVer );
    if( bRes )
    {
        aElemClass = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm,
            RTL_TEXTENCODING_ASCII_US);
        rStrm >> bAddRemoveOk;
    }
    return bRes;
}

sal_Bool SbxStdCollection::StoreData( SvStream& rStrm ) const
{
    sal_Bool bRes = SbxCollection::StoreData( rStrm );
    if( bRes )
    {
        write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStrm, aElemClass,
            RTL_TEXTENCODING_ASCII_US);
        rStrm << bAddRemoveOk;
    }
    return bRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
