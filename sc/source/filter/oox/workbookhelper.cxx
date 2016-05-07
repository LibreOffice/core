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

#include "workbookhelper.hxx"

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XUnnamedDatabaseRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <osl/thread.h>
#include <oox/helper/progressbar.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/ole/vbaproject.hxx>
#include <oox/token/properties.hxx>
#include <vcl/msgbox.hxx>
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "biffcodec.hxx"
#include "connectionsbuffer.hxx"
#include "defnamesbuffer.hxx"
#include "excelchartconverter.hxx"
#include "excelfilter.hxx"
#include "externallinkbuffer.hxx"
#include "formulaparser.hxx"
#include "pagesettings.hxx"
#include "pivotcachebuffer.hxx"
#include "pivottablebuffer.hxx"
#include "scenariobuffer.hxx"
#include "sharedstringsbuffer.hxx"
#include "stylesbuffer.hxx"
#include "tablebuffer.hxx"
#include "themebuffer.hxx"
#include "unitconverter.hxx"
#include "viewsettings.hxx"
#include "workbooksettings.hxx"
#include "worksheetbuffer.hxx"
#include "scmod.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "rangenam.hxx"
#include "tokenarray.hxx"
#include "tokenuno.hxx"
#include "convuno.hxx"
#include "dbdata.hxx"
#include "datauno.hxx"
#include "globalnames.hxx"
#include "documentimport.hxx"
#include "drwlayer.hxx"
#include "globstr.hrc"

#include "formulabuffer.hxx"
#include <vcl/mapmod.hxx>
#include "editutil.hxx"
#include <editeng/editstat.hxx>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Calc.hxx>

#include <memory>

namespace oox {
namespace xls {

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::oox::core::FilterBase;
using ::oox::core::FragmentHandler;
using ::oox::core::XmlFilterBase;

bool IgnoreCaseCompare::operator()( const OUString& rName1, const OUString& rName2 ) const
{
    // TODO: compare with collator
    return rName1.compareToIgnoreAsciiCase(rName2 ) < 0;
}

class WorkbookGlobals
{
public:
    // noncopyable ------------------------------------------------------------

    WorkbookGlobals(const WorkbookGlobals&) = delete;
    const WorkbookGlobals& operator=(const WorkbookGlobals&) = delete;

    explicit            WorkbookGlobals( ExcelFilter& rFilter );
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
    const StorageRef&   getVbaProjectStorage() const { return mxVbaPrjStrg; }
    /** Returns the index of the current Calc sheet, if filter currently processes a sheet. */
    inline sal_Int16    getCurrentSheetIndex() const { return mnCurrSheet; }

    /** Sets the VBA project storage used to import VBA source code and forms. */
    inline void         setVbaProjectStorage( const StorageRef& rxVbaPrjStrg ) { mxVbaPrjStrg = rxVbaPrjStrg; }
    /** Sets the index of the current Calc sheet, if filter currently processes a sheet. */
    inline void         setCurrentSheetIndex( sal_Int16 nSheet ) { mnCurrSheet = nSheet; }

    // document model ---------------------------------------------------------

    inline ScEditEngineDefaulter& getEditEngine() const
    {
        return *mxEditEngine.get();
    }

    ScDocument& getScDocument() { return *mpDoc; }

    ScDocumentImport& getDocImport();

