/*************************************************************************
 *
 *  $RCSfile: read.cxx,v $
 *
 *  $Revision: 1.44 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:53:47 $
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

#include <stdlib.h>
#include <stdio.h>

#include "document.hxx"
#include "docoptio.hxx"
#include "globstr.hrc"
#include "stlsheet.hxx"
#include "stlpool.hxx"
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
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
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

#ifndef SC_XCLIMPCHARTS_HXX
#include "XclImpCharts.hxx"
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
        Biff8RecDumper aDumper( *pExcRoot, FALSE );
        if( aDumper.Dump( aIn ) )
            return eERR_OK;
    }
#endif

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
        aIn.GetStreamSize(), GetDocShell(), STR_LOAD_DOC ) );

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
            pProgress->Progress( aIn.Tell() );

        switch( eAkt )
        {
            // ----------------------------------------------------------------
            case Z_BiffNull:    // ------------------------------- Z_BiffNull -
            {
                switch( nOpcode )
                {
                    case 0x09:                          // BOF          [ 2   ]
                        Bof2();
                        if( pExcRoot->eDateiTyp == Biff2 )
                        {
                            eAkt = Z_Biff2;
                            NeueTabelle();
                        }
                        break;
                    case 0x0209:                        // BOF          [  3  ]
                        Bof3();
                        if( pExcRoot->eDateiTyp == Biff3 )
                        {
                            eAkt = Z_Biff3;
                            NeueTabelle();
                        }
                        break;
                    case 0x0409:                        // BOF          [   4 ]
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
                    case 0x0809:                        // BOF          [    5]
                    {
                        Bof5();
                        if( pExcRoot->eDateiTyp == Biff5W )
                        {
                            eAkt = Z_Biff5WPre;

                            nBdshtTab = 0;

                            aIn.StoreGlobalPosition(); // und Position merken
                        }

                        DBG_ASSERT( pExcRoot->eDateiTyp != Biff5,
                            "+ImportExcel::Read(): Tabelle ohne Workbook!" );
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
                    case 0x04:  Label25(); break;       // LABEL        [ 2  5]
                    case 0x05:  Boolerr25(); break;     // BOOLERR      [ 2  5]
                    case 0x06:  Formula25(); break;     // FORMULA      [ 2  5]
                    case 0x07:  RecString(); break;     // STRING       [ 2345]
                    case 0x08:  Row25(); break;         // ROW          [ 2  5]
                    case 0x0A:                          // EOF          [ 2345]
                        EndSheet();
                        eAkt = Z_Ende;
                        break;
                    case 0x14:
                    case 0x15:  GetPageSettings().ReadHeaderFooter( maStrm );   break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x18:  GetNameBuffer().ReadName( maStrm );             break;
                    case 0x1C:  Note(); break;          // NOTE         [ 2345]
                    case 0x1D:  Selection(); break;     // SELECTION    [ 2345]
                    case 0x1E:  GetNumFmtBuffer().ReadFormat( maStrm );         break;
                    case 0x20:  Columndefault(); break; // COLUMNDEFAULT[ 2   ]
                    case 0x21:  Array25(); break;       // ARRAY        [ 2  5]
                    case 0x23:  Externname25(); break;  // EXTERNNAME   [ 2  5]
                    case 0x24:  Colwidth(); break;      // COLWIDTH     [ 2   ]
                    case 0x25:  Defrowheight2(); break; // DEFAULTROWHEI[ 2   ]
                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:  GetPageSettings().ReadMargin( maStrm );         break;
                    case 0x2A:  GetPageSettings().ReadPrintheaders( maStrm );   break;
                    case 0x2B:  GetPageSettings().ReadPrintgridlines( maStrm ); break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        if( Filepass() )
                        {
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
                        break;
                    case 0x31:  GetFontBuffer().ReadFont( maStrm );             break;
                    case 0x41:  Pane(); break;          // PANE         [ 2345]
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x43:  GetXFBuffer().ReadXF( maStrm );                 break;
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
                        EndSheet();
                        eAkt = Z_Ende;
                        break;
                    case 0x14:
                    case 0x15:  GetPageSettings().ReadHeaderFooter( maStrm );   break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x1A:
                    case 0x1B:  GetPageSettings().ReadPageBreaks( maStrm );     break;
                    case 0x1C:  Note(); break;          // NOTE         [ 2345]
                    case 0x1D:  Selection(); break;     // SELECTION    [ 2345]
                    case 0x1E:  GetNumFmtBuffer().ReadFormat( maStrm );         break;
                    case 0x22:  Rec1904(); break;       // 1904         [ 2345]
                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:  GetPageSettings().ReadMargin( maStrm );         break;
                    case 0x2A:  GetPageSettings().ReadPrintheaders( maStrm );   break;
                    case 0x2B:  GetPageSettings().ReadPrintgridlines( maStrm ); break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        if( Filepass() )
                        {
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
                        break;
                    case 0x41:  Pane(); break;          // PANE         [ 2345]
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x7D:  Colinfo(); break;       // COLINFO      [  345]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    case 0x92:  Palette(); break;       // PALETTE      [  345]
                    case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345]
                    case 0x0201: Blank34(); break;      // BLANK        [  34 ]
                    case 0x0203: Number34(); break;     // NUMBER       [  34 ]
                    case 0x0204: Label34(); break;      // LABEL        [  34 ]
                    case 0x0205: Boolerr34(); break;    // BOOLERR      [  34 ]
                    case 0x0206: Formula3(); break;     // FORMULA      [  3  ]
                    case 0x0207: RecString(); break;    // STRING       [ 2345]
                    case 0x0208: Row34(); break;        // ROW          [  34 ]
                    case 0x0218: GetNameBuffer().ReadName( maStrm );            break;
                    case 0x0221: Array34(); break;      // ARRAY        [  34 ]
                    case 0x0223: Externname34(); break; // EXTERNNAME   [  34 ]
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x0231: GetFontBuffer().ReadFont( maStrm );            break;
                    case 0x023E: Window2_5(); break;    // WINDOW       [    5]
                    case 0x0243: GetXFBuffer().ReadXF( maStrm );                break;
                    case 0x0293: GetXFBuffer().ReadStyle( maStrm );             break;
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
                        EndSheet();
                        eAkt = Z_Ende;
                        break;
                    case 0x12:  Protect(); break;       // SHEET PROTECTION
                    case 0x14:
                    case 0x15:  GetPageSettings().ReadHeaderFooter( maStrm );   break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x1A:
                    case 0x1B:  GetPageSettings().ReadPageBreaks( maStrm );     break;
                    case 0x1C:  Note(); break;          // NOTE         [ 2345]
                    case 0x1D:  Selection(); break;     // SELECTION    [ 2345]
                    case 0x22:  Rec1904(); break;       // 1904         [ 2345]
                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:  GetPageSettings().ReadMargin( maStrm );         break;
                    case 0x2A:  GetPageSettings().ReadPrintheaders( maStrm );   break;
                    case 0x2B:  GetPageSettings().ReadPrintgridlines( maStrm ); break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        if( Filepass() )
                        {
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
                    case 0x41:  Pane(); break;          // PANE         [ 2345]
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x7D:  Colinfo(); break;       // COLINFO      [  345]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    case 0x92:  Palette(); break;       // PALETTE      [  345]
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                    case 0xA1:  GetPageSettings().ReadSetup( maStrm );          break;
                    case 0x0200: Dimensions(); break;   // DIMENSIONS   [ 2345]
                    case 0x0201: Blank34(); break;      // BLANK        [  34 ]
                    case 0x0203: Number34(); break;     // NUMBER       [  34 ]
                    case 0x0204: Label34(); break;      // LABEL        [  34 ]
                    case 0x0205: Boolerr34(); break;    // BOOLERR      [  34 ]
                    case 0x0207: RecString(); break;    // STRING       [ 2345]
                    case 0x0208: Row34(); break;        // ROW          [  34 ]
                    case 0x0218: GetNameBuffer().ReadName( maStrm );            break;
                    case 0x0221: Array34(); break;      // ARRAY        [  34 ]
                    case 0x0223: Externname34(); break; // EXTERNNAME   [  34 ]
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x0231: GetFontBuffer().ReadFont( maStrm );            break;
                    case 0x023E: Window2_5(); break;    // WINDOW       [    5]
                    case 0x027E: Rk(); break;           // RK           [  34 ]
                    case 0x0406: Formula4(); break;     // FORMULA      [   4 ]
                    case 0x041E: GetNumFmtBuffer().ReadFormat( maStrm );        break;
                    case 0x0443: GetXFBuffer().ReadXF( maStrm );                break;
                    case 0x0293: GetXFBuffer().ReadStyle( maStrm );             break;
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
                        if( Filepass() )
                        {
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
                        break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    case 0x8F:  Bundleheader(); break;  // BUNDLEHEADER [   4 ]
                    case 0x92:  Palette(); break;       // PALETTE      [  345]
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                    case 0x0218: GetNameBuffer().ReadName( maStrm );            break;
                    case 0x0223: Externname34(); break; // EXTERNNAME   [  34 ]
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x0231: GetFontBuffer().ReadFont( maStrm );            break;
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
                    case 0x041E: GetNumFmtBuffer().ReadFormat( maStrm );        break;
                    case 0x0443: GetXFBuffer().ReadXF( maStrm );                break;
                    case 0x0293: GetXFBuffer().ReadStyle( maStrm );             break;
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
                    case 0x15:  GetPageSettings().ReadHeaderFooter( maStrm );   break;
                    case 0x1A:
                    case 0x1B:  GetPageSettings().ReadPageBreaks( maStrm );     break;
                    case 0x1C:                          // NOTE         [ 2345]
                        Note();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x1D:  Selection(); break;     // SELECTION    [ 2345]
                    case 0x2F:                          // FILEPASS     [ 2345]
                        if( Filepass() )
                        {
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
                        break;
                    case 0x41:  Pane(); break;          // PANE         [ 2345]
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x7D:  Colinfo(); break;       // COLINFO      [  345]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    case 0x8F:  Bundleheader(); break;  // BUNDLEHEADER [   4 ]
                    case 0x92:  Palette(); break;       // PALETTE      [  345]
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                    case 0xA1:  GetPageSettings().ReadSetup( maStrm );          break;
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
                        Label34();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0205:                        // BOOLERR      [  34 ]
                        Boolerr34();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0208: Row34(); break;        // ROW          [  34 ]
                    case 0x0218: GetNameBuffer().ReadName( maStrm );            break;
                    case 0x0221:                        // ARRAY        [  34 ]
                        Array34();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x0231: GetFontBuffer().ReadFont( maStrm );            break;
                    case 0x027E:                        // RK           [  34 ]
                        Rk();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x0406:                        // FORMULA      [   4 ]
                        Formula4();
                        eAkt = Z_Biff4T;
                        break;
                    case 0x041E: GetNumFmtBuffer().ReadFormat( maStrm );        break;
                    case 0x0443: GetXFBuffer().ReadXF( maStrm );                break;
                    case 0x0293: GetXFBuffer().ReadStyle( maStrm );             break;
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
                    case 0x0204: Label34(); break;      // LABEL        [  34 ]
                    case 0x0205: Boolerr34(); break;    // BOOLERR      [  34 ]
                    case 0x0207: RecString(); break;    // STRING       [ 2345]
                    case 0x0208: Row34(); break;        // ROW          [  34 ]
                    case 0x0221: Array34(); break;      // ARRAY        [  34 ]
                    case 0x023E: Window2_5(); break;    // WINDOW       [    5]
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
                        if( Filepass() )
                        {
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
                        break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x85:  Boundsheet(); break;    // BOUNDSHEET   [    5]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345]
                    // PALETTE follows XFs, but needed while reading the XFs
                    case 0x92:  Palette(); break;       // PALETTE      [  345]
                        break;
                }
            }
                break;
            case Z_Biff5W:      // --------------------------------- Z_Biff5W -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345]
                        eAkt = Z_Biff5E;
                        break;
                    case 0x18:  GetNameBuffer().ReadName( maStrm );             break;
                    case 0x1E:  GetNumFmtBuffer().ReadFormat( maStrm );         break;
                    case 0x22:  Rec1904(); break;       // 1904         [ 2345]
                    case 0x25:  Defrowheight2(); break; // DEFAULTROWHEI[ 2   ]
                    case 0x2F:                          // FILEPASS     [ 2345]
                        if( Filepass() )
                        {
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
                        break;
                    case 0x31:  GetFontBuffer().ReadFont( maStrm );             break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34 ]
                    case 0x8D:  Hideobj(); break;       // HIDEOBJ      [  345]
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45]
                    case 0xDE:  Olesize(); break;
                    case 0xE0:  GetXFBuffer().ReadXF( maStrm );                 break;
                    case 0x0293: GetXFBuffer().ReadStyle( maStrm );             break;
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345]
                    case 0x041E: GetNumFmtBuffer().ReadFormat( maStrm );        break;
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
                        Label25();
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
                    case 0x15:  GetPageSettings().ReadHeaderFooter( maStrm );   break;
                    case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                    case 0x1C:                          // NOTE         [ 2345]
                        Note();
                        eAkt = Z_Biff5T;
                        break;
                    case 0x1D:  Selection(); break;     // SELECTION    [ 2345]
                    case 0x23:  Externname25(); break;  // EXTERNNAME   [ 2  5]
                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:  GetPageSettings().ReadMargin( maStrm );         break;
                    case 0x2A:  GetPageSettings().ReadPrintheaders( maStrm );   break;
                    case 0x2B:  GetPageSettings().ReadPrintgridlines( maStrm ); break;
                    case 0x2F:                          // FILEPASS     [ 2345]
                        if( Filepass() )
                        {
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
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
                    case 0x84:  GetPageSettings().ReadCenter( maStrm );         break;
                    case 0xA1:  GetPageSettings().ReadSetup( maStrm );          break;
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
                        Label34();
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
                    case 0x04:  Label25(); break;       // LABEL        [ 2  5]
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
                    case 0xA0:  Scl(); break;           // SCL          [   45]
                    case 0xBD:  Mulrk(); break;         // MULRK        [    5]
                    case 0xBE:  Mulblank(); break;      // MULBLANK     [    5]
                    case 0xD6:  Rstring(); break;       // RSTRING      [    5]
                    case 0x0201: Blank34(); break;      // BLANK        [  34 ]
                    case 0x0203: Number34(); break;     // NUMBER       [  34 ]
                    case 0x0204: Label34(); break;      // LABEL        [  34 ]
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
                        case 0x1B:  GetPageSettings().ReadPageBreaks( maStrm );     break;
                        case 0x1D:  Selection(); break;     // SELECTION    [ 2345]
                        case 0x17:  Externsheet(); break;   // EXTERNSHEET  [ 2345]
                        case 0x21:  Array25(); break;       // ARRAY        [ 2  5]
                        case 0x23:  Externname25(); break;  // EXTERNNAME   [ 2  5]
                        case 0x41:  Pane(); break;          // PANE         [ 2345]
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
                        case 0x023E: Window2_5(); break;    // WINDOW       [    5]
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
                        case 0x1002:    ChartChart();               break;
                        case 0x1003:    ChartSeries();              break;
                        //case 0x1004:  ;break;
                        case 0x1006:    ChartDataformat();          break;
                        case 0x1007:    ChartLineform();            break;
                        case 0x1009:    ChartMarkerformat();        break;
                        case 0x100A:    ChartAreaformat();          break;
                        case 0x100B:    ChartPieformat();           break;
                        case 0x100C:    ChartAttachedlabel();       break;
                        case 0x100D:    ChartSeriestext();          break;  // SERIESTEXT   [ 2345]
                        case 0x1014:    ChartChartformat();         break;
                        case 0x1015:    ChartLegend();              break;
                        case 0x1016:    ChartSerieslist();          break;
                        case 0x1017:    ChartBar();                 break;
                        case 0x1018:    ChartLine();                break;
                        case 0x1019:    ChartPie();                 break;
                        case 0x101A:    ChartArea();                break;
                        //case 0x101B:  ;break;
                        case 0x101C:    ChartLine();                break;
                        case 0x101D:    ChartAxis();                break;
                        case 0x101E:    ChartTick();                break;
                        case 0x101F:    ChartValuerange();          break;
                        case 0x1020:    ChartCatserrange();         break;
                        case 0x1021:    ChartAxislineformat();      break;
                        //case 0x1022:  ; break;
                        case 0x1024:    ChartDefaulttext();         break;
                        case 0x1025:    ChartText();                break;
                        case 0x1026:    ChartFontx();               break;
                        case 0x1027:    ChartObjectLink();          break;  // OBJECTLINK   [ 2345]
                        case 0x1032:    ChartFrame();               break;
                        //case 0x1033:  ;break;
                        //case 0x1034:  ;break;
                        case 0x1035:    ChartPlotarea();            break;
                        case 0x103A:    Chart3D();                  break;
                        case 0x103C:    ChartPicf();                break;
                        case 0x103D:    ChartDropbar();             break;
                        case 0x103E:    ChartRadar();               break;
                        case 0x103F:    ChartSurface();             break;
                        case 0x1040:    ChartRadararea();           break;
                        case 0x1041:    ChartAxisparent();          break;
                        case 0x1043:    ChartLegendxn();            break;
                        case 0x1044:    ChartShtprops();            break;
                        case 0x1045:    ChartSertocrt();            break;
                        case 0x1046:    ChartAxesused();            break;
                        case 0x1048:    ChartSbaseref();            break;
                        case 0x104A:    ChartSerparent();           break;
                        case 0x104B:    ChartSerauxtrend();         break;
                        case 0x104E:    ChartIfmt();                break;
                        case 0x104F:    ChartPos();                 break;
                        case 0x1050:    ChartAlruns();              break;
                        case 0x1051:    ChartSelection();           break;  // AI           [    5]
                        case 0x105B:    ChartSerauxerrbar();        break;
                        case 0x105D:    ChartSerfmt();              break;
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

        if( bTabTruncated || IsTruncated() )
            eLastErr = eERR_RNGOVRFLW;
    }

    return eLastErr;
}


//___________________________________________________________________

FltError ImportExcel8::Read( void )
{
#if EXC_INCL_DUMPER
    {
        Biff8RecDumper aDumper( *pExcRoot, TRUE );
        if( aDumper.Dump( aIn ) )
            return eERR_OK;
    }
#endif

    UINT16              nOpcode;            // aktueller Opcode
    UINT16              nBofLevel = 0;

    const BOOL          bWithDrawLayer = pD->GetDrawLayer() != NULL;

    enum Zustand {
        Z_BiffNull, // Nicht in gueltigem Biff-Format
        Z_Biff8WPre,// Biff8: Prefetch Workbook
        Z_Biff8W,   // Biff8: Globals
        Z_Biff8I,   // Biff8: Initiales fuer eine Tabelle
        Z_Biff8T,   // Biff8: eine Tabelle selbst
        Z_Biff8E,   // Biff8: zwischen den Tabellen
        Z_Biff8Pre, // Biff8: Prefetch fuer Shrfmla/Array Formula
        Z_Biff8C,   // Biff8: Chart (nur noch zum Ueberlesen)

        Z_Biffn0,   // Alle Biffs: Tabelle bis naechstesss EOF ueberlesen
        Z_Ende };

    Zustand             eAkt = Z_BiffNull, ePrev = Z_BiffNull;

    FltError            eLastErr = eERR_OK;

    DBG_ASSERT( &aIn != NULL,
        "-ImportExcel8::Read(): Kein Stream - wie dass?!" );

    ::std::auto_ptr< ScfSimpleProgressBar > pProgress( new ScfSimpleProgressBar(
        aIn.GetStreamSize(), GetDocShell(), STR_LOAD_DOC ) );

    bObjSection = FALSE;

    while( eAkt != Z_Ende )
    {
        aIn.StartNextRecord();
        nOpcode = aIn.GetRecId();
        if( !aIn.IsValid() )
        {
            eAkt = Z_Ende;
            break;
        }

        if( eAkt != Z_Biff8Pre && eAkt != Z_Biff8WPre )
            pProgress->Progress( aIn.Tell() );

        if( nOpcode != EXC_ID_CONT )
        {
            aIn.InitializeRecord( TRUE );       // enable internal CONTINUE handling
            bObjSection =
                (nOpcode == 0x005D) ||          // OBJ
                (nOpcode == 0x00EB) ||          // MSODRAWINGGROUP
                (nOpcode == 0x00EC) ||          // MSODRAWING
                (nOpcode == 0x00ED) ||          // MSODRAWINGSELECTION
                (nOpcode == 0x01B6);            // TXO
        }

        switch( eAkt )
        {
            case Z_BiffNull:    // ------------------------------- Z_BiffNull -
            {
                switch( nOpcode )
                {
                    case 0x0809:                        // BOF          [    5 8 ]
                    {
                        Bof5();
                        if( pExcRoot->eHauptDateiTyp == Biff8 )
                        {
                            eAkt = Z_Biff8WPre;

                            nBdshtTab = 0;

                            aIn.StoreGlobalPosition();
                        }

                        DBG_ASSERT( pExcRoot->eDateiTyp != Biff8,
                            "+ImportExcel8::Read(): Tabelle ohne Workbook!" );
                    }
                        break;
                }
            }
                break;
            case Z_Biff8WPre:   // ------------------------------ Z_Biff8WPre -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345   ]
                        eAkt = Z_Biff8W;
                        aIn.SeekGlobalPosition();          // und zurueck an alte Position
                        break;
                    case 0x12:  DocProtect(); break;    // PROTECT      [    5678]
                    case 0x19:  WinProtection(); break;
                    case 0x2F:                          // FILEPASS     [ 2345   ]
                        if( Filepass() )
                        {
                            GetTracer().TraceLog(ePassword);
                            eLastErr = eERR_FILEPASSWD;
                            eAkt = Z_Ende;
                        }
                        break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345   ]
                    case 0x85:  Boundsheet(); break;    // BOUNDSHEET   [    5   ]
                    case 0x8C:  Country(); break;       // COUNTRY      [  345   ]
                    // PALETTE follows XFs, but needed while reading the XFs
                    case 0x92:  Palette(); break;       // PALETTE      [  345]
                        break;
                }
            }
                break;
            case Z_Biff8W:      // --------------------------------- Z_Biff8W -
            {
                switch( nOpcode )
                {
                    case 0x0A:                          // EOF          [ 2345   ]
                        eAkt = Z_Biff8E;
                        break;
                    case 0x22:  Rec1904(); break;       // 1904         [ 2345   ]
                    case 0x25:  Defrowheight2(); break; // DEFAULTROWHEI[ 2      ]
                    case 0x31:  GetFontBuffer().ReadFont( maStrm );             break;
                    case 0x42:  Codepage(); break;      // CODEPAGE     [ 2345   ]
                    case 0x55:  DefColWidth(); break;
                    case 0x56:  Builtinfmtcnt(); break; // BUILTINFMTCNT[  34    ]
                    case 0x8D:  Hideobj(); break;       // HIDEOBJ      [  345   ]
                    case 0x99:  Standardwidth(); break; // STANDARDWIDTH[   45   ]
                    case 0xD3:  bHasBasic = TRUE; break;
                    case 0xDE:  Olesize(); break;
                    case 0xE0:  GetXFBuffer().ReadXF( maStrm );                 break;
                    case 0xEB:  Msodrawinggroup(); break;
                    case 0x01BA: Codename( TRUE ); break;
                    case 0x0225: Defrowheight345();break;//DEFAULTROWHEI[  345   ]
                    case 0x0293: GetXFBuffer().ReadStyle( maStrm );             break;
                    case 0x041E: GetNumFmtBuffer().ReadFormat( maStrm );        break;

                    case EXC_ID_SST:            GetSst().ReadSst( maStrm );                     break;
                    case EXC_ID_TABID:          GetTabInfo().ReadTabid( maStrm );               break;
                    case EXC_ID_NAME:           GetNameBuffer().ReadName( maStrm );             break;

                    case EXC_ID_EXTERNSHEET:    GetLinkManager().ReadExternsheet( maStrm );     break;
                    case EXC_ID_SUPBOOK:        GetLinkManager().ReadSupbook( maStrm );         break;
                    case EXC_ID_XCT:            GetLinkManager().ReadXct( maStrm );             break;
                    case EXC_ID_CRN:            GetLinkManager().ReadCrn( maStrm );             break;
                    case EXC_ID_EXTERNNAME:     GetLinkManager().ReadExternname( maStrm );      break;

                    case EXC_ID_DCONREF:        GetPivotTableManager().ReadDconref( maStrm );   break;
                    case EXC_ID_SXIDSTM:        GetPivotTableManager().ReadSxidstm( maStrm );   break;       // SXIDSTM                ##++##
                    case EXC_ID_SXVS:           GetPivotTableManager().ReadSxvs( maStrm );      break;          // SXVS                   ##++##
                }

            }
                break;
            case Z_Biff8I:      // --------------------------------- Z_Biff8I -
            case Z_Biff8T:      // --------------------------------- Z_Biff8T -
            {
                // Z_Biff8I (initial records) and Z_Biff8T (common table records) together
                BOOL bFound = FALSE;

                // 1st: read records for Z_Biff8I only
                if( (eAkt == Z_Biff8I) && !bFound )
                {
                    bFound = TRUE;
                    switch( nOpcode )
                    {
                        case 0x000C:    Calccount();            break;  // CALCCOUNT
                        case 0x0010:    Delta();                break;  // DELTA
                        case 0x0011:    Iteration();            break;  // ITERATION
                        case 0x001D:    Selection();            break;  // SELECTION    [ 2345   ]
                        case 0x0200:    Dimensions();           break;  // DIMENSIONS   [      8 ]

                        case EXC_ID_HORPAGEBREAKS:
                        case EXC_ID_VERPAGEBREAKS:  GetPageSettings().ReadPageBreaks( maStrm );     break;
                        case EXC_ID_HEADER:
                        case EXC_ID_FOOTER:         GetPageSettings().ReadHeaderFooter( maStrm );   break;
                        case EXC_ID_LEFTMARGIN:
                        case EXC_ID_RIGHTMARGIN:
                        case EXC_ID_TOPMARGIN:
                        case EXC_ID_BOTTOMMARGIN:   GetPageSettings().ReadMargin( maStrm );         break;
                        case EXC_ID_PRINTHEADERS:   GetPageSettings().ReadPrintheaders( maStrm );   break;
                        case EXC_ID_PRINTGRIDLINES: GetPageSettings().ReadPrintgridlines( maStrm ); break;
                        case EXC_ID_HCENTER:
                        case EXC_ID_VCENTER:        GetPageSettings().ReadCenter( maStrm );         break;
                        case EXC_ID_SETUP:          GetPageSettings().ReadSetup( maStrm );          break;
                        case EXC_ID_BITMAP:         GetPageSettings().ReadBitmap( maStrm );         break;

                        case 0x0809:                                    // BOF          [    5   ]
                        {
                            Bof5();
                            NeueTabelle();
                            if( pExcRoot->eDateiTyp == Biff8C )
                            {
                                if( bWithDrawLayer && GetObjectManager().IsCurrObjChart() )
                                    ReadChart8( *pProgress, FALSE );    // zunaechst Return vergessen
                                else
                                {// Stream-Teil mit Chart ueberlesen
                                    ePrev = eAkt;
                                    eAkt = Z_Biff8C;
                                }
                            }
#ifdef DBG_UTIL
                            else
                            {
                                DBG_ASSERT( pExcRoot->eDateiTyp == Biff5C,
                                    "-ImportExcel8::Read(): Sofort zu mir (GT)!" );
                            }
#endif
                        }
                        break;
                        default:        bFound = FALSE;
                    }
                }

                // 2nd: common table records, change mode to Z_Biff8T
                if( !bFound )
                {
                    bFound = TRUE;
                    switch( nOpcode )
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

                        case EXC_ID_HLINK:          XclImpHyperlink::ReadHlink( maStrm );           break;
                        case EXC_ID_LABELRANGES:    XclImpLabelranges::ReadLabelranges( maStrm );   break;

                        case EXC_ID_CONDFMT:        GetCondFormatManager().ReadCondfmt( maStrm );   break;
                        case EXC_ID_CF:             GetCondFormatManager().ReadCF( maStrm );        break;

                        case EXC_ID_DVAL:           XclImpValidation::ReadDval( maStrm );           break;
                        case EXC_ID_DV:             XclImpValidation::ReadDV( maStrm );             break;

                        case EXC_ID_QSI:            GetWebQueryBuffer().ReadQsi( maStrm );          break;
                        case EXC_ID_WQSTRING:       GetWebQueryBuffer().ReadWqstring( maStrm );     break;
                        case EXC_ID_PQRY:           GetWebQueryBuffer().ReadParamqry( maStrm );     break;
                        case EXC_ID_WQSETT:         GetWebQueryBuffer().ReadWqsettings( maStrm );   break;
                        case EXC_ID_WQTABLES:       GetWebQueryBuffer().ReadWqtables( maStrm );     break;

                        default:
                            bFound = FALSE;
                    }
                    if( bFound )
                        eAkt = Z_Biff8T;
                }

                // 3rd: read records for Z_Biff8T only
                if( (eAkt == Z_Biff8T) && !bFound )
                {
                    bFound = TRUE;
                    switch( nOpcode )
                    {
                        case 0x0007:    RecString();            break;  // STRING       [ 2345   ]
                        case 0x003C:    Cont();                 break;  // CONTINUE
                        case 0x00A0:    Scl();                  break;  // SCL          [   45   ]
                        case 0x0207:    RecString();            break;  // STRING       [ 2345   ]

                        case EXC_ID_SXVIEW:         GetPivotTableManager().ReadSxview( maStrm );    break;
                        case EXC_ID_SXVD:           GetPivotTableManager().ReadSxvd( maStrm );      break;
                        case EXC_ID_SXVI:           GetPivotTableManager().ReadSxvi( maStrm );      break;
                        case EXC_ID_SXIVD:          GetPivotTableManager().ReadSxivd( maStrm );     break;
                        case EXC_ID_SXPI:           GetPivotTableManager().ReadSxpi( maStrm );      break;
                        case EXC_ID_SXDI:           GetPivotTableManager().ReadSxdi( maStrm );      break;
                        case EXC_ID_SXVDEX:         GetPivotTableManager().ReadSxvdex( maStrm );    break;
                        case EXC_ID_SXEX:           GetPivotTableManager().ReadSxex( maStrm );      break;

                        case 0x0809:                                    // BOF          [    5   ]
                        {
                            Bof5();
                            if( pExcRoot->eDateiTyp == Biff8C && bWithDrawLayer &&
                                GetObjectManager().IsCurrObjChart() )
                                ReadChart8( *pProgress, FALSE );    // zunaechst Return vergessen
                            else
                            {
                                ePrev = eAkt;
                                eAkt = Z_Biffn0;
                            }

                            DBG_ASSERT( pExcRoot->eDateiTyp == Biff8C,
                                "-ImportExcel8::Read(): Sofort zu mir (GT)!" );
                        }
                        break;
                        default:        bFound = FALSE;
                    }
                }

                // 4th: read records for Z_Biff8I and Z_Biff8T
                if( !bFound )
                {
                    bFound = TRUE;
                    switch( nOpcode )
                    {
                        case 0x000A:                            // EOF          [ 2345   ]
                        {
                            EndSheet();
                            eAkt = Z_Biff8E;
                            IncCurrScTab();
                        }
                        break;
                        case 0x002F:                            // FILEPASS     [ 2345   ]
                        {
                            if( Filepass() )
                            {
                                eLastErr = eERR_FILEPASSWD;
                                eAkt = Z_Ende;
                            }
                        }
                        break;
                        default:    bFound = FALSE;
                    }
                }
            }
                break;
            // ----------------------------------------------------------------
            case Z_Biff8E:      // --------------------------------- Z_Biff8E -
            {
                switch( nOpcode )
                {
                    case 0x0809:                        // BOF          [    5   ]
                    {
                        if( GetCurrScTab() > MAXTAB )    // ignore tables >255
                        {
                            ePrev = eAkt;
                            eAkt = Z_Biffn0;
                            break;
                        }

                        Bof5();
                        NeueTabelle();

                        GetObjectManager().InsertDummyObj();

                        switch( pExcRoot->eDateiTyp )
                        {
                            case Biff8:
                            case Biff8M4:
                                eAkt = Z_Biff8Pre;  // Shrfmla Prefetch, Row-Prefetch
                                nBofLevel = 0;
                                aIn.StoreGlobalPosition();
                                break;
                            case Biff8C:
                                GetObjectManager().StartNewChartObj();
                                pExcRoot->bChartTab = TRUE;
                                ReadChart8( *pProgress, TRUE );
                                pExcRoot->bChartTab = FALSE;
                                EndSheet();
                                IncCurrScTab();
                                GetTracer().TraceChartOnlySheet();
                                break;
                            case Biff8V:
                            default:
                                pD->SetVisible( GetCurrScTab(), FALSE );
                                ePrev = eAkt;
                                eAkt = Z_Biffn0;
                        }
                        DBG_ASSERT( pExcRoot->eDateiTyp != Biff8W,
                            "+ImportExcel8::Read(): Doppel-Whopper-Workbook!" );
                    }
                    break;
                }

            }
                break;
            case Z_Biff8Pre:    // ------------------------------- Z_Biff8Pre -
            {
                if( nOpcode == 0x0809 )
                    nBofLevel++;
                else if( (nOpcode == 0x000A) && nBofLevel )
                    nBofLevel--;
                else if( !nBofLevel )                       // don't read chart records
                {
                    switch( nOpcode )
                    {
                        case 0x00:  Dimensions(); break;    // DIMENSIONS   [ 2345   ]
                        case 0x08:  Row25(); break;         // ROW          [ 2  5   ]
                        case 0x0A:                          // EOF          [ 2345   ]
                            eAkt = Z_Biff8I;
                            aIn.SeekGlobalPosition();         // und zurueck an alte Position
                            break;
                        case 0x12:  Protect(); break;
                        case 0x1D:  Selection(); break;     // SELECTION    [ 2345   ]
                        case 0x41:  Pane(); break;          // PANE         [ 2345   ]
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
                        case 0x023E: Window2_5(); break;    // WINDOW       [    5]
                        case 0x04BC: Shrfmla(); break;      // SHRFMLA      [    5   ]
                    }
                }
            }
                break;
            case Z_Biff8C:  // ------------------------------------- Z_Biff8C -
            {
                if( nOpcode == 0x0A )
                    eAkt = ePrev;
            }
                break;
            case Z_Biffn0:      // --------------------------------- Z_Biffn0 -
            {
                switch( nOpcode )
                {
                    case 0x0809:    nBofLevel++;            break;
                    case 0x000A:
                    {
                        if( nBofLevel )
                            nBofLevel--;
                        else
                        {
                            eAkt = ePrev;
                            IncCurrScTab();
                        }
                    }
                    break;
                }
            }
                break;
            // ----------------------------------------------------------------
            case Z_Ende:        // ----------------------------------- Z_Ende -
                DBG_ERROR( "*ImportExcel8::Read(): Not possible state!" );
                break;
            default: DBG_ERROR( "-ImportExcel8::Read(): Zustand vergessen!" );
        }
    }

    if( eLastErr == eERR_OK )
    {
        pD->CalcAfterLoad();

        pProgress.reset();

        AdjustRowHeight();
        PostDocLoad();

        // import change tracking data
        XclImpChangeTrack aImpChTr( pExcRoot );
        aImpChTr.Apply();

        if( bTabTruncated || IsTruncated() )
            eLastErr = eERR_RNGOVRFLW;
    }

    return eLastErr;
}


//___________________________________________________________________

FltError ImportExcel8::ReadChart8( ScfSimpleProgressBar& rProgress, const BOOL bOwnTab )
{
    bFirstScl   = TRUE;

    FltError    eLastErr = eERR_OK;
    BOOL        bLoop;
    UINT16      nOpcode;            // current opcode

    XclImpChart* pChart = GetObjectManager().GetCurrChartData();
    if( !pChart )
    {
        bLoop = TRUE;
        while( bLoop )
        {
            bLoop = aIn.StartNextRecord();
            if( bLoop )
                bLoop = (aIn.GetRecId() != 0x000A);
        }
        rProgress.Progress( aIn.Tell() );

        return eERR_OK;
    }

    bLoop = TRUE;
    while( bLoop )
    {
        bLoop = aIn.StartNextRecord();
        nOpcode = aIn.GetRecId();

        rProgress.Progress( aIn.Tell() );

        // page settings - only for charts in entire sheet
        if( bOwnTab ) switch( nOpcode )
        {
            case 0x0014:
            case 0x0015:    GetPageSettings().ReadHeaderFooter( maStrm );       break;
            case 0x0026:
            case 0x0027:
            case 0x0028:
            case 0x0029:    GetPageSettings().ReadMargin( maStrm );             break;
            case 0x002A:    GetPageSettings().ReadPrintheaders( maStrm );       break;
            case 0x002B:    GetPageSettings().ReadPrintgridlines( maStrm );     break;
            case 0x00A0:    ChartScl();                                         break;  // SCL
            case 0x00A1:    GetPageSettings().ReadSetup( maStrm );              break;
        }

        switch( nOpcode )
        {
            case 0x000A:    ChartEof(); bLoop = FALSE;                          break;  // EOF
            case 0x005D:    GetTracer().TraceChartEmbeddedObj();                break;  // OBJ
            case 0x0858:    pChart->ReadPivotChartTableName();                  break;  // Pivot Chart Table Name
            case 0x1002:    pChart->ReadChart();                                break;  // CHART
            case 0x1003:    pChart->ReadSeries( aIn );                          break;  // SERIES
            case 0x1006:    pChart->ReadDataformat( aIn );                      break;  // DATAFORMAT
            case 0x1007:    pChart->ReadLineformat( aIn );                      break;  // LINEFORMAT
            case 0x1009:    pChart->ReadMarkerformat( aIn );                    break;  // MARKERFORMAT
            case 0x100A:    pChart->ReadAreaformat( aIn );                      break;  // AREAFORMAT
            case 0x100B:    pChart->ReadPieformat( aIn );                       break;  // PIEFORMAT
            case 0x100C:    pChart->ReadAttachedlabel( aIn );                   break;  // ATTACHEDLABEL
            case 0x100D:    pChart->ReadSeriestext( aIn );                      break;  // SERIESTEXT
            case 0x1014:    pChart->ReadChartformat();                          break;  // CHARTFORMAT
            case 0x1015:    pChart->ReadLegend( aIn );                          break;  // LEGEND
            case 0x1017:    pChart = GetObjectManager().ReplaceChartData( aIn, ctBar );    break;  // BAR
            case 0x1018:    pChart = GetObjectManager().ReplaceChartData( aIn, ctLine );   break;  // LINE
            case 0x1019:    pChart = GetObjectManager().ReplaceChartData( aIn, ctPie );    break;  // PIE
            case 0x101A:    pChart = GetObjectManager().ReplaceChartData( aIn, ctArea );   break;  // AREA
            case 0x101B:    pChart = GetObjectManager().ReplaceChartData( aIn, ctScatter );break;  // SCATTER
            case 0x101C:    pChart->ReadChartline( aIn );                       break;  // CHARTLINE
            case 0x101D:    pChart->ReadAxis( aIn );                            break;  // AXIS
            case 0x101E:    pChart->ReadTick( aIn );                            break;  // TICK
            case 0x101F:    pChart->ReadValuerange( aIn );                      break;  // VALUERANGE
            case 0x1020:    pChart->ReadCatserrange( aIn );                     break;  // CATSERRANGE
            case 0x1021:    pChart->ReadAxislineformat( aIn );                  break;  // AXISLINEFORMAT
            case 0x1024:    pChart->ReadDefaulttext( aIn );                     break;  // DEFAULTTEXT
            case 0x1025:    pChart->ReadText( aIn );                            break;  // TEXT
            case 0x1026:    pChart->ReadFontx( aIn );                           break;  // FONTX
            case 0x1027:    pChart->ReadObjectlink( aIn );                      break;  // OBJECTLINK
            case 0x1032:    pChart->ReadFrame();                                break;  // FRAME
            case 0x1033:    pChart->ReadBegin();                                break;  // BEGIN
            case 0x1034:    pChart->ReadEnd();                                  break;  // END
            case 0x1035:    pChart->ReadPlotarea();                             break;  // PLOTAREA
            case 0x103A:    pChart->Read3D( aIn );                              break;  // 3D
            case 0x103C:    pChart->ReadPicf( aIn );                            break;  // PICF
            case 0x103D:    pChart->ReadDropbar( aIn );                         break;  // DROPBAR
            case 0x103E:    pChart = GetObjectManager().ReplaceChartData( aIn, ctNet );    break;  // RADAR
            case 0x103F:    pChart = GetObjectManager().ReplaceChartData( aIn, ctSurface );break;  // SURFACE
            case 0x1041:    pChart->ReadAxisparent( aIn );                      break;  // AXISPARENT
            case 0x1045:    pChart->ReadSertocrt( aIn );                        break;  // SERTOCRT
            case 0x1046:    pChart->ReadAxesused( aIn );                        break;  // AXESUSED
            case 0x104B:    pChart->ReadSerauxtrend();                          break;  // SERAUXTREND
            case 0x104E:    pChart->ReadIfmt( aIn );                            break;  // IFMT
            case 0x1050:    pChart->ReadAlruns();                               break;  // ALRUNS
            case 0x1051:    pChart->ReadAi( aIn, (ExcelToSc8&)*pFormConv );     break;  // AI
            case 0x105D:    pChart->ReadSerfmt( aIn );                          break;  // SERFMT
            case 0x105F:    pChart->Read3DDataformat( aIn );                    break;  // 3DDATAFORMAT
            case 0x1066:    pChart->ReadGelframe( aIn );                        break;  // GELFRAME
        }
    }

    return eLastErr;
}

//___________________________________________________________________

