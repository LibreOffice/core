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

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <osl/thread.h>
#include "oox/drawingml/theme.hxx"
#include "oox/helper/progressbar.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/ole/vbaproject.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/biffcodec.hxx"
#include "oox/xls/connectionsbuffer.hxx"
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
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetbuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::oox::core::BinaryFilterBase;
using ::oox::core::FilterBase;
using ::oox::core::FragmentHandler;
using ::oox::core::XmlFilterBase;
using ::oox::drawingml::Theme;
using ::rtl::OUString;

// ============================================================================

bool IgnoreCaseCompare::operator()( const OUString& rName1, const OUString& rName2 ) const
{
    // there is no wrapper in rtl::OUString, TODO: compare with collator
    return ::rtl_ustr_compareIgnoreAsciiCase_WithLength(
        rName1.getStr(), rName1.getLength(), rName2.getStr(), rName2.getLength() ) < 0;
}

// ============================================================================

class WorkbookGlobals
{
public:
    explicit            WorkbookGlobals( ExcelFilter& rFilter );
    explicit            WorkbookGlobals( ExcelBiffFilter& rFilter, BiffType eBiff );
                        ~WorkbookGlobals();

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
    /** Returns the VBA project storage. */
    inline StorageRef   getVbaProjectStorage() const { return mxVbaPrjStrg; }
    /** Returns the index of the current Calc sheet, if filter currently processes a sheet. */
    inline sal_Int16    getCurrentSheetIndex() const { return mnCurrSheet; }

    /** Sets the VBA project storage used to import VBA source code and forms. */
    inline void         setVbaProjectStorage( const StorageRef& rxVbaPrjStrg ) { mxVbaPrjStrg = rxVbaPrjStrg; }
    /** Sets the index of the current Calc sheet, if filter currently processes a sheet. */
    inline void         setCurrentSheetIndex( sal_Int16 nSheet ) { mnCurrSheet = nSheet; }

    // document model ---------------------------------------------------------

    /** Returns a reference to the source/target spreadsheet document model. */
    inline Reference< XSpreadsheetDocument > getDocument() const { return mxDoc; }
    /** Returns the cell or page styles container from the Calc document. */
    Reference< XNameContainer > getStyleFamily( bool bPageStyles ) const;
    /** Returns the specified cell or page style from the Calc document. */
    Reference< XStyle > getStyleObject( const OUString& rStyleName, bool bPageStyle ) const;
    /** Creates and returns a defined name on-the-fly in the Calc document. */
    Reference< XNamedRange > createNamedRangeObject( OUString& orName, sal_Int32 nNameFlags ) const;
    /** Creates and returns a database range on-the-fly in the Calc document. */
    Reference< XDatabaseRange > createDatabaseRangeObject( OUString& orName, const CellRangeAddress& rRangeAddr ) const;
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
    /** Returns the collection of external data connections. */
    inline ConnectionsBuffer&  getConnections() const { return *mxConnections; }
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

    // OOXML/BIFF12 specific --------------------------------------------------

    /** Returns the base OOXML/BIFF12 filter object. */
    inline XmlFilterBase& getOoxFilter() const { return *mpOoxFilter; }

    // BIFF2-BIFF8 specific ---------------------------------------------------

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
    typedef ::std::auto_ptr< ConnectionsBuffer >        ConnectionsBfrPtr;
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
    ConnectionsBfrPtr   mxConnections;          /// All external data connections.
    PivotCacheBfrPtr    mxPivotCaches;          /// All pivot caches in the document.
    PivotTableBfrPtr    mxPivotTables;          /// All pivot tables in the document.

    // converters
    FormulaParserPtr    mxFmlaParser;           /// Import formula parser.
    UnitConvPtr         mxUnitConverter;        /// General unit converter.
    AddressConvPtr      mxAddrConverter;        /// Cell address and cell range address converter.
    ExcelChartConvPtr   mxChartConverter;       /// Chart object converter.
    PageSettConvPtr     mxPageSettConverter;    /// Page/print settings converter.

    // OOXML/BIFF12 specific
    XmlFilterBase*      mpOoxFilter;            /// Base OOXML/BIFF12 filter object.

