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

#include <parser.hxx>
#include <database.hxx>
#include <globals.hxx>
#include <osl/file.hxx>

void SvIdlParser::ReadSvIdl( bool bImported, const OUString & rPath )
{
    rBase.SetPath(rPath); // only valid for this iteration
    SvToken& rTok = rInStm.GetToken();

    while( true )
    {
        rTok = rInStm.GetToken();
        if( rTok.IsEof() )
            return;

        if( rTok.Is( SvHash_module() ) )
        {
            tools::SvRef<SvMetaModule> aModule = new SvMetaModule( bImported );
            ReadModuleHeader(*aModule);
            rBase.GetModuleList().push_back( aModule );
        }
    }
}

void SvIdlParser::ReadModuleHeader(SvMetaModule& rModule)
{
    rInStm.GetToken_Next();
    OString aName = ReadIdentifier();
    rBase.Push( &rModule ); // onto the context stack
    rModule.SetName( aName );
    ReadModuleBody(rModule);
    rBase.GetStack().pop_back(); // remove from stack
}

void SvIdlParser::ReadModuleBody(SvMetaModule& rModule)
{
    if( rInStm.ReadIf( '[' ) )
    {
        while( true )
        {
            OString aSlotIdFile;
            if( !ReadStringSvIdl( SvHash_SlotIdFile(), rInStm, aSlotIdFile ) )
                break;
            if( !rBase.ReadIdFile( OStringToOUString(aSlotIdFile, RTL_TEXTENCODING_ASCII_US)) )
            {
                throw SvParseException( rInStm, "cannot read file: " + aSlotIdFile );
            }
            rInStm.ReadIfDelimiter();
        }
        ReadChar( ']' );
    }

    if( !rInStm.ReadIf( '{' ) )
        return;

    sal_uInt32 nBeginPos = 0;
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        ReadModuleElement( rModule );
        rInStm.ReadIfDelimiter();
    }
    ReadChar( '}' );
}

void SvIdlParser::ReadModuleElement( SvMetaModule& rModule )
{
    if( rInStm.GetToken().Is( SvHash_interface() ) )
    {
        ReadInterfaceOrShell(rModule, MetaTypeType::Interface);
    }
    else if( rInStm.GetToken().Is( SvHash_shell() ) )
    {
        ReadInterfaceOrShell(rModule, MetaTypeType::Shell);
    }
    else if( rInStm.GetToken().Is( SvHash_enum() ) )
    {
        ReadEnum();
    }
    else if( rInStm.GetToken().Is( SvHash_item() ) )
    {
        ReadItem();
    }
    else if( rInStm.GetToken().Is( SvHash_struct() ) )
    {
        ReadStruct();
    }
    else if( rInStm.GetToken().Is( SvHash_include() ) )
    {
        ReadInclude(rModule);
    }
    else
    {
        tools::SvRef<SvMetaSlot> xSlot( new SvMetaSlot() );

        if( xSlot->ReadSvIdl( rBase, rInStm ) )
        {
            if( xSlot->Test( rInStm ) )
            {
                // announce globally
                rBase.AppendSlot( xSlot );
            }
        }
    }
}

void SvIdlParser::ReadInclude( SvMetaModule& rModule )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    bool bOk = false;
    rInStm.GetToken_Next();
    OUString aFullName(OStringToOUString(ReadString(), RTL_TEXTENCODING_ASCII_US));
    rBase.StartNewFile( aFullName );
    osl::FileBase::RC searchError = osl::File::searchFileURL(aFullName, rBase.GetPath(), aFullName);
    if( osl::FileBase::E_None != searchError )
    {
        OStringBuffer aStr("cannot find file:");
        aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
        throw SvParseException(aStr.makeStringAndClear(), rInStm.GetToken());
    }
    osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );
    rBase.AddDepFile( aFullName );
    SvTokenStream aTokStm( aFullName );
    if( SVSTREAM_OK != aTokStm.GetStream().GetError() )
    {
        OStringBuffer aStr("cannot open file: ");
        aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
        throw SvParseException(aStr.makeStringAndClear(), rInStm.GetToken());
    }
    // rescue error from old file
    SvIdlError aOldErr = rBase.GetError();
    // reset error
    rBase.SetError( SvIdlError() );

    try {
        SvIdlParser aIncludeParser( rBase, aTokStm );
        sal_uInt32 nBeginPos = 0xFFFFFFFF; // can not happen with Tell
        while( nBeginPos != aTokStm.Tell() )
        {
            nBeginPos = aTokStm.Tell();
            aIncludeParser.ReadModuleElement(rModule);
            aTokStm.ReadIfDelimiter();
        }
    } catch (const SvParseException& ex) {
        rBase.SetError(ex.aError);
        rBase.WriteError(aTokStm);
    }
    bOk = aTokStm.GetToken().IsEof();
    if( !bOk )
    {
        rBase.WriteError( aTokStm );
    }
    // recover error from old file
    rBase.SetError( aOldErr );
    if( !bOk )
        rInStm.Seek( nTokPos );
}

