/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagesettings.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:03:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/pagesettings.hxx"
#include <algorithm>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::awt::Size;
using ::com::sun::star::sheet::XHeaderFooterContent;
using ::com::sun::star::style::XStyle;
using ::oox::core::Relations;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const double OOX_MARGIN_DEFAULT_LR                  = 0.748;    /// Left/right default margin in inches.
const double OOX_MARGIN_DEFAULT_TB                  = 0.984;    /// Top/bottom default margin in inches.
const double OOX_MARGIN_DEFAULT_HF                  = 0.512;    /// Header/footer default margin in inches.

const sal_uInt16 OOBIN_PRINTOPT_HORCENTER           = 0x0001;
const sal_uInt16 OOBIN_PRINTOPT_VERCENTER           = 0x0002;
const sal_uInt16 OOBIN_PRINTOPT_PRINTHEADING        = 0x0004;
const sal_uInt16 OOBIN_PRINTOPT_PRINTGRID           = 0x0008;

const sal_uInt16 OOBIN_HEADERFOOTER_DIFFEVEN        = 0x0001;
const sal_uInt16 OOBIN_HEADERFOOTER_DIFFFIRST       = 0x0002;
const sal_uInt16 OOBIN_HEADERFOOTER_SCALEDOC        = 0x0004;
const sal_uInt16 OOBIN_HEADERFOOTER_ALIGNMARGIN     = 0x0008;

const sal_uInt16 OOBIN_PAGESETUP_INROWS             = 0x0001;
const sal_uInt16 OOBIN_PAGESETUP_LANDSCAPE          = 0x0002;
const sal_uInt16 OOBIN_PAGESETUP_INVALID            = 0x0004;
const sal_uInt16 OOBIN_PAGESETUP_BLACKWHITE         = 0x0008;
const sal_uInt16 OOBIN_PAGESETUP_DRAFTQUALITY       = 0x0010;
const sal_uInt16 OOBIN_PAGESETUP_PRINTNOTES         = 0x0020;
const sal_uInt16 OOBIN_PAGESETUP_DEFAULTORIENT      = 0x0040;
const sal_uInt16 OOBIN_PAGESETUP_USEFIRSTPAGE       = 0x0080;
const sal_uInt16 OOBIN_PAGESETUP_NOTES_END          = 0x0100;   // different to BIFF flag

const sal_uInt16 OOBIN_CHARTPAGESETUP_LANDSCAPE     = 0x0001;
const sal_uInt16 OOBIN_CHARTPAGESETUP_INVALID       = 0x0002;
const sal_uInt16 OOBIN_CHARTPAGESETUP_BLACKWHITE    = 0x0004;
const sal_uInt16 OOBIN_CHARTPAGESETUP_DEFAULTORIENT = 0x0008;
const sal_uInt16 OOBIN_CHARTPAGESETUP_USEFIRSTPAGE  = 0x0010;
const sal_uInt16 OOBIN_CHARTPAGESETUP_DRAFTQUALITY  = 0x0020;

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

OoxPageData::OoxPageData() :
    mfLeftMargin( OOX_MARGIN_DEFAULT_LR ),
    mfRightMargin( OOX_MARGIN_DEFAULT_LR ),
    mfTopMargin( OOX_MARGIN_DEFAULT_TB ),
    mfBottomMargin( OOX_MARGIN_DEFAULT_TB ),
    mfHeaderMargin( OOX_MARGIN_DEFAULT_HF ),
    mfFooterMargin( OOX_MARGIN_DEFAULT_HF ),
    mnPaperSize( 1 ),
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

void OoxPageData::setBinPrintErrors( sal_uInt8 nPrintErrors )
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
    maOoxData.mbHorCenter     = rAttribs.getBool( XML_horizontalCentered, false );
    maOoxData.mbVerCenter     = rAttribs.getBool( XML_verticalCentered, false );
    maOoxData.mbPrintGrid     = rAttribs.getBool( XML_gridLines, false );
    maOoxData.mbPrintHeadings = rAttribs.getBool( XML_headings, false );
}

