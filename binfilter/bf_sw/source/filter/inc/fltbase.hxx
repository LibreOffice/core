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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

namespace binfilter {

class SwFilterBase
{
protected:
    SvStream	*pIn;
    sal_Char	*pReadBuff;	//		Groessenangabe
    INT32		nBytesLeft;	// noch zu lesende Bytes des aktuelle Records

    CharSet		eQuellChar;	// Quell-Zeichensatz (interner Zeichensatz)
//	CharSet		eZielChar;	// Ziel-Zeichensatz

    USHORT		nTab;		// z.Zt. bearbeitete Tabelle
    USHORT		nReadBuffSize;// temporaerer Lesepuffer mit

    // ----------------------------------------------------------
    inline void ReadChar( char &rC );
    inline void ReadByte( BYTE &rN );
    inline void Read( short &rN );
    inline void ReadUnicode( sal_Unicode &rU );
    inline void Read( BYTE &rN0, USHORT &rN1, USHORT &rN2 );
    inline void Read( USHORT &rN );
    inline void Read( USHORT &rN1, USHORT &rN2 );
    inline void Read( USHORT &rN1, USHORT &rN2, USHORT &rN3, USHORT &rN4 );
    inline void Read( double &rF );
            void Read( String &rS );		// liest 0-terminierten C-String!
    inline void ClearBytesLeft( void );
};


inline void SwFilterBase::ReadChar( char &rC )
    {
    *pIn >> rC;
    nBytesLeft--;
    }

inline void SwFilterBase::ReadByte( BYTE &rN )
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

inline void SwFilterBase::Read( BYTE &rN0, USHORT &rN1, USHORT &rN2 )
    {
    *pIn >> rN0 >> rN1 >> rN2;
    nBytesLeft -= 5;
    }

inline void SwFilterBase::Read( USHORT &rN )
    {
    *pIn >> rN;
    nBytesLeft -= 2;
    }

inline void SwFilterBase::Read( USHORT &rN1, USHORT &rN2 )
    {
    *pIn >> rN1 >> rN2;
    nBytesLeft -= 4;
    }

inline void SwFilterBase::Read( USHORT &rN1, USHORT &rN2, USHORT &rN3, USHORT &rN4 )
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


} //namespace binfilter
#endif

