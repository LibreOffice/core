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

#include "oox/xls/workbookhelper.hxx"
#include <osl/thread.h>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XExternalDocLinks.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include "properties.hxx"
#include "oox/helper/progressbar.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/ole/vbaproject.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/biffcodec.hxx"
#include "oox/xls/defnamesbuffer.hxx"
#include "oox/xls/excelchartconverter.hxx"
#include "oox/xls/excelfilter.hxx"
#include "oox/xls/externallinkbuffer.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/pivotcachebuffer.hxx"
#include "oox/xls/pivottablebuffer.hxx"
#include "oox/xls/scenariobuffer.hxx"
#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/tablebuffer.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/unitconverter.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/webquerybuffer.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetbuffer.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::document::XActionLockable;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::table::XCell;
using ::com::sun::star::table::XCellRange;
using ::com::sun::star::sheet::XSpreadsheetDocument;
using ::com::sun::star::sheet::XSpreadsheet;
using ::com::sun::star::sheet::XNamedRange;
using ::com::sun::star::sheet::XNamedRanges;
using ::com::sun::star::sheet::XDatabaseRanges;
using ::com::sun::star::sheet::XExternalDocLinks;
using ::com::sun::star::style::XStyle;
using ::com::sun::star::style::XStyleFamiliesSupplier;
using ::oox::core::BinaryFilterBase;
using ::oox::core::FilterBase;
using ::oox::core::FragmentHandler;
using ::oox::core::XmlFilterBase;
using ::oox::drawingml::Theme;

namespace oox {
namespace xls {

// ============================================================================

bool IgnoreCaseCompare::operator()( const OUString& rName1, const OUString& rName2 ) const
{
    // there is no wrapper in rtl::OUString, TODO: compare with collator
    return ::rtl_ustr_compareIgnoreAsciiCase_WithLength(
        rName1.getStr(), rName1.getLength(), rName2.getStr(), rName2.getLength() ) < 0;
}

// ============================================================================

class WorkbookData
{
public:
    explicit            WorkbookData( ExcelFilter& rFilter );
    explicit            WorkbookData( ExcelBiffFilter& rFilter, BiffType eBiff );
                        ~WorkbookData();

    /** Returns true, if this helper refers to a valid document. */
    inline bool         isValid() const { return mxDoc.is(); }

    // filter -----------------------------------------------------------------

    /** Returns the base filter object (base class of all filters). */
    inline FilterBase&  getBaseFilter() const { return mrBaseFilter; }
    /** Returns the filter progress bar. */
    inline SegmentProgressBar& getProgressBar() const { return *mxProgressBar; }
    /** Returns the file type of the current filter. */
    inline FilterType   getFilterType() const { return meFilterType; }
    /** Returns true, if the file is a multi-sheet document, or false if single-sheet. */
    inline bool         isWorkbookFile() const { return mbWorkbook; }
    /** Returns the index of the current sheet in the Calc document. */
    inline sal_Int16    getCurrentSheetIndex() const { return mnCurrSheet; }
    /** Sets the index of the current sheet in the Calc document. */
    inline void         setCurrentSheetIndex( sal_Int16 nSheet ) { mnCurrSheet = nSheet; }
    /** Returns the VBA project storage. */
    inline StorageRef   getVbaProjectStorage() const { return mxVbaPrjStrg; }
    /** Sets the VBA project storage. */
    inline void         setVbaProjectStorage( const StorageRef& rxVbaPrjStrg ) { mxVbaPrjStrg = rxVbaPrjStrg; }

    // document model ---------------------------------------------------------

    /** Returns a reference to the source/target spreadsheet document model. */
    inline Reference< XSpreadsheetDocument > getDocument() const { return mxDoc; }
    /** Returns the reference device of the document. */
    Reference< XDevice > getReferenceDevice() const;
    /** Returns the container for defined names from the Calc document. */
    Reference< XNamedRanges > getNamedRanges() const;
    /** Returns the container for database ranges from the Calc document. */
    Reference< XDatabaseRanges > getDatabaseRanges() const;
    /** Returns the container for external documents from the Calc document. */
    Reference< XExternalDocLinks > getExternalDocLinks() const;
    /** Returns the container for DDE links from the Calc document. */
    Reference< XNameAccess > getDdeLinks() const;
    /** Returns the cell or page styles container from the Calc document. */
    Reference< XNameContainer > getStyleFamily( bool bPageStyles ) const;
    /** Returns the specified cell or page style from the Calc document. */
    Reference< XStyle > getStyleObject( const OUString& rStyleName, bool bPageStyle ) const;
    /** Creates and returns a defined name on-the-fly in the Calc document. */
    Reference< XNamedRange > createNamedRangeObject( OUString& orName, sal_Int32 nNameFlags ) const;
    /** Creates and returns a com.sun.star.style.Style object for cells or pages. */
    Reference< XStyle > createStyleObject( OUString& orStyleName, bool bPageStyle ) const;

