/*************************************************************************
 *
 *  $RCSfile: impop.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-29 14:59:57 $
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


#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/flditem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/sizeitem.hxx>
#include <svx/ulspitem.hxx>
#include <svtools/zforlist.hxx>

#if defined( WNT ) || defined( WIN )
#include <math.h>
#else
#include <stdlib.h>
#endif

#include "cell.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "compiler.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "markdata.hxx"
#include "olinetab.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "compiler.hxx"
#include "viewopti.hxx"
#include "docoptio.hxx"
#include "scextopt.hxx"
#include "editutil.hxx"
#include "filtopt.hxx"

#include "imp_op.hxx"
#include "excimp8.hxx"
#include "xfbuff.hxx"
#include "vfbuff.hxx"
#include "fontbuff.hxx"
#include "spstring.hxx"
#include "excform.hxx"
#include "flttools.hxx"




const double ImportExcel::fExcToTwips =
    ( double ) TWIPS_PER_CHAR / 256.0;




#define EXCTOSTR( s )   String( s, eQuellChar )
#define ASSTOSTR( s )   s, eQuellChar


ImportExcel::ImportExcel( SvStream& aStream, ScDocument* pDoc ):
    ImportTyp( aStream, pDoc, RTL_TEXTENCODING_MS_1252 ),
    aColOutlineBuff( MAXCOL + 1 ), aRowOutlineBuff( MAXROW + 1 )
{
    pChart = pUsedChartFirst = pUsedChartLast = NULL;

    aColRowBuff.SetDefWidth( STD_COL_WIDTH );
    aColRowBuff.SetDefHeight( ( UINT16 ) STD_ROW_HEIGHT );
    nTab = nBdshtTab = 0;
    nIxfeIndex = 0;     // zur Sicherheit auf 0
    aExtNameBuff.SetBase( 1 );

    pPrintRanges = new _ScRangeListTabs;
    pPrintTitles = new _ScRangeListTabs;

    pStyleSheetItemSet = NULL;

    // Root-Daten fuellen - nach new's ohne Root als Parameter

    pExcRoot = new RootData;
    pExcRoot->pDoc = pDoc;
    pExcRoot->pFormTable = pDoc->GetFormatTable();
    pExcRoot->pScRangeName = pDoc->GetRangeName();
    pExcRoot->pColor = new ColorBuffer( pExcRoot );
    pExcRoot->pFontBuffer = new FontBuffer( pExcRoot );
    pExcRoot->eDefLanguage = ScGlobal::eLnge;   //LANGUAGE_SYSTEM;
    pExcRoot->aStandard.AssignAscii( "General" );
    pExcRoot->eDateiTyp = pExcRoot->eHauptDateiTyp = pExcRoot->eGlobalDateiTyp = BiffX;
    pExcRoot->pExtSheetBuff = new ExtSheetBuffer( pExcRoot );   //&aExtSheetBuff;
    pExcRoot->pTabNameBuff = new NameBuffer( pExcRoot );        //&aTabNameBuff;
    pExcRoot->pRNameBuff = new RangeNameBuffer( pExcRoot );     //&aRangeNameBuff;
    pExcRoot->pShrfmlaBuff = new ShrfmlaBuffer( pExcRoot );     //&aShrfrmlaBuff;
    pExcRoot->pExtNameBuff = new ExtNameBuff;
    pExcRoot->pAktTab = &nTab;          // nicht schoen, aber einfach
    pExcRoot->pCharset = &eQuellChar;   // dto.
    pExcRoot->pExtDocOpt = new ScExtDocOptions;
    if( pDoc->GetExtDocOptions() )
        *pExcRoot->pExtDocOpt = *pDoc->GetExtDocOptions();
    pExcRoot->pProgress = NULL;
    pExcRoot->pEdEng = NULL;
    pExcRoot->pEdEngHF = NULL;
    pExcRoot->pXF_Buffer = new XF_Buffer( pExcRoot );

    // ab Biff8
    pExcRoot->nCondRangeCnt = ( UINT32 ) -1;    // GetCondFormStyleName() starts with increment!

    aColRowBuff.Set( pExcRoot );
    aExtNameBuff.Set( pExcRoot );

    pFltTab = new FltTabelle( pExcRoot );

    pValueFormBuffer = new ValueFormBuffer( pExcRoot );

    pFormConv = new ExcelToSc( pExcRoot, aStream, nTab );

    bTabTruncated = FALSE;

    pAutoFilter = new AutoFilterBuffer;

    // options from configuration
    ScFilterOptions aFilterOpt;

    pExcRoot->fColScale = aFilterOpt.GetExcelColScale();
    if( pExcRoot->fColScale <= 0.0 )
        pExcRoot->fColScale = 1.0;

    pExcRoot->fRowScale = aFilterOpt.GetExcelRowScale();
    if( pExcRoot->fRowScale <= 0.0 )
        pExcRoot->fRowScale = 1.0;

    pExcRoot->bBreakSharedFormula = FALSE;
    pExcRoot->bChartTab = FALSE;

    // Excel-Dokument per Default auf 31.12.1899, entspricht Excel-Einstellungen mit 1.1.1900
    ScDocOptions aOpt = pD->GetDocOptions();
    aOpt.SetDate( 30, 12, 1899 );
    pD->SetDocOptions( aOpt );
    pD->GetFormatTable()->ChangeNullDate( 30, 12, 1899 );
}


ImportExcel::~ImportExcel( void )
{
    delete pPrintRanges;
    delete pPrintTitles;

    pExcRoot->pDoc->SetSrcCharSet( eQuellChar );

    delete pFormConv;
    delete pValueFormBuffer;

    delete pFltTab;

    delete pExcRoot;
}


void ImportExcel::Dimensions( void )
{
    if( nBytesLeft < 8 )        // hack for Applix-rubbish
        return;

    UINT16 nRowFirst, nRowLast, nColFirst, nColLast;

    aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

    aColRowBuff.SetDimension(
        ScRange( nColFirst, nRowFirst, nTab, nColLast, nRowLast, nTab ) );
}


void ImportExcel::Blank25( void )
{
    UINT16 nRow, nCol, nXF;

    aIn >> nRow >> nCol;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {
        aIn.SeekRel( 3 );
        nXF = 0;
    }
    else
        aIn >> nXF;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        aColRowBuff.Used( nCol, nRow );
        pFltTab->SetXF( nCol, nRow, nXF, TRUE );
    }
    else

    pLastFormCell = NULL;
}


void ImportExcel::Integer( void )
{
    UINT16 nRow, nCol, nInt;

    aIn >> nRow >> nCol;
    aIn.SeekRel( 3 );
    aIn >> nInt;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        ScValueCell* pZelle = new ScValueCell( nInt );

        pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL) TRUE );
        aColRowBuff.Used( nCol, nRow );
        pFltTab->SetXF( nCol, nRow, 0 );
    }
    else

    pLastFormCell = NULL;
}


void ImportExcel::Number25( void )
{
    UINT16 nRow, nCol, nXF;
    double fValue;

    aIn >> nRow >> nCol;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {
        aIn.SeekRel( 3 );
        nXF = 0;
    }
    else
        aIn >> nXF;

    aIn >> fValue;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        ScValueCell* pZelle = new ScValueCell( fValue );

        pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL) TRUE );
        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Label25( void )
{
    UINT16          nR, nC, nXF;
    StringInfoLen   eLen;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {// nur fuer BIFF2
        BYTE    nAttr0, nAttr1, nAttr2;

        aIn >> nR >> nC >> nAttr0 >> nAttr1 >> nAttr2;

        nXF = nAttr0 & 0x3F;
        if( nXF == 63 )
            // IXFE-record stand davor
            nXF = nIxfeIndex;

        eLen = LenByte;
    }
    else
    {
        aIn >> nR >> nC >> nXF;

        eLen = LenWord;
    }

    SetTextCell( nC, nR, ReadExcString( eLen ), nXF );

    pLastFormCell = NULL;
}


void ImportExcel::Boolerr25( void )
{
    UINT16  nRow, nCol, nXF;
    BYTE    bErrOrVal, nError;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {// nur fuer BIFF2
        BYTE nAttr0, nAttr1, nAttr2;

        aIn >> nRow >> nCol >> nAttr0 >> nAttr1 >> nAttr2;

        nXF = nAttr0 & 0x3F;
        if( nXF == 63 )
            // IXFE-record stand davor
            nXF = nIxfeIndex;
    }
    else
        aIn >> nRow >> nCol >> nXF;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        double              fVal;
        const ScTokenArray  *pErgebnis;

        aIn >> nError >> bErrOrVal;

        // Simulation ueber Formelzelle!
        pErgebnis = ErrorToFormula( bErrOrVal, nError, fVal );

        ScFormulaCell *pZelle = new ScFormulaCell(
            pD, ScAddress( nCol, nRow, nTab ), pErgebnis );

        pZelle->SetDouble( fVal );

        pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL)TRUE );

        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::RecString( void )
{
    if( pLastFormCell )
    {
        ByteString  aText;

        if( pExcRoot->eHauptDateiTyp == Biff2 )
            ReadExcString( LenByte, aText );
        else
            ReadExcString( LenWord, aText );

        pLastFormCell->SetString( EXCTOSTR( aText ) );

        pLastFormCell = NULL;
    }
}


void ImportExcel::Row25( void )
{
    UINT16  nRow, nRowHeight;

    aIn >> nRow;
    aIn.SeekRel( 4 );   // Mic und Mac ueberspringen

    if( nRow <= MAXROW )
    {
        aIn >> nRowHeight;  // direkt in Twips angegeben
        aIn.SeekRel( 2 );

        if( pExcRoot->eHauptDateiTyp == Biff2 )
        {// -------------------- BIFF2
            nRowHeight = ( UINT16 ) ( ( double ) nRowHeight * pExcRoot->fRowScale );
            aColRowBuff.SetHeight( nRow, nRowHeight );
        }
        else
        {// -------------------- BIFF5
            UINT16  nGrbit;

            aIn.SeekRel( 2 );   // reserved
            aIn >> nGrbit;

            aRowOutlineBuff.SetLevel( nRow, EXC_ROW_GETLEVEL( nGrbit ),
                TRUEBOOL( nGrbit & EXC_ROW_COLLAPSED ), TRUEBOOL( nGrbit & EXC_ROW_ZEROHEIGHT ) );

            aColRowBuff.SetRowSettings( nRow, nRowHeight, nGrbit );
        }
    }
}


void ImportExcel::Bof2( void )
{
    //POST: eDateiTyp = Biff2
    UINT16 nTyp;

    aIn.SeekRel( 2 );
    aIn >> nTyp;

    pExcRoot->eHauptDateiTyp = Biff2;

    if( nTyp == 0x0010 )        // Worksheet?
    {
        pExcRoot->eDateiTyp = Biff2;
    }
    else if( nTyp == 0x0020 )   // Chart?
    {
        pExcRoot->eDateiTyp = Biff2C;
    }
    else if( nTyp == 0x0040 )   // Macro?
    {
        pExcRoot->eDateiTyp = Biff2M;
    }
    else
    {
        pExcRoot->eDateiTyp = BiffX;
        pExcRoot->eHauptDateiTyp = BiffX;
    }
}


void ImportExcel::Eof( void )
{
    //  POST: darf nur nach einer GUELTIGEN Tabelle gerufen werden!
    EndSheet();
    nTab++;
}


BOOL ImportExcel::Password( void )
{
    // POST: return = TRUE, wenn Password <> 0
    UINT16 nPasswd;

    aIn >> nPasswd;

    return nPasswd != 0x0000;
}


void ImportExcel::Header( void )
{
    if( nBytesLeft )
        GetHF( TRUE );
}


void ImportExcel::Footer( void )
{
    if( nBytesLeft )
        GetHF( FALSE );
}


void ImportExcel::Externsheet( void )
{
    String      aFile;
    String      aTabName;
    BYTE        nLen;
    BOOL        bSameWorkBook = FALSE;

    aIn >> nLen;
    EncodeExternSheet( aIn, aFile, aTabName, nBytesLeft, bSameWorkBook, nLen, FALSE );
    ExcelNameToScName( aTabName );
    pExcRoot->pExtSheetBuff->Add( aFile, aTabName, bSameWorkBook );
}


void ImportExcel::Name25( void )
{
    const ScTokenArray* pErgebnis;
    UINT16              nLenDef;
    ByteString          _aName;
    BYTE                nLenName, nLen;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {// --------------------------------------------------- Biff2
        BYTE            nAttr0, nAttr1, nLenExpr;

        aIn >> nAttr0 >> nAttr1;
        aIn.SeekRel( 1 );
        aIn >> nLenName >> nLenExpr;
        nBytesLeft -= 5;

        pFormConv->Reset( nLenExpr );

        // Namen einlesen
        AppendString( aIn, _aName, nLenName );
        nBytesLeft -= nLenName;


        String          aName( ASSTOSTR( _aName ) );
        ExcelNameToScName( aName );

        if( nAttr0 & 0x02 )
            // function or command?
            pFormConv->GetDummy( pErgebnis );
        else    // ...oder nur Formel
        {   // Parsed expression Scannen
            long nAnzBytes = nLenExpr;  // nLenExpr = Anz. Bytes VOR Read
            pFormConv->Convert( pErgebnis, nAnzBytes, FT_RangeName );
        }

        pExcRoot->pRNameBuff->Store( aName, pErgebnis );

        aIn.SeekRel( 1 );   // cceDup ueberfluessig
        nBytesLeft--;
    }// ------------------------------------------Ende fuer Biff2
    else
    {// --------------------------------------------------- Biff5
        UINT16          nOpt;
        UINT16          nLenSeekRel = 0;
        UINT16          nSheet;
        ByteString      _aName;

        aIn >> nOpt;
        aIn.SeekRel( 1 );
        aIn >> nLenName >> nLenDef >> nSheet;
        aIn.SeekRel( 2 );
        aIn >> nLen;            // length of custom menu text
        nLenSeekRel += nLen;
        aIn >> nLen;            // length of description text
        nLenSeekRel += nLen;
        aIn >> nLen;            // length of help topic text
        nLenSeekRel += nLen;
        aIn >> nLen;            // length of status bar text
        nLenSeekRel += nLen;
        nBytesLeft -= 14;

        pFormConv->Reset( nLenDef );

        // Namen einlesen
        AppendString( aIn, _aName, nLenName );
        nBytesLeft -= nLenName;
        // jetzt steht Lesemarke an der Formel

        String          aName( ASSTOSTR( _aName ) );

        const UINT32    nFormStart = aIn.Tell();
        const BOOL      bHidden = TRUEBOOL( nOpt & EXC_NAME_HIDDEN );
        const BOOL      bBuildIn = TRUEBOOL( nOpt & EXC_NAME_BUILTIN );

        sal_Char        cFirstNameChar = aName.GetChar( 0 );
        const BOOL      bPrintArea = bBuildIn && ( cFirstNameChar == EXC_BUILTIN_PRINTAREA );
        const BOOL      bPrintTitles = bBuildIn && ( cFirstNameChar == EXC_BUILTIN_PRINTTITLES );

        if( bBuildIn )
        {// Build-in name
            aName.AssignAscii( GetExcBuiltInName( cFirstNameChar ) );
        }
        else
            ExcelNameToScName( aName );

        if( nOpt & (EXC_NAME_VB | EXC_NAME_BIG) )
            // function or command?
            pFormConv->GetDummy( pErgebnis );
        else if( bBuildIn )
        {
            if( bPrintArea )
            {// Druckbereich
                long    nAnzBytes = nLenDef;
                pFormConv->Convert( *pPrintRanges, nAnzBytes, FT_RangeName );
            }
            else if( bPrintTitles )
            {// Druckbereich
                long    nAnzBytes = nLenDef;
                pFormConv->Convert( *pPrintTitles, nAnzBytes, FT_RangeName );
            }
            aIn.Seek( nFormStart );

            long        nAnzBytes = nLenDef;
            pFormConv->Convert( pErgebnis, nAnzBytes, FT_RangeName );
        }
        else    // ...oder nur Formel
            // Parsed expression Scannen
        {
            long        nAnzBytes = nLenDef;
            pFormConv->Convert( pErgebnis, nAnzBytes, FT_RangeName );
        }

        if( bHidden )
            pExcRoot->pRNameBuff->Store( aName, NULL, nSheet );
        else
            // ohne hidden
            pExcRoot->pRNameBuff->Store( aName, pErgebnis, nSheet, bPrintArea );
    }// ----------------------------------------- Ende fuer Biff5
}


void ImportExcel::Verticalpagebreaks( void )
{
    UINT16      n;
    UINT16      nCol;

    aIn >> n;

    while( n )
    {
        aIn >> nCol;

        aColRowBuff.SetVertPagebreak( nCol );

        n--;
    }
}


void ImportExcel::Horizontalpagebreaks( void )
{
    UINT16      n;
    UINT16      nRow;

    aIn >> n;

    while( n )
    {
        aIn >> nRow;

        aColRowBuff.SetHorizPagebreak( nRow );

        n--;
    }
}


void ImportExcel::Note( void )
{
    UINT16      nCol, nRow;
    ByteString  aNote;

    aIn >> nRow >> nCol;

    if( nRow <= MAXROW && nCol <= MAXCOL )
        pD->SetNote( nCol, nRow, nTab, ScPostIt( EXCTOSTR( ReadExcString( LenWord ) ) ) );
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Selection( void )
{
    UINT16  nNumRefs, nFirstRow, nLastRow;
    UINT8   nFirstCol, nLastCol;

    aIn.SeekRel( 7 );

    aIn >> nNumRefs;

    if( nNumRefs )
    {
        nNumRefs--;

        if( nNumRefs )
            aIn.SeekRel( nNumRefs * 6 );    // nur letzte Selektion interessiert

        aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;

        aColRowBuff.SetSelection( ScRange( ( UINT16 ) nFirstCol, nFirstRow, nTab,
                                            ( UINT16 ) nLastCol, nLastRow, nTab ) );
    }
}


void ImportExcel::Format235( void )
{
    ByteString  aFormString;

    if( pExcRoot->eHauptDateiTyp == Biff5 )
        aIn.SeekRel( 2 );

    pValueFormBuffer->NewValueFormat( EXCTOSTR( ReadExcString( LenByte ) ) );
}


void ImportExcel::Formatcount( void )
{
}


void ImportExcel::Columndefault( void )
{// Default Cell Attributes
    UINT16  nColMic, nColMac;
    BYTE    nOpt0;

    aIn >> nColMic >> nColMac;

    DBG_ASSERT( nBytesLeft - 4 == ( INT32 ) ( nColMac - nColMic ) * 3,
                "+ImportExcel::Columndefault(): Wie jetzt?!!..." );

    nColMac--;

    if( nColMac > MAXCOL )
        nColMac = MAXCOL;

    for( UINT16 nCol = nColMic ; nCol <= nColMac ; nCol++ )
    {
        aIn >> nOpt0;
        aIn.SeekRel( 2 );   // nur 0. Attribut-Byte benutzt

        if( nOpt0 & 0x80 )  // Col hidden?
            aColRowBuff.HideCol( nCol );
    }
}


void ImportExcel::Array25( void )
{
    UINT16      nFirstRow, nLastRow, nFormLen;
    BYTE        nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {//                     BIFF2
        BYTE    nDummy;
        aIn.SeekRel( 1 );
        aIn >> nDummy;

        nFormLen = nDummy;
    }
    else
    {//                     BIFF5
        aIn.SeekRel( 6 );
        aIn >> nFormLen;
    }

    if( nLastRow <= MAXROW && nLastCol <= MAXCOL )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;

        pFormConv->Reset( nFormLen, ScAddress( nFirstCol, nFirstRow, nTab ) );

        long                nAnzBytes = nFormLen;

        pFormConv->Convert( pErgebnis, nAnzBytes );
        nBytesLeft += nAnzBytes - nFormLen;

        DBG_ASSERT( pErgebnis, "*ImportExcel::Array25(): ScTokenArray ist NULL!" );

        ScMarkData          aMarkData;
        aMarkData.SelectOneTable( nTab );
        pD->InsertMatrixFormula(
            nFirstCol, nFirstRow, nLastCol, nLastRow, aMarkData,
            EMPTY_STRING, pErgebnis );

        UINT16              nRowCnt, nColCnt;

        for( nColCnt = nFirstCol + 1 ; nColCnt <= nLastCol ; nColCnt++ )
            for( nRowCnt = nFirstRow ; nRowCnt <= nLastRow ; nRowCnt++ )
            {
                pFltTab->SetXF( nColCnt, nRowCnt, nLastXF );
                aColRowBuff.Used( nColCnt, nRowCnt );
            }
    }
}


void ImportExcel::Rec1904( void )
{
    UINT16  n1904;

    aIn >> n1904;

    if( n1904 )
    {// 1904 date system
        ScDocOptions aOpt = pD->GetDocOptions();
        aOpt.SetDate( 1, 1, 1904 );
        pD->SetDocOptions( aOpt );
        pD->GetFormatTable()->ChangeNullDate( 1, 1, 1904 );
    }
}


void ImportExcel::Externname25( void )
{
    UINT32      nRes;
    UINT16      nOpt;
    UINT16      nLenExpr = 0;

    aIn >> nOpt >> nRes;

    String      aName( ASSTOSTR( ReadExcString( LenByte ) ) );

    if( ( nOpt & 0x0001 ) || ( ( nOpt & 0xFFFE ) == 0x0000 ) )
    {// external name
        ExcelNameToScName( aName );
        pExcRoot->pExtNameBuff->AddName( aName );
    }
    else if( nOpt & 0x0010 )
    {// ole link
        pExcRoot->pExtNameBuff->AddOLE( aName, nRes );      // nRes is storage ID
    }
    else
    {// dde link
        pExcRoot->pExtNameBuff->AddDDE( aName );
    }
}


void ImportExcel::Colwidth( void )
{// Column Width
    BYTE    nColFirst, nColLast;
    UINT16  nColWidth;

    aIn >> nColFirst >> nColLast >> nColWidth;

    if( nColLast > MAXCOL )
        nColLast = MAXCOL;

    aColRowBuff.SetWidthRange( nColFirst, nColLast, CalcColWidth( nColWidth ) );
}


void ImportExcel::Defrowheight2( void )
{
    UINT16  nDef;

    aIn >> nDef;

    nDef &=0x7FFF;
    nDef = ( UINT16 ) ( ( double ) nDef * pExcRoot->fRowScale );

    aColRowBuff.SetDefHeight( nDef );
}


void ImportExcel::Leftmargin( void )
{
    GetAndSetMargin( IMPEXC_MARGINSIDE_LEFT );
}


void ImportExcel::Rightmargin( void )
{
    GetAndSetMargin( IMPEXC_MARGINSIDE_RIGHT );
}


void ImportExcel::Topmargin( void )
{
    GetAndSetMargin( IMPEXC_MARGINSIDE_TOP );
}


void ImportExcel::Bottommargin( void )
{
    GetAndSetMargin( IMPEXC_MARGINSIDE_BOTTOM );
}


void ImportExcel::Printheaders( void )
{
    UINT16  nPrintHeaders;

    aIn >> nPrintHeaders;

    if( nPrintHeaders == 1 )
        pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_HEADERS, TRUE ) );
    else
        pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_HEADERS, FALSE ) );
}


void ImportExcel::Prntgrdlns( void )
{
    UINT16  nPrintGrid;

    aIn >> nPrintGrid;

    if( nPrintGrid == 1 )
        pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_GRID, TRUE ) );
    else
        pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_GRID, FALSE ) );
}


BOOL ImportExcel::Filepass( void )
{
    // POST: return = TRUE, wenn Password <> 0
    UINT32  nPasswd;

    aIn >> nPasswd;

    return nPasswd != 0x00000000;
}


void ImportExcel::Font25( void )
{
    UINT16      nHeight, nIndexCol;
    BYTE        nAttr0;
    ByteString  aName;
    FontBuffer* pFB = pExcRoot->pFontBuffer;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {// Biff2
        aIn >> nHeight >> nAttr0;
        aIn.SeekRel( 1 );
        nIndexCol =  32767;

        pFB->NewFont( nHeight, nAttr0, nIndexCol, EXCTOSTR( ReadExcString( LenByte ) ) );
    }
    else
    {// Biff5
        BYTE    nUnder, nFam, nChar;
        UINT16  nWeight, nScript;

        aIn >> nHeight >> nAttr0;
        aIn.SeekRel( 1 );
        aIn >> nIndexCol >> nWeight >> nScript >> nUnder >> nFam >> nChar;
        aIn.SeekRel( 1 );   // Reserved

        pFB->NewFont( nHeight, nAttr0, nScript, nUnder, nIndexCol, nWeight, nFam, nChar,
                        EXCTOSTR( ReadExcString( LenByte ) ) );
    }
}


void ImportExcel::Pane( void )
{
    aColRowBuff.ReadSplit( aIn );
}


void ImportExcel::Codepage( void )
{
    UINT16  nPage;

    aIn >> nPage;

    switch( nPage )
    {
        case 0x01B5:    // IBM PC 437 (Multiplan)
            eQuellChar = RTL_TEXTENCODING_IBM_437;
            break;
        case 0x0352:    // Herkunft ?
            eQuellChar = RTL_TEXTENCODING_IBM_850;
            break;
        case 0x8000:    // Apple Macintosh
        case 0x2710:    // ???????????????????????????????????????????????
            eQuellChar = RTL_TEXTENCODING_APPLE_ROMAN;
            break;
        case 0x04E4:    // ANSI (Windows ) Biff4+5
        case 0x8001:    // ANSI (Windows ) Biff2+3
            eQuellChar = RTL_TEXTENCODING_MS_1252;
            break;
    }
}


void ImportExcel::XF2( void )
{
    BYTE        nDummyFormat, nDummyFont, nAttr;
    UINT16      nIndexFormat, nIndexFont;
    CellBorder  aBorder;
    XF_Data*    pXFD = new XF_Data;

    pXFD->SetCellXF();      // keine Style-XFs in Biff2

    aIn >> nDummyFont;
    aIn.SeekRel( 1 );
    aIn >> nDummyFormat;
    aIn >> nAttr;

    nIndexFormat = nDummyFormat &  0x3F;
    nIndexFont = nDummyFont;

    if( nAttr & 0x08 )
        aBorder.nLeftLine = 1;  // = durchgezogen, duenn
    else
        aBorder.nLeftLine = 0;  // = keine Linie

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
        aBorder.nBottomColor = 8;   // = schwarz?

    pXFD->SetBorder( aBorder );
    pXFD->SetFont( nIndexFont );
    pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nIndexFormat ) );
    pXFD->SetAlign( ( ExcHorizAlign ) ( nAttr & 0x07 ) );
    pXFD->SetAlign( EVA_Top );
    pXFD->SetTextOrient( ETO_NoRot );
    pXFD->SetLocked( TRUEBOOL( nDummyFormat & 0x40 ) );
    pXFD->SetHidden( TRUEBOOL( nDummyFormat & 0x80 ) );

    pExcRoot->pXF_Buffer->NewXF( pXFD );
}


void ImportExcel::Ixfe( void )
{
    aIn >> nIxfeIndex;
}


void ImportExcel::DefColWidth( void )
{
    UINT16  nWidth;
    aIn >> nWidth;

    aColRowBuff.SetDefWidth( CalcColWidth( (UINT16) ( (double)nWidth * 292.5 ) ) );
}


void ImportExcel::Builtinfmtcnt( void )
{
}


void ImportExcel::Colinfo( void )
{// Column Formatting Information
    UINT16  nColFirst, nColLast, nColWidth, nXF;
    UINT16  nOpt;

    aIn >> nColFirst >> nColLast >> nColWidth >> nXF >> nOpt;

    if( nColFirst > MAXCOL )
        return;

    if( nColLast > MAXCOL )
        nColLast = MAXCOL;

    aColOutlineBuff.SetLevelRange( nColFirst, nColLast, EXC_COL_GETLEVEL( nOpt ),
        TRUEBOOL( nOpt & EXC_COL_COLLAPSED ), TRUEBOOL( nOpt & EXC_COL_HIDDEN ) );

    if( nOpt & EXC_COL_HIDDEN ) // Cols hidden?
        aColRowBuff.HideColRange( nColFirst, nColLast );

    aColRowBuff.SetWidthRange( nColFirst, nColLast, CalcColWidth( nColWidth ) );

    aColRowBuff.SetDefaultXF( nColFirst, nColLast, nXF );
}


void ImportExcel::Rk( void )
{// Cell Value, RK Number
    UINT16              nRow, nCol, nXF;
    UINT32              nRkNum;

    aIn >> nRow >> nCol >> nXF >> nRkNum;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        ScValueCell*    pZelle = new ScValueCell( RkToDouble( nRkNum ) );

        pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL)TRUE );
        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Wsbool( void )
{
    UINT16 nFlags;
    aIn >> nFlags;

    aRowOutlineBuff.SetButtonMode( TRUEBOOL( nFlags & 0x0040 ) );
    aColOutlineBuff.SetButtonMode( TRUEBOOL( nFlags & 0x0080 ) );
}


void ImportExcel::Gridset( void )
{
}


void ImportExcel::Hcenter( void )
{
    DBG_ASSERT( pStyleSheetItemSet, "-ImportExcel::Hcenter(): kein StyleSheet - Schiet!" );
    UINT16  nCenter;
    aIn >> nCenter;
    if( nCenter == 1 )
        pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_HORCENTER, TRUE ) );
}


void ImportExcel::Vcenter( void )
{
    DBG_ASSERT( pStyleSheetItemSet, "-ImportExcel::Vcenter(): kein StyleSheet - Schiet!" );
    UINT16  nCenter;
    aIn >> nCenter;
    if( nCenter == 1 )
        pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_VERCENTER, TRUE )  );
}


void ImportExcel::Boundsheet( void )
{
    UINT16      nGrbit;

    if( pExcRoot->eHauptDateiTyp == Biff5 )
    {
        aIn.SeekRel( 4 );
        aIn >> nGrbit;
    }
    else
        nGrbit = 0x0000;

    String      aName( ASSTOSTR( ReadExcString( LenByte ) ) );

    ExcelNameToScName( aName );

    *pExcRoot->pTabNameBuff << aName;

    if( nBdshtTab > 0 )
    {
        DBG_ASSERT( !pD->HasTable( nBdshtTab ), "*ImportExcel::Boundsheet(): Tabelle schon vorhanden!" );

        pD->MakeTable( nBdshtTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nBdshtTab, FALSE );

    pD->RenameTab( nBdshtTab, aName );
    nBdshtTab++;
}


void ImportExcel::Country( void )
{
    UINT16  nCountry, nWinIni;

    aIn >> nCountry >> nWinIni;

    switch( nWinIni )
    {
        case 1:     // USA
            pExcRoot->eDefLanguage = LANGUAGE_ENGLISH_US;
            break;
        case 2:     // Canada
            pExcRoot->eDefLanguage = LANGUAGE_ENGLISH_CAN;
            break;
        case 3:     // Latin America, except Brazil
            pExcRoot->eDefLanguage = LANGUAGE_SPANISH;
            break;
        case 31:    // Netherlands
            pExcRoot->eDefLanguage = LANGUAGE_DUTCH;
            break;
        case 32:    // Belgium
            pExcRoot->eDefLanguage = LANGUAGE_DUTCH_BELGIAN;
            break;
        case 33:    // France
            pExcRoot->eDefLanguage = LANGUAGE_FRENCH;
            break;
        case 34:    // Spain
            pExcRoot->eDefLanguage = LANGUAGE_SPANISH;
            break;
        case 39:    // Italy
            pExcRoot->eDefLanguage = LANGUAGE_ITALIAN;
            break;
        case 41:    // Switzerland
            pExcRoot->eDefLanguage = LANGUAGE_GERMAN_SWISS;
            break;
        case 43:    // Austria
            pExcRoot->eDefLanguage = LANGUAGE_GERMAN_AUSTRIAN;
            break;
        case 44:    // United Kingdom
            pExcRoot->eDefLanguage = LANGUAGE_ENGLISH_UK;
            break;
        case 45:    // Denmark
            pExcRoot->eDefLanguage = LANGUAGE_DANISH;
            break;
        case 46:    // Sweden
            pExcRoot->eDefLanguage = LANGUAGE_SWEDISH;
            break;
        case 47:    // Norway
            pExcRoot->eDefLanguage = LANGUAGE_NORWEGIAN;
            break;
        case 49:    // Germany
            pExcRoot->eDefLanguage = LANGUAGE_GERMAN;
            break;
        case 52:    // Mexico
            pExcRoot->eDefLanguage = LANGUAGE_SPANISH_MEXICAN;
            break;
        case 55:    // Brazil
            pExcRoot->eDefLanguage = LANGUAGE_PORTUGUESE_BRAZILIAN;
            break;
        case 61:    // Australia
            pExcRoot->eDefLanguage = LANGUAGE_ENGLISH_AUS;
            break;
        case 64:    // New Zealand
            pExcRoot->eDefLanguage = LANGUAGE_ENGLISH_NZ;
            break;
        case 81:    // Japan
            pExcRoot->eDefLanguage = LANGUAGE_JAPANESE;
            break;
        case 82:    // South Korea
            pExcRoot->eDefLanguage = LANGUAGE_KOREAN;
            break;
        case 351:   // Portugal
            pExcRoot->eDefLanguage = LANGUAGE_PORTUGUESE;
            break;
        case 354:   // Iceland
            pExcRoot->eDefLanguage = LANGUAGE_ICELANDIC;
            break;
        case 358:   // Finland
            pExcRoot->eDefLanguage = LANGUAGE_FINNISH;
            break;
        case 785:   // Arabic
            pExcRoot->eDefLanguage = LANGUAGE_ARABIC;
            break;
        case 886:   // Republic of China
            pExcRoot->eDefLanguage = LANGUAGE_CHINESE;
            break;
        case 972:   // Israel
            pExcRoot->eDefLanguage = LANGUAGE_HEBREW;
            break;
    }
}


void ImportExcel::Hideobj( void )
{
    UINT16      nHide;
    ScVObjMode  eOle, eChart, eDraw;

    aIn >> nHide;

    ScViewOptions aOpts( pD->GetViewOptions() );

    switch( nHide )
    {
        case 1:                         // Placeholders
            eOle   = VOBJ_MODE_SHOW;    // in Excel 97 werden nur Charts als Platzhalter angezeigt
            eChart = VOBJ_MODE_DUMMY;
            eDraw  = VOBJ_MODE_SHOW;
            break;
        case 2:                         // Hide all
            eOle   = VOBJ_MODE_HIDE;
            eChart = VOBJ_MODE_HIDE;
            eDraw  = VOBJ_MODE_HIDE;
            break;
        default:                        // Show all
            eOle   = VOBJ_MODE_SHOW;
            eChart = VOBJ_MODE_SHOW;
            eDraw  = VOBJ_MODE_SHOW;
            break;
    }

    aOpts.SetObjMode( VOBJ_TYPE_OLE,   eOle );
    aOpts.SetObjMode( VOBJ_TYPE_CHART, eChart );
    aOpts.SetObjMode( VOBJ_TYPE_DRAW,  eDraw );

    pD->SetViewOptions( aOpts );
}


void ImportExcel::Bundleheader( void )
{
}


void ImportExcel::Palette( void )
{
    UINT16  nAnz;
    BYTE    nRed, nGreen, nBlue, nDummy;

    aIn >> nAnz;

#ifdef DEB_UTIL
    nBytesLeft -= 2;
#endif

    for( UINT16 nC = 0 ; nC < nAnz ; nC++ )
    {
        aIn >> nRed >> nGreen >> nBlue >> nDummy;
        pExcRoot->pColor->NewColor( nRed, nGreen, nBlue );

#ifdef DEB_UTIL
        nBytesLeft -= 4;
#endif
    }

#ifdef DEB_UTIL
    DBG_ASSERT( nBytesLeft >= 0, "*ImportExcel::Palette(): Ups, das war zuviel!" );
    DBG_ASSERT( nBytesLeft <= 0, "*ImportExcel::Palette(): Ups, da ha'mer jemanden vergessen!" );
#endif
}


void ImportExcel::Standardwidth( void )
{
    UINT16  nWidth;
    aIn >> nWidth;

    aColRowBuff.SetDefWidth( CalcColWidth( nWidth ), TRUE );
}


void ImportExcel::Scl( void )
{
    UINT16  nNscl, nDscl;

    aIn >> nNscl >> nDscl;

    pExcRoot->pExtDocOpt->SetZoom( nNscl, nDscl );
}


void ImportExcel::Setup( void )
{
    struct PAPER_SIZE
    {
        INT32       nH;
        INT32       nW;
    };
    static const    nAnzSizes = 42;
    static SvxPaper pSvxPS[ nAnzSizes ] =
    {
        SVX_PAPER_USER,     // undefined
        SVX_PAPER_LETTER,   // Letter 8 1/2 x 11 in
        SVX_PAPER_USER,     // Letter Small 8 1/2 x 11 in
        SVX_PAPER_TABLOID,  // Tabloid 11 x 17 in
        SVX_PAPER_USER,     // Ledger 17 x 11 in
        SVX_PAPER_LEGAL,    // Legal 8 1/2 x 14 in
        SVX_PAPER_USER,     // Statement 5 1/2 x 8 1/2 in
        SVX_PAPER_USER,     // Executive 7 1/4 x 10 1/2 in
        SVX_PAPER_A3,       // A3 297 x 420 mm
        SVX_PAPER_A4,       // A4 210 x 297 mm
        SVX_PAPER_USER,     // A4 Small 210 x 297 mm
        SVX_PAPER_A5,       // A5 148 x 210 mm
        SVX_PAPER_B4,       // B4 250 x 354
        SVX_PAPER_B5,       // B5 182 x 257 mm
        SVX_PAPER_USER,     // Folio 8 1/2 x 13 in
        SVX_PAPER_USER,     // Quarto 215 x 275 mm
        SVX_PAPER_USER,     // 10x14 in
        SVX_PAPER_USER,     // 11x17 in
        SVX_PAPER_USER,     // Note 8 1/2 x 11 in
        SVX_PAPER_USER,     // Envelope #9 3 7/8 x 8 7/8
        SVX_PAPER_USER,     // Envelope #10 4 1/8 x 9 1/2
        SVX_PAPER_USER,     // Envelope #11 4 1/2 x 10 3/8
        SVX_PAPER_USER,     // Envelope #12 4 \276 x 11
        SVX_PAPER_USER,     // Envelope #14 5 x 11 1/2
        SVX_PAPER_A4,       // C size sheet
        SVX_PAPER_A4,       // D size sheet
        SVX_PAPER_A4,       // E size sheet
        SVX_PAPER_DL,       // Envelope DL 110 x 220mm
        SVX_PAPER_C5,       // Envelope C5 162 x 229 mm
        SVX_PAPER_USER,     // Envelope C3  324 x 458 mm
        SVX_PAPER_C4,       // Envelope C4  229 x 324 mm
        SVX_PAPER_C6,       // Envelope C6  114 x 162 mm
        SVX_PAPER_C65,      // Envelope C65 114 x 229 mm
        SVX_PAPER_B4,       // Envelope B4  250 x 353 mm
        SVX_PAPER_B5,       // Envelope B5  176 x 250 mm
        SVX_PAPER_B6,       // Envelope B6  176 x 125 mm
        SVX_PAPER_USER,     // Envelope 110 x 230 mm
        SVX_PAPER_USER,     // Envelope Monarch 3.875 x 7.5 in
        SVX_PAPER_USER,     // 6 3/4 Envelope 3 5/8 x 6 1/2 in
        SVX_PAPER_USER,     // US Std Fanfold 14 7/8 x 11 in
        SVX_PAPER_USER,     // German Std Fanfold 8 1/2 x 12 in
        SVX_PAPER_USER      // German Legal Fanfold 8 1/2 x 13 in
    };
    static const    PAPER_SIZE pPS[ nAnzSizes ] =
    {
    {(INT32)(0),(INT32)(0)},                                        // undefined
    {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*11)},       // Letter 8 1/2 x 11 in
    {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*11)},       // Letter Small 8 1/2 x 11 in
    {(INT32)(TWIPS_PER_INCH*11),(INT32)(TWIPS_PER_INCH*17)},        // Tabloid 11 x 17 in
    {(INT32)(TWIPS_PER_INCH*17),(INT32)(TWIPS_PER_INCH*11)},        // Ledger 17 x 11 in
    {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*14)},       // Legal 8 1/2 x 14 in
    {(INT32)(TWIPS_PER_INCH*5.5),(INT32)(TWIPS_PER_INCH*8.5)},      // Statement 5 1/2 x 8 1/2 in
    {(INT32)(TWIPS_PER_INCH*7.25),(INT32)(TWIPS_PER_INCH*10.5)},    // Executive 7 1/4 x 10 1/2 in
    {(INT32)(TWIPS_PER_CM*29.7),(INT32)(TWIPS_PER_CM*42.0)},        // A3 297 x 420 mm
    {(INT32)(TWIPS_PER_CM*21.0),(INT32)(TWIPS_PER_CM*29.7)},        // A4 210 x 297 mm
    {(INT32)(TWIPS_PER_CM*21.0),(INT32)(TWIPS_PER_CM*29.7)},        // A4 Small 210 x 297 mm
    {(INT32)(TWIPS_PER_CM*14.8),(INT32)(TWIPS_PER_CM*21.0)},        // A5 148 x 210 mm
    {(INT32)(TWIPS_PER_CM*25.0),(INT32)(TWIPS_PER_CM*35.4)},        // B4 250 x 354
    {(INT32)(TWIPS_PER_CM*18.2),(INT32)(TWIPS_PER_CM*25.7)},        // B5 182 x 257 mm
    {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*13)},       // Folio 8 1/2 x 13 in
    {(INT32)(TWIPS_PER_CM*21.5),(INT32)(TWIPS_PER_CM*27.5)},        // Quarto 215 x 275 mm
    {(INT32)(TWIPS_PER_INCH*10),(INT32)(TWIPS_PER_INCH*14)},        // 10x14 in
    {(INT32)(TWIPS_PER_INCH*11),(INT32)(TWIPS_PER_INCH*17)},        // 11x17 in
    {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*11)},       // Note 8 1/2 x 11 in
    {(INT32)(TWIPS_PER_INCH*3.875),(INT32)(TWIPS_PER_INCH*8.875)},  // Envelope #9 3 7/8 x 8 7/8
    {(INT32)(TWIPS_PER_INCH*4.125),(INT32)(TWIPS_PER_INCH*9.5)},    // Envelope #10 4 1/8 x 9 1/2
    {(INT32)(TWIPS_PER_INCH*4.5),(INT32)(TWIPS_PER_INCH*10.375)},   // Envelope #11 4 1/2 x 10 3/8
    {(INT32)(TWIPS_PER_INCH*4.03),(INT32)(TWIPS_PER_INCH*11)},      // Envelope #12 4 \276 x 11
    {(INT32)(TWIPS_PER_INCH*14.5),(INT32)(TWIPS_PER_INCH*11.5)},    // Envelope #14 5 x 11 1/2
    {(INT32)(0),(INT32)(0)},                                        // C size sheet
    {(INT32)(0),(INT32)(0)},                                        // D size sheet
    {(INT32)(0),(INT32)(0)},                                        // E size sheet
    {(INT32)(TWIPS_PER_CM*11.0),(INT32)(TWIPS_PER_CM*22.0)},        // Envelope DL 110 x 220mm
    {(INT32)(TWIPS_PER_CM*16.2),(INT32)(TWIPS_PER_CM*22.9)},        // Envelope C5 162 x 229 mm
    {(INT32)(TWIPS_PER_CM*32.4),(INT32)(TWIPS_PER_CM*45.8)},        // Envelope C3  324 x 458 mm
    {(INT32)(TWIPS_PER_CM*22.9),(INT32)(TWIPS_PER_CM*32.4)},        // Envelope C4  229 x 324 mm
    {(INT32)(TWIPS_PER_CM*11.4),(INT32)(TWIPS_PER_CM*16.2)},        // Envelope C6  114 x 162 mm
    {(INT32)(TWIPS_PER_CM*11.4),(INT32)(TWIPS_PER_CM*22.9)},        // Envelope C65 114 x 229 mm
    {(INT32)(TWIPS_PER_CM*25.0),(INT32)(TWIPS_PER_CM*35.3)},        // Envelope B4  250 x 353 mm
    {(INT32)(TWIPS_PER_CM*17.6),(INT32)(TWIPS_PER_CM*25.0)},        // Envelope B5  176 x 250 mm
    {(INT32)(TWIPS_PER_CM*17.6),(INT32)(TWIPS_PER_CM*12.5)},        // Envelope B6  176 x 125 mm
    {(INT32)(TWIPS_PER_CM*11.0),(INT32)(TWIPS_PER_CM*23.0)},        // Envelope 110 x 230 mm
    {(INT32)(TWIPS_PER_INCH*3.875),(INT32)(TWIPS_PER_INCH*7.5)},    // Envelope Monarch 3.875 x 7.5 in
    {(INT32)(TWIPS_PER_INCH*3.625),(INT32)(TWIPS_PER_INCH*6.5)},    // 6 3/4 Envelope 3 5/8 x 6 1/2 in
    {(INT32)(TWIPS_PER_INCH*14.875),(INT32)(TWIPS_PER_INCH*11)},    // US Std Fanfold 14 7/8 x 11 in
    {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*12)},       // German Std Fanfold 8 1/2 x 12 in
    {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*13)}        // German Legal Fanfold 8 1/2 x 13 in
    };


    UINT16          nPaperSize, nScale, nStartPage, nFitWidth, nFitHeight, nOpt;

    aIn >> nPaperSize >> nScale >> nStartPage >> nFitWidth >> nFitHeight >> nOpt;

    SvxPageItem     aPageItem( ( const SvxPageItem& ) pStyleSheetItemSet->Get( ATTR_PAGE ) );

    pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_TOPDOWN, !( nOpt & 0x0001 ) ) );

    if( !( nOpt & 0x0004 ) )
    {// fNoPls      (TRUE->ignore nPaperSize, nScale, nPrintRes,
        //                  nVertRes, nCopies, fLandscape)
        pExcRoot->bDefaultPage = FALSE;
        BOOL    bLandscape = !TRUEBOOL( nOpt & 0x0002 );
        aPageItem.SetLandscape( bLandscape );

        Size        aSize;
        if( nPaperSize >= nAnzSizes )
            aSize = Size( 0, 0 );
        else if( pSvxPS[ nPaperSize ] == SVX_PAPER_USER )
            aSize = Size( pPS[ nPaperSize ].nH, pPS[ nPaperSize ].nW );
        else
            aSize = SvxPaperInfo::GetPaperSize( pSvxPS[ nPaperSize ] );

        if( bLandscape )
        {
            long    nTmp;
            nTmp = aSize.nA;
            aSize.nA = aSize.nB;
            aSize.nB = nTmp;
        }

        pStyleSheetItemSet->Put( SvxSizeItem( ATTR_PAGE_SIZE, aSize ) );

        pStyleSheetItemSet->Put( SfxUInt16Item( ATTR_PAGE_SCALE, nScale ) );
    }

    if( nOpt & 0x0020 )
        // fNotes
        pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_NOTES, TRUE ) );

    if( nOpt & 0x0080 )
        // fUsePage
        pStyleSheetItemSet->Put(
            SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, nStartPage ) );

    pStyleSheetItemSet->Put( aPageItem );
}


void ImportExcel::Shrfmla( void )
{
    UINT16              nFirstRow, nLastRow, nLenExpr;
    BYTE                nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
    aIn.SeekRel( 2 );
    aIn >> nLenExpr;

    // jetzt steht Lesemarke an der Formel

    const ScTokenArray* pErgebnis;

    pFormConv->Reset( nLenExpr );

    long                nAnzBytes = nLenExpr;
    pFormConv->Convert( pErgebnis, nAnzBytes, FT_SharedFormula );


    DBG_ASSERT( pErgebnis, "+ImportExcel::Shrfmla(): ScTokenArray ist NULL!" );

    pExcRoot->pShrfmlaBuff->Store(
            ScRange(
                ScAddress( nFirstCol, nFirstRow, nTab ),
                ScAddress( nLastCol, nLastRow, nTab ) ),
            *pErgebnis );

    pLastFormCell = NULL;
}


void ImportExcel::Mulrk( void )
{
    UINT16  nRow, nColFirst, nXF;
    UINT32  nRkNum;

    aIn >> nRow >> nColFirst;
    nBytesLeft -= 4;

    if( nRow <= MAXROW )
    {
        for( UINT16 nCol = nColFirst ; nBytesLeft > 2 ; nCol++ )
        {
            aIn >> nXF >> nRkNum;
            nBytesLeft -= 6;

            if( nCol <= MAXCOL )
            {
                ScValueCell* pZelle = new ScValueCell( RkToDouble( nRkNum ) );

                pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL)TRUE );
                aColRowBuff.Used( nCol, nRow );
                pFltTab->SetXF( nCol, nRow, nXF );
            }
        }
        DBG_ASSERT( nBytesLeft == 2, "+ImportExcel::Mulrk(): Was'n das?!!!" );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Mulblank( void )
{
    UINT16  nRow, nCol, nColFirst, nXF;

    aIn >> nRow >> nColFirst;
    nBytesLeft -= 4;

    if( nRow <= MAXROW )
    {
        for( nCol = nColFirst ; nBytesLeft > 2 ; nCol++ )
        {
            aIn >> nXF;
            nBytesLeft -= 2;

            if( nCol <= MAXCOL )
            {
                aColRowBuff.Used( nCol, nRow );
                pFltTab->SetXF( nCol, nRow, nXF, TRUE );
            }
        }
        aIn >> nRow;    // nRow zum Testen von letzter Col missbraucht
        nBytesLeft -= 2;
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Rstring( void )
{
    UINT16      nRow, nCol, nXF;
    BYTE        nCount;
    ByteString  aString;

    aIn >> nRow >> nCol >> nXF;

    ReadExcString( LenWord, aString );

    aIn >> nCount;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        if( aString.Len() )
        {
            EditTextObject* pTextObj = CreateFormText( nCount, EXCTOSTR( aString ), nXF );

            ScBaseCell*     pZelle = new ScEditCell( pTextObj, pD, GetEdEng().GetEditTextObjectPool() );

            delete pTextObj;

            pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL)TRUE );
        }

        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::XF5( void )
{
    UINT16      nAttr0, nAlign, nIndexFormat, nIndexFont, nFillCol,
                nFill_Bottom, nBorder0, nBorder1;
    CellBorder  aBorder;
    CellFill    aFill;
    XF_Data*    pXFD = new XF_Data;

    aIn >> nIndexFont >> nIndexFormat >> nAttr0 >> nAlign >> nFillCol
        >> nFill_Bottom >> nBorder0 >> nBorder1;

    aBorder.nTopLine = ( BYTE ) nBorder0 & 0x0007;              // .............210
    aBorder.nTopColor = ( nBorder0 & 0xFE00 ) >> 9;             // 5432109.........
    aBorder.nLeftLine = ( BYTE ) ( ( nBorder0 & 0x0038 ) >> 3 );// ..........543...
    aBorder.nLeftColor = nBorder1 & 0x007F ;                    // .........6543210
    aBorder.nBottomLine = ( BYTE ) ((nFill_Bottom & 0x01C0)>>6);// .......876......
    aBorder.nBottomColor = ( nFill_Bottom & 0xFE00 ) >> 9;      // 5432109.........
    aBorder.nRightLine = ( BYTE ) ( ( nBorder0 & 0x01C0 ) >> 6);//........876.....
    aBorder.nRightColor = ( nBorder1& 0x3F80 ) >> 7;            // ..3210987.......

    aFill.nPattern = ( BYTE ) nFill_Bottom & 0x003F;            // ..........543210
    aFill.nForeColor = nFillCol & 0x007F;                       // .........6543210
    aFill.nBackColor = ( nFillCol & 0x1F80 ) >> 7;              // ...210987.......

    if( nAttr0 & 0x0004 )
    {// Style-XF
        pXFD->SetStyleXF();
    }
    else
    {// Cell-XF
        pXFD->SetCellXF();
    }

    pXFD->SetParent( ( nAttr0 & 0xFFF0 ) >> 4 );

    pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nIndexFormat ) );

    pXFD->SetFont( nIndexFont );

    pXFD->SetAlign( ( ExcHorizAlign ) ( nAlign & 0x0007 ) );
    pXFD->SetAlign( ( ExcVertAlign ) ( ( nAlign & 0x0030 ) >> 4 ) );
            // using 2 instead of 3 Bit, because of lack of Applix to export correct alignment...
    pXFD->SetTextOrient( ( ExcTextOrient ) ( ( nAlign & 0x0300 ) >> 8 )  );

    pXFD->SetWrap( ( ExcWrapText ) ( ( nAlign & 0x0008 ) >> 3 ) );

    pXFD->SetBorder( aBorder );

    pXFD->SetFill( aFill );

    pXFD->SetLocked( TRUEBOOL( nAttr0 & 0x0001 ) );
    pXFD->SetHidden( TRUEBOOL( nAttr0 & 0x0002 ) );

    pExcRoot->pXF_Buffer->NewXF( pXFD );
}


void ImportExcel::Blank34( void )
{
    UINT16  nRow, nCol, nXF;

    aIn >> nRow >> nCol >> nXF;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF, TRUE );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Number34( void )
{
    UINT16  nRow, nCol, nXF;
    double  fValue;

    aIn >> nRow >> nCol >> nXF >> fValue;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        ScValueCell* pZelle = new ScValueCell( fValue );
        DBG_ASSERT( pZelle != NULL, "ImportExcel::Number34(): Nix Zelle!" );

        pD->PutCell( nCol, nRow, nTab, pZelle, ( BOOL )TRUE );

        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Label34( void )
{
    UINT16  nR, nC, nXF;

    aIn >> nR >> nC >> nXF;

    SetTextCell( nC, nR, ReadExcString( LenWord ), nXF );

    pLastFormCell = NULL;
}


void ImportExcel::Boolerr34( void )
{
    UINT16                  nRow, nCol, nXF;
    BYTE                    bErrOrVal, nError;

    aIn >> nRow >> nCol >> nXF;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        double              fVal;
        const ScTokenArray* pErgebnis;

        aIn >> nError >> bErrOrVal;

        // Simulation ueber Formelzelle!
        pErgebnis = ErrorToFormula( bErrOrVal, nError, fVal );

        ScFormulaCell*      pZelle = new ScFormulaCell(
            pD, ScAddress( nCol, nRow, nTab ), pErgebnis );

        pZelle->SetDouble( fVal );

        pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL)TRUE );

        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Row34( void )
{
    UINT16  nRow, nRowHeight, nGrbit;

    aIn >> nRow;
    aIn.SeekRel( 4 );   // Mic und Mac ueberspringen

    if( nRow <= MAXROW )
    {
        aIn >> nRowHeight;  // direkt in Twips angegeben
        aIn.SeekRel( 4 );

        aIn >> nGrbit;

        aRowOutlineBuff.SetLevel( nRow, EXC_ROW_GETLEVEL( nGrbit ),
            TRUEBOOL( nGrbit & EXC_ROW_COLLAPSED ), TRUEBOOL( nGrbit & EXC_ROW_ZEROHEIGHT ) );

        aColRowBuff.SetRowSettings( nRow, nRowHeight, nGrbit );
    }
}


void ImportExcel::Bof3( void )
{
    UINT16      nTyp;

    aIn.SeekRel( 2 );
    aIn >> nTyp;

    DBG_ASSERT( nTyp != 0x0100, "*ImportExcel::Bof3(): Biff3 als Workbook?!" );

    pExcRoot->eHauptDateiTyp = Biff3;

    if( nTyp == 0x0010 )        // Sheet?
        pExcRoot->eDateiTyp = Biff3;
    else if( nTyp == 0x0100 )   // Book?
        pExcRoot->eDateiTyp = Biff3W;
    else if( nTyp == 0x0020 )   // Chart?
        pExcRoot->eDateiTyp = Biff3C;
    else if( nTyp == 0x0040 )   // Macro?
        pExcRoot->eDateiTyp = Biff3M;
    else
    {
        pExcRoot->eDateiTyp = BiffX;
        pExcRoot->eHauptDateiTyp = BiffX;
    }
}


void ImportExcel::Name34( void )
{
    BYTE                nLenName;
    UINT16              nAttr, nLenExpr;
    const ScTokenArray* pErgebnis;
    BOOL                bPrintArea;
    BOOL                bPrintTitles;
    BOOL                bBuildIn;

    aIn >> nAttr;
    aIn.SeekRel( 1 );
    aIn >> nLenName >> nLenExpr;
    nBytesLeft -= 6;

    pFormConv->Reset( nLenExpr );

    // Namen einlesen
    String              aName( ASSTOSTR( ReadString( aIn, nLenName ) ) );
    sal_Char            cFirstNameChar = aName.GetChar( 0 );

    const UINT32        nFormStart = aIn.Tell();

    if( nLenName == 1 && cFirstNameChar < EXC_BUILTIN_UNKNOWN )
    {// Build-in name (0x00 - 0x0D als erstes Zeichen)
        bPrintArea = ( cFirstNameChar == EXC_BUILTIN_PRINTAREA );
        bPrintTitles = ( cFirstNameChar == EXC_BUILTIN_PRINTTITLES );
        bBuildIn = TRUE;

        aName.AssignAscii( GetExcBuiltInName( cFirstNameChar ) );
    }
    else
    {
        ExcelNameToScName( aName );

        bPrintArea = bPrintTitles = bBuildIn = FALSE;
    }

    const BOOL          bHidden = TRUEBOOL( nAttr & EXC_NAME_HIDDEN );

    if( nAttr & EXC_NAME_VB )
        // function, command or name on macro sheet?
        pFormConv->GetDummy( pErgebnis );
    else if( bBuildIn )
    {
        if( bPrintArea )
        {// Druckbereich
            long        nAnzBytes = nLenExpr;
            pFormConv->Convert( *pPrintRanges, nAnzBytes, FT_RangeName );
        }
        else if( bPrintTitles )
        {// Druckbereich
            long        nAnzBytes = nLenExpr;
            pFormConv->Convert( *pPrintTitles, nAnzBytes, FT_RangeName );
        }
        aIn.Seek( nFormStart );

        long            nAnzBytes = nLenExpr;
        pFormConv->Convert( pErgebnis, nAnzBytes, FT_RangeName );
    }
    else
        // Parsed expression Scannen
    {
        long            nAnzBytes = nLenExpr;
        pFormConv->Convert( pErgebnis, nAnzBytes, FT_RangeName );
    }

    if( bHidden ) // ohne hidden und complex
        pExcRoot->pRNameBuff->Store( aName, NULL );
    else
        pExcRoot->pRNameBuff->Store( aName, pErgebnis, 0, bPrintArea );
}


void ImportExcel::Array34( void )
{
    UINT16                  nFirstRow, nLastRow, nFormLen;
    BYTE                    nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
    aIn.SeekRel( 6 );
    aIn >> nFormLen;

    if( nLastRow <= MAXROW && nLastCol <= MAXCOL )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;

        pFormConv->Reset( nFormLen, ScAddress( nFirstCol, nFirstRow, nTab ) );

        long                nAnzBytes = nFormLen;

        pFormConv->Convert( pErgebnis, nAnzBytes );
        nBytesLeft += nAnzBytes - nFormLen;

        DBG_ASSERT( pErgebnis, "+ImportExcel::Array34(): ScTokenArray ist NULL!" );

        ScMarkData          aMarkData;
        aMarkData.SelectOneTable( nTab );
        pD->InsertMatrixFormula( nFirstCol, nFirstRow, nLastCol, nLastRow, aMarkData, EMPTY_STRING, pErgebnis);

        UINT16              nRowCnt, nColCnt;

        for( nColCnt = nFirstCol + 1 ; nColCnt <= nLastCol ; nColCnt++ )
            for( nRowCnt = nFirstRow ; nRowCnt <= nLastRow ; nRowCnt++ )
            {
                pFltTab->SetXF( nColCnt, nRowCnt, nLastXF );
                aColRowBuff.Used( nColCnt, nRowCnt );
            }
    }

    pLastFormCell = NULL;
}


void ImportExcel::Externname34( void )
{
}


void ImportExcel::Defrowheight345( void )
{
    UINT16  nDef, nOpt;

    aIn >> nOpt >> nDef;

    nDef = ( UINT16 ) ( ( double ) nDef * pExcRoot->fRowScale );

    if( nOpt && 0x0002 )
        aColRowBuff.SetDefHeight( 0 );
    else
        aColRowBuff.SetDefHeight( nDef );
}


void ImportExcel::Font34( void )
{
    UINT16      nHeight, nIndexCol;
    BYTE        nAttr0;

    aIn >> nHeight >> nAttr0;
    aIn.SeekRel( 1 );
    aIn >> nIndexCol;

    pExcRoot->pFontBuffer->NewFont( nHeight, nAttr0, nIndexCol, EXCTOSTR( ReadExcString( LenByte ) ) );
}


void ImportExcel::Window2_5( void )
{
    ScExtDocOptions&    rExtOpt = *pExcRoot->pExtDocOpt;
    UINT16              nOpt;
    aIn >> nOpt;

    if( nOpt & 0x0400 )
    {// Paged
        UINT16          nCol, nRow;

        aIn >> nRow >> nCol;
        if( nRow > MAXROW )
            nRow = MAXROW;
        if( nCol > MAXCOL )
            nCol = MAXCOL;

        rExtOpt.SetActTab( nTab );
        rExtOpt.SetVisCorner( nCol, nRow );
    }

    if( nOpt & 0x0008 )     // Frozen
        aColRowBuff.SetFrozen( TRUE );

    if( !( nOpt & 0x0020 ) )
    {// Use RGB
        BYTE            nGridR, nGridG, nGridB;

        aIn >> nGridR >> nGridG >> nGridB;

        rExtOpt.SetGridCol( nGridR, nGridG, nGridB );
    }

    if( !nTab )     // nur Tabelle 0 wird uebernommen!
    {
        ScViewOptions   aOpts( pD->GetViewOptions() );

        if( nOpt & 0x0001 )
            // Display Formulas
            aOpts.SetOption( VOPT_FORMULAS, TRUE );
        else
            aOpts.SetOption( VOPT_FORMULAS, FALSE );

        if( nOpt & 0x0002 )
            // Display Gridlines
            aOpts.SetOption( VOPT_GRID, TRUE );
        else
            // Display No Gridlines
            aOpts.SetOption( VOPT_GRID, FALSE );

        if( nOpt & 0x0004 )
            aOpts.SetOption( VOPT_HEADER, TRUE );
        else
            // No Row-/Col-Headings
            aOpts.SetOption( VOPT_HEADER, FALSE );

        if( nOpt & 0x0010 )
            aOpts.SetOption( VOPT_NULLVALS, TRUE );
        else
            // Suppress Zero-Values
            aOpts.SetOption( VOPT_NULLVALS, FALSE );

        if( nOpt & 0x0080 )
            aOpts.SetOption( VOPT_OUTLINER, TRUE );
        else
            // Hide Outline
            aOpts.SetOption( VOPT_OUTLINER, FALSE );

        pD->SetViewOptions( aOpts );
    }
}


void ImportExcel::XF3( void )
{
    BYTE        nDummyFormat, nDummyFont;
    UINT16      nAttr0, nAttr1, nIndexFormat, nIndexFont, nBorder, nFill;
    CellBorder  aBorder;
    CellFill    aFill;
    XF_Data*    pXFD = new XF_Data;

    aIn >> nDummyFont >> nDummyFormat >> nAttr0 >> nAttr1 >> nFill >> nBorder;

    aBorder.nTopLine = ( BYTE ) nBorder & 0x0007;               // .............210
    aBorder.nTopColor = ( nBorder & 0x00F8 ) >> 3;              // ........76543...
    aBorder.nLeftLine = ( BYTE ) ( ( nBorder & 0x0700 ) >> 8 ); // .....098........
    aBorder.nLeftColor = ( nBorder & 0xF800 ) >> 11;            // 54321...........
    aIn >> nBorder;
    aBorder.nBottomLine = ( BYTE ) nBorder & 0x0007;            // .............210
    aBorder.nBottomColor = ( nBorder & 0x00F8 ) >> 3;           // ........76543...
    aBorder.nRightLine = ( BYTE ) ( ( nBorder & 0x0700 ) >> 8 );// .....098........
    aBorder.nRightColor = ( nBorder & 0xF800 ) >> 11;           // 54321...........

    nIndexFormat = nDummyFormat;
    nIndexFont = nDummyFont;
    aFill.nPattern = ( BYTE ) ( nFill & 0x003F );               // ..........543210
    aFill.nForeColor = ( nFill & 0x07C0 ) >> 6;                 // .....09876......
    aFill.nBackColor = ( nFill & 0xF800 ) >> 11;                // 54321...........

    if( nAttr0 & 0x0004 )
    {// Style-XF
        pXFD->SetStyleXF();
    }
    else
    {// Cell-XF
        pXFD->SetCellXF();
    }

    pXFD->SetParent( ( nAttr1 & 0xFFF0 ) >> 4 );

    pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nIndexFormat ) );

    pXFD->SetFont( nIndexFont );

    pXFD->SetAlign( ( ExcHorizAlign ) ( nAttr1 & 0x0007 ) );

    pXFD->SetWrap( ( ExcWrapText ) ( ( nAttr1 & 0x0008 ) >> 3 ) );

    pXFD->SetAlign( EVA_Top );
    pXFD->SetTextOrient( ETO_NoRot );

    pXFD->SetBorder( aBorder );

    pXFD->SetFill( aFill );

    pXFD->SetLocked( TRUEBOOL( nAttr0 & 0x0001 ) );
    pXFD->SetHidden( TRUEBOOL( nAttr0 & 0x0002 ) );

    pExcRoot->pXF_Buffer->NewXF( pXFD );
}


void ImportExcel::Bof4( void )
{
    //POST: eDateiTyp = Biff4
    UINT16  nTyp;

    aIn.SeekRel( 2 );
    aIn >> nTyp;

    pExcRoot->eHauptDateiTyp = Biff4;

    if( nTyp == 0x0010 )        // Sheet?
    {
        pExcRoot->eDateiTyp = Biff4;
    }
    else if( nTyp == 0x0100 )   // Book?
    {
        pExcRoot->eDateiTyp = Biff4W;
    }
    else if( nTyp == 0x0020 )   // Chart?
    {
        pExcRoot->eDateiTyp = Biff4C;
    }
    else if( nTyp == 0x0040 )   // Macro?
    {
        pExcRoot->eDateiTyp = Biff4M;
    }
    else
    {
        pExcRoot->eDateiTyp = BiffX;
        pExcRoot->eHauptDateiTyp = BiffX;
    }
}


void ImportExcel::Format4( void )
{
    aIn.SeekRel( 2 );
    pValueFormBuffer->NewValueFormat( EXCTOSTR( ReadExcString( LenByte ) ) );
}


void ImportExcel::XF4( void )
{
    BYTE        nDummyFormat, nDummyFont;
    UINT16      nAttr0, nAttr1, nIndexFormat, nIndexFont, nBorder, nFill;
    CellBorder  aBorder;
    CellFill    aFill;
    XF_Data*    pXFD = new XF_Data;

    aIn >> nDummyFont >> nDummyFormat >> nAttr0 >> nAttr1 >> nFill >> nBorder;
    aBorder.nTopLine = ( BYTE ) nBorder & 0x0007;               // .............210
    aBorder.nTopColor = ( nBorder & 0x00F8 ) >> 3;              // ........76543...
    aBorder.nLeftLine = ( BYTE ) ( ( nBorder & 0x0700 ) >> 8 ); // .....098........
    aBorder.nLeftColor = ( nBorder & 0xF800 ) >> 11;            // 54321...........
    aIn >> nBorder;
    aBorder.nBottomLine = ( BYTE ) nBorder & 0x0007;            // .............210
    aBorder.nBottomColor = ( nBorder & 0x00F8 ) >> 3;           // ........76543...
    aBorder.nRightLine = ( BYTE ) ( ( nBorder & 0x0700 ) >> 8 );// .....098........
    aBorder.nRightColor = ( nBorder & 0xF800 ) >> 11;           // 54321...........

    nIndexFormat = nDummyFormat;
    nIndexFont = nDummyFont;
    aFill.nPattern = ( BYTE ) nFill & 0x003F;                   // ..........543210
    aFill.nForeColor = ( nFill & 0x07C0 ) >> 6;                 // .....09876......
    aFill.nBackColor = ( nFill & 0xF800 ) >> 11;                // 54321...........

    if( nAttr0 & 0x0004 )
    {// Style-XF
        pXFD->SetStyleXF();
    }
    else
    {// Cell-XF
        pXFD->SetCellXF();
    }

    pXFD->SetParent( ( nAttr0 & 0xFFF0 ) >> 4 );

    pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nIndexFormat ) );

    pXFD->SetFont( nIndexFont );

    pXFD->SetAlign( ( ExcHorizAlign ) ( nAttr1 & 0x0007 ) );

    pXFD->SetWrap( ( ExcWrapText ) ( ( nAttr1 & 0x0008 ) >> 3 ) );

    pXFD->SetAlign( ( ExcVertAlign ) ( ( nAttr1 & 0x0030 ) >> 4 ) );
    pXFD->SetTextOrient(
        ( ExcTextOrient ) ( ( nAttr1 & 0x00C0 ) >> 6 )  );

    pXFD->SetBorder( aBorder );

    pXFD->SetFill( aFill );

    pXFD->SetLocked( TRUEBOOL( nAttr0 & 0x0001 ) );
    pXFD->SetHidden( TRUEBOOL( nAttr0 & 0x0002 ) );

    pExcRoot->pXF_Buffer->NewXF( pXFD );
}


void ImportExcel::Bof5( void )
{
    //POST: eDateiTyp = Typ der zu lesenden Datei
    UINT16      nSubType, nVers;
    BiffTyp     eHaupt = Biff5;
    BiffTyp     eDatei;

    aIn >> nVers >> nSubType;

    switch( nSubType )
    {
        case 0x0005:                        // Workbook-Globals?
            eDatei = Biff5W;
            break;
        case 0x0006:                        // Visual Basic?
            eDatei = Biff5V;
            break;
        case 0x0010:                        // Worksheet?
            eDatei = Biff5;
            break;
        case 0x0020:                        // Chart?
            eDatei = Biff5C;
            break;
        case 0x0040:                        // Excel4 Macro?
            eDatei = Biff5M4;
//          eHaupt = Biff4;                 // !!!!!!!!! RICHTIG? !!!!!!!!!!!!
            break;
        case 0x0100:                        // Workbook?
            eDatei = Biff5W;
            break;
        default:
            pExcRoot->eDateiTyp = BiffX;
            pExcRoot->eHauptDateiTyp = BiffX;
            return;
    }

    if( nVers == 0x0600 && pExcRoot->eGlobalDateiTyp != Biff5W )
    {// Biff8
        eHaupt = ( BiffTyp ) ( eHaupt - Biff5 + Biff8 );
        eDatei = ( BiffTyp ) ( eDatei - Biff5 + Biff8 );
    }

    pExcRoot->eHauptDateiTyp = eHaupt;
    pExcRoot->eDateiTyp = eDatei;
}


double ImportExcel::RkToDouble( const UINT32& nRk )
{
    //PLATTFORMABHAENGIG//
    double  fVal;

    // jetzt kommt Code aus'm Excel-Developer's-Kit-Buch V5 (S.223)
    if( nRk & 0x02 )
        // Integer
        fVal = ( double ) ( *( ( INT32 * ) &nRk ) >> 2 );
    else
    {// 64-Bit IEEE-Float
#ifdef __BIGENDIAN
//680xx und alle anderen vernuenftigen Prozessoren...
        *( ( UINT32 * ) &fVal + 1 ) = 0;    // unteren 32 Bits = 0
        *( ( UINT32 * ) &fVal ) = nRk & 0xFFFFFFFC; // Bit 0, 1 = 0
#else
//Intel-Sch...
        *( ( UINT32 * ) &fVal ) = 0;    // unteren 32 Bits = 0
        *( ( UINT32 * ) &fVal + 1 ) = nRk & 0xFFFFFFFC; // Bit 0, 1 = 0
#endif
    }

    if( nRk & 0x01 )
        fVal *= 0.01;

    return fVal;
}


void ImportExcel::ReadExcString( StringInfoLen eLen, ByteString& aString )
{
    //  POST:   ACHTUNG: Funktion setzt nBytesLeft selbstaendig!!!!!
    //                      (geht auch nicht anders!)
    //          ACHTUNG: es wird an String angehaengt

    if( nBytesLeft < 0 )        // hack for Applix rubbish, never needed before!
        return;

    UINT16      nLaenge;

    if( eLen == LenByte )
    {
        BYTE    nHelp;
        aIn >> nHelp;
        nLaenge = nHelp;
        nBytesLeft--;
    }
    else
    {
        aIn >> nLaenge;
        nBytesLeft -= 2;
    }

    if( nBytesLeft < nLaenge )  // hack for Applix rubbish, never needed before!
        nLaenge = nBytesLeft;

    AppendString( aIn, aString, nLaenge );
    nBytesLeft -= nLaenge;
}


void ImportExcel::ReadExcString( StringInfoLen eLen, String& aString )
{
    ByteString  aTmp;

    ReadExcString( eLen, aTmp );

    aString += String( aTmp, eQuellChar );
}


void ImportExcel::ResetBof( void )
{   // setzt alle Einstellungen fuer neuen Tabellenbeginn zurueck
    aColRowBuff.Reset();
    pFltTab->Reset();
}


void ImportExcel::EndSheet( void )
{   // mach 'Gemarmel' am Ende eines Sheets
    aColOutlineBuff.SetOutlineArray( pD->GetOutlineTable( nTab, TRUE )->GetColArray() );
    aColOutlineBuff.MakeScOutline();
    aColOutlineBuff.Reset();

    aRowOutlineBuff.SetOutlineArray( pD->GetOutlineTable( nTab, TRUE )->GetRowArray() );
    aRowOutlineBuff.MakeScOutline();
    aRowOutlineBuff.Reset();

    aColRowBuff.Apply( nTab );

    pFltTab->Apply( nTab );

    pExcRoot->pExtSheetBuff->Reset();
    pExcRoot->pExtNameBuff->Reset();
}


ByteString ImportExcel::ReadExcString( StringInfoLen eLen )
{
    //  POST:   ACHTUNG: Funktion setzt nBytesLeft selbstaendig!!!!!
    //                      (geht auch nicht anders!)
    ByteString  aRet;
    UINT16      nLaenge;

    if( eLen == LenByte )
    {
        BYTE    nHelp;
        aIn >> nHelp;
        nLaenge = nHelp;
        nBytesLeft--;
    }
    else
    {
        aIn >> nLaenge;
        nBytesLeft -= 2;
    }

    if( nLaenge )
    {
        sal_Char*   p = new sal_Char[ nLaenge + 1 ];
        aIn.Read( p, nLaenge );
        p[ nLaenge ] = 0x00;
        aRet = p;
        delete[] p;
        nBytesLeft -= nLaenge;
    }

    return aRet;
}


void ImportExcel::NeueTabelle( void )
{
    if( nTab > 0 && !pD->HasTable( nTab ) )
        pD->MakeTable( nTab );

    // fuer neue Tabelle eigene Tabellenvorlage erzeugen
    String      aStyleName( GetPageStyleName( nTab ) );

    pStyleSheetItemSet =
        &pD->GetStyleSheetPool()->Make( aStyleName, SFX_STYLE_FAMILY_PAGE,
        SFXSTYLEBIT_USERDEF ).GetItemSet();

    pExcRoot->bDefaultPage = TRUE;
}


const ScTokenArray* ImportExcel::ErrorToFormula( BYTE bErrOrVal, BYTE nError, double& rVal )
{
    BoolError   eType;
    if( bErrOrVal )
    {// Error Value
        switch( nError )
        {
            case 0x00:  eType = BE_NULL;    break;
            case 0x07:  eType = BE_DIV0;    break;
            case 0x0F:  eType = BE_VALUE;   break;
            case 0x17:  eType = BE_REF;     break;
            case 0x1D:  eType = BE_NAME;    break;
            case 0x24:  eType = BE_NUM;     break;
            case 0x2A:  eType = BE_NA;      break;
            default:    eType = BE_UNKNOWN; break;
        }
        rVal = 0.0;
    }
    else
    {// Boolean Value
        if( nError )
        {
            eType = BE_TRUE;
            rVal = 1.0;
        }
        else
        {
            eType = BE_FALSE;
            rVal = 0.0;
        }
    }
    return pFormConv->GetBoolErr( eType );
}


void ImportExcel::ScanHeadFootParts( const String& rIn, EditTextObject*& rpLeft,
    EditTextObject*& rpMid, EditTextObject*& rpRight )
{
    enum State { SearchEmb, SearchFunc, AddLeft, AddMid, AddRight, ScanSize, ScanFont };

    EditEngine&         rEdEng = pExcRoot->GetEdEngForHF();
    ESelection          aAppSel( 0xFFFF, 0xFFFF );

    rEdEng.SetText( EMPTY_STRING );     //NN: wenn nur ein Feld belegt ist, kommt kein &C

    const sal_Unicode*  pAct = rIn.GetBuffer();

    State               eAct = AddMid;
    State               ePrev = AddMid;
    String              aTmpString;

    while( *pAct )
    {
        switch( eAct )
        {
            case SearchEmb:             // '&' suchen
                if( *pAct == '&' )
                    eAct = SearchFunc;
                break;
            case SearchFunc:            // auf Funktionen verteilen
                switch( *pAct )
                {
                    case 'C':               // Start mittlerer Teil
                        if( ePrev == AddLeft )
                        {
                            delete rpLeft;
                            rpLeft = rEdEng.CreateTextObject();
                        }
                        else if( ePrev == AddRight )
                        {
                            delete rpRight;
                            rpRight = rEdEng.CreateTextObject();
                        }

                        if( !rpMid )
                        {
                            rEdEng.SetText( EMPTY_STRING );
                            rpMid = rEdEng.CreateTextObject();
                        }
                        else
                            rEdEng.SetText( *rpMid );

                        eAct = ePrev = AddMid;
                        break;
                    case 'L':               // Start linker Teil
                        if( ePrev == AddMid )
                        {
                            delete rpMid;
                            rpMid = rEdEng.CreateTextObject();
                        }
                        else if( ePrev == AddRight )
                        {
                            delete rpRight;
                            rpRight = rEdEng.CreateTextObject();
                        }

                        if( !rpLeft )
                        {
                            rEdEng.SetText( EMPTY_STRING );
                            rpLeft = rEdEng.CreateTextObject();
                        }
                        else
                            rEdEng.SetText( *rpLeft );

                        eAct = ePrev = AddLeft;
                        break;
                    case 'R':               // Start rechter Teil
                        if( ePrev == AddMid )
                        {
                            delete rpMid;
                            rpMid = rEdEng.CreateTextObject();
                        }
                        else if( ePrev == AddLeft )
                        {
                            delete rpLeft;
                            rpLeft = rEdEng.CreateTextObject();
                        }

                        if( !rpRight )
                        {
                            rEdEng.SetText( EMPTY_STRING );
                            rpRight = rEdEng.CreateTextObject();
                        }
                        else
                            rEdEng.SetText( *rpRight );

                        eAct = ePrev = AddRight;
                        break;
                    case 'P':               // Seitennummer
                        rEdEng.QuickInsertField( SvxFieldItem( SvxPageField() ), aAppSel );
                        eAct = ePrev;
                        break;
                    case 'N':               // Seitenzahl
                        rEdEng.QuickInsertField( SvxFieldItem( SvxPagesField() ), aAppSel );
                        eAct = ePrev;
                        break;
                    case 'D':               // Datum
                        rEdEng.QuickInsertField( SvxFieldItem( SvxDateField() ), aAppSel );
                        eAct = ePrev;
                        break;
                    case 'T':               // Zeit
                        rEdEng.QuickInsertField( SvxFieldItem( SvxTimeField() ), aAppSel );
                        eAct = ePrev;
                        break;
                    case 'F':               // Datei
                        rEdEng.QuickInsertField( SvxFieldItem( SvxExtFileField() ), aAppSel );
                        eAct = ePrev;
                        break;
                    case 'A':               // Tabellen-Name
                        rEdEng.QuickInsertField( SvxFieldItem( SvxTableField() ), aAppSel );
                        eAct = ePrev;
                        break;
                    case '\"':              // Font
                        eAct = ScanFont;
                        break;
                    default:
                        if( *pAct >= '0' && *pAct <= '9' )  // Font Size
                            eAct = ScanSize;
                        else
                            eAct = ePrev;
                }
                break;
            case AddLeft:               // linkes Drittel fuellen
            case AddMid:                // mittleres Drittel fuellen
            case AddRight:              // rechtes Drittel fuellen
                if( *pAct == '&' )
                {
                    if( aTmpString.Len() )
                    {
                        rEdEng.QuickInsertText( aTmpString, aAppSel );
                        aTmpString.Erase();
                    }

                    eAct = SearchFunc;
                }
                else
                    aTmpString += *pAct;
                break;
            case ScanFont:              // Font ueberlesen
                if( *pAct == '\"' )
                    eAct = ePrev;
                break;
            case ScanSize:              // Font Size ueberlesen
                if( *pAct < '0' || *pAct > '9' )
                {
                    if( *pAct == '&' )
                    {
                        if( aTmpString.Len() )
                        {
                            rEdEng.QuickInsertText( aTmpString, aAppSel );
                            aTmpString.Erase();
                        }

                        eAct = SearchFunc;
                    }
                    else
                    {
                        aTmpString += *pAct;
                        eAct = ePrev;
                    }
                }
                break;
            default:
                DBG_ERROR( "*ImportExcel::ScanHeadFootParts(): State vergessen!" );
        }
        pAct++;
    }

    if( aTmpString.Len() )
    {
        rEdEng.QuickInsertText( aTmpString, aAppSel );
        aTmpString.Erase();
    }

    switch( ePrev )
    {
        case AddMid:
            delete rpMid;
            rpMid = rEdEng.CreateTextObject();
            break;
        case AddLeft:
            delete rpLeft;
            rpLeft = rEdEng.CreateTextObject();
            break;
        case AddRight:
            delete rpRight;
            rpRight = rEdEng.CreateTextObject();
            break;
    }

    //NN: Calc kommt durcheinander, wenn nicht alle 3 Pointer gesetzt sind:
    if( !rpMid || !rpLeft || !rpRight )
    {
        rEdEng.SetText( EMPTY_STRING );
        if( !rpMid )
            rpMid = rEdEng.CreateTextObject();
        if( !rpLeft )
            rpLeft = rEdEng.CreateTextObject();
        if( !rpRight )
            rpRight = rEdEng.CreateTextObject();
    }
}


void ImportExcel::GetHF( BOOL bHeader )
{
    String              aExcHF;
    EditTextObject*     pLeft = NULL;
    EditTextObject*     pMid = NULL;
    EditTextObject*     pRight = NULL;

    GetHFString( aExcHF );

    ScanHeadFootParts( aExcHF, pLeft, pMid, pRight );

    ScPageHFItem*       pHFItemLeft;
    ScPageHFItem*       pHFItemRight;
    if( bHeader )
    {   // erzeuge Header
        pHFItemLeft = new ScPageHFItem( ATTR_PAGE_HEADERLEFT );
        pHFItemRight = new ScPageHFItem( ATTR_PAGE_HEADERRIGHT );
    }
    else
    {   // erzeuge Footer
        pHFItemLeft = new ScPageHFItem( ATTR_PAGE_FOOTERLEFT );
        pHFItemRight = new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT );
    }

    DBG_ASSERT( pStyleSheetItemSet, "-ImportExcel::SetHF(): kein Set, keine Kopfer/Fusser!" );

    if( pLeft )
    {
        pHFItemLeft->SetLeftArea( *pLeft );
        pHFItemRight->SetLeftArea( *pLeft );
        delete pLeft;
    }

    if( pMid )
    {
        pHFItemLeft->SetCenterArea( *pMid );
        pHFItemRight->SetCenterArea( *pMid );
        delete pMid;
    }

    if( pRight )
    {
        pHFItemLeft->SetRightArea( *pRight );
        pHFItemRight->SetRightArea( *pRight );
        delete pRight;
    }

    pStyleSheetItemSet->Put( *pHFItemLeft );
    pStyleSheetItemSet->Put( *pHFItemRight );

    delete pHFItemLeft;
    delete pHFItemRight;
}


void ImportExcel::GetHFString( String& r )
{
    ByteString  aTmpStr;
    ReadExcString( LenByte, aTmpStr );

    r = EXCTOSTR( aTmpStr );
}


void ImportExcel::GetAndSetMargin( IMPEXC_MARGINSIDE eSide )
{
    double              fMargin;
    aIn >> fMargin;
    nBytesLeft -= 8;
    fMargin *= 1440.0;
    fMargin += 0.4999999999999; // casten -> runden

    DBG_ASSERT( pStyleSheetItemSet, "-ImportExcel::GetAndSetMargin(): nix StyleSheetItemSet!" );

    if( eSide == IMPEXC_MARGINSIDE_TOP || eSide == IMPEXC_MARGINSIDE_BOTTOM )
    {
        SvxULSpaceItem  aItem( ATTR_ULSPACE );
        aItem = ( const SvxULSpaceItem& ) pStyleSheetItemSet->Get( ATTR_ULSPACE );
        if( eSide == IMPEXC_MARGINSIDE_TOP )
            aItem.SetUpperValue( ( UINT16 ) fMargin );
        else
            aItem.SetLowerValue( ( UINT16 ) fMargin );
        pStyleSheetItemSet->Put( aItem );
    }
    else
    {
        SvxLRSpaceItem  aItem( ATTR_LRSPACE );
        aItem = ( const SvxLRSpaceItem& ) pStyleSheetItemSet->Get( ATTR_LRSPACE );
        if( eSide == IMPEXC_MARGINSIDE_RIGHT )
            aItem.SetRightValue( ( UINT16 ) fMargin );
        else
            aItem.SetLeftValue( ( UINT16 ) fMargin );
        pStyleSheetItemSet->Put( aItem );
    }
}


String ImportExcel::GetPageStyleName( UINT16 nTab )
{
    String          aRet( RTL_CONSTASCII_STRINGPARAM( "TAB_" ) );
    const String*   pTabName = pExcRoot->pTabNameBuff->Get( nTab );

    if( pTabName )
        // wenn Name vorhanden
        aRet += *pTabName;
    else
    {// kein Name vergeben -> Pech gehabt
        aRet.AppendAscii( "TAB" );
        aRet += String::CreateFromInt32( nTab );
    }

    return aRet;
}


EditTextObject* ImportExcel::CreateFormText( BYTE nAnzFrms, const String& rS, const UINT16 nXF )
{
    EditEngine&     rEdEng = GetEdEng();

    rEdEng.SetText( rS );

    SfxItemSet      aItemSet( rEdEng.GetEmptyItemSet() );
    pExcRoot->pFontBuffer->Fill( pExcRoot->pXF_Buffer->GetExcFont( nXF ), aItemSet, FALSE );

    ESelection      aSel( 0, 0 );

    if( nAnzFrms )
    {
        BYTE                nChar, nFont;
        const sal_Unicode*  pAktChar = rS.GetBuffer();
        sal_Unicode         cAkt = *pAktChar;
        UINT16              nCnt = 0;

        aIn >> nChar >> nFont;
        nAnzFrms--;

        while( cAkt )
        {
            if( nCnt >= nChar )
            {// neuer Item-Set
                rEdEng.QuickSetAttribs( aItemSet, aSel );

                aItemSet.ClearItem( 0 );

                pExcRoot->pFontBuffer->Fill( nFont, aItemSet, FALSE );
                if( nAnzFrms )
                {
                    aIn >> nChar >> nFont;
                    nAnzFrms--;
                }
                else
                    nChar = 0xFF;

                aSel.nStartPara = aSel.nEndPara;
                aSel.nStartPos = aSel.nEndPos;
            }

            if( cAkt == '\n' )
            {// new Paragraph
                aSel.nEndPara++;
                aSel.nEndPos = 0;
            }
            else
                aSel.nEndPos++;

            pAktChar++;
            cAkt = *pAktChar;
            nCnt++;
        }

        // letzten ItemSet setzten
        rEdEng.QuickSetAttribs( aItemSet, aSel );
    }
    else
    {
        aSel.nEndPos = rS.Len();
        rEdEng.QuickSetAttribs( aItemSet, aSel );
    }

    return rEdEng.CreateTextObject();
}


ScEditEngineDefaulter& ImportExcel::GetEdEng( void ) const
{
    return pExcRoot->GetEdEng();
}


void ImportExcel::PostDocLoad( void )
{
    ScDocOptions        aDocOpt( pD->GetDocOptions() );
    aDocOpt.SetIgnoreCase( TRUE );      // immer in Excel
    pD->SetDocOptions( aDocOpt );

    pExcRoot->pExtDocOpt->fColScale = pExcRoot->fColScale;
    pD->SetExtDocOptions( pExcRoot->pExtDocOpt );
    pExcRoot->pExtDocOpt = NULL;

    EndAllChartObjects();

    pD->UpdateAllCharts();

    const UINT16        nLast = pD->GetTableCount();
    const ScRange*      p;

    if( pPrintRanges->HasRanges() )
    {
        UINT16          nPos;

        for( UINT16 n = 0 ; n < nLast ; n++ )
        {
            p = pPrintRanges->First( n );
            if( p )
            {
                DBG_ASSERT( pPrintRanges->GetActList(),
                            "-ImportExcel::PostDocLoad(): Imaginaere Tabelle gefunden!" );

                pD->SetPrintRangeCount( n, ( UINT16 ) pPrintRanges->GetActList()->Count() );

                nPos = 0;

                while( p )
                {
                    pD->SetPrintRange( n, nPos, *p );

                    nPos++;
                    p = pPrintRanges->Next();
                }
            }
        }
    }

    if( pPrintTitles->HasRanges() )
    {
        for( UINT16 n = 0 ; n < nLast ; n++ )
        {
            p = pPrintTitles->First( n );
            if( p )
            {
                DBG_ASSERT( pPrintTitles->GetActList(),
                    "-ImportExcel::PostDocLoad(): Imaginaere Tabelle gefunden!" );

                BOOL    bRowVirgin = TRUE;
                BOOL    bColVirgin = TRUE;

                while( p )
                {
                    if( p->aStart.Col() == 0 && p->aEnd.Col() == MAXCOL && bRowVirgin )
                    {
                        pD->SetRepeatRowRange( n, p );
                        bRowVirgin = FALSE;
                    }

                    if( p->aStart.Row() == 0 && p->aEnd.Row() == MAXROW && bColVirgin )
                    {
                        pD->SetRepeatColRange( n, p );
                        bColVirgin = FALSE;
                    }

                    p = pPrintTitles->Next();
                }
            }
        }
    }
}


void ImportExcel::SetTextCell( const UINT16 nC, const UINT16 nR, ByteString& r, const UINT16 nXF )
{
    if( nR <= MAXROW && nC <= MAXCOL )
    {
        if( r.Len() )
        {
            String                  aStr( r, eQuellChar );

            ScBaseCell*             pZelle;

            if( pExcRoot->pXF_Buffer->HasAttribSuperOrSubscript( nXF ) )
            {// jetzt kommt 'ne Edit-Engine in's Spiel!
                EditTextObject*     pTObj = CreateFormText( 0, aStr, nXF );

                pZelle = new ScEditCell( pTObj, pD, GetEdEng().GetEditTextObjectPool() );

                delete pTObj;
            }
            else
                pZelle = ScBaseCell::CreateTextCell( aStr, pD );

            pD->PutCell( nC, nR, nTab, pZelle, ( BOOL ) TRUE );
        }

        aColRowBuff.Used( nC, nR );

        pFltTab->SetXF( nC, nR, nXF );
    }
    else
        bTabTruncated = TRUE;
}


UINT16 ImportExcel::CalcColWidth( const UINT16 n )
{
    double      f = ( double ) n;

    f *= pExcRoot->fColScale;
    f *= 23.0;
    f -= 90.0;
    f *= 25.0 / 1328.0;

    if( f < 0.0 )
        f = 0.0;

    return ( UINT16 ) f;
}




