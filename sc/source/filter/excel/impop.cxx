/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

#include "formulacell.hxx"
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
#include "documentimport.hxx"

#if defined( WNT )
#include <math.h>
#else
#include <stdlib.h>
#endif

using namespace ::com::sun::star;


const double ImportExcel::fExcToTwips =
    ( double ) TWIPS_PER_CHAR / 256.0;


ImportTyp::ImportTyp( ScDocument* pDoc, rtl_TextEncoding eQ )
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
    maScOleSize( ScAddress::INITIALIZE_INVALID ),
    mpLastFormula(NULL),
    mnLastRefIdx( 0 ),
    mnIxfeIndex( 0 ),
    mnLastRecId(0),
    mbBiff2HasXfs(false),
    mbBiff2HasXfsValid(false)
{
    nBdshtTab = 0;

    
    pExcRoot = &GetOldRoot();
    pExcRoot->pIR = this;   
    pExcRoot->eDateiTyp = BiffX;
    pExcRoot->pExtSheetBuff = new ExtSheetBuffer( pExcRoot );   
    pExcRoot->pShrfmlaBuff = new SharedFormulaBuffer( pExcRoot );     
    pExcRoot->pExtNameBuff = new ExtNameBuff ( *this );

    pExtNameBuff = new NameBuffer( pExcRoot );          
    pExtNameBuff->SetBase( 1 );

    pOutlineListBuffer = new XclImpOutlineListBuffer( );

    
    pFormConv = pExcRoot->pFmlaConverter = new ExcelToSc( GetRoot() );

    bTabTruncated = false;

    
    ScDocOptions aOpt = pD->GetDocOptions();
    aOpt.SetDate( 30, 12, 1899 );
    pD->SetDocOptions( aOpt );
    pD->GetFormatTable()->ChangeNullDate( 30, 12, 1899 );

    ScDocOptions aDocOpt( pD->GetDocOptions() );
    aDocOpt.SetIgnoreCase( true );              
    aDocOpt.SetFormulaRegexEnabled( false );    
    aDocOpt.SetLookUpColRowNames( false );      
    pD->SetDocOptions( aDocOpt );
}


ImportExcel::~ImportExcel( void )
{
    GetDoc().SetSrcCharSet( GetTextEncoding() );

    delete pExtNameBuff;

    delete pOutlineListBuffer;

    delete pFormConv;
}

void ImportExcel::SetLastFormula( SCCOL nCol, SCROW nRow, double fVal, sal_uInt16 nXF, ScFormulaCell* pCell )
{
    LastFormulaMapType::iterator it = maLastFormulaCells.find(nCol);
    if (it == maLastFormulaCells.end())
    {
        std::pair<LastFormulaMapType::iterator, bool> r =
            maLastFormulaCells.insert(
                LastFormulaMapType::value_type(nCol, LastFormula()));
        it = r.first;
    }

    it->second.mnCol = nCol;
    it->second.mnRow = nRow;
    it->second.mpCell = pCell;
    it->second.mfValue = fVal;
    it->second.mnXF = nXF;

    mpLastFormula = &it->second;
}

ImportExcel::LastFormula* ImportExcel::GetLastFormula( SCCOL nCol )
{
    LastFormulaMapType::iterator it = maLastFormulaCells.find(nCol);
    return it == maLastFormulaCells.end() ? NULL : &it->second;
}

void ImportExcel::ReadFileSharing()
{
    sal_uInt16 nRecommendReadOnly, nPasswordHash;
    maStrm >> nRecommendReadOnly >> nPasswordHash;

    if( (nRecommendReadOnly != 0) || (nPasswordHash != 0) )
    {
        if( SfxItemSet* pItemSet = GetMedium().GetItemSet() )
            pItemSet->Put( SfxBoolItem( SID_DOC_READONLY, true ) );

        if( SfxObjectShell* pShell = GetDocShell() )
        {
            if( nRecommendReadOnly != 0 )
                pShell->SetLoadReadonly( sal_True );
            if( nPasswordHash != 0 )
                pShell->SetModifyPasswordHash( nPasswordHash );
        }
    }
}