    // buffers ----------------------------------------------------------------

    /** Returns the global workbook settings object. */
    inline WorkbookSettings& getWorkbookSettings() const { return *mxWorkbookSettings; }
    /** Returns the workbook and sheet view settings object. */
    inline ViewSettings& getViewSettings() const { return *mxViewSettings; }
    /** Returns the worksheet buffer containing sheet names and properties. */
    inline WorksheetBuffer& getWorksheets() const { return *mxWorksheets; }
    /** Returns the office theme object read from the theme substorage. */
    inline ThemeBuffer& getTheme() const { return *mxTheme; }
    /** Returns all cell formatting objects read from the styles substream. */
    inline StylesBuffer& getStyles() const { return *mxStyles; }
    /** Returns the shared strings read from the shared strings substream. */
    inline SharedStringsBuffer& getSharedStrings() const { return *mxSharedStrings; }
    /** Returns the external links read from the external links substream. */
    inline ExternalLinkBuffer& getExternalLinks() const { return *mxExtLinks; }
    /** Returns the defined names read from the workbook globals. */
    inline DefinedNamesBuffer& getDefinedNames() const { return *mxDefNames; }
    /** Returns the tables collection (equivalent to Calc's database ranges). */
    inline TableBuffer& getTables() const { return *mxTables; }
    /** Returns the scenarios collection. */
    inline ScenarioBuffer& getScenarios() const { return *mxScenarios; }
    /** Returns the web queries. */
    inline WebQueryBuffer& getWebQueries() const { return *mxWebQueries; }
    /** Returns the collection of pivot caches. */
    inline PivotCacheBuffer& getPivotCaches() const { return *mxPivotCaches; }
    /** Returns the collection of pivot tables. */
    inline PivotTableBuffer& getPivotTables() { return *mxPivotTables; }

    // converters -------------------------------------------------------------

    /** Returns the import formula parser. */
    inline FormulaParser& getFormulaParser() const { return *mxFmlaParser; }
    /** Returns the measurement unit converter. */
    inline UnitConverter& getUnitConverter() const { return *mxUnitConverter; }
    /** Returns the converter for string to cell address/range conversion. */
    inline AddressConverter& getAddressConverter() const { return *mxAddrConverter; }
    /** Returns the chart object converter. */
    inline ExcelChartConverter& getChartConverter() const { return *mxChartConverter; }
    /** Returns the page/print settings converter. */
    inline PageSettingsConverter& getPageSettingsConverter() const { return *mxPageSettConverter; }

    // OOX specific -----------------------------------------------------------

    /** Returns the base OOX filter object. */
    inline XmlFilterBase& getOoxFilter() const { return *mpOoxFilter; }

    // BIFF specific ----------------------------------------------------------

    /** Returns the base BIFF filter object. */
    inline BinaryFilterBase& getBiffFilter() const { return *mpBiffFilter; }
    /** Returns the BIFF type in binary filter. */
    inline BiffType     getBiff() const { return meBiff; }
    /** Returns the text encoding used to import/export byte strings. */
    inline rtl_TextEncoding getTextEncoding() const { return meTextEnc; }
    /** Sets the text encoding to import/export byte strings. */
    void                setTextEncoding( rtl_TextEncoding eTextEnc );
    /** Sets code page read from a CODEPAGE record for byte string import. */
    void                setCodePage( sal_uInt16 nCodePage );
    /** Sets text encoding from the default application font, if CODEPAGE record is missing. */
    void                setAppFontEncoding( rtl_TextEncoding eAppFontEnc );
    /** Enables workbook file mode, used for BIFF4 workspace files. */
    void                setIsWorkbookFile();
    /** Recreates global buffers that are used per sheet in specific BIFF versions. */
    void                createBuffersPerSheet( sal_Int16 nSheet );
    /** Returns the codec helper that stores the encoder/decoder object. */
    inline BiffCodecHelper& getCodecHelper() { return *mxCodecHelper; }

private:
    /** Initializes some basic members and sets needed document properties. */
    void                initialize( bool bWorkbookFile );
    /** Finalizes the filter process (sets some needed document properties). */
    void                finalize();

private:
    typedef ::std::auto_ptr< SegmentProgressBar >       ProgressBarPtr;
    typedef ::std::auto_ptr< WorkbookSettings >         WorkbookSettPtr;
    typedef ::std::auto_ptr< ViewSettings >             ViewSettingsPtr;
    typedef ::std::auto_ptr< WorksheetBuffer >          WorksheetBfrPtr;
    typedef ::boost::shared_ptr< ThemeBuffer >          ThemeBfrRef;
    typedef ::std::auto_ptr< StylesBuffer >             StylesBfrPtr;
    typedef ::std::auto_ptr< SharedStringsBuffer >      SharedStrBfrPtr;
    typedef ::std::auto_ptr< ExternalLinkBuffer >       ExtLinkBfrPtr;
    typedef ::std::auto_ptr< DefinedNamesBuffer >       DefNamesBfrPtr;
    typedef ::std::auto_ptr< TableBuffer >              TableBfrPtr;
    typedef ::std::auto_ptr< ScenarioBuffer >           ScenarioBfrPtr;
    typedef ::std::auto_ptr< WebQueryBuffer >           WebQueryBfrPtr;
    typedef ::std::auto_ptr< PivotCacheBuffer >         PivotCacheBfrPtr;
    typedef ::std::auto_ptr< PivotTableBuffer >         PivotTableBfrPtr;
    typedef ::std::auto_ptr< FormulaParser >            FormulaParserPtr;
    typedef ::std::auto_ptr< UnitConverter >            UnitConvPtr;
    typedef ::std::auto_ptr< AddressConverter >         AddressConvPtr;
    typedef ::std::auto_ptr< ExcelChartConverter >      ExcelChartConvPtr;
    typedef ::std::auto_ptr< PageSettingsConverter >    PageSettConvPtr;
    typedef ::std::auto_ptr< BiffCodecHelper >          BiffCodecHelperPtr;

