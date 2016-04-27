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

void SvIdlParser::ReadSvIdl( const OUString & rPath )
{
    rBase.SetPath(rPath); // only valid for this iteration
    SvToken& rTok = rInStm.GetToken();

    while( true )
    {
        rTok = rInStm.GetToken();
        if( rTok.IsEof() )
            return;

        Read( SvHash_module() );
        tools::SvRef<SvMetaModule> aModule = new SvMetaModule;
        ReadModuleHeader(*aModule);
        rBase.GetModuleList().push_back( aModule );
    }
}

void SvIdlParser::ReadModuleHeader(SvMetaModule& rModule)
{
    OString aName = ReadIdentifier();
    rModule.SetName( aName );
    rBase.Push( &rModule ); // onto the context stack
    ReadModuleBody(rModule);
    rBase.GetStack().pop_back(); // remove from stack
}

void SvIdlParser::ReadModuleBody(SvMetaModule& rModule)
{
    if( ReadIf( '[' ) )
    {
        while( true )
        {
            OString aSlotIdFile;
            if( !ReadStringSvIdl( SvHash_SlotIdFile(), rInStm, aSlotIdFile ) )
                break;
            if( !rBase.ReadIdFile( aSlotIdFile ) )
            {
                throw SvParseException( rInStm, "cannot read file: " + aSlotIdFile );
            }
            ReadIfDelimiter();
        }
        Read( ']' );
    }

    if( !ReadIf( '{' ) )
        return;

    sal_uInt32 nBeginPos = 0;
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        ReadModuleElement( rModule );
        ReadIfDelimiter();
    }
    Read( '}' );
}

void SvIdlParser::ReadModuleElement( SvMetaModule& rModule )
{
    if( ReadIf( SvHash_interface() ) )
    {
        ReadInterfaceOrShell(rModule, MetaTypeType::Interface);
    }
    else if( ReadIf( SvHash_shell() ) )
    {
        ReadInterfaceOrShell(rModule, MetaTypeType::Shell);
    }
    else if( ReadIf( SvHash_enum() ) )
    {
        ReadEnum();
    }
    else if( ReadIf( SvHash_item() ) )
    {
        ReadItem();
    }
    else if( ReadIf( SvHash_struct() ) )
    {
        ReadStruct();
    }
    else if( ReadIf( SvHash_include() ) )
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
    tools::SvRef<SvMetaType> xStruct(new SvMetaType() );
    xStruct->SetType( MetaTypeType::Struct );
    xStruct->SetName( ReadIdentifier() );
    Read( '{' );
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
        if( !ReadIfDelimiter() )
            break;
        if( rInStm.GetToken().IsChar() && rInStm.GetToken().GetChar() == '}')
            break;
    }
    Read( '}' );
    ReadDelimiter();
    // announce globally
    rBase.GetTypeList().push_back( xStruct );
}

void SvIdlParser::ReadItem()
{
    tools::SvRef<SvMetaType> xItem(new SvMetaType() );
    xItem->SetItem(true);
    xItem->SetRef( ReadKnownType() );
    xItem->SetName( ReadIdentifier() );
    // announce globally
    rBase.GetTypeList().push_back( xItem );
}

void SvIdlParser::ReadEnum()
{
    tools::SvRef<SvMetaTypeEnum> xEnum( new SvMetaTypeEnum() );
    xEnum->SetType( MetaTypeType::Enum );
    xEnum->SetName( ReadIdentifier() );

    Read('{');
    while( true )
    {
        ReadEnumValue( *xEnum );
        if( !ReadIfDelimiter() )
            break;
    }
    Read( '}' );
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

    aClass->SetType( aMetaTypeType );

    aClass->SetName( ReadIdentifier() );

    if( ReadIf( ':' ) )
    {
        aClass->aSuperClass = ReadKnownClass();
    }
    if( ReadIf( '{' ) )
    {
        sal_uInt32 nBeginPos = 0; // can not happen with Tell
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            ReadInterfaceOrShellEntry(*aClass);
            ReadIfDelimiter();
        }
        Read( '}' );
    }
    rModule.aClassList.push_back( aClass );
    // announce globally
    rBase.GetClassList().push_back( aClass );
}

void SvIdlParser::ReadInterfaceOrShellEntry(SvMetaClass& rClass)
{
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken&    rTok = rInStm.GetToken_Next();

    if( rTok.Is( SvHash_import() ) )
    {
        SvMetaClass * pClass = ReadKnownClass();
        SvClassElement aEle(pClass);
        rTok = rInStm.GetToken();
        if( rTok.IsString() )
        {
            aEle.SetPrefix( rTok.GetString() );
            rInStm.GetToken_Next();
        }
        rClass.aClassElementList.push_back( aEle );
        return;
    }
    else
    {
        rInStm.Seek( nTokPos );
        SvMetaType * pType = rBase.ReadKnownType( rInStm );
        tools::SvRef<SvMetaAttribute> xAttr;
        bool bOk = false;
        if( !pType || pType->IsItem() )
        {
            xAttr = new SvMetaSlot( pType );
            bOk = ReadInterfaceOrShellSlot(static_cast<SvMetaSlot&>(*xAttr));
        }
        else
        {
            xAttr = new SvMetaAttribute( pType );
            ReadInterfaceOrShellMethodOrAttribute(*xAttr);
            bOk = true;
        }
        if( bOk )
            bOk = xAttr->Test( rInStm );
        if( bOk )
            bOk = rClass.TestAttribute( rBase, rInStm, *xAttr );
        if( bOk )
        {
            if( !xAttr->GetSlotId().IsSet() )
                xAttr->SetSlotId( SvIdentifier(rBase.GetUniqueId()) );
            rClass.aAttrList.push_back( xAttr );
            return;
        }
    }
    rInStm.Seek( nTokPos );
}