    // BIFF2-BIFF8 specific
    BinaryFilterBase*   mpBiffFilter;           /// Base BIFF2-BIFF8 filter object.
    BiffCodecHelperPtr  mxCodecHelper;          /// Encoder/decoder helper.
    BiffType            meBiff;                 /// BIFF version for BIFF import/export.
    rtl_TextEncoding    meTextEnc;              /// BIFF byte string text encoding.
    bool                mbHasCodePage;          /// True = CODEPAGE record exists in imported stream.
};

// ----------------------------------------------------------------------------

WorkbookGlobals::WorkbookGlobals( ExcelFilter& rFilter ) :
    mrBaseFilter( rFilter ),
    mrExcelBase( rFilter ),
    meFilterType( FILTER_OOXML ),
    mpOoxFilter( &rFilter ),
    mpBiffFilter( 0 ),
    meBiff( BIFF_UNKNOWN )
{
    // register at the filter, needed for virtual callbacks (even during construction)
    mrExcelBase.registerWorkbookGlobals( *this );
    initialize( true );
}

WorkbookGlobals::WorkbookGlobals( ExcelBiffFilter& rFilter, BiffType eBiff ) :
    mrBaseFilter( rFilter ),
    mrExcelBase( rFilter ),
    meFilterType( FILTER_BIFF ),
    mpOoxFilter( 0 ),
    mpBiffFilter( &rFilter ),
    meBiff( eBiff )
{
    // register at the filter, needed for virtual callbacks (even during construction)
    mrExcelBase.registerWorkbookGlobals( *this );
    initialize( eBiff >= BIFF5 );
}

WorkbookGlobals::~WorkbookGlobals()
{
    finalize();
    mrExcelBase.unregisterWorkbookGlobals();
}

// document model -------------------------------------------------------------

Reference< XNameContainer > WorkbookGlobals::getStyleFamily( bool bPageStyles ) const
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
    OSL_ENSURE( xStylesNC.is(), "WorkbookGlobals::getStyleFamily - cannot access style family" );
    return xStylesNC;
}

Reference< XStyle > WorkbookGlobals::getStyleObject( const OUString& rStyleName, bool bPageStyle ) const
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
    OSL_ENSURE( xStyle.is(), "WorkbookGlobals::getStyleObject - cannot access style object" );
    return xStyle;
}

