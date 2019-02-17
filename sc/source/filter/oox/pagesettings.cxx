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

#include <pagesettings.hxx>

#include <algorithm>
#include <set>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/core/relations.hxx>
#include <stylesbuffer.hxx>
#include <unitconverter.hxx>
#include <document.hxx>
#include <biffhelper.hxx>
#include <filter/msfilter/util.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::oox::core::Relations;

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

} // namespace

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
            maModel.mnPaperWidth, aStr);
    aStr                    = rAttribs.getString ( XML_paperHeight, OUString() );
    ::sax::Converter::convertMeasure(
            maModel.mnPaperHeight, aStr );
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
            maModel.mnPaperWidth, aStr );
    aStr                    = rAttribs.getString ( XML_paperHeight, OUString() );
    ::sax::Converter::convertMeasure(
            maModel.mnPaperHeight, aStr );
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
    maModel.mfLeftMargin = rStrm.readDouble();
    maModel.mfRightMargin = rStrm.readDouble();
    maModel.mfTopMargin = rStrm.readDouble();
    maModel.mfBottomMargin = rStrm.readDouble();
    maModel.mfHeaderMargin = rStrm.readDouble();
    maModel.mfFooterMargin = rStrm.readDouble();
}

void PageSettings::importPrintOptions( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags;
    nFlags = rStrm.readuInt16();
    maModel.mbHorCenter     = getFlag( nFlags, BIFF12_PRINTOPT_HORCENTER );
    maModel.mbVerCenter     = getFlag( nFlags, BIFF12_PRINTOPT_VERCENTER );
    maModel.mbPrintGrid     = getFlag( nFlags, BIFF12_PRINTOPT_PRINTGRID );
    maModel.mbPrintHeadings = getFlag( nFlags, BIFF12_PRINTOPT_PRINTHEADING );
}

void PageSettings::importPageSetup( const Relations& rRelations, SequenceInputStream& rStrm )
{
    OUString aRelId;
    sal_uInt16 nFlags;
    maModel.mnPaperSize = rStrm.readInt32();
    maModel.mnScale = rStrm.readInt32();
    maModel.mnHorPrintRes = rStrm.readInt32();
    maModel.mnVerPrintRes = rStrm.readInt32();
    maModel.mnCopies = rStrm.readInt32();
    maModel.mnFirstPage = rStrm.readInt32();
    maModel.mnFitToWidth = rStrm.readInt32();
    maModel.mnFitToHeight = rStrm.readInt32();
    nFlags = rStrm.readuInt16();
    rStrm >> aRelId;
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
    maModel.mnPaperSize = rStrm.readInt32();
    maModel.mnHorPrintRes = rStrm.readInt32();
    maModel.mnVerPrintRes = rStrm.readInt32();
    maModel.mnCopies = rStrm.readInt32();
    nFirstPage = rStrm.readuInt16();
    nFlags = rStrm.readuInt16();
    rStrm >> aRelId;
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
    nFlags = rStrm.readuInt16();
    rStrm   >> maModel.maOddHeader   >> maModel.maOddFooter
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
    Reference<container::XNamed> xSheetName(getSheet(), UNO_QUERY);
    if( xSheetName.is() )
        aStyleNameBuffer.append( xSheetName->getName() );
    else
        aStyleNameBuffer.append( static_cast< sal_Int32 >( getSheetIndex() + 1 ) );
    OUString aStyleName = aStyleNameBuffer.makeStringAndClear();

    Reference<style::XStyle> xStyle = createStyleObject(aStyleName, true);
    PropertySet aStyleProps( xStyle );
    getPageSettingsConverter().writePageSettingsProperties( aStyleProps, maModel, getSheetType() );

    // Set page style name to the sheet.
    SCTAB nTab = getSheetIndex();
    getScDocument().SetPageStyle(nTab, aStyleName);
}

void PageSettings::importPictureData( const Relations& rRelations, const OUString& rRelId )
{
    OUString aPicturePath = rRelations.getFragmentPathFromRelId(rRelId);
    if (!aPicturePath.isEmpty())
    {
        maModel.mxGraphic = getBaseFilter().getGraphicHelper().importEmbeddedGraphic(aPicturePath);
    }
}

