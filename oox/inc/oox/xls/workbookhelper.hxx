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

#ifndef OOX_XLS_WORKBOOKHELPER_HXX
#define OOX_XLS_WORKBOOKHELPER_HXX

#include <boost/shared_ptr.hpp>
#include <rtl/ref.hxx>
#include "oox/helper/storagebase.hxx"
#include "oox/xls/biffhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace container { class XNameAccess; }
    namespace container { class XNameContainer; }
    namespace lang { class XMultiServiceFactory; }
    namespace awt { class XDevice; }
    namespace table { struct CellAddress; }
    namespace table { struct CellRangeAddress; }
    namespace table { class XCell; }
    namespace table { class XCellRange; }
    namespace sheet { class XSpreadsheetDocument; }
    namespace sheet { class XSpreadsheet; }
    namespace sheet { class XNamedRanges; }
    namespace sheet { class XNamedRange; }
    namespace sheet { class XDatabaseRanges; }
    namespace sheet { class XExternalDocLinks; }
    namespace style { class XStyle; }
} } }

namespace oox {
    class AttributeList;
    class SegmentProgressBar;
    class RecordInputStream;
}

namespace oox { namespace core {
    class BinaryFilterBase;
    class FilterBase;
    class FragmentHandler;
    class XmlFilterBase;
} }

namespace oox { namespace drawingml {
    class Theme;
} }

namespace oox {
namespace xls {

class ExcelFilter;
class ExcelBiffFilter;

// ============================================================================

/** An enumeration for all supported spreadsheet filter types. */
enum FilterType
{
    FILTER_OOX,         /// MS Excel OOXML (Office Open XML) or OOBIN.
    FILTER_BIFF,        /// MS Excel BIFF (Binary Interchange File Format).
    FILTER_UNKNOWN      /// Unknown filter type.
};

// ============================================================================

/** Functor for case-insensitive string comparison, usable in maps etc. */
struct IgnoreCaseCompare
{
    bool                operator()( const ::rtl::OUString& rName1, const ::rtl::OUString& rName2 ) const;
};

// ============================================================================

class WorkbookData;
class WorkbookSettings;
class ViewSettings;
class WorksheetBuffer;
class ThemeBuffer;
class StylesBuffer;
class SharedStringsBuffer;
class ExternalLinkBuffer;
class DefinedNamesBuffer;
class TableBuffer;
class ScenarioBuffer;
class WebQueryBuffer;
class PivotCacheBuffer;
class PivotTableBuffer;
class FormulaParser;
class UnitConverter;
class AddressConverter;
class ExcelChartConverter;
class PageSettingsConverter;
class BiffCodecHelper;

/** Helper class to provice access to global workbook data.

    All classes derived from this helper class will have access to a singleton
    object of type WorkbookData containing global workbook settings, buffers,
    converters, etc. Nearly all classes in this filter implementation are
    derived directly or indirectly from this class.
    
    This class contains just a simple reference to the WorkbookData object to
    prevent circular references, as the WorkbookData object contains a lot of
    objects derived from this class.
 */
class WorkbookHelper
{
public:
    inline /*implicit*/ WorkbookHelper( WorkbookData& rBookData ) : mrBookData( rBookData ) {}
    virtual             ~WorkbookHelper();

    // filter -----------------------------------------------------------------

    /** Returns the base filter object (base class of all filters). */
    ::oox::core::FilterBase& getBaseFilter() const;
    /** Returns a reference to the global service factory. */
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        getGlobalFactory() const;
    /** Returns the file type of the current filter. */
    FilterType          getFilterType() const;
    /** Returns the filter progress bar. */
    SegmentProgressBar& getProgressBar() const;
    /** Returns true, if the file is a multi-sheet document, or false if single-sheet. */
    bool                isWorkbookFile() const;
    /** Returns the index of the current sheet in the Calc document. */
    sal_Int16           getCurrentSheetIndex() const;

    /** Sets the index of the current sheet in the Calc document. */
    void                setCurrentSheetIndex( sal_Int16 nSheet );
    /** Sets the VBA project storage. */
    void                setVbaProjectStorage( const StorageRef& rxVbaPrjStrg );
    /** Final conversion after importing the workbook. */
    void                finalizeWorkbookImport();