void SvIdlParser::ReadStruct()
{
    ReadToken( SvHash_struct() );
    rInStm.GetToken_Next();
    tools::SvRef<SvMetaType> xStruct(new SvMetaType() );
    xStruct->SetType( MetaTypeType::Struct );
    xStruct->SetName( ReadIdentifier() );
    ReadChar( '{' );
    sal_uInt32 nBeginPos = 0; // can not happen with Tell
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        tools::SvRef<SvMetaAttribute> xAttr( new SvMetaAttribute() );
        xAttr->aType = ReadKnownType();
        xAttr->SetName(ReadIdentifier());
        xAttr->aSlotId.setString(ReadIdentifier());
        sal_uLong n;
        if( !rBase.FindId( xAttr->aSlotId.getString(), &n ) )
            throw SvParseException( rInStm, "no value for identifier <" + xAttr->aSlotId.getString() + "> " );
        xAttr->aSlotId.SetValue(n);
        xStruct->GetAttrList().push_back( xAttr );
        rInStm.ReadIfDelimiter();
        if ( rInStm.GetToken().IsChar() && rInStm.GetToken().GetChar() == '}')
            break;
    }
    ReadChar( '}' );
    // announce globally
    rBase.GetTypeList().push_back( xStruct );
}

void SvIdlParser::ReadItem()
{
    ReadToken( SvHash_item() );
    rInStm.GetToken_Next();
    tools::SvRef<SvMetaType> xItem(new SvMetaType() );
    xItem->SetItem(true);
    xItem->SetRef( ReadKnownType() );
    xItem->SetName( ReadIdentifier() );
    // announce globally
    rBase.GetTypeList().push_back( xItem );
}

void SvIdlParser::ReadEnum()
{
    rInStm.GetToken_Next();
    tools::SvRef<SvMetaTypeEnum> xEnum( new SvMetaTypeEnum() );
    xEnum->SetType( MetaTypeType::Enum );
    xEnum->SetName( ReadIdentifier() );

    ReadChar('{');
    while( true )
    {
        ReadEnumValue( *xEnum );
        if( !rInStm.ReadIfDelimiter() )
            break;
    }
    ReadChar( '}' );
    // announce globally
    rBase.GetTypeList().push_back( xEnum );
}

static OString getCommonSubPrefix(const OString &rA, const OString &rB)
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

void SvIdlParser::ReadEnumValue( SvMetaTypeEnum& rEnum )
{
    tools::SvRef<SvMetaEnumValue> aEnumVal = new SvMetaEnumValue();
    aEnumVal->SetName( ReadIdentifier() );
    if( rEnum.aEnumValueList.empty() )
    {
       // the first
       rEnum.aPrefix = aEnumVal->GetName();
    }
    else
    {
        rEnum.aPrefix = getCommonSubPrefix(rEnum.aPrefix, aEnumVal->GetName());
    }
    rEnum.aEnumValueList.push_back( aEnumVal );
}

void SvIdlParser::ReadInterfaceOrShell( SvMetaModule& rModule, MetaTypeType aMetaTypeType )
{
    tools::SvRef<SvMetaClass> aClass( new SvMetaClass() );

    rInStm.GetToken_Next();

    aClass->SetType( aMetaTypeType );

    aClass->SetName( ReadIdentifier() );

    if( rInStm.ReadIf( ':' ) )
    {
        aClass->aSuperClass = rBase.ReadKnownClass( rInStm );
        if( !aClass->aSuperClass.Is() )
            throw SvParseException( rInStm, "unknown super class" );
    }
    if( rInStm.ReadIf( '{' ) )
    {
        sal_uInt32 nBeginPos = 0; // can not happen with Tell
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            aClass->ReadContextSvIdl( rBase, rInStm );
            rInStm.ReadIfDelimiter();
        }
        ReadChar( '}' );
    }
    rModule.aClassList.push_back( aClass );
    // announce globally
    rBase.GetClassList().push_back( aClass );
}

SvMetaType * SvIdlParser::ReadKnownType()
{
    OString aName = ReadIdentifier();
    for( const auto& aType : rBase.GetTypeList() )
    {
        if( aType->GetName().equals(aName) )
        {
            return aType;
        }
    }
    throw SvParseException( rInStm, "wrong typedef: ");
}


void SvIdlParser::ReadDelimiter()
{
    if( !rInStm.ReadIfDelimiter() )
        throw SvParseException(rInStm, "expected delimiter");
}

OString SvIdlParser::ReadIdentifier()
{
    SvToken& rTok = rInStm.GetToken_Next();
    if( !rTok.IsIdentifier() )
        throw SvParseException("expected identifier", rTok);
    return rTok.GetString();
}

OString SvIdlParser::ReadString()
{
    SvToken& rTok = rInStm.GetToken_Next();
    if( !rTok.IsString() )
        throw SvParseException("expected string", rTok);
    return rTok.GetString();
}

void SvIdlParser::ReadChar(char cChar)
{
    if( !rInStm.ReadIf( cChar ) )
        throw SvParseException(rInStm, "expected char '" + OString(cChar) + "'");
}

void SvIdlParser::ReadToken(SvStringHashEntry* entry)
{
    if( !rInStm.GetToken().Is(entry) )
        throw SvParseException("expected " + entry->GetName(), rInStm.GetToken());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
