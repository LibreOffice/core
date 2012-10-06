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

#include "pagesettings.hxx"

#include <algorithm>
#include <set>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "biffinputstream.hxx"
#include "excelhandlers.hxx"
#include "stylesbuffer.hxx"
#include "unitconverter.hxx"
#include <sax/tools/converter.hxx>

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;

using ::oox::core::Relations;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

const double OOX_MARGIN_DEFAULT_LR                  = 0.748;    /// Left/right default margin in inches.
const double OOX_MARGIN_DEFAULT_TB                  = 0.984;    /// Top/bottom default margin in inches.
const double OOX_MARGIN_DEFAULT_HF                  = 0.512;    /// Header/footer default margin in inches.

const sal_uInt16 BIFF12_PRINTOPT_HORCENTER          = 0x0001;
const sal_uInt16 BIFF12_PRINTOPT_VERCENTER          = 0x0002;
const sal_uInt16 BIFF12_PRINTOPT_PRINTHEADING       = 0x0004;
const sal_uInt16 BIFF12_PRINTOPT_PRINTGRID          = 0x0008;

const sal_uInt16 BIFF12_HEADERFOOTER_DIFFEVEN       = 0x0001;
const sal_uInt16 BIFF12_HEADERFOOTER_DIFFFIRST      = 0x0002;
const sal_uInt16 BIFF12_HEADERFOOTER_SCALEDOC       = 0x0004;
const sal_uInt16 BIFF12_HEADERFOOTER_ALIGNMARGIN    = 0x0008;

const sal_uInt16 BIFF12_PAGESETUP_INROWS            = 0x0001;
const sal_uInt16 BIFF12_PAGESETUP_LANDSCAPE         = 0x0002;
const sal_uInt16 BIFF12_PAGESETUP_INVALID           = 0x0004;
const sal_uInt16 BIFF12_PAGESETUP_BLACKWHITE        = 0x0008;
const sal_uInt16 BIFF12_PAGESETUP_DRAFTQUALITY      = 0x0010;
const sal_uInt16 BIFF12_PAGESETUP_PRINTNOTES        = 0x0020;
const sal_uInt16 BIFF12_PAGESETUP_DEFAULTORIENT     = 0x0040;
const sal_uInt16 BIFF12_PAGESETUP_USEFIRSTPAGE      = 0x0080;
const sal_uInt16 BIFF12_PAGESETUP_NOTES_END         = 0x0100;   // different to BIFF flag

const sal_uInt16 BIFF12_CHARTPAGESETUP_LANDSCAPE    = 0x0001;
const sal_uInt16 BIFF12_CHARTPAGESETUP_INVALID      = 0x0002;
const sal_uInt16 BIFF12_CHARTPAGESETUP_BLACKWHITE   = 0x0004;
const sal_uInt16 BIFF12_CHARTPAGESETUP_DEFAULTORIENT= 0x0008;
const sal_uInt16 BIFF12_CHARTPAGESETUP_USEFIRSTPAGE = 0x0010;
const sal_uInt16 BIFF12_CHARTPAGESETUP_DRAFTQUALITY = 0x0020;

const sal_uInt16 BIFF_PAGESETUP_INROWS              = 0x0001;
const sal_uInt16 BIFF_PAGESETUP_PORTRAIT            = 0x0002;
const sal_uInt16 BIFF_PAGESETUP_INVALID             = 0x0004;
const sal_uInt16 BIFF_PAGESETUP_BLACKWHITE          = 0x0008;
const sal_uInt16 BIFF_PAGESETUP_DRAFTQUALITY        = 0x0010;
const sal_uInt16 BIFF_PAGESETUP_PRINTNOTES          = 0x0020;
const sal_uInt16 BIFF_PAGESETUP_DEFAULTORIENT       = 0x0040;
const sal_uInt16 BIFF_PAGESETUP_USEFIRSTPAGE        = 0x0080;
const sal_uInt16 BIFF_PAGESETUP_NOTES_END           = 0x0200;

} // namespace

// ============================================================================

PageSettingsModel::PageSettingsModel() :
    mfLeftMargin( OOX_MARGIN_DEFAULT_LR ),
    mfRightMargin( OOX_MARGIN_DEFAULT_LR ),
    mfTopMargin( OOX_MARGIN_DEFAULT_TB ),
    mfBottomMargin( OOX_MARGIN_DEFAULT_TB ),
    mfHeaderMargin( OOX_MARGIN_DEFAULT_HF ),
    mfFooterMargin( OOX_MARGIN_DEFAULT_HF ),
    mnPaperSize( 1 ),
    mnPaperWidth( 0 ),
    mnPaperHeight( 0 ),
    mnCopies( 1 ),
    mnScale( 100 ),
    mnFirstPage( 1 ),
    mnFitToWidth( 1 ),
    mnFitToHeight( 1 ),
    mnHorPrintRes( 600 ),
    mnVerPrintRes( 600 ),
    mnOrientation( XML_default ),
    mnPageOrder( XML_downThenOver ),
    mnCellComments( XML_none ),
    mnPrintErrors( XML_displayed ),
    mbUseEvenHF( false ),
    mbUseFirstHF( false ),
    mbValidSettings( true ),
    mbUseFirstPage( false ),
    mbBlackWhite( false ),
    mbDraftQuality( false ),
    mbFitToPages( false ),
    mbHorCenter( false ),
    mbVerCenter( false ),
    mbPrintGrid( false ),
    mbPrintHeadings( false )
{
}

void PageSettingsModel::setBiffPrintErrors( sal_uInt8 nPrintErrors )
{
    static const sal_Int32 spnErrorIds[] = { XML_displayed, XML_none, XML_dash, XML_NA };
    mnPrintErrors = STATIC_ARRAY_SELECT( spnErrorIds, nPrintErrors, XML_none );
}

// ============================================================================

