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
        sal_uInt32 nBeginPos = 0; // can not happen with Tell
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            OString aSlotIdFile;
            if( ReadStringSvIdl( SvHash_SlotIdFile(), rInStm, aSlotIdFile ) )
            {
                if( !rBase.ReadIdFile( OStringToOUString(aSlotIdFile, RTL_TEXTENCODING_ASCII_US)) )
                {
                    throw SvParseException( rInStm, "cannot read file: " + aSlotIdFile );
                }
            }
            rInStm.ReadIfDelimiter();
        }
        bOk = rInStm.ReadIf( ']' );
    }

    if( !bOk )
    {
        rInStm.Seek( nTokPos );
        return bOk;
    }

    if( !rInStm.ReadIf( '{' ) )
        return bOk;

    sal_uInt32 nBeginPos = 0; // can not happen with Tell
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        ReadModuleElement( rModule );
        rInStm.ReadIfDelimiter();
    }
    bOk = rInStm.ReadIf( '}' );

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvIdlParser::ReadModuleElement( SvMetaModule& rModule )
{
    sal_uInt32  nTokPos = rInStm.Tell();
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
        tools::SvRef<SvMetaTypeEnum> aEnum( new SvMetaTypeEnum() );

        if( aEnum->ReadSvIdl( rBase, rInStm ) )
        {
            // announce globally
            rBase.GetTypeList().push_back( aEnum );
        }
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
        bool bOk = false;
        rInStm.GetToken_Next();
        SvToken& rTok = rInStm.GetToken_Next();
        if( rTok.IsString() )
        {
            OUString aFullName(OStringToOUString(rTok.GetString(), RTL_TEXTENCODING_ASCII_US));
            rBase.StartNewFile( aFullName );
            osl::FileBase::RC searchError = osl::File::searchFileURL(aFullName, rBase.GetPath(), aFullName);
            if( osl::FileBase::E_None != searchError )
            {
                OStringBuffer aStr("cannot find file:");
                aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
                throw SvParseException(aStr.makeStringAndClear(), rTok);
            }
            osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );
            rBase.AddDepFile( aFullName );
            SvTokenStream aTokStm( aFullName );

            if( SVSTREAM_OK != aTokStm.GetStream().GetError() )
            {
                OStringBuffer aStr("cannot open file: ");
                aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
                throw SvParseException(aStr.makeStringAndClear(), rTok);
            }
            // rescue error from old file
            SvIdlError aOldErr = rBase.GetError();
            // reset error
            rBase.SetError( SvIdlError() );

            SvIdlParser aIncludeParser( rBase, aTokStm );
            sal_uInt32 nBeginPos = 0xFFFFFFFF; // can not happen with Tell
            while( nBeginPos != aTokStm.Tell() )
            {
                nBeginPos = aTokStm.Tell();
                aIncludeParser.ReadModuleElement(rModule);
                aTokStm.ReadIfDelimiter();
            }
            bOk = aTokStm.GetToken().IsEof();
            if( !bOk )
            {
                rBase.WriteError( aTokStm );
            }
            // recover error from old file
            rBase.SetError( aOldErr );
        }
        if( !bOk )
            rInStm.Seek( nTokPos );
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
