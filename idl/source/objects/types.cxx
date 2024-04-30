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

#include <sal/config.h>

#include <algorithm>

#include <tools/debug.hxx>

#include <types.hxx>
#include <globals.hxx>
#include <database.hxx>

SvMetaAttribute::SvMetaAttribute()
{
}

SvMetaAttribute::SvMetaAttribute( SvMetaType * pType )
    : aType( pType )
{
}

SvMetaType * SvMetaAttribute::GetType() const
{
    if( aType.is() || !GetRef() ) return aType.get();
    return static_cast<SvMetaAttribute *>(GetRef())->GetType();
}

const SvIdentifier & SvMetaAttribute::GetSlotId() const
{
    if( aSlotId.IsSet() || !GetRef() ) return aSlotId;
    return static_cast<SvMetaAttribute *>(GetRef())->GetSlotId();
}

bool SvMetaAttribute::Test( SvTokenStream & rInStm )
{
    if( GetType()->IsItem() && !GetSlotId().IsSet() )
    {
        throw SvParseException( rInStm, "slot without id declared"_ostr );
    }
    return true;
}

bool SvMetaAttribute::ReadSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos     = rInStm.Tell();
    if( !GetType() )
        // no type in ctor passed on
        aType = rBase.ReadKnownType( rInStm );
    bool bOk = false;
    if( GetType() )
    {
        ReadNameSvIdl( rInStm );
        aSlotId.ReadSvIdl( rBase, rInStm );

        bOk = true;
        SvToken& rTok  = rInStm.GetToken();
        if( rTok.IsChar() && rTok.GetChar() == '(' )
        {
            tools::SvRef<SvMetaType> xT(new SvMetaType() );
            xT->SetRef( GetType() );
            aType = xT;
            bOk = aType->ReadMethodArgs( rBase, rInStm );
        }
        if( bOk )
            bOk = SvMetaObject::ReadSvIdl( rBase, rInStm );
    }
    else
    {
        SvToken& rTok = rInStm.GetToken();
        rBase.SetError( "unknown type of token. Each new SID needs an "
                        "item statement in an SDI file, eg. "
                        "SfxVoidItem FooItem  " + rTok.GetTokenAsString() +
                        " ... which describes the slot more fully", rTok );
    }

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

size_t SvMetaAttribute::MakeSfx( OStringBuffer& rAttrArray ) const
{
    SvMetaType * pType = GetType();
    assert(pType && "no type for attribute");
    SvMetaType * pBaseType = pType->GetBaseType();
    assert(pBaseType && "no base type for attribute");
    if( pBaseType->GetMetaTypeType() == MetaTypeType::Struct )
        return pBaseType->MakeSfx( rAttrArray );
    else
    {
        rAttrArray.append('{');
        rAttrArray.append(GetSlotId().getString());
        rAttrArray.append(",\"");
        rAttrArray.append(GetName());
        rAttrArray.append("\"}");
        return 1;
    }
}

void SvMetaAttribute::Insert(SvSlotElementList&)
{
}

SvMetaType::SvMetaType()
    : nType( MetaTypeType::Base )
    , bIsItem( false )
{
}

SvMetaType::SvMetaType( const OString& rName )
    : SvMetaType()
{
    SetName( rName );
}

SvMetaType::~SvMetaType()
{}

void SvMetaType::SetType( MetaTypeType nT )
{
    nType = nT;
}

SvMetaType * SvMetaType::GetBaseType() const
{
    if( GetRef() && GetMetaTypeType() == MetaTypeType::Base )
        return static_cast<SvMetaType *>(GetRef())->GetBaseType();
    return const_cast<SvMetaType *>(this);
}

SvMetaType * SvMetaType::GetReturnType() const
{
    DBG_ASSERT( GetMetaTypeType() == MetaTypeType::Method, "no method" );
    DBG_ASSERT( GetRef(), "no return type" );
    return static_cast<SvMetaType *>(GetRef());
}

