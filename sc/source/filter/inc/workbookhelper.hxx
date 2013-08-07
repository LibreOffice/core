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

#ifndef OOX_XLS_WORKBOOKHELPER_HXX
#define OOX_XLS_WORKBOOKHELPER_HXX

#include <boost/shared_ptr.hpp>
#include <rtl/ref.hxx>
#include "oox/helper/storagebase.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "biffhelper.hxx"
#include "rangenam.hxx"

namespace com { namespace sun { namespace star {
    namespace container { class XNameAccess; }
    namespace container { class XNameContainer; }
    namespace lang { class XMultiServiceFactory; }
    namespace sheet { class XDatabaseRange; }
    namespace sheet { class XNamedRange; }
    namespace sheet { class XSpreadsheet; }
    namespace sheet { class XSpreadsheetDocument; }
    namespace sheet { struct FormulaToken; }
    namespace style { class XStyle; }
    namespace table { struct CellAddress; }
    namespace table { struct CellRangeAddress; }
    namespace table { class XCell; }
    namespace table { class XCellRange; }
} } }

namespace oox {
    class AttributeList;
    class SegmentProgressBar;
    class ISegmentProgressBar;
}

namespace oox { namespace core {
    class FilterBase;
    class FragmentHandler;
    class XmlFilterBase;
} }

class ScDocument;
class ScEditEngineDefaulter;

namespace oox {
namespace xls {

class ExcelFilter;

// ============================================================================

/** An enumeration for all supported spreadsheet filter types. */
enum FilterType
{
    FILTER_OOXML,       /// MS Excel OOXML (Office Open XML) or BIFF12.
    FILTER_BIFF,        /// MS Excel BIFF2-BIFF8 (Binary Interchange File Format).
    FILTER_UNKNOWN      /// Unknown filter type.
};

// ============================================================================

/** Functor for case-insensitive string comparison, usable in maps etc. */
struct IgnoreCaseCompare
{
    bool operator()( const OUString& rName1, const OUString& rName2 ) const;
};

// ============================================================================

class AddressConverter;
class BiffCodecHelper;
class ConnectionsBuffer;
class DefinedNamesBuffer;
class ExcelChartConverter;
class ExternalLinkBuffer;
class FormulaParser;
class PageSettingsConverter;
class PivotCacheBuffer;
class PivotTableBuffer;
class ScenarioBuffer;
class SharedStringsBuffer;
class StylesBuffer;
class TableBuffer;
class ThemeBuffer;
class UnitConverter;
class ViewSettings;
class WorkbookSettings;
class WorksheetBuffer;
class FormulaBuffer;

class WorkbookGlobals;
typedef ::boost::shared_ptr< WorkbookGlobals > WorkbookGlobalsRef;

/** Helper class to provice access to global workbook data.

    All classes derived from this helper class will have access to a singleton
    object of type WorkbookGlobals containing global workbook settings,
    buffers, converters, etc. Nearly all classes in this filter implementation
    are derived directly or indirectly from this class.

    This class contains just a simple reference to the WorkbookGlobals object
    to prevent circular references, as the WorkbookGlobals object contains a
    lot of objects derived from this class.
 */
class WorkbookHelper
{
public:
    inline /*implicit*/ WorkbookHelper( WorkbookGlobals& rBookGlob ) : mrBookGlob( rBookGlob ) {}
    virtual             ~WorkbookHelper();

    static WorkbookGlobalsRef constructGlobals( ExcelFilter& rFilter );

    // filter -----------------------------------------------------------------

    /** Returns the base filter object (base class of all filters). */
    ::oox::core::FilterBase& getBaseFilter() const;
    /** Returns the file type of the current filter. */
    FilterType          getFilterType() const;
    /** Returns the filter progress bar. */
    SegmentProgressBar& getProgressBar() const;
    /** Returns true, if the file is a multi-sheet document, or false if single-sheet. */
    bool                isWorkbookFile() const;
    /** Returns the index of the current Calc sheet, if filter currently processes a sheet. */
    sal_Int16           getCurrentSheetIndex() const;

    /** Sets the VBA project storage used to import VBA source code and forms. */
    void                setVbaProjectStorage( const StorageRef& rxVbaPrjStrg );
    /** Sets the index of the current Calc sheet, if filter currently processes a sheet. */
    void                setCurrentSheetIndex( sal_Int16 nSheet );
    /** Final conversion after importing the workbook. */
    void                finalizeWorkbookImport();
    void                useInternalChartDataTable( bool bInternal );

