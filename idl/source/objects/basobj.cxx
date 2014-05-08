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


#include <ctype.h>
#include <stdio.h>

#include <tools/debug.hxx>

#include <basobj.hxx>
#include <module.hxx>
#include <globals.hxx>
#include <database.hxx>

SV_IMPL_META_FACTORY1( SvMetaObject, SvRttiBase )
SvMetaObject::SvMetaObject()
{
}

void SvMetaObject::Load( SvPersistStream & )
{
}

void SvMetaObject::Save( SvPersistStream & )
{
}

void SvMetaObject::WriteTab( SvStream & rOutStm, sal_uInt16 nTab )
{
    while( nTab-- )
        rOutStm.WriteCharPtr( "    " );
}

void SvMetaObject::WriteStars( SvStream & rOutStm )
{
    rOutStm.WriteChar( '/' );
    for( int i = 6; i > 0; i-- )
        rOutStm.WriteCharPtr( "**********" );
    rOutStm.WriteChar( '/' ) << endl;
}

bool SvMetaObject::TestAndSeekSpaceOnly( SvStream & rOutStm, sal_uLong nBegPos )
{
    // write no empty brackets
    sal_uLong nPos = rOutStm.Tell();
    rOutStm.Seek( nBegPos );
    bool bOnlySpace = true;
    while( bOnlySpace && rOutStm.Tell() < nPos )
    {
        char c;
        rOutStm.ReadChar( c );
        if( !isspace( c ) )
            bOnlySpace = false;
    }
    if( bOnlySpace )
        // nothing written
        rOutStm.Seek( nBegPos );
    else
        rOutStm.Seek( nPos );
    return bOnlySpace;
}

void SvMetaObject::Back2Delemitter( SvStream & rOutStm )
{
    // write no empty brackets
    sal_uLong nPos = rOutStm.Tell();
    rOutStm.SeekRel( -1 );
    char c = 0;
    rOutStm.ReadChar( c );

    while( isspace( c ) && rOutStm.Tell() != 1 )
    {
        rOutStm.SeekRel( -2 );
        rOutStm.ReadChar( c );
    }

    if( c == ';' || c == ',' )
        rOutStm.SeekRel( -1 );
    else
        rOutStm.Seek( nPos );
}

bool SvMetaObject::ReadSvIdl( SvIdlDataBase &, SvTokenStream & )
{
    return false;
}

void SvMetaObject::WriteSvIdl( SvIdlDataBase &, SvStream &, sal_uInt16 /*nTab */ )
{
}

void SvMetaObject::Write( SvIdlDataBase &, SvStream &, sal_uInt16 /*nTab */,
                             WriteType, WriteAttribute )
{
}

SV_IMPL_META_FACTORY1( SvMetaName, SvMetaObject );
SvMetaName::SvMetaName()
{
}

void SvMetaName::Load( SvPersistStream & rStm )
{
    SvMetaObject::Load( rStm );
    sal_uInt8 nMask;
    rStm.ReadUChar( nMask );

    if( nMask >= 0x20 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 )  rStm >> aName;
    if( nMask & 0x02 )  rStm >> aHelpContext;
    if( nMask & 0x04 )  rStm >> aHelpText;
    if( nMask & 0x08 )  rStm >> aConfigName;
    if( nMask & 0x10 )  rStm >> aDescription;
}

void SvMetaName::Save( SvPersistStream & rStm )
{
    SvMetaObject::Save( rStm );
    sal_uInt8 nMask = 0;
    if( aName.IsSet() )         nMask |= 0x01;
    if( aHelpContext.IsSet() )  nMask |= 0x02;
    if( aHelpText.IsSet() )     nMask |= 0x04;
    if( aConfigName.IsSet() )   nMask |= 0x08;
    if( aDescription.IsSet() )  nMask |= 0x10;

    rStm.WriteUChar( nMask );
    if( nMask & 0x01 ) WriteSvString( rStm, aName );
    if( nMask & 0x02 ) WriteSvNumberIdentifier( rStm, aHelpContext );
    if( nMask & 0x04 ) WriteSvString( rStm, aHelpText );
    if( nMask & 0x08 ) WriteSvString( rStm, aConfigName );
    if( nMask & 0x10 ) WriteSvString( rStm, aDescription );
}

