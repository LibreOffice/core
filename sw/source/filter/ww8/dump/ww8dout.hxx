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

#ifndef _WW8DOUT_HXX
#define _WW8DOUT_HXX

#ifndef INC_FSTREAM
  #include <fstream.h>          // ostream
#endif
#ifndef SOLAR_H
  #include <tools/solar.h>          // sal_uInt8
#endif

class SvStream;

void OutByte(     SvStream& rSt, short nLen );
void OutBool(     SvStream& rSt, short nLen );
void OutShort(    SvStream& rSt, short      );
void OutShorts(   SvStream& rSt, short nLen );
void OutWord(     SvStream& rSt, short      );
void OutWords(    SvStream& rSt, short nLen );
void OutWordHex(  SvStream& rSt, short      );
void OutWordsHex( SvStream& rSt, short      );
void OutLongHex(  SvStream& rSt, short      );
void OutLongsHex( SvStream& rSt, short nLen );
void OutTab(      SvStream& rSt, short nLen );
void OutTab68(    SvStream& rSt, short nLen );
void OutTab190(   SvStream& rSt, short nLen );
void OutTab191(   SvStream& rSt, short nLen );
void OutTab192(   SvStream& rSt, short nLen );
void OutBool4(    SvStream& rSt, short nLen );
void OutHugeHex(  SvStream& rSt, short nLen );
void OutTabD608(  SvStream& rSt, short nLen );
void OutTabD609(  SvStream& rSt, short nLen );


// unter WNTMSCI4 muss das __cdecl sein, damit nicht nur der Fkt-Pointer
// ausgegeben wird....

ostream& __cdecl filepos( ostream& s, SvStream& rSt );

ostream& __cdecl endl1(   ostream& s );
ostream& __cdecl indent(  ostream& s, SvStream& rSt );
ostream& __cdecl indent1( ostream& s );
ostream& __cdecl indent2( ostream& s );
ostream& __cdecl hex2( ostream& s );
ostream& __cdecl hex4( ostream& s );
ostream& __cdecl hex6( ostream& s );
ostream& __cdecl hex8( ostream& s );
ostream& __cdecl dec2( ostream& s );

ostream& __cdecl begin( ostream&  s, SvStream& rSt  );
ostream& __cdecl begin1( ostream& s );
ostream& __cdecl begin2( ostream& s );
ostream& __cdecl end( ostream&  s, SvStream& rSt  );
ostream& __cdecl end1( ostream& s );
ostream& __cdecl end2( ostream& s );

extern ostream* pOut;


#endif
