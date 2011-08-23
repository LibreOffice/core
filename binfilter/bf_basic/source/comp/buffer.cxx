/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "buffer.hxx"
#include <string.h>

namespace binfilter {

const static UINT32 UP_LIMIT=0xFFFFFF00L;

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

BOOL SbiBuffer::Check( USHORT n )
{
    if( !n ) return TRUE;
    if( ( static_cast<UINT32>( nOff )+ n ) >  static_cast<UINT32>( nSize ) )
    {
        if( nInc == 0 )
            return FALSE;
        USHORT nn = 0;
        while( nn < n ) nn = nn + nInc;
        char* p;
        if( ( static_cast<UINT32>( nSize ) + nn ) > UP_LIMIT ) p = NULL;
        else p = new char [nSize + nn];
        if( !p )
        {
/*?*/ // 			pParser->Error( SbERR_PROG_TOO_LARGE );
            nInc = 0;
            delete[] pBuf; pBuf = NULL;
            return FALSE;
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
    return TRUE;
}

BOOL SbiBuffer::operator +=( INT8 n )
{
    if( Check( 1 ) )
    {
        *pCur++ = (char) n; nOff++; return TRUE;
    } else return FALSE;
}

BOOL SbiBuffer::operator +=( UINT8 n )
{
    if( Check( 1 ) )
    {
        *pCur++ = (char) n; nOff++; return TRUE;
    } else return FALSE;
}

BOOL SbiBuffer::operator +=( INT16 n )
{
    if( Check( 2 ) )
    {
        *pCur++ = (char) ( n & 0xFF );
        *pCur++ = (char) ( n >> 8 );
        nOff += 2; return TRUE;
    } else return FALSE;
}

BOOL SbiBuffer::operator +=( UINT16 n )
{
    if( Check( 2 ) )
    {
        *pCur++ = (char) ( n & 0xFF );
        *pCur++ = (char) ( n >> 8 );
        nOff += 2; return TRUE;
    } else return FALSE;
}

BOOL SbiBuffer::operator +=( UINT32 n )
{
    if( Check( 4 ) )
    {
        UINT16 n1 = static_cast<UINT16>( n & 0xFFFF );
        UINT16 n2 = static_cast<UINT16>( n >> 16 );
        if ( operator +=( n1 ) && operator +=( n2 ) )
            return TRUE;
        return TRUE;
    } 
    return FALSE;
}

BOOL SbiBuffer::operator +=( INT32 n )
{
    return operator +=( (UINT32) n );
}


BOOL SbiBuffer::operator +=( const String& n )
{
    USHORT l = n.Len() + 1;
    if( Check( l ) )
    {
        ByteString aByteStr( n, gsl_getSystemTextEncoding() );
        memcpy( pCur, aByteStr.GetBuffer(), l );
        pCur += l;
        nOff = nOff + l;
        return TRUE;
    }
    else return FALSE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
