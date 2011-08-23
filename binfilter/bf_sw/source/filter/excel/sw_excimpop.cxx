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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <stdio.h>

#include <tools/color.hxx>
#include <bf_svtools/zforlist.hxx>

#include "exlpar.hxx"
#include "exccrts.hxx"
#include "excxfbuf.hxx"
#include "excfntbf.hxx"
#include "excvfbff.hxx"
#include <osl/endian.h>
namespace binfilter {

const sal_Char *SwExcelParser::pBuildInName[] = {
    "BI_Consolidate_Area","BI_Auto_Open","BI_Auto_Close",
    "BI_Extract","BI_Database","BI_Criteria","BI_Print_Area",
    "BI_Print_Titles","BI_Recorder","BI_Data_Form",
    "BI_Auto_Activate","BI_Auto_Deactivate","BI_SheetTitle" };




// ------------------------------------------------------------------ 0x00
void SwExcelParser::Dimensions()
    {
    UINT16 nCS, nCE, nRS, nRE;

    *pIn >> nRS >> nRE >> nCS >> nCE;
    nBytesLeft -= 8;

    nRE--;	// ...Excel-Eigenheit
    nCE--;

    pExcGlob->ColLimitter( nCS );	// Cols/Rows in ihre Schranken verweisen
    pExcGlob->ColLimitter( nCE );

    pExcGlob->RowLimitter( nRS );
    pExcGlob->RowLimitter( nRE );

    pExcGlob->SetRange( nCS, nCE, nRS, nRE );
    }

// ------------------------------------------------------------------ 0x01
void SwExcelParser::Blank25()
    {
    UINT16 nRow, nCol, nXF;

    *pIn >> nRow >> nCol;
    nBytesLeft -= 4;
    if( eDateiTyp == Biff5 )
        {
        *pIn >> nXF;
        nBytesLeft -= 2;
        }
    else
        {
        pIn->SeekRel( 3 );
        nBytesLeft -= 3;
        nXF = 0;
        }

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );
        }
    }

// ------------------------------------------------------------------ 0x03
void SwExcelParser::Number25()
    {
    UINT16 nRow, nCol, nXF;
    double fValue;

    *pIn >> nRow >> nCol;
    nBytesLeft -= 4;
    if( eDateiTyp == Biff5 )
        {
        *pIn >> nXF;
        nBytesLeft -= 2;
        }
    else
        {
        pIn->SeekRel( 3 );
        nBytesLeft -= 3;
        nXF = 0;
        }

    *pIn >> fValue;
    nBytesLeft -= 8;

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );

        PutCell( nCol, nRow, fValue, nXF );
        }
    }

// ------------------------------------------------------------------ 0x04
void SwExcelParser::Label25()
    {
    UINT16 nRow, nCol, nXF;

    if( eDateiTyp == Biff5 )
        {
        *pIn >> nRow >> nCol >> nXF;
        nBytesLeft -= 6;

        ReadExcString( LenWord );
        }
    else
        {// nur fuer BIFF2
        BYTE nAttr0, nAttr1, nAttr2;

        *pIn >> nRow >> nCol >> nAttr0 >> nAttr1 >> nAttr2;
        nBytesLeft -= 7;

        ReadExcString( LenByte );

        nXF = nAttr0 & 0x3F;
        if( nXF == 63 )
            // IXFE-record stand davor
            nXF = nIxfeIndex;
        }

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );

        String aSvStr( pReadBuff, eZielChar );
        PutCell( nCol, nRow, aSvStr, nXF );
        }
    }

// ------------------------------------------------------------------ 0x06
void SwExcelParser::Formula25()
    {
    UINT16 nRow, nCol, nXF, nFormLen;
    double fCurVal;
    BYTE nAttr0, nFlag0;

    *pIn >> nRow >> nCol;
    nBytesLeft -= 4;

    if( eDateiTyp == Biff2 )
        {//						BIFF2
        BYTE nDummy;
        *pIn >> nAttr0;
        pIn->SeekRel( 2 );
        nBytesLeft -= 3;

        *pIn >> fCurVal;
        pIn->SeekRel( 1 );
        *pIn >> nDummy;
        nFormLen = nDummy;
        nBytesLeft -= 10;
        }
    else
        {//						BIFF5
        *pIn >> nXF >> fCurVal >> nFlag0;
        pIn->SeekRel( 5 );

        *pIn >> nFormLen;
        nBytesLeft -= 18;
        }

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );

        aFltTab.SetXF( nCol, nRow, nXF );

        PutCell( nCol, nRow, fCurVal, nXF );
        }
    }


// --------------------------------------------------------- 0x07 + 0x0207
void SwExcelParser::REC_String()
    {
    if( pExcGlob->IsInRange( nLastCol, nLastRow ) )
        {
        if( eDateiTyp == Biff2 )
            ReadExcString( LenByte );
        else
            ReadExcString( LenWord );

        String aSvStr( pReadBuff, eZielChar );
        PutCell( nLastCol, nLastRow, aSvStr, nLastXF );
        }
    }
// ------------------------------------------------------------------ 0x09
void SwExcelParser::Bof2()
    {
    //POST:	eDateiTyp = Biff2
    UINT16 nTyp;

    pIn->SeekRel( 2 );
    *pIn >> nTyp;
    nBytesLeft -= 4;

#ifdef USED
    ResetBof();
#endif

    if( nTyp == 0x0010 )	// Worksheet?
        {
        //eAktZust = ImpZustTabBase;
        eDateiTyp = Biff2;
        }
    else
        {
        //eAktZust = ImpZustInit;	// -> ueberlesen von Nicht-Worksheets
        eDateiTyp = BiffX;
        }
    }

// ------------------------------------------------------------------ 0x0A
void SwExcelParser::Eof()
    {
    eDateiTyp = BiffX;
    }