bool SvMetaName::SetName( const OString& rName, SvIdlDataBase * )
{
    aName.setString(rName);
    return true;
}

bool SvMetaName::ReadNameSvIdl( SvIdlDataBase & rBase,
                                SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    // read module name
    if( pTok->IsIdentifier() )
        if( SetName( pTok->GetString(), &rBase ) )
            return true;

    rInStm.Seek( nTokPos );
    return false;
}

void SvMetaName::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    if( aName.ReadSvIdl( SvHash_Name(), rInStm ) )
    {
        if( !SetName( aName.getString(), &rBase ) )
            rInStm.Seek( nTokPos );
    }

    aHelpContext.ReadSvIdl( rBase, SvHash_HelpContext(), rInStm );
    aHelpText.ReadSvIdl( rBase, rInStm );
    aConfigName.ReadSvIdl( SvHash_ConfigName(), rInStm );
    aDescription.ReadSvIdl( SvHash_Description(), rInStm );
}

void SvMetaName::DoReadContextSvIdl( SvIdlDataBase & rBase,
                                   SvTokenStream & rInStm, char cDel )
{
    sal_uInt32 nBeginPos = 0; // can not happen with Tell
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        ReadContextSvIdl( rBase, rInStm );
        if( cDel == '\0' )
            rInStm.ReadDelemiter();
        else
            rInStm.Read( cDel );
    }
}

void SvMetaName::ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & )
{
}

bool SvMetaName::Test( SvIdlDataBase &, SvTokenStream & )
{
    return true;
}

void SvMetaName::WriteContextSvIdl( SvIdlDataBase &, SvStream &, sal_uInt16 )
{
}

void SvMetaName::WriteDescription( SvStream & rOutStm )
{
    rOutStm.WriteCharPtr( "<DESCRIPTION>" ) << endl;

    OString aDesc( GetDescription().getString() );
    sal_Int32 nPos = aDesc.indexOf('\n');
    while ( nPos != -1 )
    {
        rOutStm.WriteCharPtr( aDesc.copy( 0, nPos ).getStr() ) << endl;
        aDesc = aDesc.copy(nPos+1);
        nPos = aDesc.indexOf('\n');
    }

    rOutStm.WriteCharPtr( aDesc.getStr() ) << endl;
    rOutStm.WriteCharPtr( "</DESCRIPTION>" ) << endl;
}

void SvMetaName::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                       SvStream & rOutStm,
                                       sal_uInt16 nTab )
{
    if( aHelpContext.IsSet() || aHelpText.IsSet() || aConfigName.IsSet() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "// class SvMetaName" ) << endl;
    }
    if( aHelpContext.IsSet() )
    {
        WriteTab( rOutStm, nTab );
        aHelpContext.WriteSvIdl( SvHash_HelpContext(), rOutStm, nTab );
        rOutStm.WriteChar( ';' ) << endl;
    }
    if( aHelpText.IsSet() )
    {
        WriteTab( rOutStm, nTab );
        aHelpText.WriteSvIdl( rBase, rOutStm, nTab );
        rOutStm.WriteChar( ';' ) << endl;
    }
    if( aConfigName.IsSet() )
    {
        WriteTab( rOutStm, nTab );
        aConfigName.WriteSvIdl( SvHash_ConfigName(), rOutStm, nTab );
        rOutStm.WriteChar( ';' ) << endl;
    }
}

