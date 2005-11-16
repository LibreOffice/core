/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ww8dout.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:07 $
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

#ifndef _WW8DOUT_HXX
#define _WW8DOUT_HXX

#ifndef INC_FSTREAM
  #include <fstream.h>          // ostream
#endif
#ifndef SOLAR_H
  #include <tools/solar.h>          // BYTE
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
