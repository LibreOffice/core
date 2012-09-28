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

#include <tools/solar.h>
#include <rtl/math.hxx>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>

#include "scitems.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include <svx/algitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>

#include "cell.hxx"
#include "rangenam.hxx"
#include "document.hxx"
#include "postit.hxx"

#include "op.h"
#include "optab.h"
#include "tool.h"
#include "decl.h"
#include "lotform.hxx"
#include "lotrange.hxx"

#include "root.hxx"

#include "ftools.hxx"

#include <vector>
#include <map>

extern WKTYP eTyp;           // -> filter.cxx, aktueller Dateityp
extern sal_Bool bEOF;           // -> filter.cxx, zeigt Dateiende an
extern sal_uInt8 nDefaultFormat; // -> tool.cxx, Default-Zellenformat
extern ScDocument* pDoc;           // -> filter.cxx, Aufhaenger zum Dokumentzugriff
extern CharSet eCharVon;       // -> filter.cxx, character set specified

static sal_uInt16 nDefWidth = ( sal_uInt16 ) ( TWIPS_PER_CHAR * 10 );

extern std::map<sal_uInt16, ScPatternAttr> aLotusPatternPool;

void NI( SvStream& r, sal_uInt16 n )
{
    r.SeekRel( n );
}


void OP_BOF( SvStream& r, sal_uInt16 /*n*/ )
{
    r.SeekRel( 2 );        // Versionsnummer ueberlesen
}


void OP_EOF( SvStream& /*r*/, sal_uInt16 /*n*/ )
{
    bEOF = sal_True;
}


void OP_Integer( SvStream& r, sal_uInt16 /*n*/ )
{
    sal_uInt8           nFormat;
    sal_uInt16          nCol, nRow;
    SCTAB           nTab = 0;
    sal_Int16           nValue;

    r >> nFormat >> nCol >> nRow >> nValue;

    if (ValidColRow( static_cast<SCCOL>(nCol), nRow))
    {
        ScValueCell*  pZelle = new ScValueCell( ( double ) nValue );
        pDoc->PutCell( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab, pZelle, true );

        // 0 Stellen nach'm Komma!
        SetFormat( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab, nFormat, 0 );
    }
}


void OP_Number( SvStream& r, sal_uInt16 /*n*/ )
{
    sal_uInt8           nFormat;
    sal_uInt16          nCol, nRow;
    SCTAB           nTab = 0;
    double          fValue;

    r >> nFormat >> nCol >> nRow >> fValue;

    if (ValidColRow( static_cast<SCCOL>(nCol), nRow))
    {
        fValue = ::rtl::math::round( fValue, 15 );
        ScValueCell*  pZelle = new ScValueCell( fValue );
        pDoc->PutCell( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab, pZelle, true );

        SetFormat( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab, nFormat, nDezFloat );
    }
}


void OP_Label( SvStream& r, sal_uInt16 n )
{
    sal_uInt8           nFormat;
    sal_uInt16          nCol, nRow;
    SCTAB           nTab = 0;

    r >> nFormat >> nCol >> nRow;

    n -= (n > 5) ? 5 : n;

    sal_Char* pText = new sal_Char[n + 1];
    r.Read( pText, n );
    pText[n] = 0;

    if (ValidColRow( static_cast<SCCOL>(nCol), nRow))
    {
        nFormat &= 0x80;    // Bit 7 belassen
        nFormat |= 0x75;    // protected egal, special-text gesetzt

        PutFormString( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab, pText );

        SetFormat( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab, nFormat, nDezStd );
    }

    delete [] pText;
}

void OP_Formula( SvStream& r, sal_uInt16 /*n*/ )
{
    sal_uInt8               nFormat;
    sal_uInt16              nCol, nRow, nFormulaSize;
    SCTAB                   nTab = 0;

    r >> nFormat >> nCol >> nRow;
    r.SeekRel( 8 );    // Ergebnis ueberspringen
    r >> nFormulaSize;

    const ScTokenArray* pErg;
    sal_Int32               nBytesLeft = nFormulaSize;
    ScAddress           aAddress( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab );

    LotusToSc           aConv( r, pLotusRoot->eCharsetQ, false );
    aConv.Reset( aAddress );
    aConv.Convert( pErg, nBytesLeft );

    if (ValidColRow( static_cast<SCCOL>(nCol), nRow))
    {
        ScFormulaCell*    pZelle = new ScFormulaCell( pLotusRoot->pDoc, aAddress, pErg );

        pZelle->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );

        pDoc->PutCell( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab, pZelle, true );

        // nFormat = Standard -> Nachkommastellen wie Float
        SetFormat( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), nTab, nFormat, nDezFloat );
    }
}


