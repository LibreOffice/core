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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <string.h>
// auto strip #include "segmentc.hxx"

#include "swgstr.hxx"
#include "swgids.hxx"
namespace binfilter {

#define BUFSIZE 4096				// Groesse des Datenpuffers

/////////////////////////// class swstream ////////////////////////////////

// Diese Klasse stellt nur die Passwort-Funktionalitaet zur Verfuegung.

 swcrypter::swcrypter()
 {
    bPasswd = FALSE;
    memset( cPasswd, 0, PASSWDLEN );
 }

 BOOL swcrypter::setpasswd( const String& rP )
 {
    // Dies sind Randomwerte, die konstant zur Verschluesselung
    // des Passworts verwendet werden.
    static BYTE __READONLY_DATA cEncode[] =
    { 0xAB, 0x9E, 0x43, 0x05, 0x38, 0x12, 0x4d, 0x44,
      0xD5, 0x7e, 0xe3, 0x84, 0x98, 0x23, 0x3f, 0xba };
 
    bPasswd = TRUE;
    xub_StrLen len = rP.Len();
    if( len > PASSWDLEN ) len = PASSWDLEN;
    memcpy( cPasswd, cEncode, PASSWDLEN );
    sal_Char cBuf[ PASSWDLEN ];
    memset( cBuf, ' ', PASSWDLEN );
 // before unicode: memcpy( cBuf, p, len );
    for( xub_StrLen i=0; i < len; i++ )
    {
        sal_Unicode c = rP.GetChar( i );
        if( c > 255 )
            return FALSE;
        cBuf[i] = (sal_Char)c;
    }
    encode( cBuf, PASSWDLEN );
    memcpy( cPasswd, cBuf, PASSWDLEN );
 
    return TRUE;
 }

 void swcrypter::copypasswd( const sal_Char* p )
 {
    memcpy( cPasswd, p, PASSWDLEN );
    bPasswd = TRUE;
 }

 void swcrypter::encode( sal_Char* pSrc, USHORT nLen )
 {
    USHORT nCryptPtr = 0;
    BYTE cBuf[ PASSWDLEN ];
    memcpy( cBuf, cPasswd, PASSWDLEN );
    BYTE* p = cBuf;
 
    while( nLen-- )
    {
        *pSrc = *pSrc ^ ( *p ^ (BYTE) ( cBuf[ 0 ] * nCryptPtr ) );
        *p += ( nCryptPtr < (PASSWDLEN-1) ) ? *(p+1) : cBuf[ 0 ];
        if( !*p ) *p += 1;
        p++;
        if( ++nCryptPtr >= PASSWDLEN ) nCryptPtr = 0, p = cBuf;
        pSrc++;
    }
 }

/////////////////////////// class swstreambase /////////////////////////////

// Diese Klasse stellt einen dynamischen Puffer zur Verfuegung,
// der fuer das Einlesen von Texten verwendet wird.

swstreambase::swstreambase( SvStream& r ) : pStrm( &r )
{
    pStrm->SetBufferSize( BUFSIZE );
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    pBuf = new sal_Char [128];
    nBuflen = 128;
    nLong = 3;
    bTempStrm = FALSE;
}

swstreambase::~swstreambase()
{
    delete pBuf;
    if( bTempStrm ) delete pStrm;
}

void swstreambase::checkbuf( USHORT n )
{
    if( n > nBuflen ) {
        n = ( ( n + 127 ) / 128 ) * 128;
        if( n == 0 ) n = 0xFFFF;
        delete pBuf;
        pBuf = new sal_Char[ n ];
        nBuflen = n;
    }
}

BYTE swstreambase::get()
{
    BYTE c = 0;
    pStrm->Read( (sal_Char*) &c, 1 );
    return c;
}

void swstreambase::setbad()
{
    pStrm->SetError( SVSTREAM_GENERALERROR );
}

long swstreambase::filesize()
{
    long cur = pStrm->Tell();
    long siz = pStrm->Seek( STREAM_SEEK_TO_END );
    pStrm->Seek( cur );
    return siz;
}

// E/A fuer longs als 3-Byte-Zahlen

swstreambase& swstreambase::operator>>( long& n )
{
    BYTE c[ 4 ];
    pStrm->Read( (sal_Char*) &c, nLong );
    n = c[ 0 ]
      + (long) (USHORT) ( c[ 1 ] << 8 )
      + ( (long) c[ 2 ] << 16 );
    if( nLong == 4 )
    {
        n += (long) c[ 3 ] << 24;
        if ( ( sizeof( long ) > 4 ) && ( c[ 3 ] & 0x80 ) )
            // Vorzeichenerweiterung!
            n |= ~0xFFFFFFFFL;
    } else if( c[ 2 ] & 0x80 )
        // Vorzeichenerweiterung!
        n |= ~0xFFFFFFL;
    return *this;
}

//////////////////////////// class swistream ///////////////////////////////

swistream::swistream( SvStream& r ) : swstreambase( r )
{
    nOffset = 0;
    cType = SWG_EOF;
}

long swistream::size()
{
    return nOffset - pStrm->Tell();
}

sal_Char* swistream::text()
{
    BOOL bSkip = FALSE;
    // der String ohne 0, Laenge ist im Offset (von SWG_TEXT)
    long len = size();
    // Sicherheitshalber kuerzen
    if( len > 0xFFF0L ) len = 0xFFF0L, bSkip = TRUE;
    // Eventuell den Puffer vergroessern
    checkbuf( (USHORT) len + 1 );
    pStrm->Read( pBuf, (USHORT) len );
    if( bPasswd )
        encode( pBuf, (USHORT) len );
    pBuf[ (USHORT) len ] = 0;
    if( bSkip ) skip();
    return pBuf;
}

BYTE swistream::peek()
{
    BYTE ch = get();
    pStrm->SeekRel( -1 );
    return ch;
}

BYTE swistream::next()
{
    long pos = pStrm->Tell();
    short n = nLong; nLong = 3;
    cType = get();
    long val;
    swstreambase::operator>>( val );
    // Man achte darauf: Dieser Wert ist immer positiv,
    // es findet keine VZ-Erweiterung statt!
    val &= 0x00FFFFFFL;
    if( good() ) nOffset = val + pos;
    nLong = n;
    return cType;
}

void swistream::undonext()
{
    // der Read-Pointer wird um 4 Bytes zurueckversetzt
    long pos = pStrm->Tell();
    if( pos >= 4 )
    {
        pStrm->Seek( pos - 4 );
        nOffset = -1;   // damit skip() klappt
    }
}

void swistream::skip( long posn )
{
    if( posn == -1L ) posn = nOffset;
    if( posn != -1L ) pStrm->Seek( posn );
}

BYTE swistream::skipnext()
{
    pStrm->Seek( nOffset );
    return next();
}


}
