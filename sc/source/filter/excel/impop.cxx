/*************************************************************************
 *
 *  $RCSfile: impop.cxx,v $
 *
 *  $Revision: 1.63 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:52:01 $
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

#include "paramisc.hxx"
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
#include <svx/pageitem.hxx>
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
#ifndef SC_XIPAGE_HXX
#include "xipage.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
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




ImportExcel::ImportExcel( SfxMedium& rMedium, SvStream& rSvStrm, XclBiff eBiff, ScDocument* pDoc ):
    XclImpRootData( eBiff, rMedium, *pDoc, RTL_TEXTENCODING_MS_1252 ),
    ImportTyp( pDoc, RTL_TEXTENCODING_MS_1252 ),
    XclImpRoot( static_cast< XclImpRootData& >( *this ) ),
    maStrm( rSvStrm, *this ),
    aIn( maStrm )
{
    pChart = pUsedChartFirst = pUsedChartLast = NULL;

    nBdshtTab = 0;
    nFirstVisTab = 0xFFFF;
    nIxfeIndex = 0;     // zur Sicherheit auf 0

    // Root-Daten fuellen - nach new's ohne Root als Parameter
    pExcRoot = mpRD;
    pExcRoot->pDoc = pDoc;
    pExcRoot->pIR = this;   // ExcRoot -> XclImpRoot
    pExcRoot->pScRangeName = pDoc->GetRangeName();
    pExcRoot->aStandard.AssignAscii( "General" );
    pExcRoot->eDateiTyp = pExcRoot->eHauptDateiTyp = BiffX;
    pExcRoot->pExtSheetBuff = new ExtSheetBuffer( pExcRoot );   //&aExtSheetBuff;
    pExcRoot->pTabNameBuff = new NameBuffer( pExcRoot );        //&aTabNameBuff;
    pExcRoot->pShrfmlaBuff = new ShrfmlaBuffer( pExcRoot );     //&aShrfrmlaBuff;
    pExcRoot->pExtNameBuff = new ExtNameBuff ( pExcRoot );
    pExcRoot->pCharset = &eQuellChar;   // dto.
    GetExtDocOptions().SetChanged( TRUE );

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
        pColRowBuff->SetDimension( ScRange( static_cast<SCCOL>(nColFirst),
                    static_cast<SCROW>(nRowFirst), GetCurrScTab(),
                    static_cast<SCCOL>(nColLast), static_cast<SCROW>(nRowLast),
                    GetCurrScTab() ) );
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
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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
        GetDoc().PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow),
                GetCurrScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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
        GetDoc().PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow),
                GetCurrScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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
        ScAddress aPos( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), GetCurrScTab());
        ScFormulaCell* pCell = new ScFormulaCell( pD, aPos, pErgebnis );
        pCell->SetDouble( fVal );
        GetDoc().PutCell( aPos, pCell );

        pColRowBuff->Used( nCol, nRow );

        if( bErrOrVal )     // !=0 -> Error
            GetXFIndexBuffer().SetXF( nCol, nRow, nXF );
        else                // ==0 -> Boolean
            GetXFIndexBuffer().SetBoolXF( nCol, nRow, nXF );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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
    maStrm.DisableDecryption();
    maStrm.Ignore( 2 );
    maStrm >> nSubType;

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
    IncCurrScTab();
}


BOOL ImportExcel::Password( void )
{
    // POST: return = TRUE, wenn Password <> 0
    UINT16 nPasswd;

    aIn >> nPasswd;

    return nPasswd != 0x0000;
}


void ImportExcel::Externsheet( void )
{
    String aUrl, aTabName;
    bool bSameWorkBook;
    String aEncodedUrl( aIn.ReadByteString( false ) );
    XclImpUrlHelper::DecodeUrl( aUrl, aTabName, bSameWorkBook, *pExcRoot->pIR, aEncodedUrl );
    ScfTools::ConvertToScSheetName( aTabName );
    pExcRoot->pExtSheetBuff->Add( aUrl, aTabName, bSameWorkBook );
}


void ImportExcel::Note( void )
{
    UINT16  nCol, nRow;
    aIn >> nRow >> nCol;

    if( nRow <= MAXROW && nCol <= MAXCOL )
        pD->SetNote( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow),
                GetCurrScTab(), aIn.ReadByteString( TRUE ) );
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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
        pColRowBuff->SetSelection( ScRange(
            static_cast<SCCOL>(nFirstCol), static_cast<SCROW>(nFirstRow), GetCurrScTab(),
            static_cast<SCCOL>(nLastCol), static_cast<SCROW>(nLastRow), GetCurrScTab()));
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
        nColMac = static_cast<UINT16>(MAXCOL);

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

        pFormConv->Reset( ScAddress( static_cast<SCCOL>(nFirstCol),
                    static_cast<SCROW>(nFirstRow), GetCurrScTab() ) );
        pFormConv->Convert( pErgebnis, nFormLen );

        DBG_ASSERT( pErgebnis, "*ImportExcel::Array25(): ScTokenArray ist NULL!" );

        ScMarkData          aMarkData;
        aMarkData.SelectOneTable( GetCurrScTab() );
        pD->InsertMatrixFormula( static_cast<SCCOL>(nFirstCol),
                static_cast<SCROW>(nFirstRow), static_cast<SCCOL>(nLastCol),
                static_cast<SCROW>(nLastRow), aMarkData, EMPTY_STRING,
                pErgebnis );

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
        nColLast = static_cast<UINT16>(MAXCOL);

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


void ImportExcel::Protect( void )
{
    if( aIn.ReaduInt16() )
    {
        uno::Sequence<sal_Int8> aEmptyPass;
        GetDoc().SetTabProtection( GetCurrScTab(), TRUE, aEmptyPass );
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
        nColLast = static_cast<UINT16>(MAXCOL);

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
        GetDoc().PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow),
                GetCurrScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

    pLastFormCell = NULL;
}


void ImportExcel::Wsbool( void )
{
    UINT16 nFlags;
    aIn >> nFlags;

    pRowOutlineBuff->SetButtonMode( HasFlag( nFlags, EXC_WSBOOL_ROWBELOW ) );
    pColOutlineBuff->SetButtonMode( HasFlag( nFlags, EXC_WSBOOL_COLBELOW ) );

    GetPageSettings().SetFitToPages( ::get_flag( nFlags, EXC_WSBOOL_FITTOPAGE ) );
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

    const SCTAB nTab = static_cast<SCTAB>(nBdshtTab);
    if( nTab > 0 )
    {
        DBG_ASSERT( !pD->HasTable( nTab ), "*ImportExcel::Boundsheet(): Tabelle schon vorhanden!" );

        pD->MakeTable( nTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nTab, FALSE );
    else if( nFirstVisTab == 0xFFFF )
        nFirstVisTab = nBdshtTab;       // first visible for WINDOW2 import

    pD->RenameTab( nTab, aName );
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

    GetExtDocOptions().SetZoom( nNscl, nDscl );
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

    pExcRoot->pShrfmlaBuff->Store( ScRange( static_cast<SCCOL>(nFirstCol),
                static_cast<SCROW>(nFirstRow), GetCurrScTab(),
                static_cast<SCCOL>(nLastCol), static_cast<SCROW>(nLastRow),
                GetCurrScTab()), *pErgebnis );

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
                GetDoc().PutCell( static_cast<SCCOL>(nCol),
                        static_cast<SCROW>(nRow), GetCurrScTab(), pCell );

                pColRowBuff->Used( nCol, nRow );
            }
        }
        DBG_ASSERT( aIn.GetRecLeft() == 2, "+ImportExcel::Mulrk(): Was'n das?!!!" );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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

            GetDoc().PutCell( static_cast<SCCOL>(nCol),
                    static_cast<SCROW>(nRow), GetCurrScTab(), pZelle );
        }
        pColRowBuff->Used( nCol, nRow );
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

    pLastFormCell = NULL;
}


void ImportExcel::Olesize( void )
{
    aIn.Ignore( 2 );
    UINT16  nFirstRow, nLastRow;
    UINT8   nFirstCol, nLastCol;
    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
    GetExtDocOptions().SetOleSize( static_cast<SCCOL>(nFirstCol),
            static_cast<SCROW>(nFirstRow), static_cast<SCCOL>(nLastCol),
            static_cast<SCROW>(nLastRow) );
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
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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
        GetDoc().PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow),
                GetCurrScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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

        ScAddress aPos( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow),
                GetCurrScTab() );

        ScFormulaCell*      pZelle = new ScFormulaCell( pD, aPos, pErgebnis );

        pZelle->SetDouble( fVal );

        GetDoc().PutCell( aPos, pZelle );
        pColRowBuff->Used( nCol, nRow );

        if( bErrOrVal )     // !=0 -> Error
            GetXFIndexBuffer().SetXF( nCol, nRow, nXF );
        else                // ==0 -> Boolean
            GetXFIndexBuffer().SetBoolXF( nCol, nRow, nXF );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
        }

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
    maStrm.DisableDecryption();
    maStrm.Ignore( 2 );
    maStrm >> nSubType;

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

        pFormConv->Reset( ScAddress( static_cast<SCCOL>(nFirstCol),
                    static_cast<SCROW>(nFirstRow), GetCurrScTab() ) );
        pFormConv->Convert( pErgebnis, nFormLen );

        DBG_ASSERT( pErgebnis, "+ImportExcel::Array34(): ScTokenArray ist NULL!" );

        ScMarkData          aMarkData;
        aMarkData.SelectOneTable( GetCurrScTab() );
        pD->InsertMatrixFormula( static_cast<SCCOL>(nFirstCol),
                static_cast<SCROW>(nFirstRow), static_cast<SCCOL>(nLastCol),
                static_cast<SCROW>(nLastRow), aMarkData, EMPTY_STRING,
                pErgebnis);

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
            SCTAB nTab = GetCurrScTab();
            switch( aTabOpParam.nMode )
            {
                case 0:     // COL
                    aTabOpParam.aRefFormulaCell.Set(
                            static_cast<SCCOL>(nFirstCol),
                            static_cast<SCROW>(nFirstRow - 1), nTab, FALSE,
                            FALSE, FALSE );
                    aTabOpParam.aRefFormulaEnd.Set(
                            static_cast<SCCOL>(nLastCol),
                            static_cast<SCROW>(nFirstRow - 1), nTab, FALSE,
                            FALSE, FALSE );
                    aTabOpParam.aRefColCell.Set( static_cast<SCCOL>(nInpCol),
                            static_cast<SCROW>(nInpRow), nTab, FALSE, FALSE,
                            FALSE );
                    nRow++;
                break;
                case 1:     // ROW
                    aTabOpParam.aRefFormulaCell.Set(
                            static_cast<SCCOL>(nFirstCol - 1),
                            static_cast<SCROW>(nFirstRow), nTab, FALSE, FALSE,
                            FALSE );
                    aTabOpParam.aRefFormulaEnd.Set(
                            static_cast<SCCOL>(nFirstCol - 1),
                            static_cast<SCROW>(nLastRow), nTab, FALSE, FALSE,
                            FALSE );
                    aTabOpParam.aRefRowCell.Set( static_cast<SCCOL>(nInpCol),
                            static_cast<SCROW>(nInpRow), nTab, FALSE, FALSE,
                            FALSE );
                    nCol++;
                break;
                case 2:     // TWO-INPUT
                    aTabOpParam.aRefFormulaCell.Set(
                            static_cast<SCCOL>(nFirstCol - 1),
                            static_cast<SCROW>(nFirstRow - 1), nTab, FALSE,
                            FALSE, FALSE );
                    aTabOpParam.aRefRowCell.Set( static_cast<SCCOL>(nInpCol),
                            static_cast<SCROW>(nInpRow), nTab, FALSE, FALSE,
                            FALSE );
                    aTabOpParam.aRefColCell.Set( static_cast<SCCOL>(nInpCol2),
                            static_cast<SCROW>(nInpRow2), nTab, FALSE, FALSE,
                            FALSE );
                break;
            }

            ScMarkData aMarkData;
            aMarkData.SelectOneTable( nTab );
            pD->InsertTableOp( aTabOpParam, static_cast<SCCOL>(nCol),
                    static_cast<SCROW>(nRow), static_cast<SCCOL>(nLastCol),
                    static_cast<SCROW>(nLastRow), aMarkData );
        }
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nLastRow, MAXROW);
        }

    pLastFormCell = NULL;
}


void ImportExcel::Window2_5( void )
{
    ScExtDocOptions&    rExtOpt = GetExtDocOptions();
    UINT16              nOpt, nRow, nCol;
    UINT32              nColorIndex;

    aIn >> nOpt >> nRow >> nCol >> nColorIndex;

    nRow = Min( nRow, (UINT16)MAXROW );
    nCol = Min( nCol, (UINT16)MAXCOL );
    pColRowBuff->SetVisCorner( nCol, nRow );

    if( nOpt & EXC_WIN2_DISPLAYED )
        rExtOpt.SetActTab( static_cast<sal_uInt16>(GetCurrScTab()) );
    pColRowBuff->SetTabSelected( TRUEBOOL( nOpt & EXC_WIN2_SELECTED ) );
    pColRowBuff->SetFrozen( TRUEBOOL( nOpt & EXC_WIN2_FROZEN ) );

    if( static_cast<sal_uInt16>(GetCurrScTab()) == nFirstVisTab )     // import from first visible sheet
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
        GetTracer().TraceInvisibleGrid(TRUEBOOL( nOpt & EXC_WIN2_SHOWGRID ));
    }

    // #106948# RTL layout
    GetDoc().SetLayoutRTL( GetCurrScTab(), ::get_flag( nOpt, EXC_WIN2_MIRRORED ) );
}


void ImportExcel::Bof4( void )
{
    sal_uInt16 nSubType;
    maStrm.DisableDecryption();
    maStrm.Ignore( 2 );
    maStrm >> nSubType;

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

    maStrm.DisableDecryption();
    maStrm >> nVers >> nSubType;

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
    pColRowBuff->Apply( GetCurrScTab() );
    GetXFIndexBuffer().Apply();

    pExcRoot->pExtSheetBuff->Reset();

    GetPageSettings().CreatePageStyle();

    if( pExcRoot->eHauptDateiTyp < Biff8 )
        pExcRoot->pExtNameBuff->Reset();
}


void ImportExcel::NeueTabelle( void )
{
    SCTAB nTab = GetCurrScTab();
    if( nTab > 0 && !pD->HasTable( nTab ) )
        pD->MakeTable( nTab );

    pOutlineListBuffer->Append(new OutlineDataBuffer(*pExcRoot, nTab ));          //#94039# prevent empty rootdata

    pColRowBuff = pOutlineListBuffer->Last()->GetColRowBuff();
    pColOutlineBuff = pOutlineListBuffer->Last()->GetColOutline();
    pRowOutlineBuff = pOutlineListBuffer->Last()->GetRowOutline();
}


const ScTokenArray* ImportExcel::ErrorToFormula( BYTE bErrOrVal, BYTE nError, double& rVal )
{
    return pFormConv->GetBoolErr( XclTools::ErrorToEnum( rVal, bErrOrVal, nError ) );
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
        for( SCTAB nTab = 0; nTab < GetDoc().GetTableCount(); ++nTab )
            pDocObj->AdjustRowHeight( 0, MAXROW, nTab );
    }
}


void ImportExcel::PostDocLoad( void )
{
    /*  Set automatic page numbering in Default page style (default is "page number = 1").
        Otherwise hidden tables (i.e. for scenarios) which have Default page style will
        break automatic page numbering. */
    if( SfxStyleSheetBase* pStyleSheet = GetStyleSheetPool().Find( ScGlobal::GetRscString( STR_STYLENAME_STANDARD ), SFX_STYLE_FAMILY_PAGE ) )
        pStyleSheet->GetItemSet().Put( SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 0 ) );

    // Apply any Outlines for each sheet
    for(OutlineDataBuffer* pBuffer = pOutlineListBuffer->First(); pBuffer; pBuffer = pOutlineListBuffer->Next() )
        pBuffer->Apply(pD);


    // visible area if embedded OLE
    ScModelObj* pDocObj = GetDocModelObj();
    if( pDocObj )
    {
        ScExtDocOptions& rExtDocOpt = GetExtDocOptions();
        SvEmbeddedObject* pEmbObj = pDocObj->GetEmbeddedObject();
        const ScRange* pOleSize = rExtDocOpt.GetOleSize();
        if( pEmbObj && pOleSize )
        {
            pEmbObj->SetVisArea( GetDoc().GetMMRect(
                pOleSize->aStart.Col(), pOleSize->aStart.Row(),
                pOleSize->aEnd.Col(), pOleSize->aEnd.Row(),
                static_cast<SCTAB>(rExtDocOpt.nActTab) ) );
            GetDoc().SetVisibleTab( static_cast<SCTAB>(rExtDocOpt.nActTab) );
        }

        // #111099# open forms in alive mode (has no effect, if no controls in document)
        pDocObj->setPropertyValue( CREATE_OUSTRING( SC_UNO_APPLYFMDES ), ::comphelper::makeBoolAny( sal_False ) );
    }

    // document owns the passed extended document options -> create a new object
    GetDoc().SetExtDocOptions( new ScExtDocOptions( GetExtDocOptions() ) );

    EndAllChartObjects();

    GetDoc().UpdateAllCharts();

    const SCTAB     nLast = pD->GetTableCount();
    const ScRange*      p;

    if( pExcRoot->pPrintRanges->HasRanges() )
    {
        for( SCTAB n = 0 ; n < nLast ; n++ )
        {
            p = pExcRoot->pPrintRanges->First( static_cast<UINT16>(n) );
            if( p )
            {
                DBG_ASSERT( pExcRoot->pPrintRanges->GetActList(),
                            "-ImportExcel::PostDocLoad(): Imaginaere Tabelle gefunden!" );

                pD->ClearPrintRanges( n );
                while( p )
                {
                    pD->AddPrintRange( n, *p );
                    p = pExcRoot->pPrintRanges->Next();
                }
            }
            else
            {
                // #i4063# no print ranges -> print entire sheet
                pD->SetPrintEntireSheet( n );
            }
        }
        GetTracer().TracePrintRange();
    }

    if( pExcRoot->pPrintTitles->HasRanges() )
    {
        for( SCTAB n = 0 ; n < nLast ; n++ )
        {
            p = pExcRoot->pPrintTitles->First( static_cast<UINT16>(n) );
            if( p )
            {
                DBG_ASSERT( pExcRoot->pPrintTitles->GetActList(),
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

                    p = pExcRoot->pPrintTitles->Next();
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

            GetDoc().PutCell( static_cast<SCCOL>(nC), static_cast<SCROW>(nR),
                    GetCurrScTab(), pZelle );
        }
        pColRowBuff->Used( nC, nR );
        GetXFIndexBuffer().SetXF( nC, nR, nXF );
    }
    else
        {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nR, MAXROW);
        }
}


OutlineDataBuffer::OutlineDataBuffer(RootData& rRootData, SCTAB nTabNo) :
    nTab (nTabNo)
{
    pColOutlineBuff = new OutlineBuffer (MAXCOLCOUNT);
    pRowOutlineBuff = new OutlineBuffer (MAXROWCOUNT);
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