void OP_ColumnWidth( SvStream& r, sal_uInt16 /*n*/ )
{
    sal_uInt16              nCol, nBreite;
    sal_uInt8               nWidthSpaces;
    SCTAB                   nTab = 0;

    r >> nCol >> nWidthSpaces;

    if (ValidCol( static_cast<SCCOL>(nCol)))
    {
        if( nWidthSpaces )
            // Annahme: 10cpi-Zeichensatz
            nBreite = ( sal_uInt16 ) ( TWIPS_PER_CHAR * nWidthSpaces );
        else
        {
            pDoc->SetColHidden(static_cast<SCCOL>(nCol), static_cast<SCCOL>(nCol), 0, true);
            nBreite = nDefWidth;
        }

        pDoc->SetColWidth( static_cast<SCCOL> (nCol), nTab, nBreite );
    }
}


void OP_NamedRange( SvStream& r, sal_uInt16 /*n*/ )
    {
    // POST:    waren Koordinaten ungueltig, wird nicht gespeichert
    sal_uInt16              nColSt, nRowSt, nColEnd, nRowEnd;

    sal_Char cPuffer[ 16+1 ];
    r.Read( cPuffer, 16 );
    cPuffer[ 16 ] = 0;

    r >> nColSt >> nRowSt >> nColEnd >> nRowEnd;

    if (ValidColRow( static_cast<SCCOL>(nColSt), nRowSt) && ValidColRow( static_cast<SCCOL>(nColEnd), nRowEnd))
    {
        LotusRange*      pRange;

        if( nColSt == nColEnd && nRowSt == nRowEnd )
            pRange = new LotusRange( static_cast<SCCOL> (nColSt), static_cast<SCROW> (nRowSt) );
        else
            pRange = new LotusRange( static_cast<SCCOL> (nColSt), static_cast<SCROW> (nRowSt),
                    static_cast<SCCOL> (nColEnd), static_cast<SCROW> (nRowEnd) );

        sal_Char cBuf[sizeof(cPuffer)+1];
        if( isdigit( *cPuffer ) )
        {  // erstes Zeichen im Namen eine Zahl -> 'A' vor Namen setzen
            cBuf[0] = 'A';
            strcpy( cBuf + 1, cPuffer );       // #100211# - checked
        }
        else
            strcpy( cBuf, cPuffer );           // #100211# - checked

        String        aTmp( cBuf, pLotusRoot->eCharsetQ );

        ScfTools::ConvertToScDefinedName( aTmp );

        pLotusRoot->pRangeNames->Append( pRange, aTmp );
    }
}


void OP_SymphNamedRange( SvStream& r, sal_uInt16 /*n*/ )
{
    // POST:    waren Koordinaten ungueltig, wird nicht gespeichert
    sal_uInt16              nColSt, nRowSt, nColEnd, nRowEnd;
    sal_uInt8               nType;

    sal_Char cPuffer[ 16+1 ];
    r.Read( cPuffer, 16 );
    cPuffer[ 16 ] = 0;

    r >> nColSt >> nRowSt >> nColEnd >> nRowEnd >> nType;

    if (ValidColRow( static_cast<SCCOL>(nColSt), nRowSt) && ValidColRow( static_cast<SCCOL>(nColEnd), nRowEnd))
    {
        LotusRange*      pRange;

        if( nType )
            pRange = new LotusRange( static_cast<SCCOL> (nColSt), static_cast<SCROW> (nRowSt) );
        else
            pRange = new LotusRange( static_cast<SCCOL> (nColSt), static_cast<SCROW> (nRowSt),
                    static_cast<SCCOL> (nColEnd), static_cast<SCROW> (nRowEnd) );

        sal_Char cBuf[sizeof(cPuffer)+1];
        if( isdigit( *cPuffer ) )
        {  // erstes Zeichen im Namen eine Zahl -> 'A' vor Namen setzen
            cBuf[0] = 'A';
            strcpy( cBuf + 1, cPuffer );       // #100211# - checked
        }
        else
            strcpy( cBuf, cPuffer );           // #100211# - checked

        String    aTmp( cBuf, pLotusRoot->eCharsetQ );
        ScfTools::ConvertToScDefinedName( aTmp );

        pLotusRoot->pRangeNames->Append( pRange, aTmp );
    }
}