    // document model ---------------------------------------------------------

    /** Returns a reference to the source/target spreadsheet document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                        getDocument() const;
    /** Returns a reference to the service factory of the spreadsheet document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        getDocumentFactory() const;
    /** Returns the reference device of the document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >
                        getReferenceDevice() const;
    /** Returns the container for defined names from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XNamedRanges >
                        getNamedRanges() const;
    /** Returns the container for database ranges from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDatabaseRanges >
                        getDatabaseRanges() const;
    /** Returns the container for external documents from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalDocLinks >
                        getExternalDocLinks() const;
    /** Returns the container for DDE links from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        getDdeLinks() const;

    /** Returns a reference to the specified spreadsheet in the document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
                        getSheetFromDoc( sal_Int16 nSheet ) const;
    /** Returns a reference to the specified spreadsheet in the document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
                        getSheetFromDoc( const ::rtl::OUString& rSheet ) const;

    /** Returns the XCell interface for the passed cell address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                        getCellFromDoc(
                            const ::com::sun::star::table::CellAddress& rAddress ) const;
    /** Returns the XCellRange interface for the passed cell range address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                        getCellRangeFromDoc(
                            const ::com::sun::star::table::CellRangeAddress& rRange ) const;

    /** Returns the cell or page styles container from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        getStyleFamily( bool bPageStyles ) const;
    /** Returns the specified cell or page style from the Calc document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
                        getStyleObject( const ::rtl::OUString& rStyleName, bool bPageStyle ) const;

    /** Creates and returns a defined name on-the-fly in the Calc document.
        The name will not be buffered in this defined names buffer.
        @param orName  (in/out-parameter) Returns the resulting used name. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XNamedRange >
                        createNamedRangeObject(
                            ::rtl::OUString& orName,
                            sal_Int32 nNameFlags = 0 ) const;
    /** Creates and returns a com.sun.star.style.Style object for cells or pages. */
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >
                        createStyleObject(
                            ::rtl::OUString& orStyleName,
                            bool bPageStyle ) const;

    // buffers ----------------------------------------------------------------

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
    /** Returns the web queries. */
    WebQueryBuffer&     getWebQueries() const;
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
    ExcelChartConverter& getChartConverter() const;
    /** Returns the page and print settings converter. */
    PageSettingsConverter& getPageSettingsConverter() const;

    // OOX specific (MUST NOT be called in BIFF filter) -----------------------

    /** Returns the base OOX filter object.
        Must not be called, if current filter is not the OOX filter. */
    ::oox::core::XmlFilterBase& getOoxFilter() const;

    /** Imports a fragment using the passed fragment handler, which contains
        the full path to the fragment stream. */
    bool                importOoxFragment( const ::rtl::Reference< ::oox::core::FragmentHandler >& rxHandler );

    // BIFF specific (MUST NOT be called in OOX filter) -----------------------

    /** Returns the base BIFF filter object. */
    ::oox::core::BinaryFilterBase& getBiffFilter() const;
    /** Returns the BIFF type in binary filter. */
    BiffType            getBiff() const;

    /** Returns the text encoding used to import/export byte strings. */
    rtl_TextEncoding    getTextEncoding() const;
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
    BiffCodecHelper&    getCodecHelper() const;

private:
    WorkbookData&       mrBookData;
};

// ============================================================================

namespace prv {

typedef ::boost::shared_ptr< WorkbookData > WorkbookDataRef;

struct WorkbookDataOwner
{
    explicit            WorkbookDataOwner( WorkbookDataRef xBookData );
    virtual             ~WorkbookDataOwner();
    WorkbookDataRef     mxBookData;
};

} // namespace prv

// ----------------------------------------------------------------------------

class WorkbookHelperRoot : private prv::WorkbookDataOwner, public WorkbookHelper
{
public:
    explicit            WorkbookHelperRoot( ExcelFilter& rFilter );
    explicit            WorkbookHelperRoot( ExcelBiffFilter& rFilter, BiffType eBiff );

    /** Returns true, if this helper refers to a valid document. */
    bool                isValid() const;

private:
                        WorkbookHelperRoot( const WorkbookHelperRoot& );
    WorkbookHelperRoot& operator=( const WorkbookHelperRoot& );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
