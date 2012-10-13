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


#include "imp_op.hxx"

#include <filter/msfilter/countryid.hxx>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/flditem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/colritem.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <svl/zforlist.hxx>

#include <sfx2/objsh.hxx>
#include "tools/urlobj.hxx"
#include "docuno.hxx"

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
#include "viewopti.hxx"
#include "docoptio.hxx"
#include "scextopt.hxx"
#include "editutil.hxx"
#include "filtopt.hxx"
#include "scerrors.hxx"
#include "unonames.hxx"
#include "paramisc.hxx"
#include "postit.hxx"

#include "fapihelper.hxx"
#include "xltools.hxx"
#include "xltable.hxx"
#include "xlview.hxx"
#include "xltracer.hxx"
#include "xihelper.hxx"
#include "xipage.hxx"
#include "xiview.hxx"
#include "xilink.hxx"
#include "xiescher.hxx"
#include "xicontent.hxx"

#include "excimp8.hxx"
#include "excform.hxx"

#if defined( WNT )
#include <math.h>
#else
#include <stdlib.h>
#endif

using namespace ::com::sun::star;


const double ImportExcel::fExcToTwips =
    ( double ) TWIPS_PER_CHAR / 256.0;


ImportTyp::ImportTyp( ScDocument* pDoc, CharSet eQ )
{
    eQuellChar = eQ;
    pD = pDoc;
}


ImportTyp::~ImportTyp()
{
}


FltError ImportTyp::Read()
{
    return eERR_INTERN;
}


ImportExcel::ImportExcel( XclImpRootData& rImpData, SvStream& rStrm ):
    ImportTyp( &rImpData.mrDoc, rImpData.meTextEnc ),
    XclImpRoot( rImpData ),
    maStrm( rStrm, GetRoot() ),
    aIn( maStrm ),
    maScOleSize( ScAddress::INITIALIZE_INVALID )
{
    mnLastRefIdx = 0;
    nBdshtTab = 0;
    nIxfeIndex = 0;     // zur Sicherheit auf 0

    // Root-Daten fuellen - nach new's ohne Root als Parameter
    pExcRoot = &GetOldRoot();
    pExcRoot->pIR = this;   // ExcRoot -> XclImpRoot
    pExcRoot->eDateiTyp = BiffX;
    pExcRoot->pExtSheetBuff = new ExtSheetBuffer( pExcRoot );   //&aExtSheetBuff;
    pExcRoot->pShrfmlaBuff = new ShrfmlaBuffer( pExcRoot );     //&aShrfrmlaBuff;
    pExcRoot->pExtNameBuff = new ExtNameBuff ( *this );

    pExtNameBuff = new NameBuffer( pExcRoot );          //prevent empty rootdata
    pExtNameBuff->SetBase( 1 );

    pOutlineListBuffer = new XclImpOutlineListBuffer( );

    // ab Biff8
    pFormConv = pExcRoot->pFmlaConverter = new ExcelToSc( GetRoot() );

    bTabTruncated = false;

    // Excel-Dokument per Default auf 31.12.1899, entspricht Excel-Einstellungen mit 1.1.1900
    ScDocOptions aOpt = pD->GetDocOptions();
    aOpt.SetDate( 30, 12, 1899 );
    pD->SetDocOptions( aOpt );
    pD->GetFormatTable()->ChangeNullDate( 30, 12, 1899 );

    ScDocOptions aDocOpt( pD->GetDocOptions() );
    aDocOpt.SetIgnoreCase( sal_True );              // always in Excel
    aDocOpt.SetFormulaRegexEnabled( false );    // regular expressions? what's that?
    aDocOpt.SetLookUpColRowNames( false );      // default: no natural language refs
    pD->SetDocOptions( aDocOpt );
}


ImportExcel::~ImportExcel( void )
{
    GetDoc().SetSrcCharSet( GetTextEncoding() );

    delete pExtNameBuff;

    delete pOutlineListBuffer;

    delete pFormConv;
}


void ImportExcel::ReadFileSharing()
{
    sal_uInt16 nRecommendReadOnly, nPasswordHash;
    maStrm >> nRecommendReadOnly >> nPasswordHash;

    if( (nRecommendReadOnly != 0) || (nPasswordHash != 0) )
    {
        if( SfxItemSet* pItemSet = GetMedium().GetItemSet() )
            pItemSet->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );

        if( SfxObjectShell* pShell = GetDocShell() )
        {
            if( nRecommendReadOnly != 0 )
                pShell->SetLoadReadonly( sal_True );
            if( nPasswordHash != 0 )
                pShell->SetModifyPasswordHash( nPasswordHash );
        }
    }
}

sal_uInt16 ImportExcel::ReadXFIndex( bool bBiff2 )
{
    sal_uInt16 nXFIdx = 0;
    if( bBiff2 )
    {
        sal_uInt8 nXFIdx2 = 0;
        maStrm >> nXFIdx2;
        maStrm.Ignore( 2 );
        nXFIdx = nXFIdx2 & 0x3F;
        if( nXFIdx == 63 )
            nXFIdx = nIxfeIndex;
    }
    else
        aIn >> nXFIdx;
    return nXFIdx;
}