void OP_Footer( SvStream& r, sal_uInt16 n )
{
    r.SeekRel( n );
}


void OP_Header( SvStream& r, sal_uInt16 n )
{
    r.SeekRel( n );
}


void OP_Margins( SvStream& r, sal_uInt16 n )
{
    r.SeekRel( n );
}


void OP_HiddenCols( SvStream& r, sal_uInt16 /*n*/ )
{
    sal_uInt16      nByte, nBit;
    SCCOL       nCount;
    sal_uInt8       nAkt;
    nCount = 0;

    for( nByte = 0 ; nByte < 32 ; nByte++ ) // 32 Bytes mit ...
    {
        r >> nAkt;
        for( nBit = 0 ; nBit < 8 ; nBit++ ) // ...jeweils 8 Bits = 256 Bits
        {
            if( nAkt & 0x01 )   // unterstes Bit gesetzt?
                // -> Hidden Col
                pDoc->SetColHidden(nCount, nCount, 0, true);

            nCount++;
            nAkt = nAkt / 2;    // der Naechste bitte...
        }
    }
}


void OP_Window1( SvStream& r, sal_uInt16 n )
{
    r.SeekRel( 4 );    // Cursor Pos ueberspringen

    r >> nDefaultFormat;

    r.SeekRel( 1 );    // 'unused' ueberspringen

    r >> nDefWidth;

    r.SeekRel( n - 8 );  // und den Rest ueberspringen

    nDefWidth = ( sal_uInt16 ) ( TWIPS_PER_CHAR * nDefWidth );

    // statt Defaulteinstellung in SC alle Cols zu Fuss setzen
    for( SCCOL nCol = 0 ; nCol <= MAXCOL ; nCol++ )
        pDoc->SetColWidth( nCol, 0, nDefWidth );
}


void OP_Blank( SvStream& r, sal_uInt16 /*n*/ )
{
    sal_uInt16      nCol, nRow;
    sal_uInt8       nFormat;
    r >> nFormat >> nCol >> nRow;

    SetFormat( static_cast<SCCOL> (nCol), static_cast<SCROW> (nRow), 0, nFormat, nDezFloat );
}

void OP_BOF123( SvStream& r, sal_uInt16 /*n*/ )
{
    r.SeekRel( 26 );
}


void OP_EOF123( SvStream& /*r*/, sal_uInt16 /*n*/ )
{
    bEOF = sal_True;
}

void OP_Label123( SvStream& r, sal_uInt16 n )
{
    sal_uInt8      nTab, nCol;
    sal_uInt16    nRow;
    r >> nRow >> nTab >> nCol;
    n -= (n > 4) ? 4 : n;

    sal_Char* pText = new sal_Char[n + 1];
    r.Read( pText, n );
    pText[ n ] = 0;

    PutFormString( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), static_cast<SCTAB>(nTab), pText );

    delete []pText;
}

void OP_Number123( SvStream& r, sal_uInt16 /*n*/ )
{
    sal_uInt8    nCol,nTab;
    sal_uInt16  nRow;
    sal_uInt32   nValue;

    r >> nRow >> nTab >> nCol >> nValue;

    if (ValidColRow( static_cast<SCCOL>(nCol), nRow) && nTab <= pDoc->GetMaxTableNumber())
    {
        double fValue = Snum32ToDouble( nValue );

        ScValueCell *pCell = new ScValueCell( fValue );
        pDoc->PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), static_cast<SCTAB>(nTab), pCell, true );
    }
}

void OP_Formula123( SvStream& r, sal_uInt16 n )
{
    sal_uInt8 nCol,nTab;
    sal_uInt16 nRow;

    r >> nRow >> nTab >> nCol;
    r.SeekRel( 8 );    // Result- jump over

    const ScTokenArray* pErg;
    sal_Int32 nBytesLeft = (n > 12) ? n - 12 : 0;
    ScAddress aAddress( nCol, nRow, nTab );

    LotusToSc aConv( r, pLotusRoot->eCharsetQ, sal_True );
    aConv.Reset( aAddress );
    aConv.Convert( pErg, nBytesLeft );

    if (ValidColRow( static_cast<SCCOL>(nCol), nRow) && nTab <= pDoc->GetMaxTableNumber())
    {
        ScFormulaCell* pCell = new ScFormulaCell( pLotusRoot->pDoc, aAddress, pErg );

        pCell->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );

        pDoc->PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), static_cast<SCTAB>(nTab), pCell, true );
    }
}

