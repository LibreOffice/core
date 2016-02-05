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
#include <ctype.h>
#include <stdio.h>

#include <tools/debug.hxx>

#include <types.hxx>
#include <globals.hxx>
#include <database.hxx>

SvMetaAttribute::SvMetaAttribute()
    : aExport( true, false )
    , aReadOnlyDoc ( true, false )
    , bNewAttr( false )
{
}

SvMetaAttribute::SvMetaAttribute( SvMetaType * pType )
    : aType( pType )
    , aExport( true, false )
    , aReadOnlyDoc ( true, false)
    , bNewAttr( false )
{
}

SvMetaType * SvMetaAttribute::GetType() const
{
    if( aType.Is() || !GetRef() ) return aType;
    return static_cast<SvMetaAttribute *>(GetRef())->GetType();
}

const SvIdentifier & SvMetaAttribute::GetSlotId() const
{
    if( aSlotId.IsSet() || !GetRef() ) return aSlotId;
    return static_cast<SvMetaAttribute *>(GetRef())->GetSlotId();
}

bool SvMetaAttribute::GetExport() const
{
    if( aExport.IsSet() || !GetRef() ) return aExport;
    return static_cast<SvMetaAttribute *>(GetRef())->GetExport();
}

bool SvMetaAttribute::GetHidden() const
{
    // when export is set, but hidden is not the default is used
    if ( aExport.IsSet() )
        return !aExport;
    else if( !GetRef() )
        return false;
    else
        return static_cast<SvMetaAttribute *>(GetRef())->GetHidden();
}

bool SvMetaAttribute::GetReadOnlyDoc() const
{
    if( aReadOnlyDoc.IsSet() || !GetRef() ) return aReadOnlyDoc;
    return static_cast<SvMetaSlot *>(GetRef())->GetReadOnlyDoc();
}

bool SvMetaAttribute::IsMethod() const
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    return pType->GetType() == MetaTypeType::Method;
}

bool SvMetaAttribute::IsVariable() const
{
    SvMetaType * pType = GetType();
    return pType->GetType() != MetaTypeType::Method;
}

OString SvMetaAttribute::GetMangleName( bool ) const
{
    return GetName().getString();
}

bool SvMetaAttribute::Test( SvIdlDataBase & rBase,
                            SvTokenStream & rInStm )
{
    bool bOk = true;
    if( GetType()->IsItem() && !GetSlotId().IsSet() )
    {
        rBase.SetError( "slot without id declared", rInStm.GetToken() );
        rBase.WriteError( rInStm );
        bOk = false;
    }
    return bOk;
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
        ReadNameSvIdl( rBase, rInStm );
        aSlotId.ReadSvIdl( rBase, rInStm );

        bOk = true;
        SvToken& rTok  = rInStm.GetToken();
        if( bOk && rTok.IsChar() && rTok.GetChar() == '(' )
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

void SvMetaAttribute::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm )
{
    SvMetaReference::ReadAttributesSvIdl( rBase, rInStm );
    aExport.ReadSvIdl( SvHash_Export(), rInStm );
    aReadOnlyDoc.ReadSvIdl( SvHash_ReadOnlyDoc(), rInStm );
}

sal_uLong SvMetaAttribute::MakeSfx( OStringBuffer& rAttrArray )
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" );
    if( pBaseType->GetType() == MetaTypeType::Struct )
        return pBaseType->MakeSfx( rAttrArray );
    else
    {
        rAttrArray.append('{');
        rAttrArray.append(GetSlotId().getString());
        rAttrArray.append(",\"");
        rAttrArray.append(GetName().getString());
        rAttrArray.append("\"}");
        return 1;
    }
}

void SvMetaAttribute::Insert (SvSlotElementList&, const OString&, SvIdlDataBase&)
{
}

#define CTOR                            \
    : pAttrList( nullptr )              \
    , nType( MetaTypeType::Base )       \
    , bIsItem( false )                  \
    , bIsShell( false )                 \
    , cParserChar( 'h' )

