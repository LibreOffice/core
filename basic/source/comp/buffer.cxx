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


#include "buffer.hxx"
#include "sbcomp.hxx"

const static sal_uInt32 UP_LIMIT=0xFFFFFF00L;

// The SbiBuffer will be expanded in increments of at least 16 Bytes.
// This is necessary, because many classes emanate from a buffer length
// of x*16 Bytes.

SbiBuffer::SbiBuffer( SbiParser* p, short n )
{
    pParser = p;
    n = ( (n + 15 ) / 16 ) * 16;
    if( !n ) n = 16;
    pBuf  = NULL;
    pCur  = NULL;
    nInc  = n;
    nSize =
    nOff  = 0;
}

SbiBuffer::~SbiBuffer()
{
    delete[] pBuf;
}

// Reach out the buffer
// This lead to the deletion of the buffer!

char* SbiBuffer::GetBuffer()
{
    char* p = pBuf;
    pBuf = NULL;
    pCur = NULL;
    return p;
}

// Test, if the buffer can contain n Bytes.
// In case of doubt it will be enlarged

bool SbiBuffer::Check( sal_uInt16 n )
{
    if( !n ) return true;
    if( ( static_cast<sal_uInt32>( nOff )+ n ) >  static_cast<sal_uInt32>( nSize ) )
    {
        if( nInc == 0 )
            return false;
        sal_uInt16 nn = 0;
        while( nn < n ) nn = nn + nInc;
        char* p;
        if( ( static_cast<sal_uInt32>( nSize ) + nn ) > UP_LIMIT ) p = NULL;
        else p = new char [nSize + nn];
        if( !p )
        {
            pParser->Error( SbERR_PROG_TOO_LARGE );
            nInc = 0;
            delete[] pBuf; pBuf = NULL;
            return false;
        }
        else
        {
            if( nSize ) memcpy( p, pBuf, nSize );
            delete[] pBuf;
            pBuf = p;
            pCur = pBuf + nOff;
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
        sal_uInt8* p = (sal_uInt8*) pBuf + off;
        *p++ = (char) ( val1 & 0xFF );
        *p++ = (char) ( val1 >> 8 );
        *p++ = (char) ( val2 & 0xFF );
        *p   = (char) ( val2 >> 8 );
    }
}

// Forward References upon label und procedures
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
            ip = (sal_uInt8*) pBuf + i;
            sal_uInt8* pTmp = ip;
                     i =  *pTmp++; i |= *pTmp++ << 8; i |= *pTmp++ << 16; i |= *pTmp++ << 24;

            if( i >= nOff )
            {
                pParser->Error( SbERR_INTERNAL_ERROR, "BACKCHAIN" );
                break;
            }
            *ip++ = (char) ( val1 & 0xFF );
            *ip++ = (char) ( val1 >> 8 );
            *ip++ = (char) ( val2 & 0xFF );
            *ip   = (char) ( val2 >> 8 );
        } while( i );
    }
}

bool SbiBuffer::operator +=( sal_Int8 n )
{
    if( Check( 1 ) )
    {
        *pCur++ = (char) n; nOff++; return true;
    } else return false;
}

bool SbiBuffer::operator +=( sal_uInt8 n )
{
    if( Check( 1 ) )
    {
        *pCur++ = (char) n; nOff++; return true;
    } else return false;
}

bool SbiBuffer::operator +=( sal_Int16 n )
{
    if( Check( 2 ) )
    {
        *pCur++ = (char) ( n & 0xFF );
        *pCur++ = (char) ( n >> 8 );
        nOff += 2; return true;
    } else return false;
}

bool SbiBuffer::operator +=( sal_uInt16 n )
{
    if( Check( 2 ) )
    {
        *pCur++ = (char) ( n & 0xFF );
        *pCur++ = (char) ( n >> 8 );
        nOff += 2; return true;
    } else return false;
}

bool SbiBuffer::operator +=( sal_uInt32 n )
{
    if( Check( 4 ) )
    {
        sal_uInt16 n1 = static_cast<sal_uInt16>( n & 0xFFFF );
        sal_uInt16 n2 = static_cast<sal_uInt16>( n >> 16 );
        if ( operator +=( n1 ) && operator +=( n2 ) )
            return true;
        return true;
    }
    return false;
}

bool SbiBuffer::operator +=( sal_Int32 n )
{
    return operator +=( (sal_uInt32) n );
}


bool SbiBuffer::operator +=( const String& n )
{
    sal_uInt16 l = n.Len() + 1;
    if( Check( l ) )
    {
        rtl::OString aByteStr(rtl::OUStringToOString(n, osl_getThreadTextEncoding()));
        memcpy( pCur, aByteStr.getStr(), l );
        pCur += l;
        nOff = nOff + l;
        return true;
    }
    else return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