bool SvIdlParser::ReadInterfaceOrShellSlot(SvMetaSlot& rSlot)
{
    sal_uInt32  nTokPos = rInStm.Tell();
    bool        bOk     = true;

    SvMetaAttribute * pAttr = rBase.ReadKnownAttr( rInStm, rSlot.GetType() );
    if( pAttr )
    {
        SvMetaSlot * pKnownSlot = dynamic_cast<SvMetaSlot*>( pAttr );
        if( !pKnownSlot )
            throw SvParseException( rInStm, "attribute " + pAttr->GetName() + " is method or variable but not a slot" );
        rSlot.SetRef( pKnownSlot );
        rSlot.SetName( pKnownSlot->GetName() );
        if( ReadIf( '[' ) )
        {
            sal_uInt32 nBeginPos = 0; // can not happen with Tell
            while( nBeginPos != rInStm.Tell() )
            {
                nBeginPos = rInStm.Tell();
                rSlot.ReadAttributesSvIdl( rBase, rInStm );
                ReadIfDelimiter();
            }
            bOk = ReadIf( ']' );
        }
    }
    else
    {
        bOk = rSlot.SvMetaAttribute::ReadSvIdl( rBase, rInStm );
        SvMetaAttribute *pAttr2 = rBase.FindKnownAttr( rSlot.GetSlotId() );
        if( pAttr2 )
        {
            SvMetaSlot * pKnownSlot = dynamic_cast<SvMetaSlot*>( pAttr2 );
            if( !pKnownSlot )
                throw SvParseException( rInStm, "attribute " + pAttr2->GetName() + " is method or variable but not a slot" );
            rSlot.SetRef( pKnownSlot );
            // names may differ, because explicitly given
            if ( pKnownSlot->GetName() != rSlot.GetName() )
                throw SvParseException( rInStm, "Illegal definition!" );
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );

    return bOk;
}

void SvIdlParser::ReadInterfaceOrShellMethodOrAttribute( SvMetaAttribute& rAttr )
{
    rAttr.SetName( ReadIdentifier() );
    rAttr.aSlotId.setString( ReadIdentifier() );
    sal_uLong n;
    if( !rBase.FindId( rAttr.aSlotId.getString(), &n ) )
        throw SvParseException( rInStm, "no value for identifier <" + rAttr.aSlotId.getString() + "> " );
    rAttr.aSlotId.SetValue(n);

    Read( '(' );
    // read method arguments
    tools::SvRef<SvMetaType> xT(new SvMetaType() );
    xT->SetRef(rAttr.GetType() );
    rAttr.aType = xT;
    sal_uInt32 nBeginPos = 0; // can not happen with Tell
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        tools::SvRef<SvMetaAttribute> xAttr( new SvMetaAttribute() );
        if( xAttr->ReadSvIdl( rBase, rInStm ) )
        {
            if( xAttr->Test( rInStm ) )
                rAttr.aType->GetAttrList().push_back( xAttr );
        }
        ReadIfDelimiter();
    }
    Read( ')' );
    rAttr.aType->SetType( MetaTypeType::Method );
}

SvMetaClass * SvIdlParser::ReadKnownClass()
{
    OString aName(ReadIdentifier());
    SvMetaClass* pClass = rBase.FindKnownClass( aName );
    if( !pClass )
        throw SvParseException( rInStm, "unknown class" );
    return pClass;
}

SvMetaType * SvIdlParser::ReadKnownType()
{
    OString aName = ReadIdentifier();
    for( const auto& aType : rBase.GetTypeList() )
    {
        if( aType->GetName() == aName )
        {
            return aType;
        }
    }
    throw SvParseException( rInStm, "wrong typedef: ");
}


void SvIdlParser::ReadDelimiter()
{
    if( !ReadIfDelimiter() )
        throw SvParseException(rInStm, "expected delimiter");
}

bool SvIdlParser::ReadIfDelimiter()
{
    if( rInStm.GetToken().IsChar()
        && (';' == rInStm.GetToken().GetChar()
             || ',' == rInStm.GetToken().GetChar()) )
    {
        rInStm.GetToken_Next();
        return true;
    }
    return false;
}

OString SvIdlParser::ReadIdentifier()
{
    SvToken& rTok = rInStm.GetToken();
    if( !rTok.IsIdentifier() )
        throw SvParseException("expected identifier", rTok);
    rInStm.GetToken_Next();
    return rTok.GetString();
}

OString SvIdlParser::ReadString()
{
    SvToken& rTok = rInStm.GetToken();
    if( !rTok.IsString() )
        throw SvParseException("expected string", rTok);
    rInStm.GetToken_Next();
    return rTok.GetString();
}

void SvIdlParser::Read(char cChar)
{
    if( !(rInStm.GetToken().IsChar() && rInStm.GetToken().GetChar() == cChar ) )
        throw SvParseException(rInStm, "expected char '" + OString(cChar) + "'");
    rInStm.GetToken_Next();
}

bool SvIdlParser::ReadIf(char cChar)
{
    if( rInStm.GetToken().IsChar() && rInStm.GetToken().GetChar() == cChar )
    {
        rInStm.GetToken_Next();
        return true;
    }
    return false;
}

void SvIdlParser::Read(SvStringHashEntry* entry)
{
    if( !rInStm.GetToken().Is(entry) )
        throw SvParseException("expected " + entry->GetName(), rInStm.GetToken());
    rInStm.GetToken_Next();
}

bool SvIdlParser::ReadIf(SvStringHashEntry* entry)
{
    if( rInStm.GetToken().Is(entry) )
    {
        rInStm.GetToken_Next();
        return true;
    }
    return false;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
