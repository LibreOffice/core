/*************************************************************************
 *
 *  $RCSfile: impop.cxx,v $
 *
 *  $Revision: 1.56 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:47:38 $
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

#ifndef _IMP_OP_HXX
#include "imp_op.hxx"
#endif

#ifndef INCLUDED_SVX_COUNTRYID_HXX
#include <svx/countryid.hxx>
#endif

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
#include <svx/colritem.hxx>
#include <sfx2/printer.hxx>
#include <svtools/zforlist.hxx>

#include <so3/embobj.hxx>
#include <sfx2/objsh.hxx>
#include "docuno.hxx"

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
#include "scerrors.hxx"
#include "unonames.hxx"

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif

#include "excimp8.hxx"
#include "excform.hxx"

using namespace ::com::sun::star;


const double ImportExcel::fExcToTwips =
    ( double ) TWIPS_PER_CHAR / 256.0;


ImportTyp::ImportTyp( ScDocument* pDoc, CharSet eQ )
{
    eQuellChar = eQ;
    pD = pDoc;

    pExtOpt = NULL;
}


ImportTyp::~ImportTyp()
{
    ScExtDocOptions*    p = pD->GetExtDocOptions();
    if( p )
    {
        if( pExtOpt )
        {
            *p = *pExtOpt;
            delete pExtOpt;
        }
    }
    else
        pD->SetExtDocOptions( pExtOpt );
}


FltError ImportTyp::Read()
{
    return eERR_INTERN;
}


ScExtDocOptions &ImportTyp::GetExtOpt( void )
{
    if( !pExtOpt )
    {
        pExtOpt = new ScExtDocOptions;
        ScExtDocOptions*    pOrg = pD->GetExtDocOptions();
        if( pOrg )
            *pExtOpt = *pOrg;
    }
    return *pExtOpt;
}




ImportExcel::ImportExcel( SvStream& rSvStrm, ScDocument* pDoc, const String& rDocUrl ):
    XclImpRootData( xlBiffUnknown, *pDoc, rDocUrl, RTL_TEXTENCODING_MS_1252 ),
    ImportTyp( pDoc, RTL_TEXTENCODING_MS_1252 ),
    XclImpRoot( static_cast< XclImpRootData& >( *this ) ),
    maStrm( rSvStrm, *this ),
    aIn( maStrm ),
    bFitToPage( sal_False ),
    bHasHeader( sal_False ),
    bHasFooter( sal_False ),
    bHasTopMargin (sal_False ),
    bHasBottomMargin (sal_False ),
    bHasLeftMargin (sal_False ),
    bHasRightMargin (sal_False )
{
    pChart = pUsedChartFirst = pUsedChartLast = NULL;

    nBdshtTab = 0;
    nFirstVisTab = 0xFFFF;
    nIxfeIndex = 0;     // zur Sicherheit auf 0

    pPrintRanges = new _ScRangeListTabs;
    pPrintTitles = new _ScRangeListTabs;

    pStyleSheetItemSet = NULL;

    // Root-Daten fuellen - nach new's ohne Root als Parameter
    pExcRoot = mpRD;
    pExcRoot->pDoc = pDoc;
    pExcRoot->pIR = this;   // ExcRoot -> XclImpRoot
    pExcRoot->pScRangeName = pDoc->GetRangeName();
    pExcRoot->aStandard.AssignAscii( "General" );
    pExcRoot->eDateiTyp = pExcRoot->eHauptDateiTyp = BiffX;
    pExcRoot->pExtSheetBuff = new ExtSheetBuffer( pExcRoot );   //&aExtSheetBuff;
    pExcRoot->pTabNameBuff = new NameBuffer( pExcRoot );        //&aTabNameBuff;
    pExcRoot->pRNameBuff = new RangeNameBuffer( pExcRoot );     //&aRangeNameBuff;
    pExcRoot->pShrfmlaBuff = new ShrfmlaBuffer( pExcRoot );     //&aShrfrmlaBuff;
    pExcRoot->pExtNameBuff = new ExtNameBuff ( pExcRoot );
    pExcRoot->pCharset = &eQuellChar;   // dto.
    pExcRoot->pExtDocOpt = new ScExtDocOptions;
    if( pDoc->GetExtDocOptions() )
        *pExcRoot->pExtDocOpt = *pDoc->GetExtDocOptions();
    pExcRoot->pExtDocOpt->SetChanged( TRUE );

    pExtNameBuff = new NameBuffer( pExcRoot );          //#94039# prevent empty rootdata
    pExtNameBuff->SetBase( 1 );

    pOutlineListBuffer = new OutlineListBuffer( );

    // ab Biff8
    pFormConv = pExcRoot->pFmlaConverter = new ExcelToSc( pExcRoot, aIn );

    bTabTruncated = FALSE;

    // options from configuration
    ScFilterOptions aFilterOpt;

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

    ScDocOptions aDocOpt( pD->GetDocOptions() );
    aDocOpt.SetIgnoreCase( TRUE );              // always in Excel
    aDocOpt.SetFormulaRegexEnabled( FALSE );    // regular expressions? what's that?
    pD->SetDocOptions( aDocOpt );
}


ImportExcel::~ImportExcel( void )
{
    delete pPrintRanges;
    delete pPrintTitles;

    GetDoc().SetSrcCharSet( GetCharSet() );

    delete pExtNameBuff;

    delete pOutlineListBuffer;

    delete pFormConv;
}


void ImportExcel::Dimensions( void )
{
    UINT16 nRowFirst, nRowLast, nColFirst, nColLast;

    aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

    if( aIn.IsValid() )
        pColRowBuff->SetDimension(
            ScRange( nColFirst, nRowFirst, GetScTab(), nColLast, nRowLast, GetScTab() ) );
}


void ImportExcel::Blank25( void )
{
    UINT16 nRow, nCol, nXF;

    aIn >> nRow >> nCol;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {
        aIn.Ignore( 3 );
        nXF = 0;
    }
    else
        aIn >> nXF;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        pColRowBuff->Used( nCol, nRow );
        GetXFIndexBuffer().SetBlankXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Integer( void )
{
    UINT16 nRow, nCol, nInt;
    sal_uInt8 nXFData;

    aIn >> nRow >> nCol >> nXFData;
    aIn.Ignore( 2 );
    aIn >> nInt;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        sal_uInt16 nXF = nXFData & 0x3F;
        if( nXF == 63 ) nXF = nIxfeIndex;
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        ScBaseCell* pCell = new ScValueCell( nInt );
        GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Number25( void )
{
    UINT16 nRow, nCol, nXF;
    double fValue;

    aIn >> nRow >> nCol;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {
        aIn.Ignore( 3 );
        nXF = 0;
    }
    else
        aIn >> nXF;

    aIn >> fValue;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        ScBaseCell* pCell = new ScValueCell( fValue );
        GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Label25( void )
{
    UINT16  nR, nC, nXF;
    bool    b16BitLen;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {// nur fuer BIFF2
        BYTE    nAttr0, nAttr1, nAttr2;

        aIn >> nR >> nC >> nAttr0 >> nAttr1 >> nAttr2;

        nXF = nAttr0 & 0x3F;
        if( nXF == 63 )
            // IXFE-record stand davor
            nXF = nIxfeIndex;

        b16BitLen = false;
    }
    else
    {
        aIn >> nR >> nC >> nXF;

        b16BitLen = true;
    }

    String aTmpStr( aIn.ReadByteString( b16BitLen ) );
    SetTextCell( nC, nR, aTmpStr, nXF );

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
        ScFormulaCell* pCell = new ScFormulaCell(
            pD, ScAddress( nCol, nRow, GetScTab() ), pErgebnis );
        pCell->SetDouble( fVal );
        GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );

        if( bErrOrVal )     // !=0 -> Error
            GetXFIndexBuffer().SetXF( nCol, nRow, nXF );
        else                // ==0 -> Boolean
            GetXFIndexBuffer().SetBoolXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::RecString( void )
{
    if( pLastFormCell )
    {
        pLastFormCell->SetString( aIn.ReadByteString( pExcRoot->eHauptDateiTyp != Biff2 ) );

        pLastFormCell = NULL;
    }
}


void ImportExcel::Row25( void )
{
    UINT16  nRow, nRowHeight;

    aIn >> nRow;
    aIn.Ignore( 4 );    // Mic und Mac ueberspringen

    if( nRow <= MAXROW )
    {
        aIn >> nRowHeight;  // direkt in Twips angegeben
        aIn.Ignore( 2 );

        if( pExcRoot->eHauptDateiTyp == Biff2 )
        {// -------------------- BIFF2
            nRowHeight = ( UINT16 ) ( ( double ) nRowHeight * pExcRoot->fRowScale );
            pColRowBuff->SetHeight( nRow, nRowHeight );
        }
        else
        {// -------------------- BIFF5
            UINT16  nGrbit;

            aIn.Ignore( 2 );    // reserved
            aIn >> nGrbit;

            pRowOutlineBuff->SetLevel( nRow, EXC_ROW_GETLEVEL( nGrbit ),
                TRUEBOOL( nGrbit & EXC_ROW_COLLAPSED ), TRUEBOOL( nGrbit & EXC_ROW_ZEROHEIGHT ) );

            pColRowBuff->SetRowSettings( nRow, nRowHeight, nGrbit );
        }
    }
}


void ImportExcel::Bof2( void )
{
    sal_uInt16 nSubType;
#if SC_XCL_USEDECR
    maStrm.UseDecryption( false );
#endif
    maStrm.Ignore( 2 );
    maStrm >> nSubType;
    SetBiff( xlBiff2 );
#if SC_XCL_USEDECR
    maStrm.UseDecryption( true );
#endif

    pExcRoot->eHauptDateiTyp = Biff2;
    if( nSubType == 0x0010 )        // Worksheet?
        pExcRoot->eDateiTyp = Biff2;
    else if( nSubType == 0x0020 )   // Chart?
        pExcRoot->eDateiTyp = Biff2C;
    else if( nSubType == 0x0040 )   // Macro?
        pExcRoot->eDateiTyp = Biff2M;
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
    IncScTab();
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
    if( aIn.GetRecLeft() )
    {
        GetHF( TRUE );
        bHasHeader = sal_True;
    }
}


void ImportExcel::Footer( void )
{
    if( aIn.GetRecLeft() )
    {
        GetHF( FALSE );
        bHasFooter = sal_True;
    }
}


void ImportExcel::Externsheet( void )
{
    String aEncodedUrl, aUrl, aTabName;
    bool bSameWorkBook;
    aIn.AppendByteString( aEncodedUrl, false );
    XclImpUrlHelper::DecodeUrl( aUrl, aTabName, bSameWorkBook, *pExcRoot->pIR, aEncodedUrl );
    ScfTools::ConvertToScSheetName( aTabName );
    pExcRoot->pExtSheetBuff->Add( aUrl, aTabName, bSameWorkBook );
}


void ImportExcel::Name25( void )
{
    const ScTokenArray* pErgebnis;
    UINT16              nLenDef;
    BYTE                nLenName, nLen;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {// --------------------------------------------------- Biff2
        BYTE            nAttr0, nAttr1, nLenExpr;

        aIn >> nAttr0 >> nAttr1;
        aIn.Ignore( 1 );
        aIn >> nLenName >> nLenExpr;

        // Namen einlesen
        String aName( aIn.ReadRawByteString( nLenName ) );
        ScfTools::ConvertToScDefinedName( aName );

        pFormConv->Reset();
        if( nAttr0 & 0x02 )
            pFormConv->GetDummy( pErgebnis );       // function or command?
        else
            pFormConv->Convert( pErgebnis, nLenExpr, FT_RangeName );    // ...or simple formula

        pExcRoot->pRNameBuff->Store( aName, pErgebnis );

        aIn.Ignore( 1 );    // cceDup ueberfluessig
    }// ------------------------------------------Ende fuer Biff2
    else
    {// --------------------------------------------------- Biff5
        UINT16          nOpt;
        UINT16          nLenSeekRel = 0;
        UINT16          nSheet;

        aIn >> nOpt;
        aIn.Ignore( 1 );
        aIn >> nLenName >> nLenDef;
        aIn.Ignore( 2 );
        aIn >> nSheet
            >> nLen;            // length of custom menu text
        nLenSeekRel += nLen;
        aIn >> nLen;            // length of description text
        nLenSeekRel += nLen;
        aIn >> nLen;            // length of help topic text
        nLenSeekRel += nLen;
        aIn >> nLen;            // length of status bar text
        nLenSeekRel += nLen;

        // Namen einlesen
        String aName( aIn.ReadRawByteString( nLenName ) );
        // jetzt steht Lesemarke an der Formel

        const BOOL      bHidden = TRUEBOOL( nOpt & EXC_NAME_HIDDEN );
        const BOOL      bBuildIn = TRUEBOOL( nOpt & EXC_NAME_BUILTIN );

        sal_Char        cFirstNameChar = ( sal_Char ) aName.GetChar( 0 );
        const BOOL      bPrintArea = bBuildIn && ( cFirstNameChar == EXC_BUILTIN_PRINTAREA );
        const BOOL      bPrintTitles = bBuildIn && ( cFirstNameChar == EXC_BUILTIN_PRINTTITLES );
        RangeType       eNameType = RT_ABSAREA;

        if(bPrintArea)
            eNameType = RT_PRINTAREA;


        if( bBuildIn )
            aName = XclTools::GetBuiltInName( cFirstNameChar, nSheet );
        else
            ScfTools::ConvertToScDefinedName( aName );

        pFormConv->Reset();
        if( nOpt & (EXC_NAME_VB | EXC_NAME_BIG) )
            // function or command?
            pFormConv->GetDummy( pErgebnis );
        else if( bBuildIn )
        {
            aIn.PushPosition();

            if( bPrintArea )
                pFormConv->Convert( *pPrintRanges, nLenDef, FT_RangeName );
            else if( bPrintTitles )
                pFormConv->Convert( *pPrintTitles, nLenDef, FT_RangeName );

            aIn.PopPosition();

            pFormConv->Convert( pErgebnis, nLenDef, FT_RangeName );
        }
        else    // ...oder nur Formel
            pFormConv->Convert( pErgebnis, nLenDef, FT_RangeName );

        if( bHidden )
            pExcRoot->pRNameBuff->Store( aName, NULL, nSheet );
        else
            // ohne hidden
            pExcRoot->pRNameBuff->Store( aName, pErgebnis, nSheet, eNameType);
    }// ----------------------------------------- Ende fuer Biff5
}


void ImportExcel::Verticalpagebreaks( void )
{
    UINT16 n = aIn.ReaduInt16();

    while( n )
    {
        pColRowBuff->SetVertPagebreak( aIn.ReaduInt16() );
        n--;
    }
}


void ImportExcel::Horizontalpagebreaks( void )
{
    UINT16 n = aIn.ReaduInt16();

    while( n )
    {
        pColRowBuff->SetHorizPagebreak( aIn.ReaduInt16() );
        n--;
    }
}


void ImportExcel::Note( void )
{
    UINT16  nCol, nRow;
    aIn >> nRow >> nCol;

    if( nRow <= MAXROW && nCol <= MAXCOL )
        pD->SetNote( nCol, nRow, GetScTab(), aIn.ReadByteString( TRUE ) );
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Selection( void )
{
    UINT16  nNumRefs, nFirstRow, nLastRow;
    UINT8   nPane, nFirstCol, nLastCol;

    aIn >> nPane;
    aIn.Ignore( 6 );
    aIn >> nNumRefs;

    if( (nPane == pColRowBuff->GetActivePane()) && nNumRefs )
    {
        nNumRefs--;
        aIn.Ignore( nNumRefs * 6 );     // nur letzte Selektion interessiert
        aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
        pColRowBuff->SetSelection( ScRange( ( UINT16 ) nFirstCol, nFirstRow, GetScTab(),
                                            ( UINT16 ) nLastCol, nLastRow, GetScTab() ) );
    }
}


void ImportExcel::Columndefault( void )
{// Default Cell Attributes
    UINT16  nColMic, nColMac;
    BYTE    nOpt0;

    aIn >> nColMic >> nColMac;

    DBG_ASSERT( aIn.GetRecLeft() - 4 == (ULONG)(nColMac - nColMic) * 3,
                "+ImportExcel::Columndefault(): Wie jetzt?!!..." );

    nColMac--;

    if( nColMac > MAXCOL )
        nColMac = MAXCOL;

    for( UINT16 nCol = nColMic ; nCol <= nColMac ; nCol++ )
    {
        aIn >> nOpt0;
        aIn.Ignore( 2 );    // nur 0. Attribut-Byte benutzt

        if( nOpt0 & 0x80 )  // Col hidden?
            pColRowBuff->HideCol( nCol );
    }
}


void ImportExcel::Array25( void )
{
    UINT16      nFirstRow, nLastRow, nFormLen;
    BYTE        nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {//                     BIFF2
        aIn.Ignore( 1 );
        nFormLen = aIn.ReaduInt8();
    }
    else
    {//                     BIFF5
        aIn.Ignore( 6 );
        aIn >> nFormLen;
    }

    if( nLastRow <= MAXROW && nLastCol <= MAXCOL )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;

        pFormConv->Reset( ScAddress( nFirstCol, nFirstRow, GetScTab() ) );
        pFormConv->Convert( pErgebnis, nFormLen );

        DBG_ASSERT( pErgebnis, "*ImportExcel::Array25(): ScTokenArray ist NULL!" );

        ScMarkData          aMarkData;
        aMarkData.SelectOneTable( GetScTab() );
        pD->InsertMatrixFormula(
            nFirstCol, nFirstRow, nLastCol, nLastRow, aMarkData,
            EMPTY_STRING, pErgebnis );

/*      UINT16              nRowCnt, nColCnt;

        for( nColCnt = nFirstCol + 1 ; nColCnt <= nLastCol ; nColCnt++ )
            for( nRowCnt = nFirstRow ; nRowCnt <= nLastRow ; nRowCnt++ )
            {
                pCellStyleBuffer->SetXF( nColCnt, nRowCnt, nLastXF );
                pColRowBuff->Used( nColCnt, nRowCnt );
            }*/
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

    String aName( aIn.ReadByteString( FALSE ) );

    if( ( nOpt & 0x0001 ) || ( ( nOpt & 0xFFFE ) == 0x0000 ) )
    {// external name
        ScfTools::ConvertToScDefinedName( aName );
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

    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( nColWidth, GetCharWidth() );
    pColRowBuff->SetWidthRange( nColFirst, nColLast, nScWidth );
}