bool SvMetaName::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    bool bOk = true;
    if( rInStm.Read( '[' ) )
    {
        sal_uInt32 nBeginPos = 0; // can not happen with Tell
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            ReadAttributesSvIdl( rBase, rInStm );
            rInStm.ReadDelemiter();
        }
        bOk = rInStm.Read( ']' );
    }

    if( bOk )
    {
        if( rInStm.Read( '{' ) )
        {
            DoReadContextSvIdl( rBase, rInStm );
            bOk = rInStm.Read( '}' );
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvMetaName::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                             sal_uInt16 nTab )
{
    sal_uLong nBeginPos = rOutStm.Tell();
    WriteTab( rOutStm, nTab );
    rOutStm.WriteChar( '[' ) << endl;
    sal_uLong nOldPos = rOutStm.Tell();
    WriteAttributesSvIdl( rBase, rOutStm, nTab +1 );

    // write no empty brackets
    if( TestAndSeekSpaceOnly( rOutStm, nOldPos ) )
        // nothing written
        rOutStm.Seek( nBeginPos );
    else
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteChar( ']' );
        nBeginPos = rOutStm.Tell();
          rOutStm << endl;
    }

    WriteTab( rOutStm, nTab );
    rOutStm.WriteChar( '{' ) << endl;
    nOldPos = rOutStm.Tell();
    WriteContextSvIdl( rBase, rOutStm, nTab +1 );

    // write no empty brackets
    if( TestAndSeekSpaceOnly( rOutStm, nOldPos ) )
        // nothing written
        rOutStm.Seek( nBeginPos );
    else
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteChar( '}' );
    }
}

void SvMetaName::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                           sal_uInt16 nTab,
                         WriteType nT, WriteAttribute nA )
{
    sal_uLong nBeginPos = rOutStm.Tell();
    WriteTab( rOutStm, nTab );
    rOutStm.WriteChar( '[' ) << endl;
    sal_uLong nOldPos = rOutStm.Tell();
    WriteAttributes( rBase, rOutStm, nTab +1, nT, nA );

    // write no empty brackets
    sal_uLong nPos = rOutStm.Tell();
    rOutStm.Seek( nOldPos );
    bool bOnlySpace = true;
    while( bOnlySpace && rOutStm.Tell() < nPos )
    {
        char c;
        rOutStm.ReadChar( c );
        if( !isspace( c ) )
            bOnlySpace = false;
    }
    if( bOnlySpace )
        // nothing written
        rOutStm.Seek( nBeginPos );
    else
    {
        rOutStm.Seek( nPos );
        WriteTab( rOutStm, nTab );
        rOutStm.WriteChar( ']' ) << endl;
    }
}

void SvMetaName::WriteAttributes( SvIdlDataBase &, SvStream & rOutStm,
                                sal_uInt16 nTab,
                                 WriteType, WriteAttribute )
{
    if( GetHelpText().IsSet() || GetHelpContext().IsSet() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "// class SvMetaName" ) << endl;
    }
    if( GetHelpText().IsSet() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "helpstring(\"" ).WriteCharPtr( GetHelpText().getString().getStr() ).WriteCharPtr( "\")," ) << endl;
    }
    if( GetHelpContext().IsSet() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "helpcontext(" )
               .WriteCharPtr( OString::number(GetHelpContext().GetValue()).getStr() )
               .WriteCharPtr( ")," ) << endl;
    }
}

void SvMetaName::WriteContext( SvIdlDataBase &, SvStream &,
                                sal_uInt16,
                                 WriteType, WriteAttribute )
{
}

SV_IMPL_META_FACTORY1( SvMetaReference, SvMetaName );

SvMetaReference::SvMetaReference()
{
}

void SvMetaReference::Load( SvPersistStream & rStm )
{
    SvMetaName::Load( rStm );

    sal_uInt8 nMask;
    rStm.ReadUChar( nMask );
    if( nMask >= 0x2 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 )
    {
        SvMetaReference * pRef;
        rStm >> pRef;
        aRef = pRef;
    }
}

void SvMetaReference::Save( SvPersistStream & rStm )
{
    SvMetaName::Save( rStm );

    // create mask
    sal_uInt8 nMask = 0;
    if( aRef.Is() )
        nMask |= 0x01;

    // write data
    rStm.WriteUChar( nMask );
    if( nMask & 0x01 ) WriteSvPersistBase( rStm, aRef );
}

SV_IMPL_META_FACTORY1( SvMetaExtern, SvMetaReference );

SvMetaExtern::SvMetaExtern()
    : pModule( NULL )
    , bReadUUId( false )
    , bReadVersion( false )
{
}

void SvMetaExtern::Load( SvPersistStream & rStm )
{
    SvMetaReference::Load( rStm );

    sal_uInt8 nMask;
    rStm.ReadUChar( nMask );
    if( nMask >= 0x20 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) rStm >> pModule;
    if( nMask & 0x02 ) rStm >> aUUId;
    if( nMask & 0x04 ) rStm >> aVersion;
    if( nMask & 0x08 ) bReadUUId = true;
    if( nMask & 0x10 ) bReadVersion = true;
}

