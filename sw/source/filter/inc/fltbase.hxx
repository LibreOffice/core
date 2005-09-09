/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fltbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:49:01 $
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
#ifndef __FLTBASE_HXX__
#define __FLTBASE_HXX__

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class SwFilterBase
{
protected:
    SvStream    *pIn;
    sal_Char    *pReadBuff; //      Groessenangabe
    INT32       nBytesLeft; // noch zu lesende Bytes des aktuelle Records

    CharSet     eQuellChar; // Quell-Zeichensatz (interner Zeichensatz)
//  CharSet     eZielChar;  // Ziel-Zeichensatz

    USHORT      nTab;       // z.Zt. bearbeitete Tabelle
    USHORT      nReadBuffSize;// temporaerer Lesepuffer mit

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
            void Read( String &rS );        // liest 0-terminierten C-String!
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


#endif