sal_uInt16 ImportExcel::ReadXFIndex( const ScAddress& rScPos, bool bBiff2 )
{
    sal_uInt16 nXFIdx = 0;
    if( bBiff2 )
    {
        /*  #i71453# On first call, check if the file contains XF records (by
            trying to access the first XF with index 0). If there are no XFs,
            the explicit formatting information contained in each cell record
            will be used instead. */
        if( !mbBiff2HasXfsValid )
        {
            mbBiff2HasXfsValid = true;
            mbBiff2HasXfs = GetXFBuffer().GetXF( 0 ) != 0;
        }
        
        sal_uInt8 nFlags1, nFlags2, nFlags3;
        maStrm >> nFlags1 >> nFlags2 >> nFlags3;
        /*  If the file contains XFs, extract and set the XF identifier,
            otherwise get the explicit formatting. */
        if( mbBiff2HasXfs )
        {
            nXFIdx = ::extract_value< sal_uInt16 >( nFlags1, 0, 6 );
            /*  If the identifier is equal to 63, then the real identifier is
                contained in the preceding IXFE record (stored in mnBiff2XfId). */
            if( nXFIdx == 63 )
                nXFIdx = mnIxfeIndex;
        }
        else
        {
            /*  Let the XclImpXF class do the conversion of the imported
                formatting. The XF buffer is empty, therefore will not do any
                conversion based on the XF index later on. */
            XclImpXF::ApplyPatternForBiff2CellFormat( GetRoot(), rScPos, nFlags1, nFlags2, nFlags3 );
        }
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
            
            --aXclUsedArea.maLast.mnCol;
            --aXclUsedArea.maLast.mnRow;
            
            SCTAB nScTab = GetCurrScTab();
            ScRange& rScUsedArea = GetExtDocOptions().GetOrCreateTabSettings( nScTab ).maUsedArea;
            GetAddressConverter().ConvertRange( rScUsedArea, aXclUsedArea, nScTab, nScTab, false );
            
        }
    }
    else
    {
        sal_uInt32 nXclRow1, nXclRow2;
        maStrm >> nXclRow1 >> nXclRow2 >> aXclUsedArea.maFirst.mnCol >> aXclUsedArea.maLast.mnCol;
        if( (nXclRow1 < nXclRow2) && (aXclUsedArea.GetColCount() > 1) &&
            (nXclRow1 <= static_cast< sal_uInt32 >( GetScMaxPos().Row() )) )
        {
            
            --nXclRow2;
            --aXclUsedArea.maLast.mnCol;
            
            aXclUsedArea.maFirst.mnRow = static_cast< sal_uInt16 >( nXclRow1 );
            aXclUsedArea.maLast.mnRow = limit_cast< sal_uInt16 >( nXclRow2, aXclUsedArea.maFirst.mnRow, SAL_MAX_UINT16 );
            
            SCTAB nScTab = GetCurrScTab();
            ScRange& rScUsedArea = GetExtDocOptions().GetOrCreateTabSettings( nScTab ).maUsedArea;
            GetAddressConverter().ConvertRange( rScUsedArea, aXclUsedArea, nScTab, nScTab, false );
            
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
        sal_uInt16 nXFIdx = ReadXFIndex( aScPos, maStrm.GetRecId() == EXC_ID2_BLANK );

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
        sal_uInt16 nXFIdx = ReadXFIndex( aScPos, true );
        sal_uInt16 nValue;
        maStrm >> nValue;

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        GetDocImport().setNumericCell(aScPos, nValue);
    }
}

