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

TYPEINIT1( SvMetaObject, SvRttiBase )
SvMetaObject::SvMetaObject()
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

void SvMetaObject::Write( SvIdlDataBase &, SvStream &, sal_uInt16 /*nTab */,
                             WriteType, WriteAttribute )
{
}

TYPEINIT1( SvMetaName, SvMetaObject );
SvMetaName::SvMetaName()
{
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

TYPEINIT1( SvMetaReference, SvMetaName );

SvMetaReference::SvMetaReference()
{
}

TYPEINIT1( SvMetaExtern, SvMetaReference );

SvMetaExtern::SvMetaExtern()
    : pModule( NULL )
    , bReadUUId( false )
    , bReadVersion( false )
{
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
    pModule = static_cast<SvMetaModule *>(rBase.GetStack().Get( TYPE( SvMetaModule ) ));
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

bool SvMetaExtern::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    SetModule( rBase );
    GetUUId(); // id gets created
    return SvMetaReference::ReadSvIdl( rBase, rInStm );
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