// ------------------------------------------------------------------ 0x1E
void SwExcelParser::Format235()
    {
    String aFormString;

    if( eDateiTyp == Biff5 || eDateiTyp == Biff5W )
        {
        pIn->SeekRel( 2 );
        nBytesLeft -= 2;
        }

    ReadExcString( LenByte, aFormString );
    pValueFormBuffer->NewValueFormat( aFormString );
    }


// ------------------------------------------------------------------ 0x24
void SwExcelParser::Colwidth()
    {// Column Width
    BYTE nColFirst, nColLast;
    UINT16 nColWidth;

    *pIn >> nColFirst >> nColLast >> nColWidth;
    nBytesLeft -= 4;

    nColWidth = ( UINT16 ) ( fExcToTwips * nColWidth );

    aColRowBuff.SetWidthRange( nColFirst, nColLast, nColWidth );
    }


// ------------------------------------------------------------------ 0x31
void SwExcelParser::Font25()
    {
    UINT16	nHeight, nIndexCol;
    BYTE	nAttr0;
    String	aName;

    if( eDateiTyp == Biff2 )
        {// Biff2
        *pIn >> nHeight >> nAttr0;
        pIn->SeekRel( 1 );
        nBytesLeft -= 4;
        nIndexCol =  32767;

        ReadExcString( LenByte, aName );

        // Font in Pool batschen
        pExcGlob->pFontBuff->NewFont( nHeight, nAttr0, nIndexCol, aName );
        }
    else
        {// Biff5
        BYTE	nUnderline, nFamily, nCharSet;
        UINT16	nWeight;

        *pIn >> nHeight >> nAttr0;
        pIn->SeekRel( 1 );
        *pIn >> nIndexCol >> nWeight;
        pIn->SeekRel( 2 );	// Super/Sub
        *pIn >> nUnderline >> nFamily >> nCharSet;
        pIn->SeekRel( 1 );	// Reserved
        nBytesLeft -= 14;

        ReadExcString( LenByte, aName );

        // Font in Pool batschen
        pExcGlob->pFontBuff->NewFont(
            nHeight, nAttr0, nUnderline, nIndexCol, nWeight,
            nFamily, nCharSet, aName );
        }
    }

// ------------------------------------------------------------------ 0x42
void SwExcelParser::Codepage()
    {
    UINT16 nPage;

    *pIn >> nPage;
    nBytesLeft -= 2;

    switch( nPage )
        {
        case 0x01B5:	// IBM PC 437 (Multiplan)
            eQuellChar = RTL_TEXTENCODING_IBM_437;
            break;
        case 0x8000:	// Apple Macintosh
        case 0x2710:	// ???????????????????????????????????????????????
            eQuellChar = RTL_TEXTENCODING_APPLE_ROMAN;
            break;
        case 0x04E4:	// ANSI (Windows ) Biff4+5
        case 0x8001:	// ANSI (Windows ) Biff2+3
            eQuellChar = RTL_TEXTENCODING_MS_1252;
            break;
        }
    }

// ------------------------------------------------------------------ 0x43
void SwExcelParser::XF2()
    {
    BYTE nDummyFormat, nDummyFont, nAttr;
    UINT16 nIndexFormat, nIndexFont;
    CellBorder aBorder;
    XF_Data *pXFD = new XF_Data;

    pXFD->SetCellXF();		// keine Style-XFs in Biff2

    *pIn >> nDummyFont;
    pIn->SeekRel( 1 );
    *pIn >> nDummyFormat;
    *pIn >> nAttr;
    nBytesLeft -= 4;

    nIndexFormat = nDummyFormat &  0x3F;
    nIndexFont = nDummyFont;

    if( nAttr & 0x08 )
        aBorder.nLeftLine = 1;	// = durchgezogen, duenn
    else
        aBorder.nLeftLine = 0;	// = keine Linie

    if( nAttr & 0x10 )
        aBorder.nRightLine = 1;
    else
        aBorder.nRightLine = 0;

    if( nAttr & 0x20 )
        aBorder.nTopLine = 1;
    else
        aBorder.nTopLine = 0;

    if( nAttr & 0x40 )
        aBorder.nBottomLine = 1;
    else
        aBorder.nBottomLine = 0;

    aBorder.nLeftColor = aBorder.nRightColor = aBorder.nTopColor =
        aBorder.nBottomColor = 8;	// = schwarz?

    pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nIndexFormat ) );
    pXFD->SetBorder( aBorder );
    pXFD->SetFont( nIndexFont );
    pXFD->SetAlign( ( ExcHorizAlign ) ( nAttr & 0x07 ) );

    pExcGlob->pXF_Buff->NewXF( pXFD );
}

// ------------------------------------------------------------------ 0x44
void SwExcelParser::Ixfe()
{
    *pIn >> nIxfeIndex;
    nBytesLeft -= 2;
}

// ------------------------------------------------------------------ 0x55
void SwExcelParser::DefColWidth()
{
    UINT16 nWidth;
    *pIn >> nWidth;
    nBytesLeft -= 2;

    //nWidth = ( UINT16 ) ( TWIPS_PER_CHAR * nWidth );
    nWidth = ( UINT16 ) ( ((20.0 * 72.27) / 13.6) * nWidth );

    aColRowBuff.SetDefWidth( nWidth );
}

// ------------------------------------------------------------------ 0x7D
void SwExcelParser::Colinfo()
    {// Column Formatting Information
    UINT16 nColFirst, nColLast, nColWidth, nXF;
    BYTE nOpt0, nOpt1;

    *pIn >> nColFirst >> nColLast >> nColWidth >> nXF >> nOpt0 >> nOpt1;
    nBytesLeft -= 10;

    nColWidth = ( UINT16 ) ( fExcToTwips * nColWidth );

    aColRowBuff.SetWidthRange( nColFirst, nColLast, nColWidth );
    }

// ------------------------------------------------------------------ 0x7E
void SwExcelParser::Rk()
    {// Cell Value, RK Number
    UINT16 nRow, nCol, nXF;
    UINT32 nRkNum;

    *pIn >> nRow >> nCol >> nXF >> nRkNum;
    nBytesLeft -= 10;

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );

        PutCell( nCol, nRow, RkToDouble( nRkNum ), nXF );
        }
    }

