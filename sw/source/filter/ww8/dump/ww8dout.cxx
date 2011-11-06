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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <stdio.h>          // getchar

//#include "defs.hxx"
#include <tools/solar.h>            // sal_uInt8
//#include "wwscan.hxx" // aWwStor
#include "ww8dout.hxx"
#include <tools/stream.hxx>

#include <sot/storage.hxx>

extern SvStorageStreamRef xStrm;
extern SvStorageStreamRef xTableStream;
extern SvStorageStreamRef xDataStream;  // ist bei Ver6-7 mit xStrm identisch,
void DumpSprms( sal_uInt8 nVersion, SvStream& rSt, short nLen );

ostream* pOut = 0;

#define DumpVer8

//-----------------------------------------
//          Streams
//-----------------------------------------


static int level = 0;
static long lastpos = -1;

ostream& __cdecl endl1( ostream& s ){
//ostream& endl1( ostream& s ) {
    s << endl;
    return s;
}

ostream&  __cdecl hex2( ostream& s ) {
    s.width( 2 ); s.fill( (const char)250 );
    s << hex;
    return s;
}

ostream&  __cdecl hex4( ostream& s ) {
    s.width( 4 ); s.fill( (const char)250 );
    s << hex ;
    return s;
}

ostream&  __cdecl hex6( ostream& s ) {
    s.width( 6 ); s.fill( (const char)250 );
    s << hex ;
    return s;
}

ostream&  __cdecl hex8( ostream& s ) {
    s.width( 8 ); s.fill( (const char)250 );
    s << hex ;
    return s;
}

ostream&  __cdecl dec2( ostream& s ) {
    s << dec;
    s.width( 0 ); s.fill( 0 );
    return s;
}

ostream&  __cdecl filepos( ostream& s, SvStream& rSt ) {
    long pos = rSt.Tell();

    if ( pos != lastpos ){
#ifndef DumpVer8
        if( &rSt == &xStrm )
            s << "D";
        else
            s << "T";
#endif
        s.width( 6 );
        s.fill( (const char)250 );
        s << hex << pos << dec << ' ';
        s.width( 0 ); s.fill( 0 );
        lastpos = pos;
    }else{
#ifndef DumpVer8
        s << "        ";
#else
        s << "       ";
#endif
    }
    return s;
}

ostream&  __cdecl indent( ostream& s, SvStream& rSt ) {
    filepos( s, rSt );
    for( int i = 0; i < level; i++ ) s << "  ";
    return s;
}

ostream&  __cdecl indent1( ostream& s ) {
    for( int i = 0; i < level; i++ ) s << "  ";
    return s;
}

ostream&  __cdecl indent2( ostream& s ) {
#ifndef DumpVer8
    s << "        " << indent1;
#else
    s << "       " << indent1;
#endif
    return s;
}

ostream&  __cdecl begin( ostream& s, SvStream& rSt  ) { indent( s, rSt ) << "BEGIN "; level++; return s; }

// begin1 ohne indent
ostream&  __cdecl begin1( ostream& s ) { s << "BEGIN "; level++; return s; }

// begin2 ohne Nummer
ostream&  __cdecl begin2( ostream& s ) { s << indent2 << "BEGIN "; level++; return s; }

ostream&  __cdecl end( ostream& s, SvStream& rSt  ) { level--; return indent( s, rSt ) << "END "; }

// end1 ohne filepos
ostream&  __cdecl end1( ostream& s ) { level--; return s << indent1 << "END "; }

// end2 ohne Nummer
ostream&  __cdecl end2( ostream& s ) { level--; return s << indent2 << "END "; }

//-----------------------------------------
//          Ausgabe-Funktionen
//-----------------------------------------

void OutBool( SvStream& rSt, short )
{
    sal_uInt8 nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    switch( nPara ){
    case 0:  *pOut << "F"; break;
    case 1:  *pOut << "T"; break;
    default: *pOut << "ERROR:" << (sal_uInt16)nPara; break;
    }
}

void OutBool4( SvStream& rSt, short )
{
    sal_uInt8 nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    switch( nPara ){
    case 0:   *pOut << "F"; break;
    case 1:   *pOut << "T"; break;
    case 128: *pOut << "==Style"; break;
    case 129: *pOut << "!=Style"; break;
    default:  *pOut << "ERROR:" << (sal_uInt16)nPara; break;
    }
}

void OutByte( SvStream& rSt, short )
{
    sal_uInt8 nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << (sal_uInt16)nPara;
}

void OutShort( SvStream& rSt, short )
{
    short nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << nPara;
}

void OutShorts( SvStream& rSt, short nLen )
{
    sal_Int16 nPara;

    for( short i = 0; i < nLen / 2; i++ ){
        rSt.Read( &nPara, sizeof( nPara ) );
        *pOut << nPara << ' ';
    }
}

void OutWord( SvStream& rSt, short )
{
    sal_uInt16 nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << nPara;
}

void OutWords( SvStream& rSt, short nLen )
{
    sal_uInt16 nPara;

    for( short i = 0; i < nLen / 2; i++ ){
        rSt.Read( &nPara, sizeof( nPara ) );
        *pOut << nPara;
    }
}

void OutWordHex( SvStream& rSt, short )
{
    sal_uInt16 nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << "0x" << hex4 << nPara << dec;
}

