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

#include <o3tl/safeint.hxx>
#include <tools/stream.hxx>

#include <basic/sbx.hxx>
#include <basic/sberrors.hxx>
#include "sbxres.hxx"


static OUString pCount;
static OUString pAdd;
static OUString pItem;
static OUString pRemove;
static sal_uInt16 nCountHash = 0, nAddHash, nItemHash, nRemoveHash;


SbxCollection::SbxCollection()
             : SbxObject( u""_ustr )
{
    if( !nCountHash )
    {
        pCount  = GetSbxRes( StringId::CountProp );
        pAdd    = GetSbxRes( StringId::AddMeth );
        pItem   = GetSbxRes( StringId::ItemMeth );
        pRemove = GetSbxRes( StringId::RemoveMeth );
        nCountHash  = MakeHashCode( pCount );
        nAddHash    = MakeHashCode( pAdd );
        nItemHash   = MakeHashCode( pItem );
        nRemoveHash = MakeHashCode( pRemove );
    }
    Initialize();
    // For Access on itself
    StartListening(GetBroadcaster(), DuplicateHandling::Prevent);
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
    SetFlag( SbxFlagBits::Fixed );
    ResetFlag( SbxFlagBits::Write );
    SbxVariable* p;
    p = Make( pCount , SbxClassType::Property, SbxINTEGER );
    p->ResetFlag( SbxFlagBits::Write );
    p->SetFlag( SbxFlagBits::DontStore );
    p = Make( pAdd, SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p = Make( pItem , SbxClassType::Method, SbxOBJECT );
    p->SetFlag( SbxFlagBits::DontStore );
    p = Make( pRemove, SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
}

SbxVariable* SbxCollection::Find( const OUString& rName, SbxClassType t )
{
    if( GetParameters() )
    {
        SbxObject* pObj = static_cast<SbxObject*>(GetObject());
        return pObj ? pObj->Find( rName, t ) : nullptr;
    }
    else
    {
        return SbxObject::Find( rName, t );
    }
}

void SbxCollection::Notify( SfxBroadcaster& rCst, const SfxHint& rHint )
{
    const SbxHint* p = dynamic_cast<const SbxHint*>(&rHint);
    if( p )
    {
        const SfxHintId nId = p->GetId();
        bool bRead  = ( nId == SfxHintId::BasicDataWanted );
        bool bWrite = ( nId == SfxHintId::BasicDataChanged );
        SbxVariable* pVar = p->GetVar();
        SbxArray* pArg = pVar->GetParameters();
        if( bRead || bWrite )
        {
            OUString aVarName( pVar->GetName() );
            if( pVar == this )
            {
                CollItem( pArg );
            }
            else if( pVar->GetHashCode() == nCountHash
                  && aVarName.equalsIgnoreAsciiCase( pCount ) )
            {
                pVar->PutLong(sal::static_int_cast<sal_Int32>(pObjs->Count()));
            }
            else if( pVar->GetHashCode() == nAddHash
                  && aVarName.equalsIgnoreAsciiCase( pAdd ) )
            {
                CollAdd( pArg );
            }
            else if( pVar->GetHashCode() == nItemHash
                  && aVarName.equalsIgnoreAsciiCase( pItem ) )
            {
                CollItem( pArg );
            }
            else if( pVar->GetHashCode() == nRemoveHash
                  && aVarName.equalsIgnoreAsciiCase( pRemove ) )
            {
                CollRemove( pArg );
            }
            else
            {
                SbxObject::Notify( rCst, rHint );
            }
            return;
        }
    }
    SbxObject::Notify( rCst, rHint );
}

// Default: argument is object

void SbxCollection::CollAdd( SbxArray* pPar_ )
{
    if (pPar_->Count() != 2)
    {
        SetError( ERRCODE_BASIC_WRONG_ARGS );
    }
    else
    {
        SbxBase* pObj = pPar_->Get(1)->GetObject();
        if( !pObj || dynamic_cast<const SbxObject*>(pObj) == nullptr )
        {
            SetError( ERRCODE_BASIC_BAD_ARGUMENT );
        }
        else
        {
            Insert( static_cast<SbxObject*>(pObj) );
        }
    }
}

// Default: index from 1 or object name

void SbxCollection::CollItem( SbxArray* pPar_ )
{
    if (pPar_->Count() != 2)
    {
        SetError( ERRCODE_BASIC_WRONG_ARGS );
    }
    else
    {
        SbxVariable* pRes = nullptr;
        SbxVariable* p = pPar_->Get(1);
        if( p->GetType() == SbxSTRING )
        {
            pRes = Find( p->GetOUString(), SbxClassType::Object );
        }
        else
        {
            short n = p->GetInteger();
            if (n >= 1 && o3tl::make_unsigned(n) <= pObjs->Count())
            {
                pRes = pObjs->Get(static_cast<sal_uInt32>(n) - 1);
            }
        }
        if( !pRes )
        {
            SetError( ERRCODE_BASIC_BAD_INDEX );
        }
        pPar_->Get(0)->PutObject(pRes);
    }
}

// Default: index from 1

void SbxCollection::CollRemove( SbxArray* pPar_ )
{
    if (pPar_->Count() != 2)
        SetError( ERRCODE_BASIC_WRONG_ARGS );
    else
    {
        short n = pPar_->Get(1)->GetInteger();
        if (n < 1 || o3tl::make_unsigned(n) > pObjs->Count())
            SetError( ERRCODE_BASIC_BAD_INDEX );
        else
            Remove(pObjs->Get(static_cast<sal_uInt32>(n) - 1));
    }
}

bool SbxCollection::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    bool bRes = SbxObject::LoadData( rStrm, nVer );
    Initialize();
    return bRes;
}


SbxStdCollection::SbxStdCollection()
                  : bAddRemoveOk( true )
{}

SbxStdCollection::SbxStdCollection( const SbxStdCollection& r )
                  : SvRefBase( r ), SbxCollection( r ),
                    aElemClass( r.aElemClass ), bAddRemoveOk( r.bAddRemoveOk )
{}

SbxStdCollection& SbxStdCollection::operator=( const SbxStdCollection& r )
{
    if( &r != this )
    {
        if( !r.aElemClass.equalsIgnoreAsciiCase( aElemClass ) )
        {
            SetError( ERRCODE_BASIC_CONVERSION );
        }
        else
        {
            SbxCollection::operator=( r );
        }
    }
    return *this;
}

SbxStdCollection::~SbxStdCollection()
{}

// Default: Error, if wrong object

void SbxStdCollection::Insert( SbxVariable* p )
{
    SbxObject* pObj = dynamic_cast<SbxObject*>( p );
    if( pObj && !pObj->IsClass( aElemClass ) )
        SetError( ERRCODE_BASIC_BAD_ACTION );
    else
        SbxCollection::Insert( p );
}

void SbxStdCollection::CollAdd( SbxArray* pPar_ )
{
    if( !bAddRemoveOk )
        SetError( ERRCODE_BASIC_BAD_ACTION );
    else
        SbxCollection::CollAdd( pPar_ );
}

void SbxStdCollection::CollRemove( SbxArray* pPar_ )
{
    if( !bAddRemoveOk )
        SetError( ERRCODE_BASIC_BAD_ACTION );
    else
        SbxCollection::CollRemove( pPar_ );
}

bool SbxStdCollection::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    bool bRes = SbxCollection::LoadData( rStrm, nVer );
    if( bRes )
    {
        aElemClass = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
            RTL_TEXTENCODING_ASCII_US);
        rStrm.ReadCharAsBool( bAddRemoveOk );
    }
    return bRes;
}

std::pair<bool, sal_uInt32> SbxStdCollection::StoreData( SvStream& rStrm ) const
{
    const auto& [bRes, nVersion] = SbxCollection::StoreData(rStrm);
    if( bRes )
    {
        write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, aElemClass,
            RTL_TEXTENCODING_ASCII_US);
        rStrm.WriteBool( bAddRemoveOk );
    }
    return { bRes, nVersion };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
