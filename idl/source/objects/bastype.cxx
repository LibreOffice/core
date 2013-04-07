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

static sal_Bool ReadRangeSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm,
                            sal_uLong nMin, sal_uLong nMax, sal_uLong* pValue )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();
    if( pTok->Is( pName ) )
    {
        sal_Bool bOk = sal_False;
        if( rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken_Next();
            if( pTok->IsInteger() )
            {
                sal_uLong n = pTok->GetNumber();
                if ( n >= nMin && n <= nMax )
                {
                    *pValue = n;
                    bOk = sal_True;
                }
            }
        }

        if( bOk )
            return sal_True;
    }

    rInStm.Seek( nTokPos );
    return sal_False;
}

sal_uInt32 SvUINT32::Read( SvStream & rStm )
{
    return SvPersistStream::ReadCompressed( rStm );
}

void SvUINT32::Write( SvStream & rStm, sal_uInt32 nVal )
{
    SvPersistStream::WriteCompressed( rStm, nVal );
}

SvStream& operator << (SvStream & rStm, const SvBOOL & rb )
{
    sal_uInt8 n = rb.nVal;
    if( rb.bSet )
        n |= 0x02;
    rStm << n;
    return rStm;
}
SvStream& operator >> (SvStream & rStm, SvBOOL & rb )
{
    sal_uInt8 n;
    rStm >> n;
    rb.nVal = (n & 0x01) ? sal_True : sal_False;
    rb.bSet = (n & 0x02) ? sal_True : sal_False;
    if( n & ~0x03 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "format error" );
    }
    return rStm;
}

SvStream& operator << (SvStream & rStm, const SvVersion & r )
{
    if( (r.GetMajorVersion() || r.GetMinorVersion())
      && r.GetMajorVersion() <= 0x0F && r.GetMinorVersion() <= 0x0F )
    { // compress version number in 1 byte
        // format first 4 bit for major, then 4 bit for minor
        // 0.0 gets not compressed

        int n = r.GetMajorVersion() << 4;
        n |= r.GetMinorVersion();
        rStm << (sal_uInt8)n;
    }
    else
    {
        rStm << (sal_uInt8)0;
        rStm << r.GetMajorVersion();
        rStm << r.GetMinorVersion();
    }
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvVersion & r )
{
    sal_uInt8 n;
    rStm >> n;
    if( n == 0 )
    { // not compressed
        rStm >> r.nMajorVersion;
        rStm >> r.nMinorVersion;
    }
    else
    { // compressed
        r.nMajorVersion = (n >> 4) & 0x0F;
        r.nMinorVersion = n & 0x0F;
    }
    return rStm;
}


sal_Bool SvBOOL::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        sal_Bool bOk = sal_True;
        sal_Bool bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsBool() )
            {
                *this = pTok->GetBool();

                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        else
            *this = sal_True; //default action set to TRUE
        if( bOk )
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

sal_Bool SvBOOL::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm )
{
    if( nVal )
        rOutStm << pName->GetName().getStr();
    else
        rOutStm << pName->GetName().getStr() << "(FALSE)";
    return sal_True;
}

OString SvBOOL::GetSvIdlString( SvStringHashEntry * pName )
{
    if( nVal )
        return pName->GetName();

    return OStringBuffer(pName->GetName()).
        append(RTL_CONSTASCII_STRINGPARAM("(FALSE)")).
        makeStringAndClear();
}

sal_Bool SvIdentifier::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        sal_Bool bOk = sal_True;
        sal_Bool bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsIdentifier() )
            {
                setString(pTok->GetString());
                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        if( bOk )
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

sal_Bool SvIdentifier::WriteSvIdl( SvStringHashEntry * pName,
                               SvStream & rOutStm,
                               sal_uInt16 /*nTab */ )
{
    rOutStm << pName->GetName().getStr() << '(';
    rOutStm << getString().getStr() << ')';
    return sal_True;
}

SvStream& operator << (SvStream & rStm, const SvIdentifier & r )
{
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStm, r.getString());
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvIdentifier & r )
{
    r.setString(read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStm));
    return rStm;
}


sal_Bool SvNumberIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
                                    SvStringHashEntry * pName,
                                    SvTokenStream & rInStm )
{
    if( SvIdentifier::ReadSvIdl( pName, rInStm ) )
    {
        sal_uLong n;
        if( rBase.FindId( getString(), &n ) )
        {
            nValue = n;
            return sal_True;
        }
        else
        {
            OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                "no value for identifier <"));
            aStr.append(getString()).append(RTL_CONSTASCII_STRINGPARAM("> "));
            rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    return sal_False;
}

sal_Bool SvNumberIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
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
            return sal_True;
        }
        else
        {
            OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                "no value for identifier <"));
            aStr.append(getString()).append(RTL_CONSTASCII_STRINGPARAM("> "));
            rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

SvStream& operator << (SvStream & rStm, const SvNumberIdentifier & r )
{
    rStm << (SvIdentifier &)r;
    SvPersistStream::WriteCompressed( rStm, r.nValue );
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvNumberIdentifier & r )
{
    rStm >> (SvIdentifier &)r;
    r.nValue = SvPersistStream::ReadCompressed( rStm );
    return rStm;
}


sal_Bool SvString::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        sal_Bool bOk = sal_True;
        sal_Bool bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsString() )
            {
                setString(pTok->GetString());
                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        if( bOk )
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

sal_Bool SvString::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                           sal_uInt16 /*nTab */ )
{
    rOutStm << pName->GetName().getStr() << "(\"";
    rOutStm << m_aStr.getStr() << "\")";
    return sal_True;
}

SvStream& operator << (SvStream & rStm, const SvString & r )
{
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStm, r.getString());
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvString & r )
{
    r.setString(read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStm));
    return rStm;
}


sal_Bool SvHelpText::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
{
    return SvString::ReadSvIdl( SvHash_HelpText(), rInStm );
}

sal_Bool SvHelpText::WriteSvIdl( SvIdlDataBase &, SvStream & rOutStm, sal_uInt16 nTab )
{
    return SvString::WriteSvIdl( SvHash_HelpText(), rOutStm, nTab );
}

sal_Bool SvUUId::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_uuid() ) )
    {
        sal_Bool bOk = sal_True;
        sal_Bool bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsString() )
            {
                pTok = rInStm.GetToken_Next();
                bOk = MakeId(OStringToOUString(pTok->GetString(), RTL_TEXTENCODING_ASCII_US));
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        if( bOk )
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

sal_Bool SvUUId::WriteSvIdl( SvStream & rOutStm )
{
    // write global id
    rOutStm << SvHash_uuid()->GetName().getStr() << "(\"";
    rOutStm << OUStringToOString(GetHexName(), RTL_TEXTENCODING_UTF8).getStr() << "\")";
    return sal_True;
}


sal_Bool SvVersion::ReadSvIdl( SvTokenStream & rInStm )
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
                return sal_True;
            }
        }
        else
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

sal_Bool SvVersion::WriteSvIdl( SvStream & rOutStm )
{
    rOutStm << SvHash_Version()->GetName().getStr() << '('
        << OString::number(nMajorVersion).getStr()
        << '.'
        << OString::number(nMinorVersion).getStr()
        << ')';
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