enum HFPortionId
{
    HF_LEFT,
    HF_CENTER,
    HF_RIGHT,
    HF_COUNT
};

struct HFPortionInfo
{
    Reference<text::XText>  mxText;                 /// XText interface of this portion.
    Reference<text::XTextCursor> mxStart;           /// Start position of current text range for formatting.
    Reference<text::XTextCursor> mxEnd;             /// End position of current text range for formatting.
    double              mfTotalHeight;          /// Sum of heights of previous lines in points.
    double              mfCurrHeight;           /// Height of the current text line in points.

    bool                initialize( const Reference<text::XText>& rxText );
};

bool HFPortionInfo::initialize( const Reference<text::XText>& rxText )
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

class HeaderFooterParser : public WorkbookHelper
{
public:
    explicit            HeaderFooterParser( const WorkbookHelper& rHelper );

    /** Parses the passed string and creates the header/footer contents.
        @returns  The total height of the converted header or footer in points. */
    double              parse(
                            const Reference<sheet::XHeaderFooterContent>& rxContext,
                            const OUString& rData );

private:
    /** Returns the current edit engine text object. */
    HFPortionInfo& getPortion() { return maPortions[ meCurrPortion ]; }
    /** Returns the start cursor of the current text range. */
    const Reference<text::XTextCursor>& getStartPos() { return getPortion().mxStart; }
    /** Returns the end cursor of the current text range. */
    const Reference<text::XTextCursor>& getEndPos() { return getPortion().mxEnd; }

    /** Returns the current line height of the specified portion. */
    double              getCurrHeight( HFPortionId ePortion ) const;

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
    Reference<text::XTextContent> createField( const OUString& rServiceName ) const;
    /** Appends the passed text field. */
    void                appendField( const Reference<text::XTextContent>& rxContent );

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

    const std::set< OString >    maBoldNames;            /// All names for bold font style in lowercase UTF-8.
    const std::set< OString >    maItalicNames;          /// All names for italic font style in lowercase UTF-8.
    HFPortionInfoVec    maPortions;
    HFPortionId         meCurrPortion;          /// Identifier of current H/F portion.
    OUStringBuffer      maBuffer;               /// Text data to append to current text range.
    FontModel           maFontModel;            /// Font attributes of current text range.
};

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

static const OUStringLiteral gaPageNumberService( "com.sun.star.text.TextField.PageNumber" );
static const OUStringLiteral gaPageCountService( "com.sun.star.text.TextField.PageCount" );
static const OUStringLiteral gaSheetNameService( "com.sun.star.text.TextField.SheetName" );
static const OUStringLiteral gaFileNameService( "com.sun.star.text.TextField.FileName" );
static const OUStringLiteral gaDateTimeService( "com.sun.star.text.TextField.DateTime" );