/*
// ------------------------------------------------------------------ 0x85
void SwExcelParser::Boundsheet()
    {
    BYTE nOpt0, nOpt1;
    UINT32 nStrPos;
    String aName;

    if( eDateiTyp == Biff5W )
        {
        *pIn >> nStrPos;
        *pIn >> nOpt0 >> nOpt1;
        nBytesLeft -= 6;

        if( nOpt0 == 0x00 )	// nur Namen von Worksheets speichern
            {
            ReadExcString( LenByte, aName );
            aName.Convert( eQuellChar, eZielChar );
            //aTabNameBuff << aName;
//			aExcDok.NewTab( aName, nStrPos );			####################################
            }
        }
    else
        {
        ReadExcString( LenByte, aName );

        aName.Convert( eQuellChar, eZielChar );
        //aTabNameBuff << aName;
//		aExcDok.Set( aName, nTab );						####################################
        nTab++;
        }

//	if( nOpt0 == 0x00 )	// nur Namen von Worksheets speichern
//		{
//		aDataPool.InsertEntry( aAnsiName, EET_Table, aAnsiName,
//			aTabNameBuff.GetLastIndex(), 0, 0xFFFF, 0, 0xFFFF );
        // zunaechst gesamte Tabelle selektieren, da Groesse noch nicht bekannt
//		}
    }
*/

// ------------------------------------------------------------------ 0x92
void SwExcelParser::Palette()
    {
    UINT16 nAnz;
    BYTE nRed, nGreen, nBlue, nDummy;

    *pIn >> nAnz;
    nBytesLeft -= 2;

    for( UINT16 nC = 0 ; nC < nAnz ; nC++ )
        {
        *pIn >> nRed >> nGreen >> nBlue >> nDummy;
        pExcGlob->pColorBuff->NewColor( nRed, nGreen, nBlue );
        nBytesLeft -= 4;
        }

    DBG_ASSERT( nBytesLeft >= 0,
        "*SwExcelParser::Palette(): Ups, das war zuviel!" );
    DBG_ASSERT( nBytesLeft <= 0,
        "*SwExcelParser::Palette(): Ups, da ha'mer jemanden vergessen!" );
    }

// ------------------------------------------------------------------ 0x99
void SwExcelParser::Standardwidth()
    {
    UINT16 nWidth;
    *pIn >> nWidth;
    nBytesLeft -= 2;
    aColRowBuff.SetDefWidth( ( UINT16 ) fExcToTwips * nWidth );
    }

// ------------------------------------------------------------------ 0xBD
void SwExcelParser::Mulrk()
    {
    UINT16 nRow, nColFirst, nXF;
    UINT32 nRkNum;

    *pIn >> nRow >> nColFirst;
    nBytesLeft -= 4;

    if( pExcGlob->IsInRowRange( nRow ) )
        {
        for( UINT16 nCol = nColFirst ; nBytesLeft > 2 ; nCol++ )
            {
            *pIn >> nXF >> nRkNum;
            nBytesLeft -= 6;

            if( pExcGlob->IsInColRange( nCol ) )
                {
                aColRowBuff.Used( nCol, nRow );
                aFltTab.SetXF( nCol, nRow, nXF );

                PutCell( nCol, nRow, RkToDouble( nRkNum ), nXF );
                }
            }
        DBG_ASSERT( nBytesLeft == 2,
            "+SwExcelParser::Mulrk(): Was'n das?!!!" );
        }
    }

// ------------------------------------------------------------------ 0xBE
void SwExcelParser::Mulblank()
    {
    UINT16 nRow, nCol, nColFirst, nXF;

    *pIn >> nRow >> nColFirst;
    nBytesLeft -= 4;

    if( pExcGlob->IsInRowRange( nRow ) )
        {
        for( nCol = nColFirst ; nBytesLeft > 2 ; nCol++ )
            {
            *pIn >> nXF;
            nBytesLeft -= 2;

            if( pExcGlob->IsInColRange( nCol ) )
                {
                aColRowBuff.Used( nCol, nRow );
                aFltTab.SetXF( nCol, nRow, nXF );
                }
            }
        }
    }

// ------------------------------------------------------------------ 0xD6
void SwExcelParser::Rstring()
    {
    UINT16 nRow, nCol, nXF;
    BYTE nCount;
    String aString;

    *pIn >> nRow >> nCol >> nXF;
    nBytesLeft -= 6;
    ReadExcString( LenWord, aString );
    *pIn >> nCount;
    nBytesLeft--;
    pIn->SeekRel( nCount * 2 );	// STRUN-Structures ueberspringen
    nBytesLeft -= nCount * 2;

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );

        PutCell( nCol, nRow, aString, nXF );
        }
    }

