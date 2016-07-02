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

#include "xepage.hxx"
#include <svl/itemset.hxx>
#include "scitems.hxx"
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include "document.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "attrib.hxx"
#include "xehelper.hxx"
#include "xeescher.hxx"

#include <set>
#include <limits>

using namespace ::oox;

using ::std::set;
using ::std::numeric_limits;

// Page settings records ======================================================

// Header/footer --------------------------------------------------------------

XclExpHeaderFooter::XclExpHeaderFooter( sal_uInt16 nRecId, const OUString& rHdrString ) :
    XclExpRecord( nRecId ),
    maHdrString( rHdrString )
{
}

void XclExpHeaderFooter::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    sal_Int32 nElement = GetRecId() == EXC_ID_HEADER ?  XML_oddHeader : XML_oddFooter;
    rWorksheet->startElement( nElement, FSEND );
    rWorksheet->writeEscaped( maHdrString );
    rWorksheet->endElement( nElement );
}

void XclExpHeaderFooter::WriteBody( XclExpStream& rStrm )
{
    if( !maHdrString.isEmpty() )
    {
        XclExpString aExString;
        if( rStrm.GetRoot().GetBiff() <= EXC_BIFF5 )
            aExString.AssignByte( maHdrString, rStrm.GetRoot().GetTextEncoding(), EXC_STR_8BITLENGTH );
        else
            aExString.Assign( maHdrString, EXC_STR_DEFAULT, 255 );  // 16-bit length, but max 255 chars
        rStrm << aExString;
    }
}

// General page settings ------------------------------------------------------

XclExpSetup::XclExpSetup( const XclPageData& rPageData ) :
    XclExpRecord( EXC_ID_SETUP, 34 ),
    mrData( rPageData )
{
}

void XclExpSetup::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FastAttributeList* pAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
    if( rStrm.getVersion() != oox::core::ISOIEC_29500_2008 ||
        mrData.mnStrictPaperSize != EXC_PAPERSIZE_USER )
    {
        pAttrList->add( XML_paperSize,           OString::number(  mrData.mnPaperSize ).getStr() );
    }
    else
    {
        pAttrList->add( XML_paperWidth,          OString::number(  mrData.mnPaperWidth ).concat(OString("mm")).getStr() );
        pAttrList->add( XML_paperHeight,         OString::number(  mrData.mnPaperHeight ).concat(OString("mm")).getStr() );
        // pAttrList->add( XML_paperUnits,          "mm" );
    }
    pAttrList->add( XML_scale,              OString::number(  mrData.mnScaling ).getStr() );
    pAttrList->add( XML_firstPageNumber,    OString::number(  mrData.mnStartPage ).getStr() );
    pAttrList->add( XML_fitToWidth,         OString::number(  mrData.mnFitToWidth ).getStr() );
    pAttrList->add( XML_fitToHeight,        OString::number(  mrData.mnFitToHeight ).getStr() );
    pAttrList->add( XML_pageOrder,          mrData.mbPrintInRows ? "overThenDown" : "downThenOver" );
    pAttrList->add( XML_orientation,        mrData.mbPortrait ? "portrait" : "landscape" );   // OOXTODO: "default"?
    // tdf#48767 if XML_usePrinterDefaults field is exist, then XML_orientation is always "portrait" in MS Excel
    // To resolve that import issue, if XML_usePrinterDefaults has default value (false) then XML_usePrinterDefaults is not added.
    if ( !mrData.mbValid )
        pAttrList->add( XML_usePrinterDefaults, XclXmlUtils::ToPsz( !mrData.mbValid ) );
    pAttrList->add( XML_blackAndWhite,      XclXmlUtils::ToPsz( mrData.mbBlackWhite ) );
    pAttrList->add( XML_draft,              XclXmlUtils::ToPsz( mrData.mbDraftQuality ) );
    pAttrList->add( XML_cellComments,       mrData.mbPrintNotes ? "atEnd" : "none" );         // OOXTODO: "asDisplayed"?
    pAttrList->add( XML_useFirstPageNumber, XclXmlUtils::ToPsz( mrData.mbManualStart ) );
    // OOXTODO: XML_errors, // == displayed|blank|dash|NA
    pAttrList->add( XML_horizontalDpi,      OString::number(  mrData.mnHorPrintRes ).getStr() );
    pAttrList->add( XML_verticalDpi,        OString::number(  mrData.mnVerPrintRes ).getStr() );
    pAttrList->add( XML_copies,             OString::number(  mrData.mnCopies ).getStr() );
    // OOXTODO: devMode settings part RelationshipId: FSNS( XML_r, XML_id ),

    css::uno::Reference< css::xml::sax::XFastAttributeList > aAttrs(pAttrList);
    rStrm.GetCurrentStream()->singleElement( XML_pageSetup, aAttrs );
}

