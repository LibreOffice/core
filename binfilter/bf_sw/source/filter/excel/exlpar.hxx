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
#ifndef _EXLPAR_HXX
#define _EXLPAR_HXX

#include "fltbase.hxx"

#include "exccrts.hxx"
// auto strip #include "excdocum.hxx"
class SvStream; 
class String; 
namespace binfilter {


// einige Forward-Deklaratioen

class SwPaM;
class SwDoc;

class ValueFormBuffer;


enum StringInfoLen {
    LenByte = 0,// 1 Byte lange Stringlaengeninfo
    LenWord	= 1	// 2 Byte 	"		"
    };

enum BiffTyp {
    Biff2 = 0,
    Biff3 = 2,
    Biff3W = 3,
    Biff4 = 4,
    Biff4W = 5,
    Biff5 = 8,
    Biff5W = 9,
    BiffX = -1 };


class SwExcelParser : public SwFilterBase
{
    int 			bNewDoc : 1;
    BiffTyp			eDateiTyp;	// aktuelles Tabellen-Format
    static const double fExcToTwips;
    static const sal_Char *pBuildInName[];

    CharSet			eQuellChar;	// Quell-Zeichensatz (interner Zeichensatz)
    CharSet			eZielChar;	// Ziel-Zeichensatz

    USHORT			nLastCol;
    USHORT			nLastRow;
    USHORT			nLastXF;

//	ULONG			nLastBof;	// Stream Position NACH letzten BOF

    ColRowSettings	aColRowBuff;		// Col/Row-Einstellungen 1 Tabelle
    FltTabelle		aFltTab;			// Attribute-Optimierung

#ifdef USED
    ExcDokument		aExcDok;			// merkt sich Tabellen-Parameter
    ExcDataPool		aDataPool;			// merkt sich benannte Bereiche,
                                        //	Tabellenname, ... fuer'n Dialog
#endif

    ValueFormBuffer	*pValueFormBuffer;	// Exc->Sv-Numberformats

    USHORT			nIxfeIndex;	// merkt sich Angabe im IXFE-Record

    BOOL			bResultString;

    // ---------------------------------------------------------------
//	void			NI();				// nicht implementierter Opcode
    void			Dimensions();		// 0x00
    void			Blank25();			// 0x01
    void			Number25();			// 0x03
    void			Label25();			// 0x04
    void			Formula25();		// 0x06
    void			REC_String();		// 0x07
//	void			Row25();			// 0x08
    void			Bof2();				// 0x09
    void			Eof();				// 0x0A
//	void			Externsheet();		// 0x17
//	void			Name25();			// 0x18
//	void			Note();				// 0x1C
    void			Format235();		// 0x1E
//	void			Formatcount();		// 0x1F
//	void			Columndefault();	// 0x20
//	void			Array25();			// 0x21
//	void			Externname25();		// 0x23
    void			Colwidth();			// 0x24
//	void			Defrowheight2();	// 0x25
    void			Font25();			// 0x31
    void			Codepage();			// 0x42
    void			XF2();				// 0x43
    void			Ixfe();				// 0x44
    void			DefColWidth();		// 0x55
//	void			Builtinfmtcnt();	// 0x56
    void			Colinfo();			// 0x7D
    void			Rk();				// 0x7E
    void			Boundsheet();		// 0x85
//	void			Country();			// 0x8C
//	void			Bundlesoffset();	// 0x8E
//	void			Bundleheader();		// 0x8F
    void			Palette();			// 0x92
    void			Standardwidth();	// 0x99
//	void			Shrfmla();			// 0xBC
    void			Mulrk();			// 0xBD
    void			Mulblank();			// 0xBE
    void			Rstring();			// 0xD6
    void			XF5();				// 0xE0
    //									// 0x0200 -> 0x00
    void			Blank34();			// 0x0201
    void			Number34();			// 0x0203
    void			Label34();			// 0x0204
    void			Formula3();			// 0x0206
//	void			Row34();			// 0x0208
    void			Bof3();				// 0x0209
//	void			Name34();			// 0x0218
    void			Array34();			// 0x0221
    void			Externname34();		// 0x0223
    void			Defrowheight345();	// 0225
    void			Font34();			// 0x0231
    void			XF3();				// 0x0243
    //void			Rk();				// 0x027E -> 0x7E
    void			Formula4();			// 0x0406
    void			Bof4();				// 0x0409
    void			Format4();			// 0x041E
    void			XF4();				// 0x0443
    void			Bof5();				// 0x0809
    // ---------------------------------------------------------------
    double			RkToDouble( const UINT32 &nRk );
    String			&ReadExcString( StringInfoLen eLen, String &aString );
#ifdef USED
    void			ResetBof();
#endif
    void			EndSheet();
    void			ReadExcString( StringInfoLen eLen );
    void			PutCell( USHORT nCol, USHORT nRow, const String &rText,
                        USHORT nXF );
    void			PutCell( USHORT nCol, USHORT nRow, double fVal,
                        USHORT nXF );
    void			Parse();
    BOOL			GetRange( USHORT nLenExpr, USHORT &rTab, USHORT &rCS,
                        USHORT &rCE, USHORT &rRS, USHORT &RE );
    // ---------------------------------------------------------------
    void			Init();
    void			ScanDokuInfos();	// Info-Sammler fuer Arbeitsmappen
    void			ScanBiff2();
    void			ScanBiff3();
    void			ScanBiff4();
    void			ScanBiff3W();
    void			ScanBiff4W();
    void			ScanBiff5W();
#ifdef USED
    void			ReadTab( const ExcUiEntry &rUiEntry, ULONG nStrPos );
#endif
public:
    // Parameter bReadNewDoc gibt an, ob in ein bestehendes Dokument ein-
    // gelesen wird. TRUE: in ein "neues" Dokument einfuegen.
    SwExcelParser( SwDoc& rDoc, const SwPaM & rCrsr, SvStream& pIn,
        int bReadNewDoc, CharSet eQ );
    ~SwExcelParser();

    ULONG CallParser();
};

} //namespace binfilter
#endif
    // _EXLPAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
