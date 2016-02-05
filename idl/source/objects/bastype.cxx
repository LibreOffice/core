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
#include <ctype.h>
#include <bastype.hxx>
#include <lex.hxx>
#include <globals.hxx>
#include <hash.hxx>
#include <database.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>

bool SvBOOL::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        bool bOk = true;
        bool bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = &rInStm.GetToken();
            if( pTok->IsBool() )
            {
                *this = pTok->GetBool();

                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        else
            *this = true; //default action set to TRUE
        if( bOk )
            return true;
    }
    rInStm.Seek( nTokPos );
    return false;
}

bool SvIdentifier::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        bool bOk = true;
        bool bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = &rInStm.GetToken();
            if( pTok->IsIdentifier() )
            {
                setString(pTok->GetString());
                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        if( bOk )
            return true;
    }
    rInStm.Seek( nTokPos );
    return false;
}

void SvIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->IsIdentifier() )
    {
        sal_uLong n;
        if( rBase.FindId( pTok->GetString(), &n ) )
        {
            setString(pTok->GetString());
            nValue = n;
            return;
        }
        else
        {
            OStringBuffer aStr("no value for identifier <");
            aStr.append(getString()).append("> ");
            rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    rInStm.Seek( nTokPos );
}

bool SvString::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        bool bOk = true;
        bool bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = &rInStm.GetToken();
            if( pTok->IsString() )
            {
                setString(pTok->GetString());
                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        if( bOk )
            return true;
    }
    rInStm.Seek( nTokPos );
    return false;
}

void SvHelpText::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
{
    SvString::ReadSvIdl( SvHash_HelpText(), rInStm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