void PageSettings::importPageMargins( const AttributeList& rAttribs )
{
    maOoxData.mfLeftMargin   = rAttribs.getDouble( XML_left,   OOX_MARGIN_DEFAULT_LR );
    maOoxData.mfRightMargin  = rAttribs.getDouble( XML_right,  OOX_MARGIN_DEFAULT_LR );
    maOoxData.mfTopMargin    = rAttribs.getDouble( XML_top,    OOX_MARGIN_DEFAULT_TB );
    maOoxData.mfBottomMargin = rAttribs.getDouble( XML_bottom, OOX_MARGIN_DEFAULT_TB );
    maOoxData.mfHeaderMargin = rAttribs.getDouble( XML_header, OOX_MARGIN_DEFAULT_HF );
    maOoxData.mfFooterMargin = rAttribs.getDouble( XML_footer, OOX_MARGIN_DEFAULT_HF );
}

void PageSettings::importPageSetup( const Relations& rRelations, const AttributeList& rAttribs )
{
    maOoxData.maBinSettPath   = rRelations.getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ) ) );
    maOoxData.mnPaperSize     = rAttribs.getInteger( XML_paperSize, 1 );
    maOoxData.mnCopies        = rAttribs.getInteger( XML_copies, 1 );
    maOoxData.mnScale         = rAttribs.getInteger( XML_scale, 100 );
    maOoxData.mnFirstPage     = rAttribs.getInteger( XML_firstPageNumber, 1 );
    maOoxData.mnFitToWidth    = rAttribs.getInteger( XML_fitToWidth, 1 );
    maOoxData.mnFitToHeight   = rAttribs.getInteger( XML_fitToHeight, 1 );
    maOoxData.mnHorPrintRes   = rAttribs.getInteger( XML_horizontalDpi, 600 );
    maOoxData.mnVerPrintRes   = rAttribs.getInteger( XML_verticalDpi, 600 );
    maOoxData.mnOrientation   = rAttribs.getToken( XML_orientation, XML_default );
    maOoxData.mnPageOrder     = rAttribs.getToken( XML_pageOrder, XML_downThenOver );
    maOoxData.mnCellComments  = rAttribs.getToken( XML_cellComments, XML_none );
    maOoxData.mnPrintErrors   = rAttribs.getToken( XML_errors, XML_displayed );
    maOoxData.mbValidSettings = rAttribs.getBool( XML_usePrinterDefaults, true );
    maOoxData.mbUseFirstPage  = rAttribs.getBool( XML_useFirstPageNumber, false );
    maOoxData.mbBlackWhite    = rAttribs.getBool( XML_blackAndWhite, false );
    maOoxData.mbDraftQuality  = rAttribs.getBool( XML_draft, false );
}

void PageSettings::importChartPageSetup( const Relations& rRelations, const AttributeList& rAttribs )
{
    maOoxData.maBinSettPath   = rRelations.getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ) ) );
    maOoxData.mnPaperSize     = rAttribs.getInteger( XML_paperSize, 1 );
    maOoxData.mnCopies        = rAttribs.getInteger( XML_copies, 1 );
    maOoxData.mnFirstPage     = rAttribs.getInteger( XML_firstPageNumber, 1 );
    maOoxData.mnHorPrintRes   = rAttribs.getInteger( XML_horizontalDpi, 600 );
    maOoxData.mnVerPrintRes   = rAttribs.getInteger( XML_verticalDpi, 600 );
    maOoxData.mnOrientation   = rAttribs.getToken( XML_orientation, XML_default );
    maOoxData.mbValidSettings = rAttribs.getBool( XML_usePrinterDefaults, true );
    maOoxData.mbUseFirstPage  = rAttribs.getBool( XML_useFirstPageNumber, false );
    maOoxData.mbBlackWhite    = rAttribs.getBool( XML_blackAndWhite, false );
    maOoxData.mbDraftQuality  = rAttribs.getBool( XML_draft, false );
}

void PageSettings::importHeaderFooter( const AttributeList& rAttribs )
{
    maOoxData.mbUseEvenHF  = rAttribs.getBool( XML_differentOddEven, false );
    maOoxData.mbUseFirstHF = rAttribs.getBool( XML_differentFirst, false );
}

void PageSettings::importHeaderFooterCharacters( const OUString& rChars, sal_Int32 nElement )
{
    switch( nElement )
    {
        case XLS_TOKEN( oddHeader ):    maOoxData.maOddHeader += rChars;    break;
        case XLS_TOKEN( oddFooter ):    maOoxData.maOddFooter += rChars;    break;
        case XLS_TOKEN( evenHeader ):   maOoxData.maEvenHeader += rChars;   break;
        case XLS_TOKEN( evenFooter ):   maOoxData.maEvenFooter += rChars;   break;
        case XLS_TOKEN( firstHeader ):  maOoxData.maFirstHeader += rChars;  break;
        case XLS_TOKEN( firstFooter ):  maOoxData.maFirstFooter += rChars;  break;
    }
}

