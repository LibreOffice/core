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
#ifndef _SWGSTR_HXX
#define _SWGSTR_HXX

#include <tools/stream.hxx>

typedef long long3;                     // For documentation: 3-byte-longs.

#define MAX_BEGIN 64                    // Maximum block nesting.
#define PASSWDLEN 16                    // Maximum password length.

// New version with SvStreams.

// Functionality for passwords and encoding.

class swcrypter {
protected:
    sal_Char   cPasswd[ PASSWDLEN ];    // Password buffer.
    sal_Bool   bPasswd;                     // sal_True if with password.
    void   encode( sal_Char*, sal_uInt16 ); // Encode/decode buffer.
public:
    swcrypter();
    sal_Bool setpasswd( const String& );    // Set password.
    void copypasswd( const sal_Char* ); // Set password directly.
    const sal_Char* getpasswd() { return cPasswd; }
};

// Reader/Writer-stream base class with buffer administration for texts
// ans special I/O for 3-byte-longs.

class swstreambase : public swcrypter {
protected:
    SvStream* pStrm;                    // The actual stream.
    sal_Char*  pBuf;                        // Temporary buffer.
    sal_uInt16 nBuflen;                     // Length of temporary buffer.
    short  nLong;                       // Long-length (3 or 4).
    sal_Bool   bTempStrm;                   // sal_True: delete stream.
    void   checkbuf( sal_uInt16 );          // Test buffer length.

    swstreambase( SvStream& );

    swstreambase( const swstreambase& );
    int operator=( const swstreambase& );
public:
    ~swstreambase();
    SvStream& Strm()                    { return *pStrm; }
    void clear();                       // Delete buffer.

    // Supplementary functions for I/O of LONGs as 3-byte numbers.

    void long3()                        { nLong = 3; }
    void long4()                        { nLong = 4; }


    // Alias and helper functions.

    void seek( long nPos )              { pStrm->Seek( nPos );  }
    long tell()                         { return pStrm->Tell(); }
    long filesize();

    void setbad();
    int good()                          { return ( pStrm->GetError() == SVSTREAM_OK ); }
    int operator!()                     { return ( pStrm->GetError() != SVSTREAM_OK ); }
    int eof()                           { return pStrm->IsEof(); }

    sal_uInt8 get();
    void get( void* p, sal_uInt16 n )       { pStrm->Read( (sal_Char*) p, n ); }

    inline swstreambase& operator>>( sal_Char& );
    inline swstreambase& operator>>( sal_uInt8& );
    inline swstreambase& operator>>( short& );
    inline swstreambase& operator>>( sal_uInt16& );
           swstreambase& operator>>( long& );
    inline swstreambase& operator>>( sal_uLong& );
};

inline swstreambase& swstreambase::operator>>( sal_Char& c )
{
    *pStrm >> c; return *this;
}

inline swstreambase& swstreambase::operator>>( sal_uInt8& c )
{
    *pStrm >> c; return *this;
}

inline swstreambase& swstreambase::operator>>( short& c )
{
    *pStrm >> c; return *this;
}

inline swstreambase& swstreambase::operator>>( sal_uInt16& c )
{
    *pStrm >> c; return *this;
}

inline swstreambase& swstreambase::operator>>( sal_uLong& c )
{
    return *this >> (long&) c;
}

class swistream : public swstreambase {
    sal_uInt8   cType;                      // Record type.
    sal_uLong  nOffset;                     // Record offset portion.
public:
    swistream( SvStream& );

    sal_uInt8 peek();                       // Test 1 byte.
    sal_uInt8 next();                       // Block start.
    sal_uInt8 cur() { return cType; }       // Current block.
    sal_uInt8 skipnext();                   // Skip record.
    void undonext();                    // Undo next().
    long getskip()                      { return nOffset; }
    void skip( long = -1L );            // Skip block.
    sal_Char* text();                   // Read text string (after BEGIN).
    long size();                        // Current record length.

private:
    swistream( const swistream& );
    int operator=( const swistream& );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
