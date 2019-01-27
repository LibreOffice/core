/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <xipage.hxx>
#include <svl/itemset.hxx>
#include <vcl/graph.hxx>
#include <scitems.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <document.hxx>
#include <stlsheet.hxx>
#include <attrib.hxx>
#include <xistream.hxx>
#include <xihelper.hxx>
#include <xiescher.hxx>

// Page settings ==============================================================

XclImpPageSettings::XclImpPageSettings( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
    Initialize();
}

void XclImpPageSettings::Initialize()
{
    maData.SetDefaults();
    mbValidPaper = false;
}

void XclImpPageSettings::ReadSetup( XclImpStream& rStrm )
{
    OSL_ENSURE_BIFF( GetBiff() >= EXC_BIFF4 );
    if( GetBiff() < EXC_BIFF4 )
        return;

    // BIFF4 - BIFF8
    sal_uInt16 nFlags;
    maData.mnPaperSize = rStrm.ReaduInt16();
    maData.mnScaling = rStrm.ReaduInt16();
    maData.mnStartPage = rStrm.ReaduInt16();
    maData.mnFitToWidth = rStrm.ReaduInt16();
    maData.mnFitToHeight = rStrm.ReaduInt16();
    nFlags = rStrm.ReaduInt16();

    mbValidPaper = maData.mbValid = !::get_flag( nFlags, EXC_SETUP_INVALID );
    maData.mbPrintInRows = ::get_flag( nFlags, EXC_SETUP_INROWS );
    maData.mbPortrait = ::get_flag( nFlags, EXC_SETUP_PORTRAIT );
    maData.mbBlackWhite = ::get_flag( nFlags, EXC_SETUP_BLACKWHITE );
    maData.mbManualStart = true;

    // new in BIFF5 - BIFF8
    if( GetBiff() >= EXC_BIFF5 )
    {
        maData.mnHorPrintRes = rStrm.ReaduInt16();
        maData.mnVerPrintRes = rStrm.ReaduInt16();
        maData.mfHeaderMargin = rStrm.ReadDouble();
        maData.mfFooterMargin = rStrm.ReadDouble();
        maData.mnCopies = rStrm.ReaduInt16();

        maData.mbDraftQuality = ::get_flag( nFlags, EXC_SETUP_DRAFT );
        maData.mbPrintNotes = ::get_flag( nFlags, EXC_SETUP_PRINTNOTES );
        maData.mbManualStart = ::get_flag( nFlags, EXC_SETUP_STARTPAGE );
    }
}

void XclImpPageSettings::ReadMargin( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_LEFTMARGIN:     maData.mfLeftMargin = rStrm.ReadDouble();   break;
        case EXC_ID_RIGHTMARGIN:    maData.mfRightMargin = rStrm.ReadDouble();  break;
        case EXC_ID_TOPMARGIN:      maData.mfTopMargin = rStrm.ReadDouble();    break;
        case EXC_ID_BOTTOMMARGIN:   maData.mfBottomMargin = rStrm.ReadDouble(); break;
        default:    OSL_FAIL( "XclImpPageSettings::ReadMargin - unknown record" );
    }
}

void XclImpPageSettings::ReadCenter( XclImpStream& rStrm )
{
    OSL_ENSURE_BIFF( GetBiff() >= EXC_BIFF3 );  // read it anyway
    bool bCenter = (rStrm.ReaduInt16() != 0);
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_HCENTER:    maData.mbHorCenter = bCenter;   break;
        case EXC_ID_VCENTER:    maData.mbVerCenter = bCenter;   break;
        default:    OSL_FAIL( "XclImpPageSettings::ReadCenter - unknown record" );
    }
}

void XclImpPageSettings::ReadHeaderFooter( XclImpStream& rStrm )
{
    OUString aString;
    if( rStrm.GetRecLeft() )
        aString = (GetBiff() <= EXC_BIFF5) ? rStrm.ReadByteString( false ) : rStrm.ReadUniString();

    switch( rStrm.GetRecId() )
    {
        case EXC_ID_HEADER:     maData.maHeader = aString;  break;
        case EXC_ID_FOOTER:     maData.maFooter = aString;  break;
        default:    OSL_FAIL( "XclImpPageSettings::ReadHeaderFooter - unknown record" );
    }
}

