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

#include <buffer.hxx>
#include <parser.hxx>

const static sal_uInt32 UP_LIMIT=0xFFFFFF00;

// The SbiBuffer will be expanded in increments of at least 16 Bytes.
// This is necessary, because many classes emanate from a buffer length
// of x*16 Bytes.

SbiBuffer::SbiBuffer( SbiParser* p, short n )
{
    pParser = p;
    n = ( (n + 15 ) / 16 ) * 16;
    if( !n ) n = 16;
    pBuf  = nullptr;
    pCur  = nullptr;
    nInc  = n;
    nSize =
    nOff  = 0;
}

SbiBuffer::~SbiBuffer()
{
}

// Reach out the buffer
// This lead to the deletion of the buffer!

char* SbiBuffer::GetBuffer()
{
    char* p = pBuf.release();
    pCur = nullptr;
    return p;
}

// Test, if the buffer can contain n Bytes.
// In case of doubt it will be enlarged

bool SbiBuffer::Check( sal_Int32 n )
{
    if( !n )
    {
        return true;
    }
    if( nOff + n  >  nSize )
    {
        if( nInc == 0 )
        {
            return false;
        }

        sal_Int32 nn = 0;
        while( nn < n )
        {
            nn = nn + nInc;
        }
        char* p;
        if( ( nSize + nn ) > UP_LIMIT )
        {
            p = nullptr;
        }
        else
        {
            p = new char [nSize + nn];
        }
        if( !p )
        {
            pParser->Error( ERRCODE_BASIC_PROG_TOO_LARGE );
            nInc = 0;
            pBuf.reset();
            return false;
        }
        else
        {
            if( nSize ) memcpy( p, pBuf.get(), nSize );
            pBuf.reset(p);
            pCur = pBuf.get() + nOff;
            nSize = nSize + nn;
        }
    }
    return true;
}

// Patch of a Location

void SbiBuffer::Patch( sal_uInt32 off, sal_uInt32 val )
{
    if( ( off + sizeof( sal_uInt32 ) ) < nOff )
    {
        sal_uInt16 val1 = static_cast<sal_uInt16>( val & 0xFFFF );
        sal_uInt16 val2 = static_cast<sal_uInt16>( val >> 16 );
        sal_uInt8* p = reinterpret_cast<sal_uInt8*>(pBuf.get()) + off;
        *p++ = static_cast<char>( val1 & 0xFF );
        *p++ = static_cast<char>( val1 >> 8 );
        *p++ = static_cast<char>( val2 & 0xFF );
        *p   = static_cast<char>( val2 >> 8 );
    }
}

// Forward References upon label and procedures
// establish a linkage. The beginning of the linkage is at the passed parameter,
// the end of the linkage is 0.

void SbiBuffer::Chain( sal_uInt32 off )
{
    if( off && pBuf )
    {
        sal_uInt8 *ip;
        sal_uInt32 i = off;
        sal_uInt32 val1 = (nOff & 0xFFFF);
        sal_uInt32 val2 = (nOff >> 16);
        do
        {
            ip = reinterpret_cast<sal_uInt8*>(pBuf.get()) + i;
            sal_uInt8* pTmp = ip;
            i =  *pTmp++; i |= *pTmp++ << 8; i |= *pTmp++ << 16; i |= *pTmp++ << 24;

            if( i >= nOff )
            {
                pParser->Error( ERRCODE_BASIC_INTERNAL_ERROR, "BACKCHAIN" );
                break;
            }
            *ip++ = static_cast<char>( val1 & 0xFF );
            *ip++ = static_cast<char>( val1 >> 8 );
            *ip++ = static_cast<char>( val2 & 0xFF );
            *ip   = static_cast<char>( val2 >> 8 );
        } while( i );
    }
}

void SbiBuffer::operator +=( sal_Int8 n )
{
    if( Check( 1 ) )
    {
        *pCur++ = static_cast<char>(n);
        nOff += 1;
    }
}

bool SbiBuffer::operator +=( sal_uInt8 n )
{
    if( Check( 1 ) )
    {
        *pCur++ = static_cast<char>(n);
        nOff += 1;
        return true;
    }
    else
    {
        return false;
    }
}

void SbiBuffer::operator +=( sal_Int16 n )
{
    if( Check( 2 ) )
    {
        *pCur++ = static_cast<char>( n & 0xFF );
        *pCur++ = static_cast<char>( n >> 8 );
        nOff += 2;
    }
}

bool SbiBuffer::operator +=( sal_uInt16 n )
{
    if( Check( 2 ) )
    {
        *pCur++ = static_cast<char>( n & 0xFF );
        *pCur++ = static_cast<char>( n >> 8 );
        nOff += 2;
        return true;
    }
    else
    {
        return false;
    }
}

bool SbiBuffer::operator +=( sal_uInt32 n )
{
    if( Check( 4 ) )
    {
        sal_uInt16 n1 = static_cast<sal_uInt16>( n & 0xFFFF );
        sal_uInt16 n2 = static_cast<sal_uInt16>( n >> 16 );
        operator +=(n1) && operator +=(n2);
        return true;
    }
    else
    {
        return false;
    }
}

void SbiBuffer::operator +=( sal_Int32 n )
{
    operator +=( static_cast<sal_uInt32>(n) );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
