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

#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

#include <o3tl/hash_combine.hxx>
#include <oox/helper/storagebase.hxx>
#include <address.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>

namespace oox::drawingml::chart { class ChartConverter; }
namespace rtl { template <class reference_type> class Reference; }

namespace com::sun::star {
    namespace container { class XNameContainer; }
    namespace sheet { class XDatabaseRange; }
    namespace sheet { class XSpreadsheet; }
    namespace style { class XStyle; }
    namespace table { class XCellRange; }
}

namespace oox {
    class SegmentProgressBar;
}

namespace oox::core {
    class FilterBase;
    class FragmentHandler;
    class XmlFilterBase;
    class FastParser;
}

class ScDocument;
class ScDocumentImport;
class ScEditEngineDefaulter;
class ScDBData;
class ScRangeData;
class ScModelObj;
class ScDatabaseRangeObj;
class ScTableSheetObj;

namespace oox::xls {

enum class FontClassification : sal_uInt8
{
    None                = 0x0000,
    Asian               = 0x0001,
    Cmplx               = 0x0002,
    Latin               = 0x0004
};

}

namespace o3tl {
    template<> struct typed_flags<oox::xls::FontClassification> : is_typed_flags<oox::xls::FontClassification, 0x07> {};
}

namespace oox::xls {

class ExcelFilter;

/** Functor for case-insensitive string comparison, usable in maps etc. */
struct IgnoreCaseCompare
{
    bool operator()( std::u16string_view rName1, std::u16string_view rName2 ) const;
};

class AddressConverter;
class ConnectionsBuffer;
class DefinedNamesBuffer;
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
typedef std::shared_ptr< WorkbookGlobals > WorkbookGlobalsRef;

struct FontDescriptorHash
{
    size_t operator()( const css::awt::FontDescriptor& rKey) const
    {
        std::size_t seed = rKey.Name.hashCode();
        o3tl::hash_combine(seed, rKey.Height);
        o3tl::hash_combine(seed, rKey.Width);
        o3tl::hash_combine(seed, rKey.StyleName.hashCode());
        o3tl::hash_combine(seed, rKey.Family);
        o3tl::hash_combine(seed, rKey.CharSet);
        o3tl::hash_combine(seed, rKey.Pitch);
        o3tl::hash_combine(seed, rKey.CharacterWidth);
        o3tl::hash_combine(seed, rKey.Weight);
        o3tl::hash_combine(seed, rKey.Slant);
        o3tl::hash_combine(seed, rKey.Underline);
        o3tl::hash_combine(seed, rKey.Strikeout);
        o3tl::hash_combine(seed, rKey.Orientation);
        o3tl::hash_combine(seed, rKey.Kerning);
        o3tl::hash_combine(seed, rKey.WordLineMode);
        o3tl::hash_combine(seed, rKey.Type);
        return seed;
    }
};

typedef std::unordered_map<css::awt::FontDescriptor, FontClassification, FontDescriptorHash> FontClassificationMap;

/** Helper class to provide access to global workbook data.

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
    /*implicit*/ WorkbookHelper( WorkbookGlobals& rBookGlob ) : mrBookGlob( rBookGlob ) {}
    virtual             ~WorkbookHelper();

    WorkbookHelper(WorkbookHelper const &) = default;
    WorkbookHelper(WorkbookHelper &&) = default;
    WorkbookHelper & operator =(WorkbookHelper const &) = delete; // due to mrBookGlob
    WorkbookHelper & operator =(WorkbookHelper &&) = delete; // due to mrBookGlob

    static WorkbookGlobalsRef constructGlobals( ExcelFilter& rFilter );

    // filter -----------------------------------------------------------------

    /** Returns the base filter object (base class of all filters). */
    ::oox::core::FilterBase& getBaseFilter() const;
    /** Returns the filter progress bar. */
    SegmentProgressBar& getProgressBar() const;
    /** Returns the index of the current Calc sheet, if filter currently processes a sheet. */
    sal_Int16           getCurrentSheetIndex() const;
    /** Returns true when reading a file generated by a known good generator. */
    bool                isGeneratorKnownGood() const;
    /** Returns true if any formula cell is calculated. */
    bool                hasCalculatedFormulaCells() const;
    /** Set if any formula cell is calculated. */
    void                setCalculatedFormulaCells();

