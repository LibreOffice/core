/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pivottablebuffer.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_XLS_PIVOTCACHEBUFFER_HXX
#define OOX_XLS_PIVOTCACHEBUFFER_HXX

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XDataPilotField; }
} } }

namespace oox { namespace core { class Relations; } }

namespace oox {
namespace xls {

class WorksheetHelper;

// ============================================================================

class PivotCacheItem
{
public:
    explicit            PivotCacheItem();

    /** Reads the string value from a pivot cache item. */
    void                readString( const AttributeList& rAttribs );
    /** Reads the double value from a pivot cache item. */
    void                readNumeric( const AttributeList& rAttribs );
    /** Reads the date/time value from a pivot cache item. */
    void                readDate( const AttributeList& rAttribs );
    /** Reads the boolean value from a pivot cache item. */
    void                readBool( const AttributeList& rAttribs );
    /** Reads the error code value from a pivot cache item. */
    void                readError( const AttributeList& rAttribs, const UnitConverter& rUnitConverter );
    /** Reads the index of a shared item. */
    void                readIndex( const AttributeList& rAttribs );

    /** Reads the string value from a pivot cache item. */
    void                readString( RecordInputStream& rStrm );
    /** Reads the double value from a pivot cache item. */
    void                readDouble( RecordInputStream& rStrm );
    /** Reads the date/time value from a pivot cache item. */
    void                readDate( RecordInputStream& rStrm );
    /** Reads the boolean value from a pivot cache item. */
    void                readBool( RecordInputStream& rStrm );
    /** Reads the error code value from a pivot cache item. */
    void                readError( RecordInputStream& rStrm );
    /** Reads the index of a shared item. */
    void                readIndex( RecordInputStream& rStrm );

    /** Reads the string value from a pivot cache item. */
    void                readString( BiffInputStream& rStrm, const WorkbookHelper& rHelper );
    /** Reads the double value from a pivot cache item. */
    void                readDouble( BiffInputStream& rStrm );
    /** Reads the integer value from a pivot cache item. */
    void                readInteger( BiffInputStream& rStrm );
    /** Reads the date/time value from a pivot cache item. */
    void                readDate( BiffInputStream& rStrm );
    /** Reads the boolean value from a pivot cache item. */
    void                readBool( BiffInputStream& rStrm );
    /** Reads the error code value from a pivot cache item. */
    void                readError( BiffInputStream& rStrm );

    /** Returns the type of the item. */
    inline sal_Int32    getType() const { return mnType; }
    /** Returns the value of the item. */
    inline const ::com::sun::star::uno::Any& getValue() const { return maValue; }
    /** Returns the string representation of the item. */
    ::rtl::OUString     getName() const;

private:
    ::com::sun::star::uno::Any maValue;     /// Value of the item.
    sal_Int32           mnType;             /// Value type (OOX token identifier).
};

// ----------------------------------------------------------------------------

class PivotCacheItemList : public WorkbookHelper
{
public:
    explicit            PivotCacheItemList( const WorkbookHelper& rHelper );

