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

#ifdef DBG_UTIL
#include <stdio.h>
#endif
#include <tools/debug.hxx>

#include <fltglbls.hxx>
#include <exlpar.hxx>
#include <doc.hxx>
#include <docsh.hxx>

#include <mdiexp.hxx>			// ...Percent()
#include <statstr.hrc>			// ResId fuer Statusleiste
namespace binfilter {



// ---------------------------------------------- SwExcelParser::Parse() -
void SwExcelParser::Parse()
    {
    // Behelfsmaessig, nur zum Vorfuehren
    // HP-Compiler kommt mit Identifier String nicht klar,
    // ueberall zu ExcString geaendert.
    enum Zustand { ScanType, ScanDim, Initials, GlobInitials34,
    GlobInitials5, Eof5, Tab, Ende, ExcString };
    Zustand eAkt;

    USHORT	nOpcode;			// aktueller Opcode
    USHORT	nLaengeRec;			// Laenge aktueller Record
    ULONG	nLastTab = 0;		// Stream Position von letzter Tab
    ULONG	nLastPos = 0;		// letzte Stream-Pos. vor Record

    pExcGlob->eHauptDateiTyp = ERT_Biff5; // ...wenn nicht genauer bestimmt

    // Laufbalken anzeigen
    {
        pIn->Seek(STREAM_SEEK_TO_END);
        pIn->ResetError();
//       ::StartProgress( STR_STATSTR_W4WREAD, 0, pIn->Tell(),
//                           pExcGlob->pD->GetDocShell() );
        pIn->Seek(STREAM_SEEK_TO_BEGIN);
        pIn->ResetError();
    }

    eAkt = ScanType;
    while( eAkt != Ende )
        {
        nLastPos = pIn->Tell();
//      if( eAkt != ScanDim )
//          ::binfilter::SetProgressState( pIn->Tell(), pExcGlob->pD->GetDocShell() );

        *pIn >> nOpcode >> nLaengeRec;
        nBytesLeft = nLaengeRec;
        if( pIn->IsEof() ) eAkt = Ende;

        // Automat: Init -> [ Global ] -> TabBase -> Tab -> ...
        switch( eAkt )
            {
            // --------------------------------- ScanType ----------------
            case ScanType:
                {
                switch( nOpcode )
                    {
                    case 0x09:							// BOF			[ 2   ]
                        Bof2();
                        if( eDateiTyp == Biff2 )
                            {
                            nLastTab = nLastPos;
                            eAkt = ScanDim;
                            pExcGlob->eHauptDateiTyp = ERT_Biff2;
                            }
                        else
                            eAkt = Ende;
                        break;
                    case 0x0209:						// BOF			[  3  ]
                        Bof3();
                        if( eDateiTyp == Biff3 )
                            {
                            nLastTab = nLastPos;
                            eAkt = ScanDim;
                            pExcGlob->eHauptDateiTyp = ERT_Biff3;
                            }
                        else if( eDateiTyp == Biff3W )
                            {
                            eAkt = GlobInitials34;
                            pExcGlob->eHauptDateiTyp = ERT_Biff3;
                            }
                        else
                            eAkt = Ende;
                        break;
                    case 0x0409:						// BOF			[   4 ]
                        Bof4();
                        if( eDateiTyp == Biff4 )
                            {
                            nLastTab = nLastPos;
                            eAkt = ScanDim;
                            pExcGlob->eHauptDateiTyp = ERT_Biff4;
                            }
                        else if( eDateiTyp == Biff4W )
                            {
                            eAkt = GlobInitials34;
                            pExcGlob->eHauptDateiTyp = ERT_Biff4;
                            }
                        else
                            eAkt = Ende;
                        break;
                    case 0x0809:						// BOF			[    5]
                        Bof5();
                        if( eDateiTyp == Biff5W )
                            {
                            eAkt = GlobInitials5;
                            pExcGlob->eHauptDateiTyp = ERT_Biff5;
                            }
                        else
                            eAkt = Ende;
                        break;
                    case 0x0A:							// EOF			[ 2345]
                        Eof();
                        eAkt = Ende;
                        break;

                    }
                }
                break;
            // --------------------------------- ScanDim -----------------
            case ScanDim:
                {
                switch( nOpcode )
                    {
                    case 0x0A:							// EOF			[ 2345]
                        eAkt = Ende;	// keine Dimension keine Kekse
                        break;
                    case 0x00:							// DIMENSIONS	[ 2  5]
                    case 0x0200:						// DIMENSIONS	[  34 ]
                        Dimensions();
                        eAkt = Initials;
                        pIn->Seek( nLastTab );
                        nBytesLeft = 0;
                        break;
                    }
                }
                break;
            // --------------------------------- Initials ----------------
            case Initials:
                {
                switch( nOpcode )
                    {
                    case 0x01:
                        pExcGlob->CreateTable();
                        Blank25();
                        eAkt = Tab;
                        break;							// BLANK		[ 2  5]
                    case 0x03:
                        pExcGlob->CreateTable();
                        Number25();
                        eAkt = Tab;
                        break;							// NUMBER		[ 2  5]
                    case 0x04:
                        pExcGlob->CreateTable();
                        Label25();
                        eAkt = Tab;
                        break;							// LABEL		[ 2  5]
                    case 0x06:
                        pExcGlob->CreateTable();
                        Formula25();
                        eAkt = Tab;
                        break;							// FORMULA		[ 2  5]
                    case 0x0A:
                        Eof();
                        eAkt = Ende;
                        break;							// EOF		[ 2345]
                    case 0x1E:	Format235(); break;		// FORMAT		[ 23 5]
//					case 0x20:	Columndefault(); break;	// COLUMNDEFAULT[ 2   ]
                    case 0x24:	Colwidth(); break;		// COLWIDTH		[ 2   ]
                    case 0x31:	Font25(); break;		// FONT			[ 2  5]
                    case 0x42:	Codepage(); break;		// CODEPAGE		[ 2345]
                    case 0x43:	XF2(); break;			// XF			[ 2   ]
                    case 0x44:	Ixfe(); break;			// IXFE			[ 2   ]
                    case 0x55:	DefColWidth(); break;	// DEFCOLWIDTH	[ 2345]
                    case 0x7D:	Colinfo(); break;		// COLINFO		[  345]
                    case 0x7E:	Rk(); eAkt = Tab; break;// RK			[    5]
//					case 0x8C:	Country(); break;		// COUNTRY		[  345]
                    case 0x92:	Palette(); break;		// PALETTE		[  345]
                    case 0x99:	Standardwidth(); break;	// STANDARDWIDTH[    5]
                    case 0xBD:
                        pExcGlob->CreateTable();
                        Mulrk();
                        eAkt = Tab;
                        break;							// MULRK		[    5]
                    case 0xBE:
                        pExcGlob->CreateTable();
                        Mulblank();
                        eAkt = Tab;
                        break;							// MULBLANK		[    5]
                    case 0xD6:
                        pExcGlob->CreateTable();
                        Rstring();
                        eAkt = Tab;
                        break;							// RSTRING		[    5]
                    case 0xE0:	XF5(); break;			// XF			[    5]
                    case 0x0201:
                        pExcGlob->CreateTable();
                        Blank34();
                        eAkt = Tab;
                        break;							// BLANK		[  34 ]
                    case 0x0203:
                        pExcGlob->CreateTable();
                        Number34();
                        eAkt = Tab;
                        break;							// NUMBER		[  34 ]
                    case 0x0204:
                        pExcGlob->CreateTable();
                        Label34();
                        eAkt = Tab;
                        break;							// LABEL		[  34 ]
                    case 0x0206:
                        pExcGlob->CreateTable();
                        Formula3();
                        eAkt = Tab;
                        break;							// FORMULA		[  3  ]
                    case 0x0231: Font34(); break;		// FONT			[  34 ]
                    case 0x0243: XF3(); break;			// XF			[  3  ]
                    case 0x027E:
                        pExcGlob->CreateTable();
                        Rk();
                        eAkt = Tab;
                        break;							// RK			[  34 ]
                    case 0x0406:
                        pExcGlob->CreateTable();
                        Formula4();
                        eAkt = Tab;
                        break;							// FORMULA		[   4 ]
                    case 0x041E: Format4(); break;		// FORMAT		[   4 ]
                    case 0x0443: XF4(); break;			// XF			[   4 ]
                    }
                }
                break;
            // --------------------------------- GlobInitials34 ----------
            case GlobInitials34:
                {
                switch( nOpcode )
                    {
                    case 0x09:							// BOF			[ 2   ]
                        Bof2();
                        if( eDateiTyp == Biff2 )
                            {
                            nLastTab = nLastPos;
                            eAkt = ScanDim;
                            }
                        else
                            eAkt = Ende;
                        break;
                    case 0x0A:							// EOF			[ 2345]
                        Eof();
                        eAkt = Ende;
                        break;
                    case 0x1E:	Format235(); break;		// FORMAT		[ 23 5]
//					case 0x20:	Columndefault(); break;	// COLUMNDEFAULT[ 2   ]
                    case 0x24:	Colwidth(); break;		// COLWIDTH		[ 2   ]
                    case 0x31:	Font25(); break;		// FONT			[ 2  5]
                    case 0x42:	Codepage(); break;		// CODEPAGE		[ 2345]
                    case 0x43:	XF2(); break;			// XF			[ 2   ]
                    case 0x44:	Ixfe(); break;			// IXFE			[ 2   ]
                    case 0x55:	DefColWidth(); break;	// DEFCOLWIDTH	[ 2345]
                    case 0x7D:	Colinfo(); break;		// COLINFO		[  345]
//					case 0x8C:	Country(); break;		// COUNTRY		[  345]
                    case 0x92:	Palette(); break;		// PALETTE		[  345]
                    case 0x99:	Standardwidth(); break;	// STANDARDWIDTH[    5]
                    case 0x0209:						// BOF			[  3  ]
                        Bof3();
                        if( eDateiTyp == Biff3 )
                            {
                            nLastTab = nLastPos;
                            eAkt = ScanDim;
                            }
                        else
                            eAkt = Ende;
                        break;
                    case 0x0231: Font34(); break;		// FONT			[  34 ]
                    case 0x0243: XF3(); break;			// XF			[  3  ]
                    case 0x041E: Format4(); break;		// FORMAT		[   4 ]
                    case 0x0409: 						// BOF			[   4 ]
                        Bof4();
                        if( eDateiTyp == Biff4 )
                            {
                            nLastTab = nLastPos;
                            eAkt = ScanDim;
                            }
                        else
                            eAkt = Ende;
                        break;
                    case 0x0443: XF4(); break;			// XF			[   4 ]
                    }
                }
                break;
            // --------------------------------- GlobInitials5 -----------
            case GlobInitials5:
                {
                switch( nOpcode )
                    {
                    case 0x0A:							// EOF			[ 2345]
                        Eof();
                        eAkt = Eof5;
                        break;
                    case 0x1E:	Format235(); break;		// FORMAT		[ 23 5]
                    case 0x31:	Font25(); break;		// FONT			[ 2  5]
                    case 0x42:	Codepage(); break;		// CODEPAGE		[ 2345]
                    case 0x55:	DefColWidth(); break;	// DEFCOLWIDTH	[ 2345]
                    case 0x7D:	Colinfo(); break;		// COLINFO		[  345]
//					case 0x8C:	Country(); break;		// COUNTRY		[  345]
                    case 0x92:	Palette(); break;		// PALETTE		[  345]
                    case 0x99:	Standardwidth(); break;	// STANDARDWIDTH[    5]
                    case 0xE0:	XF5(); break;			// XF			[    5]
                    case 0x041E: Format4(); break;		// FORMAT		[   4 ]
                    }
                }
                break;
            // --------------------------------- Eof5 --------------------
            case Eof5:
                {
                switch( nOpcode )
                    {
                    case 0x0809:						// BOF			[    5]
                        Bof5();
                        if( eDateiTyp == Biff5 )
                            {
                            nLastTab = nLastPos;
                            eAkt = ScanDim;
                            }
                        break;
                    }
                }
                break;
            // --------------------------------- Tab ---------------------
            case Tab:
                {
                switch( nOpcode )
                    {
                    case 0x01:
                        Blank25(); break;				// BLANK		[ 2  5]
                    case 0x03:	Number25(); break;		// NUMBER		[ 2  5]
                    case 0x04:
                        Label25(); break;				// LABEL		[ 2  5]
                    case 0x06:							// FORMULA		[ 2  5]
                        Formula25();
                        if( bResultString ) eAkt = ExcString;
                        break;
                    case 0x0A:							// EOF			[ 2345]
                        Eof();
                        EndSheet();
                        eAkt = Ende;
                        break;
                    case 0x7E:	Rk(); break;			// RK			[    5]
                    case 0xBD:	Mulrk(); break;			// MULRK		[    5]
                    case 0xBE:	Mulblank(); break;		// MULBLANK		[    5]
                    case 0xD6:	Rstring(); break;		// RSTRING		[    5]
                    case 0x0201: Blank34(); break;		// BLANK		[  34 ]
                    case 0x0203: Number34(); break;		// NUMBER		[  34 ]
                    case 0x0204: Label34(); break;		// LABEL		[  34 ]
                    case 0x0206:						// FORMULA		[  3  ]
                        Formula3();
                        if( bResultString ) eAkt = ExcString;
                        break;
                    case 0x027E: Rk(); break;			// RK			[  34 ]
                    case 0x0406:						// FORMULA		[   4 ]
                        Formula4();
                        if( bResultString ) eAkt = ExcString;
                        break;
                    }
                }
                break;
            // ----------------------- Record String folgt ---------------
            case ExcString:
                {
                DBG_ASSERT( nOpcode == 0x07 || nOpcode == 0x0207,
                "+SwExcelParser::Parse(): Formatfehler: Formula ohne String" );

                if( nOpcode == 0x07 || nOpcode == 0x0207 )
                    REC_String();
                bResultString = FALSE;
                eAkt = Tab;
                }
                break;
            // -----------------------------------------------------------
            default:
                DBG_ERROR(
                "*SwExcelParser::Parse(): Zustand undefiniert - Trottel!" );
            }
        pIn->SeekRel( nBytesLeft );		// nicht gelesene Bytes ueberlesen
        }

    // Laufbalken wieder abschalten
//  ::EndProgress( pExcGlob->pD->GetDocShell() );

    }
/*
// ---------------------------------------------- SwExcelParser::Parse() -
void SwExcelParser::Parse()
    {
    ExcTabelle *pTab;
    USHORT	nOpcode;			// aktueller Opcode
    USHORT	nLaengeRec;			// Laenge aktueller Record
    ULONG	nLastBof = 0;		// Stream Position vom letzten BOF

//	ScanDokuInfos();
    // jetzt ist aDataPool mit Daten zum importieren gefuellt


    // BOF -> EOF => eine Tabelle
    pTab = aExcDok.GetFirstTab();
    while( pTab )
        {
        while( !pIn->IsEof() && eAktZust != ImpZustEnd )
            {
            *pIn >> nOpcode >> nLaengeRec;
            nBytesLeft = nLaengeRec;

            // Automat: Init -> [ Global ] -> TabBase -> Tab -> ...
            switch( eAktZust )
                {
                // ----------------------- Tabelleninhalt --------------------
                case ImpZustTab:
                    {
                    switch( nOpcode )
                        {
                        case 0x01:	Blank25(); break;		// BLANK		[ 2  5]
                        case 0x03:	Number25(); break;		// NUMBER		[ 2  5]
                        case 0x04:	Label25(); break;		// LABEL		[ 2  5]
                        case 0x06:	Formula25(); break;		// FORMULA		[ 2  5]
                        case 0x08:	Row25(); break;			// ROW			[ 2  5]
                        case 0x0A:							// EOF			[ 2345]
                            Eof();
                            EndSheet();
                            eAktZust = ImpZustEnd;
                            break;
                        case 0x1C:	Note(); break;			// NOTE			[ 2345]
                        case 0x21:	Array25(); break;		// ARRAY		[ 2  5]
                        case 0x7E:	Rk(); break;			// RK			[    5]
                        case 0xBD:	Mulrk(); break;			// MULRK		[    5]
                        case 0xBE:	Mulblank(); break;		// MULBLANK		[    5]
                        case 0xD6:	Rstring(); break;		// RSTRING		[    5]
                        case 0x0201: Blank34(); break;		// BLANK		[  34 ]
                        case 0x0203: Number34(); break;		// NUMBER		[  34 ]
                        case 0x0204: Label34(); break;		// LABEL		[  34 ]
                        case 0x0206: Formula3(); break;		// FORMULA		[  3  ]
                        case 0x0208: Row34(); break;		// ROW			[  34 ]
                        case 0x0221: Array34(); break;		// ARRAY		[  34 ]
                        case 0x027E: Rk(); break;			// RK			[  34 ]
                        case 0x0406: Formula4(); break;		// FORMULA		[   4 ]
                        case 0x04BC: Shrfmla(); break;		// SHRFMLA		[    5]
                        }
                    }
                    break;
                // ----------------------- Endzustand (nutzbar?) -------------
                case ImpZustEnd:
                    {
                    //switch( nOpcode )
                        {
                        }
                    }
                    break;
                // ----------------------- Record String folgt ---------------
                case ImpZustString:
                    {
                    DBG_ASSERT( nOpcode == 0x07 || nOpcode == 0x0207,
                    "+SwExcelParser::Read(): Formatfehler: Formula ohne String" );

                    if( nOpcode == 0x07 || nOpcode == 0x0207 )
                        REC_String();

                    eAktZust = ImpZustTab;
                    }
                    break;
                // ----------------------- Tabelleninhalt Init-Phase ---------
                // -----------------------------------------------------------
                default:
                    DBG_ERROR(
                    "*SwExcelParser::Read(): Zustand undefiniert - Trottel!" );
                }
            pIn->SeekRel( nBytesLeft );		// nicht gelesene Bytes ueberlesen
            }
        pTab = aExcDok.GetNextTab();
        }

    }
*/



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