// ------------------------------------------------------------------ 0xE0
void SwExcelParser::XF5()
    {
    UINT16 nAttr0, nAlign, nIndexFormat, nIndexFont, nFillCol,
        nFill_Bottom, nBorder0, nBorder1;
    CellBorder aBorder;
    CellFill aFill;
    XF_Data *pXFD = new XF_Data;

    *pIn >> nIndexFont >> nIndexFormat >> nAttr0 >> nAlign >> nFillCol
        >> nFill_Bottom >> nBorder0 >> nBorder1;
    nBytesLeft -= 16;

    aBorder.nTopLine = ( BYTE ) nBorder0 & 0x0007;				// .............210
    aBorder.nTopColor = ( nBorder0 & 0xFE00 ) >> 9;				// 5432109.........
    aBorder.nLeftLine = ( BYTE ) ( ( nBorder0 & 0x0038 ) >> 3 );// ..........543...
    aBorder.nLeftColor = nBorder1 & 0x007F ;					// .........6543210
    aBorder.nBottomLine = ( BYTE ) ((nFill_Bottom & 0x01C0)>>6);// .......876......
    aBorder.nBottomColor = ( nFill_Bottom & 0xFE00 ) >> 9;		// 5432109.........
    aBorder.nRightLine = ( BYTE ) ( ( nBorder0 & 0x01C0 ) >> 6);//........876.....
    aBorder.nRightColor = ( nBorder1& 0x3F80 ) >> 7;			// ..3210987.......

    aFill.nPattern = ( BYTE ) nFill_Bottom & 0x003F;			// ..........543210
    aFill.nForeColor = nFillCol & 0x007F;						// .........6543210
    aFill.nBackColor = ( nFillCol & 0x1F80 ) >> 7;				// ...210987.......

    if( nAttr0 & 0x0004 )
        {// Style-XF
        pXFD->SetStyleXF();
        }
    else
        {// Cell-XF
        pXFD->SetCellXF();
        }

    pXFD->SetParent( ( nAttr0 & 0xFFF0 ) >> 4 );

    if( nAlign & 0x0400 )
        pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nIndexFormat ) );
    if( nAlign & 0x0800 )
        pXFD->SetFont( nIndexFont );
    if( nAlign & 0x1000 )
        pXFD->SetAlign( ( ExcHorizAlign ) ( nAlign & 0x0007 ) );
    if( nAlign & 0x2000 )
        pXFD->SetBorder( aBorder );
    if( nAlign & 0x4000 )
        pXFD->SetFill( aFill );

    pExcGlob->pXF_Buff->NewXF( pXFD );
    }

// ---------------------------------------------------------------- 0x0201
void SwExcelParser::Blank34()
    {
    UINT16 nRow, nCol, nXF;

    *pIn >> nRow >> nCol >> nXF;
    nBytesLeft -= 6;

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );
        }
    }

// ---------------------------------------------------------------- 0x0203
void SwExcelParser::Number34()
    {
    UINT16 nRow, nCol, nXF;
    double fValue;

    *pIn >> nRow >> nCol >> nXF >> fValue;
    nBytesLeft -= 14;

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );

        PutCell( nCol, nRow, fValue, nXF );
        }
    }

// ---------------------------------------------------------------- 0x0204
void SwExcelParser::Label34()
    {
    UINT16 nRow, nCol, nXF;

    *pIn >> nRow >> nCol >> nXF;
    nBytesLeft -= 6;

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );

        ReadExcString( LenWord );
        String aSvStr( pReadBuff, eZielChar );

        PutCell( nCol, nRow, aSvStr, nXF );
        }
    }

// ---------------------------------------------------------------- 0x0206
void SwExcelParser::Formula3()
    {
    UINT16 nRow, nCol, nXF, nFormLen;
    double fCurVal;
    BYTE nFlag0;

    *pIn >> nRow >> nCol >> nXF >> fCurVal >> nFlag0;
    pIn->SeekRel( 1 );
    *pIn >> nFormLen;
    nBytesLeft -= 18;

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );

        PutCell( nCol, nRow, fCurVal, nXF );
        }
    }

// ---------------------------------------------------------------- 0x0209
void SwExcelParser::Bof3()
    {
    //POST:	eDateiTyp = Biff3
    UINT16 nTyp;

    pIn->SeekRel( 2 );
    *pIn >> nTyp;
    nBytesLeft -= 4;

#ifdef USED
    ResetBof();
#endif

    if( nTyp == 0x0010 )		// Worksheet
        {
        //eAktZust = ImpZustTabBase;
        eDateiTyp = Biff3;
        }
    else if( nTyp == 0x0100 )	// Workbook
        {
        //eAktZust = ImpZustGlobals;
        eDateiTyp = Biff3W;
        }
    else
        {
        //eAktZust = ImpZustInit;	// -> ueberlesen von Nicht-Worksheets/-Workbooks
        eDateiTyp = BiffX;
        }
    }

// ---------------------------------------------------------------- 0x0218
#ifdef USED
void SwExcelParser::Name34()
    {
    BYTE nLenName;
    UINT16 nAttr, nLenExpr;
    UINT16 nT, nCS, nCE, nRS, nRE;
    String aExpr, aAnsiName;
    SpString aName;
    sal_Char cZ;

    *pIn >> nAttr;
    pIn->SeekRel( 1 );
    *pIn >> nLenName >> nLenExpr;
    nBytesLeft -= 6;

    // Namen einlesen
    for( UINT16 nLauf = 0 ; nLauf < nLenName ; nLauf++ )
        {
        *pIn >> cZ;
        aName += cZ;
        }
    nBytesLeft -= nLenName;

    if( aName.Len() == 1 && ( const sal_Char ) aName < 0x0D )
        {// Build-in name (0x00 - 0x0C als erstes Zeichen)
        aAnsiName = pBuildInName[ ( const sal_Char ) aName ];
        }
    else
        {
        aAnsiName = ( const sal_Char * ) aName;
        aAnsiName.Convert( eQuellChar, eZielChar );
        }

    if( GetRange( nLenExpr, nT, nCS, nCE, nRS, nRE ) )
        {
        aDataPool.InsertEntry( aAnsiName, EET_NamedRange,
            aTabNameBuff[ nT ], nT, nCS, nCE, nRS, nRE );
        }
    }

// ---------------------------------------------------------------- 0x0221
void SwExcelParser::Array34()
    {
    }

// ---------------------------------------------------------------- 0x0223
void SwExcelParser::Externname34()
    {
    }

// ---------------------------------------------------------------- 0x0225
void SwExcelParser::Defrowheight345()
    {
    }
#endif

// ---------------------------------------------------------------- 0x0231
void SwExcelParser::Font34()
    {
    UINT16 nHeight, nIndexCol;
    BYTE nAttr0;
    String aName;

    *pIn >> nHeight >> nAttr0;
    pIn->SeekRel( 1 );
    *pIn >> nIndexCol;
    nBytesLeft -= 6;

    ReadExcString( LenByte, aName );
    // Font in Pool batschen
    pExcGlob->pFontBuff->NewFont( nHeight, nAttr0, nIndexCol, aName );
    }

