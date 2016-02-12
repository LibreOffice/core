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

bool SvIdlParser::ReadSvIdl( bool bImported, const OUString & rPath )
{
    rBase.SetPath(rPath); // only valid for this iteration
    bool bOk = true;
    SvToken& rTok = rInStm.GetToken();

    while( bOk )
    {
        rTok = rInStm.GetToken();
        if( rTok.IsEof() )
            return true;

        if( rTok.Is( SvHash_module() ) )
        {
            tools::SvRef<SvMetaModule> aModule = new SvMetaModule( bImported );
            if( ReadModuleHeader(*aModule) )
                rBase.GetModuleList().push_back( aModule );
            else
                bOk = false;
        }
        else
            bOk = false;
    }
    if( !bOk || !rTok.IsEof() )
    {
         // error treatment
         rBase.WriteError( rInStm );
         return false;
    }
    return true;
}

bool SvIdlParser::ReadModuleHeader(SvMetaModule& rModule)
{
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken&    rTok  = rInStm.GetToken_Next();
    bool        bOk = true;

    rTok = rInStm.GetToken_Next();
    if( !rTok.IsIdentifier() )
    {
        rInStm.Seek( nTokPos );
        return false;
    }
    rBase.Push( &rModule ); // onto the context stack
    rModule.SetName( rTok.GetString() );
    bOk = ReadModuleBody(rModule);
    rBase.GetStack().pop_back(); // remove from stack
    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

bool SvIdlParser::ReadModuleBody(SvMetaModule& rModule)
{
    sal_uInt32 nTokPos = rInStm.Tell();
    bool bOk = true;
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
        return bOk;

    sal_uInt32 nBeginPos = 0;
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        ReadModuleElement( rModule );
        rInStm.ReadIfDelimiter();
    }
    ReadChar( '}' );

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvIdlParser::ReadModuleElement( SvMetaModule& rModule )
{
    if( rInStm.GetToken().Is( SvHash_interface() )
      || rInStm.GetToken().Is( SvHash_shell() ) )
    {
        tools::SvRef<SvMetaClass> aClass( new SvMetaClass() );
        if( aClass->ReadSvIdl( rBase, rInStm ) )
        {
            rModule.aClassList.push_back( aClass );
            // announce globally
            rBase.GetClassList().push_back( aClass );
        }
    }
    else if( rInStm.GetToken().Is( SvHash_enum() ) )
    {
        ReadEnum();
    }
    else if( rInStm.GetToken().Is( SvHash_item() )
      || rInStm.GetToken().Is( SvHash_struct() ) )
    {
        tools::SvRef<SvMetaType> xItem(new SvMetaType() );

        if( xItem->ReadSvIdl( rBase, rInStm ) )
        {
            // announce globally
            rBase.GetTypeList().push_back( xItem );
        }
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

void SvIdlParser::ReadEnum()
{
    tools::SvRef<SvMetaTypeEnum> xEnum( new SvMetaTypeEnum() );
    rInStm.GetToken_Next();
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
    sal_uInt32 nTokPos = rInStm.Tell();

    tools::SvRef<SvMetaEnumValue> aEnumVal = new SvMetaEnumValue();
    bool bOk = aEnumVal->ReadSvIdl( rBase, rInStm );
    if( bOk )
    {
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
    if( !bOk )
        rInStm.Seek( nTokPos );
}



void SvIdlParser::ReadChar(char cChar)
{
    if( !rInStm.ReadIf( cChar ) )
        throw SvParseException(rInStm, "expected char '" + OString(cChar) + "'");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