void XclImpPageSettings::ReadPageBreaks( XclImpStream& rStrm )
{
    ScfUInt16Vec* pVec = nullptr;
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_HORPAGEBREAKS:  pVec = &maData.maHorPageBreaks;     break;
        case EXC_ID_VERPAGEBREAKS:  pVec = &maData.maVerPageBreaks;     break;
        default:    OSL_FAIL( "XclImpPageSettings::ReadPageBreaks - unknown record" );
    }

    if( pVec )
    {
        bool bIgnore = GetBiff() == EXC_BIFF8;  // ignore start/end columns or rows in BIFF8

        sal_uInt16 nCount, nBreak;
        nCount = rStrm.ReaduInt16();
        pVec->clear();
        pVec->reserve( nCount );

        while( nCount-- )
        {
            nBreak = rStrm.ReaduInt16();
            if( nBreak )
                pVec->push_back( nBreak );
            if( bIgnore )
                rStrm.Ignore( 4 );
        }
    }
}

void XclImpPageSettings::ReadPrintHeaders( XclImpStream& rStrm )
{
    maData.mbPrintHeadings = (rStrm.ReaduInt16() != 0);
}

void XclImpPageSettings::ReadPrintGridLines( XclImpStream& rStrm )
{
    maData.mbPrintGrid = (rStrm.ReaduInt16() != 0);
}

void XclImpPageSettings::ReadImgData( XclImpStream& rStrm )
{
    Graphic aGraphic = XclImpDrawing::ReadImgData( GetRoot(), rStrm );
    if( aGraphic.GetType() != GraphicType::NONE )
        maData.mxBrushItem.reset( new SvxBrushItem( aGraphic, GPOS_TILED, ATTR_BACKGROUND ) );
}

void XclImpPageSettings::SetPaperSize( sal_uInt16 nXclPaperSize, bool bPortrait )
{
    maData.mnPaperSize = nXclPaperSize;
    maData.mbPortrait = bPortrait;
    mbValidPaper = true;
}

namespace {

void lclPutMarginItem( SfxItemSet& rItemSet, sal_uInt16 nRecId, double fMarginInch )
{
    sal_uInt16 nMarginTwips = XclTools::GetTwipsFromInch( fMarginInch );
    switch( nRecId )
    {
        case EXC_ID_TOPMARGIN:
        case EXC_ID_BOTTOMMARGIN:
        {
            SvxULSpaceItem aItem( rItemSet.Get( ATTR_ULSPACE ) );
            if( nRecId == EXC_ID_TOPMARGIN )
                aItem.SetUpperValue( nMarginTwips );
            else
                aItem.SetLowerValue( nMarginTwips );
            rItemSet.Put( aItem );
        }
        break;
        case EXC_ID_LEFTMARGIN:
        case EXC_ID_RIGHTMARGIN:
        {
            SvxLRSpaceItem aItem( rItemSet.Get( ATTR_LRSPACE ) );
            if( nRecId == EXC_ID_LEFTMARGIN )
                aItem.SetLeftValue( nMarginTwips );
            else
                aItem.SetRightValue( nMarginTwips );
            rItemSet.Put( aItem );
        }
        break;
        default:
            OSL_FAIL( "XclImpPageSettings::SetMarginItem - unknown record id" );
    }
}

} // namespace