SvMetaType::SvMetaType()
    CTOR
{
}

SvMetaType::SvMetaType( const OString& rName, char cPC,
                        const OString& rCName )
    CTOR
{
    SetName( rName );
    cParserChar = cPC;
    aCName.setString(rCName);
}

SvMetaType::SvMetaType( const OString& rName,
                        char cPc,
                        const OString& rCName,
                        const OString& rBasicName,
                        const OString& rBasicPostfix )
    CTOR
{
    SetName( rName );
    cParserChar = cPc;
    aCName.setString(rCName);
    aBasicName.setString(rBasicName);
    aBasicPostfix.setString(rBasicPostfix);
}

SvMetaType::~SvMetaType() {
    delete pAttrList;
}

SvRefMemberList<SvMetaAttribute *>& SvMetaType::GetAttrList() const
{
    if( !pAttrList )
        const_cast<SvMetaType *>(this)->pAttrList = new SvRefMemberList<SvMetaAttribute *>();
    return *pAttrList;
}

void SvMetaType::SetType( MetaTypeType nT )
{
    nType = nT;
    if( nType == MetaTypeType::Class )
    {
        OStringBuffer aTmp(C_PREF);
        aTmp.append("Object *");
        aCName.setString(aTmp.makeStringAndClear());
    }
}

SvMetaType * SvMetaType::GetBaseType() const
{
    if( GetRef() && GetType() == MetaTypeType::Base )
        return static_cast<SvMetaType *>(GetRef())->GetBaseType();
    return const_cast<SvMetaType *>(this);
}

SvMetaType * SvMetaType::GetReturnType() const
{
    DBG_ASSERT( GetType() == MetaTypeType::Method, "no method" );
    DBG_ASSERT( GetRef(), "no return type" );
    return static_cast<SvMetaType *>(GetRef());
}

const OString& SvMetaType::GetBasicName() const
{
    if( aBasicName.IsSet() || !GetRef() )
        return aBasicName.getString();
    else
        return static_cast<SvMetaType*>(GetRef())->GetBasicName();
}

const OString& SvMetaType::GetCName() const
{
    if( aCName.IsSet() || !GetRef() )
        return aCName.getString();
    else
        return static_cast<SvMetaType *>(GetRef())->GetCName();
}

bool SvMetaType::SetName( const OString& rName, SvIdlDataBase * pBase )
{
    aCName.setString(rName);
    return SvMetaReference::SetName( rName, pBase );
}

bool SvMetaType::ReadHeaderSvIdl( SvIdlDataBase & rBase,
                                  SvTokenStream & rInStm )
{
    bool bOk = false;
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_interface() )
      || pTok->Is( SvHash_shell() ) )
    {
        if( pTok->Is( SvHash_shell() ) )
            bIsShell = true;
        SetType( MetaTypeType::Class );
        bOk = ReadNamesSvIdl( rBase, rInStm );

    }
    else if( pTok->Is( SvHash_struct() ) )
    {
        SetType( MetaTypeType::Struct );
        bOk = ReadNamesSvIdl( rBase, rInStm );
    }
    else if( pTok->Is( SvHash_enum() ) )
    {
        SetType( MetaTypeType::Enum );
        bOk = ReadNameSvIdl( rBase, rInStm );
    }
    else if( pTok->Is( SvHash_item() ) )
    {
          if( pTok->Is( SvHash_item() ) )
            bIsItem = true;

        SvMetaType * pType = rBase.ReadKnownType( rInStm );
        if( pType )
        {
            SetRef( pType );
            if( ReadNameSvIdl( rBase, rInStm ) )
            {
                if( rInStm.Read( '(' ) )
                {
                    DoReadContextSvIdl( rBase, rInStm );
                    if( rInStm.Read( ')' ) )
                    {
                        SetType( MetaTypeType::Method );
                        bOk = true;
                    }
                }
                else
                {
                    bOk = true;
                }
            }
        }
        else
        {
            OString aStr("wrong typedef: ");
            rBase.SetError( aStr, rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    if( bOk )
        SetModule( rBase );
    else
        rInStm.Seek( nTokPos );
    return bOk;
}

bool SvMetaType::ReadSvIdl( SvIdlDataBase & rBase,
                            SvTokenStream & rInStm )
{
    if( ReadHeaderSvIdl( rBase, rInStm ) )
    {
        rBase.Write(OString('.'));
        return SvMetaExtern::ReadSvIdl( rBase, rInStm );
    }
    return false;
}

bool SvMetaType::ReadNamesSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    bool bOk = ReadNameSvIdl( rBase, rInStm );

    return bOk;
}

void SvMetaType::ReadContextSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    tools::SvRef<SvMetaAttribute> xAttr( new SvMetaAttribute() );
    if( xAttr->ReadSvIdl( rBase, rInStm ) )
    {
        if( xAttr->Test( rBase, rInStm ) )
            GetAttrList().push_back( xAttr );
    }
}

