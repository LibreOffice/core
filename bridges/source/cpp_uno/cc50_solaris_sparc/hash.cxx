/*************************************************************************
 *
 *  $RCSfile: hash.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: pl $ $Date: 2001-03-16 15:24:35 $
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


#ifndef TEST
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#else
typedef unsigned int sal_uInt32;
#endif

#include <string.h>

/*
 *  build a hash for a character buffer using the NIST algorithm
 */

class NIST_Hash
{

    // helper functions
    sal_uInt32 f1( sal_uInt32 x, sal_uInt32 y, sal_uInt32 z )
    {
        return z ^ ( x & ( y ^ z ) );
    }

    sal_uInt32 f2( sal_uInt32 x, sal_uInt32 y, sal_uInt32 z )
    {
        return x ^ y ^ z;
    }

    sal_uInt32 f3( sal_uInt32 x, sal_uInt32 y, sal_uInt32 z )
    {
        return ( x & y ) + ( z & ( x ^ y ) );
    }

    sal_uInt32 rotl( sal_uInt32 nValue, sal_uInt32 nBits )
    {
        return ( nValue << nBits ) | ( nValue >> (32-nBits) );
    }

    sal_uInt32 expand_nostore( sal_uInt32 index )
    {
        return data[index&15] ^ data[(index-14)&15] ^ data[(index-8)&15] ^ data[(index-3)&15];
    }

    sal_uInt32 expand_store( sal_uInt32 index )
    {
        return data[index&15] ^= data[(index-14)&15] ^ data[(index-8)&15] ^ data[(index-3)&15];
    }

    void subRound( sal_uInt32 a, sal_uInt32& b, sal_uInt32 c, sal_uInt32 d, sal_uInt32& e, sal_uInt32 constant, sal_uInt32 datum, sal_uInt32 nFunction )
    {
        e += rotl(a,5);
        switch( nFunction )
        {
            case 1: e += f1( b, c, d );break;
            case 2:
            case 4: e += f2( b, c, d );break;
            case 3: e += f3( b, c, d );break;
        }
        e += constant + datum;
        b = rotl( b, 30 );
    }

    void transform();
    void final();

    // data members
    sal_uInt32 data[16];
    sal_uInt32 hashdata[5];
public:
    NIST_Hash( const char* pString, sal_uInt32 nLen );

    sal_uInt32 *getHash() { return hashdata; }
};