// ---------------------------------------------------------------- 0x0243
void SwExcelParser::XF3()
    {
    BYTE nDummyFormat, nDummyFont;
    UINT16 nAttr0, nAttr1, nIndexFormat, nIndexFont, nBorder, nFill;
    CellBorder aBorder;
    CellFill aFill;
    XF_Data *pXFD = new XF_Data;

    *pIn >> nDummyFont >> nDummyFormat >> nAttr0 >> nAttr1 >> nFill >> nBorder;

    aBorder.nTopLine = ( BYTE ) nBorder & 0x0007;				// .............210
    aBorder.nTopColor = ( nBorder & 0x00F8 ) >> 3;				// ........76543...
    aBorder.nLeftLine = ( BYTE ) ( ( nBorder & 0x0700 ) >> 8 );	// .....098........
    aBorder.nLeftColor = ( nBorder & 0xF800 ) >> 11;			// 54321...........
    *pIn >> nBorder;
    aBorder.nBottomLine = ( BYTE ) nBorder & 0x0007;			// .............210
    aBorder.nBottomColor = ( nBorder & 0x00F8 ) >> 3;			// ........76543...
    aBorder.nRightLine = ( BYTE ) ( ( nBorder & 0x0700 ) >> 8 );// .....098........
    aBorder.nRightColor = ( nBorder & 0xF800 ) >> 11;			// 54321...........
    nBytesLeft -= 12;

    nIndexFormat = nDummyFormat;
    nIndexFont = nDummyFont;
    aFill.nPattern = ( BYTE ) ( nFill & 0x003F );				// ..........543210
    aFill.nForeColor = ( nFill & 0x07C0 ) >> 6;					// .....09876......
    aFill.nBackColor = ( nFill & 0xF800 ) >> 11;				// 54321...........

    if( nAttr0 & 0x0400 )
        pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nIndexFormat ) );
    if( nAttr0 & 0x0004 )
        {// Style-XF
        pXFD->SetStyleXF();
        }
    else
        {// Cell-XF
        pXFD->SetCellXF();
        }

    pXFD->SetParent( ( nAttr1 & 0xFFF0 ) >> 4 );

    if( nAttr0 & 0x0800 )
        pXFD->SetFont( nIndexFont );
    if( nAttr0 & 0x1000 )
        pXFD->SetAlign( ( ExcHorizAlign ) ( nAttr1 & 0x0007 ) );
    if( nAttr0 & 0x2000 )
        pXFD->SetBorder( aBorder );
    if( nAttr0 & 0x4000 )
        pXFD->SetFill( aFill );

    pExcGlob->pXF_Buff->NewXF( pXFD );
    }

// ----------------------------------------------------- 0x7E <----- 0x27E

// ---------------------------------------------------------------- 0x0406
void SwExcelParser::Formula4()
    {
    UINT16 nRow, nCol, nXF, nFormLen;
    double fCurVal;
    BYTE nFlag0;

    *pIn >> nRow >> nCol >> nXF >> fCurVal >> nFlag0;
    pIn->SeekRel( 1 );
    *pIn >> nFormLen;
    nBytesLeft -= 18;

    if( pExcGlob->IsInRange( nCol, nRow ) )
        {
        aColRowBuff.Used( nCol, nRow );
        aFltTab.SetXF( nCol, nRow, nXF );

        PutCell( nCol, nRow, fCurVal, nXF );
        }
    }

// ---------------------------------------------------------------- 0x0409
void SwExcelParser::Bof4()
    {
    //POST:	eDateiTyp = Biff4
    UINT16 nTyp;

    pIn->SeekRel( 2 );
    *pIn >> nTyp;
    nBytesLeft -= 4;

#ifdef USED
    ResetBof();
#endif

    if( nTyp == 0x0010 )		// Worksheet
        {
        //eAktZust = ImpZustTabBase;
        eDateiTyp = Biff4;
        }
    else if( nTyp == 0x0100 )	// Workbook
        {
        //eAktZust = ImpZustGlobals;
        eDateiTyp = Biff4W;
        }
    else
        {
        //eAktZust = ImpZustInit;	// -> ueberlesen von Nicht-Worksheets/-Workbooks
        eDateiTyp = BiffX;
        }

    }

// ---------------------------------------------------------------- 0x041E
void SwExcelParser::Format4()
    {
    String aFormString;

    pIn->SeekRel( 2 );
    nBytesLeft -=2 ;

    ReadExcString( LenByte, aFormString );

    pValueFormBuffer->NewValueFormat( aFormString );
    }