    /** Imports the item from the passed attribute list. */
    void                importItem( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Imports the item from the passed stream and record. */
    void                importItem( sal_Int32 nRecId, RecordInputStream& rStrm );
    /** Imports a complete item list from the passed stream. */
    void                importItemList( BiffInputStream& rStrm, sal_uInt16 nCount );

    /** Returns true, if this item list is empty. */
    inline bool         empty() const { return maItems.empty(); }
    /** Returns the size of the item list. */
    inline size_t       size() const { return maItems.size(); }

    /** Returns the specified item. */
    const PivotCacheItem* getCacheItem( sal_Int32 nItemIdx ) const;
    /** Returns the names of all items. */
    void                getCacheItemNames( ::std::vector< ::rtl::OUString >& orItemNames ) const;

private:
    /** Creates and returns a new item at the end of the items list. */
    PivotCacheItem&     createItem();
    /** Imports an array of items from the PCITEM_ARRAY record */
    void                importArray( RecordInputStream& rStrm );

private:
    typedef ::std::vector< PivotCacheItem > CacheItemVector;
    CacheItemVector     maItems;            /// All items of this list.
};

// ============================================================================

struct PCFieldModel
{
    ::rtl::OUString     maName;             /// Fixed name of the cache field.
    ::rtl::OUString     maCaption;          /// Caption of the ccahe field.
    ::rtl::OUString     maPropertyName;     /// OLAP property name.
    ::rtl::OUString     maFormula;          /// Formula of a calculated field.
    sal_Int32           mnNumFmtId;         /// Number format for all items.
    sal_Int32           mnSqlType;          /// Data type from ODBC data source.
    sal_Int32           mnHierarchy;        /// Hierarchy this field is part of.
    sal_Int32           mnLevel;            /// Hierarchy level this field is part of.
    sal_Int32           mnMappingCount;     /// Number of property mappings.
    bool                mbDatabaseField;    /// True = field from source data; false = calculated field.
    bool                mbServerField;      /// True = ODBC server-based page field.
    bool                mbUniqueList;       /// True = list of unique ODBC items exists.
    bool                mbMemberPropField;  /// True = contains OLAP member properties.

    explicit            PCFieldModel();
};

// ----------------------------------------------------------------------------

struct PCSharedItemsModel
{
    bool                mbHasSemiMixed;     /// True = has (blank|string|bool|error) item(s), maybe other types.
    bool                mbHasNonDate;       /// True = has non-date item(s), maybe date items.
    bool                mbHasDate;          /// True = has date item(s), maybe other types.
    bool                mbHasString;        /// True = has (string|bool|error) item(s), maybe other types.
    bool                mbHasBlank;         /// True = has blank item(s), maybe other types.
    bool                mbHasMixed;         /// True = has [(string|bool|error) and (number|date)] or (number and date).
    bool                mbIsNumeric;        /// True = has numeric item(s), maybe other types except date.
    bool                mbIsInteger;        /// True = has numeric item(s) with only integers, maybe other types except date.
    bool                mbHasLongText;      /// True = contains strings with >255 charascters.
    bool                mbHasLongIndexes;   /// True = indexes to shared items are 16-bit (BIFF only).

    explicit            PCSharedItemsModel();
};

// ----------------------------------------------------------------------------

struct PCFieldGroupModel
{
    ::com::sun::star::util::DateTime maStartDate;   /// Manual or calculated start date for range grouping.
    ::com::sun::star::util::DateTime maEndDate;     /// Manual or calculated end date for range grouping.
    double              mfStartValue;       /// Manual or calculated start value for range grouping.
    double              mfEndValue;         /// Manual or calculated end value for range grouping.
    double              mfInterval;         /// Interval for numeric range grouping.
    sal_Int32           mnParentField;      /// Index of cache field that contains item groups based on this field.
    sal_Int32           mnBaseField;        /// Index of cache field this grouped field is based on.
    sal_Int32           mnGroupBy;          /// Type of numeric or date range grouping.
    bool                mbRangeGroup;       /// True = items are grouped by numeric ranges or date ranges.
    bool                mbDateGroup;        /// True = items are grouped by date ranges or by item names.
    bool                mbAutoStart;        /// True = start value for range groups is calculated from source data.
    bool                mbAutoEnd;          /// True = end value for range groups is calculated from source data.

    explicit            PCFieldGroupModel();

    /** Sets the group-by value for BIFF/OOBIN import. */
    void                setBinGroupBy( sal_uInt8 nGroupBy );
};

// ----------------------------------------------------------------------------

/** Helper struct for mapping original item names from/to group item names. */
struct PivotCacheGroupItem
{
    ::rtl::OUString     maOrigName;
    ::rtl::OUString     maGroupName;