void ImportExcel::ReadNumber()
{
    XclAddress aXclPos;
    maStrm >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        sal_uInt16 nXFIdx = ReadXFIndex( aScPos, maStrm.GetRecId() == EXC_ID2_NUMBER );
        double fValue;
        maStrm >> fValue;

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        GetDocImport().setNumericCell(aScPos, fValue);
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
        sal_uInt16 nXFIdx = ReadXFIndex( aScPos, bBiff2 );
        XclStrFlags nFlags = (bBiff2 && (GetBiff() <= EXC_BIFF5)) ? EXC_STR_8BITLENGTH : EXC_STR_DEFAULT;
        XclImpString aString;

        
        rtl_TextEncoding eOldTextEnc = GetTextEncoding();
        if( const XclImpFont* pFont = GetXFBuffer().GetFont( nXFIdx ) )
            SetTextEncoding( pFont->GetFontEncoding() );
        aString.Read( maStrm, nFlags );
        SetTextEncoding( eOldTextEnc );

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        XclImpStringHelper::SetToDocument(GetDocImport(), aScPos, GetRoot(), aString, nXFIdx);
    }
}

void ImportExcel::ReadBoolErr()
{
    XclAddress aXclPos;
    maStrm >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        sal_uInt16 nXFIdx = ReadXFIndex( aScPos, maStrm.GetRecId() == EXC_ID2_BOOLERR );
        sal_uInt8 nValue, nType;
        maStrm >> nValue >> nType;

        if( nType == EXC_BOOLERR_BOOL )
            GetXFRangeBuffer().SetBoolXF( aScPos, nXFIdx );
        else
            GetXFRangeBuffer().SetXF( aScPos, nXFIdx );

        double fValue;
        const ScTokenArray* pScTokArr = ErrorToFormula( nType, nValue, fValue );
        ScFormulaCell* pCell = pScTokArr ? new ScFormulaCell(pD, aScPos, *pScTokArr) : new ScFormulaCell(pD, aScPos);
        pCell->SetHybridDouble( fValue );
        GetDocImport().setFormulaCell(aScPos, pCell);
    }
}