    /** Returns a reference to the source/target spreadsheet document model. */
    const Reference< XSpreadsheetDocument >& getDocument() const { return mxDoc; }
    /** Returns the cell or page styles container from the Calc document. */
    Reference< XNameContainer > getStyleFamily( bool bPageStyles ) const;
    /** Returns the specified cell or page style from the Calc document. */
    Reference< XStyle > getStyleObject( const OUString& rStyleName, bool bPageStyle ) const;
    /** Creates and returns a defined name on-the-fly in the Calc document. */
    ScRangeData* createNamedRangeObject( OUString& orName, const Sequence< FormulaToken>& rTokens, sal_Int32 nIndex, sal_Int32 nNameFlags );
    /** Creates and returns a defined name on the-fly in the correct Calc sheet. */
    ScRangeData* createLocalNamedRangeObject( OUString& orName, const Sequence< FormulaToken>& rTokens, sal_Int32 nIndex, sal_Int32 nNameFlags, sal_Int32 nTab );
    /** Creates and returns a database range on-the-fly in the Calc document. */
    Reference< XDatabaseRange > createDatabaseRangeObject( OUString& orName, const CellRangeAddress& rRangeAddr );
    /** Creates and returns an unnamed database range on-the-fly in the Calc document. */
    Reference< XDatabaseRange > createUnnamedDatabaseRangeObject( const CellRangeAddress& rRangeAddr );
    /** Finds the (already existing) database range of the given formula token index. */
    ScDBData* findDatabaseRangeByIndex( sal_uInt16 nIndex );
    /** Creates and returns a com.sun.star.style.Style object for cells or pages. */
    Reference< XStyle > createStyleObject( OUString& orStyleName, bool bPageStyle );
    /** Helper to switch chart data table - specifically for xlsx imports */
    void useInternalChartDataTable( bool bInternal );

    // buffers ----------------------------------------------------------------

    inline FormulaBuffer& getFormulaBuffer() const { return *mxFormulaBuffer; }
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

    /** Returns a shared import formula parser. */
    inline FormulaParser& getFormulaParser() const { return *mxFmlaParser; }
    /** Returns an unshared import formula parser. */
    inline FormulaParser* createFormulaParser() { return new FormulaParser(*this); }
    /** Returns the measurement unit converter. */
    inline UnitConverter& getUnitConverter() const { return *mxUnitConverter; }
    /** Returns the converter for string to cell address/range conversion. */
    inline AddressConverter& getAddressConverter() const { return *mxAddrConverter; }
    /** Returns the chart object converter. */
    inline oox::drawingml::chart::ChartConverter* getChartConverter() const { return mxChartConverter.get(); }
    /** Returns the page/print settings converter. */
    inline PageSettingsConverter& getPageSettingsConverter() const { return *mxPageSettConverter; }

    // OOXML/BIFF12 specific --------------------------------------------------

    /** Returns the base OOXML/BIFF12 filter object. */
    inline XmlFilterBase& getOoxFilter() const { return *mpOoxFilter; }

    // BIFF2-BIFF8 specific ---------------------------------------------------

    /** Returns the BIFF type in binary filter. */
    inline BiffType     getBiff() const { return meBiff; }
    /** Returns the text encoding used to import/export byte strings. */
    inline rtl_TextEncoding getTextEncoding() const { return meTextEnc; }
    /** Returns the codec helper that stores the encoder/decoder object. */
    inline BiffCodecHelper& getCodecHelper() { return *mxCodecHelper; }

private:
    /** Initializes some basic members and sets needed document properties. */
    void                initialize();
    /** Finalizes the filter process (sets some needed document properties). */
    void                finalize();

private:
    typedef ::std::unique_ptr< ScEditEngineDefaulter >    EditEngineDefaulterPtr;
    typedef ::std::unique_ptr< FormulaBuffer >          FormulaBufferPtr;
    typedef ::std::unique_ptr< SegmentProgressBar >     ProgressBarPtr;
    typedef ::std::unique_ptr< WorkbookSettings >       WorkbookSettPtr;
    typedef ::std::unique_ptr< ViewSettings >           ViewSettingsPtr;
    typedef ::std::unique_ptr< WorksheetBuffer >        WorksheetBfrPtr;
    typedef ::std::shared_ptr< ThemeBuffer >          ThemeBfrRef;
    typedef ::std::unique_ptr< StylesBuffer >           StylesBfrPtr;
    typedef ::std::unique_ptr< SharedStringsBuffer >    SharedStrBfrPtr;
    typedef ::std::unique_ptr< ExternalLinkBuffer >     ExtLinkBfrPtr;
    typedef ::std::unique_ptr< DefinedNamesBuffer >     DefNamesBfrPtr;
    typedef ::std::unique_ptr< TableBuffer >            TableBfrPtr;
    typedef ::std::unique_ptr< ScenarioBuffer >         ScenarioBfrPtr;
    typedef ::std::unique_ptr< ConnectionsBuffer >      ConnectionsBfrPtr;
    typedef ::std::unique_ptr< PivotCacheBuffer >       PivotCacheBfrPtr;
    typedef ::std::unique_ptr< PivotTableBuffer >       PivotTableBfrPtr;
    typedef ::std::unique_ptr< FormulaParser >          FormulaParserPtr;
    typedef ::std::unique_ptr< UnitConverter >          UnitConvPtr;
    typedef ::std::unique_ptr< AddressConverter >       AddressConvPtr;
    typedef ::std::unique_ptr< oox::drawingml::chart::ChartConverter > ExcelChartConvPtr;
    typedef ::std::unique_ptr< PageSettingsConverter >  PageSettConvPtr;
    typedef ::std::unique_ptr< BiffCodecHelper >        BiffCodecHelperPtr;