void ImportExcel::ReadDimensions()
{
    XclRange aXclUsedArea( ScAddress::UNINITIALIZED );
    if( (maStrm.GetRecId() == EXC_ID2_DIMENSIONS) || (GetBiff() <= EXC_BIFF5) )
    {
        maStrm >> aXclUsedArea;
        if( (aXclUsedArea.GetColCount() > 1) && (aXclUsedArea.GetRowCount() > 1) )
        {
            // Excel stores first unused row/column index
            --aXclUsedArea.maLast.mnCol;
            --aXclUsedArea.maLast.mnRow;
            // create the Calc range
            SCTAB nScTab = GetCurrScTab();
            ScRange& rScUsedArea = GetExtDocOptions().GetOrCreateTabSettings( nScTab ).maUsedArea;
            GetAddressConverter().ConvertRange( rScUsedArea, aXclUsedArea, nScTab, nScTab, false );
            // if any error occurs in ConvertRange(), rScUsedArea keeps untouched
        }
    }
    else
    {
        sal_uInt32 nXclRow1, nXclRow2;
        maStrm >> nXclRow1 >> nXclRow2 >> aXclUsedArea.maFirst.mnCol >> aXclUsedArea.maLast.mnCol;
        if( (nXclRow1 < nXclRow2) && (aXclUsedArea.GetColCount() > 1) &&
            (nXclRow1 <= static_cast< sal_uInt32 >( GetScMaxPos().Row() )) )
        {
            // Excel stores first unused row/column index
            --nXclRow2;
            --aXclUsedArea.maLast.mnCol;
            // convert row indexes to 16-bit values
            aXclUsedArea.maFirst.mnRow = static_cast< sal_uInt16 >( nXclRow1 );
            aXclUsedArea.maLast.mnRow = limit_cast< sal_uInt16 >( nXclRow2, aXclUsedArea.maFirst.mnRow, SAL_MAX_UINT16 );
            // create the Calc range
            SCTAB nScTab = GetCurrScTab();
            ScRange& rScUsedArea = GetExtDocOptions().GetOrCreateTabSettings( nScTab ).maUsedArea;
            GetAddressConverter().ConvertRange( rScUsedArea, aXclUsedArea, nScTab, nScTab, false );
            // if any error occurs in ConvertRange(), rScUsedArea keeps untouched
        }
    }
}

void ImportExcel::ReadBlank()
{
    XclAddress aXclPos;
    aIn >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        sal_uInt16 nXFIdx = ReadXFIndex( maStrm.GetRecId() == EXC_ID2_BLANK );

        GetXFRangeBuffer().SetBlankXF( aScPos, nXFIdx );
    }
}

void ImportExcel::ReadInteger()
{
    XclAddress aXclPos;
    maStrm >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        sal_uInt16 nXFIdx = ReadXFIndex( true );
        sal_uInt16 nValue;
        maStrm >> nValue;

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        GetDoc().PutCell( aScPos, new ScValueCell( nValue ) );
    }
}

void ImportExcel::ReadNumber()
{
    XclAddress aXclPos;
    maStrm >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        sal_uInt16 nXFIdx = ReadXFIndex( maStrm.GetRecId() == EXC_ID2_NUMBER );
        double fValue;
        maStrm >> fValue;

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        GetDoc().PutCell( aScPos, new ScValueCell( fValue ) );
    }
}

void ImportExcel::ReadLabel()
{
    XclAddress aXclPos;
    maStrm >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        /*  Record ID   BIFF    XF type     String type
            0x0004      2-7     3 byte      8-bit length, byte string
            0x0004      8       3 byte      16-bit length, unicode string
            0x0204      2-7     2 byte      16-bit length, byte string
            0x0204      8       2 byte      16-bit length, unicode string */
        bool bBiff2 = maStrm.GetRecId() == EXC_ID2_LABEL;
        sal_uInt16 nXFIdx = ReadXFIndex( bBiff2 );
        XclStrFlags nFlags = (bBiff2 && (GetBiff() <= EXC_BIFF5)) ? EXC_STR_8BITLENGTH : EXC_STR_DEFAULT;
        XclImpString aString;

        // #i63105# use text encoding from FONT record
        rtl_TextEncoding eOldTextEnc = GetTextEncoding();
        if( const XclImpFont* pFont = GetXFBuffer().GetFont( nXFIdx ) )
            SetTextEncoding( pFont->GetFontEncoding() );
        aString.Read( maStrm, nFlags );
        SetTextEncoding( eOldTextEnc );

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        if( ScBaseCell* pCell = XclImpStringHelper::CreateCell( GetRoot(), aString, nXFIdx ) )
            GetDoc().PutCell( aScPos, pCell );
    }
}

void ImportExcel::ReadBoolErr()
{
    XclAddress aXclPos;
    maStrm >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        sal_uInt16 nXFIdx = ReadXFIndex( maStrm.GetRecId() == EXC_ID2_BOOLERR );
        sal_uInt8 nValue, nType;
        maStrm >> nValue >> nType;

        if( nType == EXC_BOOLERR_BOOL )
            GetXFRangeBuffer().SetBoolXF( aScPos, nXFIdx );
        else
            GetXFRangeBuffer().SetXF( aScPos, nXFIdx );

        double fValue;
        const ScTokenArray* pScTokArr = ErrorToFormula( nType, nValue, fValue );
        ScFormulaCell* pCell = new ScFormulaCell( pD, aScPos, pScTokArr );
        pCell->SetHybridDouble( fValue );
        GetDoc().PutCell( aScPos, pCell );
    }
}