    // document model ---------------------------------------------------------
    ScDocument& getScDocument() const;
    ScEditEngineDefaulter& getEditEngine() const;
    /** Returns a reference to the source/target spreadsheet document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                        getDocument() const;

    /** Returns a reference to the specified spreadsheet in the document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
                        getSheetFromDoc( sal_Int32 nSheet ) const;
    /** Returns a reference to the specified spreadsheet in the document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
                        getSheetFromDoc( const OUString& rSheet ) const;

    /** Returns the XCellRange interface for the passed cell range address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                        getCellRangeFromDoc(
                            const ::com::sun::star::table::CellRangeAddress& rRange ) const;

    /** Returns the cell or page styles container from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        getStyleFamily( bool bPageStyles ) const;
    /** Returns the specified cell or page style from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
                        getStyleObject( const OUString& rStyleName, bool bPageStyle ) const;

    /** Creates and returns a defined name on-the-fly in the Calc document.
        The name will not be buffered in the global defined names buffer.
        @param orName  (in/out-parameter) Returns the resulting used name. */
    ScRangeData* createNamedRangeObject(
                            OUString& orName,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken>& rTokens,
                            sal_Int32 nIndex,
                            sal_Int32 nNameFlags = 0 ) const;

    /** Creates and returns a defined name on-the-fly in the sheet.
        The name will not be buffered in the global defined names buffer.
        @param orName  (in/out-parameter) Returns the resulting used name. */
    ScRangeData* createLocalNamedRangeObject(
                            OUString& orName,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken>& rTokens,
                            sal_Int32 nIndex,
                            sal_Int32 nNameFlags = 0, sal_Int32 nTab = -1 ) const;

    /** Creates and returns a database range on-the-fly in the Calc document.
        The range will not be buffered in the global table buffer.
        @param orName  (in/out-parameter) Returns the resulting used name. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDatabaseRange >
                        createDatabaseRangeObject(
                            OUString& orName,
                            const ::com::sun::star::table::CellRangeAddress& rRangeAddr ) const;

    /** Creates and returns an unnamed database range on-the-fly in the Calc document.
        The range will not be buffered in the global table buffer. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDatabaseRange >
                        createUnnamedDatabaseRangeObject(
                            const ::com::sun::star::table::CellRangeAddress& rRangeAddr ) const;

    /** Creates and returns a com.sun.star.style.Style object for cells or pages. */
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
                        createStyleObject(
                            OUString& orStyleName,
                            bool bPageStyle ) const;

    // buffers ----------------------------------------------------------------

    FormulaBuffer&      getFormulaBuffer() const;
    /** Returns the global workbook settings object. */
    WorkbookSettings&   getWorkbookSettings() const;
    /** Returns the workbook and sheet view settings object. */
    ViewSettings&       getViewSettings() const;
    /** Returns the worksheet buffer containing sheet names and properties. */
    WorksheetBuffer&    getWorksheets() const;
    /** Returns the office theme object read from the theme substorage. */
    ThemeBuffer&        getTheme() const;
    /** Returns all cell formatting objects read from the styles substream. */
    StylesBuffer&       getStyles() const;
    /** Returns the shared strings read from the shared strings substream. */
    SharedStringsBuffer& getSharedStrings() const;
    /** Returns the external links read from the external links substream. */
    ExternalLinkBuffer& getExternalLinks() const;
    /** Returns the defined names read from the workbook globals. */
    DefinedNamesBuffer& getDefinedNames() const;
    /** Returns the tables collection (equivalent to Calc's database ranges). */
    TableBuffer&        getTables() const;
    /** Returns the scenarios collection. */
    ScenarioBuffer&     getScenarios() const;
    /** Returns the collection of external data connections. */
    ConnectionsBuffer&  getConnections() const;
    /** Returns the collection of pivot caches. */
    PivotCacheBuffer&   getPivotCaches() const;
    /** Returns the collection of pivot tables. */
    PivotTableBuffer&   getPivotTables() const;

    // converters -------------------------------------------------------------

    /** Returns the import formula parser (import filter only!). */
    FormulaParser&      getFormulaParser() const;
    /** Returns the measurement unit converter. */
    UnitConverter&      getUnitConverter() const;
    /** Returns the converter for string to cell address/range conversion. */
    AddressConverter&   getAddressConverter() const;
    /** Returns the chart object converter. */
    oox::drawingml::chart::ChartConverter* getChartConverter() const;
    /** Returns the page and print settings converter. */
    PageSettingsConverter& getPageSettingsConverter() const;

    // OOXML/BIFF12 specific (MUST NOT be called in BIFF filter) --------------

    /** Returns the base OOXML/BIFF12 filter object.
        Must not be called, if current filter is not the OOXML/BIFF12 filter. */
    ::oox::core::XmlFilterBase& getOoxFilter() const;

    /** Imports a fragment using the passed fragment handler, which contains
        the full path to the fragment stream. */
    bool                importOoxFragment( const ::rtl::Reference< ::oox::core::FragmentHandler >& rxHandler );

    // BIFF2-BIFF8 specific (MUST NOT be called in OOXML/BIFF12 filter) -------

    /** Returns the BIFF type in binary filter. */
    BiffType            getBiff() const;

    /** Returns the text encoding used to import/export byte strings. */
    rtl_TextEncoding    getTextEncoding() const;

    /** Returns the codec helper that stores the encoder/decoder object. */
    BiffCodecHelper&    getCodecHelper() const;

private:
    WorkbookGlobals&    mrBookGlob;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