void ImportExcel::Defrowheight2( void )
{
    UINT16  nDef;

    aIn >> nDef;

    nDef &=0x7FFF;
    nDef = ( UINT16 ) ( ( double ) nDef * pExcRoot->fRowScale );

    pColRowBuff->SetDefHeight( nDef );
}


void ImportExcel::Leftmargin( void )
{
    SetMarginItem( *pStyleSheetItemSet, aIn.ReadDouble(), xlLeftMargin );
    bHasLeftMargin = sal_True;
}


void ImportExcel::Rightmargin( void )
{
    SetMarginItem( *pStyleSheetItemSet, aIn.ReadDouble(), xlRightMargin );
    bHasRightMargin = sal_True;
}


void ImportExcel::Topmargin( void )
{
    SetMarginItem( *pStyleSheetItemSet, aIn.ReadDouble(), xlTopMargin );
    bHasTopMargin = sal_True;
}


void ImportExcel::Bottommargin( void )
{
    SetMarginItem( *pStyleSheetItemSet, aIn.ReadDouble(), xlBottomMargin );
    bHasBottomMargin = sal_True;
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

void ImportExcel::Protect( void )
{
    if( aIn.ReaduInt16() )
    {
        uno::Sequence<sal_Int8> aEmptyPass;
        GetDoc().SetTabProtection( GetScTab(), TRUE, aEmptyPass );
    }
}

void ImportExcel::DocProtect( void )
{
    if( aIn.ReaduInt16() )
    {
        uno::Sequence<sal_Int8> aEmptyPass;
        GetDoc().SetDocProtection( TRUE, aEmptyPass );
    }
}


BOOL ImportExcel::Filepass( void )
{
#if SC_XCL_USEDECR
    if( pExcRoot->eHauptDateiTyp <= Biff5 )
    {
        aIn.UseDecryption( false );
        sal_uInt16 nKey, nHash;
        aIn >> nKey >> nHash;

        bool bValid = (maPassword.Len() > 0);
        if( !bValid )
        {
            if( (nKey == 0xB359) && (nHash == 0x9A0A) )
            {
                // Workbook protection -> password is encoded in PASSWORD record
                maPassword.AssignAscii( "VelvetSweatshop" );
                bValid = true;
            }
        }

        if( bValid )
        {
            XclImpBiff5Decrypter* pDecrypter = new XclImpBiff5Decrypter( maPassword, nKey, nHash );
            bValid = pDecrypter->IsValid();     // validates password
            aIn.EnableDecryption( pDecrypter );
        }
        return !bValid;
    }
    else
#endif
        // POST: return = TRUE, wenn Password <> 0
        return aIn.ReaduInt32() != 0;
}


void ImportExcel::Pane( void )
{
    pColRowBuff->ReadSplit( aIn );
}


void ImportExcel::Codepage( void )
{
    rtl_TextEncoding eEnc = XclTools::GetTextEncoding( maStrm.ReaduInt16() );
    if( eEnc != RTL_TEXTENCODING_DONTKNOW )
        SetCharSet( eEnc );
}


void ImportExcel::Ixfe( void )
{
    aIn >> nIxfeIndex;
}


void ImportExcel::DefColWidth( void )
{
    // stored as entire characters -> convert to 1/256 of characters (as in COLINFO)
    double fDefWidth = 256.0 * maStrm.ReaduInt16();

    // #i3006# additional space for default width - Excel adds space depending on font size
    long nFontHt = GetFontBuffer().GetAppFontData().mnHeight;
    fDefWidth += 40960.0 / ::std::max( nFontHt - 15L, 60L ) + 50.0;
    fDefWidth = ::std::min( fDefWidth, 65535.0 );

    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( static_cast< sal_uInt16 >( fDefWidth ), GetCharWidth() );
    pColRowBuff->SetDefWidth( nScWidth );
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

    bool bHidden = ::get_flag( nOpt, EXC_COLINFO_HIDDEN );
    bool bCollapsed = ::get_flag( nOpt, EXC_COLINFO_COLLAPSED );
    sal_uInt8 nLevel;
    ::extract_value( nLevel, nOpt, 8, 3 );
    pColOutlineBuff->SetLevelRange( nColFirst, nColLast, nLevel, bCollapsed, bHidden );

    if( bHidden )
        pColRowBuff->HideColRange( nColFirst, nColLast );

    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( nColWidth, GetCharWidth() );
    pColRowBuff->SetWidthRange( nColFirst, nColLast, nScWidth );
    pColRowBuff->SetDefaultXF( nColFirst, nColLast, nXF );
}


void ImportExcel::Rk( void )
{// Cell Value, RK Number
    UINT16              nRow, nCol, nXF;
    INT32               nRkNum;

    aIn >> nRow >> nCol >> nXF >> nRkNum;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        ScBaseCell* pCell = new ScValueCell( XclTools::GetDoubleFromRK( nRkNum ) );
        GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Wsbool( void )
{
    UINT16 nFlags;
    aIn >> nFlags;

    pRowOutlineBuff->SetButtonMode( HasFlag( nFlags, EXC_WSBOOL_ROWBELOW ) );
    pColOutlineBuff->SetButtonMode( HasFlag( nFlags, EXC_WSBOOL_COLBELOW ) );

    bFitToPage = HasFlag( nFlags, EXC_WSBOOL_FITTOPAGE );
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
        aIn.Ignore( 4 );
        aIn >> nGrbit;
    }
    else
        nGrbit = 0x0000;

    String aName( aIn.ReadByteString( FALSE ) );
    ScfTools::ConvertToScSheetName( aName );

    *pExcRoot->pTabNameBuff << aName;

    if( nBdshtTab > 0 )
    {
        DBG_ASSERT( !pD->HasTable( nBdshtTab ), "*ImportExcel::Boundsheet(): Tabelle schon vorhanden!" );

        pD->MakeTable( nBdshtTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nBdshtTab, FALSE );
    else if( nFirstVisTab == 0xFFFF )
        nFirstVisTab = nBdshtTab;       // first visible for WINDOW2 import

    pD->RenameTab( nBdshtTab, aName );
    nBdshtTab++;
}


void ImportExcel::Country( void )
{
    sal_uInt16 nUICountry, nDocCountry;
    maStrm >> nUICountry >> nDocCountry;

    // Store system language in XclRoot
    LanguageType eLanguage = ::svx::ConvertCountryToLanguage( static_cast< ::svx::CountryId >( nDocCountry ) );
    if( eLanguage != LANGUAGE_DONTKNOW )
        SetDocLanguage( eLanguage );

    // Set Excel UI language in add-in name translator
    eLanguage = ::svx::ConvertCountryToLanguage( static_cast< ::svx::CountryId >( nUICountry ) );
    if( eLanguage != LANGUAGE_DONTKNOW )
        SetUILanguage( eLanguage );
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
    GetPalette().ReadPalette( maStrm );
}


void ImportExcel::Standardwidth( void )
{
    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( maStrm.ReaduInt16(), GetCharWidth() );
    pColRowBuff->SetDefWidth( nScWidth, TRUE );
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

        if( aSize.nA == 0 || aSize.nB == 0 )
            // now try default from printer
            aSize = SvxPaperInfo::GetPaperSize( pD->GetPrinter() );

        if( bLandscape )
        {
            long    nTmp;
            nTmp = aSize.nA;
            aSize.nA = aSize.nB;
            aSize.nB = nTmp;
        }

        pStyleSheetItemSet->Put( SvxSizeItem( ATTR_PAGE_SIZE, aSize ) );

        // bFitToPage set in WSBOOL record. For now assuming that it always occurs before SETUP.
        if( !bFitToPage || !nFitWidth || !nFitHeight )
            pStyleSheetItemSet->Put( SfxUInt16Item( ATTR_PAGE_SCALE, nScale ) );
    }

    if( nOpt & 0x0020 )
        // fNotes
        pStyleSheetItemSet->Put( SfxBoolItem( ATTR_PAGE_NOTES, TRUE ) );

    // fUsePage
    pStyleSheetItemSet->Put( SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, ( nOpt & 0x0080 )? nStartPage : 0 ) );

    pStyleSheetItemSet->Put( aPageItem );

    // bFitToPage set in WSBOOL record. For now assuming that it always occurs before SETUP.
    if( bFitToPage && nFitWidth && nFitHeight )
        pStyleSheetItemSet->Put( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, nFitWidth * nFitHeight ) );
}


