/*************************************************************************
 *
 *  $RCSfile: op.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2001-02-26 06:58:59 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <tools/solar.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if defined( MAC ) || defined( ICC )
#include <stdlib.h>
#endif

#include "cell.hxx"
#include "rangenam.hxx"
#include "document.hxx"

#include "op.h"
#include "tool.h"
#include "math.h"
#include "decl.h"
#include "lotform.hxx"
#include "lotrange.hxx"

#include "root.hxx"

#ifndef _FLTTOOLS_HXX
#include "flttools.hxx"
#endif

extern sal_Char*    pAnsi;          // -> memory.cxx, Puffer zum Umwandeln von OEM->ANSI
extern sal_Char*    pErgebnis;      // -> memory.cxx, Ergebnispuffer
extern WKTYP        eTyp;           // -> filter.cxx, aktueller Dateityp
extern BOOL         bEOF;           // -> filter.cxx, zeigt Dateiende an
extern sal_Char*    pPuffer0;       // -> memory.cxx
extern sal_Char*    pPuffer1;
extern BYTE         nDefaultFormat; // -> tool.cxx, Default-Zellenformat
extern ScDocument*  pDoc;           // -> filter.cxx, Aufhaenger zum Dokumentzugriff
extern BYTE*        pFormelBuffer;  // -> memory.cxx, fuer

static UINT16       nDefWidth = ( UINT16 ) ( TWIPS_PER_CHAR * 10 );




void NI( SvStream& r, UINT16 n )
{
    r.SeekRel( n );
}


void OP_BOF( SvStream& r, UINT16 n )
{
    r.SeekRel( 2 );        // Versionsnummer ueberlesen
}


void OP_EOF( SvStream& r, UINT16 n )
{
    bEOF = TRUE;
}


void OP_Integer( SvStream& r, UINT16 n )
{
    BYTE            nFormat;
    UINT16          nCol, nRow, nTab = 0;
    INT16           nValue;

    r >> nFormat >> nCol >> nRow >> nValue;

    ScValueCell*    pZelle = new ScValueCell( ( double ) nValue );
    pDoc->PutCell( nCol, nRow, nTab, pZelle, ( BOOL ) TRUE );

    // 0 Stellen nach'm Komma!
    SetFormat( nCol, nRow, nTab, nFormat, 0 );
}


void OP_Number( SvStream& r, UINT16 n )
{
    BYTE            nFormat;
    UINT16          nCol, nRow, nTab = 0;
    double          fValue;

    r >> nFormat >> nCol >> nRow >> fValue;

    // pErgebnis kurzzeitig als Dummy-Puffer missbrauchen
    sprintf( pErgebnis, "%.15lg", fValue ); // auf 15 Stellen runden
    ScValueCell*    pZelle = new ScValueCell( atof( pErgebnis ) );
    pDoc->PutCell( nCol, nRow, nTab, pZelle, ( BOOL ) TRUE );

    SetFormat( nCol, nRow, nTab, nFormat, nDezFloat );
}


void OP_Label( SvStream& r, UINT16 n )
{
    BYTE            nFormat;
    UINT16          nCol, nRow, nTab = 0;
    sal_Char        pText[ 256 ];

    r >> nFormat >> nCol >> nRow;
    n -= 5;

    r.Read( pText, n );
    pText[ n + 1 ] = 0;   // zur Sicherheit Nullterminator anhaengen
    // der Mega-Hack: bei Text wird HART Text als Format eingebrannt!!!!!!!
    nFormat &= 0x80;    // Bit 7 belassen
    nFormat |= 0x75;    // protected egal, special-text gesetzt

    // Sonderzeichenanpassung
    DosToSystem( pText );

    PutFormString( nCol, nRow, nTab, pText );

    SetFormat( nCol, nRow, nTab, nFormat, nDezStd );
}


void OP_Text( SvStream& r, UINT16 n )        // WK3
{
    UINT16          nRow;
    BYTE            nCol, nTab;
    sal_Char        pText[ 256 ];

    r >> nRow >> nTab >> nCol;
    n -= 4;

    r.Read( pText, n );
    pText[ n ] = 0;   // zur Sicherheit Nullterminator anhaengen

    PutFormString( nCol, nRow, nTab, pText );
}


void OP_Formula( SvStream& r, UINT16 n )
{
    BYTE                nFormat;
    UINT16              nCol, nRow, nTab = 0, nFormulaSize;

    r >> nFormat >> nCol >> nRow;
    r.SeekRel( 8 );    // Ergebnis ueberspringen
    r >> nFormulaSize;

    const ScTokenArray* pErg;
    INT32               nBytesLeft = nFormulaSize;
    ScAddress           aAddress( nCol, nRow, nTab );

    LotusToSc           aConv( r, pLotusRoot->eCharsetQ );
    aConv.Reset( aAddress );
    aConv.Convert( pErg, nBytesLeft );

    ScFormulaCell*      pZelle = new ScFormulaCell( pLotusRoot->pDoc, aAddress, pErg );

    pZelle->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );

    pDoc->PutCell( nCol, nRow, nTab, pZelle, ( BOOL ) TRUE );

    // nFormat = Standard -> Nachkommastellen wie Float
    SetFormat( nCol, nRow, nTab, nFormat, nDezFloat );
}


void OP_ColumnWidth( SvStream& r, UINT16 n )
{
    UINT16              nCol, nBreite, nTab = 0;
    BYTE                nWidthSpaces;

    r >> nCol >> nWidthSpaces;

    if( nWidthSpaces )
        // Annahme: 10cpi-Zeichensatz
        nBreite = ( UINT16 ) ( TWIPS_PER_CHAR * nWidthSpaces );
    else
    {
        pDoc->SetColFlags( nCol, 0, pDoc->GetColFlags( nCol, 0 ) | CR_HIDDEN );
        nBreite = nDefWidth;
    }

    pDoc->SetColWidth( nCol, nTab, nBreite );
}


void OP_NamedRange( SvStream& r, UINT16 n )
    {
    // POST:    waren Koordinaten ungueltig, wird nicht gespeichert
    UINT16              nColSt, nRowSt, nColEnd, nRowEnd;
    sal_Char            cPuffer[ 32 ];

    r.Read( cPuffer, 16 );

    r >> nColSt >> nRowSt >> nColEnd >> nRowEnd;

    LotusRange*         pRange;

    if( nColSt == nColEnd && nRowSt == nRowEnd )
        pRange = new LotusRange( nColSt, nRowSt );
    else
        pRange = new LotusRange( nColSt, nRowSt, nColEnd, nRowEnd );

    if( isdigit( *cPuffer ) )
    {   // erstes Zeichen im Namen eine Zahl -> 'A' vor Namen setzen
        *pAnsi = 'A';
        strcpy( pAnsi + 1, cPuffer );
    }
    else
        strcpy( pAnsi, cPuffer );

    DosToSystem( pAnsi );

    String              aTmp( pAnsi, pLotusRoot->eCharsetQ );

    ScFilterTools::ConvertName( aTmp );

    pLotusRoot->pRangeNames->Append( pRange, aTmp );
}


void OP_SymphNamedRange( SvStream& r, UINT16 n )
{
    // POST:    waren Koordinaten ungueltig, wird nicht gespeichert
    UINT16              nColSt, nRowSt, nColEnd, nRowEnd, nN = 0;
    BYTE                nType;
    sal_Char*           pName;
    sal_Char            cPuffer[ 32 ];

    r.Read( cPuffer, 16 );
    cPuffer[ 16 ] = 0;
    pName = cPuffer;

    r >> nColSt >> nRowSt >> nColEnd >> nRowEnd >> nType;

    LotusRange*         pRange;

    if( nType )
        pRange = new LotusRange( nColSt, nRowSt );
    else
        pRange = new LotusRange( nColSt, nRowSt, nColEnd, nRowEnd );

    if( isdigit( *cPuffer ) )
    {   // erstes Zeichen im Namen eine Zahl -> 'A' vor Namen setzen
        *pAnsi = 'A';
        strcpy( pAnsi + 1, cPuffer );
    }
    else
        strcpy( pAnsi, cPuffer );

    DosToSystem( pAnsi );

    String      aTmp( pAnsi, pLotusRoot->eCharsetQ );
    ScFilterTools::ConvertName( aTmp );

    pLotusRoot->pRangeNames->Append( pRange, aTmp );
}


void OP_Footer( SvStream& r, UINT16 n )
{
    r.SeekRel( n );
}


void OP_Header( SvStream& r, UINT16 n )
{
    r.SeekRel( n );
}


void OP_Margins( SvStream& r, UINT16 n )
{
    r.SeekRel( n );
}


void OP_HiddenCols( SvStream& r, UINT16 n )
{
    UINT16      nByte, nBit, nCount;
    BYTE        nAkt;
    nCount = 0;

    for( nByte = 0 ; nByte < 32 ; nByte++ ) // 32 Bytes mit ...
    {
        r >> nAkt;
        for( nBit = 0 ; nBit < 8 ; nBit++ ) // ...jeweils 8 Bits = 256 Bits
        {
            if( nAkt & 0x01 )   // unterstes Bit gesetzt?
                // -> Hidden Col
                pDoc->SetColFlags( nCount, 0, pDoc->GetColFlags( nCount, 0 ) | CR_HIDDEN );

            nCount++;
            nAkt = nAkt / 2;    // der Nächste bitte...
        }
    }
}


void OP_Window1( SvStream& r, UINT16 n )
{
    r.SeekRel( 4 );    // Cursor Pos ueberspringen

    r >> nDefaultFormat;

    r.SeekRel( 1 );    // 'unused' ueberspringen

    r >> nDefWidth;

    r.SeekRel( n - 8 );  // und den Rest ueberspringen

    nDefWidth = ( UINT16 ) ( TWIPS_PER_CHAR * nDefWidth );

    // statt Defaulteinstellung in SC alle Cols zu Fuss setzen
    for( UINT16 nCol = 0 ; nCol <= MAXCOL ; nCol++ )
        pDoc->SetColWidth( nCol, 0, nDefWidth );
}


void OP_Blank( SvStream& r, UINT16 n )
{
    UINT16      nCol, nRow;
    BYTE        nFormat;
    r >> nFormat >> nCol >> nRow;

    SetFormat( nCol, nRow, 0, nFormat, nDezFloat );
}


