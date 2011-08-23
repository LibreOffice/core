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

#ifndef OOX_XLS_PAGESETTINGS_HXX
#define OOX_XLS_PAGESETTINGS_HXX

#include "oox/xls/worksheethelper.hxx"

namespace oox { class PropertySet; }
namespace oox { namespace core { class Relations; } }

namespace oox {
namespace xls {

class HeaderFooterParser;

// ============================================================================

/** Holds page style data for a single sheet. */
struct PageSettingsModel
{
    ::rtl::OUString     maGraphicUrl;           /// URL of the graphic object.
    ::rtl::OUString     maBinSettPath;          /// Relation identifier of binary printer settings.
    ::rtl::OUString     maOddHeader;            /// Header string for odd pages.
    ::rtl::OUString     maOddFooter;            /// Footer string for odd pages.
    ::rtl::OUString     maEvenHeader;           /// Header string for even pages.
    ::rtl::OUString     maEvenFooter;           /// Footer string for even pages.
    ::rtl::OUString     maFirstHeader;          /// Header string for first page of the sheet.
    ::rtl::OUString     maFirstFooter;          /// Footer string for first page of the sheet.
    double              mfLeftMargin;           /// Margin between left edge of page and begin of sheet area.
    double              mfRightMargin;          /// Margin between end of sheet area and right edge of page.
    double              mfTopMargin;            /// Margin between top egde of page and begin of sheet area.
    double              mfBottomMargin;         /// Margin between end of sheet area and bottom edge of page.
    double              mfHeaderMargin;         /// Margin between top edge of page and begin of header.
    double              mfFooterMargin;         /// Margin between end of footer and bottom edge of page.
    sal_Int32           mnPaperSize;            /// Paper size (enumeration).
    sal_Int32           mnPaperWidth;           /// Paper width in twips
    sal_Int32           mnPaperHeight;          /// Paper height in twips
    sal_Int32           mnCopies;               /// Number of copies to print.
    sal_Int32           mnScale;                /// Page scale (zoom in percent).
    sal_Int32           mnFirstPage;            /// First page number.
    sal_Int32           mnFitToWidth;           /// Fit to number of pages in horizontal direction.
    sal_Int32           mnFitToHeight;          /// Fit to number of pages in vertical direction.
    sal_Int32           mnHorPrintRes;          /// Horizontal printing resolution in DPI.
    sal_Int32           mnVerPrintRes;          /// Vertical printing resolution in DPI.
    sal_Int32           mnOrientation;          /// Landscape or portrait.
    sal_Int32           mnPageOrder;            /// Page order through sheet area (to left or down).
    sal_Int32           mnCellComments;         /// Cell comments printing mode.
    sal_Int32           mnPrintErrors;          /// Cell error printing mode.
    bool                mbUseEvenHF;            /// True = use maEvenHeader/maEvenFooter.
    bool                mbUseFirstHF;           /// True = use maFirstHeader/maFirstFooter.
    bool                mbValidSettings;        /// True = use imported settings.
    bool                mbUseFirstPage;         /// True = start page numbering with mnFirstPage.
    bool                mbBlackWhite;           /// True = print black and white.
    bool                mbDraftQuality;         /// True = print in draft quality.
    bool                mbFitToPages;           /// True = Fit to width/height; false = scale in percent.
    bool                mbHorCenter;            /// True = horizontally centered.
    bool                mbVerCenter;            /// True = vertically centered.
    bool                mbPrintGrid;            /// True = print grid lines.
    bool                mbPrintHeadings;        /// True = print column/row headings.

    explicit            PageSettingsModel();

    /** Sets the OOBIN or BIFF print errors mode. */
    void                setBinPrintErrors( sal_uInt8 nPrintErrors );
};

// ============================================================================

class PageSettings : public WorksheetHelper
{
public:
    explicit            PageSettings( const WorksheetHelper& rHelper );

    /** Imports printing options from a printOptions element. */
    void                importPrintOptions( const AttributeList& rAttribs );
    /** Imports pageMarings element containing page margins. */
    void                importPageMargins( const AttributeList& rAttribs );
    /** Imports pageSetup element for worksheets. */
    void                importPageSetup( const ::oox::core::Relations& rRelations, const AttributeList& rAttribs );
    /** Imports pageSetup element for chart sheets. */
    void                importChartPageSetup( const ::oox::core::Relations& rRelations, const AttributeList& rAttribs );
    /** Imports header and footer settings from a headerFooter element. */
    void                importHeaderFooter( const AttributeList& rAttribs );
    /** Imports header/footer characters from a headerFooter element. */
    void                importHeaderFooterCharacters( const ::rtl::OUString& rChars, sal_Int32 nElement );
    /** Imports the picture element. */
    void                importPicture( const ::oox::core::Relations& rRelations, const AttributeList& rAttribs );