// ---------------------------------------------------------------- 0x0443
void SwExcelParser::XF4()
    {
    BYTE nDummyFormat, nDummyFont;
    UINT16 nAttr0, nAttr1, nIndexFormat, nIndexFont, nBorder, nFill;
    CellBorder aBorder;
    CellFill aFill;
    XF_Data *pXFD = new XF_Data;

    *pIn >> nDummyFont >> nDummyFormat >> nAttr0 >> nAttr1 >> nFill >> nBorder;
    aBorder.nTopLine = ( BYTE ) nBorder & 0x0007;				// .............210
    aBorder.nTopColor = ( nBorder & 0x00F8 ) >> 3;				// ........76543...
    aBorder.nLeftLine = ( BYTE ) ( ( nBorder & 0x0700 ) >> 8 );	// .....098........
    aBorder.nLeftColor = ( nBorder & 0xF800 ) >> 11;			// 54321...........
    *pIn >> nBorder;
    aBorder.nBottomLine = ( BYTE ) nBorder & 0x0007;			// .............210
    aBorder.nBottomColor = ( nBorder & 0x00F8 ) >> 3;			// ........76543...
    aBorder.nRightLine = ( BYTE ) ( ( nBorder & 0x0700 ) >> 8 );// .....098........
    aBorder.nRightColor = ( nBorder & 0xF800 ) >> 11;			// 54321...........
    nBytesLeft -= 12;

    nIndexFormat = nDummyFormat;
    nIndexFont = nDummyFont;
    aFill.nPattern = ( BYTE ) nFill & 0x003F;					// ..........543210
    aFill.nForeColor = ( nFill & 0x07C0 ) >> 6;					// .....09876......
    aFill.nBackColor = ( nFill & 0xF800 ) >> 11;				// 54321...........

    if( nAttr1 & 0x0400 )
        pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nIndexFormat ) );
    if( nAttr0 & 0x0004 )
        {// Style-XF
        pXFD->SetStyleXF();
        }
    else
        {// Cell-XF
        pXFD->SetCellXF();
        }

    pXFD->SetParent( ( nAttr0 & 0xFFF0 ) >> 4 );

    if( nAttr1 & 0x0800 )
        pXFD->SetFont( nIndexFont );
    if( nAttr1 & 0x1000 )
        pXFD->SetAlign( ( ExcHorizAlign ) ( nAttr1 & 0x0007 ) );
    if( nAttr1 & 0x2000 )
        pXFD->SetBorder( aBorder );
    if( nAttr1 & 0x4000 )
        pXFD->SetFill( aFill );

    pExcGlob->pXF_Buff->NewXF( pXFD );
    }

// ---------------------------------------------------------------- 0x0809
void SwExcelParser::Bof5()
    {
    UINT16 nSubType, nVers;

    *pIn >> nVers >> nSubType;
    nBytesLeft -= 4;

#ifdef USED
    ResetBof();
#endif

    if( nSubType == 0x0010 )		// Worksheet
        {
        eDateiTyp = Biff5;
        }
    else if( nSubType == 0x0005 )	// Workbook Globals
        {
        //eAktZust = ImpZustGlobals;
        eDateiTyp = Biff5W;
        }
    else if( nSubType == 0x0100 )	// Workspace File
        {
        //eAktZust = ImpZustGlobals;	// !!!!!!!! ACHTUNG: verifizieren !!!!!!!!1
        eDateiTyp = Biff5W;
        }
    else
        {
        //eAktZust = ImpZustInit;
        eDateiTyp = BiffX;
        }
    }

//-------------------------------------------- SwExcelParser::RkToDouble -
double SwExcelParser::RkToDouble( const UINT32 &nRk )
    {
    //PLATTFORMABHAENGIG//
    double fVal;

    // jetzt kommt Code aus'm Excel-Developer's-Kit-Buch V5 (S.223)
    if( nRk & 0x02 )
        // Integer
        fVal = ( double ) ( *( ( INT32 * ) &nRk ) >> 2 );
    else
        {// 64-Bit IEEE-Float
#ifdef OSL_BIGENDIAN
//680xx und alle anderen vernuenftigen Prozessoren...
        *( ( UINT32 * ) &fVal + 1 ) = 0;	// unteren 32 Bits = 0
        *( ( UINT32 * ) &fVal ) = nRk & 0xFFFFFFFC;	// Bit 0, 1 = 0
#else
//Intel-Sch...
        *( ( UINT32 * ) &fVal ) = 0;	// unteren 32 Bits = 0
        *( ( UINT32 * ) &fVal + 1 ) = nRk & 0xFFFFFFFC;	// Bit 0, 1 = 0
#endif
        }

    if( nRk & 0x01 )
        fVal /= 100;

    return fVal;
    }

//----------------------------------------- SwExcelParser::ReadExcString -
String &SwExcelParser::ReadExcString( StringInfoLen eLen, String &aString )
    {
    //	POST:	ACHTUNG: Funktion setzt nBytesLeft selbstaendig!!!!!
    //						(geht auch nicht anders!)
    //			ACHTUNG: es wird an String angehaengt
    UINT16 nLaenge;
    BYTE nHelp;

    if( eLen == LenByte )
        {
        *pIn >> nHelp;
        nLaenge = nHelp;
        nBytesLeft--;
        }
    else
        {
        *pIn >> nLaenge;
        nBytesLeft -= 2;
        }

    nBytesLeft -= nLaenge;
    sal_Char* pStr = new sal_Char[ nLaenge + 2 ];
    pIn->Read( pStr, nLaenge );
    pStr[ nLaenge ] = 0;
    aString = String(pStr, eZielChar);
    delete[] pStr;

    return aString;
    }


#ifdef USED
//---------------------------------------------- SwExcelParser::ResetBof -
void SwExcelParser::ResetBof()
    {// setzt alle Einstellungen fuer neuen Tabellenbeginn zurueck
//	    pFontBuffer->Reset();
//		pXF_Buffer->Reset();
//		pColorBuffer->Reset();
//		aNameBuff.Reset();
//		aExtNameBuff.Reset();
//		aExtSheetBuff.Reset();
//		aColRowBuff.NewTab( nTab );
//		aFltTab.Reset( nTab );
    }
#endif


//---------------------------------------------- SwExcelParser::EndSheet -
void SwExcelParser::EndSheet()
    {// mach 'Gemarmel' am Ende eines Sheets
    aColRowBuff.Apply();
    aFltTab.Apply();
    }

//----------------------------------------- SwExcelParser::ReadExcString -
void SwExcelParser::ReadExcString( StringInfoLen eLen )
    {
    //	POST:	ACHTUNG: Funktion setzt nBytesLeft selbstaendig!!!!!
    //						(geht auch nicht anders!)
    //			Inhalt landet im pReadBuff
    UINT16 nLaenge;

    if( eLen == LenByte )
        {
        BYTE nHelp;
        *pIn >> nHelp;
        nLaenge = nHelp;
        nBytesLeft--;
        }
    else
        {
        *pIn >> nLaenge;
        nBytesLeft -= 2;
        }

    if( nLaenge >= nReadBuffSize )
        // genug Platz fuer String UND Nullbyte?
        nLaenge = nReadBuffSize - 1;

    pIn->Read( pReadBuff, nLaenge );	// String einlesen
    pReadBuff[ nLaenge ] = 0;		// ...und nullterminieren

    nBytesLeft -= nLaenge;
    }

