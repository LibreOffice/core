/*************************************************************************
 *
 *  $RCSfile: ww8dout.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>          // getchar

//#include "defs.hxx"
#include <tools/solar.h>            // BYTE
//#include "wwscan.hxx" // aWwStor
#include "ww8dout.hxx"

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

extern SvStorageStreamRef xStrm;
extern SvStorageStreamRef xTableStream;
extern SvStorageStreamRef xDataStream;  // ist bei Ver6-7 mit xStrm identisch,
void DumpSprms( BYTE nVersion, SvStream& rSt, short nLen );

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
    register long pos = rSt.Tell();

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
    BYTE nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    switch( nPara ){
    case 0:  *pOut << "F"; break;
    case 1:  *pOut << "T"; break;
    default: *pOut << "ERROR:" << (USHORT)nPara; break;
    }
}

void OutBool4( SvStream& rSt, short )
{
    BYTE nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    switch( nPara ){
    case 0:   *pOut << "F"; break;
    case 1:   *pOut << "T"; break;
    case 128: *pOut << "==Style"; break;
    case 129: *pOut << "!=Style"; break;
    default:  *pOut << "ERROR:" << (USHORT)nPara; break;
    }
}

void OutByte( SvStream& rSt, short )
{
    BYTE nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << (USHORT)nPara;
}

void OutShort( SvStream& rSt, short )
{
    short nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << nPara;
}

void OutShorts( SvStream& rSt, short nLen )
{
    INT16 nPara;

    for( short i = 0; i < nLen / 2; i++ ){
        rSt.Read( &nPara, sizeof( nPara ) );
        *pOut << nPara << ' ';
    }
}

void OutWord( SvStream& rSt, short )
{
    USHORT nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << nPara;
}

void OutWords( SvStream& rSt, short nLen )
{
    USHORT nPara;

    for( short i = 0; i < nLen / 2; i++ ){
        rSt.Read( &nPara, sizeof( nPara ) );
        *pOut << nPara;
    }
}

void OutWordHex( SvStream& rSt, short )
{
    USHORT nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << "0x" << hex4 << nPara << dec;
}

void OutWordsHex( SvStream& rSt, short nLen )
{
    USHORT nPara;
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
    ULONG nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    *pOut << "0x" << hex8 << nPara << dec;
}

void OutTab68( SvStream& rSt, short )
{
    ULONG nPara;

    rSt.Read( &nPara, sizeof( nPara ) );
    if(  nPara == 0 )
        *pOut << "None";
    else
        *pOut << "0x" << hex8 << nPara << dec;
}


void OutTab( SvStream& rSt, short )
{
    BYTE nDel, nIns, nType;
    short nPos, i;

    rSt.Read( &nDel, sizeof( nDel ) );
    *pOut << "Del " << (USHORT)nDel;
    if ( nDel ) *pOut << ": ";
    else        *pOut << ", ";

    for( i=1; i<=nDel; i++){
        rSt.Read( &nPos, sizeof( nPos ) );
        *pOut << nPos;
        if( i<nDel ) *pOut << ',';
        else *pOut << ' ';
    }
    rSt.Read( &nIns, sizeof( nIns ) );
    *pOut << "Ins " << (USHORT)nIns;
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
        *pOut << (USHORT)nType;
        if( i<nIns ) *pOut << ',';
        else *pOut << ' ';
    }

//  nSiz = 1 + 2 * nDel + 1 + nIns * 3; // genaue Laenge,
//      stimmt auch bei Laenge > 256
//  bei diesem Tab-Befehl anscheinend nicht noetig
}

void OutTab190( SvStream& rSt, short nLen )
{
    BYTE nCols;
    rSt.Read( &nCols, sizeof( nCols ) );
    *pOut << (USHORT)nCols << " Cols: ";

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
    BYTE nCols;
    rSt.Read( &nCols, sizeof( nCols ) );
    *pOut << (USHORT)nCols << " Cols, SHDs: ";
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
    BYTE nHi, nCols;
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
        UINT16 nVal;
        rSt.Read( &nVal, sizeof( nVal ) );
        *pOut << (nVal & 0x1f);
        *pOut << "|" << ((nVal >> 5) & 0x1f);
        *pOut << "|" << ((nVal >> 10) & 0x3f);
        if( i < nLen )
            *pOut << ", ";
    }
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/dump/ww8dout.cxx,v 1.1.1.1 2000-09-18 17:14:59 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.9  2000/09/18 16:05:03  willem.vandorp
      OpenOffice header added.

      Revision 1.8  2000/02/11 14:39:38  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.7  1999/11/02 18:06:52  jp
      Dump of hugepapx

      Revision 1.6  1999/10/28 20:35:15  jp
      write table shading vakues

      Revision 1.5  1999/10/27 17:58:09  jp
      changes for tables

      Revision 1.4  1999/06/15 12:14:44  JP
      new: ListTable and other Contents


      Rev 1.3   15 Jun 1999 14:14:44   JP
   new: ListTable and other Contents

      Rev 1.2   25 Jun 1998 15:44:42   KHZ
   Strukturaenderung fuer PAPX FKPs

      Rev 1.1   10 Jun 1998 17:22:34   KHZ
   Zwischenstand-Sicherung Dumper

      Rev 1.0   27 May 1998 15:30:18   KHZ
   Initial revision.


*************************************************************************/