void PageSettings::importPicture( const Relations& rRelations, const AttributeList& rAttribs )
{
    maOoxData.maPicturePath = rRelations.getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ) ) );
}

void PageSettings::importPageMargins( RecordInputStream& rStrm )
{
    rStrm   >> maOoxData.mfLeftMargin   >> maOoxData.mfRightMargin
            >> maOoxData.mfTopMargin    >> maOoxData.mfBottomMargin
            >> maOoxData.mfHeaderMargin >> maOoxData.mfFooterMargin;
}

void PageSettings::importPrintOptions( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> nFlags;
    maOoxData.mbHorCenter     = getFlag( nFlags, OOBIN_PRINTOPT_HORCENTER );
    maOoxData.mbVerCenter     = getFlag( nFlags, OOBIN_PRINTOPT_VERCENTER );
    maOoxData.mbPrintGrid     = getFlag( nFlags, OOBIN_PRINTOPT_PRINTGRID );
    maOoxData.mbPrintHeadings = getFlag( nFlags, OOBIN_PRINTOPT_PRINTHEADING );
}

void PageSettings::importPageSetup( const Relations& rRelations, RecordInputStream& rStrm )
{
    OUString aRelId;
    sal_uInt16 nFlags;
    rStrm   >> maOoxData.mnPaperSize >> maOoxData.mnScale
            >> maOoxData.mnHorPrintRes >> maOoxData.mnVerPrintRes
            >> maOoxData.mnCopies >> maOoxData.mnFirstPage
            >> maOoxData.mnFitToWidth >> maOoxData.mnFitToHeight
            >> nFlags >> aRelId;
    maOoxData.setBinPrintErrors( extractValue< sal_uInt8 >( nFlags, 9, 2 ) );
    maOoxData.maBinSettPath   = rRelations.getFragmentPathFromRelId( aRelId );
    maOoxData.mnOrientation   = getFlagValue( nFlags, OOBIN_PAGESETUP_DEFAULTORIENT, XML_default, getFlagValue( nFlags, OOBIN_PAGESETUP_LANDSCAPE, XML_landscape, XML_portrait ) );
    maOoxData.mnPageOrder     = getFlagValue( nFlags, OOBIN_PAGESETUP_INROWS, XML_overThenDown, XML_downThenOver );
    maOoxData.mnCellComments  = getFlagValue( nFlags, OOBIN_PAGESETUP_PRINTNOTES, getFlagValue( nFlags, OOBIN_PAGESETUP_NOTES_END, XML_atEnd, XML_asDisplayed ), XML_none );
    maOoxData.mbValidSettings = !getFlag( nFlags, OOBIN_PAGESETUP_INVALID );
    maOoxData.mbUseFirstPage  = getFlag( nFlags, OOBIN_PAGESETUP_USEFIRSTPAGE );
    maOoxData.mbBlackWhite    = getFlag( nFlags, OOBIN_PAGESETUP_BLACKWHITE );
    maOoxData.mbDraftQuality  = getFlag( nFlags, OOBIN_PAGESETUP_DRAFTQUALITY );
}

void PageSettings::importChartPageSetup( const Relations& rRelations, RecordInputStream& rStrm )
{
    OUString aRelId;
    sal_uInt16 nFirstPage, nFlags;
    rStrm   >> maOoxData.mnPaperSize >> maOoxData.mnHorPrintRes >> maOoxData.mnVerPrintRes
            >> maOoxData.mnCopies >> nFirstPage >> nFlags >> aRelId;
    maOoxData.maBinSettPath   = rRelations.getFragmentPathFromRelId( aRelId );
    maOoxData.mnFirstPage     = nFirstPage; // 16-bit in CHARTPAGESETUP
    maOoxData.mnOrientation   = getFlagValue( nFlags, OOBIN_CHARTPAGESETUP_DEFAULTORIENT, XML_default, getFlagValue( nFlags, OOBIN_CHARTPAGESETUP_LANDSCAPE, XML_landscape, XML_portrait ) );
    maOoxData.mbValidSettings = !getFlag( nFlags, OOBIN_CHARTPAGESETUP_INVALID );
    maOoxData.mbUseFirstPage  = getFlag( nFlags, OOBIN_CHARTPAGESETUP_USEFIRSTPAGE );
    maOoxData.mbBlackWhite    = getFlag( nFlags, OOBIN_CHARTPAGESETUP_BLACKWHITE );
    maOoxData.mbDraftQuality  = getFlag( nFlags, OOBIN_CHARTPAGESETUP_DRAFTQUALITY );
}