    inline explicit     PivotCacheGroupItem( const ::rtl::OUString& rItemName ) :
                            maOrigName( rItemName ), maGroupName( rItemName ) {}
};

typedef ::std::vector< PivotCacheGroupItem > PivotCacheGroupItemVector;

// ----------------------------------------------------------------------------

class PivotCacheField : public WorkbookHelper
{
public:
    explicit            PivotCacheField( const WorkbookHelper& rHelper, bool bIsDatabaseField );

    /** Imports pivot cache field settings from the cacheField element. */
    void                importCacheField( const AttributeList& rAttribs );
    /** Imports shared items settings from the sharedItems element. */
    void                importSharedItems( const AttributeList& rAttribs );
    /** Imports a shared item from the passed element. */
    void                importSharedItem( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Imports grouping settings from the fieldGroup element. */
    void                importFieldGroup( const AttributeList& rAttribs );
    /** Imports numeric grouping settings from the rangePr element. */
    void                importRangePr( const AttributeList& rAttribs );
    /** Imports an item of the mapping between group items and base items from the passed element. */
    void                importDiscretePrItem( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Imports a group item from the passed element. */
    void                importGroupItem( sal_Int32 nElement, const AttributeList& rAttribs );

    /** Imports pivot cache field settings from the PCDFIELD record. */
    void                importPCDField( RecordInputStream& rStrm );
    /** Imports shared items settings from the PCDFSHAREDITEMS record. */
    void                importPCDFSharedItems( RecordInputStream& rStrm );
    /** Imports one or more shared items from the passed record. */
    void                importPCDFSharedItem( sal_Int32 nRecId, RecordInputStream& rStrm );
    /** Imports grouping settings from the PCDFIELDGROUP record. */
    void                importPCDFieldGroup( RecordInputStream& rStrm );
    /** Imports numeric grouping settings from the PCDFRANGEPR record. */
    void                importPCDFRangePr( RecordInputStream& rStrm );
    /** Imports an item of the mapping between group items and base items from the passed record. */
    void                importPCDFDiscretePrItem( sal_Int32 nRecId, RecordInputStream& rStrm );
    /** Imports one or more group items from the passed record. */
    void                importPCDFGroupItem( sal_Int32 nRecId, RecordInputStream& rStrm );

    /** Imports pivot cache field settings from the PCDFIELD record. */
    void                importPCDField( BiffInputStream& rStrm );
    /** Imports numeric grouping settings from the PCDFRANGEPR record. */
    void                importPCDFRangePr( BiffInputStream& rStrm );
    /** Imports the mapping between group items and base items from the PCDFDISCRETEPR record. */
    void                importPCDFDiscretePr( BiffInputStream& rStrm );

    /** Returns true, if the field is based on source data, or false if it is grouped or calculated. */
    inline bool         isDatabaseField() const { return maFieldModel.mbDatabaseField; }

    /** Returns true, if the field contains a list of shared items. */
    inline bool         hasSharedItems() const { return !maSharedItems.empty(); }
    /** Returns true, if the field contains a list of grouping items. */
    inline bool         hasGroupItems() const { return !maGroupItems.empty(); }
    /** Returns true, if the field has inplace numeric grouping settings. */
    inline bool         hasNumericGrouping() const { return maFieldGroupModel.mbRangeGroup && !maFieldGroupModel.mbDateGroup; }
    /** Returns true, if the field has inplace date grouping settings. */
    inline bool         hasDateGrouping() const { return maFieldGroupModel.mbRangeGroup && maFieldGroupModel.mbDateGroup; }
    /** Returns true, if the field has a parent group field that groups the items of this field. */
    inline bool         hasParentGrouping() const { return maFieldGroupModel.mnParentField >= 0; }

    /** Returns the name of the cache field. */
    inline const ::rtl::OUString& getName() const { return maFieldModel.maName; }
    /** Returns the index of the parent group field that groups the items of this field. */
    inline sal_Int32    getParentGroupField() const { return maFieldGroupModel.mnParentField; }
    /** Returns the index of the base field grouping is based on. */
    inline sal_Int32    getGroupBaseField() const { return maFieldGroupModel.mnBaseField; }

    /** Returns the shared or group item with the specified index. */
    const PivotCacheItem* getCacheItem( sal_Int32 nItemIdx ) const;
    /** Returns the names of all shared or group items. */
    void                getCacheItemNames( ::std::vector< ::rtl::OUString >& orItemNames ) const;

    /** Creates inplace numeric grouping settings. */
    void                convertNumericGrouping(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >& rxDPField ) const;
    /** Creates inplace date grouping settings or a new date group field. */
    ::rtl::OUString     createDateGroupField(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >& rxBaseDPField ) const;
    /** Creates a new grouped DataPilot field and returns its name. */
    ::rtl::OUString     createParentGroupField(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >& rxBaseDPField,
                            PivotCacheGroupItemVector& orItemNames ) const;

    /** Writes the title of the field into the passed sheet at the passed address. */
    void                writeSourceHeaderCell( WorksheetHelper& rSheetHelper,
                            sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Writes a source field item value into the passed sheet. */
    void                writeSourceDataCell( WorksheetHelper& rSheetHelper,
                            sal_Int32 nCol, sal_Int32 nRow,
                            const PivotCacheItem& rItem ) const;

    /** Reads an item from the PCRECORD record and writes it to the passed sheet. */
    void                importPCRecordItem( RecordInputStream& rStrm,
                            WorksheetHelper& rSheetHelper, sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Reads an item index from the PCITEM_INDEXLIST record and writes the item to the passed sheet. */
    void                importPCItemIndex( BiffInputStream& rStrm,
                            WorksheetHelper& rSheetHelper, sal_Int32 nCol, sal_Int32 nRow ) const;


private:
    /** Tries to write the passed value to the passed sheet position. */
    void                writeItemToSourceDataCell( WorksheetHelper& rSheetHelper,
                            sal_Int32 nCol, sal_Int32 nRow, const PivotCacheItem& rItem ) const;
    /** Tries to write the value of a shared item to the passed sheet position. */
    void                writeSharedItemToSourceDataCell( WorksheetHelper& rSheetHelper,
                            sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nItemIdx ) const;

private:
    typedef ::std::vector< sal_Int32 > IndexVector;

    PivotCacheItemList  maSharedItems;      /// All shared items of this field.
    PivotCacheItemList  maGroupItems;       /// All group items of this field.
    IndexVector         maDiscreteItems;    /// Mapping between group and base items.
    PCFieldModel        maFieldModel;       /// Settings for this cache field.
    PCSharedItemsModel  maSharedItemsModel; /// Settings for shared items.
    PCFieldGroupModel   maFieldGroupModel;  /// Settings for item grouping.
};

// ============================================================================

struct PCDefinitionModel
{
    ::rtl::OUString     maRelId;            /// Relation identifier for cache records fragment.
    ::rtl::OUString     maRefreshedBy;      /// Name of user who last refreshed the cache.
    double              mfRefreshedDate;    /// Date/time of last refresh.
    sal_Int32           mnRecords;          /// Number of data records in the cache.
    sal_Int32           mnMissItemsLimit;   /// Limit for discarding unused items.
    sal_uInt16          mnDatabaseFields;   /// Number of database (source data) fields (BIFF only).
    bool                mbInvalid;          /// True = cache needs refresh.
    bool                mbSaveData;         /// True = cached item values are present.
    bool                mbRefreshOnLoad;    /// True = try to refresh cache on load.
    bool                mbOptimizeMemory;   /// True = application may optimize memory usage.
    bool                mbEnableRefresh;    /// True = refreshing cache is enabled in UI.
    bool                mbBackgroundQuery;  /// True = application queries data asynchonously.
    bool                mbUpgradeOnRefresh; /// True = application may upgrade cache version.
    bool                mbTupleCache;       /// True = cache stores OLAP functions.
    bool                mbSupportSubquery;  /// True = data source supports subqueries.
    bool                mbSupportDrill;     /// True = data source supports drilldown.

    explicit            PCDefinitionModel();
};

// ----------------------------------------------------------------------------

struct PCSourceModel
{
    sal_Int32           mnSourceType;       /// Type of the source data (sheet, consolidation, scenario, external).
    sal_Int32           mnConnectionId;     /// Connection identifier for external data source.

    explicit            PCSourceModel();
};

// ----------------------------------------------------------------------------

struct PCWorksheetSourceModel
{
    ::rtl::OUString     maRelId;            /// Relation identifier for an external document URL.
    ::rtl::OUString     maSheet;            /// Sheet name for cell range or sheet-local defined names.
    ::rtl::OUString     maDefName;          /// Defined name containing a cell range if present.
    ::com::sun::star::table::CellRangeAddress
                        maRange;            /// Source cell range of the data.

    explicit            PCWorksheetSourceModel();
};

// ----------------------------------------------------------------------------

class PivotCache : public WorkbookHelper
{
public:
    explicit            PivotCache( const WorkbookHelper& rHelper );

    /** Reads pivot cache global settings from the pivotCacheDefinition element. */
    void                importPivotCacheDefinition( const AttributeList& rAttribs );
    /** Reads cache source settings from the cacheSource element. */
    void                importCacheSource( const AttributeList& rAttribs );
    /** Reads sheet source settings from the worksheetSource element. */
    void                importWorksheetSource( const AttributeList& rAttribs, const ::oox::core::Relations& rRelations );

    /** Reads pivot cache global settings from the PCDEFINITION record. */
    void                importPCDefinition( RecordInputStream& rStrm );
    /** Reads cache source settings from the PCDSOURCE record. */
    void                importPCDSource( RecordInputStream& rStrm );
    /** Reads sheet source settings from the PCDSHEETSOURCE record. */
    void                importPCDSheetSource( RecordInputStream& rStrm, const ::oox::core::Relations& rRelations );

    /** Reads cache source settings from the PCDSOURCE record. */
    void                importPCDSource( BiffInputStream& rStrm );
    /** Reads pivot cache global settings from the PCDEFINITION record. */
    void                importPCDefinition( BiffInputStream& rStrm );

    /** Creates and returns a new pivot cache field. */
    PivotCacheField&    createCacheField( bool bInitDatabaseField = false );
    /** Checks validity of source data and creates a dummy data sheet for external sheet sources. */
    void                finalizeImport();

    /** Returns true, if the pivot cache is based on a valid data source, so
        that pivot tables can be created based on this pivot cache. */
    inline bool         isValidDataSource() const { return mbValidSource; }
    /** Returns true, if the pivot cache is based on a dummy sheet created in finalizeImport. */
    inline bool         isBasedOnDummySheet() const { return mbDummySheet; }
    /** Returns the internal cell range the cache is based on. */
    inline const ::com::sun::star::table::CellRangeAddress&
                        getSourceRange() const { return maSheetSrcModel.maRange; }
    /** Returns the relation identifier of the pivot cache records fragment. */
    inline const ::rtl::OUString& getRecordsRelId() const { return maDefModel.maRelId; }

    /** Returns the number of pivot cache fields. */
    sal_Int32           getCacheFieldCount() const;
    /** Returns the cache field with the specified index. */
    const PivotCacheField* getCacheField( sal_Int32 nFieldIdx ) const;
    /** Returns the source column index of the field with the passed index. */
    sal_Int32           getCacheDatabaseIndex( sal_Int32 nFieldIdx ) const;

    /** Writes the titles of all source fields into the passed sheet. */
    void                writeSourceHeaderCells( WorksheetHelper& rSheetHelper ) const;
    /** Writes a source field item value into the passed sheet. */
    void                writeSourceDataCell( WorksheetHelper& rSheetHelper,
                            sal_Int32 nCol, sal_Int32 nRow,
                            const PivotCacheItem& rItem ) const;

    /** Reads a PCRECORD record and writes all item values to the passed sheet. */
    void                importPCRecord( RecordInputStream& rStrm,
                            WorksheetHelper& rSheetHelper, sal_Int32 nRow ) const;
    /** Reads a PCITEM_INDEXLIST record and writes all item values to the passed sheet. */
    void                importPCItemIndexList( BiffInputStream& rStrm,
                            WorksheetHelper& rSheetHelper, sal_Int32 nRow ) const;

private:
    /** Reads the worksheet source range from the DCONREF record. */
    void                importDConRef( BiffInputStream& rStrm );
    /** Reads the defined name used for source data from the DCONNAME record. */
    void                importDConName( BiffInputStream& rStrm );
    /** Reads the built-in defined name used for source data from the DCONBINAME record. */
    void                importDConBIName( BiffInputStream& rStrm );
    /** Reads the sheet name and URL from the DCONREF, DCONNAME, or DCONBINAME records. */
    void                importDConUrl( BiffInputStream& rStrm );

    /** Finalizes the pivot cache if it is based on internal sheet data. */
    void                finalizeInternalSheetSource();
    /** Finalizes the pivot cache if it is based on sheet data of an external spreadsheet document. */
    void                finalizeExternalSheetSource();
    /** Creates a dummy sheet that will be filled with the pivot cache data. */
    void                prepareSourceDataSheet();

private:
    typedef RefVector< PivotCacheField >    PivotCacheFieldVector;
    typedef ::std::vector< sal_Int32 >      IndexVector;

    PivotCacheFieldVector maFields;         /// All pivot cache fields.
    PivotCacheFieldVector maDatabaseFields; /// All cache fields that are based on source data.
    IndexVector         maDatabaseIndexes;  /// Database field index for all fields.
    PCDefinitionModel   maDefModel;         /// Global pivot cache settings.
    PCSourceModel       maSourceModel;      /// Pivot cache source settings.
    PCWorksheetSourceModel maSheetSrcModel; /// Sheet source data if cache type is sheet.
    ::rtl::OUString     maTargetUrl;        /// URL of an external source document.
    bool                mbValidSource;      /// True = pivot cache is based on supported data source.
    bool                mbDummySheet;       /// True = pivot cache is based on a dummy sheet.
};

// ============================================================================

class PivotCacheBuffer : public WorkbookHelper
{
public:
    explicit            PivotCacheBuffer( const WorkbookHelper& rHelper );

    /** Registers a pivot cache definition fragment. The fragment will be loaded on demand (OOX/OOBIN only). */
    void                registerPivotCacheFragment( sal_Int32 nCacheId, const ::rtl::OUString& rFragmentPath );
    /** Reads the reference to a pivot cache stream. The stream will be loaded on demand (BIFF only). */
    void                importPivotCacheRef( BiffInputStream& rStrm );

    /** Imports and stores a pivot cache definition fragment on first call,
        returns the imported cache on subsequent calls with the same identifier. */
    PivotCache*         importPivotCacheFragment( sal_Int32 nCacheId );

private:
    /** Creates and returns a new pivot cache object with the passed identifier. */
    PivotCache&         createPivotCache( sal_Int32 nCacheId );

private:
    typedef ::std::map< sal_Int32, ::rtl::OUString >    FragmentPathMap;
    typedef RefMap< sal_Int32, PivotCache >             PivotCacheMap;
    typedef ::std::vector< sal_Int32 >                  PivotCacheIdVector;

    FragmentPathMap     maFragmentPaths;
    PivotCacheMap       maCaches;
    PivotCacheIdVector  maCacheIds;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