void XclExpSetup::WriteBody( XclExpStream& rStrm )
{
    XclBiff eBiff = rStrm.GetRoot().GetBiff();

    sal_uInt16 nFlags = 0;
    ::set_flag( nFlags, EXC_SETUP_INROWS,       mrData.mbPrintInRows );
    ::set_flag( nFlags, EXC_SETUP_PORTRAIT,     mrData.mbPortrait );
    ::set_flag( nFlags, EXC_SETUP_INVALID,      !mrData.mbValid );
    ::set_flag( nFlags, EXC_SETUP_BLACKWHITE,   mrData.mbBlackWhite );
    if( eBiff >= EXC_BIFF5 )
    {
        ::set_flag( nFlags, EXC_SETUP_DRAFT,        mrData.mbDraftQuality );
        /*  Set the Comments/Notes to "At end of sheet" if Print Notes is true.
            We don't currently support "as displayed on sheet". Thus this value
            will be re-interpreted to "At end of sheet". */
        const sal_uInt16 nNotes = EXC_SETUP_PRINTNOTES | EXC_SETUP_NOTES_END;
        ::set_flag( nFlags, nNotes,                 mrData.mbPrintNotes );
        ::set_flag( nFlags, EXC_SETUP_STARTPAGE,    mrData.mbManualStart );
    }

    rStrm   << mrData.mnPaperSize << mrData.mnScaling << mrData.mnStartPage
            << mrData.mnFitToWidth << mrData.mnFitToHeight << nFlags;
    if( eBiff >= EXC_BIFF5 )
    {
        rStrm   << mrData.mnHorPrintRes << mrData.mnVerPrintRes
                << mrData.mfHeaderMargin << mrData.mfFooterMargin << mrData.mnCopies;
    }
}

// Manual page breaks ---------------------------------------------------------

XclExpPageBreaks::XclExpPageBreaks( sal_uInt16 nRecId, const ScfUInt16Vec& rPageBreaks, sal_uInt16 nMaxPos ) :
    XclExpRecord( nRecId ),
    mrPageBreaks( rPageBreaks ),
    mnMaxPos( nMaxPos )
{
}

void XclExpPageBreaks::Save( XclExpStream& rStrm )
{
    if( !mrPageBreaks.empty() )
    {
        SetRecSize( 2 + ((rStrm.GetRoot().GetBiff() <= EXC_BIFF5) ? 2 : 6) * mrPageBreaks.size() );
        XclExpRecord::Save( rStrm );
    }
}

void XclExpPageBreaks::WriteBody( XclExpStream& rStrm )
{
    bool bWriteRange = (rStrm.GetRoot().GetBiff() == EXC_BIFF8);

    rStrm << static_cast< sal_uInt16 >( mrPageBreaks.size() );
    for( ScfUInt16Vec::const_iterator aIt = mrPageBreaks.begin(), aEnd = mrPageBreaks.end(); aIt != aEnd; ++aIt )
    {
        rStrm << *aIt;
        if( bWriteRange )
            rStrm << sal_uInt16( 0 ) << mnMaxPos;
    }
}