void OP_IEEENumber123( SvStream& r, sal_uInt16 /*n*/ )
{
    sal_uInt8 nCol,nTab;
    sal_uInt16 nRow;
    double dValue;

    r >> nRow >> nTab >> nCol >> dValue;

    if (ValidColRow( static_cast<SCCOL>(nCol), nRow) && nTab <= pDoc->GetMaxTableNumber())
    {
        ScValueCell *pCell = new ScValueCell(dValue);
        pDoc->PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), static_cast<SCTAB>(nTab), pCell, true );
    }
}

void OP_Note123( SvStream& r, sal_uInt16 n)
{
    sal_uInt8 nTab, nCol;
    sal_uInt16 nRow;
    r >> nRow >> nTab >> nCol;
    n -= (n > 4) ? 4 : n;

    sal_Char* pText = new sal_Char[n + 1];
    r.Read( pText, n );
    pText[ n ] = 0;

    String aNoteText(pText,pLotusRoot->eCharsetQ);
    delete [] pText;

    ScAddress aPos( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), static_cast<SCTAB>(nTab) );
    ScNoteUtil::CreateNoteFromString( *pDoc, aPos, aNoteText, false, false );
}

void OP_HorAlign123( sal_uInt8 nAlignPattern, SfxItemSet& rPatternItemSet )
{
//      pre:  Pattern is stored in the last 3 bites of the 21st byte
//      post: Appropriate Horizontal Alignement is set in rPattern according to the bit pattern.
//
//      LEFT:001, RIGHT:010, CENTER:011, JUSTIFY:110,
//      LEFT-Text/RIGHT-NUMBER:100, DEFAULT:000

    nAlignPattern = ( nAlignPattern & 0x07);

    switch (nAlignPattern)
    {
        case 1:
            rPatternItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY ) );
            break;
          case 2:
            rPatternItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY ) );
            break;
        case 3:
            rPatternItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY) );
            break;
        case 4:
            rPatternItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY ) );
            break;
        case 6:
            rPatternItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_BLOCK, ATTR_HOR_JUSTIFY ) );
            break;
        default:
            rPatternItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY ) );
            break;
      }
}

void OP_VerAlign123( sal_uInt8 nAlignPattern,SfxItemSet& rPatternItemSet  )
{
//      pre:  Pattern is stored in the last 3 bites of the 22nd byte
//      post: Appropriate Verticle Alignement is set in rPattern according to the bit pattern.
//
//      TOP:001, MIDDLE:010, DOWN:100, DEFAULT:000

    nAlignPattern = ( nAlignPattern & 0x07);

    switch (nAlignPattern)
    {
        case 0:
            rPatternItemSet.Put( SvxVerJustifyItem(SVX_VER_JUSTIFY_STANDARD, ATTR_VER_JUSTIFY) );
            break;
        case 1:
            rPatternItemSet.Put( SvxVerJustifyItem(SVX_VER_JUSTIFY_TOP, ATTR_VER_JUSTIFY) );
            break;
        case 2:
            rPatternItemSet.Put( SvxVerJustifyItem(SVX_VER_JUSTIFY_CENTER, ATTR_VER_JUSTIFY) );
            break;
        case 4:
            rPatternItemSet.Put( SvxVerJustifyItem(SVX_VER_JUSTIFY_BOTTOM, ATTR_VER_JUSTIFY) );
            break;
        default:
            rPatternItemSet.Put( SvxVerJustifyItem(SVX_VER_JUSTIFY_STANDARD, ATTR_VER_JUSTIFY) );
            break;
    }
}