//-------------------------------- SwExcelParser::PutCell( ..., String ) -
void SwExcelParser::PutCell( UINT16 nCol, UINT16 nRow, const String &rText,
    UINT16 nXF )
    {
    String	aAusgabeString;
    String	aDummyEingabe = rText;
    Color	*pDummyColor;

    DBG_ASSERT( pExcGlob->IsInColRange( nCol ),
        "-SwExcelParser::PutCell(): Col ist nicht im erlaubten Bereich!" );
    DBG_ASSERT( pExcGlob->IsInRowRange( nRow ),
        "-SwExcelParser::PutCell(): Row ist nicht im erlaubten Bereich!" );

    pExcGlob->Normalize( nCol, nRow );

    pExcGlob->pNumFormatter->GetOutputString(
        aDummyEingabe,
        pExcGlob->pXF_Buff->GetNumFormat( nXF ),
        aAusgabeString,
        &pDummyColor);
    pExcGlob->InsertText( nCol, nRow, aAusgabeString );
}

//-------------------------------- SwExcelParser::PutCell( ..., double ) -
void SwExcelParser::PutCell( UINT16 nCol, UINT16 nRow, double fVal,
    UINT16 nXF )
{
    String	aAusgabeString;
    UINT16	nUpperWord;
    BYTE	nType, nTypeValue;
    Color	*pDummyColor;

    DBG_ASSERT( pExcGlob->IsInColRange( nCol ),
        "-SwExcelParser::PutCell(): Col ist nicht im erlaubten Bereich!" );
    DBG_ASSERT( pExcGlob->IsInRowRange( nRow ),
        "-SwExcelParser::PutCell(): Row ist nicht im erlaubten Bereich!" );

    pExcGlob->Normalize( nCol, nRow );

#ifdef OSL_BIGENDIAN
//680xx und alle anderen vernuenftigen Prozessoren...
    nUpperWord = *( (UINT16*) &fVal );					// Intel-Word #3
    if( nUpperWord == 0xFFFF )
        { // Ergebnis keine Zahl!
        nType = *( ( (BYTE*) &fVal ) + 7 );				// Intel-Byte #0
        nTypeValue = *( ( (BYTE*) &fVal ) + 5 );		// Intel-Byte #2
        //}
#else
//Intel-Sch...
    nUpperWord = *( (UINT16*) &fVal + 3 );				// Intel-Word #3
    if( nUpperWord == 0xFFFF )
        { // Ergebnis keine Zahl!
        nType = *( (BYTE*) &fVal );						// Intel-Byte #0
        nTypeValue = *( ( (BYTE*) &fVal ) + 2 );		// Intel-Byte #2
        //}
#endif
        // ...gemeinsamer Code
        if( nType )
            {// Ergebnis KEIN String
            if( nType == 1 )
                {// Boolean
                if( nTypeValue )
                    aAusgabeString.AssignAscii("TRUE");
                else
                    aAusgabeString.AssignAscii("FALSE");
                }
            else if( nType == 2 )
                {// Boolerr
                switch( nTypeValue )
                    {
                    case 0x00:	aAusgabeString.AssignAscii("#NULL!"); break;
                    case 0x07:	aAusgabeString.AssignAscii("#DIV/0"); break;
                    case 0x0F:	aAusgabeString.AssignAscii("#VALUE!"); break;
                    case 0x17:	aAusgabeString.AssignAscii("#REF!"); break;
                    case 0x1D:	aAusgabeString.AssignAscii("#NAME?"); break;
                    case 0x24:	aAusgabeString.AssignAscii("#NUM!"); break;
                    case 0x2A:	aAusgabeString.AssignAscii("#N/A"); break;
                    default:	aAusgabeString.AssignAscii("#UNKNOWN");
                    }
                }
            else
                {
                aAusgabeString.AssignAscii("Unknown type of result");
                }
            }// Ende: Ergebnis KEIN String
        else
            {// String folgt Formula-Record
            //eAktZust = ImpZustString;
            bResultString = TRUE;
            nLastCol = nCol;
            nLastRow = nRow;
            nLastXF = nXF;
            }
// dieses ifdef ist nur fuer das segprag tool, wegen paariger Klammerung
#ifdef OSL_BIGENDIAN
    } // Ende: Ergebnis keine Zahl!
#else
    } // Ende: Ergebnis keine Zahl!
#endif
    else
        {// Ergebnis: 8-Byte IEEE-Number
        pExcGlob->pNumFormatter->GetOutputString(
            fVal,
            pExcGlob->pXF_Buff->GetNumFormat( nXF ),
            aAusgabeString,
            &pDummyColor);
        }

    if( !bResultString )
        pExcGlob->InsertText( nCol, nRow, aAusgabeString );

}// Ende: PutCell()

#ifdef USED

//-------------------------------------------- SwExcelParser::GetRange() -
BOOL SwExcelParser::GetRange( UINT16 nLenExpr, UINT16 &rTab, UINT16 &rCS,
    UINT16 &rCE, UINT16 &rRS, UINT16 &rRE )
    {
    // sucht in Formelausdruck nach einer Area-Refernce
    // return	= TRUE	-> gueltiger Bereich gefunden
    //			= FALSE	-> nichts gefunden
    BOOL bRet = TRUE;
    BYTE nOpcode;
    //INT32 nCount = nLenExpr;

    *pIn >> nOpcode;
    nBytesLeft--;

    switch( nOpcode )
        {
        case 0x3A:
        case 0x5A:
        case 0x7A:	// 3-D Cell-Reference
            {
            UINT16 nRowFirst, nRowLast, nTabFirst, nTabLast;
            INT16 nIndExt;
            BYTE nColFirst, nColLast;
            *pIn >> nIndExt;
            pIn->SeekRel( 8 );
            *pIn >> nTabFirst >> nTabLast >> nRowFirst >> nRowLast
                >> nColFirst >> nColLast;
            nBytesLeft -= 20;

            if( ( nRowFirst & 0xC0000 ) ||		// First relativ
                ( nRowLast & 0xC000 ) ||		// Last relativ
                ( nIndExt >= 0 ) )				// externe Reference
                bRet = FALSE;
            else
                {// alle Angaben OK... aber nur erste Tabelle wird genommen
                rTab = nTabFirst;
                rCS = nColFirst;
                rCE = nColLast;
                rRS = nRowFirst & 0xC000;
                rRE = nRowLast & 0xC000;
                }
            }
            break;
        default:
            bRet = FALSE;
        }

    return bRet;
    }