void ImportExcel::ReadRk()
{
    XclAddress aXclPos;
    maStrm >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        sal_uInt16 nXFIdx = ReadXFIndex( false );
        sal_Int32 nRk;
        maStrm >> nRk;

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        GetDoc().PutCell( aScPos, new ScValueCell( XclTools::GetDoubleFromRK( nRk ) ) );
    }
}


void ImportExcel::Window1()
{
    GetDocViewSettings().ReadWindow1( maStrm );
}




void ImportExcel::Row25( void )
{
    sal_uInt16  nRow, nRowHeight;

    aIn >> nRow;
    aIn.Ignore( 4 );   // Mic und Mac ueberspringen

    if( ValidRow( nRow ) )
    {
        aIn >> nRowHeight;  // direkt in Twips angegeben
        aIn.Ignore( 2 );

        if( GetBiff() == EXC_BIFF2 )
        {// -------------------- BIFF2
            pColRowBuff->SetHeight( nRow, nRowHeight );
        }
        else
        {// -------------------- BIFF5
            sal_uInt16  nGrbit;

            aIn.Ignore( 2 );   // reserved
            aIn >> nGrbit;

            sal_uInt8 nLevel = ::extract_value< sal_uInt8 >( nGrbit, 0, 3 );
            pRowOutlineBuff->SetLevel( nRow, nLevel, ::get_flag( nGrbit, EXC_ROW_COLLAPSED ) );
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

    if( nSubType == 0x0020 )        // Chart
        pExcRoot->eDateiTyp = Biff2C;
    else if( nSubType == 0x0040 )   // Macro
        pExcRoot->eDateiTyp = Biff2M;
    else                            // #i51490# Excel interprets invalid indexes as worksheet
        pExcRoot->eDateiTyp = Biff2;
}


void ImportExcel::Eof( void )
{
    //  POST: darf nur nach einer GUELTIGEN Tabelle gerufen werden!
    EndSheet();
    IncCurrScTab();
}


void ImportExcel::SheetPassword( void )
{
    if (GetRoot().GetBiff() != EXC_BIFF8)
        return;

    GetRoot().GetSheetProtectBuffer().ReadPasswordHash( aIn, GetCurrScTab() );
}


void ImportExcel::Externsheet( void )
{
    String aUrl, aTabName;
    bool bSameWorkBook;
    String aEncodedUrl( aIn.ReadByteString( false ) );
    XclImpUrlHelper::DecodeUrl( aUrl, aTabName, bSameWorkBook, *pExcRoot->pIR, aEncodedUrl );
    mnLastRefIdx = pExcRoot->pExtSheetBuff->Add( aUrl, aTabName, bSameWorkBook );
}


void ImportExcel:: WinProtection( void )
{
    if (GetRoot().GetBiff() != EXC_BIFF8)
        return;

    GetRoot().GetDocProtectBuffer().ReadWinProtect( aIn );
}


void ImportExcel::Columndefault( void )
{// Default Cell Attributes
    sal_uInt16  nColMic, nColMac;
    sal_uInt8   nOpt0;

    aIn >> nColMic >> nColMac;

    OSL_ENSURE( aIn.GetRecLeft() == (sal_Size)(nColMac - nColMic) * 3 + 2,
                "ImportExcel::Columndefault - wrong record size" );

    nColMac--;

    if( nColMac > MAXCOL )
        nColMac = static_cast<sal_uInt16>(MAXCOL);

    for( sal_uInt16 nCol = nColMic ; nCol <= nColMac ; nCol++ )
    {
        aIn >> nOpt0;
        aIn.Ignore( 2 );   // nur 0. Attribut-Byte benutzt

        if( nOpt0 & 0x80 )  // Col hidden?
            pColRowBuff->HideCol( nCol );
    }
}


void ImportExcel::Array25( void )
{
    sal_uInt16      nFirstRow, nLastRow, nFormLen;
    sal_uInt8       nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;

    if( GetBiff() == EXC_BIFF2 )
    {//                     BIFF2
        aIn.Ignore( 1 );
        nFormLen = aIn.ReaduInt8();
    }
    else
    {//                     BIFF5
        aIn.Ignore( 6 );
        aIn >> nFormLen;
    }

    if( ValidColRow( nLastCol, nLastRow ) )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;

        pFormConv->Reset( ScAddress( static_cast<SCCOL>(nFirstCol),
                    static_cast<SCROW>(nFirstRow), GetCurrScTab() ) );
        pFormConv->Convert( pErgebnis, maStrm, nFormLen, true, FT_CellFormula);

        OSL_ENSURE( pErgebnis, "*ImportExcel::Array25(): ScTokenArray ist NULL!" );

        ScMarkData          aMarkData;
        aMarkData.SelectOneTable( GetCurrScTab() );
        pD->InsertMatrixFormula( static_cast<SCCOL>(nFirstCol),
                static_cast<SCROW>(nFirstRow), static_cast<SCCOL>(nLastCol),
                static_cast<SCROW>(nLastRow), aMarkData, EMPTY_STRING,
                pErgebnis );
    }
}


void ImportExcel::Rec1904( void )
{
    sal_uInt16  n1904;

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
    sal_uInt32      nRes;
    sal_uInt16      nOpt;

    aIn >> nOpt >> nRes;

    String aName( aIn.ReadByteString( false ) );

    if( ( nOpt & 0x0001 ) || ( ( nOpt & 0xFFFE ) == 0x0000 ) )
    {// external name
        ScfTools::ConvertToScDefinedName( aName );
        pExcRoot->pExtNameBuff->AddName( aName, mnLastRefIdx );
    }
    else if( nOpt & 0x0010 )
    {// ole link
        pExcRoot->pExtNameBuff->AddOLE( aName, mnLastRefIdx, nRes );        // nRes is storage ID
    }
    else
    {// dde link
        pExcRoot->pExtNameBuff->AddDDE( aName, mnLastRefIdx );
    }
}


void ImportExcel::Colwidth( void )
{// Column Width
    sal_uInt8   nColFirst, nColLast;
    sal_uInt16  nColWidth;

    aIn >> nColFirst >> nColLast >> nColWidth;

//! TODO: add a check for the unlikely case of changed MAXCOL (-> XclImpAddressConverter)
//   if( nColLast > MAXCOL )
//       nColLast = static_cast<sal_uInt16>(MAXCOL);

    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( nColWidth, GetCharWidth() );
    pColRowBuff->SetWidthRange( nColFirst, nColLast, nScWidth );
}


void ImportExcel::Defrowheight2( void )
{
    sal_uInt16 nDefHeight;
    maStrm >> nDefHeight;
    nDefHeight &= 0x7FFF;
    pColRowBuff->SetDefHeight( nDefHeight, EXC_DEFROW_UNSYNCED );
}


void ImportExcel::SheetProtect( void )
{
    if (GetRoot().GetBiff() != EXC_BIFF8)
        return;

    GetRoot().GetSheetProtectBuffer().ReadProtect( aIn, GetCurrScTab() );
}

void ImportExcel::DocProtect( void )
{
    if (GetRoot().GetBiff() != EXC_BIFF8)
        return;

    GetRoot().GetDocProtectBuffer().ReadDocProtect( aIn );
}

void ImportExcel::DocPasssword( void )
{
    if (GetRoot().GetBiff() != EXC_BIFF8)
        return;

    GetRoot().GetDocProtectBuffer().ReadPasswordHash( aIn );
}

void ImportExcel::Codepage( void )
{
    SetCodePage( maStrm.ReaduInt16() );
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
    fDefWidth += XclTools::GetXclDefColWidthCorrection( nFontHt );

    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( limit_cast< sal_uInt16 >( fDefWidth ), GetCharWidth() );
    pColRowBuff->SetDefWidth( nScWidth );
}


void ImportExcel::Builtinfmtcnt( void )
{
}


void ImportExcel::Colinfo( void )
{// Column Formatting Information
    sal_uInt16  nColFirst, nColLast, nColWidth, nXF;
    sal_uInt16  nOpt;

    aIn >> nColFirst >> nColLast >> nColWidth >> nXF >> nOpt;

    if( nColFirst > MAXCOL )
        return;

    if( nColLast > MAXCOL )
        nColLast = static_cast<sal_uInt16>(MAXCOL);

    bool bHidden = ::get_flag( nOpt, EXC_COLINFO_HIDDEN );
    bool bCollapsed = ::get_flag( nOpt, EXC_COLINFO_COLLAPSED );
    sal_uInt8 nLevel = ::extract_value< sal_uInt8 >( nOpt, 8, 3 );
    pColOutlineBuff->SetLevelRange( nColFirst, nColLast, nLevel, bCollapsed );

    if( bHidden )
        pColRowBuff->HideColRange( nColFirst, nColLast );

    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( nColWidth, GetCharWidth() );
    pColRowBuff->SetWidthRange( nColFirst, nColLast, nScWidth );
    pColRowBuff->SetDefaultXF( nColFirst, nColLast, nXF );
}


void ImportExcel::Wsbool( void )
{
    sal_uInt16 nFlags;
    aIn >> nFlags;

    pRowOutlineBuff->SetButtonMode( ::get_flag( nFlags, EXC_WSBOOL_ROWBELOW ) );
    pColOutlineBuff->SetButtonMode( ::get_flag( nFlags, EXC_WSBOOL_COLBELOW ) );

    GetPageSettings().SetFitToPages( ::get_flag( nFlags, EXC_WSBOOL_FITTOPAGE ) );
}


void ImportExcel::Boundsheet( void )
{
    sal_uInt16 nGrbit = 0;

    if( GetBiff() == EXC_BIFF5 )
    {
        aIn.DisableDecryption();
        maSheetOffsets.push_back( aIn.ReaduInt32() );
        aIn.EnableDecryption();
        aIn >> nGrbit;
    }

    rtl::OUString aName( aIn.ReadByteString( false ) );

    SCTAB nScTab = static_cast< SCTAB >( nBdshtTab );
    if( nScTab > 0 )
    {
        OSL_ENSURE( !pD->HasTable( nScTab ), "ImportExcel::Boundsheet - sheet exists already" );
        pD->MakeTable( nScTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nScTab, false );

    if( !pD->RenameTab( nScTab, aName ) )
    {
        pD->CreateValidTabName( aName );
        pD->RenameTab( nScTab, aName );
    }

    nBdshtTab++;
}


void ImportExcel::Country( void )
{
    sal_uInt16 nUICountry, nDocCountry;
    maStrm >> nUICountry >> nDocCountry;

    // Store system language in XclRoot
    LanguageType eLanguage = ::msfilter::ConvertCountryToLanguage( static_cast< ::msfilter::CountryId >( nDocCountry ) );
    if( eLanguage != LANGUAGE_DONTKNOW )
        SetDocLanguage( eLanguage );

    // Set Excel UI language in add-in name translator
    eLanguage = ::msfilter::ConvertCountryToLanguage( static_cast< ::msfilter::CountryId >( nUICountry ) );
    if( eLanguage != LANGUAGE_DONTKNOW )
        SetUILanguage( eLanguage );
}


void ImportExcel::ReadUsesElfs()
{
    if( maStrm.ReaduInt16() != 0 )
    {
        ScDocOptions aDocOpt = GetDoc().GetDocOptions();
        aDocOpt.SetLookUpColRowNames( sal_True );
        GetDoc().SetDocOptions( aDocOpt );
    }
}


void ImportExcel::Hideobj( void )
{
    sal_uInt16      nHide;
    ScVObjMode  eOle, eChart, eDraw;

    aIn >> nHide;

    ScViewOptions aOpts( pD->GetViewOptions() );

    switch( nHide )
    {
        case 1:                         // Placeholders
            eOle   = VOBJ_MODE_SHOW;    // in Excel 97 werden nur Charts als Platzhalter angezeigt
            eChart = VOBJ_MODE_SHOW;    //#i80528# VOBJ_MODE_DUMMY replaced by VOBJ_MODE_SHOW now
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


void ImportExcel::Standardwidth( void )
{
    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( maStrm.ReaduInt16(), GetCharWidth() );
    pColRowBuff->SetDefWidth( nScWidth, sal_True );
}


void ImportExcel::Shrfmla( void )
{
    sal_uInt16              nFirstRow, nLastRow, nLenExpr;
    sal_uInt8               nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
    aIn.Ignore( 2 );
    aIn >> nLenExpr;

    // jetzt steht Lesemarke an der Formel

    const ScTokenArray* pErgebnis;

    pFormConv->Reset();
    pFormConv->Convert( pErgebnis, maStrm, nLenExpr, true, FT_SharedFormula );


    OSL_ENSURE( pErgebnis, "+ImportExcel::Shrfmla(): ScTokenArray ist NULL!" );

    pExcRoot->pShrfmlaBuff->Store( ScRange( static_cast<SCCOL>(nFirstCol),
                static_cast<SCROW>(nFirstRow), GetCurrScTab(),
                static_cast<SCCOL>(nLastCol), static_cast<SCROW>(nLastRow),
                GetCurrScTab()), *pErgebnis );
}


void ImportExcel::Mulrk( void )
{
    XclAddress aXclPos;
    sal_uInt16  nXF;
    sal_Int32   nRkNum;

    aIn >> aXclPos;

    for( XclAddress aCurrXclPos( aXclPos ); (aXclPos.mnCol <= aCurrXclPos.mnCol) && (aIn.GetRecLeft() > 2); ++aCurrXclPos.mnCol )
    {
        aIn >> nXF >> nRkNum;

        ScAddress aScPos( ScAddress::UNINITIALIZED );
        if( GetAddressConverter().ConvertAddress( aScPos, aCurrXclPos, GetCurrScTab(), true ) )
        {
            GetXFRangeBuffer().SetXF( aScPos, nXF );
            GetDoc().PutCell( aScPos, new ScValueCell( XclTools::GetDoubleFromRK( nRkNum ) ) );
        }
    }
}


void ImportExcel::Mulblank( void )
{
    XclAddress aXclPos;
    sal_uInt16  nXF;

    aIn >> aXclPos;

    for( XclAddress aCurrXclPos( aXclPos ); (aXclPos.mnCol <= aCurrXclPos.mnCol) && (aIn.GetRecLeft() > 2); ++aCurrXclPos.mnCol )
    {
        aIn >> nXF;

        ScAddress aScPos( ScAddress::UNINITIALIZED );
        if( GetAddressConverter().ConvertAddress( aScPos, aCurrXclPos, GetCurrScTab(), true ) )
            GetXFRangeBuffer().SetBlankXF( aScPos, nXF );
    }
}


void ImportExcel::Rstring( void )
{
    XclAddress aXclPos;
    sal_uInt16 nXFIdx;
    aIn >> aXclPos >> nXFIdx;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        // unformatted Unicode string with separate formatting information
        XclImpString aString;

        // #i63105# use text encoding from FONT record
        rtl_TextEncoding eOldTextEnc = GetTextEncoding();
        if( const XclImpFont* pFont = GetXFBuffer().GetFont( nXFIdx ) )
            SetTextEncoding( pFont->GetFontEncoding() );
        aString.Read( maStrm );
        SetTextEncoding( eOldTextEnc );

        // character formatting runs
        if( !aString.IsRich() )
            aString.ReadFormats( maStrm );

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        if( ScBaseCell* pCell = XclImpStringHelper::CreateCell( *this, aString, nXFIdx ) )
            GetDoc().PutCell( aScPos, pCell );
    }
}


void ImportExcel::Cellmerging()
{
    XclImpAddressConverter& rAddrConv = GetAddressConverter();
    SCTAB nScTab = GetCurrScTab();

    sal_uInt16 nCount;
    maStrm >> nCount;
    for( sal_uInt16 nIdx = 0; (nIdx < nCount) && (maStrm.GetRecLeft() >= 8); ++nIdx )
    {
        XclRange aXclRange;
        maStrm >> aXclRange;    // 16-bit rows and columns
        ScRange aScRange( ScAddress::UNINITIALIZED );
        if( rAddrConv.ConvertRange( aScRange, aXclRange, nScTab, nScTab, true ) )
            GetXFRangeBuffer().SetMerge( aScRange.aStart.Col(), aScRange.aStart.Row(), aScRange.aEnd.Col(), aScRange.aEnd.Row() );
    }
}


void ImportExcel::Olesize( void )
{
    XclRange aXclOleSize( ScAddress::UNINITIALIZED );
    maStrm.Ignore( 2 );
    aXclOleSize.Read( maStrm, false );

    SCTAB nScTab = GetCurrScTab();
    GetAddressConverter().ConvertRange( maScOleSize, aXclOleSize, nScTab, nScTab, false );
}


void ImportExcel::Row34( void )
{
    sal_uInt16  nRow, nRowHeight, nGrbit, nXF;

    aIn >> nRow;
    aIn.Ignore( 4 );   // Mic und Mac ueberspringen

    SCROW nScRow = static_cast< SCROW >( nRow );

    if( ValidRow( nScRow ) )
    {
        aIn >> nRowHeight;  // direkt in Twips angegeben
        aIn.Ignore( 4 );

        nRowHeight = nRowHeight & 0x7FFF; // Bit 15: Row Height not changed manually
        if( !nRowHeight )
            nRowHeight = (GetBiff() == EXC_BIFF2) ? 0x25 : 0x225;

        aIn >> nGrbit >> nXF;

        sal_uInt8 nLevel = ::extract_value< sal_uInt8 >( nGrbit, 0, 3 );
        pRowOutlineBuff->SetLevel( nScRow, nLevel, ::get_flag( nGrbit, EXC_ROW_COLLAPSED ) );
        pColRowBuff->SetRowSettings( nScRow, nRowHeight, nGrbit );

        if( nGrbit & EXC_ROW_USEDEFXF )
            GetXFRangeBuffer().SetRowDefXF( nScRow, nXF & EXC_ROW_XFMASK );
    }
}


void ImportExcel::Bof3( void )
{
    sal_uInt16 nSubType;
    maStrm.DisableDecryption();
    maStrm.Ignore( 2 );
    maStrm >> nSubType;

    OSL_ENSURE( nSubType != 0x0100, "*ImportExcel::Bof3(): Biff3 als Workbook?!" );
    if( nSubType == 0x0100 )        // Book
        pExcRoot->eDateiTyp = Biff3W;
    else if( nSubType == 0x0020 )   // Chart
        pExcRoot->eDateiTyp = Biff3C;
    else if( nSubType == 0x0040 )   // Macro
        pExcRoot->eDateiTyp = Biff3M;
    else                            // #i51490# Excel interprets invalid indexes as worksheet
        pExcRoot->eDateiTyp = Biff3;
}


void ImportExcel::Array34( void )
{
    sal_uInt16                  nFirstRow, nLastRow, nFormLen;
    sal_uInt8                   nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
    aIn.Ignore( (GetBiff() >= EXC_BIFF5) ? 6 : 2 );
    aIn >> nFormLen;

    if( ValidColRow( nLastCol, nLastRow ) )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;

        pFormConv->Reset( ScAddress( static_cast<SCCOL>(nFirstCol),
                    static_cast<SCROW>(nFirstRow), GetCurrScTab() ) );
        pFormConv->Convert( pErgebnis, maStrm, nFormLen, true, FT_CellFormula);

        OSL_ENSURE( pErgebnis, "+ImportExcel::Array34(): ScTokenArray ist NULL!" );

        ScMarkData          aMarkData;
        aMarkData.SelectOneTable( GetCurrScTab() );
        pD->InsertMatrixFormula( static_cast<SCCOL>(nFirstCol),
                static_cast<SCROW>(nFirstRow), static_cast<SCCOL>(nLastCol),
                static_cast<SCROW>(nLastRow), aMarkData, EMPTY_STRING,
                pErgebnis);
    }
}


void ImportExcel::Externname34( void )
{
}


void ImportExcel::Defrowheight345( void )
{
    sal_uInt16 nFlags, nDefHeight;
    maStrm >> nFlags >> nDefHeight;
    pColRowBuff->SetDefHeight( nDefHeight, nFlags );
}


void ImportExcel::TableOp( void )
{
    sal_uInt16 nFirstRow, nLastRow;
    sal_uInt8 nFirstCol, nLastCol;
    sal_uInt16 nGrbit;
    sal_uInt16 nInpRow, nInpCol, nInpRow2, nInpCol2;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol >> nGrbit
        >> nInpRow >> nInpCol >> nInpRow2 >> nInpCol2;

    if( ValidColRow( nLastCol, nLastRow ) )
    {
        if( nFirstCol && nFirstRow )
        {
            ScTabOpParam aTabOpParam;
            aTabOpParam.nMode = (nGrbit & EXC_TABLEOP_BOTH) ? 2 : ((nGrbit & EXC_TABLEOP_ROW) ? 1 : 0 );
            sal_uInt16 nCol = nFirstCol - 1;
            sal_uInt16 nRow = nFirstRow - 1;
            SCTAB nTab = GetCurrScTab();
            switch( aTabOpParam.nMode )
            {
                case 0:     // COL
                    aTabOpParam.aRefFormulaCell.Set(
                            static_cast<SCCOL>(nFirstCol),
                            static_cast<SCROW>(nFirstRow - 1), nTab, false,
                            false, false );
                    aTabOpParam.aRefFormulaEnd.Set(
                            static_cast<SCCOL>(nLastCol),
                            static_cast<SCROW>(nFirstRow - 1), nTab, false,
                            false, false );
                    aTabOpParam.aRefColCell.Set( static_cast<SCCOL>(nInpCol),
                            static_cast<SCROW>(nInpRow), nTab, false, false,
                            false );
                    nRow++;
                break;
                case 1:     // ROW
                    aTabOpParam.aRefFormulaCell.Set(
                            static_cast<SCCOL>(nFirstCol - 1),
                            static_cast<SCROW>(nFirstRow), nTab, false, false,
                            false );
                    aTabOpParam.aRefFormulaEnd.Set(
                            static_cast<SCCOL>(nFirstCol - 1),
                            static_cast<SCROW>(nLastRow), nTab, false, false,
                            false );
                    aTabOpParam.aRefRowCell.Set( static_cast<SCCOL>(nInpCol),
                            static_cast<SCROW>(nInpRow), nTab, false, false,
                            false );
                    nCol++;
                break;
                case 2:     // TWO-INPUT
                    aTabOpParam.aRefFormulaCell.Set(
                            static_cast<SCCOL>(nFirstCol - 1),
                            static_cast<SCROW>(nFirstRow - 1), nTab, false,
                            false, false );
                    aTabOpParam.aRefRowCell.Set( static_cast<SCCOL>(nInpCol),
                            static_cast<SCROW>(nInpRow), nTab, false, false,
                            false );
                    aTabOpParam.aRefColCell.Set( static_cast<SCCOL>(nInpCol2),
                            static_cast<SCROW>(nInpRow2), nTab, false, false,
                            false );
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
        bTabTruncated = sal_True;
        GetTracer().TraceInvalidRow(nLastRow, MAXROW);
    }
}


void ImportExcel::Bof4( void )
{
    sal_uInt16 nSubType;
    maStrm.DisableDecryption();
    maStrm.Ignore( 2 );
    maStrm >> nSubType;

    if( nSubType == 0x0100 )        // Book
        pExcRoot->eDateiTyp = Biff4W;
    else if( nSubType == 0x0020 )   // Chart
        pExcRoot->eDateiTyp = Biff4C;
    else if( nSubType == 0x0040 )   // Macro
        pExcRoot->eDateiTyp = Biff4M;
    else                            // #i51490# Excel interprets invalid indexes as worksheet
        pExcRoot->eDateiTyp = Biff4;
}


void ImportExcel::Bof5( void )
{
    //POST: eDateiTyp = Typ der zu lesenden Datei
    sal_uInt16      nSubType, nVers;
    BiffTyp     eDatei;

    maStrm.DisableDecryption();
    maStrm >> nVers >> nSubType;

    switch( nSubType )
    {
        case 0x0005:    eDatei = Biff5W;    break;  // workbook globals
        case 0x0006:    eDatei = Biff5V;    break;  // VB module
        case 0x0010:    eDatei = Biff5;     break;  // worksheet
        case 0x0020:    eDatei = Biff5C;    break;  // chart
        case 0x0040:    eDatei = Biff5M4;   break;  // macro sheet
        default:
            pExcRoot->eDateiTyp = BiffX;
            return;
    }

    if( nVers == 0x0600 && (GetBiff() == EXC_BIFF8) )
        eDatei = ( BiffTyp ) ( eDatei - Biff5 + Biff8 );

    pExcRoot->eDateiTyp = eDatei;
}

void ImportExcel::EndSheet( void )
{
    pExcRoot->pExtSheetBuff->Reset();

    if( GetBiff() <= EXC_BIFF5 )
    {
        pExcRoot->pExtNameBuff->Reset();
        mnLastRefIdx = 0;
    }

    FinalizeTable();
}


void ImportExcel::NeueTabelle( void )
{
    SCTAB nTab = GetCurrScTab();
    if( nTab > 0 && !pD->HasTable( nTab ) )
        pD->MakeTable( nTab );

    if (nTab == 0 && GetBiff() == EXC_BIFF2)
    {
        // For Excel 2.1 Worksheet file, we need to set the file name as the
        // sheet name.
        INetURLObject aURL(GetDocUrl());
        pD->RenameTab(0, aURL.getBase(), false);
    }

    pExcRoot->pShrfmlaBuff->Clear();

    InitializeTable( nTab );

    XclImpOutlineDataBuffer* pNewItem = new XclImpOutlineDataBuffer( GetRoot(), nTab );
    pOutlineListBuffer->push_back( pNewItem );
    pExcRoot->pColRowBuff = pColRowBuff = pNewItem->GetColRowBuff();
    pColOutlineBuff = pNewItem->GetColOutline();
    pRowOutlineBuff = pNewItem->GetRowOutline();
}


const ScTokenArray* ImportExcel::ErrorToFormula( sal_uInt8 bErrOrVal, sal_uInt8 nError, double& rVal )
{
    return pFormConv->GetBoolErr( XclTools::ErrorToEnum( rVal, bErrOrVal, nError ) );
}


void ImportExcel::AdjustRowHeight()
{
    /*  Speed up chart import: import all sheets without charts, then
        update row heights (here), last load all charts -> do not any longer
        update inside of ScDocShell::ConvertFrom() (causes update of existing
        charts during each and every change of row height). */
    if( ScModelObj* pDocObj = GetDocModelObj() )
        pDocObj->UpdateAllRowHeights();
}


void ImportExcel::PostDocLoad( void )
{
    /*  Set automatic page numbering in Default page style (default is "page number = 1").
        Otherwise hidden tables (i.e. for scenarios) which have Default page style will
        break automatic page numbering. */
    if( SfxStyleSheetBase* pStyleSheet = GetStyleSheetPool().Find( ScGlobal::GetRscString( STR_STYLENAME_STANDARD ), SFX_STYLE_FAMILY_PAGE ) )
        pStyleSheet->GetItemSet().Put( SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 0 ) );

    // outlines for all sheets, sets hidden rows and columns (#i11776# after filtered ranges)
    for (XclImpOutlineListBuffer::iterator itBuffer = pOutlineListBuffer->begin(); itBuffer != pOutlineListBuffer->end(); ++itBuffer)
        itBuffer->Convert();

    // document view settings (before visible OLE area)
    GetDocViewSettings().Finalize();

    // process all drawing objects (including OLE, charts, controls; after hiding rows/columns; before visible OLE area)
    GetObjectManager().ConvertObjects();

    // visible area (used if this document is an embedded OLE object)
    if( SfxObjectShell* pDocShell = GetDocShell() )
    {
        // visible area if embedded
        const ScExtDocSettings& rDocSett = GetExtDocOptions().GetDocSettings();
        SCTAB nDisplScTab = rDocSett.mnDisplTab;

        /*  #i44077# If a new OLE object is inserted from file, there is no
            OLESIZE record in the Excel file. Calculate used area from file
            contents (used cells and drawing objects). */
        if( !maScOleSize.IsValid() )
        {
            // used area of displayed sheet (cell contents)
            if( const ScExtTabSettings* pTabSett = GetExtDocOptions().GetTabSettings( nDisplScTab ) )
                maScOleSize = pTabSett->maUsedArea;
            // add all valid drawing objects
            ScRange aScObjArea = GetObjectManager().GetUsedArea( nDisplScTab );
            if( aScObjArea.IsValid() )
                maScOleSize.ExtendTo( aScObjArea );
        }

        // valid size found - set it at the document
        if( maScOleSize.IsValid() )
        {
            pDocShell->SetVisArea( GetDoc().GetMMRect(
                maScOleSize.aStart.Col(), maScOleSize.aStart.Row(),
                maScOleSize.aEnd.Col(), maScOleSize.aEnd.Row(), nDisplScTab ) );
            GetDoc().SetVisibleTab( nDisplScTab );
        }
    }

    // open forms in alive mode (has no effect, if no controls in document)
    if( ScModelObj* pDocObj = GetDocModelObj() )
        pDocObj->setPropertyValue( SC_UNO_APPLYFMDES, uno::Any( false ) );

    // enables extended options to be set to the view after import
    GetExtDocOptions().SetChanged( true );

    // root data owns the extended document options -> create a new object
    GetDoc().SetExtDocOptions( new ScExtDocOptions( GetExtDocOptions() ) );

    const SCTAB     nLast = pD->GetTableCount();
    const ScRange*      p;

    if( pExcRoot->pPrintRanges->HasRanges() )
    {
        for( SCTAB n = 0 ; n < nLast ; n++ )
        {
            p = pExcRoot->pPrintRanges->First(n);
            if( p )
            {
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
            p = pExcRoot->pPrintTitles->First(n);
            if( p )
            {
                sal_Bool    bRowVirgin = sal_True;
                sal_Bool    bColVirgin = sal_True;

                while( p )
                {
                    if( p->aStart.Col() == 0 && p->aEnd.Col() == MAXCOL && bRowVirgin )
                    {
                        pD->SetRepeatRowRange( n, p );
                        bRowVirgin = false;
                    }

                    if( p->aStart.Row() == 0 && p->aEnd.Row() == MAXROW && bColVirgin )
                    {
                        pD->SetRepeatColRange( n, p );
                        bColVirgin = false;
                    }

                    p = pExcRoot->pPrintTitles->Next();
                }
            }
        }
    }
}

XclImpOutlineDataBuffer::XclImpOutlineDataBuffer( const XclImpRoot& rRoot, SCTAB nScTab ) :
    XclImpRoot( rRoot ),
    mxColOutlineBuff( new XclImpOutlineBuffer( rRoot.GetXclMaxPos().Col() + 1 ) ),
    mxRowOutlineBuff( new XclImpOutlineBuffer( rRoot.GetXclMaxPos().Row() + 1 ) ),
    mxColRowBuff( new XclImpColRowSettings( rRoot ) ),
    mnScTab( nScTab )
{
}

XclImpOutlineDataBuffer::~XclImpOutlineDataBuffer()
{
}

void XclImpOutlineDataBuffer::Convert()
{
    mxColOutlineBuff->SetOutlineArray( GetDoc().GetOutlineTable( mnScTab, sal_True )->GetColArray() );
    mxColOutlineBuff->MakeScOutline();

    mxRowOutlineBuff->SetOutlineArray( GetDoc().GetOutlineTable( mnScTab, sal_True )->GetRowArray() );
    mxRowOutlineBuff->MakeScOutline();

    mxColRowBuff->ConvertHiddenFlags( mnScTab );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