void PageSettings::importHeaderFooter( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm   >> nFlags
            >> maOoxData.maOddHeader   >> maOoxData.maOddFooter
            >> maOoxData.maEvenHeader  >> maOoxData.maEvenFooter
            >> maOoxData.maFirstHeader >> maOoxData.maFirstFooter;
    maOoxData.mbUseEvenHF  = getFlag( nFlags, OOBIN_HEADERFOOTER_DIFFEVEN );
    maOoxData.mbUseFirstHF = getFlag( nFlags, OOBIN_HEADERFOOTER_DIFFFIRST );
}

void PageSettings::importPicture( const Relations& rRelations, RecordInputStream& rStrm )
{
    maOoxData.maPicturePath = rRelations.getFragmentPathFromRelId( rStrm.readString() );
}

void PageSettings::importLeftMargin( BiffInputStream& rStrm )
{
    rStrm >> maOoxData.mfLeftMargin;
}

void PageSettings::importRightMargin( BiffInputStream& rStrm )
{
    rStrm >> maOoxData.mfRightMargin;
}

void PageSettings::importTopMargin( BiffInputStream& rStrm )
{
    rStrm >> maOoxData.mfTopMargin;
}

void PageSettings::importBottomMargin( BiffInputStream& rStrm )
{
    rStrm >> maOoxData.mfBottomMargin;
}

void PageSettings::importPageSetup( BiffInputStream& rStrm )
{
    sal_uInt16 nPaperSize, nScale, nFirstPage, nFitToWidth, nFitToHeight, nFlags;
    rStrm >> nPaperSize >> nScale >> nFirstPage >> nFitToWidth >> nFitToHeight >> nFlags;

    maOoxData.mnPaperSize      = nPaperSize;   // equal in BIFF and OOX
    maOoxData.mnScale          = nScale;
    maOoxData.mnFirstPage      = nFirstPage;
    maOoxData.mnFitToWidth     = nFitToWidth;
    maOoxData.mnFitToHeight    = nFitToHeight;
    maOoxData.mnOrientation    = getFlagValue( nFlags, BIFF_PAGESETUP_PORTRAIT, XML_portrait, XML_landscape );
    maOoxData.mnPageOrder      = getFlagValue( nFlags, BIFF_PAGESETUP_INROWS, XML_overThenDown, XML_downThenOver );
    maOoxData.mbValidSettings  = !getFlag( nFlags, BIFF_PAGESETUP_INVALID );
    maOoxData.mbUseFirstPage   = true;
    maOoxData.mbBlackWhite     = getFlag( nFlags, BIFF_PAGESETUP_BLACKWHITE );

    if( getBiff() >= BIFF5 )
    {
        sal_uInt16 nHorPrintRes, nVerPrintRes, nCopies;
        rStrm >> nHorPrintRes >> nVerPrintRes >> maOoxData.mfHeaderMargin >> maOoxData.mfFooterMargin >> nCopies;

        maOoxData.mnCopies       = nCopies;
        maOoxData.mnOrientation  = getFlagValue( nFlags, BIFF_PAGESETUP_DEFAULTORIENT, XML_default, maOoxData.mnOrientation );
        maOoxData.mnHorPrintRes  = nHorPrintRes;
        maOoxData.mnVerPrintRes  = nVerPrintRes;
        maOoxData.mnCellComments = getFlagValue( nFlags, BIFF_PAGESETUP_PRINTNOTES, XML_asDisplayed, XML_none );
        maOoxData.mbUseFirstPage = getFlag( nFlags, BIFF_PAGESETUP_USEFIRSTPAGE );
        maOoxData.mbDraftQuality = getFlag( nFlags, BIFF_PAGESETUP_DRAFTQUALITY );

        if( getBiff() == BIFF8 )
        {
            maOoxData.setBinPrintErrors( extractValue< sal_uInt8 >( nFlags, 10, 2 ) );
            maOoxData.mnCellComments = getFlagValue( nFlags, BIFF_PAGESETUP_PRINTNOTES, getFlagValue( nFlags, BIFF_PAGESETUP_NOTES_END, XML_atEnd, XML_asDisplayed ), XML_none );
        }
    }
}

