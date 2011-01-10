/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include "sbcomp.hxx"
#include "buffer.hxx"
#include <string.h>

const static sal_uInt32 UP_LIMIT=0xFFFFFF00L;

// Der SbiBuffer wird in Inkrements von mindestens 16 Bytes erweitert.
// Dies ist notwendig, da viele Klassen von einer Pufferlaenge
// von x*16 Bytes ausgehen.

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

// Rausreichen des Puffers
// Dies fuehrt zur Loeschung des Puffers!

char* SbiBuffer::GetBuffer()
{
    char* p = pBuf;
    pBuf = NULL;
    pCur = NULL;
    return p;
}

// Test, ob der Puffer n Bytes aufnehmen kann.
// Im Zweifelsfall wird er vergroessert

sal_Bool SbiBuffer::Check( sal_uInt16 n )
{
    if( !n ) return sal_True;
    if( ( static_cast<sal_uInt32>( nOff )+ n ) >  static_cast<sal_uInt32>( nSize ) )
    {
        if( nInc == 0 )
            return sal_False;
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
            return sal_False;
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
    return sal_True;
}

// Angleich des Puffers auf die uebergebene Byte-Grenze

void SbiBuffer::Align( sal_Int32 n )
{
    if( nOff % n ) {
        sal_uInt32 nn =( ( nOff + n ) / n ) * n;
        if( nn <= UP_LIMIT )
        {
            nn = nn - nOff;
            if( Check( static_cast<sal_uInt16>(nn) ) )
            {
                memset( pCur, 0, nn );
                pCur += nn;
                nOff = nOff + nn;
            }
        }
    }
}

// Patch einer Location

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

// Forward References auf Labels und Prozeduren
// bauen eine Kette auf. Der Anfang der Kette ist beim uebergebenen
// Parameter, das Ende der Kette ist 0.

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

sal_Bool SbiBuffer::operator +=( sal_Int8 n )
{
    if( Check( 1 ) )
    {
        *pCur++ = (char) n; nOff++; return sal_True;
    } else return sal_False;
}

sal_Bool SbiBuffer::operator +=( sal_uInt8 n )
{
    if( Check( 1 ) )
    {
        *pCur++ = (char) n; nOff++; return sal_True;
    } else return sal_False;
}

sal_Bool SbiBuffer::operator +=( sal_Int16 n )
{
    if( Check( 2 ) )
    {
        *pCur++ = (char) ( n & 0xFF );
        *pCur++ = (char) ( n >> 8 );
        nOff += 2; return sal_True;
    } else return sal_False;
}

sal_Bool SbiBuffer::operator +=( sal_uInt16 n )
{
    if( Check( 2 ) )
    {
        *pCur++ = (char) ( n & 0xFF );
        *pCur++ = (char) ( n >> 8 );
        nOff += 2; return sal_True;
    } else return sal_False;
}

sal_Bool SbiBuffer::operator +=( sal_uInt32 n )
{
    if( Check( 4 ) )
    {
        sal_uInt16 n1 = static_cast<sal_uInt16>( n & 0xFFFF );
        sal_uInt16 n2 = static_cast<sal_uInt16>( n >> 16 );
        if ( operator +=( n1 ) && operator +=( n2 ) )
            return sal_True;
        return sal_True;
    }
    return sal_False;
}

sal_Bool SbiBuffer::operator +=( sal_Int32 n )
{
    return operator +=( (sal_uInt32) n );
}


sal_Bool SbiBuffer::operator +=( const String& n )
{
    sal_uInt16 l = n.Len() + 1;
    if( Check( l ) )
    {
        ByteString aByteStr( n, gsl_getSystemTextEncoding() );
        memcpy( pCur, aByteStr.GetBuffer(), l );
        pCur += l;
        nOff = nOff + l;
        return sal_True;
    }
    else return sal_False;
}

sal_Bool SbiBuffer::Add( const void* p, sal_uInt16 len )
{
    if( Check( len ) )
    {
        memcpy( pCur, p, len );
        pCur += len;
        nOff = nOff + len;
        return sal_True;
    } else return sal_False;
}



