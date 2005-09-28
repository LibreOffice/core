/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: read.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 11:44:16 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>

#include "document.hxx"
#ifndef _SCERRORS_HXX
#include "scerrors.hxx"
#endif

#ifndef SC_FPROGRESSBAR_HXX
#include "fprogressbar.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XIPAGE_HXX
#include "xipage.hxx"
#endif
#ifndef SC_XIVIEW_HXX
#include "xiview.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XINAME_HXX
#include "xiname.hxx"
#endif
#ifndef SC_XICONTENT_HXX
#include "xicontent.hxx"
#endif
#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif
#ifndef SC_XIPIVOT_HXX
#include "xipivot.hxx"
#endif

#ifndef SC_XCLIMPCHANGETRACK_HXX
#include "XclImpChangeTrack.hxx"
#endif

#include "root.hxx"
#include "biffdump.hxx"
#include "imp_op.hxx"
#include "excimp8.hxx"

FltError ImportExcel::Read( void )
{
#if EXC_INCL_DUMPER
    {
        Biff8RecDumper aDumper( GetRoot(), FALSE );
        if( aDumper.Dump( aIn ) )
            return eERR_OK;
    }
#endif

    XclImpPageSettings&     rPageSett       = GetPageSettings();
    XclImpTabViewSettings&  rTabViewSett    = GetTabViewSettings();
    XclImpPalette&          rPal            = GetPalette();
    XclImpFontBuffer&       rFontBfr        = GetFontBuffer();
    XclImpNumFmtBuffer&     rNumFmtBfr      = GetNumFmtBuffer();
    XclImpXFBuffer&         rXFBfr          = GetXFBuffer();
    XclImpTabInfo&          rTabInfo        = GetTabInfo();
    XclImpNameManager&      rNameMgr        = GetNameManager();

    const BOOL  bWithDrawLayer = pD->GetDrawLayer() != NULL;

    enum Zustand {
        Z_BiffNull, // Nicht in gueltigem Biff-Format
        Z_Biff2,    // Biff2: nur eine Tabelle
        Z_Biff2C,   // Biff2: Chart

        Z_Biff3,    // Biff3: nur eine Tabelle
        Z_Biff3C,   // Biff3: Chart

        Z_Biff4,    // Biff4: nur eine Tabelle
        Z_Biff4W,   // Biff4 Workbook: Globals
        Z_Biff4I,   // Biff4 Workbook: Initiales fuer eine Tabelle
        Z_Biff4T,   // Biff4 Workbook: eine Tabelle selbst
        Z_Biff4E,   // Biff4 Workbook: zwischen den Tabellen
        Z_Biff4C,   // Biff4: Chart

        Z_Biff5WPre,// Biff5: Prefetch Workbook
        Z_Biff5W,   // Biff5: Globals
        Z_Biff5I,   // Biff5: Initiales fuer eine Tabelle
        Z_Biff5T,   // Biff5: eine Tabelle selbst
        Z_Biff5E,   // Biff5: zwischen den Tabellen
        Z_Biff5Pre, // Biff5: Prefetch fuer Shrfmla/Array Formula
        Z_Biff5C,   // Biff5: Chart
        Z_Biffn0,   // Alle Biffs: Tabelle bis naechstesss EOF ueberlesen
        Z_Ende };

    Zustand             eAkt = Z_BiffNull, ePrev = Z_BiffNull;

    FltError            eLastErr = eERR_OK;
    UINT16              nOpcode;
    UINT16              nBofLevel = 0;
    BOOL                bBiff4Workbook = FALSE;

    DBG_ASSERT( &aIn != NULL, "-ImportExcel::Read(): Kein Stream - wie dass?!" );

    ::std::auto_ptr< ScfSimpleProgressBar > pProgress( new ScfSimpleProgressBar(
        aIn.GetSvStreamSize(), GetDocShell(), STR_LOAD_DOC ) );

    while( eAkt != Z_Ende )
    {
        aIn.StartNextRecord();
        nOpcode = aIn.GetRecId();

        if( !aIn.IsValid() )
        {
            eAkt = Z_Ende;
            break;
        }

        if( eAkt != Z_Biff5Pre && eAkt != Z_Biff5WPre )
            pProgress->ProgressAbs( aIn.GetSvStreamPos() );

        switch( eAkt )
        {
            // ----------------------------------------------------------------
            case Z_BiffNull:    // ------------------------------- Z_BiffNull -
            {
                switch( nOpcode )
                {
                    case EXC_ID2_BOF:
                    case EXC_ID3_BOF:
                    case EXC_ID4_BOF:
                    case EXC_ID5_BOF:
                    {
                        // #i23425# don't rely on the record ID, but on the detected BIFF version
                        switch( GetBiff() )
                        {
                            case EXC_BIFF2:
                                Bof2();
                                if( pExcRoot->eDateiTyp == Biff2 )
                                {
                                    eAkt = Z_Biff2;
                                    NeueTabelle();
                                }
                            break;
                            case EXC_BIFF3:
                                Bof3();
                                if( pExcRoot->eDateiTyp == Biff3 )
                                {
                                    eAkt = Z_Biff3;
                                    NeueTabelle();
                                }
                            break;
                            case EXC_BIFF4:
                                Bof4();
                                if( pExcRoot->eDateiTyp == Biff4 )
                                {
                                    eAkt = Z_Biff4;
                                    NeueTabelle();
                                }
                                else if( pExcRoot->eDateiTyp == Biff4W )
                                {
                                    eAkt = Z_Biff4W;
                                    bBiff4Workbook = TRUE;
                                }
                            break;
                            case EXC_BIFF5:
                                Bof5();
                                if( pExcRoot->eDateiTyp == Biff5W )
                                {
                                    eAkt = Z_Biff5WPre;

                                    nBdshtTab = 0;

                                    aIn.StoreGlobalPosition(); // und Position merken
                                }

                                DBG_ASSERT( pExcRoot->eDateiTyp != Biff5,
                                    "+ImportExcel::Read(): Tabelle ohne Workbook!" );
                            break;
                            default:
                                DBG_ERROR_BIFF();
                        }
                    }
                    break;
                }
            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff2:       // ---------------------------------- Z_Biff2 -
            {
                switch( nOpcode )
                {
                    case 0x00:  Dimensions(); break;    // DIMENSIONS   [ 2345]
                    case 0x01:  Blank25(); break;       // BLANK        [ 2  5]
                    case 0x02:  Integer(); break;       // INTEGER      [ 2   ]
                    case 0x03:  Number25(); break;      // NUMBER       [ 2  5]
                    case 0x04:  Label(); break;         // LABEL        [ 2  5]
                    case 0x05:  Boolerr25(); break;     // BOOLERR      [ 2  5]
                    case 0x06:  Formula25(); break;     // FORMULA      [ 2  5]
                    case 0x07:  RecString(); break;     // STRING       [ 2345]
                    case 0x08:  Row25(); break;         // ROW          [ 2  5]
                    case 0x0A:                          // EOF          [ 2345]
                        rNumFmtBfr.CreateScFormats();
                        EndSheet();
                        eAkt = Z_Ende;
                        break;
                    case 0x14:
                    case 0x15:  rPageSett.ReadHeaderFooter( maStrm );   break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x18:  rNameMgr.ReadName( maStrm );            break;
                    case 0x1C:  Note(); break;          // NOTE         [ 2345]
                    case 0x1D:  rTabViewSett.ReadSelection( maStrm );   break;
                    case 0x1E:  rNumFmtBfr.ReadFormat( maStrm );        break;
                    case 0x20:  Columndefault(); break; // COLUMNDEFAULT[ 2   ]
                    case 0x21:  Array25(); break;       // ARRAY        [ 2  5]
                    case 0x23:  Externname25(); break;  // EXTERNNAME   [ 2  5]
                    case 0x24:  Colwidth(); break;      // COLWIDTH     [ 2   ]
                    case 0x25:  Defrowheight2(); break; // DEFAULTROWHEI[ 2   ]
                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:  rPageSett.ReadMargin( maStrm );         break;
                    case 0x2A:  rPageSett.ReadPrintHeaders( maStrm );   break;
                    case 0x2B:  rPageSett.ReadPrintGridLines( maStrm ); break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        eLastErr = XclImpDecryptHelper::ReadFilepass( maStrm );
                        if( eLastErr != ERRCODE_NONE )
                            eAkt = Z_Ende;
                        break;
                    case EXC_ID2_FONT:  rFontBfr.ReadFont( maStrm );    break;
                    case EXC_ID_EFONT:  rFontBfr.ReadEfont( maStrm );   break;
                    case 0x41:  rTabViewSett.ReadPane( maStrm );        break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x43:  rXFBfr.ReadXF( maStrm );                break;
                    case 0x44:  Ixfe(); break;          // IXFE         [ 2   ]
                    case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345]
                }
            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff3:       // ---------------------------------- Z_Biff3 -
            {
                switch( nOpcode )
                {
                    case 0x00:  Dimensions(); break;    // DIMENSIONS   [ 2345]
                    case 0x0A:                          // EOF          [ 2345]
                        rNumFmtBfr.CreateScFormats();
                        EndSheet();
                        eAkt = Z_Ende;
                        break;
                    case 0x14:
                    case 0x15:  rPageSett.ReadHeaderFooter( maStrm );   break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x1A:
                    case 0x1B:  rPageSett.ReadPageBreaks( maStrm );     break;
                    case 0x1C:  Note(); break;          // NOTE         [ 2345]
                    case 0x1D:  rTabViewSett.ReadSelection( maStrm );   break;
                    case 0x1E:  rNumFmtBfr.ReadFormat( maStrm );        break;
                    case 0x22:  Rec1904(); break;       // 1904         [ 2345]
                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:  rPageSett.ReadMargin( maStrm );         break;
                    case 0x2A:  rPageSett.ReadPrintHeaders( maStrm );   break;
                    case 0x2B:  rPageSett.ReadPrintGridLines( maStrm ); break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        eLastErr = XclImpDecryptHelper::ReadFilepass( maStrm );
                        if( eLastErr != ERRCODE_NONE )
                            eAkt = Z_Ende;
                        break;
                    case 0x41:  rTabViewSett.ReadPane( maStrm );        break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x7D:  Colinfo(); break;       // COLINFO      [  345]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    case 0x92:  rPal.ReadPalette( maStrm );             break;
                    case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345]
                    case 0x0201: Blank34(); break;      // BLANK        [  34 ]
                    case 0x0203: Number34(); break;     // NUMBER       [  34 ]
                    case 0x0204: Label(); break;        // LABEL        [  34 ]
                    case 0x0205: Boolerr34(); break;    // BOOLERR      [  34 ]
                    case 0x0206: Formula3(); break;     // FORMULA      [  3  ]
                    case 0x0207: RecString(); break;    // STRING       [ 2345]
                    case 0x0208: Row34(); break;        // ROW          [  34 ]
                    case 0x0218: rNameMgr.ReadName( maStrm );           break;
                    case 0x0221: Array34(); break;      // ARRAY        [  34 ]
                    case 0x0223: Externname34(); break; // EXTERNNAME   [  34 ]
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x0231: rFontBfr.ReadFont( maStrm );           break;
                    case 0x023E: rTabViewSett.ReadWindow2( maStrm );    break;
                    case 0x0243: rXFBfr.ReadXF( maStrm );               break;
                    case 0x0293: rXFBfr.ReadStyle( maStrm );            break;
                    case 0x027E: Rk(); break;           // RK           [  34 ]
                }
            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff4:       // ---------------------------------- Z_Biff4 -
            {
                switch( nOpcode )
                {
                    case 0x00:  Dimensions(); break;    // DIMENSIONS   [ 2345]
                    case 0x0A:                          // EOF          [ 2345]
                        rNumFmtBfr.CreateScFormats();
                        EndSheet();
                        eAkt = Z_Ende;
                        break;
                    case 0x12:  Protect(); break;       // SHEET PROTECTION
                    case 0x14:
                    case 0x15:  rPageSett.ReadHeaderFooter( maStrm );   break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x1A:
                    case 0x1B:  rPageSett.ReadPageBreaks( maStrm );     break;
                    case 0x1C:  Note(); break;          // NOTE         [ 2345]
                    case 0x1D:  rTabViewSett.ReadSelection( maStrm );   break;
                    case 0x22:  Rec1904(); break;       // 1904         [ 2345]
                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:  rPageSett.ReadMargin( maStrm );         break;
                    case 0x2A:  rPageSett.ReadPrintHeaders( maStrm );   break;
                    case 0x2B:  rPageSett.ReadPrintGridLines( maStrm ); break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        eLastErr = XclImpDecryptHelper::ReadFilepass( maStrm );
                        if( eLastErr != ERRCODE_NONE )
                            eAkt = Z_Ende;
                        break;
                    case 0x41:  rTabViewSett.ReadPane( maStrm );        break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x7D:  Colinfo(); break;       // COLINFO      [  345]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    case 0x92:  rPal.ReadPalette( maStrm );             break;
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                    case 0xA1:  rPageSett.ReadSetup( maStrm );          break;
                    case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345]
                    case 0x0201: Blank34(); break;      // BLANK        [  34 ]
                    case 0x0203: Number34(); break;     // NUMBER       [  34 ]
                    case 0x0204: Label(); break;        // LABEL        [  34 ]
                    case 0x0205: Boolerr34(); break;    // BOOLERR      [  34 ]
                    case 0x0207: RecString(); break;    // STRING       [ 2345]
                    case 0x0208: Row34(); break;        // ROW          [  34 ]
                    case 0x0218: rNameMgr.ReadName( maStrm );           break;
                    case 0x0221: Array34(); break;      // ARRAY        [  34 ]
                    case 0x0223: Externname34(); break; // EXTERNNAME   [  34 ]
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x0231: rFontBfr.ReadFont( maStrm );           break;
                    case 0x023E: rTabViewSett.ReadWindow2( maStrm );    break;
                    case 0x027E: Rk(); break;           // RK           [  34 ]
                    case 0x0406: Formula4(); break;     // FORMULA      [   4 ]
                    case 0x041E: rNumFmtBfr.ReadFormat( maStrm );       break;
                    case 0x0443: rXFBfr.ReadXF( maStrm );               break;
                    case 0x0293: rXFBfr.ReadStyle( maStrm );            break;
                }
            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff4W:      // --------------------------------- Z_Biff4W -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345]
                        eAkt = Z_Ende;
                        break;
                    case 0x12:  DocProtect(); break;    // PROTECT      [    5]
                    case 0x2F:                          // FILEPASS     [ 2345]
                        eLastErr = XclImpDecryptHelper::ReadFilepass( maStrm );
                        if( eLastErr != ERRCODE_NONE )
                            eAkt = Z_Ende;
                        break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    case 0x8F:  Bundleheader(); break;  // BUNDLEHEADER [   4 ]
                    case 0x92:  rPal.ReadPalette( maStrm );             break;
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                    case 0x0218: rNameMgr.ReadName( maStrm );           break;
                    case 0x0223: Externname34(); break; // EXTERNNAME   [  34 ]
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x0231: rFontBfr.ReadFont( maStrm );           break;
                    case 0x0409:                        // BOF          [   4 ]
                        Bof4();
                        if( pExcRoot->eDateiTyp == Biff4 )
                        {
                            eAkt = Z_Biff4I;
                            NeueTabelle();
                        }
                        else
                            eAkt = Z_Ende;
                        break;
                    case 0x041E: rNumFmtBfr.ReadFormat( maStrm );       break;
                    case 0x0443: rXFBfr.ReadXF( maStrm );               break;
                    case 0x0293: rXFBfr.ReadStyle( maStrm );            break;
                }

            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff4I:      // --------------------------------- Z_Biff4I -
            {
                switch( nOpcode )
                {
                    case 0x00:  Dimensions(); break;    // DIMENSIONS   [ 2345]
                    case 0x0A:                          // EOF          [ 2345]
                        eAkt = Z_Biff4E;
                        IncCurrScTab();
                        break;
                    case 0x12:  Protect(); break;       // SHEET PROTECTION
                    case 0x14:
                    case 0x15:  rPageSett.ReadHeaderFooter( maStrm );   break;
                    case 0x1A:
                    case 0x1B:  rPageSett.ReadPageBreaks( maStrm );     break;
                    case 0x1C:                          // NOTE         [ 2345]
                        Note();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x1D:  rTabViewSett.ReadSelection( maStrm );   break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        eLastErr = XclImpDecryptHelper::ReadFilepass( maStrm );
                        if( eLastErr != ERRCODE_NONE )
                            eAkt = Z_Ende;
                        break;
                    case 0x41:  rTabViewSett.ReadPane( maStrm );        break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x7D:  Colinfo(); break;       // COLINFO      [  345]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    case 0x8F:  Bundleheader(); break;  // BUNDLEHEADER [   4 ]
                    case 0x92:  rPal.ReadPalette( maStrm );             break;
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                    case 0xA1:  rPageSett.ReadSetup( maStrm );          break;
                    case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345]
                    case 0x0201:                        // BLANK        [  34 ]
                        Blank34();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0203:                        // NUMBER       [  34 ]
                        Number34();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0204:                        // LABEL        [  34 ]
                        Label();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0205:                        // BOOLERR      [  34 ]
                        Boolerr34();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0208: Row34(); break;        // ROW          [  34 ]
                    case 0x0218: rNameMgr.ReadName( maStrm );           break;
                    case 0x0221:                        // ARRAY        [  34 ]
                        Array34();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x0231: rFontBfr.ReadFont( maStrm );           break;
                    case 0x027E:                        // RK           [  34 ]
                        Rk();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0406:                        // FORMULA      [   4 ]
                        Formula4();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x041E: rNumFmtBfr.ReadFormat( maStrm );       break;
                    case 0x0443: rXFBfr.ReadXF( maStrm );               break;
                    case 0x0293: rXFBfr.ReadStyle( maStrm );            break;
                }

            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff4T:      // --------------------------------- Z_Biff4T -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345]
                        EndSheet();
                        IncCurrScTab();
                        eAkt = Z_Biff4E;
                        break;
                    case 0x1C:  Note(); break;          // NOTE         [ 2345]
                    case 0x0201: Blank34(); break;      // BLANK        [  34 ]
                    case 0x0203: Number34(); break;     // NUMBER       [  34 ]
                    case 0x0204: Label(); break;        // LABEL        [  34 ]
                    case 0x0205: Boolerr34(); break;    // BOOLERR      [  34 ]
                    case 0x0207: RecString(); break;    // STRING       [ 2345]
                    case 0x0208: Row34(); break;        // ROW          [  34 ]
                    case 0x0221: Array34(); break;      // ARRAY        [  34 ]
                    case 0x023E: rTabViewSett.ReadWindow2( maStrm );    break;
                    case 0x027E: Rk(); break;           // RK           [  34 ]
                    case 0x0406: Formula4(); break;     // FORMULA      [   4 ]
                }

            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff4E:      // --------------------------------- Z_Biff4E -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345]
                        eAkt = Z_Ende;
                        break;
                    case 0x8F:  break;                  // BUNDLEHEADER [   4 ]
                    case 0x0409:                        // BOF          [   4 ]
                        Bof4();
                        NeueTabelle();
                        if( pExcRoot->eDateiTyp == Biff4 )
                        {
                            eAkt = Z_Biff4I;
                        }
                        else
                        {
                            ePrev = eAkt;
                            eAkt = Z_Biffn0;
                        }
                        break;
                }

            }
                break;
            case Z_Biff5WPre:   // ------------------------------ Z_Biff5WPre -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345]
                        eAkt = Z_Biff5W;
                        aIn.SeekGlobalPosition();  // und zurueck an alte Position
                        break;
                    case 0x12:  DocProtect(); break;    // PROTECT      [    5]
                    case 0x2F:                          // FILEPASS     [ 2345]
                        eLastErr = XclImpDecryptHelper::ReadFilepass( maStrm );
                        if( eLastErr != ERRCODE_NONE )
                            eAkt = Z_Ende;
                        break;
                    case 0x3D:  Window1(); break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x85:  Boundsheet(); break;    // BOUNDSHEET   [    5]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    // PALETTE follows XFs, but already needed while reading the XFs
                    case 0x92:  rPal.ReadPalette( maStrm );             break;
                        break;
                }
            }
                break;
            case Z_Biff5W:      // --------------------------------- Z_Biff5W -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345]
                        rNumFmtBfr.CreateScFormats();
                        rXFBfr.CreateUserStyles();
                        eAkt = Z_Biff5E;
                        break;
                    case 0x18:  rNameMgr.ReadName( maStrm );            break;
                    case 0x1E:  rNumFmtBfr.ReadFormat( maStrm );        break;
                    case 0x22:  Rec1904(); break;       // 1904         [ 2345]
                    case 0x25:  Defrowheight2(); break; // DEFAULTROWHEI[ 2   ]
                    case 0x31:  rFontBfr.ReadFont( maStrm );            break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x8D:  Hideobj(); break;       // HIDEOBJ      [  345]
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                    case 0xDE:  Olesize(); break;
                    case 0xE0:  rXFBfr.ReadXF( maStrm );                break;
                    case 0x0293: rXFBfr.ReadStyle( maStrm );            break;
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x041E: rNumFmtBfr.ReadFormat( maStrm );       break;
                }

            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff5I:      // --------------------------------- Z_Biff5I -
            {
                switch( nOpcode )
                {
                    case 0x00:  Dimensions(); break;    // DIMENSIONS   [ 2345]
                    case 0x01:                          // BLANK        [ 2  5]
                        Blank25();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x03:                          // NUMBER       [ 2  5]
                        Number25();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x04:                          // LABEL        [ 2  5]
                        Label();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x05:                          // BOOLERR      [ 2  5]
                        Boolerr25();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x06:                          // FORMULA      [ 2  5]
                        Formula25();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0A:                          // EOF          [ 2345]
                        EndSheet();
                        eAkt = Z_Biff5E;
                        IncCurrScTab();
                        break;
                    case 0x14:
                    case 0x15:  rPageSett.ReadHeaderFooter( maStrm );   break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x1C:                          // NOTE         [ 2345]
                        Note();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x1D:  rTabViewSett.ReadSelection( maStrm );   break;
                    case 0x23:  Externname25(); break;  // EXTERNNAME   [ 2  5]
                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:  rPageSett.ReadMargin( maStrm );         break;
                    case 0x2A:  rPageSett.ReadPrintHeaders( maStrm );   break;
                    case 0x2B:  rPageSett.ReadPrintGridLines( maStrm ); break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        eLastErr = XclImpDecryptHelper::ReadFilepass( maStrm );
                        if( eLastErr != ERRCODE_NONE )
                            eAkt = Z_Ende;
                        break;
                    case 0x5D:
                        if( bWithDrawLayer )
                            Obj();

                        eAkt = Z_Biff5T; break;
                    case 0x7E:                          // RK           [    5]
                        Rk();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x83:
                    case 0x84:  rPageSett.ReadCenter( maStrm );         break;
                    case 0xA1:  rPageSett.ReadSetup( maStrm );          break;
                    case 0xBD:                          // MULRK        [    5]
                        Mulrk();
                        eAkt = Z_Biff5T;
                        break;
                    case 0xBE:                          // MULBLANK     [    5]
                        Mulblank();
                        eAkt = Z_Biff5T;
                        break;
                    case 0xD6:                          // RSTRING      [    5]
                        Rstring();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345]
                    case 0x0201:                        // BLANK        [  34 ]
                        Blank34();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0203:                        // NUMBER       [  34 ]
                        Number34();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0204:                        // LABEL        [  34 ]
                        Label();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0205:                        // BOOLERR      [  34 ]
                        Boolerr34();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0206:                        // FORMULA      [  3  ]
                        Formula3();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0236:                        // TABLE        [    5]
                        TableOp();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x027E:                        // RK           [  34 ]
                        Rk();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0406:                        // FORMULA      [   4 ]
                        Formula4();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x0809:                        // BOF          [    5]
                        Bof5();
                        NeueTabelle();
                        if( pExcRoot->eDateiTyp == Biff5C )
                        {
                            ePrev = eAkt;
                            eAkt = Z_Biff5C;
                        }
                        else if( pExcRoot->eDateiTyp == Biff5M4 )
                        {
                            ePrev = eAkt;
                            eAkt = Z_Biffn0;
                        }
#ifdef DBG_UTIL
                        else
                        {
                            DBG_ASSERT( pExcRoot->eDateiTyp == Biff5C,
                                "-ImportExcel::Read(): Sofort zu mir (GT)!" );
                        }
#endif
                        break;
                }

            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff5T:      // --------------------------------- Z_Biff5T -
            {
                switch( nOpcode )
                {
                    case 0x01:  Blank25(); break;       // BLANK        [ 2  5]
                    case 0x03:  Number25(); break;      // NUMBER       [ 2  5]
                    case 0x04:  Label(); break;         // LABEL        [ 2  5]
                    case 0x05:  Boolerr25(); break;     // BOOLERR      [ 2  5]
                    case 0x06:  Formula25(); break;     // FORMULA      [ 2  5]
                    case 0x07:  RecString(); break;     // STRING       [ 2345]
                    case 0x0A:                          // EOF          [ 2345]
                        EndSheet();
                        IncCurrScTab();
                        eAkt = Z_Biff5E;
                        break;
                    case 0x1C:  Note(); break;          // NOTE         [ 2345]
                    case 0x5D:                          // OBJ          [ 2345]
                        if( bWithDrawLayer )
                            Obj();

                        break;
                    case 0x7E:  Rk(); break;            // RK           [    5]
                    case 0xA0:  rTabViewSett.ReadScl( maStrm );         break;
                    case 0xBD:  Mulrk(); break;         // MULRK        [    5]
                    case 0xBE:  Mulblank(); break;      // MULBLANK     [    5]
                    case 0xD6:  Rstring(); break;       // RSTRING      [    5]
                    case 0x0201: Blank34(); break;      // BLANK        [  34 ]
                    case 0x0203: Number34(); break;     // NUMBER       [  34 ]
                    case 0x0204: Label(); break;        // LABEL        [  34 ]
                    case 0x0205: Boolerr34(); break;    // BOOLERR      [  34 ]
                    case 0x0206: Formula3(); break;     // FORMULA      [  3  ]
                    case 0x0207: RecString(); break;    // STRING       [ 2345]
                    case 0x0236: TableOp(); break;      // TABLE        [    5]
                    case 0x027E: Rk(); break;           // RK           [  34 ]
                    case 0x0406: Formula4(); break;     // FORMULA      [   4 ]
                    case 0x0809:                        // BOF          [    5]
                        Bof5();
                        if( pExcRoot->eDateiTyp == Biff5C )
                        {
                            ePrev = eAkt;
                            eAkt = Z_Biff5C;
                        }
                        else
                        {
                            ePrev = eAkt;
                            eAkt = Z_Biffn0;
                        }

                        DBG_ASSERT( pExcRoot->eDateiTyp == Biff5C,
                            "-ImportExcel::Read(): Sofort zu mir (GT)!" );
                        break;
                }

            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff5E:      // --------------------------------- Z_Biff5E -
            {
                switch( nOpcode )
                {
                    case 0x0809:                        // BOF          [    5]
                        Bof5();
                        NeueTabelle();
                        switch( pExcRoot->eDateiTyp )
                        {
                            case Biff5:
                                eAkt = Z_Biff5Pre;  // Shrfmla Prefetch, Row-Prefetch
                                nBofLevel = 0;

                                aIn.StoreGlobalPosition(); // und Position merken
                                break;
                            case Biff5C:
                                eAkt = Z_Biff5C;
                                ePrev = Z_Biff5E;
                                break;
                            case Biff5M4:
                            case Biff5V:
                            default:
                                NeueTabelle();
                                pD->SetVisible( GetCurrScTab(), FALSE );
                                ePrev = eAkt;
                                eAkt = Z_Biffn0;
                        }
                        DBG_ASSERT( pExcRoot->eDateiTyp != Biff5W,
                            "+ImportExcel::Read(): Doppel-Whopper-Workbook!" );

                        break;
                }

            }
                break;
            case Z_Biff5Pre:    // ------------------------------- Z_Biff5Pre -
            {
                if( nOpcode == 0x0809 )
                    nBofLevel++;
                else if( (nOpcode == 0x000A) && nBofLevel )
                    nBofLevel--;
                else if( !nBofLevel )                       // don't read chart records
                {
                    switch( nOpcode )
                    {
                        case 0x00:  Dimensions(); break;    // DIMENSIONS   [ 2345]
                        case 0x08:  Row25(); break;         // ROW          [ 2  5]
                        case 0x0A:                          // EOF          [ 2345]
                            eAkt = Z_Biff5I;
                            aIn.SeekGlobalPosition(); // und zurueck an alte Position
                            break;
                        case 0x12:  Protect(); break;       // SHEET PROTECTION
                        case 0x1A:
                        case 0x1B:  rPageSett.ReadPageBreaks( maStrm );     break;
                        case 0x1D:  rTabViewSett.ReadSelection( maStrm );   break;
                        case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                        case 0x21:  Array25(); break;       // ARRAY        [ 2  5]
                        case 0x23:  Externname25(); break;  // EXTERNNAME   [ 2  5]
                        case 0x41:  rTabViewSett.ReadPane( maStrm );        break;
                        case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                        case 0x55:  DefColWidth(); break;
                        case 0x7D:  Colinfo(); break;       // COLINFO      [  345]
                        case 0x81:  Wsbool(); break;        // WSBOOL       [ 2345]
                        case 0x8C:  Country(); break;       // COUNTRY      [  345]
                        case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                        case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345]
                        case 0x0208: Row34(); break;        // ROW          [  34 ]
                        case 0x0221: Array34(); break;      // ARRAY        [  34 ]
                        case 0x0223: Externname34(); break; // EXTERNNAME   [  34 ]
                        case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                        case 0x023E: rTabViewSett.ReadWindow2( maStrm );    break;
                        case 0x04BC: Shrfmla(); break;      // SHRFMLA      [    5]
                    }
                }
            }
                break;
            case Z_Biff5C:  // ------------------------------------- Z_Biff5C -
            {
                if( bWithDrawLayer )
                {
                    switch( nOpcode )
                    {
                        case 0x0A:                                          // EOF          [ 2345]
                            EndChartObj();
                            eAkt = ePrev;   // und retur (zur Tabelle?)
                            break;
                        case 0x1027:    ChartObjectLink();          break;  // OBJECTLINK   [ 2345]
                        case 0x1051:    ChartSelection();           break;  // AI           [    5]
                    }
                }
                else if( nOpcode == 0x0A )
                    eAkt = ePrev;
            }
                break;
            case Z_Biffn0:      // --------------------------------- Z_Biffn0 -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345]
                        eAkt = ePrev;
                        IncCurrScTab();
                        break;
                }

            }
                break;
            // ----------------------------------------------------------------
            case Z_Ende:        // ----------------------------------- Z_Ende -
                DBG_ERROR( "*ImportExcel::Read(): Not possible state!" );
                break;
            default: DBG_ERROR( "-ImportExcel::Read(): Zustand vergessen!" );
        }
    }

    if( eLastErr == eERR_OK )
    {
        pD->CalcAfterLoad();

        pProgress.reset();

        AdjustRowHeight();
        PostDocLoad();

        const XclImpAddressConverter& rAddrConv = GetAddressConverter();
        if( rAddrConv.IsTabTruncated() )
            eLastErr = SCWARN_IMPORT_SHEET_OVERFLOW;
        else if( bTabTruncated || rAddrConv.IsRowTruncated() )
            eLastErr = SCWARN_IMPORT_ROW_OVERFLOW;
        else if( rAddrConv.IsColTruncated() )
            eLastErr = SCWARN_IMPORT_COLUMN_OVERFLOW;
    }

    return eLastErr;
}


