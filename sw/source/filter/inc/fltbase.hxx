/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