void ImportExcel::Setup5()
{
    Setup();
    aIn.Seek( 16 );

    // header margin
    SvxSetItem aHeaderSetItem( (const SvxSetItem&) pStyleSheetItemSet->Get( ATTR_PAGE_HEADERSET ) );
    SetMarginItem( aHeaderSetItem.GetItemSet(), aIn.ReadDouble(), xlBottomMargin );
    pStyleSheetItemSet->Put( aHeaderSetItem );

    // footer margin
    SvxSetItem aFooterSetItem( (const SvxSetItem&) pStyleSheetItemSet->Get( ATTR_PAGE_FOOTERSET ) );
    SetMarginItem( aFooterSetItem.GetItemSet(), aIn.ReadDouble(), xlTopMargin );
    pStyleSheetItemSet->Put( aFooterSetItem );
}


void ImportExcel::Shrfmla( void )
{
    UINT16              nFirstRow, nLastRow, nLenExpr;
    BYTE                nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
    aIn.Ignore( 2 );
    aIn >> nLenExpr;

    // jetzt steht Lesemarke an der Formel

    const ScTokenArray* pErgebnis;

    pFormConv->Reset();
    pFormConv->Convert( pErgebnis, nLenExpr, FT_SharedFormula );


    DBG_ASSERT( pErgebnis, "+ImportExcel::Shrfmla(): ScTokenArray ist NULL!" );

    pExcRoot->pShrfmlaBuff->Store(
            ScRange( nFirstCol, nFirstRow, GetScTab(), nLastCol, nLastRow, GetScTab() ),
            *pErgebnis );

    pLastFormCell = NULL;
}