Reference< XNamedRange > WorkbookGlobals::createNamedRangeObject( OUString& orName, sal_Int32 nNameFlags ) const
{
    // create the name and insert it into the Calc document
    Reference< XNamedRange > xNamedRange;
    if( orName.getLength() > 0 ) try
    {
        // find an unused name
        PropertySet aDocProps( mxDoc );
        Reference< XNamedRanges > xNamedRanges( aDocProps.getAnyProperty( PROP_NamedRanges ), UNO_QUERY_THROW );
        Reference< XNameAccess > xNameAccess( xNamedRanges, UNO_QUERY_THROW );
        orName = ContainerHelper::getUnusedName( xNameAccess, orName, '_' );
        // create the named range
        xNamedRanges->addNewByName( orName, OUString(), CellAddress( 0, 0, 0 ), nNameFlags );
        xNamedRange.set( xNamedRanges->getByName( orName ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xNamedRange.is(), "WorkbookGlobals::createNamedRangeObject - cannot create defined name" );
    return xNamedRange;
}

Reference< XDatabaseRange > WorkbookGlobals::createDatabaseRangeObject( OUString& orName, const CellRangeAddress& rRangeAddr ) const
{
    // validate cell range
    CellRangeAddress aDestRange = rRangeAddr;
    bool bValidRange = getAddressConverter().validateCellRange( aDestRange, true, true );

    // create database range and insert it into the Calc document
    Reference< XDatabaseRange > xDatabaseRange;
    if( bValidRange && (orName.getLength() > 0) ) try
    {
        // find an unused name
        PropertySet aDocProps( mxDoc );
        Reference< XDatabaseRanges > xDatabaseRanges( aDocProps.getAnyProperty( PROP_DatabaseRanges ), UNO_QUERY_THROW );
        Reference< XNameAccess > xNameAccess( xDatabaseRanges, UNO_QUERY_THROW );
        orName = ContainerHelper::getUnusedName( xNameAccess, orName, '_' );
        // create the database range
        xDatabaseRanges->addNewByName( orName, aDestRange );
        xDatabaseRange.set( xDatabaseRanges->getByName( orName ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xDatabaseRange.is(), "WorkbookGlobals::createDatabaseRangeObject - cannot create database range" );
    return xDatabaseRange;
}

Reference< XStyle > WorkbookGlobals::createStyleObject( OUString& orStyleName, bool bPageStyle ) const
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
    OSL_ENSURE( xStyle.is(), "WorkbookGlobals::createStyleObject - cannot create style" );
    return xStyle;
}

// BIFF specific --------------------------------------------------------------

void WorkbookGlobals::setTextEncoding( rtl_TextEncoding eTextEnc )
{
    if( eTextEnc != RTL_TEXTENCODING_DONTKNOW )
        meTextEnc = eTextEnc;
}

void WorkbookGlobals::setCodePage( sal_uInt16 nCodePage )
{
    setTextEncoding( BiffHelper::calcTextEncodingFromCodePage( nCodePage ) );
    mbHasCodePage = true;
}

void WorkbookGlobals::setAppFontEncoding( rtl_TextEncoding eAppFontEnc )
{
    if( !mbHasCodePage )
        setTextEncoding( eAppFontEnc );
}

void WorkbookGlobals::setIsWorkbookFile()
{
    OSL_ENSURE( meBiff == BIFF4, "WorkbookGlobals::setIsWorkbookFile - invalid call" );
    mbWorkbook = true;
}

void WorkbookGlobals::createBuffersPerSheet( sal_Int16 nSheet )
{
    switch( meBiff )
    {
        case BIFF2:
        case BIFF3:
            OSL_ENSURE( nSheet == 0, "WorkbookGlobals::createBuffersPerSheet - unexpected sheet index" );
            mxDefNames->setLocalCalcSheet( nSheet );
        break;

        case BIFF4:
            OSL_ENSURE( mbWorkbook || (nSheet == 0), "WorkbookGlobals::createBuffersPerSheet - unexpected sheet index" );
            // #i11183# sheets in BIFF4W files have own styles and names
            if( nSheet > 0 )
            {
                mxStyles.reset( new StylesBuffer( *this ) );
                mxDefNames.reset( new DefinedNamesBuffer( *this ) );
                mxExtLinks.reset( new ExternalLinkBuffer( *this ) );
            }
            mxDefNames->setLocalCalcSheet( nSheet );
        break;

        case BIFF5:
            // BIFF5 stores external references per sheet
            if( nSheet > 0 )
                mxExtLinks.reset( new ExternalLinkBuffer( *this ) );
        break;

        case BIFF8:
        break;

        case BIFF_UNKNOWN:
        break;
    }
}

// private --------------------------------------------------------------------

void WorkbookGlobals::initialize( bool bWorkbookFile )
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
    OSL_ENSURE( mxDoc.is(), "WorkbookGlobals::initialize - no spreadsheet document" );

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
    mxConnections.reset( new ConnectionsBuffer( *this ) );
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
        Reference< XActionLockable > xLockable( aPropSet.getAnyProperty( PROP_NamedRanges ), UNO_QUERY );
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

        case FILTER_OOXML:
        break;

        case FILTER_UNKNOWN:
        break;
    }
}

void WorkbookGlobals::finalize()
{
    // set some document properties needed after import
    if( mrBaseFilter.isImportFilter() )
    {
        PropertySet aPropSet( mxDoc );
        // #i74668# do not insert default sheets
        aPropSet.setProperty( PROP_IsLoaded, true );
        // #i79890# enable automatic update of defined names (before IsAdjustHeightEnabled!)
        Reference< XActionLockable > xLockable( aPropSet.getAnyProperty( PROP_NamedRanges ), UNO_QUERY );
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
        // #111099# open forms in alive mode (has no effect, if no controls in document)
        aPropSet.setProperty( PROP_ApplyFormDesignMode, false );
    }
}

// ============================================================================

WorkbookHelper::~WorkbookHelper()
{
}

/*static*/ WorkbookGlobalsRef WorkbookHelper::constructGlobals( ExcelFilter& rFilter )
{
    WorkbookGlobalsRef xBookGlob( new WorkbookGlobals( rFilter ) );
    if( !xBookGlob->isValid() )
        xBookGlob.reset();
    return xBookGlob;
}

/*static*/ WorkbookGlobalsRef WorkbookHelper::constructGlobals( ExcelBiffFilter& rFilter, BiffType eBiff )
{
    WorkbookGlobalsRef xBookGlob( new WorkbookGlobals( rFilter, eBiff ) );
    if( !xBookGlob->isValid() )
        xBookGlob.reset();
    return xBookGlob;
}

// filter ---------------------------------------------------------------------

FilterBase& WorkbookHelper::getBaseFilter() const
{
    return mrBookGlob.getBaseFilter();
}

FilterType WorkbookHelper::getFilterType() const
{
    return mrBookGlob.getFilterType();
}

SegmentProgressBar& WorkbookHelper::getProgressBar() const
{
    return mrBookGlob.getProgressBar();
}

bool WorkbookHelper::isWorkbookFile() const
{
    return mrBookGlob.isWorkbookFile();
}

sal_Int16 WorkbookHelper::getCurrentSheetIndex() const
{
    return mrBookGlob.getCurrentSheetIndex();
}

void WorkbookHelper::setVbaProjectStorage( const StorageRef& rxVbaPrjStrg )
{
    mrBookGlob.setVbaProjectStorage( rxVbaPrjStrg );
}

void WorkbookHelper::setCurrentSheetIndex( sal_Int16 nSheet )
{
    mrBookGlob.setCurrentSheetIndex( nSheet );
}

void WorkbookHelper::finalizeWorkbookImport()
{
    // workbook settings, document and sheet view settings
    mrBookGlob.getWorkbookSettings().finalizeImport();
    mrBookGlob.getViewSettings().finalizeImport();

    /*  Insert all pivot tables. Must be done after loading all sheets, because
        data pilots expect existing source data on creation. */
    mrBookGlob.getPivotTables().finalizeImport();

    /*  Insert scenarios after all sheet processing is done, because new hidden
        sheets are created for scenarios which would confuse code that relies
        on certain sheet indexes. Must be done after pivot tables too. */
    mrBookGlob.getScenarios().finalizeImport();

    /*  Set 'Default' page style to automatic page numbering (default is manual
        number 1). Otherwise hidden sheets (e.g. for scenarios) which have
        'Default' page style will break automatic page numbering for following
        sheets. Automatic numbering is set by passing the value 0. */
    PropertySet aDefPageStyle( getStyleObject( CREATE_OUSTRING( "Default" ), true ) );
    aDefPageStyle.setProperty< sal_Int16 >( PROP_FirstPageNumber, 0 );

    /*  Import the VBA project (after finalizing workbook settings which
        contains the workbook code name). */
    StorageRef xVbaPrjStrg = mrBookGlob.getVbaProjectStorage();
    if( xVbaPrjStrg.get() && xVbaPrjStrg->isStorage() )
        getBaseFilter().getVbaProject().importVbaProject( *xVbaPrjStrg, getBaseFilter().getGraphicHelper() );
}

// document model -------------------------------------------------------------

Reference< XSpreadsheetDocument > WorkbookHelper::getDocument() const
{
    return mrBookGlob.getDocument();
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
    return mrBookGlob.getStyleFamily( bPageStyles );
}

Reference< XStyle > WorkbookHelper::getStyleObject( const OUString& rStyleName, bool bPageStyle ) const
{
    return mrBookGlob.getStyleObject( rStyleName, bPageStyle );
}

Reference< XNamedRange > WorkbookHelper::createNamedRangeObject( OUString& orName, sal_Int32 nNameFlags ) const
{
    return mrBookGlob.createNamedRangeObject( orName, nNameFlags );
}

Reference< XDatabaseRange > WorkbookHelper::createDatabaseRangeObject( OUString& orName, const CellRangeAddress& rRangeAddr ) const
{
    return mrBookGlob.createDatabaseRangeObject( orName, rRangeAddr );
}

Reference< XStyle > WorkbookHelper::createStyleObject( OUString& orStyleName, bool bPageStyle ) const
{
    return mrBookGlob.createStyleObject( orStyleName, bPageStyle );
}

// buffers --------------------------------------------------------------------

WorkbookSettings& WorkbookHelper::getWorkbookSettings() const
{
    return mrBookGlob.getWorkbookSettings();
}

ViewSettings& WorkbookHelper::getViewSettings() const
{
    return mrBookGlob.getViewSettings();
}

WorksheetBuffer& WorkbookHelper::getWorksheets() const
{
    return mrBookGlob.getWorksheets();
}

ThemeBuffer& WorkbookHelper::getTheme() const
{
    return mrBookGlob.getTheme();
}

StylesBuffer& WorkbookHelper::getStyles() const
{
    return mrBookGlob.getStyles();
}

SharedStringsBuffer& WorkbookHelper::getSharedStrings() const
{
    return mrBookGlob.getSharedStrings();
}

ExternalLinkBuffer& WorkbookHelper::getExternalLinks() const
{
    return mrBookGlob.getExternalLinks();
}

DefinedNamesBuffer& WorkbookHelper::getDefinedNames() const
{
    return mrBookGlob.getDefinedNames();
}

TableBuffer& WorkbookHelper::getTables() const
{
    return mrBookGlob.getTables();
}

ScenarioBuffer& WorkbookHelper::getScenarios() const
{
    return mrBookGlob.getScenarios();
}

ConnectionsBuffer& WorkbookHelper::getConnections() const
{
    return mrBookGlob.getConnections();
}

PivotCacheBuffer& WorkbookHelper::getPivotCaches() const
{
    return mrBookGlob.getPivotCaches();
}

PivotTableBuffer& WorkbookHelper::getPivotTables() const
{
    return mrBookGlob.getPivotTables();
}

// converters -----------------------------------------------------------------

FormulaParser& WorkbookHelper::getFormulaParser() const
{
    return mrBookGlob.getFormulaParser();
}

UnitConverter& WorkbookHelper::getUnitConverter() const
{
    return mrBookGlob.getUnitConverter();
}

AddressConverter& WorkbookHelper::getAddressConverter() const
{
    return mrBookGlob.getAddressConverter();
}

ExcelChartConverter& WorkbookHelper::getChartConverter() const
{
    return mrBookGlob.getChartConverter();
}

PageSettingsConverter& WorkbookHelper::getPageSettingsConverter() const
{
    return mrBookGlob.getPageSettingsConverter();
}

// OOXML/BIFF12 specific ------------------------------------------------------

XmlFilterBase& WorkbookHelper::getOoxFilter() const
{
    OSL_ENSURE( mrBookGlob.getFilterType() == FILTER_OOXML, "WorkbookHelper::getOoxFilter - invalid call" );
    return mrBookGlob.getOoxFilter();
}

bool WorkbookHelper::importOoxFragment( const ::rtl::Reference< FragmentHandler >& rxHandler )
{
    return getOoxFilter().importFragment( rxHandler );
}

// BIFF specific --------------------------------------------------------------

BinaryFilterBase& WorkbookHelper::getBiffFilter() const
{
    OSL_ENSURE( mrBookGlob.getFilterType() == FILTER_BIFF, "WorkbookHelper::getBiffFilter - invalid call" );
    return mrBookGlob.getBiffFilter();
}

BiffType WorkbookHelper::getBiff() const
{
    return mrBookGlob.getBiff();
}

rtl_TextEncoding WorkbookHelper::getTextEncoding() const
{
    return mrBookGlob.getTextEncoding();
}

void WorkbookHelper::setTextEncoding( rtl_TextEncoding eTextEnc )
{
    mrBookGlob.setTextEncoding( eTextEnc );
}

void WorkbookHelper::setCodePage( sal_uInt16 nCodePage )
{
    mrBookGlob.setCodePage( nCodePage );
}

void WorkbookHelper::setAppFontEncoding( rtl_TextEncoding eAppFontEnc )
{
    mrBookGlob.setAppFontEncoding( eAppFontEnc );
}

void WorkbookHelper::setIsWorkbookFile()
{
    mrBookGlob.setIsWorkbookFile();
}

void WorkbookHelper::createBuffersPerSheet( sal_Int16 nSheet )
{
    mrBookGlob.createBuffersPerSheet( nSheet );
}

BiffCodecHelper& WorkbookHelper::getCodecHelper() const
{
    return mrBookGlob.getCodecHelper();
}

// ============================================================================

} // namespace xls
} // namespace oox
