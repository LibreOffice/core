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

static bool ReadRangeSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm,
                            sal_uLong nMin, sal_uLong nMax, sal_uLong* pValue )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();
    if( pTok->Is( pName ) )
    {
        bool bOk = false;
        if( rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken_Next();
            if( pTok->IsInteger() )
            {
                sal_uLong n = pTok->GetNumber();
                if ( n >= nMin && n <= nMax )
                {
                    *pValue = n;
                    bOk = true;
                }
            }
        }

        if( bOk )
            return true;
    }

    rInStm.Seek( nTokPos );
    return false;
}

SvStream& operator >> (SvStream & rStm, SvBOOL & rb )
{
    sal_uInt8 n;
    rStm.ReadUChar( n );
    rb.nVal = (n & 0x01) != 0;
    rb.bSet = (n & 0x02) != 0;
    if( n & ~0x03 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "format error" );
    }
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvVersion & r )
{
    sal_uInt8 n;
    rStm.ReadUChar( n );
    if( n == 0 )
    { // not compressed
        rStm.ReadUInt16( r.nMajorVersion );
        rStm.ReadUInt16( r.nMinorVersion );
    }
    else
    { // compressed
        r.nMajorVersion = (n >> 4) & 0x0F;
        r.nMinorVersion = n & 0x0F;
    }
    return rStm;
}


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

bool SvIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
                                    SvStringHashEntry * pName,
                                    SvTokenStream & rInStm )
{
    if( SvIdentifier::ReadSvIdl( pName, rInStm ) )
    {
        sal_uLong n;
        if( rBase.FindId( getString(), &n ) )
        {
            nValue = n;
            return true;
        }
        else
        {
            OStringBuffer aStr("no value for identifier <");
            aStr.append(getString()).append("> ");
            rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    return false;
}

bool SvIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
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
            return true;
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
    return false;
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

SvStream& operator >> (SvStream & rStm, SvString & r )
{
    r.setString(read_uInt16_lenPrefixed_uInt8s_ToOString(rStm));
    return rStm;
}


bool SvHelpText::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
{
    return SvString::ReadSvIdl( SvHash_HelpText(), rInStm );
}

bool SvUUId::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_uuid() ) )
    {
        bool bOk = true;
        bool bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = &rInStm.GetToken();
            if( pTok->IsString() )
            {
                pTok = rInStm.GetToken_Next();
                bOk = MakeId(OStringToOUString(pTok->GetString(), RTL_TEXTENCODING_ASCII_US));
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

bool SvVersion::ReadSvIdl( SvTokenStream & rInStm )
{
    sal_uLong n = 0;

    sal_uInt32 nTokPos = rInStm.Tell();
    if( ReadRangeSvIdl( SvHash_Version(), rInStm, 0 , 0xFFFF, &n ) )
    {
        nMajorVersion = (sal_uInt16)n;
        if( rInStm.Read( '.' ) )
        {
            SvToken * pTok = rInStm.GetToken_Next();
            if( pTok->IsInteger() && pTok->GetNumber() <= 0xFFFF )
            {
                nMinorVersion = (sal_uInt16)pTok->GetNumber();
                return true;
            }
        }
        else
            return true;
    }
    rInStm.Seek( nTokPos );
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