    OUString            maCellStyles;           /// Style family name for cell styles.
    OUString            maPageStyles;           /// Style family name for page styles.
    OUString            maCellStyleServ;        /// Service name for a cell style.
    OUString            maPageStyleServ;        /// Service name for a page style.
    Reference< XSpreadsheetDocument > mxDoc;    /// Document model.
    FilterBase&         mrBaseFilter;           /// Base filter object.
    ExcelFilter&        mrExcelFilter;          /// Base object for registration of this structure.
    FilterType          meFilterType;           /// File type of the filter.
    ProgressBarPtr      mxProgressBar;          /// The progress bar.
    StorageRef          mxVbaPrjStrg;           /// Storage containing the VBA project.
    sal_Int16           mnCurrSheet;            /// Current sheet index in Calc document.
    bool                mbWorkbook;             /// True = multi-sheet file.

    // buffers
    FormulaBufferPtr    mxFormulaBuffer;
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

    EditEngineDefaulterPtr mxEditEngine;

    // OOXML/BIFF12 specific
    XmlFilterBase*      mpOoxFilter;            /// Base OOXML/BIFF12 filter object.

    // BIFF2-BIFF8 specific
    BiffCodecHelperPtr  mxCodecHelper;          /// Encoder/decoder helper.
    BiffType            meBiff;                 /// BIFF version for BIFF import/export.
    rtl_TextEncoding    meTextEnc;              /// BIFF byte string text encoding.
    bool                mbHasCodePage;          /// True = CODEPAGE record exists in imported stream.
    ScDocument* mpDoc;
    ScDocShell* mpDocShell;
    std::unique_ptr<ScDocumentImport> mxDocImport;
};

WorkbookGlobals::WorkbookGlobals( ExcelFilter& rFilter ) :
    mrBaseFilter( rFilter ),
    mrExcelFilter( rFilter ),
    meFilterType( FILTER_OOXML ),
    mpOoxFilter( &rFilter ),
    meBiff( BIFF_UNKNOWN ),
    mpDoc(nullptr),
    mpDocShell(nullptr)
{
    // register at the filter, needed for virtual callbacks (even during construction)
    mrExcelFilter.registerWorkbookGlobals( *this );
    initialize();
}

WorkbookGlobals::~WorkbookGlobals()
{
    finalize();
    mrExcelFilter.unregisterWorkbookGlobals();
}

ScDocumentImport& WorkbookGlobals::getDocImport()
{
    return *mxDocImport;
}

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

namespace {

ScRangeData* lcl_addNewByNameAndTokens( ScDocument& rDoc, ScRangeName* pNames, const OUString& rName, const Sequence<FormulaToken>& rTokens, sal_Int16 nIndex, sal_Int32 nUnoType )
{
    bool bDone = false;
    ScRangeData::Type nNewType = ScRangeData::Type::Name;
    if ( nUnoType & NamedRangeFlag::FILTER_CRITERIA )    nNewType |= ScRangeData::Type::Criteria;
    if ( nUnoType & NamedRangeFlag::PRINT_AREA )         nNewType |= ScRangeData::Type::PrintArea;
    if ( nUnoType & NamedRangeFlag::COLUMN_HEADER )      nNewType |= ScRangeData::Type::ColHeader;
    if ( nUnoType & NamedRangeFlag::ROW_HEADER )         nNewType |= ScRangeData::Type::RowHeader;
    ScTokenArray aTokenArray;
    (void)ScTokenConversion::ConvertToTokenArray( rDoc, aTokenArray, rTokens );
    ScRangeData* pNew = new ScRangeData( &rDoc, rName, aTokenArray, ScAddress(), nNewType );
    pNew->GuessPosition();
    if ( nIndex )
        pNew->SetIndex( nIndex );
    if ( pNames->insert(pNew) )
        bDone = true;
    if (!bDone)
        throw RuntimeException();
    return pNew;
}

OUString findUnusedName( const ScRangeName* pRangeName, const OUString& rSuggestedName )
{
    OUString aNewName = rSuggestedName;
    sal_Int32 nIndex = 0;
    while(pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(aNewName)))
        aNewName = OUStringBuffer(rSuggestedName).append( '_' ).append( nIndex++ ).makeStringAndClear();