void OutWordsHex( SvStream& rSt, short nLen )
{
    sal_uInt16 nPara;
    nLen /= sizeof( nPara );
    for( short i = 0; i < nLen; i++ ){
        rSt.Read( &nPara, sizeof( nPara ) );
        *pOut << "0x" << hex4 << nPara << dec;
        if( i < nLen - 1 )
            *pOut << ' ';
    }
}

void OutLongsHex( SvStream& rSt, short nLen )
{
    long nPara;
    nLen /= sizeof( nPara );
    for( short i = 0; i < nLen; i++ )
    {
        rSt.Read( &nPara, sizeof( nPara ) );
        *pOut << "0x" << hex8 << nPara << dec;
        if( i < nLen - 1 )
            *pOut << ' ';
    }
}

void OutLongHex( SvStream& rSt, short )
{
    sal_uLong nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << "0x" << hex8 << nPara << dec;
}

void OutTab68( SvStream& rSt, short )
{
    sal_uLong nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    if(  nPara == 0 )
        *pOut << "None";
    else
        *pOut << "0x" << hex8 << nPara << dec;
}


void OutTab( SvStream& rSt, short )
{
    sal_uInt8 nDel, nIns, nType;
    short nPos, i;

    rSt.Read( &nDel, sizeof( nDel ) );
    *pOut << "Del " << (sal_uInt16)nDel;
    if ( nDel ) *pOut << ": ";
    else        *pOut << ", ";

    for( i=1; i<=nDel; i++){
        rSt.Read( &nPos, sizeof( nPos ) );
        *pOut << nPos;
        if( i<nDel ) *pOut << ',';
        else *pOut << ' ';
    }
    rSt.Read( &nIns, sizeof( nIns ) );
    *pOut << "Ins " << (sal_uInt16)nIns;
    if ( nIns ) *pOut << ": ";

    for( i=1; i<=nIns; i++){
        rSt.Read( &nPos, sizeof( nPos ) );
        *pOut << nPos;
        if( i<nIns ) *pOut << ',';
        else *pOut << ' ';
    }
    if ( nIns ) *pOut << "Typ: ";

    for( i=1; i<=nIns; i++){
        rSt.Read( &nType, sizeof( nType ) );
        *pOut << (sal_uInt16)nType;
        if( i<nIns ) *pOut << ',';
        else *pOut << ' ';
    }

//  nSiz = 1 + 2 * nDel + 1 + nIns * 3; // genaue Laenge,
//      stimmt auch bei Laenge > 256
//  bei diesem Tab-Befehl anscheinend nicht noetig
}

void OutTab190( SvStream& rSt, short nLen )
{
    sal_uInt8 nCols;
    rSt.Read( &nCols, sizeof( nCols ) );
    *pOut << (sal_uInt16)nCols << " Cols: ";

    short nPos, i;
    for( i = 0; i <= nCols; i++ ){
        rSt.Read( &nPos, sizeof( nPos ) );
        *pOut << nPos;
        if( i < nCols )
            *pOut << ", ";
    }
    *pOut << dec << "; ";
    for( i = 0; i < nCols; i++ ){
        *pOut << "Col " << i << " TC: ";
        OutWordsHex( rSt, 10 );
        if( i < nCols - 1 )
            *pOut << "; ";
    }
}

void OutTab191( SvStream& rSt, short nLen )
{
    sal_uInt8 nCols;
    rSt.Read( &nCols, sizeof( nCols ) );
    *pOut << (sal_uInt16)nCols << " Cols, SHDs: ";
    OutWordsHex( rSt, ( nCols + 1 ) * 2 );
}

void OutTab192( SvStream& rSt, short )
{
    OutWord(    rSt, 0 );
    OutWordHex( rSt, 0 );
}

void OutHugeHex( SvStream& rSt, short nLen )
{
    long nPos;
    rSt.Read( &nPos, sizeof( nPos ) );

    long nCurr = xDataStream->Tell();
    xDataStream->Seek( nPos );

    xDataStream->Read( &nLen, sizeof( nLen ) );

//  *pOut << ", Len max: " << nLen << ", ID:" << nIStd << endl1;
    *pOut << endl1;
    DumpSprms( 8, *xDataStream, nLen );

    xDataStream->Seek( nCurr );
}

void OutTabD608( SvStream& rSt, short nLen )
{
    sal_uInt8 nHi, nCols;
    rSt.Read( &nHi, sizeof( nHi ) );
//  nLen += ((short)nHi) << 8;

    rSt.Read( &nCols, sizeof( nCols ) );
    *pOut << " Cols: " << (short)nCols << ' ' << endl1 << indent2;

    short nPos, i;
    for( i = 0; i <= nCols; ++i )
    {
        rSt.Read( &nPos, sizeof( nPos ) );
        *pOut << nPos;
        if( i < nCols )
            *pOut << ", ";
    }
    nLen -= ( nCols + 1 ) * 2;
    nLen /= 20;

    for( i = 0; i < nLen; ++i )
    {
        *pOut << endl1 << indent2 << "Col " << i << " TC: ";
        OutLongsHex( rSt, 20 );
        if( i < nLen - 1 )
            *pOut << "; ";
    }
}

void OutTabD609( SvStream& rSt, short nLen )
{
    *pOut << " Brush(FBS): ";
    for( short i = 0; i < nLen / 2; ++i )
    {
        sal_uInt16 nVal;
        rSt.Read( &nVal, sizeof( nVal ) );
        *pOut << (nVal & 0x1f);
        *pOut << "|" << ((nVal >> 5) & 0x1f);
        *pOut << "|" << ((nVal >> 10) & 0x3f);
        if( i < nLen )
            *pOut << ", ";
    }
}