void ImportExcel::Mulrk( void )
{
    UINT16  nRow, nColFirst, nXF;
    INT32   nRkNum;

    aIn >> nRow >> nColFirst;

    if( nRow <= MAXROW )
    {
        for( UINT16 nCol = nColFirst ; aIn.GetRecLeft() > 2 ; nCol++ )
        {
            aIn >> nXF >> nRkNum;

            if( nCol <= MAXCOL )
            {
                GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

                ScBaseCell* pCell = new ScValueCell( XclTools::GetDoubleFromRK( nRkNum ) );
                GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

                pColRowBuff->Used( nCol, nRow );
            }
        }
        DBG_ASSERT( aIn.GetRecLeft() == 2, "+ImportExcel::Mulrk(): Was'n das?!!!" );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Mulblank( void )
{
    UINT16  nRow, nCol, nColFirst, nXF;

    aIn >> nRow >> nColFirst;

    if( nRow <= MAXROW )
    {
        for( nCol = nColFirst ; aIn.GetRecLeft() > 2 ; nCol++ )
        {
            aIn >> nXF;

            if( nCol <= MAXCOL )
            {
                pColRowBuff->Used( nCol, nRow );
                GetXFIndexBuffer().SetBlankXF( nCol, nRow, nXF );
            }
        }
        aIn >> nRow;    // nRow zum Testen von letzter Col missbraucht
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Rstring( void )
{
    UINT16      nRow, nCol, nXF;
    BYTE        nCount;

    aIn >> nRow >> nCol >> nXF;

    String aString( aIn.ReadByteString( TRUE ) );

    aIn >> nCount;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        if( aString.Len() )
        {
            EditTextObject* pTextObj = CreateFormText( nCount, aString, nXF );

            ScBaseCell*     pZelle = new ScEditCell( pTextObj, pD, GetEditEngine().GetEditTextObjectPool() );

            delete pTextObj;

            GetDoc().PutCell( nCol, nRow, GetScTab(), pZelle );
        }
        pColRowBuff->Used( nCol, nRow );
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Olesize( void )
{
    aIn.Ignore( 2 );
    UINT16  nFirstRow, nLastRow;
    UINT8   nFirstCol, nLastCol;
    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
    pExcRoot->pExtDocOpt->SetOleSize( nFirstCol, nFirstRow, nLastCol, nLastRow );
}


void ImportExcel::Blank34( void )
{
    UINT16  nRow, nCol, nXF;

    aIn >> nRow >> nCol >> nXF;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        pColRowBuff->Used( nCol, nRow );
        GetXFIndexBuffer().SetBlankXF( nCol, nRow, nXF );
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
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        ScBaseCell* pCell = new ScValueCell( fValue );
        GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Label34( void )
{
    UINT16  nR, nC, nXF;

    aIn >> nR >> nC >> nXF;

    String aTmpStr( aIn.ReadByteString( TRUE ) );
    SetTextCell( nC, nR, aTmpStr, nXF );

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
            pD, ScAddress( nCol, nRow, GetScTab() ), pErgebnis );

        pZelle->SetDouble( fVal );

        GetDoc().PutCell( nCol, nRow, GetScTab(), pZelle );
        pColRowBuff->Used( nCol, nRow );

        if( bErrOrVal )     // !=0 -> Error
            GetXFIndexBuffer().SetXF( nCol, nRow, nXF );
        else                // ==0 -> Boolean
            GetXFIndexBuffer().SetBoolXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Row34( void )
{
    UINT16  nRow, nRowHeight, nGrbit, nXF;

    aIn >> nRow;
    aIn.Ignore( 4 );    // Mic und Mac ueberspringen

    if( nRow <= MAXROW )
    {
        aIn >> nRowHeight;  // direkt in Twips angegeben
        aIn.Ignore( 4 );

        aIn >> nGrbit >> nXF;

        pRowOutlineBuff->SetLevel( nRow, EXC_ROW_GETLEVEL( nGrbit ),
            TRUEBOOL( nGrbit & EXC_ROW_COLLAPSED ), TRUEBOOL( nGrbit & EXC_ROW_ZEROHEIGHT ) );

        pColRowBuff->SetRowSettings( nRow, nRowHeight, nGrbit );

        if( nGrbit & EXC_ROW_GHOSTDIRTY )
            GetXFIndexBuffer().SetRowDefXF( nRow, nXF & EXC_ROW_XFMASK );
    }
}


void ImportExcel::Bof3( void )
{
    sal_uInt16 nSubType;
#if SC_XCL_USEDECR
    maStrm.UseDecryption( false );
#endif
    maStrm.Ignore( 2 );
    maStrm >> nSubType;
    SetBiff( xlBiff3 );
#if SC_XCL_USEDECR
    maStrm.UseDecryption( true );
#endif

    DBG_ASSERT( nSubType != 0x0100, "*ImportExcel::Bof3(): Biff3 als Workbook?!" );
    pExcRoot->eHauptDateiTyp = Biff3;
    if( nSubType == 0x0010 )        // Sheet?
        pExcRoot->eDateiTyp = Biff3;
    else if( nSubType == 0x0100 )   // Book?
        pExcRoot->eDateiTyp = Biff3W;
    else if( nSubType == 0x0020 )   // Chart?
        pExcRoot->eDateiTyp = Biff3C;
    else if( nSubType == 0x0040 )   // Macro?
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
    aIn.Ignore( 1 );
    aIn >> nLenName >> nLenExpr;

    // Namen einlesen
    String aName( aIn.ReadRawByteString( nLenName ) );
    sal_Char cFirstNameChar = ( sal_Char ) aName.GetChar( 0 );

    if( nLenName == 1 && cFirstNameChar < EXC_BUILTIN_UNKNOWN )
    {// Build-in name (0x00 - 0x0D als erstes Zeichen)
        bPrintArea = ( cFirstNameChar == EXC_BUILTIN_PRINTAREA );
        bPrintTitles = ( cFirstNameChar == EXC_BUILTIN_PRINTTITLES );
        bBuildIn = TRUE;

        aName = XclTools::GetBuiltInName( cFirstNameChar );
    }
    else
    {
        ScfTools::ConvertToScDefinedName( aName );

        bPrintArea = bPrintTitles = bBuildIn = FALSE;
    }

    const BOOL          bHidden = TRUEBOOL( nAttr & EXC_NAME_HIDDEN );

    pFormConv->Reset();
    if( nAttr & EXC_NAME_VB )
        // function, command or name on macro sheet?
        pFormConv->GetDummy( pErgebnis );
    else
    {
        if( bBuildIn )
        {
            aIn.PushPosition();

            if( bPrintArea )
                pFormConv->Convert( *pPrintRanges, nLenExpr, FT_RangeName );
            else if( bPrintTitles )
                pFormConv->Convert( *pPrintTitles, nLenExpr, FT_RangeName );

            aIn.PopPosition();
        }
        pFormConv->Convert( pErgebnis, nLenExpr, FT_RangeName );
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
    aIn.Ignore( (pExcRoot->eHauptDateiTyp >= Biff5) ? 6 : 2 );
    aIn >> nFormLen;

    if( nLastRow <= MAXROW && nLastCol <= MAXCOL )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;

        pFormConv->Reset( ScAddress( nFirstCol, nFirstRow, GetScTab() ) );
        pFormConv->Convert( pErgebnis, nFormLen );

        DBG_ASSERT( pErgebnis, "+ImportExcel::Array34(): ScTokenArray ist NULL!" );

        ScMarkData          aMarkData;
        aMarkData.SelectOneTable( GetScTab() );
        pD->InsertMatrixFormula( nFirstCol, nFirstRow, nLastCol, nLastRow, aMarkData, EMPTY_STRING, pErgebnis);

/*      UINT16              nRowCnt, nColCnt;

        for( nColCnt = nFirstCol + 1 ; nColCnt <= nLastCol ; nColCnt++ )
            for( nRowCnt = nFirstRow ; nRowCnt <= nLastRow ; nRowCnt++ )
            {
                pCellStyleBuffer->SetXF( nColCnt, nRowCnt, nLastXF );
                pColRowBuff->Used( nColCnt, nRowCnt );
            }*/
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

    if( nOpt & 0x0002 )
        pColRowBuff->SetDefHeight( 0 );
    else
        pColRowBuff->SetDefHeight( nDef );
}


void ImportExcel::TableOp( void )
{
    UINT16 nFirstRow, nLastRow;
    UINT8 nFirstCol, nLastCol;
    UINT16 nGrbit;
    UINT16 nInpRow, nInpCol, nInpRow2, nInpCol2;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol >> nGrbit
        >> nInpRow >> nInpCol >> nInpRow2 >> nInpCol2;

    if( (nLastRow <= MAXROW) && (nLastCol <= MAXCOL) )
    {
        if( nFirstCol && nFirstRow )
        {
            ScTabOpParam aTabOpParam;
            aTabOpParam.nMode = (nGrbit & EXC_TABOP_BOTH) ? 2 : ((nGrbit & EXC_TABOP_ROW) ? 1 : 0 );
            USHORT nCol = nFirstCol - 1;
            USHORT nRow = nFirstRow - 1;
            sal_uInt16 nTab = GetScTab();
            switch( aTabOpParam.nMode )
            {
                case 0:     // COL
                    aTabOpParam.aRefFormulaCell.Put( nFirstCol, nFirstRow - 1, nTab, FALSE, FALSE, FALSE );
                    aTabOpParam.aRefFormulaEnd.Put( nLastCol, nFirstRow - 1, nTab, FALSE, FALSE, FALSE );
                    aTabOpParam.aRefColCell.Put( nInpCol, nInpRow, nTab, FALSE, FALSE, FALSE );
                    nRow++;
                break;
                case 1:     // ROW
                    aTabOpParam.aRefFormulaCell.Put( nFirstCol - 1, nFirstRow, nTab, FALSE, FALSE, FALSE );
                    aTabOpParam.aRefFormulaEnd.Put( nFirstCol - 1, nLastRow, nTab, FALSE, FALSE, FALSE );
                    aTabOpParam.aRefRowCell.Put( nInpCol, nInpRow, nTab, FALSE, FALSE, FALSE );
                    nCol++;
                break;
                case 2:     // TWO-INPUT
                    aTabOpParam.aRefFormulaCell.Put( nFirstCol - 1, nFirstRow - 1, nTab, FALSE, FALSE, FALSE );
                    aTabOpParam.aRefRowCell.Put( nInpCol, nInpRow, nTab, FALSE, FALSE, FALSE );
                    aTabOpParam.aRefColCell.Put( nInpCol2, nInpRow2, nTab, FALSE, FALSE, FALSE );
                break;
            }

            ScMarkData aMarkData;
            aMarkData.SelectOneTable( nTab );
            pD->InsertTableOp( aTabOpParam, nCol, nRow, nLastCol, nLastRow, aMarkData );
        }
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel::Window2_5( void )
{
    ScExtDocOptions&    rExtOpt = *pExcRoot->pExtDocOpt;
    UINT16              nOpt, nRow, nCol;
    UINT32              nColorIndex;

    aIn >> nOpt >> nRow >> nCol >> nColorIndex;

    nRow = Min( nRow, (UINT16)MAXROW );
    nCol = Min( nCol, (UINT16)MAXCOL );
    pColRowBuff->SetVisCorner( nCol, nRow );

    if( nOpt & EXC_WIN2_DISPLAYED )
        rExtOpt.SetActTab( GetScTab() );
    pColRowBuff->SetTabSelected( TRUEBOOL( nOpt & EXC_WIN2_SELECTED ) );
    pColRowBuff->SetFrozen( TRUEBOOL( nOpt & EXC_WIN2_FROZEN ) );

    if( GetScTab() == nFirstVisTab )     // import from first visible sheet
    {
        if( !( nOpt & EXC_WIN2_DEFAULTCOLOR ) )
            rExtOpt.SetGridCol( GetPalette().GetColor( static_cast< sal_Int16 >( nColorIndex ) ) );

        ScViewOptions aOpts( pD->GetViewOptions() );
        aOpts.SetOption( VOPT_FORMULAS, TRUEBOOL( nOpt & EXC_WIN2_SHOWFORMULAS ) );
        aOpts.SetOption( VOPT_GRID, TRUEBOOL( nOpt & EXC_WIN2_SHOWGRID ) );
        aOpts.SetOption( VOPT_HEADER, TRUEBOOL( nOpt & EXC_WIN2_SHOWHEADINGS ) );
        aOpts.SetOption( VOPT_NULLVALS, TRUEBOOL( nOpt & EXC_WIN2_SHOWZEROS ) );
        aOpts.SetOption( VOPT_OUTLINER, TRUEBOOL( nOpt & EXC_WIN2_OUTLINE ) );
        pD->SetViewOptions( aOpts );
    }
}


void ImportExcel::Bof4( void )
{
    sal_uInt16 nSubType;
#if SC_XCL_USEDECR
    maStrm.UseDecryption( false );
#endif
    maStrm.Ignore( 2 );
    maStrm >> nSubType;
    SetBiff( xlBiff4 );
#if SC_XCL_USEDECR
    maStrm.UseDecryption( true );
#endif

    pExcRoot->eHauptDateiTyp = Biff4;
    if( nSubType == 0x0010 )        // Sheet?
        pExcRoot->eDateiTyp = Biff4;
    else if( nSubType == 0x0100 )   // Book?
        pExcRoot->eDateiTyp = Biff4W;
    else if( nSubType == 0x0020 )   // Chart?
        pExcRoot->eDateiTyp = Biff4C;
    else if( nSubType == 0x0040 )   // Macro?
        pExcRoot->eDateiTyp = Biff4M;
    else
    {
        pExcRoot->eDateiTyp = BiffX;
        pExcRoot->eHauptDateiTyp = BiffX;
    }
}


void ImportExcel::Bof5( void )
{
    //POST: eDateiTyp = Typ der zu lesenden Datei
    UINT16      nSubType, nVers;
    BiffTyp     eHaupt = Biff5;
    BiffTyp     eDatei;

#if SC_XCL_USEDECR
    maStrm.UseDecryption( false );
#endif
    maStrm >> nVers >> nSubType;
    if( nSubType == 0x0005 )    // nVers may be wrong in Worksheet BOFs
        SetBiff( (nVers == 0x0600) ? xlBiff8 : xlBiff5 );
#if SC_XCL_USEDECR
    maStrm.UseDecryption( true );
#endif

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

    if( nVers == 0x0600 && pExcRoot->eHauptDateiTyp != Biff5 )
    {// Biff8
        eHaupt = ( BiffTyp ) ( eHaupt - Biff5 + Biff8 );
        eDatei = ( BiffTyp ) ( eDatei - Biff5 + Biff8 );
    }

    pExcRoot->eHauptDateiTyp = eHaupt;
    pExcRoot->eDateiTyp = eDatei;
}

void ImportExcel::EndSheet( void )
{
    pColRowBuff->Apply( GetScTab() );
    GetXFIndexBuffer().Apply();

    pExcRoot->pExtSheetBuff->Reset();

    if( pExcRoot->eHauptDateiTyp < Biff8 )
        pExcRoot->pExtNameBuff->Reset();

    // no top(upper)/bottom(lower) MARGIN record
    // note zero is a legitimate value
    if( !bHasTopMargin || !bHasBottomMargin )
    {
        SvxULSpaceItem aItem( (const SvxULSpaceItem&) pStyleSheetItemSet->Get( ATTR_ULSPACE ));
        sal_uInt16 nMarginULTwips = EXC_ULMARGIN_DEFAULT_TWIPS;
        if( !bHasTopMargin )
            aItem.SetUpperValue( nMarginULTwips );
        if( !bHasBottomMargin )
            aItem.SetLowerValue( nMarginULTwips );
        pStyleSheetItemSet->Put( aItem );
    }

    // no left/right MARGIN record
    // note zero is a legitimate value
    if( !bHasLeftMargin || !bHasRightMargin )
    {
        SvxLRSpaceItem aItem( (const SvxLRSpaceItem&) pStyleSheetItemSet->Get( ATTR_LRSPACE ));
        sal_uInt16 nMarginLRTwips = EXC_LRMARGIN_DEFAULT_TWIPS;
        if( !bHasLeftMargin )
            aItem.SetLeftValue( nMarginLRTwips );
        if( !bHasRightMargin )
            aItem.SetRightValue( nMarginLRTwips );
        pStyleSheetItemSet->Put( aItem );
    }

    // no or empty HEADER record
    if( !bHasHeader )
    {
        SvxSetItem aHeaderSetItem( (const SvxSetItem&) pStyleSheetItemSet->Get( ATTR_PAGE_HEADERSET ) );
        aHeaderSetItem.GetItemSet().Put( SfxBoolItem( ATTR_PAGE_ON, sal_False ) );
        pStyleSheetItemSet->Put( aHeaderSetItem );
    }
    // no or empty FOOTER record
    if( !bHasFooter )
    {
        SvxSetItem aFooterSetItem( (const SvxSetItem&) pStyleSheetItemSet->Get( ATTR_PAGE_FOOTERSET ) );
        aFooterSetItem.GetItemSet().Put( SfxBoolItem( ATTR_PAGE_ON, sal_False ) );
        pStyleSheetItemSet->Put( aFooterSetItem );
    }
}


void ImportExcel::NeueTabelle( void )
{
    sal_uInt16 nTab = GetScTab();
    if( nTab > 0 && !pD->HasTable( nTab ) )
        pD->MakeTable( nTab );

    // fuer neue Tabelle eigene Tabellenvorlage erzeugen
    String      aStyleName( GetPageStyleName( nTab ) );

    ScStyleSheetPool* pStyleShPool = pD->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSh = pStyleShPool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
    pExcRoot->pStyleSheetItemSet = pStyleSheetItemSet = pStyleSh ? &pStyleSh->GetItemSet() :
        &pStyleShPool->Make( aStyleName, SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_USERDEF ).GetItemSet();

    pExcRoot->bDefaultPage = TRUE;

    bFitToPage = sal_False;
    bHasHeader = sal_False;
    bHasFooter = sal_False;
    bHasTopMargin    = sal_False;
    bHasBottomMargin = sal_False;
    bHasLeftMargin   = sal_False;
    bHasRightMargin  = sal_False;

    pOutlineListBuffer->Append(new OutlineDataBuffer(*pExcRoot, nTab ));          //#94039# prevent empty rootdata

    pColRowBuff = pOutlineListBuffer->Last()->GetColRowBuff();
    pColOutlineBuff = pOutlineListBuffer->Last()->GetColOutline();
    pRowOutlineBuff = pOutlineListBuffer->Last()->GetRowOutline();
}


const ScTokenArray* ImportExcel::ErrorToFormula( BYTE bErrOrVal, BYTE nError, double& rVal )
{
    return pFormConv->GetBoolErr( XclTools::ErrorToEnum( rVal, bErrOrVal, nError ) );
}


void ImportExcel::GetHF( BOOL bHeader )
{
    String aExcHF;
    GetHFString( aExcHF );

    ::std::auto_ptr< ScPageHFItem > pHFItemLeft, pHFItemRight;
    if( bHeader )
    {
        pHFItemLeft.reset( new ScPageHFItem( ATTR_PAGE_HEADERLEFT ) );
        pHFItemRight.reset( new ScPageHFItem( ATTR_PAGE_HEADERRIGHT ) );
    }
    else
    {
        pHFItemLeft.reset( new ScPageHFItem( ATTR_PAGE_FOOTERLEFT ) );
        pHFItemRight.reset( new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT ) );
    }

    XclImpHFConverter aHFConv( *this );
    aHFConv.ParseString( aExcHF );
    const EditTextObject* pTextObj = aHFConv.GetTextObject( xlHFLeft );
    if( pTextObj )
    {
        pHFItemLeft->SetLeftArea( *pTextObj );
        pHFItemRight->SetLeftArea( *pTextObj );
    }
    pTextObj = aHFConv.GetTextObject( xlHFCenter );
    if( pTextObj )
    {
        pHFItemLeft->SetCenterArea( *pTextObj );
        pHFItemRight->SetCenterArea( *pTextObj );
    }
    pTextObj = aHFConv.GetTextObject( xlHFRight );
    if( pTextObj )
    {
        pHFItemLeft->SetRightArea( *pTextObj );
        pHFItemRight->SetRightArea( *pTextObj );
    }

    DBG_ASSERT( pStyleSheetItemSet, "-ImportExcel::SetHF(): kein Set, keine Kopfer/Fusser!" );
    pStyleSheetItemSet->Put( *pHFItemLeft );
    pStyleSheetItemSet->Put( *pHFItemRight );
}


void ImportExcel::GetHFString( String& rStr )
{
    aIn.AppendByteString( rStr, FALSE );
}


String ImportExcel::GetPageStyleName( UINT16 nTab )
{
    String          aRet( RTL_CONSTASCII_USTRINGPARAM( "TAB_" ) );
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
    EditEngine&     rEdEng = GetEditEngine();

    rEdEng.SetText( rS );

    SfxItemSet      aItemSet( rEdEng.GetEmptyItemSet() );
    GetFontBuffer().FillToItemSet( aItemSet, xlFontEEIDs, GetXFBuffer().GetFontIndex( nXF ) );

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

                GetFontBuffer().FillToItemSet( aItemSet, xlFontEEIDs, nFont );
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


void ImportExcel::AdjustRowHeight()
{
    // #93255# speed up chart import: import all sheets without charts, then
    // update row heights (here), last load all charts -> do not any longer
    // update inside of ScDocShell::ConvertFrom() (causes update of existing
    // charts during each and every change of row height)
    ScModelObj* pDocObj = GetDocModelObj();
    if( pDocObj )
    {
        for( sal_uInt16 nTab = 0; nTab < GetDoc().GetTableCount(); ++nTab )
            pDocObj->AdjustRowHeight( 0, MAXROW, nTab );
    }
}


void ImportExcel::PostDocLoad( void )
{

    // Apply any Outlines for each sheet
    for(OutlineDataBuffer* pBuffer = pOutlineListBuffer->First(); pBuffer; pBuffer = pOutlineListBuffer->Next() )
        pBuffer->Apply(pD);


    // visible area if embedded OLE
    ScModelObj* pDocObj = GetDocModelObj();
    if( pDocObj )
    {
        SvEmbeddedObject* pEmbObj = pDocObj->GetEmbeddedObject();
        const ScRange* pOleSize = pExcRoot->pExtDocOpt->GetOleSize();
        if( pEmbObj && pOleSize )
        {
            pEmbObj->SetVisArea( GetDoc().GetMMRect(
                pOleSize->aStart.Col(), pOleSize->aStart.Row(),
                pOleSize->aEnd.Col(), pOleSize->aEnd.Row(), pExcRoot->pExtDocOpt->nActTab ) );
            GetDoc().SetVisibleTab( pExcRoot->pExtDocOpt->nActTab );
        }

        // #111099# open forms in alive mode (has no effect, if no controls in document)
        pDocObj->setPropertyValue( CREATE_OUSTRING( SC_UNO_APPLYFMDES ), ::comphelper::makeBoolAny( sal_False ) );
    }

    pD->SetExtDocOptions( pExcRoot->pExtDocOpt );
    pExcRoot->pExtDocOpt = NULL;

    EndAllChartObjects();

    GetDoc().UpdateAllCharts();

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


void ImportExcel::SetTextCell( const UINT16 nC, const UINT16 nR, String& r, const UINT16 nXF )
{
    if( nR <= MAXROW && nC <= MAXCOL )
    {
        if( r.Len() )
        {
            ScBaseCell*             pZelle;

            if( GetXFBuffer().HasEscapement( nXF ) )
            {// jetzt kommt 'ne Edit-Engine in's Spiel!
                EditTextObject*     pTObj = CreateFormText( 0, r, nXF );

                pZelle = new ScEditCell( pTObj, pD, GetEditEngine().GetEditTextObjectPool() );

                delete pTObj;
            }
            else
                pZelle = ScBaseCell::CreateTextCell( r, pD );

            GetDoc().PutCell( nC, nR, GetScTab(), pZelle );
        }
        pColRowBuff->Used( nC, nR );
        GetXFIndexBuffer().SetXF( nC, nR, nXF );
    }
    else
        bTabTruncated = TRUE;
}


void ImportExcel::SetMarginItem( SfxItemSet& rItemSet, double fMarginInch, XclMarginType eType )
{
    sal_uInt16 nMarginTwips = XclTools::GetTwipsFromInch( fMarginInch );
    if( (eType == xlTopMargin) || (eType == xlBottomMargin) )
    {
        SvxULSpaceItem aItem( ATTR_ULSPACE );
        aItem = (const SvxULSpaceItem&) rItemSet.Get( ATTR_ULSPACE );
        if( eType == xlTopMargin )
            aItem.SetUpperValue( nMarginTwips );
        else
            aItem.SetLowerValue( nMarginTwips );
        rItemSet.Put( aItem );
    }
    else
    {
        SvxLRSpaceItem aItem( ATTR_LRSPACE );
        aItem = (const SvxLRSpaceItem&) rItemSet.Get( ATTR_LRSPACE );
        if( eType == xlRightMargin )
            aItem.SetRightValue( nMarginTwips );
        else
            aItem.SetLeftValue( nMarginTwips );
        rItemSet.Put( aItem );
    }
}

OutlineDataBuffer::OutlineDataBuffer(RootData& rRootData, UINT16 nTabNo) :
    nTab (nTabNo)
{
    pColOutlineBuff = new OutlineBuffer (MAXCOL + 1);
    pRowOutlineBuff = new OutlineBuffer (MAXROW + 1);
    pColRowBuff = new ColRowSettings( rRootData );

    pColRowBuff->SetDefWidth( STD_COL_WIDTH );
    pColRowBuff->SetDefHeight( ( UINT16 ) STD_ROW_HEIGHT );
}

void OutlineDataBuffer::Apply(ScDocument* pD)
{
    pColOutlineBuff->SetOutlineArray( pD->GetOutlineTable( nTab, TRUE )->GetColArray() );
    pColOutlineBuff->MakeScOutline();

    pRowOutlineBuff->SetOutlineArray( pD->GetOutlineTable( nTab, TRUE )->GetRowArray() );
    pRowOutlineBuff->MakeScOutline();

    pColRowBuff->SetHiddenFlags(nTab);
}
