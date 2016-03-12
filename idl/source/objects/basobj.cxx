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

#include <tools/debug.hxx>

#include <basobj.hxx>
#include <module.hxx>
#include <globals.hxx>
#include <database.hxx>

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

void SvMetaObject::Back2Delimiter( SvStream & rOutStm )
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

SvMetaObject::SvMetaObject()
{
}

void SvMetaObject::SetName( const OString& rName )
{
    aName = rName;
}

bool SvMetaObject::ReadNameSvIdl( SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken& rTok = rInStm.GetToken_Next();

    // read module name
    if( rTok.IsIdentifier() )
    {
        SetName( rTok.GetString() );
        return true;
    }

    rInStm.Seek( nTokPos );
    return false;
}

void SvMetaObject::ReadAttributesSvIdl( SvIdlDataBase & ,
                                      SvTokenStream & )
{
}

void SvMetaObject::DoReadContextSvIdl( SvIdlDataBase & rBase,
                                   SvTokenStream & rInStm )
{
    sal_uInt32 nBeginPos = 0; // can not happen with Tell
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        ReadContextSvIdl( rBase, rInStm );
        rInStm.ReadIfDelimiter();
    }
}

void SvMetaObject::ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & )
{
}

bool SvMetaObject::Test( SvTokenStream & )
{
    return true;
}

bool SvMetaObject::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    bool bOk = true;
    if( rInStm.ReadIf( '[' ) )
    {
        sal_uInt32 nBeginPos = 0; // can not happen with Tell
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            ReadAttributesSvIdl( rBase, rInStm );
            rInStm.ReadIfDelimiter();
        }
        bOk = rInStm.ReadIf( ']' );
    }

    if( bOk )
    {
        if( rInStm.ReadIf( '{' ) )
        {
            DoReadContextSvIdl( rBase, rInStm );
            bOk = rInStm.ReadIf( '}' );
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}


SvMetaReference::SvMetaReference()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
