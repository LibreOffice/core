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