    OUString            maCellStyles;           /// Style family name for cell styles.
    OUString            maPageStyles;           /// Style family name for page styles.
    OUString            maCellStyleServ;        /// Service name for a cell style.
    OUString            maPageStyleServ;        /// Service name for a page style.
    Reference< XSpreadsheetDocument > mxDoc;    /// Document model.
    FilterBase&         mrBaseFilter;           /// Base filter object.
    ExcelFilterBase&    mrExcelBase;            /// Base object for registration of this structure.
    FilterType          meFilterType;           /// File type of the filter.
    ProgressBarPtr      mxProgressBar;          /// The progress bar.
    StorageRef          mxVbaPrjStrg;           /// Storage containing the VBA project.
    sal_Int16           mnCurrSheet;            /// Current sheet index in Calc dcument.
    bool                mbWorkbook;             /// True = multi-sheet file.

    // buffers
    WorkbookSettPtr     mxWorkbookSettings;     /// Global workbook settings.
    ViewSettingsPtr     mxViewSettings;         /// Workbook and sheet view settings.
    WorksheetBfrPtr     mxWorksheets;           /// Sheet info buffer.
    ThemeBfrRef         mxTheme;                /// Formatting theme from theme substream.
    StylesBfrPtr        mxStyles;               /// All cell style objects from styles substream.
    SharedStrBfrPtr     mxSharedStrings;        /// All strings from shared strings substream.
    ExtLinkBfrPtr       mxExtLinks;             /// All external links.
    DefNamesBfrPtr      mxDefNames;             /// All defined names.
    TableBfrPtr         mxTables;               /// All tables (database ranges).
    ScenarioBfrPtr      mxScenarios;            /// All scenarios.
    WebQueryBfrPtr      mxWebQueries;           /// Web queries buffer.
    PivotCacheBfrPtr    mxPivotCaches;          /// All pivot caches in the document.
    PivotTableBfrPtr    mxPivotTables;          /// All pivot tables in the document.

    // converters
    FormulaParserPtr    mxFmlaParser;           /// Import formula parser.
    UnitConvPtr         mxUnitConverter;        /// General unit converter.
    AddressConvPtr      mxAddrConverter;        /// Cell address and cell range address converter.
    ExcelChartConvPtr   mxChartConverter;       /// Chart object converter.
    PageSettConvPtr     mxPageSettConverter;    /// Page/print settings converter.

    // OOX specific
    XmlFilterBase*      mpOoxFilter;            /// Base OOX filter object.