void PageSettings::importHorCenter( BiffInputStream& rStrm )
{
    maOoxData.mbHorCenter = rStrm.readuInt16() != 0;
}

void PageSettings::importVerCenter( BiffInputStream& rStrm )
{
    maOoxData.mbVerCenter = rStrm.readuInt16() != 0;
}

void PageSettings::importPrintHeaders( BiffInputStream& rStrm )
{
    maOoxData.mbPrintHeadings = rStrm.readuInt16() != 0;
}

void PageSettings::importPrintGridLines( BiffInputStream& rStrm )
{
    maOoxData.mbPrintGrid = rStrm.readuInt16() != 0;
}

void PageSettings::importHeader( BiffInputStream& rStrm )
{
    if( rStrm.getRecLeft() > 0 )
        maOoxData.maOddHeader = (getBiff() == BIFF8) ? rStrm.readUniString() : rStrm.readByteString( false, getTextEncoding() );
    else
        maOoxData.maOddHeader = OUString();
}

void PageSettings::importFooter( BiffInputStream& rStrm )
{
    if( rStrm.getRecLeft() > 0 )
        maOoxData.maOddFooter = (getBiff() == BIFF8) ? rStrm.readUniString() : rStrm.readByteString( false, getTextEncoding() );
    else
        maOoxData.maOddFooter = OUString();
}

void PageSettings::importPicture( BiffInputStream& /*rStrm*/ )
{
}

void PageSettings::setFitToPagesMode( bool bFitToPages )
{
    maOoxData.mbFitToPages = bFitToPages;
}

void PageSettings::finalizeImport()
{
    OUStringBuffer aStyleNameBuffer( CREATE_OUSTRING( "PageStyle_" ) );
    Reference< XNamed > xSheetName( getXSpreadsheet(), UNO_QUERY );
    if( xSheetName.is() )
        aStyleNameBuffer.append( xSheetName->getName() );
    else
        aStyleNameBuffer.append( static_cast< sal_Int32 >( getSheetIndex() + 1 ) );
    OUString aStyleName = aStyleNameBuffer.makeStringAndClear();

    Reference< XStyle > xStyle = createStyleObject( aStyleName, true );
    PropertySet aStyleProps( xStyle );
    getPageSettingsPropertyHelper().writePageSettingsProperties( aStyleProps, maOoxData, getSheetType() );

    PropertySet aSheetProps( getXSpreadsheet() );
    aSheetProps.setProperty( CREATE_OUSTRING( "PageStyle" ), aStyleName );
}

// ============================================================================

