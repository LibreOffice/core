/*************************************************************************
 *
 *  $RCSfile: buffer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "sbcomp.hxx"
#pragma hdrstop
#include "buffer.hxx"
#include <string.h>

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

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
    delete pBuf;
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
    if( ((long) nOff + n ) > (long) nSize )
    {
        USHORT nn = 0;
        while( nn < n ) nn += nInc;
        char* p;
        if( ((long) nSize + nn ) > 0xFF00L ) p = NULL;
        else p = new char [nSize + nn];
        if( !p )
        {
            pParser->Error( SbERR_PROG_TOO_LARGE );
            nInc = 0;
            delete pBuf; pBuf = NULL;
            return FALSE;
        }
        else
        {
            if( nSize ) memcpy( p, pBuf, nSize );
            delete pBuf;
            pBuf = p;
            pCur = pBuf + nOff;
            nSize += nn;
        }
    }
    return TRUE;
}

// Angleich des Puffers auf die uebergebene Byte-Grenze

void SbiBuffer::Align( short n )
{
    if( nOff % n ) {
        USHORT nn =( ( nOff + n ) / n ) * n;
        if( nn <= 0xFF00 )
        {
            nn -= nOff;
            if( Check( nn ) )
            {
                memset( pCur, 0, nn );
                pCur += nn;
                nOff += nn;
            }
        }
    }
}

// Patch einer Location

void SbiBuffer::Patch( USHORT off, UINT16 val )
{
    if( ( off + sizeof( UINT16 ) ) < nOff )
    {
        BYTE* p = (BYTE*) pBuf + off;
        *p++ = (char) ( val & 0xFF );
        *p   = (char) ( val >> 8 );
    }
}

// Forward References auf Labels und Prozeduren
// bauen eine Kette auf. Der Anfang der Kette ist beim uebergebenen
// Parameter, das Ende der Kette ist 0.

void SbiBuffer::Chain( USHORT off )
{
    if( off && pBuf )
    {
        BYTE *ip;
        USHORT i = off;
        USHORT val = nOff;
        do
        {
            ip = (BYTE*) pBuf + i;
            i = ( *ip ) | ( *(ip+1) << 8 );
            if( i >= nOff )
            {
                pParser->Error( SbERR_INTERNAL_ERROR, "BACKCHAIN" );
                break;
            }
            *ip++ = (char) ( val & 0xFF );
            *ip   = (char) ( val >> 8 );
        } while( i );
    }
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

BOOL SbiBuffer::operator +=( const String& n )
{
    USHORT l = n.Len() + 1;
    if( Check( l ) )
    {
        ByteString aByteStr( n, gsl_getSystemTextEncoding() );
        memcpy( pCur, aByteStr.GetBuffer(), l );
        pCur += l;
        nOff += l;
        return TRUE;
    }
    else return FALSE;
}

BOOL SbiBuffer::Add( const void* p, USHORT len )
{
    if( Check( len ) )
    {
        memcpy( pCur, p, len );
        pCur += len;
        nOff += len;
        return TRUE;
    } else return FALSE;
}