void ImportExcel::ReadRk()
{
    XclAddress aXclPos;
    maStrm >> aXclPos;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        sal_uInt16 nXFIdx = ReadXFIndex( aScPos, false );
        sal_Int32 nRk;
        maStrm >> nRk;

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        GetDocImport().setNumericCell(aScPos, XclTools::GetDoubleFromRK(nRk));
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
    aIn.Ignore( 4 );   

    if( ValidRow( nRow ) )
    {
        aIn >> nRowHeight;  
        aIn.Ignore( 2 );

        if( GetBiff() == EXC_BIFF2 )
        {
            pColRowBuff->SetHeight( nRow, nRowHeight );
        }
        else
        {
            sal_uInt16  nGrbit;

            aIn.Ignore( 2 );   
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

    if( nSubType == 0x0020 )        
        pExcRoot->eDateiTyp = Biff2C;
    else if( nSubType == 0x0040 )   
        pExcRoot->eDateiTyp = Biff2M;
    else                            
        pExcRoot->eDateiTyp = Biff2;
}


void ImportExcel::Eof( void )
{
    
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
    OUString aUrl, aTabName;
    bool bSameWorkBook;
    OUString aEncodedUrl( aIn.ReadByteString( false ) );
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
{
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
        aIn.Ignore( 2 );   

        if( nOpt0 & 0x80 )  
            pColRowBuff->HideCol( nCol );
    }
}


void ImportExcel::Array25( void )
{
    sal_uInt16      nFirstRow, nLastRow, nFormLen;
    sal_uInt8       nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;

    if( GetBiff() == EXC_BIFF2 )
    {
        aIn.Ignore( 1 );
        nFormLen = aIn.ReaduInt8();
    }
    else
    {
        aIn.Ignore( 6 );
        aIn >> nFormLen;
    }

    if( ValidColRow( nLastCol, nLastRow ) )
    {
        
        const ScTokenArray* pErgebnis;

        pFormConv->Reset( ScAddress( static_cast<SCCOL>(nFirstCol),
                    static_cast<SCROW>(nFirstRow), GetCurrScTab() ) );
        pFormConv->Convert( pErgebnis, maStrm, nFormLen, true, FT_CellFormula);

        OSL_ENSURE( pErgebnis, "*ImportExcel::Array25(): ScTokenArray is NULL!" );

        ScDocumentImport& rDoc = GetDocImport();
        ScRange aArrayRange(nFirstCol, nFirstRow, GetCurrScTab(), nLastCol, nLastRow, GetCurrScTab());
        rDoc.setMatrixCells(aArrayRange, *pErgebnis, formula::FormulaGrammar::GRAM_ENGLISH_XL_A1);
    }
}


void ImportExcel::Rec1904( void )
{
    sal_uInt16  n1904;

    aIn >> n1904;

    if( n1904 )
    {
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

    OUString aName( aIn.ReadByteString( false ) );

    if( ( nOpt & 0x0001 ) || ( ( nOpt & 0xFFFE ) == 0x0000 ) )
    {
        aName = ScfTools::ConvertToScDefinedName( aName );
        pExcRoot->pExtNameBuff->AddName( aName, mnLastRefIdx );
    }
    else if( nOpt & 0x0010 )
    {
        pExcRoot->pExtNameBuff->AddOLE( aName, mnLastRefIdx, nRes );        
    }
    else
    {
        pExcRoot->pExtNameBuff->AddDDE( aName, mnLastRefIdx );
    }
}


void ImportExcel::Colwidth( void )
{
    sal_uInt8   nColFirst, nColLast;
    sal_uInt16  nColWidth;

    aIn >> nColFirst >> nColLast >> nColWidth;





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
    maStrm >> mnIxfeIndex;
}


void ImportExcel::DefColWidth( void )
{
    
    double fDefWidth = 256.0 * maStrm.ReaduInt16();

    
    long nFontHt = GetFontBuffer().GetAppFontData().mnHeight;
    fDefWidth += XclTools::GetXclDefColWidthCorrection( nFontHt );

    sal_uInt16 nScWidth = XclTools::GetScColumnWidth( limit_cast< sal_uInt16 >( fDefWidth ), GetCharWidth() );
    pColRowBuff->SetDefWidth( nScWidth );
}


void ImportExcel::Builtinfmtcnt( void )
{
}


void ImportExcel::Colinfo( void )
{
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

    OUString aName( aIn.ReadByteString( false ) );

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

    
    LanguageType eLanguage = ::msfilter::ConvertCountryToLanguage( static_cast< ::msfilter::CountryId >( nDocCountry ) );
    if( eLanguage != LANGUAGE_DONTKNOW )
        SetDocLanguage( eLanguage );

    
    eLanguage = ::msfilter::ConvertCountryToLanguage( static_cast< ::msfilter::CountryId >( nUICountry ) );
    if( eLanguage != LANGUAGE_DONTKNOW )
        SetUILanguage( eLanguage );
}


void ImportExcel::ReadUsesElfs()
{
    if( maStrm.ReaduInt16() != 0 )
    {
        ScDocOptions aDocOpt = GetDoc().GetDocOptions();
        aDocOpt.SetLookUpColRowNames( true );
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
        case 1:                         
            eOle   = VOBJ_MODE_SHOW;    
            eChart = VOBJ_MODE_SHOW;    
            eDraw  = VOBJ_MODE_SHOW;
            break;
        case 2:                         
            eOle   = VOBJ_MODE_HIDE;
            eChart = VOBJ_MODE_HIDE;
            eDraw  = VOBJ_MODE_HIDE;
            break;
        default:                        
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
    pColRowBuff->SetDefWidth( nScWidth, true );
}


void ImportExcel::Shrfmla( void )
{
    switch (mnLastRecId)
    {
        case EXC_ID2_FORMULA:
        case EXC_ID3_FORMULA:
        case EXC_ID4_FORMULA:
            
        break;
        default:
            return;
    }

    if (!mpLastFormula)
        
        return;

    sal_uInt16              nFirstRow, nLastRow, nLenExpr;
    sal_uInt8               nFirstCol, nLastCol;

    aIn >> nFirstRow >> nLastRow >> nFirstCol >> nLastCol;
    aIn.Ignore( 2 );
    aIn >> nLenExpr;

    

    const ScTokenArray* pErgebnis;

    pFormConv->Reset();
    pFormConv->Convert( pErgebnis, maStrm, nLenExpr, true, FT_SharedFormula );

    OSL_ENSURE( pErgebnis, "+ImportExcel::Shrfmla(): ScTokenArray is NULL!" );

    
    
    SCCOL nCol1 = mpLastFormula->mnCol;
    SCROW nRow1 = mpLastFormula->mnRow;

    ScAddress aPos(nCol1, nRow1, GetCurrScTab());
    pExcRoot->pShrfmlaBuff->Store(aPos, *pErgebnis);

    

    ScDocumentImport& rDoc = GetDocImport();

    ScFormulaCell* pCell = new ScFormulaCell(pD, aPos, *pErgebnis);
    rDoc.getDoc().EnsureTable(aPos.Tab());
    rDoc.setFormulaCell(aPos, pCell);
    pCell->SetNeedNumberFormat(false);
    if (!rtl::math::isNan(mpLastFormula->mfValue))
        pCell->SetResultDouble(mpLastFormula->mfValue);

    GetXFRangeBuffer().SetXF(aPos, mpLastFormula->mnXF);
    mpLastFormula->mpCell = pCell;
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
            GetDocImport().setNumericCell(aScPos, XclTools::GetDoubleFromRK(nRkNum));
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
        
        XclImpString aString;

        
        rtl_TextEncoding eOldTextEnc = GetTextEncoding();
        if( const XclImpFont* pFont = GetXFBuffer().GetFont( nXFIdx ) )
            SetTextEncoding( pFont->GetFontEncoding() );
        aString.Read( maStrm );
        SetTextEncoding( eOldTextEnc );

        
        if( !aString.IsRich() )
            aString.ReadFormats( maStrm );

        GetXFRangeBuffer().SetXF( aScPos, nXFIdx );
        XclImpStringHelper::SetToDocument(GetDocImport(), aScPos, *this, aString, nXFIdx);
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
        maStrm >> aXclRange;    
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
    aIn.Ignore( 4 );   

    SCROW nScRow = static_cast< SCROW >( nRow );

    if( ValidRow( nScRow ) )
    {
        aIn >> nRowHeight;  
        aIn.Ignore( 4 );

        nRowHeight = nRowHeight & 0x7FFF; 
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

    OSL_ENSURE( nSubType != 0x0100, "*ImportExcel::Bof3(): Biff3 as Workbook?!" );
    if( nSubType == 0x0100 )        
        pExcRoot->eDateiTyp = Biff3W;
    else if( nSubType == 0x0020 )   
        pExcRoot->eDateiTyp = Biff3C;
    else if( nSubType == 0x0040 )   
        pExcRoot->eDateiTyp = Biff3M;
    else                            
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
        
        const ScTokenArray* pErgebnis;

        pFormConv->Reset( ScAddress( static_cast<SCCOL>(nFirstCol),
                    static_cast<SCROW>(nFirstRow), GetCurrScTab() ) );
        pFormConv->Convert( pErgebnis, maStrm, nFormLen, true, FT_CellFormula);

        OSL_ENSURE( pErgebnis, "+ImportExcel::Array34(): ScTokenArray is NULL!" );

        ScDocumentImport& rDoc = GetDocImport();
        ScRange aArrayRange(nFirstCol, nFirstRow, GetCurrScTab(), nLastCol, nLastRow, GetCurrScTab());
        rDoc.setMatrixCells(aArrayRange, *pErgebnis, formula::FormulaGrammar::GRAM_ENGLISH_XL_A1);
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
            aTabOpParam.meMode = (nGrbit & EXC_TABLEOP_BOTH) ? ScTabOpParam::Both : ((nGrbit & EXC_TABLEOP_ROW) ? ScTabOpParam::Row : ScTabOpParam::Column);
            sal_uInt16 nCol = nFirstCol - 1;
            sal_uInt16 nRow = nFirstRow - 1;
            SCTAB nTab = GetCurrScTab();
            switch (aTabOpParam.meMode)
            {
                case ScTabOpParam::Column:
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
                case ScTabOpParam::Row:
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
                case ScTabOpParam::Both:     
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

            ScDocumentImport& rDoc = GetDocImport();
            ScRange aTabOpRange(nCol, nRow, nTab, nLastCol, nLastRow, nTab);
            rDoc.setTableOpCells(aTabOpRange, aTabOpParam);
        }
    }
    else
    {
        bTabTruncated = true;
        GetTracer().TraceInvalidRow(nLastRow, MAXROW);
    }
}


void ImportExcel::Bof4( void )
{
    sal_uInt16 nSubType;
    maStrm.DisableDecryption();
    maStrm.Ignore( 2 );
    maStrm >> nSubType;

    if( nSubType == 0x0100 )        
        pExcRoot->eDateiTyp = Biff4W;
    else if( nSubType == 0x0020 )   
        pExcRoot->eDateiTyp = Biff4C;
    else if( nSubType == 0x0040 )   
        pExcRoot->eDateiTyp = Biff4M;
    else                            
        pExcRoot->eDateiTyp = Biff4;
}


void ImportExcel::Bof5( void )
{
    
    sal_uInt16      nSubType, nVers;
    BiffTyp     eDatei;

    maStrm.DisableDecryption();
    maStrm >> nVers >> nSubType;

    switch( nSubType )
    {
        case 0x0005:    eDatei = Biff5W;    break;  
        case 0x0006:    eDatei = Biff5V;    break;  
        case 0x0010:    eDatei = Biff5;     break;  
        case 0x0020:    eDatei = Biff5C;    break;  
        case 0x0040:    eDatei = Biff5M4;   break;  
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


void ImportExcel::NewTable( void )
{
    SCTAB nTab = GetCurrScTab();
    if( nTab > 0 && !pD->HasTable( nTab ) )
        pD->MakeTable( nTab );

    if (nTab == 0 && GetBiff() == EXC_BIFF2)
    {
        
        
        INetURLObject aURL(GetDocUrl());
        pD->RenameTab(0, aURL.getBase(), false);
    }

    pExcRoot->pShrfmlaBuff->Clear();
    maLastFormulaCells.clear();
    mpLastFormula = NULL;

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

    
    for (XclImpOutlineListBuffer::iterator itBuffer = pOutlineListBuffer->begin(); itBuffer != pOutlineListBuffer->end(); ++itBuffer)
        itBuffer->Convert();

    
    GetDocViewSettings().Finalize();

    
    GetObjectManager().ConvertObjects();

    
    if( SfxObjectShell* pDocShell = GetDocShell() )
    {
        
        const ScExtDocSettings& rDocSett = GetExtDocOptions().GetDocSettings();
        SCTAB nDisplScTab = rDocSett.mnDisplTab;

        /*  #i44077# If a new OLE object is inserted from file, there is no
            OLESIZE record in the Excel file. Calculate used area from file
            contents (used cells and drawing objects). */
        if( !maScOleSize.IsValid() )
        {
            
            if( const ScExtTabSettings* pTabSett = GetExtDocOptions().GetTabSettings( nDisplScTab ) )
                maScOleSize = pTabSett->maUsedArea;
            
            ScRange aScObjArea = GetObjectManager().GetUsedArea( nDisplScTab );
            if( aScObjArea.IsValid() )
                maScOleSize.ExtendTo( aScObjArea );
        }

        
        if( maScOleSize.IsValid() )
        {
            pDocShell->SetVisArea( GetDoc().GetMMRect(
                maScOleSize.aStart.Col(), maScOleSize.aStart.Row(),
                maScOleSize.aEnd.Col(), maScOleSize.aEnd.Row(), nDisplScTab ) );
            GetDoc().SetVisibleTab( nDisplScTab );
        }
    }

    
    if( ScModelObj* pDocObj = GetDocModelObj() )
        pDocObj->setPropertyValue( SC_UNO_APPLYFMDES, uno::Any( false ) );

    
    GetExtDocOptions().SetChanged( true );

    
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
    mxColOutlineBuff->SetOutlineArray( GetDoc().GetOutlineTable( mnScTab, true )->GetColArray() );
    mxColOutlineBuff->MakeScOutline();

    mxRowOutlineBuff->SetOutlineArray( GetDoc().GetOutlineTable( mnScTab, true )->GetRowArray() );
    mxRowOutlineBuff->MakeScOutline();

    mxColRowBuff->ConvertHiddenFlags( mnScTab );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