sal_uLong SvMetaType::MakeSfx( OStringBuffer& rAttrArray )
{
    sal_uLong nC = 0;

    if( GetBaseType()->GetType() == MetaTypeType::Struct )
    {
        sal_uLong nAttrCount = GetAttrCount();
        // write the single attributes
        for( sal_uLong n = 0; n < nAttrCount; n++ )
        {
            nC += (*pAttrList)[n]->MakeSfx( rAttrArray );
            if( n +1 < nAttrCount )
                rAttrArray.append(", ");
        }
    }
    return nC;
}

void SvMetaType::WriteSfxItem(
    const OString& rItemName, SvIdlDataBase& rBase, SvStream& rOutStm )
{
    WriteStars( rOutStm );
    OStringBuffer aVarName(" a");
    aVarName.append(rItemName).append("_Impl");

    OStringBuffer aTypeName("SfxType");
    OStringBuffer aAttrArray;
    sal_uLong   nAttrCount = MakeSfx( aAttrArray );
    OString aAttrCount(
        OString::number(nAttrCount));
    aTypeName.append(aAttrCount);

    bool bExport = false, bReturn = false;
    // these are exported from sfx library
    if (rItemName == "SfxBoolItem" ||
        rItemName == "SfxStringItem" ||
        rItemName == "SfxUInt16Item" ||
        rItemName == "SfxUInt32Item" ||
        rItemName == "SfxVoidItem")
    {
        bExport = true;
        if (!rBase.sSlotMapFile.endsWith("sfxslots.hxx"))
            bReturn = true;
    }

    rOutStm.WriteCharPtr( "extern " );
    if (bExport)
        rOutStm.WriteCharPtr( "SFX2_DLLPUBLIC " );
    rOutStm.WriteCharPtr( aTypeName.getStr() )
           .WriteCharPtr( aVarName.getStr() ).WriteChar( ';' ) << endl;
    if (bReturn)
        return;

    // write the implementation part
    rOutStm.WriteCharPtr( "#ifdef SFX_TYPEMAP" ) << endl;
    rOutStm.WriteCharPtr( "#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS))) || STATIC_LINKING)" ) << endl;
    rOutStm.WriteCharPtr( "__attribute__((__weak__))" ) << endl;
    rOutStm.WriteCharPtr( "#endif" ) << endl;
    if (bExport)
        rOutStm.WriteCharPtr( "SFX2_DLLPUBLIC " );
    rOutStm.WriteCharPtr( aTypeName.getStr() ).WriteCharPtr( aVarName.getStr() )
           .WriteCharPtr( " = " ) << endl;
    rOutStm.WriteChar( '{' ) << endl;

    rOutStm.WriteCharPtr( "\tcreateSfxPoolItem<" ).WriteCharPtr( rItemName.getStr() )
        .WriteCharPtr(">, &typeid(").WriteCharPtr( rItemName.getStr() ).WriteCharPtr( "), " );
    rOutStm.WriteCharPtr( aAttrCount.getStr() );
    if( nAttrCount )
    {
        rOutStm.WriteCharPtr( ", { " );
        // write the single attributes
        rOutStm.WriteCharPtr( aAttrArray.getStr() );
        rOutStm.WriteCharPtr( " }" );
    }
    rOutStm << endl;
    rOutStm.WriteCharPtr( "};" ) << endl;
    rOutStm.WriteCharPtr( "#endif" ) << endl << endl;
}