    /** Sets the VBA project storage used to import VBA source code and forms. */
    void                setVbaProjectStorage( const StorageRef& rxVbaPrjStrg );
    /** Sets the index of the current Calc sheet, if filter currently processes a sheet. */
    void                setCurrentSheetIndex( SCTAB nSheet );
    /** Final conversion after importing the workbook. */
    void                finalizeWorkbookImport();
    void                useInternalChartDataTable( bool bInternal );

    // document model ---------------------------------------------------------
    ScDocument& getScDocument();
    const ScDocument& getScDocument() const;

    ScDocumentImport& getDocImport();
    const ScDocumentImport& getDocImport() const;

    ScEditEngineDefaulter& getEditEngine() const;
    /** Returns a reference to the source/target spreadsheet document model (XSpreadsheetDocument). */
    const rtl::Reference< ScModelObj >& getDocument() const;

    /** Returns a reference to the specified spreadsheet in the document model. */
    rtl::Reference< ScTableSheetObj >
                        getSheetFromDoc( sal_Int32 nSheet ) const;
    /** Returns a reference to the specified spreadsheet in the document model. */
    css::uno::Reference< css::sheet::XSpreadsheet >
                        getSheetFromDoc( const OUString& rSheet ) const;

    /** Returns the XCellRange interface for the passed cell range address. */
    css::uno::Reference< css::table::XCellRange >
                        getCellRangeFromDoc( const ScRange& rRange ) const;

    /** Returns the cell styles container from the Calc document. */
    css::uno::Reference< css::container::XNameContainer >
                        getCellStyleFamily() const;
    /** Returns the specified cell or page style from the Calc document. */
    css::uno::Reference< css::style::XStyle >
                        getStyleObject( const OUString& rStyleName, bool bPageStyle ) const;

    // second is true if ownership belongs to the caller
    typedef std::pair<ScRangeData*, bool> RangeDataRet;

    /** Creates and returns a defined name on-the-fly in the Calc document.
        The name will not be buffered in the global defined names buffer.
        @param orName  (in/out-parameter) Returns the resulting used name. */
    RangeDataRet createNamedRangeObject(
                            OUString& orName,
                            sal_Int32 nIndex,
                            sal_Int32 nNameFlags ) const;

    /** Creates and returns a defined name on-the-fly in the sheet.
        The name will not be buffered in the global defined names buffer.
        @param orName  (in/out-parameter) Returns the resulting used name. */
    RangeDataRet createLocalNamedRangeObject(
                            OUString& orName,
                            sal_Int32 nIndex,
                            sal_Int32 nNameFlags, sal_Int32 nTab ) const;

    /** Creates and returns a database range on-the-fly in the Calc document.
        The range will not be buffered in the global table buffer.
        @param orName  (in/out-parameter) Returns the resulting used name. */
    rtl::Reference<ScDatabaseRangeObj> createDatabaseRangeObject(
                            OUString& orName,
                            const ScRange& rRangeAddr ) const;

    /** Creates and returns an unnamed database range on-the-fly in the Calc document.
        The range will not be buffered in the global table buffer. */
    css::uno::Reference< css::sheet::XDatabaseRange >
                        createUnnamedDatabaseRangeObject(
                            const ScRange& rRangeAddr ) const;

    /** Finds the (already existing) database range of the given formula token index. */
    ScDBData* findDatabaseRangeByIndex( sal_uInt16 nIndex ) const;

    /** Creates and returns a com.sun.star.style.Style object for cells or pages. */
    css::uno::Reference< css::style::XStyle >
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
    /** Shared cache of Font Classifications to avoid repeated lookups */
    FontClassificationMap& getFontClassificationCache() const;

    // converters -------------------------------------------------------------

    /** Returns a shared import formula parser (import filter only!). */
    FormulaParser&      getFormulaParser() const;
    /** Returns an unshared import formula parser (import filter only!). */
    FormulaParser*      createFormulaParser() const;
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
    bool importOoxFragment( const rtl::Reference<oox::core::FragmentHandler>& rxHandler );

    bool importOoxFragment( const rtl::Reference<oox::core::FragmentHandler>& rxHandler, oox::core::FastParser& rParser );

    // BIFF2-BIFF8 specific (MUST NOT be called in OOXML/BIFF12 filter) -------

    /** Returns the text encoding used to import/export byte strings. */
    rtl_TextEncoding    getTextEncoding() const;

private:
    WorkbookGlobals&    mrBookGlob;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