    return aNewName;
}

}

ScRangeData* WorkbookGlobals::createNamedRangeObject(
    OUString& orName, const Sequence< FormulaToken>& rTokens, sal_Int32 nIndex, sal_Int32 nNameFlags )
{
    // create the name and insert it into the Calc document
    ScRangeData* pScRangeData = nullptr;
    if( !orName.isEmpty() )
    {
        ScDocument& rDoc =  getScDocument();
        ScRangeName* pNames = rDoc.GetRangeName();
        // find an unused name
        orName = findUnusedName( pNames, orName );
        // create the named range
        pScRangeData = lcl_addNewByNameAndTokens( rDoc, pNames, orName, rTokens, nIndex, nNameFlags );
    }
    return pScRangeData;
}

ScRangeData* WorkbookGlobals::createLocalNamedRangeObject(
    OUString& orName, const Sequence< FormulaToken >&  rTokens, sal_Int32 nIndex, sal_Int32 nNameFlags, sal_Int32 nTab )
{
    // create the name and insert it into the Calc document
    ScRangeData* pScRangeData = nullptr;
    if( !orName.isEmpty() )
    {
        ScDocument& rDoc =  getScDocument();
        ScRangeName* pNames = rDoc.GetRangeName( nTab );
        if(!pNames)
            throw RuntimeException("invalid sheet index used");
        // find an unused name
        orName = findUnusedName( pNames, orName );
        // create the named range
        pScRangeData = lcl_addNewByNameAndTokens( rDoc, pNames, orName, rTokens, nIndex, nNameFlags );
    }
    return pScRangeData;
}

Reference< XDatabaseRange > WorkbookGlobals::createDatabaseRangeObject( OUString& orName, const CellRangeAddress& rRangeAddr )
{
    // validate cell range
    CellRangeAddress aDestRange = rRangeAddr;
    bool bValidRange = getAddressConverter().validateCellRange( aDestRange, true, true );

    // create database range and insert it into the Calc document
    Reference< XDatabaseRange > xDatabaseRange;
    if( bValidRange && !orName.isEmpty() ) try
    {
        // find an unused name
        PropertySet aDocProps( mxDoc );
        Reference< XDatabaseRanges > xDatabaseRanges( aDocProps.getAnyProperty( PROP_DatabaseRanges ), UNO_QUERY_THROW );
        orName = ContainerHelper::getUnusedName( xDatabaseRanges, orName, '_' );
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

Reference< XDatabaseRange > WorkbookGlobals::createUnnamedDatabaseRangeObject( const CellRangeAddress& rRangeAddr )
{
    // validate cell range
    CellRangeAddress aDestRange = rRangeAddr;
    bool bValidRange = getAddressConverter().validateCellRange( aDestRange, true, true );

    // create database range and insert it into the Calc document
    Reference< XDatabaseRange > xDatabaseRange;
    if( bValidRange ) try
    {
        ScDocument& rDoc =  getScDocument();
        if( rDoc.GetTableCount() <= aDestRange.Sheet )
            throw css::lang::IndexOutOfBoundsException();
        ScRange aScRange;
        ScUnoConversion::FillScRange(aScRange, aDestRange);
        ScDBData* pNewDBData = new ScDBData( STR_DB_LOCAL_NONAME, aScRange.aStart.Tab(),
                                       aScRange.aStart.Col(), aScRange.aStart.Row(),
                                       aScRange.aEnd.Col(), aScRange.aEnd.Row() );
        rDoc.SetAnonymousDBData( aScRange.aStart.Tab() , pNewDBData );
        ScDocShell* pDocSh = static_cast< ScDocShell* >(rDoc.GetDocumentShell());
        xDatabaseRange.set(new ScDatabaseRangeObj(pDocSh, aScRange.aStart.Tab()));
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xDatabaseRange.is(), "WorkbookData::createDatabaseRangeObject - cannot create database range" );
    return xDatabaseRange;
}

ScDBData* WorkbookGlobals::findDatabaseRangeByIndex( sal_uInt16 nIndex )
{
    ScDBCollection* pDBCollection = getScDocument().GetDBCollection();
    if (!pDBCollection)
        return nullptr;
    return pDBCollection->getNamedDBs().findByIndex( nIndex );
}

Reference< XStyle > WorkbookGlobals::createStyleObject( OUString& orStyleName, bool bPageStyle )
{
    Reference< XStyle > xStyle;
    try
    {
        Reference< XNameContainer > xStylesNC( getStyleFamily( bPageStyle ), UNO_SET_THROW );
        xStyle.set( mrBaseFilter.getModelFactory()->createInstance( bPageStyle ? maPageStyleServ : maCellStyleServ ), UNO_QUERY_THROW );
        orStyleName = ContainerHelper::insertByUnusedName( xStylesNC, orStyleName, ' ', Any( xStyle ) );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xStyle.is(), "WorkbookGlobals::createStyleObject - cannot create style" );
    return xStyle;
}

void WorkbookGlobals::useInternalChartDataTable( bool bInternal )
{
    if( bInternal )
        mxChartConverter.reset( new oox::drawingml::chart::ChartConverter() );
    else
        mxChartConverter.reset( new ExcelChartConverter( *this ) );
}

// BIFF specific --------------------------------------------------------------

// private --------------------------------------------------------------------

void WorkbookGlobals::initialize()
{
    maCellStyles = "CellStyles";
    maPageStyles = "PageStyles";
    maCellStyleServ = "com.sun.star.style.CellStyle";
    maPageStyleServ = "com.sun.star.style.PageStyle";
    mnCurrSheet = -1;
    mbWorkbook = true;
    meTextEnc = osl_getThreadTextEncoding();
    mbHasCodePage = false;

    // the spreadsheet document
    mxDoc.set( mrBaseFilter.getModel(), UNO_QUERY );
    OSL_ENSURE( mxDoc.is(), "WorkbookGlobals::initialize - no spreadsheet document" );

    if (mxDoc.get())
    {
        ScModelObj* pModel = dynamic_cast<ScModelObj*>(mxDoc.get());
        if (pModel)
            mpDocShell = static_cast<ScDocShell*>(pModel->GetEmbeddedObject());
        if (mpDocShell)
            mpDoc = &mpDocShell->GetDocument();
    }

    if (!mpDoc)
        throw RuntimeException("Workbookhelper::getScDocument(): Failed to access ScDocument from model");

    Reference< XDocumentPropertiesSupplier > xPropSupplier( mxDoc, UNO_QUERY);
    Reference< XDocumentProperties > xDocProps = xPropSupplier->getDocumentProperties();

    if (xDocProps->getGenerator().startsWithIgnoreAsciiCase("Microsoft"))
    {
        ScCalcConfig aCalcConfig = mpDoc->GetCalcConfig();
        aCalcConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_XL_A1 ) ;
        mpDoc->SetCalcConfig(aCalcConfig);
    }

    mxDocImport.reset(new ScDocumentImport(*mpDoc));

    mxFormulaBuffer.reset( new FormulaBuffer( *this ) );
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

    // initialise edit engine
    ScDocument& rDoc = getScDocument();
    mxEditEngine.reset( new ScEditEngineDefaulter( rDoc.GetEnginePool() ) );
    mxEditEngine->SetRefMapMode( MAP_100TH_MM );
    mxEditEngine->SetEditTextObjectPool( rDoc.GetEditPool() );
    mxEditEngine->SetUpdateMode( false );
    mxEditEngine->EnableUndo( false );
    mxEditEngine->SetControlWord( mxEditEngine->GetControlWord() & ~EEControlBits::ALLOWBIGOBJS );

    // set some document properties needed during import
    if( mrBaseFilter.isImportFilter() )
    {
        // enable editing read-only documents (e.g. from read-only files)
        mpDoc->EnableChangeReadOnly(true);
        // #i76026# disable Undo while loading the document
        mpDoc->EnableUndo(false);
        // #i79826# disable calculating automatic row height while loading the document
        mpDoc->EnableAdjustHeight(false);
        // disable automatic update of linked sheets and DDE links
        mpDoc->EnableExecuteLink(false);

        mxProgressBar.reset( new SegmentProgressBar( mrBaseFilter.getStatusIndicator(), ScGlobal::GetRscString(STR_LOAD_DOC) ) );
        mxFmlaParser.reset( createFormulaParser() );

        //prevent unnecessary broadcasts and "half way listeners" as
        //is done in ScDocShell::BeforeXMLLoading() for ods
        mpDoc->SetInsertingFromOtherDoc(true);
    }
    else if( mrBaseFilter.isExportFilter() )
    {
        mxProgressBar.reset( new SegmentProgressBar( mrBaseFilter.getStatusIndicator(), ScGlobal::GetRscString(STR_SAVE_DOC) ) );
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
        // #i74668# do not insert default sheets
        mpDocShell->SetEmpty(false);
        // enable automatic update of linked sheets and DDE links
        mpDoc->EnableExecuteLink(true);
        // #i79826# enable updating automatic row height after loading the document
        mpDoc->EnableAdjustHeight(true);

        // #i76026# enable Undo after loading the document
        mpDoc->EnableUndo(true);

        // disable editing read-only documents (e.g. from read-only files)
        mpDoc->EnableChangeReadOnly(false);
        // #111099# open forms in alive mode (has no effect, if no controls in document)
        ScDrawLayer* pModel = mpDoc->GetDrawLayer();
        if (pModel)
            pModel->SetOpenInDesignMode(false);

    }
}


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

    // Import the VBA project (after finalizing workbook settings which
    // contains the workbook code name).  Do it before processing formulas in
    // order to correctly resolve VBA custom function names.
    StorageRef xVbaPrjStrg = mrBookGlob.getVbaProjectStorage();
    if( xVbaPrjStrg.get() && xVbaPrjStrg->isStorage() )
        getBaseFilter().getVbaProject().importModulesAndForms( *xVbaPrjStrg, getBaseFilter().getGraphicHelper() );

    // need to import formulas before scenarios
    mrBookGlob.getFormulaBuffer().finalizeImport();

    // Insert all pivot tables. Must be done after loading all sheets and
    // formulas, because data pilots expect existing source data on
    // creation.
    getPivotTables().finalizeImport();

    /*  Insert scenarios after all sheet processing is done, because new hidden
        sheets are created for scenarios which would confuse code that relies
        on certain sheet indexes. Must be done after pivot tables too. */
    mrBookGlob.getScenarios().finalizeImport();

    /*  Set 'Default' page style to automatic page numbering (default is manual
        number 1). Otherwise hidden sheets (e.g. for scenarios) which have
        'Default' page style will break automatic page numbering for following
        sheets. Automatic numbering is set by passing the value 0. */
    PropertySet aDefPageStyle( getStyleObject( "Default", true ) );
    aDefPageStyle.setProperty< sal_Int16 >( PROP_FirstPageNumber, 0 );

    // Has any string ref syntax been imported?
    // If not, we need to take action
    ScCalcConfig aCalcConfig = getScDocument().GetCalcConfig();

    if ( !aCalcConfig.mbHasStringRefSyntax )
    {
        aCalcConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_A1_XL_A1;
        getScDocument().SetCalcConfig(aCalcConfig);
    }
}

// document model -------------------------------------------------------------

ScDocument& WorkbookHelper::getScDocument()
{
    return mrBookGlob.getScDocument();
}

const ScDocument& WorkbookHelper::getScDocument() const
{
    return mrBookGlob.getScDocument();
}

ScDocumentImport& WorkbookHelper::getDocImport()
{
    return mrBookGlob.getDocImport();
}

const ScDocumentImport& WorkbookHelper::getDocImport() const
{
    return mrBookGlob.getDocImport();
}

ScEditEngineDefaulter& WorkbookHelper::getEditEngine() const
{
    return mrBookGlob.getEditEngine();
}

Reference< XSpreadsheetDocument > WorkbookHelper::getDocument() const
{
    return mrBookGlob.getDocument();
}

Reference< XSpreadsheet > WorkbookHelper::getSheetFromDoc( sal_Int32 nSheet ) const
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

Reference< XNameContainer > WorkbookHelper::getCellStyleFamily() const
{
    return mrBookGlob.getStyleFamily( false/*bPageStyles*/ );
}

Reference< XStyle > WorkbookHelper::getStyleObject( const OUString& rStyleName, bool bPageStyle ) const
{
    return mrBookGlob.getStyleObject( rStyleName, bPageStyle );
}

ScRangeData* WorkbookHelper::createNamedRangeObject( OUString& orName, const Sequence< FormulaToken>& rTokens, sal_Int32 nIndex, sal_Int32 nNameFlags ) const
{
    return mrBookGlob.createNamedRangeObject( orName, rTokens, nIndex, nNameFlags );
}

ScRangeData* WorkbookHelper::createLocalNamedRangeObject( OUString& orName, const Sequence< FormulaToken>& rTokens, sal_Int32 nIndex, sal_Int32 nNameFlags, sal_Int32 nTab ) const
{
    return mrBookGlob.createLocalNamedRangeObject( orName, rTokens, nIndex, nNameFlags, nTab );
}

Reference< XDatabaseRange > WorkbookHelper::createDatabaseRangeObject( OUString& orName, const CellRangeAddress& rRangeAddr ) const
{
    return mrBookGlob.createDatabaseRangeObject( orName, rRangeAddr );
}

Reference< XDatabaseRange > WorkbookHelper::createUnnamedDatabaseRangeObject( const CellRangeAddress& rRangeAddr ) const
{
    return mrBookGlob.createUnnamedDatabaseRangeObject( rRangeAddr );
}

ScDBData* WorkbookHelper::findDatabaseRangeByIndex( sal_uInt16 nIndex ) const
{
    return mrBookGlob.findDatabaseRangeByIndex( nIndex );
}

Reference< XStyle > WorkbookHelper::createStyleObject( OUString& orStyleName, bool bPageStyle ) const
{
    return mrBookGlob.createStyleObject( orStyleName, bPageStyle );
}

// buffers --------------------------------------------------------------------

FormulaBuffer& WorkbookHelper::getFormulaBuffer() const
{
    return mrBookGlob.getFormulaBuffer();
}

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

FormulaParser* WorkbookHelper::createFormulaParser() const
{
    return mrBookGlob.createFormulaParser();
}

UnitConverter& WorkbookHelper::getUnitConverter() const
{
    return mrBookGlob.getUnitConverter();
}

AddressConverter& WorkbookHelper::getAddressConverter() const
{
    return mrBookGlob.getAddressConverter();
}

oox::drawingml::chart::ChartConverter* WorkbookHelper::getChartConverter() const
{
    return mrBookGlob.getChartConverter();
}

void WorkbookHelper::useInternalChartDataTable( bool bInternal )
{
    mrBookGlob.useInternalChartDataTable( bInternal );
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

bool WorkbookHelper::importOoxFragment( const rtl::Reference<FragmentHandler>& rxHandler )
{
    return getOoxFilter().importFragment( rxHandler );
}

bool WorkbookHelper::importOoxFragment( const rtl::Reference<FragmentHandler>& rxHandler, oox::core::FastParser& rParser )
{
    return getOoxFilter().importFragment(rxHandler, rParser);
}

// BIFF specific --------------------------------------------------------------

BiffType WorkbookHelper::getBiff() const
{
    return mrBookGlob.getBiff();
}

rtl_TextEncoding WorkbookHelper::getTextEncoding() const
{
    return mrBookGlob.getTextEncoding();
}

BiffCodecHelper& WorkbookHelper::getCodecHelper() const
{
    return mrBookGlob.getCodecHelper();
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