void SvMetaExtern::Save( SvPersistStream & rStm )
{
    SvMetaReference::Save( rStm );

    // create mask
    sal_uInt8 nMask = 0;
    if( pModule )                   nMask |= 0x01;
    if( aUUId != SvGlobalName() )   nMask |= 0x02;
    if( aVersion != SvVersion() )   nMask |= 0x04;
    if( bReadUUId )                 nMask |= 0x08;
    if( bReadVersion )              nMask |= 0x10;

    // write data
    rStm.WriteUChar( nMask );
    if( nMask & 0x01 ) WriteSvPersistBase( rStm, pModule );
    if( nMask & 0x02 ) WriteSvGlobalName( rStm, aUUId );
    if( nMask & 0x04 ) WriteSvVersion( rStm, aVersion );
}

SvMetaModule * SvMetaExtern::GetModule() const
{
    DBG_ASSERT( pModule != NULL, "module not set" );
    return pModule;
}

const SvGlobalName & SvMetaExtern::GetUUId() const
{
    if( aUUId == SvGlobalName() )
        GetModule()->FillNextName( &((SvMetaExtern *)this)->aUUId );
    return aUUId;
}

void SvMetaExtern::SetModule( SvIdlDataBase & rBase )
{
    pModule = (SvMetaModule *)rBase.GetStack().Get( TYPE( SvMetaModule ) );
}

void SvMetaExtern::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaReference::ReadAttributesSvIdl( rBase, rInStm );
    if( aUUId.ReadSvIdl( rBase, rInStm ) )
        bReadUUId = true;
    if( aVersion.ReadSvIdl( rInStm ) )
        bReadVersion = true;
}

void SvMetaExtern::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                         SvStream & rOutStm, sal_uInt16 nTab )
{
    SvMetaReference::WriteAttributesSvIdl( rBase, rOutStm, nTab );
    if( bReadUUId || bReadVersion )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "// class SvMetaExtern" ) << endl;

        if( bReadUUId )
        {
            WriteTab( rOutStm, nTab );
            aUUId.WriteSvIdl( rOutStm );
            rOutStm.WriteChar( ';' ) << endl;
        }
        if( bReadVersion )
        {
            WriteTab( rOutStm, nTab );
            aVersion.WriteSvIdl( rOutStm );
            rOutStm.WriteChar( ';' ) << endl;
        }
    }
}

bool SvMetaExtern::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    SetModule( rBase );
    GetUUId(); // id gets created
    return SvMetaReference::ReadSvIdl( rBase, rInStm );
}

void SvMetaExtern::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                               sal_uInt16 nTab )
{
    SvMetaReference::WriteSvIdl( rBase, rOutStm, nTab );
}

void SvMetaExtern::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                        sal_uInt16 nTab,
                         WriteType nT, WriteAttribute nA )
{
    SvMetaReference::Write( rBase, rOutStm, nTab, nT, nA );
}

void SvMetaExtern::WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm,
                                     sal_uInt16 nTab,
                                     WriteType nT, WriteAttribute nA )
{
    SvMetaReference::WriteAttributes( rBase, rOutStm, nTab, nT, nA );

    WriteTab( rOutStm, nTab );
    rOutStm.WriteCharPtr( "// class SvMetaExtern" ) << endl;
    WriteTab( rOutStm, nTab );
    rOutStm.WriteCharPtr( "uuid(" ).WriteCharPtr( OUStringToOString(GetUUId().GetHexName(), RTL_TEXTENCODING_UTF8).getStr() ).WriteCharPtr( ")," ) << endl;
    WriteTab( rOutStm, nTab );
    rOutStm.WriteCharPtr( "version(" )
       .WriteCharPtr( OString::number(aVersion.GetMajorVersion()).getStr() )
       .WriteChar( '.' )
       .WriteCharPtr( OString::number(aVersion.GetMinorVersion()).getStr() )
       .WriteCharPtr( ")," ) << endl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