void SvMetaType::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if( IsItem() )
    {
        if( GetBaseType()->GetType() == MetaTypeType::Struct )
            GetBaseType()->WriteSfxItem( GetName().getString(), rBase, rOutStm );
        else
            WriteSfxItem( GetName().getString(), rBase, rOutStm );
    }
}

bool SvMetaType::ReadMethodArgs( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( rInStm.Read( '(' ) )
    {
        DoReadContextSvIdl( rBase, rInStm );
        if( rInStm.Read( ')' ) )
        {
            SetType( MetaTypeType::Method );
            return true;
        }
    }
    rInStm.Seek( nTokPos );
    return false;
}

OString SvMetaType::GetParserString() const
{
    SvMetaType * pBT = GetBaseType();
    if( pBT != this )
        return pBT->GetParserString();

    MetaTypeType type = GetType();
    OString aPStr;

    if( MetaTypeType::Method == type || MetaTypeType::Struct == type )
    {
        sal_uLong nAttrCount = GetAttrCount();
        // write the single attributes
        for( sal_uLong n = 0; n < nAttrCount; n++ )
        {
            SvMetaAttribute * pT = (*pAttrList)[n];
            aPStr += pT->GetType()->GetParserString();
        }
    }
    else
        aPStr = OString(GetParserChar());
    return aPStr;
}

SvMetaTypeString::SvMetaTypeString()
    : SvMetaType( "String", 's', "char *", "String", "$" )
{
}

SvMetaEnumValue::SvMetaEnumValue()
{
}

bool SvMetaEnumValue::ReadSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    if( !ReadNameSvIdl( rBase, rInStm ) )
        return false;
    return true;
}

SvMetaTypeEnum::SvMetaTypeEnum()
{
    SetBasicName("Integer");
}

namespace
{
    OString getCommonSubPrefix(const OString &rA, const OString &rB)
    {
        sal_Int32 nMax = std::min(rA.getLength(), rB.getLength());
        sal_Int32 nI = 0;
        while (nI < nMax)
        {
            if (rA[nI] != rB[nI])
                break;
            ++nI;
        }
        return rA.copy(0, nI);
    }
}

void SvMetaTypeEnum::ReadContextSvIdl( SvIdlDataBase & rBase,
                                       SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();

    tools::SvRef<SvMetaEnumValue> aEnumVal = new SvMetaEnumValue();
    bool bOk = aEnumVal->ReadSvIdl( rBase, rInStm );
    if( bOk )
    {
        if( aEnumValueList.empty() )
        {
           // the first
           aPrefix = aEnumVal->GetName().getString();
        }
        else
        {
            aPrefix = getCommonSubPrefix(aPrefix, aEnumVal->GetName().getString());
        }
        aEnumValueList.push_back( aEnumVal );
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
}

bool SvMetaTypeEnum::ReadSvIdl( SvIdlDataBase & rBase,
                                SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm )
      && GetType() == MetaTypeType::Enum )
    {
        if( SvMetaObject::ReadSvIdl( rBase, rInStm ) )
             return true;
    }
    rInStm.Seek( nTokPos );
    return false;
}

SvMetaTypevoid::SvMetaTypevoid()
    : SvMetaType( "void", 'v', "void", "", "" )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