/*  */

// -----------------------------------------------------------------------
void SwExcelParser::NI()
    {
    }

// ------------------------------------------------------------------ 0x08
void SwExcelParser::Row25()
    {
    }

// ------------------------------------------------------------------ 0x17
void SwExcelParser::Externsheet()
    {
    }

// ------------------------------------------------------------------ 0x18

void SwExcelParser::Name25()
    {
    String aExpr;
    UINT16 nT, nCS, nCE, nRS, nRE;

    if( eDateiTyp == Biff2 )
        {// --------------------------------------------------- Biff2
        BYTE nAttr0, nAttr1, nLenName, nLenExpr;
        UINT16 nLauf;
        sal_Char cZ;
        SpString aName;			// Bereichsname
        String aAnsiName;

        *pIn >> nAttr0 >> nAttr1;
        pIn->SeekRel( 1 );
        *pIn >> nLenName >> nLenExpr;
        nBytesLeft -= 5;

        // Namen einlesen
        for( nLauf = 0 ; nLauf < nLenName ; nLauf++ )
            {
            *pIn >> cZ;
            aName += cZ;
            }
        nBytesLeft -= nLenName;

        aAnsiName = ( const sal_Char * ) aName;
        aAnsiName.Convert( eQuellChar, eZielChar );

        if( GetRange( nLenExpr, nT, nCS, nCE, nRS, nRE ) )
            {
            aDataPool.InsertEntry( aAnsiName, EET_NamedRange,
                aTabNameBuff[ nT ], nT, nCS, nCE, nRS, nRE );
            }


        pIn->SeekRel( 1 );	// cceDup ueberfluessig
        nBytesLeft--;
        }// ------------------------------------------Ende fuer Biff2
    else
        {// --------------------------------------------------- Biff5
        sal_Char cZ;
        BYTE nLenName, nLen;
        UINT16 nOpt, nLenDef, nLenSeekRel = 0, nLauf;
        SpString aName;		// Bereichsname
        String aAnsiName;

        *pIn >> nOpt;
        pIn->SeekRel( 1 );
        *pIn >> nLenName >> nLenDef;
        pIn->SeekRel( 4 );
        *pIn >> nLen;			// length of custom menu text
        nLenSeekRel += nLen;
        *pIn >> nLen;			// length of description text
        nLenSeekRel += nLen;
        *pIn >> nLen;			// length of help topic text
        nLenSeekRel += nLen;
        *pIn >> nLen;			// length of status bar text
        nLenSeekRel += nLen;
        nBytesLeft -= 14;

        // Namen einlesen
        for( nLauf = 0 ; nLauf < nLenName ; nLauf++ )
            {
            *pIn >> cZ;
            aName += cZ;
            }
        nBytesLeft -= nLenName;
        // jetzt steht Lesemarke an der Formel

        if( nOpt & 0x0020 )
            {// Build-in name
            aAnsiName = pBuildInName[ ( const sal_Char* ) aName ];
            }
        else
            {
            aAnsiName = ( const sal_Char * ) aName;
            aAnsiName.Convert( eQuellChar, eZielChar );
            }

        if( GetRange( nLenDef, nT, nCS, nCE, nRS, nRE ) )
            {
            aDataPool.InsertEntry( aAnsiName, EET_NamedRange,
                aTabNameBuff[ nT ], nT, nCS, nCE, nRS, nRE );
            }
        }// ----------------------------------------- Ende fuer Biff5
    }

// ------------------------------------------------------------------ 0x1C
void SwExcelParser::Note()
    {
    }

// ------------------------------------------------------------------ 0x1F
void SwExcelParser::Formatcount()
    {
    }

// ------------------------------------------------------------------ 0x20
void SwExcelParser::Columndefault()
    {// Default Cell Attributes
    }

// ------------------------------------------------------------------ 0x21
void SwExcelParser::Array25()
    {
    }

// ------------------------------------------------------------------ 0x23
void SwExcelParser::Externname25()
    {
    }
// ------------------------------------------------------------------ 0x25
void SwExcelParser::Defrowheight2()
    {
    }
// ------------------------------------------------------------------ 0x56
void SwExcelParser::Builtinfmtcnt()
    {
    }
// ------------------------------------------------------------------ 0x56
void SwExcelParser::Builtinfmtcnt()
    {
    }

// ------------------------------------------------------------------ 0x8C
void SwExcelParser::Country()
    {
    }

// ------------------------------------------------------------------ 0x8E
void SwExcelParser::Bundlesoffset()
    {
    }

// ------------------------------------------------------------------ 0x8F
void SwExcelParser::Bundleheader()
    {
    String aName;

    ReadExcString( LenByte, aName );

    aName.Convert( eQuellChar, eZielChar );
    aTabNameBuff << aName;

//	aExcDok.Set( aName, nTab );					####################################
    nTab++;
    }

// ------------------------------------------------------------------ 0xBC
void SwExcelParser::Shrfmla()
    {
    }

// -------------------------------------------------------- 0x0207 -> 0x07
// ---------------------------------------------------------------- 0x0208
void SwExcelParser::Row34()
    {
    }

#endif


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