void XclExpPageBreaks::SaveXml( XclExpXmlStream& rStrm )
{
    if( mrPageBreaks.empty() )
        return;

    sal_Int32 nElement = GetRecId() == EXC_ID_HORPAGEBREAKS ? XML_rowBreaks : XML_colBreaks;
    sax_fastparser::FSHelperPtr& pWorksheet = rStrm.GetCurrentStream();
    OString sNumPageBreaks = OString::number(  mrPageBreaks.size() );
    pWorksheet->startElement( nElement,
            XML_count,              sNumPageBreaks.getStr(),
            XML_manualBreakCount,   sNumPageBreaks.getStr(),
            FSEND );
    for( ScfUInt16Vec::const_iterator aIt = mrPageBreaks.begin(), aEnd = mrPageBreaks.end(); aIt != aEnd; ++aIt )
    {
        pWorksheet->singleElement( XML_brk,
                XML_id,     OString::number(  *aIt ).getStr(),
                XML_man,    "true",
                XML_max,    OString::number(  mnMaxPos ).getStr(),
                XML_min,    "0",
                // OOXTODO: XML_pt, "",
                FSEND );
    }
    pWorksheet->endElement( nElement );
}

// Page settings ==============================================================

XclExpPageSettings::XclExpPageSettings( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
    ScDocument& rDoc = GetDoc();
    SCTAB nScTab = GetCurrScTab();

    if( SfxStyleSheetBase* pStyleSheet = GetStyleSheetPool().Find( rDoc.GetPageStyle( nScTab ), SFX_STYLE_FAMILY_PAGE ) )
    {
        const SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
        maData.mbValid = true;

        // *** page settings ***

        maData.mbPrintInRows   = !GETITEMBOOL( rItemSet, ATTR_PAGE_TOPDOWN   );
        maData.mbHorCenter     =  GETITEMBOOL( rItemSet, ATTR_PAGE_HORCENTER );
        maData.mbVerCenter     =  GETITEMBOOL( rItemSet, ATTR_PAGE_VERCENTER );
        maData.mbPrintHeadings =  GETITEMBOOL( rItemSet, ATTR_PAGE_HEADERS   );
        maData.mbPrintGrid     =  GETITEMBOOL( rItemSet, ATTR_PAGE_GRID      );
        maData.mbPrintNotes    =  GETITEMBOOL( rItemSet, ATTR_PAGE_NOTES     );

        maData.mnStartPage     = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_PAGE_FIRSTPAGENO, sal_uInt16 );
        maData.mbManualStart   = maData.mnStartPage && (!nScTab || rDoc.NeedPageResetAfterTab( nScTab - 1 ));

        const SvxLRSpaceItem& rLRItem = GETITEM( rItemSet, SvxLRSpaceItem, ATTR_LRSPACE );
        maData.mfLeftMargin    = XclTools::GetInchFromTwips( rLRItem.GetLeft() );
        maData.mfRightMargin   = XclTools::GetInchFromTwips( rLRItem.GetRight() );
        const SvxULSpaceItem& rULItem = GETITEM( rItemSet, SvxULSpaceItem, ATTR_ULSPACE );
        maData.mfTopMargin     = XclTools::GetInchFromTwips( rULItem.GetUpper() );
        maData.mfBottomMargin  = XclTools::GetInchFromTwips( rULItem.GetLower() );

        const SvxPageItem& rPageItem = GETITEM( rItemSet, SvxPageItem, ATTR_PAGE );
        const SvxSizeItem& rSizeItem = GETITEM( rItemSet, SvxSizeItem, ATTR_PAGE_SIZE );
        maData.SetScPaperSize( rSizeItem.GetSize(), !rPageItem.IsLandscape() );

        const ScPageScaleToItem& rScaleToItem = GETITEM( rItemSet, ScPageScaleToItem, ATTR_PAGE_SCALETO );
        sal_uInt16 nPages = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_PAGE_SCALETOPAGES, sal_uInt16 );
        sal_uInt16 nScale = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_PAGE_SCALE, sal_uInt16 );

        if( ScfTools::CheckItem( rItemSet, ATTR_PAGE_SCALETO, false ) && rScaleToItem.IsValid() )
        {
            maData.mnFitToWidth = rScaleToItem.GetWidth();
            maData.mnFitToHeight = rScaleToItem.GetHeight();
            maData.mbFitToPages = true;

        }
        else if( ScfTools::CheckItem( rItemSet, ATTR_PAGE_SCALETOPAGES, false ) && nPages )
        {
            maData.mnFitToWidth = 1;
            maData.mnFitToHeight = nPages;
            maData.mbFitToPages = true;
        }
        else if( nScale )
        {
            maData.mnScaling = nScale;
            maData.mbFitToPages = false;
        }

        maData.mxBrushItem.reset( new SvxBrushItem( GETITEM( rItemSet, SvxBrushItem, ATTR_BACKGROUND ) ) );

        // *** header and footer ***

        XclExpHFConverter aHFConv( GetRoot() );

        // header
        const SfxItemSet& rHdrItemSet = GETITEM( rItemSet, SvxSetItem, ATTR_PAGE_HEADERSET ).GetItemSet();
        if( GETITEMBOOL( rHdrItemSet, ATTR_PAGE_ON ) )
        {
            const ScPageHFItem& rHFItem = GETITEM( rItemSet, ScPageHFItem, ATTR_PAGE_HEADERRIGHT );
            aHFConv.GenerateString( rHFItem.GetLeftArea(), rHFItem.GetCenterArea(), rHFItem.GetRightArea() );
            maData.maHeader = aHFConv.GetHFString();
            // header height (Excel excludes header from top margin)
            sal_Int32 nHdrHeight = GETITEMBOOL( rHdrItemSet, ATTR_PAGE_DYNAMIC ) ?
                // dynamic height: calculate header height, add header <-> sheet area distance
                (aHFConv.GetTotalHeight() + GETITEM( rHdrItemSet, SvxULSpaceItem, ATTR_ULSPACE ).GetLower()) :
                // static height: ATTR_PAGE_SIZE already includes header <-> sheet area distance
                static_cast< sal_Int32 >( GETITEM( rHdrItemSet, SvxSizeItem, ATTR_PAGE_SIZE ).GetSize().Height() );
            maData.mfHeaderMargin = maData.mfTopMargin;
            maData.mfTopMargin += XclTools::GetInchFromTwips( nHdrHeight );
        }

        // footer
        const SfxItemSet& rFtrItemSet = GETITEM( rItemSet, SvxSetItem, ATTR_PAGE_FOOTERSET ).GetItemSet();
        if( GETITEMBOOL( rFtrItemSet, ATTR_PAGE_ON ) )
        {
            const ScPageHFItem& rHFItem = GETITEM( rItemSet, ScPageHFItem, ATTR_PAGE_FOOTERRIGHT );
            aHFConv.GenerateString( rHFItem.GetLeftArea(), rHFItem.GetCenterArea(), rHFItem.GetRightArea() );
            maData.maFooter = aHFConv.GetHFString();
            // footer height (Excel excludes footer from bottom margin)
            sal_Int32 nFtrHeight = GETITEMBOOL( rFtrItemSet, ATTR_PAGE_DYNAMIC ) ?
                // dynamic height: calculate footer height, add sheet area <-> footer distance
                (aHFConv.GetTotalHeight() + GETITEM( rFtrItemSet, SvxULSpaceItem, ATTR_ULSPACE ).GetUpper()) :
                // static height: ATTR_PAGE_SIZE already includes sheet area <-> footer distance
                static_cast< sal_Int32 >( GETITEM( rFtrItemSet, SvxSizeItem, ATTR_PAGE_SIZE ).GetSize().Height() );
            maData.mfFooterMargin = maData.mfBottomMargin;
            maData.mfBottomMargin += XclTools::GetInchFromTwips( nFtrHeight );
        }
    }

    // *** page breaks ***

    set<SCROW> aRowBreaks;
    rDoc.GetAllRowBreaks(aRowBreaks, nScTab, false, true);

    SCROW nMaxRow = numeric_limits<sal_uInt16>::max();
    for (set<SCROW>::const_iterator itr = aRowBreaks.begin(), itrEnd = aRowBreaks.end(); itr != itrEnd; ++itr)
    {
        SCROW nRow = *itr;
        if (nRow > nMaxRow)
            break;

        maData.maHorPageBreaks.push_back(nRow);
    }

    if (maData.maHorPageBreaks.size() > 1026)
    {
        // Excel allows only up to 1026 page breaks.  Trim any excess page breaks.
        ScfUInt16Vec::iterator itr = maData.maHorPageBreaks.begin();
        ::std::advance(itr, 1026);
        maData.maHorPageBreaks.erase(itr, maData.maHorPageBreaks.end());
    }

    set<SCCOL> aColBreaks;
    rDoc.GetAllColBreaks(aColBreaks, nScTab, false, true);
    for (set<SCCOL>::const_iterator itr = aColBreaks.begin(), itrEnd = aColBreaks.end(); itr != itrEnd; ++itr)
        maData.maVerPageBreaks.push_back(*itr);
}

