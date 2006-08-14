/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swgstr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:33:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SWGSTR_HXX
#define _SWGSTR_HXX

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

typedef long long3;                     // Zur Dokumentation: 3-byte-Longs

#define MAX_BEGIN 64                    // Maximale Blockschachtelung
#define PASSWDLEN 16                    // Maximale Passwortlaenge

// Neue Version mit SvStreams

// Passwort- und Codierungs-Funktionalitaet

class swcrypter {
protected:
    sal_Char   cPasswd[ PASSWDLEN ];    // Passwort-Puffer
    BOOL   bPasswd;                     // TRUE wenn mit Passwort
    void   encode( sal_Char*, USHORT ); // Puffer codieren/decodieren
public:
    swcrypter();
    BOOL setpasswd( const String& );    // Passwort setzen
    void copypasswd( const sal_Char* ); // Passwort direkt setzen
    const sal_Char* getpasswd() { return cPasswd; }
};

// Reader/Writer-Stream-Basisklasse mit Pufferverwaltung fuer Texte
// und Spezial-I/O fuer 3-Byte-Longs

class swstreambase : public swcrypter {
protected:
    SvStream* pStrm;                    // eigentlicher Stream
    sal_Char*  pBuf;                        // Zwischenpuffer
    USHORT nBuflen;                     // Laenge des Zwischenpuffers
    short  nLong;                       // Long-Laenge (3 oder 4)
    BOOL   bTempStrm;                   // TRUE: Stream loeschen
    void   checkbuf( USHORT );          // Testen der Pufferlaenge

    swstreambase( SvStream& );

    swstreambase( const swstreambase& );
    int operator=( const swstreambase& );
public:
    ~swstreambase();
    SvStream& Strm()                    { return *pStrm; }
    void clear();                       // Puffer loeschen

    // Zusatzfunktionen zur I/O von LONGs als 3-Byte-Zahlen

    void long3()                        { nLong = 3; }
    void long4()                        { nLong = 4; }

    // Alias- und Hilfsfunktionen

    void seek( long nPos )              { pStrm->Seek( nPos );  }
    long tell()                         { return pStrm->Tell(); }
    long filesize();                    // Dateigroesse

    void setbad();
    int good()                          { return ( pStrm->GetError() == SVSTREAM_OK ); }
    int operator!()                     { return ( pStrm->GetError() != SVSTREAM_OK ); }
    int eof()                           { return pStrm->IsEof(); }

    BYTE get();
    void get( void* p, USHORT n )       { pStrm->Read( (sal_Char*) p, n ); }

    inline swstreambase& operator>>( sal_Char& );
    inline swstreambase& operator>>( BYTE& );
    inline swstreambase& operator>>( short& );
    inline swstreambase& operator>>( USHORT& );
           swstreambase& operator>>( long& );
    inline swstreambase& operator>>( ULONG& );
};

inline swstreambase& swstreambase::operator>>( sal_Char& c )
{
    *pStrm >> c; return *this;
}

inline swstreambase& swstreambase::operator>>( BYTE& c )
{
    *pStrm >> c; return *this;
}

inline swstreambase& swstreambase::operator>>( short& c )
{
    *pStrm >> c; return *this;
}

inline swstreambase& swstreambase::operator>>( USHORT& c )
{
    *pStrm >> c; return *this;
}

inline swstreambase& swstreambase::operator>>( ULONG& c )
{
    return *this >> (long&) c;
}

class swistream : public swstreambase {
    BYTE   cType;                       // Record-Typ
    ULONG  nOffset;                     // Record-Offset-Portion
public:
    swistream( SvStream& );

    BYTE peek();                        // 1 Byte testen
    BYTE next();                        // Blockstart
    BYTE cur() { return cType; }        // aktueller Block
    BYTE skipnext();                    // Record ueberspringen
    void undonext();                    // next() rueckgaengig machen
    long getskip()                      { return nOffset; }
    void skip( long = -1L );            // Block ueberspringen
    sal_Char* text();                   // Textstring lesen (nach BEGIN)
    long size();                        // aktuelle Record-Laenge

private:
    swistream( const swistream& );
    int operator=( const swistream& );
};


#endif