void XclImpPageSettings::Finalize()
{
    ScDocument& rDoc = GetDoc();
    SCTAB nScTab = GetCurrScTab();

    // *** create page style sheet ***

    OUStringBuffer aStyleName;
    aStyleName.append("PageStyle_");

    OUString aTableName;
    if( GetDoc().GetName( nScTab, aTableName ) )
        aStyleName.append(aTableName);
    else
        aStyleName.append(static_cast<sal_Int32>(nScTab+1));

    ScStyleSheet& rStyleSheet = ScfTools::MakePageStyleSheet(
        GetStyleSheetPool(), aStyleName.makeStringAndClear(), false);

    SfxItemSet& rItemSet = rStyleSheet.GetItemSet();

    // *** page settings ***

    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_TOPDOWN,    !maData.mbPrintInRows ),    true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_HORCENTER,  maData.mbHorCenter ),       true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_VERCENTER,  maData.mbVerCenter ),       true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_HEADERS,    maData.mbPrintHeadings ),   true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_GRID,       maData.mbPrintGrid ),       true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_NOTES,      maData.mbPrintNotes ),      true );

    sal_uInt16 nStartPage = maData.mbManualStart ? maData.mnStartPage : 0;
    ScfTools::PutItem( rItemSet, SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, nStartPage ), true );

    if( maData.mxBrushItem.get() )
        rItemSet.Put( *maData.mxBrushItem );

    if( mbValidPaper )
    {
        SvxPageItem aPageItem( rItemSet.Get( ATTR_PAGE ) );
        aPageItem.SetLandscape( !maData.mbPortrait );
        rItemSet.Put( aPageItem );
        ScfTools::PutItem( rItemSet, SvxSizeItem( ATTR_PAGE_SIZE, maData.GetScPaperSize() ), true );
    }

    if( maData.mbFitToPages )
        rItemSet.Put( ScPageScaleToItem( maData.mnFitToWidth, maData.mnFitToHeight ) );
    else if( maData.mbValid )
        rItemSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, maData.mnScaling ) );

    // *** margin preparations ***

    double fLeftMargin   = maData.mfLeftMargin;
    double fRightMargin  = maData.mfRightMargin;
    double fTopMargin    = maData.mfTopMargin;
    double fBottomMargin = maData.mfBottomMargin;
    // distances between header/footer and page area
    double fHeaderHeight = 0.0;
    double fHeaderDist = 0.0;
    double fFooterHeight = 0.0;
    double fFooterDist = 0.0;
    // in Calc, "header/footer left/right margin" is X distance between header/footer and page margin
    double fHdrLeftMargin  = maData.mfHdrLeftMargin  - maData.mfLeftMargin;
    double fHdrRightMargin = maData.mfHdrRightMargin - maData.mfRightMargin;
    double fFtrLeftMargin  = maData.mfFtrLeftMargin  - maData.mfLeftMargin;
    double fFtrRightMargin = maData.mfFtrRightMargin - maData.mfRightMargin;

    // *** header and footer ***

    XclImpHFConverter aHFConv( GetRoot() );

    // header
    bool bHasHeader = !maData.maHeader.isEmpty();
    SvxSetItem aHdrSetItem( rItemSet.Get( ATTR_PAGE_HEADERSET ) );
    SfxItemSet& rHdrItemSet = aHdrSetItem.GetItemSet();
    rHdrItemSet.Put( SfxBoolItem( ATTR_PAGE_ON, bHasHeader ) );
    if( bHasHeader )
    {
        aHFConv.ParseString( maData.maHeader );
        aHFConv.FillToItemSet( rItemSet, ATTR_PAGE_HEADERLEFT );
        aHFConv.FillToItemSet( rItemSet, ATTR_PAGE_HEADERRIGHT );
        // #i23296# In Calc, "top margin" is distance to header
        fTopMargin = maData.mfHeaderMargin;
        // Calc uses distance between header and sheet data area
        fHeaderHeight = XclTools::GetInchFromTwips( aHFConv.GetTotalHeight() );
        fHeaderDist = maData.mfTopMargin - maData.mfHeaderMargin - fHeaderHeight;
    }
    if( fHeaderDist < 0.0 )
    {
        /*  #i23296# Header overlays sheet data:
            -> set fixed header height to get correct sheet data position. */
        ScfTools::PutItem( rHdrItemSet, SfxBoolItem( ATTR_PAGE_DYNAMIC, false ), true );
        // shrink header height
        long nHdrHeight = XclTools::GetTwipsFromInch( fHeaderHeight + fHeaderDist );
        ScfTools::PutItem( rHdrItemSet, SvxSizeItem( ATTR_PAGE_SIZE, Size( 0, nHdrHeight ) ), true );
        lclPutMarginItem( rHdrItemSet, EXC_ID_BOTTOMMARGIN, 0.0 );
    }
    else
    {
        // use dynamic header height
        ScfTools::PutItem( rHdrItemSet, SfxBoolItem( ATTR_PAGE_DYNAMIC, true ), true );
        lclPutMarginItem( rHdrItemSet, EXC_ID_BOTTOMMARGIN, fHeaderDist );
    }
    lclPutMarginItem( rHdrItemSet, EXC_ID_LEFTMARGIN,   fHdrLeftMargin );
    lclPutMarginItem( rHdrItemSet, EXC_ID_RIGHTMARGIN,  fHdrRightMargin );
    rItemSet.Put( aHdrSetItem );

    // footer
    bool bHasFooter = !maData.maFooter.isEmpty();
    SvxSetItem aFtrSetItem( rItemSet.Get( ATTR_PAGE_FOOTERSET ) );
    SfxItemSet& rFtrItemSet = aFtrSetItem.GetItemSet();
    rFtrItemSet.Put( SfxBoolItem( ATTR_PAGE_ON, bHasFooter ) );
    if( bHasFooter )
    {
        aHFConv.ParseString( maData.maFooter );
        aHFConv.FillToItemSet( rItemSet, ATTR_PAGE_FOOTERLEFT );
        aHFConv.FillToItemSet( rItemSet, ATTR_PAGE_FOOTERRIGHT );
        // #i23296# In Calc, "bottom margin" is distance to footer
        fBottomMargin = maData.mfFooterMargin;
        // Calc uses distance between footer and sheet data area
        fFooterHeight = XclTools::GetInchFromTwips( aHFConv.GetTotalHeight() );
        fFooterDist = maData.mfBottomMargin - maData.mfFooterMargin - fFooterHeight;
    }
    if( fFooterDist < 0.0 )
    {
        /*  #i23296# Footer overlays sheet data:
            -> set fixed footer height to get correct sheet data end position. */
        ScfTools::PutItem( rFtrItemSet, SfxBoolItem( ATTR_PAGE_DYNAMIC, false ), true );
        // shrink footer height
        long nFtrHeight = XclTools::GetTwipsFromInch( fFooterHeight + fFooterDist );
        ScfTools::PutItem( rFtrItemSet, SvxSizeItem( ATTR_PAGE_SIZE, Size( 0, nFtrHeight ) ), true );
        lclPutMarginItem( rFtrItemSet, EXC_ID_TOPMARGIN, 0.0 );
    }
    else
    {
        // use dynamic footer height
        ScfTools::PutItem( rFtrItemSet, SfxBoolItem( ATTR_PAGE_DYNAMIC, true ), true );
        lclPutMarginItem( rFtrItemSet, EXC_ID_TOPMARGIN, fFooterDist );
    }
    lclPutMarginItem( rFtrItemSet, EXC_ID_LEFTMARGIN,   fFtrLeftMargin );
    lclPutMarginItem( rFtrItemSet, EXC_ID_RIGHTMARGIN,  fFtrRightMargin );
    rItemSet.Put( aFtrSetItem );

    // *** set final margins ***

    lclPutMarginItem( rItemSet, EXC_ID_LEFTMARGIN,   fLeftMargin );
    lclPutMarginItem( rItemSet, EXC_ID_RIGHTMARGIN,  fRightMargin );
    lclPutMarginItem( rItemSet, EXC_ID_TOPMARGIN,    fTopMargin );
    lclPutMarginItem( rItemSet, EXC_ID_BOTTOMMARGIN, fBottomMargin );

    // *** put style sheet into document ***

    rDoc.SetPageStyle( nScTab, rStyleSheet.GetName() );

    // *** page breaks ***

    for( const auto& rHorPageBreak : maData.maHorPageBreaks )
    {
        SCROW nScRow = static_cast< SCROW >( rHorPageBreak );
        if( nScRow <= MAXROW )
            rDoc.SetRowBreak(nScRow, nScTab, false, true);
    }

    for( const auto& rVerPageBreak : maData.maVerPageBreaks )
    {
        SCCOL nScCol = static_cast< SCCOL >( rVerPageBreak );
        if( nScCol <= MAXCOL )
            rDoc.SetColBreak(nScCol, nScTab, false, true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