class XclExpXmlStartHeaderFooterElementRecord : public XclExpXmlElementRecord
{
public:
    explicit XclExpXmlStartHeaderFooterElementRecord(sal_Int32 const nElement)
         : XclExpXmlElementRecord(nElement) {}

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
};

void XclExpXmlStartHeaderFooterElementRecord::SaveXml(XclExpXmlStream& rStrm)
{
    // OOXTODO: we currently only emit oddHeader/oddFooter elements, and
    //          do not support the first/even/odd page distinction.
    sax_fastparser::FSHelperPtr& rStream = rStrm.GetCurrentStream();
    rStream->startElement( mnElement,
            // OOXTODO: XML_alignWithMargins,
            XML_differentFirst,     "false",    // OOXTODO
            XML_differentOddEven,   "false",    // OOXTODO
            // OOXTODO: XML_scaleWithDoc
            FSEND );
}

void XclExpPageSettings::Save( XclExpStream& rStrm )
{
    XclExpBoolRecord( EXC_ID_PRINTHEADERS, maData.mbPrintHeadings ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_PRINTGRIDLINES, maData.mbPrintGrid ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_GRIDSET, true ).Save( rStrm );
    XclExpPageBreaks( EXC_ID_HORPAGEBREAKS, maData.maHorPageBreaks, static_cast< sal_uInt16 >( GetXclMaxPos().Col() ) ).Save( rStrm );
    XclExpPageBreaks( EXC_ID_VERPAGEBREAKS, maData.maVerPageBreaks, static_cast< sal_uInt16 >( GetXclMaxPos().Row() ) ).Save( rStrm );
    XclExpHeaderFooter( EXC_ID_HEADER, maData.maHeader ).Save( rStrm );
    XclExpHeaderFooter( EXC_ID_FOOTER, maData.maFooter ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_HCENTER, maData.mbHorCenter ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_VCENTER, maData.mbVerCenter ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_LEFTMARGIN, maData.mfLeftMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_RIGHTMARGIN, maData.mfRightMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_TOPMARGIN, maData.mfTopMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_BOTTOMMARGIN, maData.mfBottomMargin ).Save( rStrm );
    XclExpSetup( maData ).Save( rStrm );

    if( (GetBiff() == EXC_BIFF8) && maData.mxBrushItem.get() )
        if( const Graphic* pGraphic = maData.mxBrushItem->GetGraphic() )
            XclExpImgData( *pGraphic, EXC_ID8_IMGDATA ).Save( rStrm );
}