bool SvMetaType::ReadHeaderSvIdl( SvTokenStream & rInStm )
{
    bool bOk = false;
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken& rTok = rInStm.GetToken_Next();

    if( rTok.Is( SvHash_interface() ) )
    {
        SetType( MetaTypeType::Interface );
        bOk = ReadNameSvIdl( rInStm );
    }
    else if( rTok.Is( SvHash_shell() ) )
    {
        SetType( MetaTypeType::Shell );
        bOk = ReadNameSvIdl( rInStm );
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

bool SvMetaType::ReadSvIdl( SvIdlDataBase & rBase,
                            SvTokenStream & rInStm )
{
    if( ReadHeaderSvIdl( rInStm ) )
    {
        rBase.Write(OString('.'));
        return SvMetaReference::ReadSvIdl( rBase, rInStm );
    }
    return false;
}

void SvMetaType::ReadContextSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    tools::SvRef<SvMetaAttribute> xAttr( new SvMetaAttribute() );
    if( xAttr->ReadSvIdl( rBase, rInStm ) )
    {
        if( xAttr->Test( rInStm ) )
            GetAttrList().push_back( xAttr.get() );
    }
}

size_t SvMetaType::MakeSfx( OStringBuffer& rAttrArray )
{
    size_t nC = 0;

    if( GetBaseType()->GetMetaTypeType() == MetaTypeType::Struct )
    {
        size_t nAttrCount = GetAttrCount();
        // write the single attributes
        for( size_t n = 0; n < nAttrCount; n++ )
        {
            nC += aAttrList[n]->MakeSfx( rAttrArray );
            if( n +1 < nAttrCount )
                rAttrArray.append(", ");
        }
    }
    return nC;
}

void SvMetaType::WriteSfxItem(
    std::string_view rItemName, SvIdlDataBase const & rBase, SvStream& rOutStm )
{
    WriteStars( rOutStm );
    OString aVarName = OString::Concat(" a") + rItemName + "_Impl";

    OStringBuffer aAttrArray(1024);
    size_t   nAttrCount = MakeSfx( aAttrArray );
    OString aAttrCount( OString::number(nAttrCount));
    OString aTypeName = "SfxType" + aAttrCount;

    bool bExport = false, bReturn = false;
    // these are exported from sfx library
    if (rItemName == "SfxBoolItem" ||
        rItemName == "SfxInt16Item" ||
        rItemName == "SfxStringItem" ||
        rItemName == "SfxUInt16Item" ||
        rItemName == "SfxUInt32Item" ||
        rItemName == "SfxVoidItem")
    {
        bExport = true;
        if (!rBase.sSlotMapFile.endsWith("sfxslots.hxx"))
            bReturn = true;
    }

    rOutStm.WriteOString( "extern " );
    if (bExport)
        rOutStm.WriteOString( "SFX2_DLLPUBLIC " );
    rOutStm.WriteOString( aTypeName )
           .WriteOString( aVarName ).WriteChar( ';' ) << endl;
    if (bReturn)
        return;

    // write the implementation part
    rOutStm.WriteOString( "#ifdef SFX_TYPEMAP" ) << endl;
    rOutStm.WriteOString( "#if !defined(_WIN32) && (defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(EMSCRIPTEN) || defined(LINUX)))" ) << endl;
    rOutStm.WriteOString( "__attribute__((__weak__))" ) << endl;
    rOutStm.WriteOString( "#endif" ) << endl;
    rOutStm.WriteOString( aTypeName ).WriteOString( aVarName )
           .WriteOString( " = " ) << endl;
    rOutStm.WriteChar( '{' ) << endl;

    rOutStm.WriteOString( "\tcreateSfxPoolItem<" ).WriteOString( rItemName )
        .WriteOString(">, &typeid(").WriteOString( rItemName ).WriteOString( "), " );
    rOutStm.WriteOString( aAttrCount );
    if( nAttrCount )
    {
        rOutStm.WriteOString( ", { " );
        // write the single attributes
        rOutStm.WriteOString( aAttrArray );
        rOutStm.WriteOString( " }" );
    }
    rOutStm << endl;
    rOutStm.WriteOString( "};" ) << endl;
    rOutStm.WriteOString( "#endif" ) << endl << endl;
}

void SvMetaType::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if( IsItem() )
    {
        if( GetBaseType()->GetMetaTypeType() == MetaTypeType::Struct )
            GetBaseType()->WriteSfxItem( GetName(), rBase, rOutStm );
        else
            WriteSfxItem( GetName(), rBase, rOutStm );
    }
}

bool SvMetaType::ReadMethodArgs( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( rInStm.ReadIf( '(' ) )
    {
        DoReadContextSvIdl( rBase, rInStm );
        if( rInStm.ReadIf( ')' ) )
        {
            SetType( MetaTypeType::Method );
            return true;
        }
    }
    rInStm.Seek( nTokPos );
    return false;
}

SvMetaTypeString::SvMetaTypeString()
    : SvMetaType( "String"_ostr )
{
}

SvMetaEnumValue::SvMetaEnumValue()
{
}

SvMetaTypeEnum::SvMetaTypeEnum()
{
}

SvMetaTypevoid::SvMetaTypevoid()
    : SvMetaType( "void"_ostr )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