void OP_CreatePattern123( SvStream& r, sal_uInt16 n)
{
    sal_uInt16 nCode,nPatternId;

    ScPatternAttr aPattern(pDoc->GetPool());
    SfxItemSet& rItemSet = aPattern.GetItemSet();

    r >> nCode;
    n -= (n > 2) ? 2 : n;

    if ( nCode == 0x0fd2 )
    {
        r >> nPatternId;

        sal_uInt8 Hor_Align, Ver_Align, temp;
        sal_Bool bIsBold,bIsUnderLine,bIsItalics;

        r.SeekRel(12);

        // Read 17th Byte
        r >> temp;

        bIsBold = (temp & 0x01);
        bIsItalics = (temp & 0x02);
        bIsUnderLine = (temp & 0x04);

        if ( bIsBold )
            rItemSet.Put( SvxWeightItem(WEIGHT_BOLD,ATTR_FONT_WEIGHT) );
        if ( bIsItalics )
            rItemSet.Put( SvxPostureItem(ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
        if ( bIsUnderLine )
            rItemSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE, ATTR_FONT_UNDERLINE ) );

        r.SeekRel(3);

        // Read 21st Byte
        r >> Hor_Align;
        OP_HorAlign123( Hor_Align, rItemSet );

        r >> Ver_Align;
        OP_VerAlign123( Ver_Align, rItemSet );

        aLotusPatternPool.insert( std::map<sal_uInt16, ScPatternAttr>::value_type( nPatternId, aPattern ) );
        n -= (n > 20) ? 20 : n;
    }
    r.SeekRel(n);
}

void OP_SheetName123( SvStream& rStream, sal_uInt16 nLength )
{
    if (nLength <= 4)
    {
        rStream.SeekRel(nLength);
        return;
    }

    // B0 36 [sheet number (2 bytes?)] [sheet name (null terminated char array)]

    sal_uInt16 nDummy;
    rStream >> nDummy; // ignore the first 2 bytes (B0 36).
    rStream >> nDummy;
    SCTAB nSheetNum = static_cast<SCTAB>(nDummy);
    pDoc->MakeTable(nSheetNum);

    ::std::vector<sal_Char> sSheetName;
    sSheetName.reserve(nLength-4);
    for (sal_uInt16 i = 4; i < nLength; ++i)
    {
        sal_Char c;
        rStream >> c;
        sSheetName.push_back(c);
    }

    if (!sSheetName.empty())
    {
        String aName(&sSheetName[0], eCharVon);
        pDoc->RenameTab(nSheetNum, aName);
    }
}

void OP_ApplyPatternArea123( SvStream& rStream )
{
    sal_uInt16 nOpcode, nLength;
    sal_uInt16 nCol = 0, nColCount = 0, nRow = 0, nRowCount = 0, nTab = 0, nData, nTabCount = 0, nLevel = 0;

    do
    {
        rStream >> nOpcode >> nLength;
        switch ( nOpcode )
        {
            case ROW_FORMAT_MARKER:
                nLevel++;
                break;
            case COL_FORMAT_MARKER:
                nLevel--;
                if( nLevel == 1 )
                {
                    nTab = nTab + nTabCount;
                    nCol = 0; nColCount = 0;
                    nRow = 0; nRowCount = 0;
                }
                break;
            case LOTUS_FORMAT_INDEX:
                if( nLength >= 2 )
                {
                    rStream >> nData;
                    rStream.SeekRel( nLength - 2 );
                    if( nLevel == 1 )
                        nTabCount = nData;
                    else if( nLevel == 2 )
                    {
                        nCol = nCol + nColCount;
                        nColCount = nData;
                        if ( nCol > 0xff ) // 256 is the max col size supported by 123
                            nCol = 0;
                    }
                    else if( nLevel == 3 )
                    {
                        nRow = nRow + nRowCount;
                        nRowCount = nData;
                        if ( nRow > 0x1fff ) // 8192 is the max row size supported by 123
                            nRow = 0;
                    }
                }
                else
                    rStream.SeekRel( nLength );
                break;
            case LOTUS_FORMAT_INFO:
                if( nLength >= 2 )
                {
                    rStream >> nData;
                    rStream.SeekRel( nLength - 2 );
                    std::map<sal_uInt16, ScPatternAttr>::iterator loc = aLotusPatternPool.find( nData );
                    // #126338# apparently, files with invalid index occur in the wild -> don't crash then
                    if ( loc != aLotusPatternPool.end() )
                        for( int i = 0; i < nTabCount; i++)
                        {
                            pDoc->ApplyPatternAreaTab( nCol, nRow, nCol +  nColCount - 1, nRow + nRowCount - 1, static_cast< SCTAB >( nTab + i ), loc->second );
                        }
                }
                else
                    rStream.SeekRel( nLength );
                break;
            default:
                rStream.SeekRel( nLength );
                break;
        }
    }
    while( nLevel && !rStream.IsEof() );

    aLotusPatternPool.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
