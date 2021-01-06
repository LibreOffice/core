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


#include <limits.h>
#include <bastype.hxx>
#include <lex.hxx>
#include <globals.hxx>
#include <hash.hxx>
#include <database.hxx>
#include <tools/stream.hxx>

bool SvBOOL::ReadSvIdl( SvStringHashEntry const * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken& rTok = rInStm.GetToken_Next();

    if( rTok.Is( pName ) )
    {
        if( rInStm.ReadIf( '=' ) )
        {
            rTok = rInStm.GetToken();
            if( !rTok.IsBool() )
                throw SvParseException(rInStm, "xxx");
            *this = rTok.GetBool();
            rInStm.GetToken_Next();
        }
        else
            *this = true; //default action set to TRUE
        return true;
    }
    rInStm.Seek( nTokPos );
    return false;
}

void SvIdentifier::ReadSvIdl( SvStringHashEntry const * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken& rTok = rInStm.GetToken_Next();

    if( rTok.Is( pName ) )
    {
        bool bOk = true;
        bool bBracket = rInStm.ReadIf( '(' );
        if( bBracket || rInStm.ReadIf( '=' ) )
        {
            rTok = rInStm.GetToken();
            if( rTok.IsIdentifier() )
            {
                setString(rTok.GetString());
                rInStm.GetToken_Next();
            }
            if( bOk && bBracket )
                bOk = rInStm.ReadIf( ')' );
        }
        if( bOk )
            return;
    }
    rInStm.Seek( nTokPos );
}

void SvIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken& rTok = rInStm.GetToken_Next();

    if( rTok.IsIdentifier() )
    {
        sal_uInt32 n;
        if( !rBase.FindId( rTok.GetString(), &n ) )
            rBase.SetAndWriteError( rInStm, "no value for identifier <" + getString() + "> " );
        setString(rTok.GetString());
        nValue = n;
        return;
    }
    rInStm.Seek( nTokPos );
}

bool ReadStringSvIdl( SvStringHashEntry const * pName, SvTokenStream & rInStm, OString& aRetString )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken& rTok = rInStm.GetToken_Next();

    if( rTok.Is( pName ) )
    {
        bool bOk = true;
        bool bBracket = rInStm.ReadIf( '(' );
        if( bBracket || rInStm.ReadIf( '=' ) )
        {
            rTok = rInStm.GetToken();
            if( rTok.IsString() )
            {
                aRetString = rTok.GetString();
                rInStm.GetToken_Next();
            }
            if( bOk && bBracket )
                bOk = rInStm.ReadIf( ')' );
        }
        if( bOk )
            return true;
    }
    rInStm.Seek( nTokPos );
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