void XclExpPageSettings::SaveXml( XclExpXmlStream& rStrm )
{
    XclExpXmlStartSingleElementRecord( XML_printOptions ).SaveXml( rStrm );
    XclExpBoolRecord( EXC_ID_PRINTHEADERS, maData.mbPrintHeadings, XML_headings ).SaveXml( rStrm );
    XclExpBoolRecord( EXC_ID_PRINTGRIDLINES, maData.mbPrintGrid, XML_gridLines ).SaveXml( rStrm );
    XclExpBoolRecord( EXC_ID_GRIDSET, true, XML_gridLinesSet ).SaveXml( rStrm );
    XclExpBoolRecord( EXC_ID_HCENTER, maData.mbHorCenter, XML_horizontalCentered ).SaveXml( rStrm );
    XclExpBoolRecord( EXC_ID_VCENTER, maData.mbVerCenter, XML_verticalCentered ).SaveXml( rStrm );
    XclExpXmlEndSingleElementRecord().SaveXml( rStrm );    // XML_printOptions

    XclExpXmlStartSingleElementRecord( XML_pageMargins ).SaveXml( rStrm );
    XclExpDoubleRecord( EXC_ID_LEFTMARGIN, maData.mfLeftMargin ).SetAttribute( XML_left )->SaveXml( rStrm );
    XclExpDoubleRecord( EXC_ID_RIGHTMARGIN, maData.mfRightMargin ).SetAttribute( XML_right )->SaveXml( rStrm );
    XclExpDoubleRecord( EXC_ID_TOPMARGIN, maData.mfTopMargin ).SetAttribute( XML_top )->SaveXml( rStrm );
    XclExpDoubleRecord( EXC_ID_BOTTOMMARGIN, maData.mfBottomMargin ).SetAttribute( XML_bottom )->SaveXml( rStrm );
    XclExpDoubleRecord( 0, maData.mfHeaderMargin).SetAttribute( XML_header )->SaveXml( rStrm );
    XclExpDoubleRecord( 0, maData.mfFooterMargin).SetAttribute( XML_footer )->SaveXml( rStrm );
    XclExpXmlEndSingleElementRecord().SaveXml( rStrm );    // XML_pageMargins

    XclExpSetup( maData ).SaveXml( rStrm );

    XclExpXmlStartHeaderFooterElementRecord(XML_headerFooter).SaveXml(rStrm);
    XclExpHeaderFooter( EXC_ID_HEADER, maData.maHeader ).SaveXml( rStrm );
    XclExpHeaderFooter( EXC_ID_FOOTER, maData.maFooter ).SaveXml( rStrm );
    XclExpXmlEndElementRecord( XML_headerFooter ).SaveXml( rStrm );

    XclExpPageBreaks( EXC_ID_HORPAGEBREAKS, maData.maHorPageBreaks,
                    static_cast< sal_uInt16 >( GetXclMaxPos().Col() ) ).SaveXml( rStrm );
    XclExpPageBreaks( EXC_ID_VERPAGEBREAKS, maData.maVerPageBreaks,
                    static_cast< sal_uInt16 >( GetXclMaxPos().Row() ) ).SaveXml( rStrm );
}

XclExpImgData* XclExpPageSettings::getGraphicExport()
{
    if( const Graphic* pGraphic = maData.mxBrushItem->GetGraphic() )
        return new XclExpImgData( *pGraphic, EXC_ID8_IMGDATA );

    return nullptr;
}

XclExpChartPageSettings::XclExpChartPageSettings( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

void XclExpChartPageSettings::Save( XclExpStream& rStrm )
{
    XclExpHeaderFooter( EXC_ID_HEADER, maData.maHeader ).Save( rStrm );
    XclExpHeaderFooter( EXC_ID_FOOTER, maData.maFooter ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_HCENTER, maData.mbHorCenter ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_VCENTER, maData.mbVerCenter ).Save( rStrm );
    XclExpSetup( maData ).Save( rStrm );
    XclExpUInt16Record( EXC_ID_PRINTSIZE, EXC_PRINTSIZE_FULL ).Save( rStrm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
