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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_PAGESETTINGS_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_PAGESETTINGS_HXX

#include <memory>
#include <string_view>

#include "worksheethelper.hxx"

namespace oox { class AttributeList; }
namespace oox { class PropertySet; }
namespace oox::core { class Relations; }

namespace oox::xls {

class HeaderFooterParser;

/** Holds page style data for a single sheet. */
struct PageSettingsModel
{
    css::uno::Reference<css::graphic::XGraphic> mxGraphic; /// Background Graphic
    OUString     maBinSettPath;          /// Relation identifier of binary printer settings.
    OUString     maOddHeader;            /// Header string for odd pages.
    OUString     maOddFooter;            /// Footer string for odd pages.
    OUString     maEvenHeader;           /// Header string for even pages.
    OUString     maEvenFooter;           /// Footer string for even pages.
    OUString     maFirstHeader;          /// Header string for first page of the sheet.
    OUString     maFirstFooter;          /// Footer string for first page of the sheet.
    double              mfLeftMargin;           /// Margin between left edge of page and begin of sheet area.
    double              mfRightMargin;          /// Margin between end of sheet area and right edge of page.
    double              mfTopMargin;            /// Margin between top edge of page and begin of sheet area.
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

    /** Sets the BIFF print errors mode. */
    void                setBiffPrintErrors( sal_uInt8 nPrintErrors );
};

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
    void                importHeaderFooterCharacters(
        std::u16string_view rChars, sal_Int32 nElement );
    /** Imports the picture element. */
    void                importPicture( const ::oox::core::Relations& rRelations, const AttributeList& rAttribs );

    /** Imports the PRINTOPTIONS record from the passed stream. */
    void                importPrintOptions( SequenceInputStream& rStrm );
    /** Imports the PAGEMARGINS record from the passed stream. */
    void                importPageMargins( SequenceInputStream& rStrm );
    /** Imports the PAGESETUP record from the passed stream. */
    void                importPageSetup( const ::oox::core::Relations& rRelations, SequenceInputStream& rStrm );
    /** Imports the CHARTPAGESETUP record from the passed stream. */
    void                importChartPageSetup( const ::oox::core::Relations& rRelations, SequenceInputStream& rStrm );
    /** Imports the HEADERFOOTER record from the passed stream. */
    void                importHeaderFooter( SequenceInputStream& rStrm );
    /** Imports the PICTURE record from the passed stream. */
    void                importPicture( const ::oox::core::Relations& rRelations, SequenceInputStream& rStrm );

    /** Sets whether percentual scaling or fit to width/height scaling is used. */
    void                setFitToPagesMode( bool bFitToPages );

    /** Creates a page style for the spreadsheet and sets all page properties. */
    void                finalizeImport();

private:
    /** Imports the binary picture data from the fragment with the passed identifier. */
    void                importPictureData( const ::oox::core::Relations& rRelations, const OUString& rRelId );

private:
    PageSettingsModel   maModel;
};

class PageSettingsConverter : public WorkbookHelper
{
public:
    explicit            PageSettingsConverter( const WorkbookHelper& rHelper );
    virtual             ~PageSettingsConverter() override;

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
                            const OUString& rOddContent,
                            const OUString& rEvenContent,
                            bool bUseEvenContent,
                            double fPageMargin,
                            double fContentMargin );

    sal_Int32           writeHeaderFooter(
                            PropertySet& rPropSet,
                            sal_Int32 nPropId,
                            const OUString& rContent );

private:
    typedef ::std::unique_ptr< HeaderFooterParser > HeaderFooterParserPtr;
    HeaderFooterParserPtr mxHFParser;
    HFHelperData        maHeaderData;
    HFHelperData        maFooterData;
};

} // namespace oox::xls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
