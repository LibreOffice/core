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
#ifndef __FLTBASE_HXX__
#define __FLTBASE_HXX__

#include <tools/stream.hxx>
#include <tools/string.hxx>

class SwFilterBase
{
protected:
    SvStream    *pIn;
    sal_Char    *pReadBuff; //      Groessenangabe
    sal_Int32       nBytesLeft; // noch zu lesende Bytes des aktuelle Records

    CharSet     eQuellChar; // Quell-Zeichensatz (interner Zeichensatz)
//  CharSet     eZielChar;  // Ziel-Zeichensatz

    sal_uInt16      nTab;       // z.Zt. bearbeitete Tabelle
    sal_uInt16      nReadBuffSize;// temporaerer Lesepuffer mit

    // ----------------------------------------------------------
    inline void ReadChar( char &rC );
    inline void ReadByte( sal_uInt8 &rN );
    inline void Read( short &rN );
    inline void ReadUnicode( sal_Unicode &rU );
    inline void Read( sal_uInt8 &rN0, sal_uInt16 &rN1, sal_uInt16 &rN2 );
    inline void Read( sal_uInt16 &rN );
    inline void Read( sal_uInt16 &rN1, sal_uInt16 &rN2 );
    inline void Read( sal_uInt16 &rN1, sal_uInt16 &rN2, sal_uInt16 &rN3, sal_uInt16 &rN4 );
    inline void Read( double &rF );
            void Read( String &rS );        // liest 0-terminierten C-String!
    inline void ClearBytesLeft( void );
};


inline void SwFilterBase::ReadChar( char &rC )
    {
    *pIn >> rC;
    nBytesLeft--;
    }

inline void SwFilterBase::ReadByte( sal_uInt8 &rN )
    {
    *pIn >> rN;
    nBytesLeft--;
    }

inline void SwFilterBase::ReadUnicode( sal_Unicode &rU )
{
    {
    sal_Char cC;
    *pIn >> cC;
    rU = ByteString::ConvertToUnicode(cC, eQuellChar);
    nBytesLeft--;
    }
}

inline void SwFilterBase::Read( short &rN )
    {
    *pIn >> rN;
    nBytesLeft -= 2;
    }

inline void SwFilterBase::Read( sal_uInt8 &rN0, sal_uInt16 &rN1, sal_uInt16 &rN2 )
    {
    *pIn >> rN0 >> rN1 >> rN2;
    nBytesLeft -= 5;
    }

inline void SwFilterBase::Read( sal_uInt16 &rN )
    {
    *pIn >> rN;
    nBytesLeft -= 2;
    }

inline void SwFilterBase::Read( sal_uInt16 &rN1, sal_uInt16 &rN2 )
    {
    *pIn >> rN1 >> rN2;
    nBytesLeft -= 4;
    }

inline void SwFilterBase::Read( sal_uInt16 &rN1, sal_uInt16 &rN2, sal_uInt16 &rN3, sal_uInt16 &rN4 )
    {
    *pIn >> rN1 >> rN2 >> rN3 >> rN4;
    nBytesLeft -= 8;
    }

inline void SwFilterBase::Read( double &rF )
    {
    *pIn >> rF;
    nBytesLeft -= 8;
    }

inline void SwFilterBase::ClearBytesLeft( void )
    {
    pIn->SeekRel( nBytesLeft );
    nBytesLeft = 0;
    }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