    // BIFF specific
    BinaryFilterBase*   mpBiffFilter;           /// Base BIFF filter object.
    BiffCodecHelperPtr  mxCodecHelper;          /// Encoder/decoder helper.
    BiffType            meBiff;                 /// BIFF version for BIFF import/export.
    rtl_TextEncoding    meTextEnc;              /// BIFF byte string text encoding.
    bool                mbHasCodePage;          /// True = CODEPAGE record exists in imported stream.
};

// ----------------------------------------------------------------------------

WorkbookData::WorkbookData( ExcelFilter& rFilter ) :
    mrBaseFilter( rFilter ),
    mrExcelBase( rFilter ),
    meFilterType( FILTER_OOX ),
    mpOoxFilter( &rFilter ),
    mpBiffFilter( 0 ),
    meBiff( BIFF_UNKNOWN )
{
    // register at the filter, needed for virtual callbacks (even during construction)
    mrExcelBase.registerWorkbookData( *this );
    initialize( true );
}

WorkbookData::WorkbookData( ExcelBiffFilter& rFilter, BiffType eBiff ) :
    mrBaseFilter( rFilter ),
    mrExcelBase( rFilter ),
    meFilterType( FILTER_BIFF ),
    mpOoxFilter( 0 ),
    mpBiffFilter( &rFilter ),
    meBiff( eBiff )
{
    // register at the filter, needed for virtual callbacks (even during construction)
    mrExcelBase.registerWorkbookData( *this );
    initialize( eBiff >= BIFF5 );
}

WorkbookData::~WorkbookData()
{
    finalize();
    mrExcelBase.unregisterWorkbookData();
}

// document model -------------------------------------------------------------

Reference< XDevice > WorkbookData::getReferenceDevice() const
{
    PropertySet aPropSet( mxDoc );
    Reference< XDevice > xDevice;
    aPropSet.getProperty( xDevice, PROP_ReferenceDevice );
    return xDevice;
}

Reference< XNamedRanges > WorkbookData::getNamedRanges() const
{
    PropertySet aPropSet( mxDoc );
    Reference< XNamedRanges > xNamedRanges;
    aPropSet.getProperty( xNamedRanges, PROP_NamedRanges );
    return xNamedRanges;
}

Reference< XDatabaseRanges > WorkbookData::getDatabaseRanges() const
{
    PropertySet aPropSet( mxDoc );
    Reference< XDatabaseRanges > xDatabaseRanges;
    aPropSet.getProperty( xDatabaseRanges, PROP_DatabaseRanges );
    return xDatabaseRanges;
}

Reference< XExternalDocLinks > WorkbookData::getExternalDocLinks() const
{
    PropertySet aPropSet( mxDoc );
    Reference< XExternalDocLinks > xDocLinks;
    aPropSet.getProperty( xDocLinks, PROP_ExternalDocLinks );
    return xDocLinks;
}

Reference< XNameAccess > WorkbookData::getDdeLinks() const
{
    PropertySet aPropSet( mxDoc );
    Reference< XNameAccess > xDdeLinks;
    aPropSet.getProperty( xDdeLinks, PROP_DDELinks );
    return xDdeLinks;
}

Reference< XNameContainer > WorkbookData::getStyleFamily( bool bPageStyles ) const
{
    Reference< XNameContainer > xStylesNC;
    try
    {
        Reference< XStyleFamiliesSupplier > xFamiliesSup( mxDoc, UNO_QUERY_THROW );
        Reference< XNameAccess > xFamiliesNA( xFamiliesSup->getStyleFamilies(), UNO_QUERY_THROW );
        xStylesNC.set( xFamiliesNA->getByName( bPageStyles ? maPageStyles : maCellStyles ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xStylesNC.is(), "WorkbookData::getStyleFamily - cannot access style family" );
    return xStylesNC;
}

Reference< XStyle > WorkbookData::getStyleObject( const OUString& rStyleName, bool bPageStyle ) const
{
    Reference< XStyle > xStyle;
    try
    {
        Reference< XNameContainer > xStylesNC( getStyleFamily( bPageStyle ), UNO_SET_THROW );
        xStyle.set( xStylesNC->getByName( rStyleName ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xStyle.is(), "WorkbookData::getStyleObject - cannot access style object" );
    return xStyle;
}

Reference< XNamedRange > WorkbookData::createNamedRangeObject( OUString& orName, sal_Int32 nNameFlags ) const
{
    // find an unused name
    Reference< XNamedRanges > xNamedRanges = getNamedRanges();
    Reference< XNameAccess > xNameAccess( xNamedRanges, UNO_QUERY );
    if( xNameAccess.is() )
        orName = ContainerHelper::getUnusedName( xNameAccess, orName, '_' );

    // create the name and insert it into the Calc document
    Reference< XNamedRange > xNamedRange;
    if( xNamedRanges.is() && (orName.getLength() > 0) ) try
    {
        xNamedRanges->addNewByName( orName, OUString(), CellAddress( 0, 0, 0 ), nNameFlags );
        xNamedRange.set( xNamedRanges->getByName( orName ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xNamedRange.is(), "WorkbookData::createNamedRangeObject - cannot create defined name" );
    return xNamedRange;
}

Reference< XStyle > WorkbookData::createStyleObject( OUString& orStyleName, bool bPageStyle ) const
{
    Reference< XStyle > xStyle;
    try
    {
        Reference< XNameContainer > xStylesNC( getStyleFamily( bPageStyle ), UNO_SET_THROW );
        xStyle.set( mrBaseFilter.getModelFactory()->createInstance( bPageStyle ? maPageStyleServ : maCellStyleServ ), UNO_QUERY_THROW );
        orStyleName = ContainerHelper::insertByUnusedName( xStylesNC, orStyleName, ' ', Any( xStyle ), false );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xStyle.is(), "WorkbookData::createStyleObject - cannot create style" );
    return xStyle;
}

// BIFF specific --------------------------------------------------------------

void WorkbookData::setTextEncoding( rtl_TextEncoding eTextEnc )
{
    if( eTextEnc != RTL_TEXTENCODING_DONTKNOW )
        meTextEnc = eTextEnc;
}

void WorkbookData::setCodePage( sal_uInt16 nCodePage )
{
    setTextEncoding( BiffHelper::calcTextEncodingFromCodePage( nCodePage ) );
    mbHasCodePage = true;
}

void WorkbookData::setAppFontEncoding( rtl_TextEncoding eAppFontEnc )
{
    if( !mbHasCodePage )
        setTextEncoding( eAppFontEnc );
}

void WorkbookData::setIsWorkbookFile()
{
    OSL_ENSURE( meBiff == BIFF4, "WorkbookData::setIsWorkbookFile - invalid call" );
    mbWorkbook = true;
}

void WorkbookData::createBuffersPerSheet( sal_Int16 nSheet )
{
    // set mnCurrSheet to enable usage of WorkbookHelper::getCurrentSheetIndex()
    mnCurrSheet = nSheet;
    switch( meBiff )
    {
        case BIFF2:
        case BIFF3:
            OSL_ENSURE( mnCurrSheet == 0, "WorkbookData::createBuffersPerSheet - unexpected sheet index" );
            mxDefNames->setLocalCalcSheet( mnCurrSheet );
        break;

        case BIFF4:
            OSL_ENSURE( mbWorkbook || (mnCurrSheet == 0), "WorkbookData::createBuffersPerSheet - unexpected sheet index" );
            // #i11183# sheets in BIFF4W files have own styles and names
            if( mbWorkbook && (mnCurrSheet > 0) )
            {
                mxStyles.reset( new StylesBuffer( *this ) );
                mxDefNames.reset( new DefinedNamesBuffer( *this ) );
                mxExtLinks.reset( new ExternalLinkBuffer( *this ) );
            }
            mxDefNames->setLocalCalcSheet( mnCurrSheet );
        break;

        case BIFF5:
            // BIFF5 stores external references per sheet
            mxExtLinks.reset( new ExternalLinkBuffer( *this ) );
        break;

        case BIFF8:
        break;

        case BIFF_UNKNOWN:
        break;
    }
    mnCurrSheet = -1;
}

// private --------------------------------------------------------------------

void WorkbookData::initialize( bool bWorkbookFile )
{
    maCellStyles = CREATE_OUSTRING( "CellStyles" );
    maPageStyles = CREATE_OUSTRING( "PageStyles" );
    maCellStyleServ = CREATE_OUSTRING( "com.sun.star.style.CellStyle" );
    maPageStyleServ = CREATE_OUSTRING( "com.sun.star.style.PageStyle" );
    mnCurrSheet = -1;
    mbWorkbook = bWorkbookFile;
    meTextEnc = osl_getThreadTextEncoding();
    mbHasCodePage = false;

    // the spreadsheet document
    mxDoc.set( mrBaseFilter.getModel(), UNO_QUERY );
    OSL_ENSURE( mxDoc.is(), "WorkbookData::initialize - no spreadsheet document" );

    mxWorkbookSettings.reset( new WorkbookSettings( *this ) );
    mxViewSettings.reset( new ViewSettings( *this ) );
    mxWorksheets.reset( new WorksheetBuffer( *this ) );
    mxTheme.reset( new ThemeBuffer( *this ) );
    mxStyles.reset( new StylesBuffer( *this ) );
    mxSharedStrings.reset( new SharedStringsBuffer( *this ) );
    mxExtLinks.reset( new ExternalLinkBuffer( *this ) );
    mxDefNames.reset( new DefinedNamesBuffer( *this ) );
    mxTables.reset( new TableBuffer( *this ) );
    mxScenarios.reset( new ScenarioBuffer( *this ) );
    mxWebQueries.reset( new WebQueryBuffer( *this ) );
    mxPivotCaches.reset( new PivotCacheBuffer( *this ) );
    mxPivotTables.reset( new PivotTableBuffer( *this ) );

    mxUnitConverter.reset( new UnitConverter( *this ) );
    mxAddrConverter.reset( new AddressConverter( *this ) );
    mxChartConverter.reset( new ExcelChartConverter( *this ) );
    mxPageSettConverter.reset( new PageSettingsConverter( *this ) );

    // set some document properties needed during import
    if( mrBaseFilter.isImportFilter() )
    {
        PropertySet aPropSet( mxDoc );
        // enable editing read-only documents (e.g. from read-only files)
        aPropSet.setProperty( PROP_IsChangeReadOnlyEnabled, true );
        // #i76026# disable Undo while loading the document
        aPropSet.setProperty( PROP_IsUndoEnabled, false );
        // #i79826# disable calculating automatic row height while loading the document
        aPropSet.setProperty( PROP_IsAdjustHeightEnabled, false );
        // disable automatic update of linked sheets and DDE links
        aPropSet.setProperty( PROP_IsExecuteLinkEnabled, false );
        // #i79890# disable automatic update of defined names
        Reference< XActionLockable > xLockable( getNamedRanges(), UNO_QUERY );
        if( xLockable.is() )
            xLockable->addActionLock();

        //! TODO: localize progress bar text
        mxProgressBar.reset( new SegmentProgressBar( mrBaseFilter.getStatusIndicator(), CREATE_OUSTRING( "Loading..." ) ) );
        mxFmlaParser.reset( new FormulaParser( *this ) );
    }
    else if( mrBaseFilter.isExportFilter() )
    {
        //! TODO: localize progress bar text
        mxProgressBar.reset( new SegmentProgressBar( mrBaseFilter.getStatusIndicator(), CREATE_OUSTRING( "Saving..." ) ) );
    }

    // filter specific
    switch( getFilterType() )
    {
        case FILTER_BIFF:
            mxCodecHelper.reset( new BiffCodecHelper( *this ) );
        break;

        case FILTER_OOX:
        break;

        case FILTER_UNKNOWN:
        break;
    }
}

void WorkbookData::finalize()
{
    // set some document properties needed after import
    if( mrBaseFilter.isImportFilter() )
    {
        PropertySet aPropSet( mxDoc );
        // #i74668# do not insert default sheets
        aPropSet.setProperty( PROP_IsLoaded, true );
        // #i79890# enable automatic update of defined names (before IsAdjustHeightEnabled!)
        Reference< XActionLockable > xLockable( getNamedRanges(), UNO_QUERY );
        if( xLockable.is() )
            xLockable->removeActionLock();
        // enable automatic update of linked sheets and DDE links
        aPropSet.setProperty( PROP_IsExecuteLinkEnabled, true );
        // #i79826# enable updating automatic row height after loading the document
        aPropSet.setProperty( PROP_IsAdjustHeightEnabled, true );
        // #i76026# enable Undo after loading the document
        aPropSet.setProperty( PROP_IsUndoEnabled, true );
        // disable editing read-only documents (e.g. from read-only files)
        aPropSet.setProperty( PROP_IsChangeReadOnlyEnabled, false );
    }
}

// ============================================================================

WorkbookHelper::~WorkbookHelper()
{
}

// filter ---------------------------------------------------------------------

FilterBase& WorkbookHelper::getBaseFilter() const
{
    return mrBookData.getBaseFilter();
}

Reference< XMultiServiceFactory > WorkbookHelper::getGlobalFactory() const
{
    return mrBookData.getBaseFilter().getGlobalFactory();
}

FilterType WorkbookHelper::getFilterType() const
{
    return mrBookData.getFilterType();
}

SegmentProgressBar& WorkbookHelper::getProgressBar() const
{
    return mrBookData.getProgressBar();
}

bool WorkbookHelper::isWorkbookFile() const
{
    return mrBookData.isWorkbookFile();
}

sal_Int16 WorkbookHelper::getCurrentSheetIndex() const
{
    return mrBookData.getCurrentSheetIndex();
}

void WorkbookHelper::setCurrentSheetIndex( sal_Int16 nSheet )
{
    mrBookData.setCurrentSheetIndex( nSheet );
}

void WorkbookHelper::setVbaProjectStorage( const StorageRef& rxVbaPrjStrg )
{
    mrBookData.setVbaProjectStorage( rxVbaPrjStrg );
}

void WorkbookHelper::finalizeWorkbookImport()
{
    // workbook settings, document and sheet view settings
    mrBookData.getWorkbookSettings().finalizeImport();
    mrBookData.getViewSettings().finalizeImport();

    /*  Insert all pivot tables. Must be done after loading all sheets, because
        data pilots expect existing source data on creation. */
    mrBookData.getPivotTables().finalizeImport();

    /*  Insert scenarios after all sheet processing is done, because new hidden
        sheets are created for scenarios which would confuse code that relies
        on certain sheet indexes. Must be done after pivot tables too. */
    mrBookData.getScenarios().finalizeImport();

    /*  Set 'Default' page style to automatic page numbering (default is manual
        number 1). Otherwise hidden sheets (e.g. for scenarios) which have
        'Default' page style will break automatic page numbering for following
        sheets. Automatic numbering is set by passing the value 0. */
    PropertySet aDefPageStyle( getStyleObject( CREATE_OUSTRING( "Default" ), true ) );
    aDefPageStyle.setProperty< sal_Int16 >( PROP_FirstPageNumber, 0 );

    /*  Import the VBA project (after finalizing workbook settings which
        contains the workbook code name). */
    StorageRef xVbaPrjStrg = mrBookData.getVbaProjectStorage();
    if( xVbaPrjStrg.get() && xVbaPrjStrg->isStorage() )
    {
        ::oox::ole::VbaProject aVbaProject( getGlobalFactory(), getBaseFilter().getModel(), CREATE_OUSTRING( "Calc" ) );
        aVbaProject.importVbaProject( *xVbaPrjStrg, getBaseFilter().getGraphicHelper() );
    }
}

// document model -------------------------------------------------------------

Reference< XSpreadsheetDocument > WorkbookHelper::getDocument() const
{
    return mrBookData.getDocument();
}

Reference< XMultiServiceFactory > WorkbookHelper::getDocumentFactory() const
{
    return mrBookData.getBaseFilter().getModelFactory();
}

Reference< XDevice > WorkbookHelper::getReferenceDevice() const
{
    return mrBookData.getReferenceDevice();
}

Reference< XNamedRanges > WorkbookHelper::getNamedRanges() const
{
    return mrBookData.getNamedRanges();
}

Reference< XDatabaseRanges > WorkbookHelper::getDatabaseRanges() const
{
    return mrBookData.getDatabaseRanges();
}

Reference< XExternalDocLinks > WorkbookHelper::getExternalDocLinks() const
{
    return mrBookData.getExternalDocLinks();
}

Reference< XNameAccess > WorkbookHelper::getDdeLinks() const
{
    return mrBookData.getDdeLinks();
}

Reference< XSpreadsheet > WorkbookHelper::getSheetFromDoc( sal_Int16 nSheet ) const
{
    Reference< XSpreadsheet > xSheet;
    try
    {
        Reference< XIndexAccess > xSheetsIA( getDocument()->getSheets(), UNO_QUERY_THROW );
        xSheet.set( xSheetsIA->getByIndex( nSheet ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    return xSheet;
}

Reference< XSpreadsheet > WorkbookHelper::getSheetFromDoc( const OUString& rSheet ) const
{
    Reference< XSpreadsheet > xSheet;
    try
    {
        Reference< XNameAccess > xSheetsNA( getDocument()->getSheets(), UNO_QUERY_THROW );
        xSheet.set( xSheetsNA->getByName( rSheet ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    return xSheet;
}

Reference< XCell > WorkbookHelper::getCellFromDoc( const CellAddress& rAddress ) const
{
    Reference< XCell > xCell;
    try
    {
        Reference< XSpreadsheet > xSheet( getSheetFromDoc( rAddress.Sheet ), UNO_SET_THROW );
        xCell = xSheet->getCellByPosition( rAddress.Column, rAddress.Row );
    }
    catch( Exception& )
    {
    }
    return xCell;
}

Reference< XCellRange > WorkbookHelper::getCellRangeFromDoc( const CellRangeAddress& rRange ) const
{
    Reference< XCellRange > xRange;
    try
    {
        Reference< XSpreadsheet > xSheet( getSheetFromDoc( rRange.Sheet ), UNO_SET_THROW );
        xRange = xSheet->getCellRangeByPosition( rRange.StartColumn, rRange.StartRow, rRange.EndColumn, rRange.EndRow );
    }
    catch( Exception& )
    {
    }
    return xRange;
}

Reference< XNameContainer > WorkbookHelper::getStyleFamily( bool bPageStyles ) const
{
    return mrBookData.getStyleFamily( bPageStyles );
}

Reference< XStyle > WorkbookHelper::getStyleObject( const OUString& rStyleName, bool bPageStyle ) const
{
    return mrBookData.getStyleObject( rStyleName, bPageStyle );
}

Reference< XNamedRange > WorkbookHelper::createNamedRangeObject( OUString& orName, sal_Int32 nNameFlags ) const
{
    return mrBookData.createNamedRangeObject( orName, nNameFlags );
}

Reference< XStyle > WorkbookHelper::createStyleObject( OUString& orStyleName, bool bPageStyle ) const
{
    return mrBookData.createStyleObject( orStyleName, bPageStyle );
}

// buffers --------------------------------------------------------------------

WorkbookSettings& WorkbookHelper::getWorkbookSettings() const
{
    return mrBookData.getWorkbookSettings();
}

ViewSettings& WorkbookHelper::getViewSettings() const
{
    return mrBookData.getViewSettings();
}

WorksheetBuffer& WorkbookHelper::getWorksheets() const
{
    return mrBookData.getWorksheets();
}

ThemeBuffer& WorkbookHelper::getTheme() const
{
    return mrBookData.getTheme();
}

StylesBuffer& WorkbookHelper::getStyles() const
{
    return mrBookData.getStyles();
}

SharedStringsBuffer& WorkbookHelper::getSharedStrings() const
{
    return mrBookData.getSharedStrings();
}

ExternalLinkBuffer& WorkbookHelper::getExternalLinks() const
{
    return mrBookData.getExternalLinks();
}

DefinedNamesBuffer& WorkbookHelper::getDefinedNames() const
{
    return mrBookData.getDefinedNames();
}

TableBuffer& WorkbookHelper::getTables() const
{
    return mrBookData.getTables();
}

ScenarioBuffer& WorkbookHelper::getScenarios() const
{
    return mrBookData.getScenarios();
}

WebQueryBuffer& WorkbookHelper::getWebQueries() const
{
    return mrBookData.getWebQueries();
}

PivotCacheBuffer& WorkbookHelper::getPivotCaches() const
{
    return mrBookData.getPivotCaches();
}

PivotTableBuffer& WorkbookHelper::getPivotTables() const
{
    return mrBookData.getPivotTables();
}

// converters -----------------------------------------------------------------

FormulaParser& WorkbookHelper::getFormulaParser() const
{
    return mrBookData.getFormulaParser();
}

UnitConverter& WorkbookHelper::getUnitConverter() const
{
    return mrBookData.getUnitConverter();
}

AddressConverter& WorkbookHelper::getAddressConverter() const
{
    return mrBookData.getAddressConverter();
}

ExcelChartConverter& WorkbookHelper::getChartConverter() const
{
    return mrBookData.getChartConverter();
}

PageSettingsConverter& WorkbookHelper::getPageSettingsConverter() const
{
    return mrBookData.getPageSettingsConverter();
}

// OOX specific ---------------------------------------------------------------

XmlFilterBase& WorkbookHelper::getOoxFilter() const
{
    OSL_ENSURE( mrBookData.getFilterType() == FILTER_OOX, "WorkbookHelper::getOoxFilter - invalid call" );
    return mrBookData.getOoxFilter();
}

bool WorkbookHelper::importOoxFragment( const ::rtl::Reference< FragmentHandler >& rxHandler )
{
    return getOoxFilter().importFragment( rxHandler );
}

// BIFF specific --------------------------------------------------------------

BinaryFilterBase& WorkbookHelper::getBiffFilter() const
{
    OSL_ENSURE( mrBookData.getFilterType() == FILTER_BIFF, "WorkbookHelper::getBiffFilter - invalid call" );
    return mrBookData.getBiffFilter();
}

BiffType WorkbookHelper::getBiff() const
{
    return mrBookData.getBiff();
}

rtl_TextEncoding WorkbookHelper::getTextEncoding() const
{
    return mrBookData.getTextEncoding();
}

void WorkbookHelper::setTextEncoding( rtl_TextEncoding eTextEnc )
{
    mrBookData.setTextEncoding( eTextEnc );
}

void WorkbookHelper::setCodePage( sal_uInt16 nCodePage )
{
    mrBookData.setCodePage( nCodePage );
}

void WorkbookHelper::setAppFontEncoding( rtl_TextEncoding eAppFontEnc )
{
    mrBookData.setAppFontEncoding( eAppFontEnc );
}

void WorkbookHelper::setIsWorkbookFile()
{
    mrBookData.setIsWorkbookFile();
}

void WorkbookHelper::createBuffersPerSheet( sal_Int16 nSheet )
{
    mrBookData.createBuffersPerSheet( nSheet );
}

BiffCodecHelper& WorkbookHelper::getCodecHelper() const
{
    return mrBookData.getCodecHelper();
}

// ============================================================================

namespace prv {

WorkbookDataOwner::WorkbookDataOwner( WorkbookDataRef xBookData ) :
    mxBookData( xBookData )
{
}

WorkbookDataOwner::~WorkbookDataOwner()
{
}

} // namespace prv

// ----------------------------------------------------------------------------

WorkbookHelperRoot::WorkbookHelperRoot( ExcelFilter& rFilter ) :
    prv::WorkbookDataOwner( prv::WorkbookDataRef( new WorkbookData( rFilter ) ) ),
    WorkbookHelper( *mxBookData )
{
}

WorkbookHelperRoot::WorkbookHelperRoot( ExcelBiffFilter& rFilter, BiffType eBiff ) :
    prv::WorkbookDataOwner( prv::WorkbookDataRef( new WorkbookData( rFilter, eBiff ) ) ),
    WorkbookHelper( *mxBookData )
{
}

bool WorkbookHelperRoot::isValid() const
{
    return mxBookData->isValid();
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