//___________________________________________________________________

FltError ImportExcel8::Read( void )
{
#if EXC_INCL_DUMPER
    {
        Biff8RecDumper aDumper( GetRoot(), TRUE );
        if( aDumper.Dump( aIn ) )
            return eERR_OK;
    }
#endif
    // read the entire BIFF8 stream
    // don't look too close - this stuff seriously needs to be reworked

    XclImpPageSettings&     rPageSett       = GetPageSettings();
    XclImpTabViewSettings&  rTabViewSett    = GetTabViewSettings();
    XclImpPalette&          rPal            = GetPalette();
    XclImpFontBuffer&       rFontBfr        = GetFontBuffer();
    XclImpNumFmtBuffer&     rNumFmtBfr      = GetNumFmtBuffer();
    XclImpXFBuffer&         rXFBfr          = GetXFBuffer();
    XclImpSst&              rSst            = GetSst();
    XclImpTabInfo&          rTabInfo        = GetTabInfo();
    XclImpNameManager&      rNameMgr        = GetNameManager();
    XclImpLinkManager&      rLinkMgr        = GetLinkManager();
    XclImpObjectManager&    rObjMgr         = GetObjectManager();
    XclImpCondFormatManager& rCondFmtMgr    = GetCondFormatManager();
    XclImpPivotTableManager& rPTableMgr     = GetPivotTableManager();
    XclImpWebQueryBuffer&   rWQBfr          = GetWebQueryBuffer();

    bool bWithDrawLayer = GetDoc().GetDrawLayer() != 0;     // document has drawing layer
    bool bInUserView = false;                               // true = In USERSVIEW(BEGIN|END) record block.

    enum XclImpReadState
    {
        EXC_STATE_BEFORE_GLOBALS,       /// Before workbook globals (wait for initial BOF).
        EXC_STATE_GLOBALS_PRE,          /// Prefetch for workbook globals.
        EXC_STATE_GLOBALS,              /// Workbook globals.
        EXC_STATE_BEFORE_SHEET,         /// Before worksheet (wait for new worksheet BOF).
        EXC_STATE_SHEET_PRE,            /// Prefetch for worksheet.
        EXC_STATE_SHEET_INIT,           /// Worksheet: initial records.
        EXC_STATE_SHEET,                /// Worksheet: remaining records.
        EXC_STATE_END                   /// Stop reading.
    };

    XclImpReadState eAkt = EXC_STATE_BEFORE_GLOBALS, ePrev = EXC_STATE_BEFORE_GLOBALS;

    FltError eLastErr = eERR_OK;

    ::std::auto_ptr< ScfSimpleProgressBar > pProgress( new ScfSimpleProgressBar(
        aIn.GetSvStreamSize(), GetDocShell(), STR_LOAD_DOC ) );

    bObjSection = FALSE;

    while( eAkt != EXC_STATE_END )
    {
        aIn.StartNextRecord();
        sal_uInt16 nRecId = aIn.GetRecId();
        if( !aIn.IsValid() )
        {
            eAkt = EXC_STATE_END;
            break;
        }

        if( eAkt != EXC_STATE_SHEET_PRE && eAkt != EXC_STATE_GLOBALS_PRE )
            pProgress->ProgressAbs( aIn.GetSvStreamPos() );

        if( nRecId != EXC_ID_CONT )
        {
            aIn.ResetRecord( true );            // enable internal CONTINUE handling
            bObjSection =
                (nRecId == 0x005D) ||           // OBJ
                (nRecId == 0x00EB) ||           // MSODRAWINGGROUP
                (nRecId == 0x00EC) ||           // MSODRAWING
                (nRecId == 0x00ED) ||           // MSODRAWINGSELECTION
                (nRecId == 0x01B6);             // TXO
        }

        /*  #i39464# Ignore records between USERSVIEWBEGIN and USERSVIEWEND
            completely (user specific view settings). Otherwise view settings
            and filters are loaded multiple times, which at least causes
            problems in auto-filters. */
        switch( nRecId )
        {
            case EXC_ID_USERSVIEWBEGIN:
                DBG_ASSERT( !bInUserView, "ImportExcel8::Read - nested user view settings" );
                bInUserView = true;
            break;
            case EXC_ID_USERSVIEWEND:
                DBG_ASSERT( bInUserView, "ImportExcel8::Read - not in user view settings" );
                bInUserView = false;
            break;
        }

        if( !bInUserView ) switch( eAkt )
        {
            // ----------------------------------------------------------------
            // before workbook globals: wait for initial workbook globals BOF
            case EXC_STATE_BEFORE_GLOBALS:
            {
                if( nRecId == EXC_ID5_BOF )
                {
                    DBG_ASSERT( GetBiff() == EXC_BIFF8, "ImportExcel8::Read - wrong BIFF version" );
                    Bof5();
                    eAkt = EXC_STATE_GLOBALS_PRE;
                    maStrm.StoreGlobalPosition();
                    nBdshtTab = 0;
                }
            }
            break;

            // ----------------------------------------------------------------
            // prefetch for workbook globals
            case EXC_STATE_GLOBALS_PRE:
            {
                switch( nRecId )
                {
                    case 0x0A:                          // EOF          [ 2345   ]
                        eAkt = EXC_STATE_GLOBALS;
                        aIn.SeekGlobalPosition();          // und zurueck an alte Position
                        break;
                    case 0x12:  DocProtect(); break;    // PROTECT      [    5678]
                    case 0x19:  WinProtection(); break;
                    case 0x2F:                          // FILEPASS     [ 2345   ]
                        eLastErr = XclImpDecryptHelper::ReadFilepass( maStrm );
                        if( eLastErr != ERRCODE_NONE )
                            eAkt = EXC_STATE_END;
                        break;
                    case 0x3D:  Window1(); break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345   ]
                    case 0x85:  Boundsheet(); break;    // BOUNDSHEET   [    5   ]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345   ]

                    // PALETTE follows XFs, but already needed while reading the XFs
                    case EXC_ID_PALETTE:        rPal.ReadPalette( maStrm );             break;
                }
            }
            break;

            // ----------------------------------------------------------------
            // workbook globals
            case EXC_STATE_GLOBALS:
            {
                switch( nRecId )
                {
                    case 0x0A:                          // EOF          [ 2345   ]
                        rNumFmtBfr.CreateScFormats();
                        rXFBfr.CreateUserStyles();
                        eAkt = EXC_STATE_BEFORE_SHEET;
                        break;
                    case 0x0E:  Precision(); break;     // PRECISION
                    case 0x22:  Rec1904(); break;       // 1904         [ 2345   ]
                    case 0x25:  Defrowheight2(); break; // DEFAULTROWHEI[ 2      ]
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345   ]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34    ]
                    case 0x8D:  Hideobj(); break;       // HIDEOBJ      [  345   ]
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45   ]
                    case 0xD3:  ReadBasic(); break;
                    case 0xDE:  Olesize(); break;
                    case 0xEB:  Msodrawinggroup(); break;
                    case 0x01BA: Codename( TRUE ); break;
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345   ]

                    case EXC_ID_USESELFS:       ReadUsesElfs();                     break;

                    case EXC_ID_FONT:           rFontBfr.ReadFont( maStrm );        break;
                    case EXC_ID_FORMAT:         rNumFmtBfr.ReadFormat( maStrm );    break;
                    case EXC_ID_XF:             rXFBfr.ReadXF( maStrm );            break;
                    case EXC_ID_STYLE:          rXFBfr.ReadStyle( maStrm );         break;

                    case EXC_ID_SST:            rSst.ReadSst( maStrm );             break;
                    case EXC_ID_TABID:          rTabInfo.ReadTabid( maStrm );       break;
                    case EXC_ID_NAME:           rNameMgr.ReadName( maStrm );        break;

                    case EXC_ID_EXTERNSHEET:    rLinkMgr.ReadExternsheet( maStrm ); break;
                    case EXC_ID_SUPBOOK:        rLinkMgr.ReadSupbook( maStrm );     break;
                    case EXC_ID_XCT:            rLinkMgr.ReadXct( maStrm );         break;
                    case EXC_ID_CRN:            rLinkMgr.ReadCrn( maStrm );         break;
                    case EXC_ID_EXTERNNAME:     rLinkMgr.ReadExternname( maStrm );  break;

                    case EXC_ID_DCONREF:        rPTableMgr.ReadDconref( maStrm );   break;
                    case EXC_ID_SXIDSTM:        rPTableMgr.ReadSxidstm( maStrm );   break;
                    case EXC_ID_SXVS:           rPTableMgr.ReadSxvs( maStrm );      break;
                }

            }
            break;

            // ----------------------------------------------------------------
            // before worksheet: wait for new worksheet BOF
            case EXC_STATE_BEFORE_SHEET:
            {
                if( nRecId == EXC_ID5_BOF )
                {
                    // #94191# import only 256 sheets
                    if( GetCurrScTab() > GetScMaxPos().Tab() )
                    {
                        XclTools::SkipSubStream( maStrm );
                        // #i29930# show warning box
                        GetAddressConverter().CheckScTab( GetCurrScTab(), true );
                        eAkt = EXC_STATE_END;
                    }
                    else
                    {
                        Bof5();
                        NeueTabelle();
                        switch( pExcRoot->eDateiTyp )
                        {
                            case Biff8:     // worksheet
                            case Biff8M4:   // macro sheet
                                eAkt = EXC_STATE_SHEET_PRE;  // Shrfmla Prefetch, Row-Prefetch
                                aIn.StoreGlobalPosition();
                            break;
                            case Biff8C:    // chart sheet
                                rObjMgr.ReadTabChart( maStrm );
                                EndSheet();
                                IncCurrScTab();
                                GetTracer().TraceChartOnlySheet();
                            break;
                            case Biff8W:    // workbook
                                DBG_ERRORFILE( "ImportExcel8::Read - double workbook globals" );
                                // run through
                            case Biff8V:    // VB module
                            default:
                                // TODO: do not create a sheet in the Calc document
                                pD->SetVisible( GetCurrScTab(), FALSE );
                                XclTools::SkipSubStream( maStrm );
                                IncCurrScTab();
                        }
                    }
                }
            }
            break;

            // ----------------------------------------------------------------
            // prefetch for worksheet
            case EXC_STATE_SHEET_PRE:
            {
                switch( nRecId )
                {
                    // skip chart substream
                    case EXC_ID2_BOF:
                    case EXC_ID3_BOF:
                    case EXC_ID4_BOF:
                    case EXC_ID5_BOF:           XclTools::SkipSubStream( maStrm );      break;

                    case EXC_ID_WINDOW2:        rTabViewSett.ReadWindow2( maStrm );     break;
                    case EXC_ID_SCL:            rTabViewSett.ReadScl( maStrm );         break;
                    case EXC_ID_PANE:           rTabViewSett.ReadPane( maStrm );        break;
                    case EXC_ID_SELECTION:      rTabViewSett.ReadSelection( maStrm );   break;

                    case 0x00:  Dimensions(); break;    // DIMENSIONS   [ 2345   ]
                    case 0x08:  Row25(); break;         // ROW          [ 2  5   ]
                    case 0x0A:                          // EOF          [ 2345   ]
                        eAkt = EXC_STATE_SHEET_INIT;
                        aIn.SeekGlobalPosition();         // und zurueck an alte Position
                        break;
                    case 0x12:  Protect(); break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345   ]
                    case 0x55:  DefColWidth(); break;
                    case 0x7D:  Colinfo(); break;       // COLINFO      [  345   ]
                    case 0x81:  Wsbool(); break;        // WSBOOL       [ 2345   ]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345   ]
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45   ]
                    case 0x9B:  FilterMode(); break;    // FILTERMODE
                    case 0x9D:  AutoFilterInfo(); break;// AUTOFILTERINFO
                    case 0x9E:  AutoFilter(); break;    // AUTOFILTER
                    case 0x01BA: Codename( FALSE ); break;
                    case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345   ]
                    case 0x0208: Row34(); break;        // ROW          [  34    ]
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345   ]
                    case 0x04BC: Shrfmla(); break;      // SHRFMLA      [    5   ]
                }
            }
            break;

            // ----------------------------------------------------------------
            // worksheet
            case EXC_STATE_SHEET_INIT:
            case EXC_STATE_SHEET:
            {
                // EXC_STATE_SHEET_INIT (initial records) and EXC_STATE_SHEET (common table records) together
                BOOL bFound = FALSE;

                // 1st: read records for EXC_STATE_SHEET_INIT only
                if( (eAkt == EXC_STATE_SHEET_INIT) && !bFound )
                {
                    bFound = TRUE;
                    switch( nRecId )
                    {
                        case 0x000C:    Calccount();            break;  // CALCCOUNT
                        case 0x0010:    Delta();                break;  // DELTA
                        case 0x0011:    Iteration();            break;  // ITERATION
                        case 0x0200:    Dimensions();           break;  // DIMENSIONS   [      8 ]

                        case EXC_ID_HORPAGEBREAKS:
                        case EXC_ID_VERPAGEBREAKS:  rPageSett.ReadPageBreaks( maStrm );     break;
                        case EXC_ID_HEADER:
                        case EXC_ID_FOOTER:         rPageSett.ReadHeaderFooter( maStrm );   break;
                        case EXC_ID_LEFTMARGIN:
                        case EXC_ID_RIGHTMARGIN:
                        case EXC_ID_TOPMARGIN:
                        case EXC_ID_BOTTOMMARGIN:   rPageSett.ReadMargin( maStrm );         break;
                        case EXC_ID_PRINTHEADERS:   rPageSett.ReadPrintHeaders( maStrm );   break;
                        case EXC_ID_PRINTGRIDLINES: rPageSett.ReadPrintGridLines( maStrm ); break;
                        case EXC_ID_HCENTER:
                        case EXC_ID_VCENTER:        rPageSett.ReadCenter( maStrm );         break;
                        case EXC_ID_SETUP:          rPageSett.ReadSetup( maStrm );          break;
                        case EXC_ID_BITMAP:         rPageSett.ReadBitmap( maStrm );         break;

                        default:        bFound = FALSE;
                    }
                }

                // 2nd: common table records, change mode to EXC_STATE_SHEET
                if( !bFound )
                {
                    bFound = TRUE;
                    switch( nRecId )
                    {
                        case 0x0001:    Blank25();              break;  // BLANK        [ 2  5   ]
                        case 0x0003:    Number25();             break;  // NUMBER       [ 2  5   ]
                        case 0x0004:    Label();                break;  // LABEL        [ 2  5   ]
                        case 0x0005:    Boolerr25();            break;  // BOOLERR      [ 2  5   ]
                        case 0x0006:
                        case 0x0206:
                        case 0x0406:    Formula25();            break;  // FORMULA      [ 2  5   ]
                        case 0x001C:    Note();                 break;  // NOTE         [ 2345   ]
                        case 0x005D:    if( bWithDrawLayer ) Obj(); break;  // OBJ      [ 2345   ]
                        case 0x007E:
                        case 0x027E:    Rk();                   break;  // RK           [    5   ]
                        case 0x00AE:    Scenman();              break;  // SCENMAN
                        case 0x00AF:    Scenario();             break;  // SCENARIO
                        case 0x00BD:    Mulrk();                break;  // MULRK        [    5   ]
                        case 0x00BE:    Mulblank();             break;  // MULBLANK     [    5   ]
                        case 0x00D6:    Rstring();              break;  // RSTRING      [    5   ]
                        case 0x00E5:    Cellmerging();          break;  // CELLMERGING
                        case 0x00EC:    Msodrawing();           break;  // MSODRAWING
                        case 0x00ED:    Msodrawingselection();  break;  // MSODRAWINGSELECTION
                        case 0x00FD:    Labelsst();             break;  // LABELSST     [      8 ]
                        case 0x01B6:    Txo();                  break;  // TXO
                        case 0x0201:    Blank34();              break;  // BLANK        [  34    ]
                        case 0x0203:    Number34();             break;  // NUMBER       [  34    ]
                        case 0x0204:    Label();                break;  // LABEL        [  34    ]
                        case 0x0205:    Boolerr34();            break;  // BOOLERR      [  34    ]
                        case 0x0236:    TableOp();              break;  // TABLE
                        case 0x0021:    Array25();              break;  // ARRAY        [ 2  5   ]
                        case 0x0221:    Array34();              break;  // ARRAY        [  34    ]

                        case EXC_ID_HLINK:          XclImpHyperlink::ReadHlink( maStrm );   break;
                        case EXC_ID_LABELRANGES:    XclImpLabelranges::ReadLabelranges( maStrm ); break;

                        case EXC_ID_CONDFMT:        rCondFmtMgr.ReadCondfmt( maStrm );      break;
                        case EXC_ID_CF:             rCondFmtMgr.ReadCF( maStrm );           break;

                        case EXC_ID_DVAL:           XclImpValidation::ReadDval( maStrm );   break;
                        case EXC_ID_DV:             XclImpValidation::ReadDV( maStrm );     break;

                        case EXC_ID_QSI:            rWQBfr.ReadQsi( maStrm );               break;
                        case EXC_ID_WQSTRING:       rWQBfr.ReadWqstring( maStrm );          break;
                        case EXC_ID_PQRY:           rWQBfr.ReadParamqry( maStrm );          break;
                        case EXC_ID_WQSETT:         rWQBfr.ReadWqsettings( maStrm );        break;
                        case EXC_ID_WQTABLES:       rWQBfr.ReadWqtables( maStrm );          break;

                        default:
                            bFound = FALSE;
                    }
                    if( bFound )
                        eAkt = EXC_STATE_SHEET;
                }

                // 3rd: read records for EXC_STATE_SHEET only
                if( (eAkt == EXC_STATE_SHEET) && !bFound )
                {
                    bFound = TRUE;
                    switch( nRecId )
                    {
                        case 0x0007:    RecString();            break;  // STRING       [ 2345   ]
                        case 0x003C:    Cont();                 break;  // CONTINUE
                        case 0x0207:    RecString();            break;  // STRING       [ 2345   ]

                        case EXC_ID_SXVIEW:         rPTableMgr.ReadSxview( maStrm );    break;
                        case EXC_ID_SXVD:           rPTableMgr.ReadSxvd( maStrm );      break;
                        case EXC_ID_SXVI:           rPTableMgr.ReadSxvi( maStrm );      break;
                        case EXC_ID_SXIVD:          rPTableMgr.ReadSxivd( maStrm );     break;
                        case EXC_ID_SXPI:           rPTableMgr.ReadSxpi( maStrm );      break;
                        case EXC_ID_SXDI:           rPTableMgr.ReadSxdi( maStrm );      break;
                        case EXC_ID_SXVDEX:         rPTableMgr.ReadSxvdex( maStrm );    break;
                        case EXC_ID_SXEX:           rPTableMgr.ReadSxex( maStrm );      break;

                        case 0x0809:                                    // BOF          [    5   ]
                        {
                            Bof5();
                            DBG_ASSERT( pExcRoot->eDateiTyp == Biff8C, "unknown BOF" );
                            XclTools::SkipSubStream( maStrm );
                            // still in Escher objects
                            bObjSection = TRUE;
                        }
                        break;
                        default:        bFound = FALSE;
                    }
                }

                // 4th: read records for EXC_STATE_SHEET_INIT and EXC_STATE_SHEET
                if( !bFound )
                {
                    bFound = TRUE;
                    switch( nRecId )
                    {
                        case 0x000A:                            // EOF          [ 2345   ]
                        {
                            EndSheet();
                            eAkt = EXC_STATE_BEFORE_SHEET;
                            IncCurrScTab();
                        }
                        break;
                        default:    bFound = FALSE;
                    }
                }
            }
            break;

            // ----------------------------------------------------------------
            default:;
        }
    }

    if( eLastErr == eERR_OK )
    {
        pD->CalcAfterLoad();

        pProgress.reset();

        AdjustRowHeight();
        PostDocLoad();

        // import change tracking data
        XclImpChangeTrack aImpChTr( GetRoot(), maStrm );
        aImpChTr.Apply();

        const XclImpAddressConverter& rAddrConv = GetAddressConverter();
        if( rAddrConv.IsTabTruncated() )
            eLastErr = SCWARN_IMPORT_SHEET_OVERFLOW;
        else if( bTabTruncated || rAddrConv.IsRowTruncated() )
            eLastErr = SCWARN_IMPORT_ROW_OVERFLOW;
        else if( rAddrConv.IsColTruncated() )
            eLastErr = SCWARN_IMPORT_COLUMN_OVERFLOW;
    }

    return eLastErr;
}

//___________________________________________________________________