PageSettings::PageSettings( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void PageSettings::importPrintOptions( const AttributeList& rAttribs )
{
    maModel.mbHorCenter     = rAttribs.getBool( XML_horizontalCentered, false );
    maModel.mbVerCenter     = rAttribs.getBool( XML_verticalCentered, false );
    maModel.mbPrintGrid     = rAttribs.getBool( XML_gridLines, false );
    maModel.mbPrintHeadings = rAttribs.getBool( XML_headings, false );
}

void PageSettings::importPageMargins( const AttributeList& rAttribs )
{
    maModel.mfLeftMargin   = rAttribs.getDouble( XML_left,   OOX_MARGIN_DEFAULT_LR );
    maModel.mfRightMargin  = rAttribs.getDouble( XML_right,  OOX_MARGIN_DEFAULT_LR );
    maModel.mfTopMargin    = rAttribs.getDouble( XML_top,    OOX_MARGIN_DEFAULT_TB );
    maModel.mfBottomMargin = rAttribs.getDouble( XML_bottom, OOX_MARGIN_DEFAULT_TB );
    maModel.mfHeaderMargin = rAttribs.getDouble( XML_header, OOX_MARGIN_DEFAULT_HF );
    maModel.mfFooterMargin = rAttribs.getDouble( XML_footer, OOX_MARGIN_DEFAULT_HF );
}

void PageSettings::importPageSetup( const Relations& rRelations, const AttributeList& rAttribs )
{
    OUString aStr;
    maModel.maBinSettPath   = rRelations.getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
    maModel.mnPaperSize     = rAttribs.getInteger( XML_paperSize, 1 );
    aStr                    = rAttribs.getString ( XML_paperWidth, OUString() );
    ::sax::Converter::convertMeasure(
            maModel.mnPaperWidth, aStr, util::MeasureUnit::MM_100TH);
    aStr                    = rAttribs.getString ( XML_paperHeight, OUString() );
    ::sax::Converter::convertMeasure(
            maModel.mnPaperHeight, aStr, util::MeasureUnit::MM_100TH );
    maModel.mnCopies        = rAttribs.getInteger( XML_copies, 1 );
    maModel.mnScale         = rAttribs.getInteger( XML_scale, 100 );
    maModel.mnFirstPage     = rAttribs.getInteger( XML_firstPageNumber, 1 );
    maModel.mnFitToWidth    = rAttribs.getInteger( XML_fitToWidth, 1 );
    maModel.mnFitToHeight   = rAttribs.getInteger( XML_fitToHeight, 1 );
    maModel.mnHorPrintRes   = rAttribs.getInteger( XML_horizontalDpi, 600 );
    maModel.mnVerPrintRes   = rAttribs.getInteger( XML_verticalDpi, 600 );
    maModel.mnOrientation   = rAttribs.getToken( XML_orientation, XML_default );
    maModel.mnPageOrder     = rAttribs.getToken( XML_pageOrder, XML_downThenOver );
    maModel.mnCellComments  = rAttribs.getToken( XML_cellComments, XML_none );
    maModel.mnPrintErrors   = rAttribs.getToken( XML_errors, XML_displayed );
    maModel.mbValidSettings = rAttribs.getBool( XML_usePrinterDefaults, false );
    maModel.mbUseFirstPage  = rAttribs.getBool( XML_useFirstPageNumber, false );
    maModel.mbBlackWhite    = rAttribs.getBool( XML_blackAndWhite, false );
    maModel.mbDraftQuality  = rAttribs.getBool( XML_draft, false );
}

void PageSettings::importChartPageSetup( const Relations& rRelations, const AttributeList& rAttribs )
{
    OUString aStr;
    maModel.maBinSettPath   = rRelations.getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
    maModel.mnPaperSize     = rAttribs.getInteger( XML_paperSize, 1 );
    aStr                    = rAttribs.getString ( XML_paperWidth, OUString() );
    ::sax::Converter::convertMeasure(
            maModel.mnPaperWidth, aStr, util::MeasureUnit::MM_100TH );
    aStr                    = rAttribs.getString ( XML_paperHeight, OUString() );
    ::sax::Converter::convertMeasure(
            maModel.mnPaperHeight, aStr, util::MeasureUnit::MM_100TH );
    maModel.mnCopies        = rAttribs.getInteger( XML_copies, 1 );
    maModel.mnFirstPage     = rAttribs.getInteger( XML_firstPageNumber, 1 );
    maModel.mnHorPrintRes   = rAttribs.getInteger( XML_horizontalDpi, 600 );
    maModel.mnVerPrintRes   = rAttribs.getInteger( XML_verticalDpi, 600 );
    maModel.mnOrientation   = rAttribs.getToken( XML_orientation, XML_default );
    maModel.mbValidSettings = rAttribs.getBool( XML_usePrinterDefaults, false );
    maModel.mbUseFirstPage  = rAttribs.getBool( XML_useFirstPageNumber, false );
    maModel.mbBlackWhite    = rAttribs.getBool( XML_blackAndWhite, false );
    maModel.mbDraftQuality  = rAttribs.getBool( XML_draft, false );
}

void PageSettings::importHeaderFooter( const AttributeList& rAttribs )
{
    maModel.mbUseEvenHF  = rAttribs.getBool( XML_differentOddEven, false );
    maModel.mbUseFirstHF = rAttribs.getBool( XML_differentFirst, false );
}

void PageSettings::importHeaderFooterCharacters( const OUString& rChars, sal_Int32 nElement )
{
    switch( nElement )
    {
        case XLS_TOKEN( oddHeader ):    maModel.maOddHeader += rChars;      break;
        case XLS_TOKEN( oddFooter ):    maModel.maOddFooter += rChars;      break;
        case XLS_TOKEN( evenHeader ):   maModel.maEvenHeader += rChars;     break;
        case XLS_TOKEN( evenFooter ):   maModel.maEvenFooter += rChars;     break;
        case XLS_TOKEN( firstHeader ):  maModel.maFirstHeader += rChars;    break;
        case XLS_TOKEN( firstFooter ):  maModel.maFirstFooter += rChars;    break;
    }
}

void PageSettings::importPicture( const Relations& rRelations, const AttributeList& rAttribs )
{
    importPictureData( rRelations, rAttribs.getString( R_TOKEN( id ), OUString() ) );
}

void PageSettings::importPageMargins( SequenceInputStream& rStrm )
{
    rStrm   >> maModel.mfLeftMargin   >> maModel.mfRightMargin
            >> maModel.mfTopMargin    >> maModel.mfBottomMargin
            >> maModel.mfHeaderMargin >> maModel.mfFooterMargin;
}

void PageSettings::importPrintOptions( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> nFlags;
    maModel.mbHorCenter     = getFlag( nFlags, BIFF12_PRINTOPT_HORCENTER );
    maModel.mbVerCenter     = getFlag( nFlags, BIFF12_PRINTOPT_VERCENTER );
    maModel.mbPrintGrid     = getFlag( nFlags, BIFF12_PRINTOPT_PRINTGRID );
    maModel.mbPrintHeadings = getFlag( nFlags, BIFF12_PRINTOPT_PRINTHEADING );
}

void PageSettings::importPageSetup( const Relations& rRelations, SequenceInputStream& rStrm )
{
    OUString aRelId;
    sal_uInt16 nFlags;
    rStrm   >> maModel.mnPaperSize >> maModel.mnScale
            >> maModel.mnHorPrintRes >> maModel.mnVerPrintRes
            >> maModel.mnCopies >> maModel.mnFirstPage
            >> maModel.mnFitToWidth >> maModel.mnFitToHeight
            >> nFlags >> aRelId;
    maModel.setBiffPrintErrors( extractValue< sal_uInt8 >( nFlags, 9, 2 ) );
    maModel.maBinSettPath   = rRelations.getFragmentPathFromRelId( aRelId );
    maModel.mnOrientation   = getFlagValue( nFlags, BIFF12_PAGESETUP_DEFAULTORIENT, XML_default, getFlagValue( nFlags, BIFF12_PAGESETUP_LANDSCAPE, XML_landscape, XML_portrait ) );
    maModel.mnPageOrder     = getFlagValue( nFlags, BIFF12_PAGESETUP_INROWS, XML_overThenDown, XML_downThenOver );
    maModel.mnCellComments  = getFlagValue( nFlags, BIFF12_PAGESETUP_PRINTNOTES, getFlagValue( nFlags, BIFF12_PAGESETUP_NOTES_END, XML_atEnd, XML_asDisplayed ), XML_none );
    maModel.mbValidSettings = !getFlag( nFlags, BIFF12_PAGESETUP_INVALID );
    maModel.mbUseFirstPage  = getFlag( nFlags, BIFF12_PAGESETUP_USEFIRSTPAGE );
    maModel.mbBlackWhite    = getFlag( nFlags, BIFF12_PAGESETUP_BLACKWHITE );
    maModel.mbDraftQuality  = getFlag( nFlags, BIFF12_PAGESETUP_DRAFTQUALITY );
}

void PageSettings::importChartPageSetup( const Relations& rRelations, SequenceInputStream& rStrm )
{
    OUString aRelId;
    sal_uInt16 nFirstPage, nFlags;
    rStrm   >> maModel.mnPaperSize >> maModel.mnHorPrintRes >> maModel.mnVerPrintRes
            >> maModel.mnCopies >> nFirstPage >> nFlags >> aRelId;
    maModel.maBinSettPath   = rRelations.getFragmentPathFromRelId( aRelId );
    maModel.mnFirstPage     = nFirstPage; // 16-bit in CHARTPAGESETUP
    maModel.mnOrientation   = getFlagValue( nFlags, BIFF12_CHARTPAGESETUP_DEFAULTORIENT, XML_default, getFlagValue( nFlags, BIFF12_CHARTPAGESETUP_LANDSCAPE, XML_landscape, XML_portrait ) );
    maModel.mbValidSettings = !getFlag( nFlags, BIFF12_CHARTPAGESETUP_INVALID );
    maModel.mbUseFirstPage  = getFlag( nFlags, BIFF12_CHARTPAGESETUP_USEFIRSTPAGE );
    maModel.mbBlackWhite    = getFlag( nFlags, BIFF12_CHARTPAGESETUP_BLACKWHITE );
    maModel.mbDraftQuality  = getFlag( nFlags, BIFF12_CHARTPAGESETUP_DRAFTQUALITY );
}

void PageSettings::importHeaderFooter( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm   >> nFlags
            >> maModel.maOddHeader   >> maModel.maOddFooter
            >> maModel.maEvenHeader  >> maModel.maEvenFooter
            >> maModel.maFirstHeader >> maModel.maFirstFooter;
    maModel.mbUseEvenHF  = getFlag( nFlags, BIFF12_HEADERFOOTER_DIFFEVEN );
    maModel.mbUseFirstHF = getFlag( nFlags, BIFF12_HEADERFOOTER_DIFFFIRST );
}

void PageSettings::importPicture( const Relations& rRelations, SequenceInputStream& rStrm )
{
    importPictureData( rRelations, BiffHelper::readString( rStrm ) );
}

void PageSettings::setFitToPagesMode( bool bFitToPages )
{
    maModel.mbFitToPages = bFitToPages;
}

void PageSettings::finalizeImport()
{
    OUStringBuffer aStyleNameBuffer( "PageStyle_" );
    Reference< XNamed > xSheetName( getSheet(), UNO_QUERY );
    if( xSheetName.is() )
        aStyleNameBuffer.append( xSheetName->getName() );
    else
        aStyleNameBuffer.append( static_cast< sal_Int32 >( getSheetIndex() + 1 ) );
    OUString aStyleName = aStyleNameBuffer.makeStringAndClear();

    Reference< XStyle > xStyle = createStyleObject( aStyleName, true );
    PropertySet aStyleProps( xStyle );
    getPageSettingsConverter().writePageSettingsProperties( aStyleProps, maModel, getSheetType() );

    PropertySet aSheetProps( getSheet() );
    aSheetProps.setProperty( PROP_PageStyle, aStyleName );
}

void PageSettings::importPictureData( const Relations& rRelations, const OUString& rRelId )
{
    OUString aPicturePath = rRelations.getFragmentPathFromRelId( rRelId );
    if( !aPicturePath.isEmpty() )
        maModel.maGraphicUrl = getBaseFilter().getGraphicHelper().importEmbeddedGraphicObject( aPicturePath );
}

// ============================================================================
// ============================================================================

enum HFPortionId
{
    HF_LEFT,
    HF_CENTER,
    HF_RIGHT,
    HF_COUNT
};

// ----------------------------------------------------------------------------

struct HFPortionInfo
{
    Reference< XText >  mxText;                 /// XText interface of this portion.
    Reference< XTextCursor > mxStart;           /// Start position of current text range for formatting.
    Reference< XTextCursor > mxEnd;             /// End position of current text range for formatting.
    double              mfTotalHeight;          /// Sum of heights of previous lines in points.
    double              mfCurrHeight;           /// Height of the current text line in points.

    bool                initialize( const Reference< XText >& rxText );
};

bool HFPortionInfo::initialize( const Reference< XText >& rxText )
{
    mfTotalHeight = mfCurrHeight = 0.0;
    mxText = rxText;
    if( mxText.is() )
    {
        mxStart = mxText->createTextCursor();
        mxEnd = mxText->createTextCursor();
    }
    bool bRet = mxText.is() && mxStart.is() && mxEnd.is();
    OSL_ENSURE( bRet, "HFPortionInfo::initialize - missing interfaces" );
    return bRet;
}

// ============================================================================

class HeaderFooterParser : public WorkbookHelper
{
public:
    explicit            HeaderFooterParser( const WorkbookHelper& rHelper );

    /** Parses the passed string and creates the header/footer contents.
        @returns  The total height of the converted header or footer in points. */
    double              parse(
                            const Reference< XHeaderFooterContent >& rxContext,
                            const OUString& rData );

private:
    /** Returns the current edit engine text object. */
    inline HFPortionInfo& getPortion() { return maPortions[ meCurrPortion ]; }
    /** Returns the start cursor of the current text range. */
    inline const Reference< XTextCursor >& getStartPos() { return getPortion().mxStart; }
    /** Returns the end cursor of the current text range. */
    inline const Reference< XTextCursor >& getEndPos() { return getPortion().mxEnd; }

    /** Returns the current line height of the specified portion. */
    double              getCurrHeight( HFPortionId ePortion ) const;
    /** Returns the current line height. */
    double              getCurrHeight() const;

    /** Updates the current line height of the specified portion, using the current font size. */
    void                updateCurrHeight( HFPortionId ePortion );
    /** Updates the current line height, using the current font size. */
    void                updateCurrHeight();

    /** Sets the font attributes at the current selection. */
    void                setAttributes();
    /** Appends and clears internal string buffer. */
    void                appendText();
    /** Appends a line break and adjusts internal text height data. */
    void                appendLineBreak();

    /** Creates a text field from the passed service name. */
    Reference< XTextContent > createField( const OUString& rServiceName ) const;
    /** Appends the passed text field. */
    void                appendField( const Reference< XTextContent >& rxContent );

    /** Sets the passed font name if it is valid. */
    void                convertFontName( const OUString& rStyle );
    /** Converts a font style given as string. */
    void                convertFontStyle( const OUString& rStyle );
    /** Converts a font color given as string. */
    void                convertFontColor( const OUString& rColor );

    /** Finalizes current portion: sets font attributes and updates text height data. */
    void                finalizePortion();
    /** Changes current header/footer portion. */
    void                setNewPortion( HFPortionId ePortion );

private:
    typedef ::std::vector< HFPortionInfo >  HFPortionInfoVec;
    typedef ::std::set< OString >           OStringSet;

    const OUString      maPageNumberService;
    const OUString      maPageCountService;
    const OUString      maSheetNameService;
    const OUString      maFileNameService;
    const OUString      maDateTimeService;
    const OStringSet    maBoldNames;            /// All names for bold font style in lowercase UTF-8.
    const OStringSet    maItalicNames;          /// All names for italic font style in lowercase UTF-8.
    HFPortionInfoVec    maPortions;
    HFPortionId         meCurrPortion;          /// Identifier of current H/F portion.
    OUStringBuffer      maBuffer;               /// Text data to append to current text range.
    FontModel           maFontModel;            /// Font attributes of current text range.
};

// ----------------------------------------------------------------------------

namespace {

// different names for bold font style (lowercase)
static const sal_Char* const sppcBoldNames[] =
{
    "bold",
    "fett",             // German 'bold'
    "demibold",
    "halbfett",         // German 'demibold'
    "black",
    "heavy"
};

// different names for italic font style (lowercase)
static const sal_Char* const sppcItalicNames[] =
{
    "italic",
    "kursiv",           // German 'italic'
    "oblique",
    "schr\303\204g",    // German 'oblique' with uppercase A umlaut
    "schr\303\244g"     // German 'oblique' with lowercase A umlaut
};

} // namespace

// ----------------------------------------------------------------------------

HeaderFooterParser::HeaderFooterParser( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maPageNumberService( "com.sun.star.text.TextField.PageNumber" ),
    maPageCountService( "com.sun.star.text.TextField.PageCount" ),
    maSheetNameService( "com.sun.star.text.TextField.SheetName" ),
    maFileNameService( "com.sun.star.text.TextField.FileName" ),
    maDateTimeService( "com.sun.star.text.TextField.DateTime" ),
    maBoldNames( sppcBoldNames, STATIC_ARRAY_END( sppcBoldNames ) ),
    maItalicNames( sppcItalicNames, STATIC_ARRAY_END( sppcItalicNames ) ),
    maPortions( static_cast< size_t >( HF_COUNT ) ),
    meCurrPortion( HF_CENTER )
{
}

double HeaderFooterParser::parse( const Reference< XHeaderFooterContent >& rxContext, const OUString& rData )
{
    if( !rxContext.is() || rData.isEmpty() ||
            !maPortions[ HF_LEFT ].initialize( rxContext->getLeftText() ) ||
            !maPortions[ HF_CENTER ].initialize( rxContext->getCenterText() ) ||
            !maPortions[ HF_RIGHT ].initialize( rxContext->getRightText() ) )
        return 0.0;

    meCurrPortion = HF_CENTER;
    maBuffer.setLength( 0 );
    maFontModel = getStyles().getDefaultFontModel();
    OUStringBuffer aFontName;           // current font name
    OUStringBuffer aFontStyle;          // current font style
    sal_Int32 nFontHeight = 0;          // current font height

    /** State of the parser. */
    enum
    {
        STATE_TEXT,         /// Literal text data.
        STATE_TOKEN,        /// Control token following a '&' character.
        STATE_FONTNAME,     /// Font name ('&' is followed by '"', reads until next '"' or ',').
        STATE_FONTSTYLE,    /// Font style name (font part after ',', reads until next '"').
        STATE_FONTHEIGHT    /// Font height ('&' is followed by num. digits, reads until non-digit).
    }
    eState = STATE_TEXT;

    const sal_Unicode* pcChar = rData.getStr();
    const sal_Unicode* pcEnd = pcChar + rData.getLength();
    for( ; (pcChar != pcEnd) && (*pcChar != 0); ++pcChar )
    {
        sal_Unicode cChar = *pcChar;
        switch( eState )
        {
            case STATE_TEXT:
            {
                switch( cChar )
                {
                    case '&':           // new token
                        appendText();
                        eState = STATE_TOKEN;
                    break;
                    case '\n':          // line break
                        appendText();
                        appendLineBreak();
                    break;
                    default:
                        maBuffer.append( cChar );
                }
            }
            break;

            case STATE_TOKEN:
            {
                // default: back to text mode, may be changed in specific cases
                eState = STATE_TEXT;
                // ignore case of token codes
                if( ('a' <= cChar) && (cChar <= 'z') )
                    (cChar -= 'a') += 'A';
                switch( cChar )
                {
                    case '&':   maBuffer.append( cChar );   break;  // the '&' character

                    case 'L':   setNewPortion( HF_LEFT );   break;  // left portion
                    case 'C':   setNewPortion( HF_CENTER ); break;  // center portion
                    case 'R':   setNewPortion( HF_RIGHT );  break;  // right portion

                    case 'P':   // page number
                        appendField( createField( maPageNumberService ) );
                    break;
                    case 'N':   // total page count
                        appendField( createField( maPageCountService ) );
                    break;
                    case 'A':   // current sheet name
                        appendField( createField( maSheetNameService ) );
                    break;

                    case 'F':   // file name
                    {
                        Reference< XTextContent > xContent = createField( maFileNameService );
                        PropertySet aPropSet( xContent );
                        aPropSet.setProperty( PROP_FileFormat, ::com::sun::star::text::FilenameDisplayFormat::NAME_AND_EXT );
                        appendField( xContent );
                    }
                    break;
                    case 'Z':   // file path (without file name), OOXML, BIFF12, and BIFF8 only
                        if( (getFilterType() == FILTER_OOXML) || ((getFilterType() == FILTER_BIFF) && (getBiff() == BIFF8)) )
                        {
                            Reference< XTextContent > xContent = createField( maFileNameService );
                            PropertySet aPropSet( xContent );
                            // FilenameDisplayFormat::PATH not supported by Calc
                            aPropSet.setProperty( PROP_FileFormat, ::com::sun::star::text::FilenameDisplayFormat::FULL );
                            appendField( xContent );
                            /*  path only is not supported -- if we find a '&Z&F'
                                combination for path/name, skip the '&F' part */
                            if( (pcChar + 2 < pcEnd) && (pcChar[ 1 ] == '&') && ((pcChar[ 2 ] == 'f') || (pcChar[ 2 ] == 'F')) )
                                pcChar += 2;
                        }
                    break;
                    case 'D':   // date
                    {
                        Reference< XTextContent > xContent = createField( maDateTimeService );
                        PropertySet aPropSet( xContent );
                        aPropSet.setProperty( PROP_IsDate, true );
                        appendField( xContent );
                    }
                    break;
                    case 'T':   // time
                    {
                        Reference< XTextContent > xContent = createField( maDateTimeService );
                        PropertySet aPropSet( xContent );
                        aPropSet.setProperty( PROP_IsDate, false );
                        appendField( xContent );
                    }
                    break;

                    case 'B':   // bold
                        setAttributes();
                        maFontModel.mbBold = !maFontModel.mbBold;
                    break;
                    case 'I':   // italic
                        setAttributes();
                        maFontModel.mbItalic = !maFontModel.mbItalic;
                    break;
                    case 'U':   // underline
                        setAttributes();
                        maFontModel.mnUnderline = (maFontModel.mnUnderline == XML_single) ? XML_none : XML_single;
                    break;
                    case 'E':   // double underline
                        setAttributes();
                        maFontModel.mnUnderline = (maFontModel.mnUnderline == XML_double) ? XML_none : XML_double;
                    break;
                    case 'S':   // strikeout
                        setAttributes();
                        maFontModel.mbStrikeout = !maFontModel.mbStrikeout;
                    break;
                    case 'X':   // superscript
                        setAttributes();
                        maFontModel.mnEscapement = (maFontModel.mnEscapement == XML_superscript) ? XML_baseline : XML_superscript;
                    break;
                    case 'Y':   // subsrcipt
                        setAttributes();
                        maFontModel.mnEscapement = (maFontModel.mnEscapement == XML_subscript) ? XML_baseline : XML_subscript;
                    break;
                    case 'O':   // outlined
                        setAttributes();
                        maFontModel.mbOutline = !maFontModel.mbOutline;
                    break;
                    case 'H':   // shadow
                        setAttributes();
                        maFontModel.mbShadow = !maFontModel.mbShadow;
                    break;

                    case 'K':   // text color (not in BIFF)
                        if( (getFilterType() == FILTER_OOXML) && (pcChar + 6 < pcEnd) )
                        {
                            setAttributes();
                            // eat the following 6 characters
                            convertFontColor( OUString( pcChar + 1, 6 ) );
                            pcChar += 6;
                        }
                    break;

                    case '\"':  // font name
                        aFontName.setLength( 0 );
                        aFontStyle.setLength( 0 );
                        eState = STATE_FONTNAME;
                    break;
                    default:
                        if( ('0' <= cChar) && (cChar <= '9') )    // font size
                        {
                            nFontHeight = cChar - '0';
                            eState = STATE_FONTHEIGHT;
                        }
                }
            }
            break;

            case STATE_FONTNAME:
            {
                switch( cChar )
                {
                    case '\"':
                        setAttributes();
                        convertFontName( aFontName.makeStringAndClear() );
                        eState = STATE_TEXT;
                    break;
                    case ',':
                        eState = STATE_FONTSTYLE;
                    break;
                    default:
                        aFontName.append( cChar );
                }
            }
            break;

            case STATE_FONTSTYLE:
            {
                switch( cChar )
                {
                    case '\"':
                        setAttributes();
                        convertFontName( aFontName.makeStringAndClear() );
                        convertFontStyle( aFontStyle.makeStringAndClear() );
                        eState = STATE_TEXT;
                    break;
                    default:
                        aFontStyle.append( cChar );
                }
            }
            break;

            case STATE_FONTHEIGHT:
            {
                if( ('0' <= cChar) && (cChar <= '9') )
                {
                    if( nFontHeight >= 0 )
                    {
                        nFontHeight *= 10;
                        nFontHeight += (cChar - '0');
                        if( nFontHeight > 1000 )
                            nFontHeight = -1;
                    }
                }
                else
                {
                    if( nFontHeight > 0 )
                    {
                        setAttributes();
                        maFontModel.mfHeight = nFontHeight;
                    }
                    --pcChar;
                    eState = STATE_TEXT;
                }
            }
            break;
        }
    }

    // finalize
    finalizePortion();
    maPortions[ HF_LEFT   ].mfTotalHeight += getCurrHeight( HF_LEFT );
    maPortions[ HF_CENTER ].mfTotalHeight += getCurrHeight( HF_CENTER );
    maPortions[ HF_RIGHT  ].mfTotalHeight += getCurrHeight( HF_RIGHT );

    return ::std::max( maPortions[ HF_LEFT ].mfTotalHeight,
        ::std::max( maPortions[ HF_CENTER ].mfTotalHeight, maPortions[ HF_RIGHT ].mfTotalHeight ) );
}

// private --------------------------------------------------------------------

double HeaderFooterParser::getCurrHeight( HFPortionId ePortion ) const
{
    double fMaxHt = maPortions[ ePortion ].mfCurrHeight;
    return (fMaxHt == 0.0) ? maFontModel.mfHeight : fMaxHt;
}

double HeaderFooterParser::getCurrHeight() const
{
    return getCurrHeight( meCurrPortion );
}

void HeaderFooterParser::updateCurrHeight( HFPortionId ePortion )
{
    double& rfMaxHt = maPortions[ ePortion ].mfCurrHeight;
    rfMaxHt = ::std::max( rfMaxHt, maFontModel.mfHeight );
}

void HeaderFooterParser::updateCurrHeight()
{
    updateCurrHeight( meCurrPortion );
}

void HeaderFooterParser::setAttributes()
{
    Reference< XTextRange > xRange( getStartPos(), UNO_QUERY );
    getEndPos()->gotoRange( xRange, sal_False );
    getEndPos()->gotoEnd( sal_True );
    if( !getEndPos()->isCollapsed() )
    {
        Font aFont( *this, maFontModel );
        aFont.finalizeImport();
        PropertySet aPropSet( getEndPos() );
        aFont.writeToPropertySet( aPropSet, FONT_PROPTYPE_TEXT );
        getStartPos()->gotoEnd( sal_False );
        getEndPos()->gotoEnd( sal_False );
    }
}

void HeaderFooterParser::appendText()
{
    if( maBuffer.getLength() > 0 )
    {
        getEndPos()->gotoEnd( sal_False );
        getEndPos()->setString( maBuffer.makeStringAndClear() );
        updateCurrHeight();
    }
}

void HeaderFooterParser::appendLineBreak()
{
    getEndPos()->gotoEnd( sal_False );
    getEndPos()->setString( OUString( sal_Unicode( '\n' ) ) );
    getPortion().mfTotalHeight += getCurrHeight();
    getPortion().mfCurrHeight = 0;
}

Reference< XTextContent > HeaderFooterParser::createField( const OUString& rServiceName ) const
{
    Reference< XTextContent > xContent;
    try
    {
        xContent.set( getBaseFilter().getModelFactory()->createInstance( rServiceName ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
        OSL_FAIL( OStringBuffer( "HeaderFooterParser::createField - error while creating text field \"" ).
            append( OUStringToOString( rServiceName, RTL_TEXTENCODING_ASCII_US ) ).
            append( '"' ).getStr() );
    }
    return xContent;
}

void HeaderFooterParser::appendField( const Reference< XTextContent >& rxContent )
{
    getEndPos()->gotoEnd( sal_False );
    try
    {
        Reference< XTextRange > xRange( getEndPos(), UNO_QUERY_THROW );
        getPortion().mxText->insertTextContent( xRange, rxContent, sal_False );
        updateCurrHeight();
    }
    catch( Exception& )
    {
    }
}

void HeaderFooterParser::convertFontName( const OUString& rName )
{
    if( !rName.isEmpty() )
    {
        // single dash is document default font
        if( (rName.getLength() == 1) && (rName[ 0 ] == '-') )
            maFontModel.maName = getStyles().getDefaultFontModel().maName;
        else
            maFontModel.maName = rName;
    }
}

void HeaderFooterParser::convertFontStyle( const OUString& rStyle )
{
    maFontModel.mbBold = maFontModel.mbItalic = false;
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rStyle.getLength();
    while( (0 <= nPos) && (nPos < nLen) )
    {
        OString aToken = OUStringToOString( rStyle.getToken( 0, ' ', nPos ), RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase();
        if( !aToken.isEmpty() )
        {
            if( maBoldNames.count( aToken ) > 0 )
                maFontModel.mbBold = true;
            else if( maItalicNames.count( aToken ) > 0 )
                maFontModel.mbItalic = true;
        }
    }
}

void HeaderFooterParser::convertFontColor( const OUString& rColor )
{
    OSL_ENSURE( rColor.getLength() == 6, "HeaderFooterParser::convertFontColor - invalid font color code" );
    if( (rColor[ 2 ] == '+') || (rColor[ 2 ] == '-') )
        // theme color: TTSNNN (TT = decimal theme index, S = +/-, NNN = decimal tint/shade in percent)
        maFontModel.maColor.setTheme(
            rColor.copy( 0, 2 ).toInt32(),
            static_cast< double >( rColor.copy( 2 ).toInt32() ) / 100.0 );
    else
        // RGB color: RRGGBB
        maFontModel.maColor.setRgb( rColor.toInt32( 16 ) );
}

void HeaderFooterParser::finalizePortion()
{
    appendText();
    setAttributes();
}

void HeaderFooterParser::setNewPortion( HFPortionId ePortion )
{
    if( ePortion != meCurrPortion )
    {
        finalizePortion();
        meCurrPortion = ePortion;
        maFontModel = getStyles().getDefaultFontModel();
    }
}

// ============================================================================

namespace {

/** Paper size in 1/100 millimeters. */
struct ApiPaperSize
{
    sal_Int32           mnWidth;
    sal_Int32           mnHeight;
};

#define IN2MM100( v )    static_cast< sal_Int32 >( (v) * 2540.0 + 0.5 )
#define MM2MM100( v )    static_cast< sal_Int32 >( (v) * 100.0 + 0.5 )

static const ApiPaperSize spPaperSizeTable[] =
{
    { 0, 0 },                                                //  0 - (undefined)
    { IN2MM100( 8.5 ),       IN2MM100( 11 )      },          //  1 - Letter paper
    { IN2MM100( 8.5 ),       IN2MM100( 11 )      },          //  2 - Letter small paper
    { IN2MM100( 11 ),        IN2MM100( 17 )      },          //  3 - Tabloid paper
    { IN2MM100( 17 ),        IN2MM100( 11 )      },          //  4 - Ledger paper
    { IN2MM100( 8.5 ),       IN2MM100( 14 )      },          //  5 - Legal paper
    { IN2MM100( 5.5 ),       IN2MM100( 8.5 )     },          //  6 - Statement paper
    { IN2MM100( 7.25 ),      IN2MM100( 10.5 )    },          //  7 - Executive paper
    { MM2MM100( 297 ),       MM2MM100( 420 )     },          //  8 - A3 paper
    { MM2MM100( 210 ),       MM2MM100( 297 )     },          //  9 - A4 paper
    { MM2MM100( 210 ),       MM2MM100( 297 )     },          // 10 - A4 small paper
    { MM2MM100( 148 ),       MM2MM100( 210 )     },          // 11 - A5 paper
    { MM2MM100( 250 ),       MM2MM100( 353 )     },          // 12 - B4 paper
    { MM2MM100( 176 ),       MM2MM100( 250 )     },          // 13 - B5 paper
    { IN2MM100( 8.5 ),       IN2MM100( 13 )      },          // 14 - Folio paper
    { MM2MM100( 215 ),       MM2MM100( 275 )     },          // 15 - Quarto paper
    { IN2MM100( 10 ),        IN2MM100( 14 )      },          // 16 - Standard paper
    { IN2MM100( 11 ),        IN2MM100( 17 )      },          // 17 - Standard paper
    { IN2MM100( 8.5 ),       IN2MM100( 11 )      },          // 18 - Note paper
    { IN2MM100( 3.875 ),     IN2MM100( 8.875 )   },          // 19 - #9 envelope
    { IN2MM100( 4.125 ),     IN2MM100( 9.5 )     },          // 20 - #10 envelope
    { IN2MM100( 4.5 ),       IN2MM100( 10.375 )  },          // 21 - #11 envelope
    { IN2MM100( 4.75 ),      IN2MM100( 11 )      },          // 22 - #12 envelope
    { IN2MM100( 5 ),         IN2MM100( 11.5 )    },          // 23 - #14 envelope
    { IN2MM100( 17 ),        IN2MM100( 22 )      },          // 24 - C paper
    { IN2MM100( 22 ),        IN2MM100( 34 )      },          // 25 - D paper
    { IN2MM100( 34 ),        IN2MM100( 44 )      },          // 26 - E paper
    { MM2MM100( 110 ),       MM2MM100( 220 )     },          // 27 - DL envelope
    { MM2MM100( 162 ),       MM2MM100( 229 )     },          // 28 - C5 envelope
    { MM2MM100( 324 ),       MM2MM100( 458 )     },          // 29 - C3 envelope
    { MM2MM100( 229 ),       MM2MM100( 324 )     },          // 30 - C4 envelope
    { MM2MM100( 114 ),       MM2MM100( 162 )     },          // 31 - C6 envelope
    { MM2MM100( 114 ),       MM2MM100( 229 )     },          // 32 - C65 envelope
    { MM2MM100( 250 ),       MM2MM100( 353 )     },          // 33 - B4 envelope
    { MM2MM100( 176 ),       MM2MM100( 250 )     },          // 34 - B5 envelope
    { MM2MM100( 176 ),       MM2MM100( 125 )     },          // 35 - B6 envelope
    { MM2MM100( 110 ),       MM2MM100( 230 )     },          // 36 - Italy envelope
    { IN2MM100( 3.875 ),     IN2MM100( 7.5 )     },          // 37 - Monarch envelope
    { IN2MM100( 3.625 ),     IN2MM100( 6.5 )     },          // 38 - 6 3/4 envelope
    { IN2MM100( 14.875 ),    IN2MM100( 11 )      },          // 39 - US standard fanfold
    { IN2MM100( 8.5 ),       IN2MM100( 12 )      },          // 40 - German standard fanfold
    { IN2MM100( 8.5 ),       IN2MM100( 13 )      },          // 41 - German legal fanfold
    { MM2MM100( 250 ),       MM2MM100( 353 )     },          // 42 - ISO B4
    { MM2MM100( 200 ),       MM2MM100( 148 )     },          // 43 - Japanese double postcard
    { IN2MM100( 9 ),         IN2MM100( 11 )      },          // 44 - Standard paper
    { IN2MM100( 10 ),        IN2MM100( 11 )      },          // 45 - Standard paper
    { IN2MM100( 15 ),        IN2MM100( 11 )      },          // 46 - Standard paper
    { MM2MM100( 220 ),       MM2MM100( 220 )     },          // 47 - Invite envelope
    { 0, 0 },                                                // 48 - (undefined)
    { 0, 0 },                                                // 49 - (undefined)
    { IN2MM100( 9.275 ),     IN2MM100( 12 )      },          // 50 - Letter extra paper
    { IN2MM100( 9.275 ),     IN2MM100( 15 )      },          // 51 - Legal extra paper
    { IN2MM100( 11.69 ),     IN2MM100( 18 )      },          // 52 - Tabloid extra paper
    { MM2MM100( 236 ),       MM2MM100( 322 )     },          // 53 - A4 extra paper
    { IN2MM100( 8.275 ),     IN2MM100( 11 )      },          // 54 - Letter transverse paper
    { MM2MM100( 210 ),       MM2MM100( 297 )     },          // 55 - A4 transverse paper
    { IN2MM100( 9.275 ),     IN2MM100( 12 )      },          // 56 - Letter extra transverse paper
    { MM2MM100( 227 ),       MM2MM100( 356 )     },          // 57 - SuperA/SuperA/A4 paper
    { MM2MM100( 305 ),       MM2MM100( 487 )     },          // 58 - SuperB/SuperB/A3 paper
    { IN2MM100( 8.5 ),       IN2MM100( 12.69 )   },          // 59 - Letter plus paper
    { MM2MM100( 210 ),       MM2MM100( 330 )     },          // 60 - A4 plus paper
    { MM2MM100( 148 ),       MM2MM100( 210 )     },          // 61 - A5 transverse paper
    { MM2MM100( 182 ),       MM2MM100( 257 )     },          // 62 - JIS B5 transverse paper
    { MM2MM100( 322 ),       MM2MM100( 445 )     },          // 63 - A3 extra paper
    { MM2MM100( 174 ),       MM2MM100( 235 )     },          // 64 - A5 extra paper
    { MM2MM100( 201 ),       MM2MM100( 276 )     },          // 65 - ISO B5 extra paper
    { MM2MM100( 420 ),       MM2MM100( 594 )     },          // 66 - A2 paper
    { MM2MM100( 297 ),       MM2MM100( 420 )     },          // 67 - A3 transverse paper
    { MM2MM100( 322 ),       MM2MM100( 445 )     }           // 68 - A3 extra transverse paper
};

} // namespace

// ----------------------------------------------------------------------------

PageSettingsConverter::HFHelperData::HFHelperData( sal_Int32 nLeftPropId, sal_Int32 nRightPropId ) :
    mnLeftPropId( nLeftPropId ),
    mnRightPropId( nRightPropId ),
    mnHeight( 0 ),
    mnBodyDist( 0 ),
    mbHasContent( false ),
    mbShareOddEven( false ),
    mbDynamicHeight( false )
{
}

// ----------------------------------------------------------------------------

PageSettingsConverter::PageSettingsConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mxHFParser( new HeaderFooterParser( rHelper ) ),
    maHeaderData( PROP_LeftPageHeaderContent, PROP_RightPageHeaderContent ),
    maFooterData( PROP_LeftPageFooterContent, PROP_RightPageFooterContent )
{
}

PageSettingsConverter::~PageSettingsConverter()
{
}

void PageSettingsConverter::writePageSettingsProperties(
        PropertySet& rPropSet, const PageSettingsModel& rModel, WorksheetType eSheetType )
{
    // special handling for chart sheets
    bool bChartSheet = eSheetType == SHEETTYPE_CHARTSHEET;

    // printout scaling
    if( bChartSheet )
    {
        // always fit chart sheet to 1 page
        rPropSet.setProperty< sal_Int16 >( PROP_ScaleToPages, 1 );
    }
    else if( rModel.mbFitToPages )
    {
        // fit to number of pages
        rPropSet.setProperty( PROP_ScaleToPagesX, getLimitedValue< sal_Int16, sal_Int32 >( rModel.mnFitToWidth, 0, 1000 ) );
        rPropSet.setProperty( PROP_ScaleToPagesY, getLimitedValue< sal_Int16, sal_Int32 >( rModel.mnFitToHeight, 0, 1000 ) );
    }
    else
    {
        // scale may be 0 which indicates uninitialized
        sal_Int16 nScale = (rModel.mbValidSettings && (rModel.mnScale > 0)) ? getLimitedValue< sal_Int16, sal_Int32 >( rModel.mnScale, 10, 400 ) : 100;
        rPropSet.setProperty( PROP_PageScale, nScale );
    }

    // paper orientation
    bool bLandscape = rModel.mnOrientation == XML_landscape;
    // default orientation for current sheet type (chart sheets default to landscape)
    if( bChartSheet && ( !rModel.mbValidSettings || (rModel.mnOrientation == XML_default) ) )
        bLandscape = true;

    // paper size
    if( !rModel.mbValidSettings )
    {
        Size aSize;
        bool bValid = false;

        if( (0 < rModel.mnPaperSize) && (rModel.mnPaperSize < static_cast< sal_Int32 >( STATIC_ARRAY_SIZE( spPaperSizeTable ) )) )
        {
            const ApiPaperSize& rPaperSize = spPaperSizeTable[ rModel.mnPaperSize ];
            aSize = Size( rPaperSize.mnWidth, rPaperSize.mnHeight );
            bValid = true;
        }
        if( rModel.mnPaperWidth > 0 && rModel.mnPaperHeight > 0 )
        {
            aSize = Size( rModel.mnPaperWidth, rModel.mnPaperHeight );
            bValid = true;
        }

        if( bValid )
        {
            if( bLandscape )
                ::std::swap( aSize.Width, aSize.Height );
            rPropSet.setProperty( PROP_Size, aSize );
        }
    }

    // header/footer
    convertHeaderFooterData( rPropSet, maHeaderData, rModel.maOddHeader, rModel.maEvenHeader, rModel.mbUseEvenHF, rModel.mfTopMargin,    rModel.mfHeaderMargin );
    convertHeaderFooterData( rPropSet, maFooterData, rModel.maOddFooter, rModel.maEvenFooter, rModel.mbUseEvenHF, rModel.mfBottomMargin, rModel.mfFooterMargin );

    // write all properties to property set
    const UnitConverter& rUnitConv = getUnitConverter();
    PropertyMap aPropMap;
    aPropMap[ PROP_IsLandscape ]           <<= bLandscape;
    aPropMap[ PROP_FirstPageNumber ]       <<= getLimitedValue< sal_Int16, sal_Int32 >( rModel.mbUseFirstPage ? rModel.mnFirstPage : 0, 0, 9999 );
    aPropMap[ PROP_PrintDownFirst ]        <<= (rModel.mnPageOrder == XML_downThenOver);
    aPropMap[ PROP_PrintAnnotations ]      <<= (rModel.mnCellComments == XML_asDisplayed);
    aPropMap[ PROP_CenterHorizontally ]    <<= rModel.mbHorCenter;
    aPropMap[ PROP_CenterVertically ]      <<= rModel.mbVerCenter;
    aPropMap[ PROP_PrintGrid ]             <<= (!bChartSheet && rModel.mbPrintGrid);     // no gridlines in chart sheets
    aPropMap[ PROP_PrintHeaders ]          <<= (!bChartSheet && rModel.mbPrintHeadings); // no column/row headings in chart sheets
    aPropMap[ PROP_LeftMargin ]            <<= rUnitConv.scaleToMm100( rModel.mfLeftMargin, UNIT_INCH );
    aPropMap[ PROP_RightMargin ]           <<= rUnitConv.scaleToMm100( rModel.mfRightMargin, UNIT_INCH );
    // #i23296# In Calc, "TopMargin" property is distance to top of header if enabled
    aPropMap[ PROP_TopMargin ]             <<= rUnitConv.scaleToMm100( maHeaderData.mbHasContent ? rModel.mfHeaderMargin : rModel.mfTopMargin, UNIT_INCH );
    // #i23296# In Calc, "BottomMargin" property is distance to bottom of footer if enabled
    aPropMap[ PROP_BottomMargin ]          <<= rUnitConv.scaleToMm100( maFooterData.mbHasContent ? rModel.mfFooterMargin : rModel.mfBottomMargin, UNIT_INCH );
    aPropMap[ PROP_HeaderIsOn ]            <<= maHeaderData.mbHasContent;
    aPropMap[ PROP_HeaderIsShared ]        <<= maHeaderData.mbShareOddEven;
    aPropMap[ PROP_HeaderIsDynamicHeight ] <<= maHeaderData.mbDynamicHeight;
    aPropMap[ PROP_HeaderHeight ]          <<= maHeaderData.mnHeight;
    aPropMap[ PROP_HeaderBodyDistance ]    <<= maHeaderData.mnBodyDist;
    aPropMap[ PROP_FooterIsOn ]            <<= maFooterData.mbHasContent;
    aPropMap[ PROP_FooterIsShared ]        <<= maFooterData.mbShareOddEven;
    aPropMap[ PROP_FooterIsDynamicHeight ] <<= maFooterData.mbDynamicHeight;
    aPropMap[ PROP_FooterHeight ]          <<= maFooterData.mnHeight;
    aPropMap[ PROP_FooterBodyDistance ]    <<= maFooterData.mnBodyDist;
    // background image
    if( !rModel.maGraphicUrl.isEmpty() )
    {
        aPropMap[ PROP_BackGraphicURL ] <<= rModel.maGraphicUrl;
        aPropMap[ PROP_BackGraphicLocation ] <<= ::com::sun::star::style::GraphicLocation_TILED;
    }

    rPropSet.setProperties( aPropMap );
}

void PageSettingsConverter::convertHeaderFooterData(
        PropertySet& rPropSet, HFHelperData& orHFData,
        const OUString rOddContent, const OUString rEvenContent, bool bUseEvenContent,
        double fPageMargin, double fContentMargin )
{
    bool bHasOddContent  = !rOddContent.isEmpty();
    bool bHasEvenContent = bUseEvenContent && !rEvenContent.isEmpty();

    sal_Int32 nOddHeight  = bHasOddContent  ? writeHeaderFooter( rPropSet, orHFData.mnRightPropId, rOddContent  ) : 0;
    sal_Int32 nEvenHeight = bHasEvenContent ? writeHeaderFooter( rPropSet, orHFData.mnLeftPropId,  rEvenContent ) : 0;

    orHFData.mnHeight = 750;
    orHFData.mnBodyDist = 250;
    orHFData.mbHasContent = bHasOddContent || bHasEvenContent;
    orHFData.mbShareOddEven = !bUseEvenContent;
    orHFData.mbDynamicHeight = true;

    if( orHFData.mbHasContent )
    {
        // use maximum height of odd/even header/footer
        orHFData.mnHeight = ::std::max( nOddHeight, nEvenHeight );
        /*  Calc contains distance between bottom of header and top of page
            body in "HeaderBodyDistance" property, and distance between bottom
            of page body and top of footer in "FooterBodyDistance" property */
        orHFData.mnBodyDist = getUnitConverter().scaleToMm100( fPageMargin - fContentMargin, UNIT_INCH ) - orHFData.mnHeight;
        /*  #i23296# Distance less than 0 means, header or footer overlays page
            body. As this is not possible in Calc, set fixed header or footer
            height (crop header/footer) to get correct top position of page body. */
        orHFData.mbDynamicHeight = orHFData.mnBodyDist >= 0;
        /*  "HeaderHeight" property is in fact distance from top of header to
            top of page body (including "HeaderBodyDistance").
            "FooterHeight" property is in fact distance from bottom of page
            body to bottom of footer (including "FooterBodyDistance"). */
        orHFData.mnHeight += orHFData.mnBodyDist;
        // negative body distance not allowed
        orHFData.mnBodyDist = ::std::max< sal_Int32 >( orHFData.mnBodyDist, 0 );
    }
}

sal_Int32 PageSettingsConverter::writeHeaderFooter(
        PropertySet& rPropSet, sal_Int32 nPropId, const OUString& rContent )
{
    OSL_ENSURE( !rContent.isEmpty(), "PageSettingsConverter::writeHeaderFooter - empty h/f string found" );
    sal_Int32 nHeight = 0;
    if( !rContent.isEmpty() )
    {
        Reference< XHeaderFooterContent > xHFContent( rPropSet.getAnyProperty( nPropId ), UNO_QUERY );
        if( xHFContent.is() )
        {
            double fTotalHeight = mxHFParser->parse( xHFContent, rContent );
            rPropSet.setProperty( nPropId, xHFContent );
            nHeight = getUnitConverter().scaleToMm100( fTotalHeight, UNIT_POINT );
        }
    }
    return nHeight;
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