void NIST_Hash::transform()
{
    // constants
    const sal_uInt32 K2     = 0x5A827999;
    const sal_uInt32 K3     = 0x6ED9EBA1;
    const sal_uInt32 K5     = 0x8F1BBCDC;
    const sal_uInt32 K10    = 0xCA62C1D6;

    sal_uInt32 a, b, c, d, e;
    a = hashdata[0];
    b = hashdata[1];
    c = hashdata[2];
    d = hashdata[3];
    e = hashdata[4];

    subRound( a, b, c, d, e, K2, data[ 0], 1 );
    subRound( e, a, b, c, d, K2, data[ 1], 1 );
    subRound( d, e, a, b, c, K2, data[ 2], 1 );
    subRound( c, d, e, a, b, K2, data[ 3], 1 );
    subRound( b, c, d, e, a, K2, data[ 4], 1 );
    subRound( a, b, c, d, e, K2, data[ 5], 1 );
    subRound( e, a, b, c, d, K2, data[ 6], 1 );
    subRound( d, e, a, b, c, K2, data[ 7], 1 );
    subRound( c, d, e, a, b, K2, data[ 8], 1 );
    subRound( b, c, d, e, a, K2, data[ 9], 1 );
    subRound( a, b, c, d, e, K2, data[10], 1 );
    subRound( e, a, b, c, d, K2, data[11], 1 );
    subRound( d, e, a, b, c, K2, data[12], 1 );
    subRound( c, d, e, a, b, K2, data[13], 1 );
    subRound( b, c, d, e, a, K2, data[14], 1 );
    subRound( a, b, c, d, e, K2, data[15], 1 );
    subRound( e, a, b, c, d, K2, expand_store( 16 ), 1 );
    subRound( d, e, a, b, c, K2, expand_store( 17 ), 1 );
    subRound( c, d, e, a, b, K2, expand_store( 18 ), 1 );
    subRound( b, c, d, e, a, K2, expand_store( 19 ), 1 );

    subRound( a, b, c, d, e, K3, expand_store( 20 ), 2 );
    subRound( e, a, b, c, d, K3, expand_store( 21 ), 2 );
    subRound( d, e, a, b, c, K3, expand_store( 22 ), 2 );
    subRound( c, d, e, a, b, K3, expand_store( 23 ), 2 );
    subRound( b, c, d, e, a, K3, expand_store( 24 ), 2 );
    subRound( a, b, c, d, e, K3, expand_store( 25 ), 2 );
    subRound( e, a, b, c, d, K3, expand_store( 26 ), 2 );
    subRound( d, e, a, b, c, K3, expand_store( 27 ), 2 );
    subRound( c, d, e, a, b, K3, expand_store( 28 ), 2 );
    subRound( b, c, d, e, a, K3, expand_store( 29 ), 2 );
    subRound( a, b, c, d, e, K3, expand_store( 30 ), 2 );
    subRound( e, a, b, c, d, K3, expand_store( 31 ), 2 );
    subRound( d, e, a, b, c, K3, expand_store( 32 ), 2 );
    subRound( c, d, e, a, b, K3, expand_store( 33 ), 2 );
    subRound( b, c, d, e, a, K3, expand_store( 34 ), 2 );
    subRound( a, b, c, d, e, K3, expand_store( 35 ), 2 );
    subRound( e, a, b, c, d, K3, expand_store( 36 ), 2 );
    subRound( d, e, a, b, c, K3, expand_store( 37 ), 2 );
    subRound( c, d, e, a, b, K3, expand_store( 38 ), 2 );
    subRound( b, c, d, e, a, K3, expand_store( 39 ), 2 );

    subRound( a, b, c, d, e, K5, expand_store( 40 ), 3 );
    subRound( e, a, b, c, d, K5, expand_store( 41 ), 3 );
    subRound( d, e, a, b, c, K5, expand_store( 42 ), 3 );
    subRound( c, d, e, a, b, K5, expand_store( 43 ), 3 );
    subRound( b, c, d, e, a, K5, expand_store( 44 ), 3 );
    subRound( a, b, c, d, e, K5, expand_store( 45 ), 3 );
    subRound( e, a, b, c, d, K5, expand_store( 46 ), 3 );
    subRound( d, e, a, b, c, K5, expand_store( 47 ), 3 );
    subRound( c, d, e, a, b, K5, expand_store( 48 ), 3 );
    subRound( b, c, d, e, a, K5, expand_store( 49 ), 3 );
    subRound( a, b, c, d, e, K5, expand_store( 50 ), 3 );
    subRound( e, a, b, c, d, K5, expand_store( 51 ), 3 );
    subRound( d, e, a, b, c, K5, expand_store( 52 ), 3 );
    subRound( c, d, e, a, b, K5, expand_store( 53 ), 3 );
    subRound( b, c, d, e, a, K5, expand_store( 54 ), 3 );
    subRound( a, b, c, d, e, K5, expand_store( 55 ), 3 );
    subRound( e, a, b, c, d, K5, expand_store( 56 ), 3 );
    subRound( d, e, a, b, c, K5, expand_store( 57 ), 3 );
    subRound( c, d, e, a, b, K5, expand_store( 58 ), 3 );
    subRound( b, c, d, e, a, K5, expand_store( 59 ), 3 );

    subRound( a, b, c, d, e, K10, expand_store( 60 ), 4 );
    subRound( e, a, b, c, d, K10, expand_store( 61 ), 4 );
    subRound( d, e, a, b, c, K10, expand_store( 62 ), 4 );
    subRound( c, d, e, a, b, K10, expand_store( 63 ), 4 );
    subRound( b, c, d, e, a, K10, expand_store( 64 ), 4 );
    subRound( a, b, c, d, e, K10, expand_store( 65 ), 4 );
    subRound( e, a, b, c, d, K10, expand_store( 66 ), 4 );
    subRound( d, e, a, b, c, K10, expand_store( 67 ), 4 );
    subRound( c, d, e, a, b, K10, expand_store( 68 ), 4 );
    subRound( b, c, d, e, a, K10, expand_store( 69 ), 4 );
    subRound( a, b, c, d, e, K10, expand_store( 70 ), 4 );
    subRound( e, a, b, c, d, K10, expand_store( 71 ), 4 );
    subRound( d, e, a, b, c, K10, expand_store( 72 ), 4 );
    subRound( c, d, e, a, b, K10, expand_store( 73 ), 4 );
    subRound( b, c, d, e, a, K10, expand_store( 74 ), 4 );
    subRound( a, b, c, d, e, K10, expand_store( 75 ), 4 );
    subRound( e, a, b, c, d, K10, expand_store( 76 ), 4 );
    subRound( d, e, a, b, c, K10, expand_nostore( 77 ), 4 );
    subRound( c, d, e, a, b, K10, expand_nostore( 78 ), 4 );
    subRound( b, c, d, e, a, K10, expand_nostore( 79 ), 4 );

    hashdata[0] += a;
    hashdata[1] += b;
    hashdata[2] += c;
    hashdata[3] += d;
    hashdata[4] += e;
}

#define BLOCKSIZE sizeof( data )

NIST_Hash::NIST_Hash( const char* pString, sal_uInt32 nLen )
{
    hashdata[0] = 0x67452301;
    hashdata[1] = 0xefcdab89;
    hashdata[2] = 0x98badcfe;
    hashdata[3] = 0x10325476;
    hashdata[4] = 0xc3d2e1f0;

    sal_uInt32 nBytes = nLen;

    while( nLen >= sizeof( data ) )
    {
        memcpy( data, pString, sizeof( data ) );
        pString += sizeof( data );
        nLen -= sizeof( data );
        transform();
    }
    memcpy( data, pString, nLen );
    ((char*)data)[nLen++] = 0x80;
    if( nLen > sizeof( data ) - 8 )
    {
        memset( ((char*)data)+nLen, 0, sizeof( data ) - nLen );
        transform();
        memset( data, 0, sizeof( data ) - 8 );
    }
    else
        memset( ((char*)data)+nLen, 0, sizeof( data ) - 8 - nLen );
    data[14] = 0;
    data[15] = nBytes << 3;
    transform();
}

#ifdef TEST
#include <stdio.h>
int main( int argc, const char** argv )
{
    const char* pHash = argc < 2 ? argv[0] : argv[1];

    NIST_Hash aHash( pHash, strlen( pHash ) );
    sal_uInt32* pBits = aHash.getHash();

    printf( "text : %s\n"
            "bits : 0x%.8x 0x%.8x 0x%.8x 0x%.8x 0x%.8x\n",
            pHash,
            pBits[0], pBits[1], pBits[2],pBits[3],pBits[4]
            );
    return 0;
}

#endif