    /** Imports the PRINTOPTIONS record from the passed stream. */
    void                importPrintOptions( RecordInputStream& rStrm );
    /** Imports the PAGEMARGINS record from the passed stream. */
    void                importPageMargins( RecordInputStream& rStrm );
    /** Imports the PAGESETUP record from the passed stream. */
    void                importPageSetup( const ::oox::core::Relations& rRelations, RecordInputStream& rStrm );
    /** Imports the CHARTPAGESETUP record from the passed stream. */
    void                importChartPageSetup( const ::oox::core::Relations& rRelations, RecordInputStream& rStrm );
    /** Imports the HEADERFOOTER record from the passed stream. */
    void                importHeaderFooter( RecordInputStream& rStrm );
    /** Imports the PICTURE record from the passed stream. */
    void                importPicture( const ::oox::core::Relations& rRelations, RecordInputStream& rStrm );

    /** Imports the LEFTMARGIN record from the passed BIFF stream. */
    void                importLeftMargin( BiffInputStream& rStrm );
    /** Imports the RIGHTMARGIN record from the passed BIFF stream. */
    void                importRightMargin( BiffInputStream& rStrm );
    /** Imports the TOPMARGIN record from the passed BIFF stream. */
    void                importTopMargin( BiffInputStream& rStrm );
    /** Imports the BOTTOMMARGIN record from the passed BIFF stream. */
    void                importBottomMargin( BiffInputStream& rStrm );
    /** Imports the SETUP record from the passed BIFF stream. */
    void                importPageSetup( BiffInputStream& rStrm );
    /** Imports the HCENTER record from the passed BIFF stream. */
    void                importHorCenter( BiffInputStream& rStrm );
    /** Imports the VCENTER record from the passed BIFF stream. */
    void                importVerCenter( BiffInputStream& rStrm );
    /** Imports the PRINTHEADERS record from the passed BIFF stream. */
    void                importPrintHeaders( BiffInputStream& rStrm );
    /** Imports the PRINTGRIDLINES record from the passed BIFF stream. */
    void                importPrintGridLines( BiffInputStream& rStrm );
    /** Imports the HEADER record from the passed BIFF stream. */
    void                importHeader( BiffInputStream& rStrm );
    /** Imports the FOOTER record from the passed BIFF stream. */
    void                importFooter( BiffInputStream& rStrm );
    /** Imports the PICTURE record from the passed BIFF stream. */
    void                importPicture( BiffInputStream& rStrm );

    /** Sets whether percentual scaling or fit to width/height scaling is used. */
    void                setFitToPagesMode( bool bFitToPages );

    /** Creates a page style for the spreadsheet and sets all page properties. */
    void                finalizeImport();

private:
    /** Imports the binary picture data from the fragment with the passed identifier. */
    void                importPictureData( const ::oox::core::Relations& rRelations, const ::rtl::OUString& rRelId );

private:
    PageSettingsModel   maModel;
};

// ============================================================================

class PageSettingsConverter : public WorkbookHelper
{
public:
    explicit            PageSettingsConverter( const WorkbookHelper& rHelper );
    virtual             ~PageSettingsConverter();

    /** Writes all properties to the passed property set of a page style object. */
    void                writePageSettingsProperties(
                            PropertySet& rPropSet,
                            const PageSettingsModel& rModel,
                            WorksheetType eSheetType );

private:
    struct HFHelperData
    {
        sal_Int32           mnLeftPropId;
        sal_Int32           mnRightPropId;
        sal_Int32           mnHeight;
        sal_Int32           mnBodyDist;
        bool                mbHasContent;
        bool                mbShareOddEven;
        bool                mbDynamicHeight;

        explicit            HFHelperData( sal_Int32 nLeftPropId, sal_Int32 nRightPropId );
    };

private:
    void                convertHeaderFooterData(
                            PropertySet& rPropSet,
                            HFHelperData& orHFData,
                            const ::rtl::OUString rOddContent,
                            const ::rtl::OUString rEvenContent,
                            bool bUseEvenContent,
                            double fPageMargin,
                            double fContentMargin );

    sal_Int32           writeHeaderFooter(
                            PropertySet& rPropSet,
                            sal_Int32 nPropId,
                            const ::rtl::OUString& rContent );

private:
    typedef ::std::auto_ptr< HeaderFooterParser > HeaderFooterParserPtr;
    HeaderFooterParserPtr mxHFParser;
    HFHelperData        maHeaderData;
    HFHelperData        maFooterData;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