HeaderFooterParser::HeaderFooterParser( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maBoldNames( sppcBoldNames, sppcBoldNames + SAL_N_ELEMENTS(sppcBoldNames) ),
    maItalicNames( sppcItalicNames, sppcItalicNames + SAL_N_ELEMENTS(sppcItalicNames) ),
    maPortions( static_cast< size_t >( HF_COUNT ) ),
    meCurrPortion( HF_CENTER )
{
}

double HeaderFooterParser::parse( const Reference<sheet::XHeaderFooterContent>& rxContext, const OUString& rData )
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
                    cChar = (cChar - 'a') + 'A';
                switch( cChar )
                {
                    case '&':   maBuffer.append( cChar );   break;  // the '&' character

                    case 'L':   setNewPortion( HF_LEFT );   break;  // left portion
                    case 'C':   setNewPortion( HF_CENTER ); break;  // center portion
                    case 'R':   setNewPortion( HF_RIGHT );  break;  // right portion

                    case 'P':   // page number
                        appendField( createField( gaPageNumberService ) );
                    break;
                    case 'N':   // total page count
                        appendField( createField( gaPageCountService ) );
                    break;
                    case 'A':   // current sheet name
                        appendField( createField( gaSheetNameService ) );
                    break;

                    case 'F':   // file name
                    {
                        Reference<text::XTextContent> xContent = createField( gaFileNameService );
                        PropertySet aPropSet( xContent );
                        aPropSet.setProperty( PROP_FileFormat, css::text::FilenameDisplayFormat::NAME_AND_EXT );
                        appendField( xContent );
                    }
                    break;
                    case 'Z':   // file path (without file name), OOXML, BIFF12, and BIFF8 only
                    {
                        Reference<text::XTextContent> xContent = createField( gaFileNameService );
                        PropertySet aPropSet( xContent );
                        // FilenameDisplayFormat::PATH not supported by Calc
                        aPropSet.setProperty( PROP_FileFormat, css::text::FilenameDisplayFormat::FULL );
                        appendField( xContent );
                        /*  path only is not supported -- if we find a '&Z&F'
                            combination for path/name, skip the '&F' part */
                        if( (pcChar + 2 < pcEnd) && (pcChar[ 1 ] == '&') && ((pcChar[ 2 ] == 'f') || (pcChar[ 2 ] == 'F')) )
                            pcChar += 2;
                    }
                    break;
                    case 'D':   // date
                    {
                        Reference<text::XTextContent> xContent = createField( gaDateTimeService );
                        PropertySet aPropSet( xContent );
                        aPropSet.setProperty( PROP_IsDate, true );
                        appendField( xContent );
                    }
                    break;
                    case 'T':   // time
                    {
                        Reference<text::XTextContent> xContent = createField( gaDateTimeService );
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
                        if( pcChar + 6 < pcEnd )
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
    Reference<text::XTextRange> xRange( getStartPos(), UNO_QUERY );
    getEndPos()->gotoRange( xRange, false );
    getEndPos()->gotoEnd( true );
    if( !getEndPos()->isCollapsed() )
    {
        Font aFont( *this, maFontModel );
        aFont.finalizeImport();
        PropertySet aPropSet( getEndPos() );
        aFont.writeToPropertySet( aPropSet );
        getStartPos()->gotoEnd( false );
        getEndPos()->gotoEnd( false );
    }
}

void HeaderFooterParser::appendText()
{
    if( !maBuffer.isEmpty() )
    {
        getEndPos()->gotoEnd( false );
        getEndPos()->setString( maBuffer.makeStringAndClear() );
        updateCurrHeight();
    }
}

void HeaderFooterParser::appendLineBreak()
{
    getEndPos()->gotoEnd( false );
    getEndPos()->setString( OUString( '\n' ) );
    getPortion().mfTotalHeight += getCurrHeight( meCurrPortion ); // add the current line height.
    getPortion().mfCurrHeight = 0;
}

Reference<text::XTextContent> HeaderFooterParser::createField( const OUString& rServiceName ) const
{
    Reference<text::XTextContent> xContent;
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

void HeaderFooterParser::appendField( const Reference<text::XTextContent>& rxContent )
{
    getEndPos()->gotoEnd( false );
    try
    {
        Reference<text::XTextRange> xRange( getEndPos(), UNO_QUERY_THROW );
        getPortion().mxText->insertTextContent( xRange, rxContent, false );
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
    if (rStyle.isEmpty())
        return;
    for( sal_Int32 nPos{ 0 }; nPos>=0; )
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
        maFontModel.maColor.setRgb( ::Color(rColor.toUInt32( 16 )) );
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
    bool bChartSheet = eSheetType == WorksheetType::Chart;

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
        sal_Int16 nScale = (rModel.mnScale > 0) ? getLimitedValue< sal_Int16, sal_Int32 >( rModel.mnScale, 10, 400 ) : 100;
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
        awt::Size aSize;
        bool bValid = false;

        if( 0 < rModel.mnPaperSize )
        {
            const msfilter::util::ApiPaperSize& rPaperSize = msfilter::util::PaperSizeConv::getApiSizeForMSPaperSizeIndex(  rModel.mnPaperSize );
            aSize = awt::Size( rPaperSize.mnWidth, rPaperSize.mnHeight );
            bValid = ( rPaperSize.mnWidth != 0 && rPaperSize.mnHeight != 0 );
        }
        if( rModel.mnPaperWidth > 0 && rModel.mnPaperHeight > 0 )
        {
            aSize = awt::Size( rModel.mnPaperWidth, rModel.mnPaperHeight );
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
    aPropMap.setProperty( PROP_IsLandscape, bLandscape);
    aPropMap.setProperty( PROP_FirstPageNumber, getLimitedValue< sal_Int16, sal_Int32 >( rModel.mbUseFirstPage ? rModel.mnFirstPage : 0, 0, 9999 ));
    aPropMap.setProperty( PROP_PrintDownFirst, (rModel.mnPageOrder == XML_downThenOver));
    aPropMap.setProperty( PROP_PrintAnnotations, (rModel.mnCellComments == XML_asDisplayed));
    aPropMap.setProperty( PROP_CenterHorizontally, rModel.mbHorCenter);
    aPropMap.setProperty( PROP_CenterVertically, rModel.mbVerCenter);
    aPropMap.setProperty( PROP_PrintGrid, (!bChartSheet && rModel.mbPrintGrid));     // no gridlines in chart sheets
    aPropMap.setProperty( PROP_PrintHeaders, (!bChartSheet && rModel.mbPrintHeadings)); // no column/row headings in chart sheets
    aPropMap.setProperty( PROP_LeftMargin, rUnitConv.scaleToMm100( rModel.mfLeftMargin, Unit::Inch ));
    aPropMap.setProperty( PROP_RightMargin, rUnitConv.scaleToMm100( rModel.mfRightMargin, Unit::Inch ));
    // #i23296# In Calc, "TopMargin" property is distance to top of header if enabled
    aPropMap.setProperty( PROP_TopMargin, rUnitConv.scaleToMm100( maHeaderData.mbHasContent ? rModel.mfHeaderMargin : rModel.mfTopMargin, Unit::Inch ));
    // #i23296# In Calc, "BottomMargin" property is distance to bottom of footer if enabled
    aPropMap.setProperty( PROP_BottomMargin, rUnitConv.scaleToMm100( maFooterData.mbHasContent ? rModel.mfFooterMargin : rModel.mfBottomMargin, Unit::Inch ));
    aPropMap.setProperty( PROP_HeaderIsOn, maHeaderData.mbHasContent);
    aPropMap.setProperty( PROP_HeaderIsShared, maHeaderData.mbShareOddEven);
    aPropMap.setProperty( PROP_HeaderIsDynamicHeight, maHeaderData.mbDynamicHeight);
    aPropMap.setProperty( PROP_HeaderHeight, maHeaderData.mnHeight);
    aPropMap.setProperty( PROP_HeaderBodyDistance, maHeaderData.mnBodyDist);
    aPropMap.setProperty( PROP_FooterIsOn, maFooterData.mbHasContent);
    aPropMap.setProperty( PROP_FooterIsShared, maFooterData.mbShareOddEven);
    aPropMap.setProperty( PROP_FooterIsDynamicHeight, maFooterData.mbDynamicHeight);
    aPropMap.setProperty( PROP_FooterHeight, maFooterData.mnHeight);
    aPropMap.setProperty( PROP_FooterBodyDistance, maFooterData.mnBodyDist);
    // background image
    if (rModel.mxGraphic.is())
    {
        aPropMap.setProperty(PROP_BackGraphic, rModel.mxGraphic);
        aPropMap.setProperty(PROP_BackGraphicLocation, css::style::GraphicLocation_TILED);
    }

    rPropSet.setProperties( aPropMap );
}

void PageSettingsConverter::convertHeaderFooterData(
        PropertySet& rPropSet, HFHelperData& orHFData,
        const OUString& rOddContent, const OUString& rEvenContent, bool bUseEvenContent,
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
        orHFData.mnBodyDist = getUnitConverter().scaleToMm100( fPageMargin - fContentMargin, Unit::Inch ) - orHFData.mnHeight;
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
        Reference<sheet::XHeaderFooterContent> xHFContent(rPropSet.getAnyProperty(nPropId), UNO_QUERY);
        if( xHFContent.is() )
        {
            double fTotalHeight = mxHFParser->parse( xHFContent, rContent );
            rPropSet.setProperty( nPropId, xHFContent );
            nHeight = getUnitConverter().scaleToMm100( fTotalHeight, Unit::Point );
        }
    }
    return nHeight;
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