namespace {

/** Property names for page style settings. */
const sal_Char* const sppcPageNames[] =
{
    "IsLandscape",
    "FirstPageNumber",
    "PrintDownFirst",
    "PrintAnnotations",
    "CenterHorizontally",
    "CenterVertically",
    "PrintGrid",
    "PrintHeaders",
    "LeftMargin",
    "RightMargin",
    "TopMargin",
    "BottomMargin",
    "HeaderIsOn",
    "HeaderIsShared",
    "HeaderIsDynamicHeight",
    "HeaderHeight",
    "HeaderBodyDistance",
    "FooterIsOn",
    "FooterIsShared",
    "FooterIsDynamicHeight",
    "FooterHeight",
    "FooterBodyDistance",
    0
};

/** Property names for page background graphic. */
const sal_Char* const sppcGraphicNames[] =
{
    "BackGraphicURL",
    "BackGraphicLocation",
    0
};

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

PageSettingsPropertyHelper::HFHelperData::HFHelperData( const OUString& rLeftProp, const OUString& rRightProp ) :
    maLeftProp( rLeftProp ),
    maRightProp( rRightProp ),
    mnHeight( 0 ),
    mnBodyDist( 0 ),
    mbHasContent( false ),
    mbShareOddEven( false ),
    mbDynamicHeight( false )
{
}

// ----------------------------------------------------------------------------

PageSettingsPropertyHelper::PageSettingsPropertyHelper( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maHFParser( rHelper ),
    maPageProps( sppcPageNames ),
    maGraphicProps( sppcGraphicNames ),
    maHeaderData( CREATE_OUSTRING( "LeftPageHeaderContent" ), CREATE_OUSTRING( "RightPageHeaderContent" ) ),
    maFooterData( CREATE_OUSTRING( "LeftPageFooterContent" ), CREATE_OUSTRING( "RightPageFooterContent" ) )
{
}

void PageSettingsPropertyHelper::writePageSettingsProperties(
        PropertySet& rPropSet, const OoxPageData& rData, WorksheetType eSheetType )
{
    // special handling for chart sheets
    bool bChartSheet = eSheetType == SHEETTYPE_CHARTSHEET;

    // printout scaling
    if( bChartSheet )
    {
        // always fit chart sheet to 1 page
        rPropSet.setProperty< sal_Int16 >( CREATE_OUSTRING( "ScaleToPages" ), 1 );
    }
    else if( rData.mbFitToPages )
    {
        // fit to number of pages
        rPropSet.setProperty( CREATE_OUSTRING( "ScaleToPagesX" ), getLimitedValue< sal_Int16, sal_Int32 >( rData.mnFitToWidth, 0, 1000 ) );
        rPropSet.setProperty( CREATE_OUSTRING( "ScaleToPagesY" ), getLimitedValue< sal_Int16, sal_Int32 >( rData.mnFitToHeight, 0, 1000 ) );
    }
    else
    {
        // scale may be 0 which indicates uninitialized
        sal_Int16 nScale = (rData.mbValidSettings && (rData.mnScale > 0)) ? getLimitedValue< sal_Int16, sal_Int32 >( rData.mnScale, 10, 400 ) : 100;
        rPropSet.setProperty( CREATE_OUSTRING( "PageScale" ), nScale );
    }

    // paper orientation
    bool bLandscape = rData.mnOrientation == XML_landscape;
    // default orientation for current sheet type (chart sheets default to landscape)
    if( !rData.mbValidSettings || (rData.mnOrientation == XML_default) )
        bLandscape = bChartSheet;

    // paper size
    if( rData.mbValidSettings && (0 < rData.mnPaperSize) && (rData.mnPaperSize < static_cast< sal_Int32 >( STATIC_ARRAY_SIZE( spPaperSizeTable ) )) )
    {
        const ApiPaperSize& rPaperSize = spPaperSizeTable[ rData.mnPaperSize ];
        Size aSize( rPaperSize.mnWidth, rPaperSize.mnHeight );
        if( bLandscape )
            ::std::swap( aSize.Width, aSize.Height );
        rPropSet.setProperty( CREATE_OUSTRING( "Size" ), aSize );
    }

    // header/footer
    convertHeaderFooterData( rPropSet, maHeaderData, rData.maOddHeader, rData.maEvenHeader, rData.mbUseEvenHF, rData.mfTopMargin,    rData.mfHeaderMargin );
    convertHeaderFooterData( rPropSet, maFooterData, rData.maOddFooter, rData.maEvenFooter, rData.mbUseEvenHF, rData.mfBottomMargin, rData.mfFooterMargin );

    // write all properties to property set
    const UnitConverter& rUnitConv = getUnitConverter();
    maPageProps
        << bLandscape
        << getLimitedValue< sal_Int16, sal_Int32 >( rData.mbUseFirstPage ? rData.mnFirstPage : 0, 0, 9999 )
        << (rData.mnPageOrder == XML_downThenOver)
        << (rData.mnCellComments == XML_asDisplayed)
        << rData.mbHorCenter
        << rData.mbVerCenter
        << (!bChartSheet && rData.mbPrintGrid)      // no gridlines in chart sheets
        << (!bChartSheet && rData.mbPrintHeadings)  // no column/row headings in chart sheets
        << rUnitConv.scaleToMm100( rData.mfLeftMargin, UNIT_INCH )
        << rUnitConv.scaleToMm100( rData.mfRightMargin, UNIT_INCH )
        // #i23296# In Calc, "TopMargin" property is distance to top of header if enabled
        << rUnitConv.scaleToMm100( maHeaderData.mbHasContent ? rData.mfHeaderMargin : rData.mfTopMargin, UNIT_INCH )
        // #i23296# In Calc, "BottomMargin" property is distance to bottom of footer if enabled
        << rUnitConv.scaleToMm100( maFooterData.mbHasContent ? rData.mfFooterMargin : rData.mfBottomMargin, UNIT_INCH )
        << maHeaderData.mbHasContent
        << maHeaderData.mbShareOddEven
        << maHeaderData.mbDynamicHeight
        << maHeaderData.mnHeight
        << maHeaderData.mnBodyDist
        << maFooterData.mbHasContent
        << maFooterData.mbShareOddEven
        << maFooterData.mbDynamicHeight
        << maFooterData.mnHeight
        << maFooterData.mnBodyDist
        >> rPropSet;

    // background image
    OSL_ENSURE( (getFilterType() == FILTER_OOX) || (rData.maPicturePath.getLength() == 0),
        "PageSettingsPropertyHelper::writePageSettingsProperties - unexpected background picture" );
    if( (getFilterType() == FILTER_OOX) && (rData.maPicturePath.getLength() > 0) )
    {
        OUString aPictureUrl = getOoxFilter().copyPictureStream( rData.maPicturePath );
        if( aPictureUrl.getLength() > 0 )
            maGraphicProps << aPictureUrl << ::com::sun::star::style::GraphicLocation_TILED >> rPropSet;
    }
}

void PageSettingsPropertyHelper::convertHeaderFooterData(
        PropertySet& rPropSet, HFHelperData& rHFData,
        const OUString rOddContent, const OUString rEvenContent, bool bUseEvenContent,
        double fPageMargin, double fContentMargin )
{
    bool bHasOddContent  = rOddContent.getLength() > 0;
    bool bHasEvenContent = bUseEvenContent && (rEvenContent.getLength() > 0);

    sal_Int32 nOddHeight  = bHasOddContent  ? writeHeaderFooter( rPropSet, rHFData.maRightProp, rOddContent  ) : 0;
    sal_Int32 nEvenHeight = bHasEvenContent ? writeHeaderFooter( rPropSet, rHFData.maLeftProp,  rEvenContent ) : 0;

    rHFData.mnHeight = 750;
    rHFData.mnBodyDist = 250;
    rHFData.mbHasContent = bHasOddContent || bHasEvenContent;
    rHFData.mbShareOddEven = !bUseEvenContent;
    rHFData.mbDynamicHeight = true;

    if( rHFData.mbHasContent )
    {
        // use maximum height of odd/even header/footer
        rHFData.mnHeight = ::std::max( nOddHeight, nEvenHeight );
        /*  Calc contains distance between bottom of header and top of page
            body in "HeaderBodyDistance" property, and distance between bottom
            of page body and top of footer in "FooterBodyDistance" property */
        rHFData.mnBodyDist = getUnitConverter().scaleToMm100( fPageMargin - fContentMargin, UNIT_INCH ) - rHFData.mnHeight;
        /*  #i23296# Distance less than 0 means, header or footer overlays page
            body. As this is not possible in Calc, set fixed header or footer
            height (crop header/footer) to get correct top position of page body. */
        rHFData.mbDynamicHeight = rHFData.mnBodyDist >= 0;
        /*  "HeaderHeight" property is in fact distance from top of header to
            top of page body (including "HeaderBodyDistance").
            "FooterHeight" property is in fact distance from bottom of page
            body to bottom of footer (including "FooterBodyDistance"). */
        rHFData.mnHeight += rHFData.mnBodyDist;
        // negative body distance not allowed
        rHFData.mnBodyDist = ::std::max< sal_Int32 >( rHFData.mnBodyDist, 0 );
    }
}

sal_Int32 PageSettingsPropertyHelper::writeHeaderFooter(
        PropertySet& rPropSet, const OUString& rPropName, const OUString& rContent )
{
    OSL_ENSURE( rContent.getLength() > 0, "PageSettingsPropertyHelper::writeHeaderFooter - empty h/f string found" );
    sal_Int32 nHeight = 0;
    if( rContent.getLength() > 0 )
    {
        Reference< XHeaderFooterContent > xHFContent;
        if( rPropSet.getProperty( xHFContent, rPropName ) && xHFContent.is() )
        {
            maHFParser.parse( xHFContent, rContent );
            rPropSet.setProperty( rPropName, xHFContent );
            nHeight = getUnitConverter().scaleToMm100( maHFParser.getTotalHeight(), UNIT_POINT );
        }
    }
    return nHeight;
}

// ============================================================================

} // namespace xls
} // namespace oox

