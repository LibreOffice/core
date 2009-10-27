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

#ifndef OOX_XLS_PIVOTTABLEBUFFER_HXX
#define OOX_XLS_PIVOTTABLEBUFFER_HXX

#include <com/sun/star/table/CellRangeAddress.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/xls/pivotcachebuffer.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XDataPilotDescriptor; }
    namespace sheet { class XDataPilotField; }
} } }

namespace oox {
namespace xls {

class PivotTable;

// ============================================================================

struct PTFieldItemModel
{
    sal_Int32           mnCacheItem;        /// Index to shared item in pivot cache.
    sal_Int32           mnType;             /// Type of the item.
    bool                mbShowDetails;      /// True = show item details (items of child fields).
    bool                mbHidden;           /// True = item is hidden.

    explicit            PTFieldItemModel();

    /** Sets item type for BIFF/OOBIN import. */
    void                setBinType( sal_uInt16 nType );
};

// ----------------------------------------------------------------------------

struct PTFieldModel
{
    sal_Int32           mnAxis;             /// Axis this field is assigned to (none, row, column, page).
    sal_Int32           mnNumFmtId;         /// Number format for field items.
    sal_Int32           mnAutoShowItems;    /// Number of items (or percent/sum) to be shown in auto show filter.
    sal_Int32           mnAutoShowRankBy;   /// Index of the data field auto show filter is based on.
    sal_Int32           mnSortType;         /// Autosorting type.
    sal_Int32           mnSortRefField;     /// Reference field for autosorting.
    sal_Int32           mnSortRefItem;      /// Item in reference field for autosorting.
    bool                mbDataField;        /// True = field appears in data area.
    bool                mbDefaultSubtotal;  /// True = show default subtotals.
    bool                mbSumSubtotal;      /// True = show sum subtotals.
    bool                mbCountASubtotal;   /// True = show count all subtotals.
    bool                mbAverageSubtotal;  /// True = show average subtotals.
    bool                mbMaxSubtotal;      /// True = show maximum subtotals.
    bool                mbMinSubtotal;      /// True = show minimum subtotals.
    bool                mbProductSubtotal;  /// True = show product subtotals.
    bool                mbCountSubtotal;    /// True = show count numbers subtotals.
    bool                mbStdDevSubtotal;   /// True = show standard deviation subtotals.
    bool                mbStdDevPSubtotal;  /// True = show standard deviation of population subtotals.
    bool                mbVarSubtotal;      /// True = show variance subtotals.
    bool                mbVarPSubtotal;     /// True = show variance of population subtotals.
    bool                mbShowAll;          /// True = show items without data.
    bool                mbOutline;          /// True = show in outline view, false = show in tabular view.
    bool                mbSubtotalTop;      /// True = show subtotals on top of items in outline or compact mode.
    bool                mbInsertBlankRow;   /// True = insert blank rows after items.
    bool                mbInsertPageBreak;  /// True = insert page breaks after items.
    bool                mbAutoShow;         /// True = auto show (top 10) filter enabled.
    bool                mbTopAutoShow;      /// True = auto show filter shows top entries, false = bottom.
    bool                mbMultiPageItems;   /// True = multiple items selectable in page diemsion.

    explicit            PTFieldModel();

    /** Sets axis type for BIFF/OOBIN import. */
    void                setBinAxis( sal_uInt8 nAxisFlags );
};

// ----------------------------------------------------------------------------

struct PTPageFieldModel
{
    ::rtl::OUString     maName;             /// Unique name of the page field.
    sal_Int32           mnField;            /// Base pivot field.
    sal_Int32           mnItem;             /// Index of field item that is shown by the page field.

    explicit            PTPageFieldModel();
};

// ----------------------------------------------------------------------------

struct PTDataFieldModel
{
    ::rtl::OUString     maName;             /// Name of the data field.
    sal_Int32           mnField;            /// Base pivot field.
    sal_Int32           mnSubtotal;         /// Subtotal aggregation function.
    sal_Int32           mnShowDataAs;       /// Show data as, based on another field.
    sal_Int32           mnBaseField;        /// Base field for 'show data as'.
    sal_Int32           mnBaseItem;         /// Base item for 'show data as'.
    sal_Int32           mnNumFmtId;         /// Number format for the result.

    explicit            PTDataFieldModel();

    /** Sets the subtotal aggregation function for BIFF/OOBIN import. */
    void                setBinSubtotal( sal_Int32 nSubtotal );
    /** Sets the 'show data as' type for BIFF/OOBIN import. */
    void                setBinShowDataAs( sal_Int32 nShowDataAs );
};

// ----------------------------------------------------------------------------

class PivotTableField : public WorkbookHelper
{
public:
    explicit            PivotTableField( PivotTable& rPivotTable, sal_Int32 nFieldIndex );

    /** Imports pivot field settings from the pivotField element. */
    void                importPivotField( const AttributeList& rAttribs );
    /** Imports settings of an item in this pivot field from the item element. */
    void                importItem( const AttributeList& rAttribs );
    /** Imports pivot field reference settings from the reference element. */
    void                importReference( const AttributeList& rAttribs );
    /** Imports pivot field item reference settings from the x element. */
    void                importReferenceItem( const AttributeList& rAttribs );

    /** Imports pivot field settings from the PTFIELD record. */
    void                importPTField( RecordInputStream& rStrm );
    /** Imports settings of an item in this pivot field from the PTFITEM record. */
    void                importPTFItem( RecordInputStream& rStrm );
    /** Imports pivot field reference settings from the PTREFERENCE record. */
    void                importPTReference( RecordInputStream& rStrm );
    /** Imports pivot field item reference settings from the PTREFERENCEITEM record. */
    void                importPTReferenceItem( RecordInputStream& rStrm );

    /** Imports pivot field settings from the PTFIELD and following records. */
    void                importPTField( BiffInputStream& rStrm );
    /** Imports pivot field settings from the PTFIELD2 record. */
    void                importPTField2( BiffInputStream& rStrm );
    /** Imports settings of an item in this pivot field from the PTFITEM record. */
    void                importPTFItem( BiffInputStream& rStrm );

    /** Finalizes the field after import, creates grouping and other settings. */
    void                finalizeImport(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotDescriptor >& rxDPDesc );
    /** Finalizes the grouped date field after import. */
    void                finalizeDateGroupingImport(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >& rxBaseDPField,
                            sal_Int32 nBaseFieldIdx );
    /** Finalizes the grouped field after import. */
    void                finalizeParentGroupingImport(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >& rxBaseDPField,
                            PivotCacheGroupItemVector& orItemNames );

    /** Returns the name of the DataPilot field in the fields collection. */
    inline const ::rtl::OUString& getDPFieldName() const { return maDPFieldName; }

    /** Converts dimension and other settings for a row field. */
    void                convertRowField();
    /** Converts dimension and other settings for a column field. */
    void                convertColField();
    /** Converts dimension and other settings for a hidden field. */
    void                convertHiddenField();
    /** Converts dimension and other settings for a page field */
    void                convertPageField( const PTPageFieldModel& rPageField );
    /** Converts dimension and other settings for a data field. */
    void                convertDataField( const PTDataFieldModel& rDataField );

private:
    /** Converts dimension and other settings for row, column, page, or hidden fields. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >
                        convertRowColPageField( sal_Int32 nAxis );

private:
    typedef ::std::vector< PTFieldItemModel > ItemModelVector;

    PivotTable&         mrPivotTable;       /// The parent pivot table object.
    ItemModelVector     maItems;            /// All items of this field.
    PTFieldModel        maModel;            /// Pivot field settings.
    ::rtl::OUString     maDPFieldName;      /// Name of the field in DataPilot field collection.
    sal_Int32           mnFieldIndex;       /// Zero-based index of this field.
};

// ============================================================================

struct PTFilterModel
{
    ::rtl::OUString     maName;             /// Name of the field filter.
    ::rtl::OUString     maDescription;      /// Description of the field filter.
    ::rtl::OUString     maStrValue1;        /// First string value for label filter.
    ::rtl::OUString     maStrValue2;        /// Second string value for label filter.
    double              mfValue;            /// Number of items or percent or sum to be shown.
    sal_Int32           mnField;            /// Base pivot field.
    sal_Int32           mnMemPropField;     /// Member property field.
    sal_Int32           mnType;             /// Filter type.
    sal_Int32           mnEvalOrder;        /// Evaluation order index.
    sal_Int32           mnId;               /// Unique identifier.
    sal_Int32           mnMeasureField;     /// Data field for filter calculation.
    sal_Int32           mnMeasureHier;      /// Hierarchy for filter calculation.
    bool                mbTopFilter;        /// True = filter shows top entries, false = bottom.

    explicit            PTFilterModel();
};

// ----------------------------------------------------------------------------

class PivotTableFilter : public WorkbookHelper
{
public:
    explicit            PivotTableFilter( const PivotTable& rPivotTable );

    /** Reads the settings of a field filter from the filter element. */
    void                importFilter( const AttributeList& rAttribs );
    /** Reads additional settings of a field filter from the top10 element. */
    void                importTop10( const AttributeList& rAttribs );

    /** Reads the settings of a field filter from the PTFILTER record. */
    void                importPTFilter( RecordInputStream& rStrm );
    /** Reads additional settings of a field filter from the TOP10FILTER record. */
    void                importTop10Filter( RecordInputStream& rStrm );

    /** Applies the filter to the associated pivot table field if possible. */
    void                finalizeImport();

private:
    const PivotTable&   mrPivotTable;
    PTFilterModel       maModel;
};

// ============================================================================

struct PTDefinitionModel
{
    ::rtl::OUString     maName;
    ::rtl::OUString     maDataCaption;
    ::rtl::OUString     maGrandTotalCaption;
    ::rtl::OUString     maRowHeaderCaption;
    ::rtl::OUString     maColHeaderCaption;
    ::rtl::OUString     maErrorCaption;
    ::rtl::OUString     maMissingCaption;
    ::rtl::OUString     maPageStyle;
    ::rtl::OUString     maPivotTableStyle;
    ::rtl::OUString     maVacatedStyle;
    ::rtl::OUString     maTag;
    sal_Int32           mnCacheId;
    sal_Int32           mnDataPosition;
    sal_Int32           mnPageWrap;
    sal_Int32           mnIndent;
    sal_Int32           mnChartFormat;
    sal_uInt16          mnRowFields;
    sal_uInt16          mnColFields;
    bool                mbDataOnRows;
    bool                mbShowError;
    bool                mbShowMissing;
    bool                mbShowItems;
    bool                mbDisableFieldList;
    bool                mbShowCalcMembers;
    bool                mbVisualTotals;
    bool                mbShowDataDropDown;
    bool                mbShowDrill;
    bool                mbPrintDrill;
    bool                mbEnableDrill;
    bool                mbPreserveFormatting;
    bool                mbPageOverThenDown;
    bool                mbSubtotalHiddenItems;
    bool                mbRowGrandTotals;
    bool                mbColGrandTotals;
    bool                mbFieldPrintTitles;
    bool                mbItemPrintTitles;
    bool                mbMergeItem;
    bool                mbShowEmptyRow;
    bool                mbShowEmptyCol;
    bool                mbShowHeaders;
    bool                mbFieldListSortAsc;
    bool                mbCustomListSort;

    explicit            PTDefinitionModel();
};

// ----------------------------------------------------------------------------

struct PTLocationModel
{
    ::com::sun::star::table::CellRangeAddress
                        maRange;            /// Target cell range for the pivot table.
    sal_Int32           mnFirstHeaderRow;   /// First row of header cells (relative in pivot table).
    sal_Int32           mnFirstDataRow;     /// First row of data cells (relative in pivot table).
    sal_Int32           mnFirstDataCol;     /// First column of data cells (relative in pivot table).
    sal_Int32           mnRowPageCount;     /// Number of rows in page filter area.
    sal_Int32           mnColPageCount;     /// Number of columns in page filter area.

    explicit            PTLocationModel();
};

// ----------------------------------------------------------------------------

class PivotTable : public WorkbookHelper
{
public:
    explicit            PivotTable( const WorkbookHelper& rHelper );

    /** Reads global pivot table settings from the pivotTableDefinition element. */
    void                importPivotTableDefinition( const AttributeList& rAttribs );
    /** Reads the location of the pivot table from the location element. */
    void                importLocation( const AttributeList& rAttribs, sal_Int16 nSheet );
    /** Reads the index of a field located in the row dimension. */
    void                importRowField( const AttributeList& rAttribs );
    /** Reads the index of a field located in the column dimension. */
    void                importColField( const AttributeList& rAttribs );
    /** Reads the settings of a field located in the page dimension from the pageField element. */
    void                importPageField( const AttributeList& rAttribs );
    /** Reads the settings of a field located in the data dimension from the dataField element. */
    void                importDataField( const AttributeList& rAttribs );

    /** Reads global pivot table settings from the PTDEFINITION record. */
    void                importPTDefinition( RecordInputStream& rStrm );
    /** Reads the location of the pivot table from the PTLOCATION record. */
    void                importPTLocation( RecordInputStream& rStrm, sal_Int16 nSheet );
    /** Reads the indexes of all fields located in the row dimension from a PTROWFIELDS record. */
    void                importPTRowFields( RecordInputStream& rStrm );
    /** Reads the indexes of all fields located in the column dimension from a PTCOLFIELDS record. */
    void                importPTColFields( RecordInputStream& rStrm );
    /** Reads the settings of a field located in the page dimension from the PTPAGEFIELD record. */
    void                importPTPageField( RecordInputStream& rStrm );
    /** Reads the settings of a field located in the data dimension from the PTDATAFIELD record. */
    void                importPTDataField( RecordInputStream& rStrm );

    /** Reads global pivot table settings from the PTDEFINITION record. */
    void                importPTDefinition( BiffInputStream& rStrm, sal_Int16 nSheet );
    /** Reads additional global pivot table settings from the PTDEFINITION2 record. */
    void                importPTDefinition2( BiffInputStream& rStrm );
    /** Reads the indexes of all fields located in the row or column dimension from a PTROWCOLFIELDS record. */
    void                importPTRowColFields( BiffInputStream& rStrm );
    /** Reads the settings of all fields located in the page dimension from a PTPAGEFIELDS record. */
    void                importPTPageFields( BiffInputStream& rStrm );
    /** Reads the settings of a field located in the data dimension from a PTDATAFIELD record. */
    void                importPTDataField( BiffInputStream& rStrm );

    /** Creates and returns a new pivot table field. */
    PivotTableField&    createTableField();
    /** Creates and returns a new pivot table filter. */
    PivotTableFilter&   createTableFilter();
    /** Inserts the pivot table into the sheet. */
    void                finalizeImport();
    /** Creates all date group fields for the specified cache field after import. */
    void                finalizeDateGroupingImport(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >& rxBaseDPField,
                            sal_Int32 nBaseFieldIdx );
    /** Creates all grouped fields for the specified cache field after import. */
    void                finalizeParentGroupingImport(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >& rxBaseDPField,
                            const PivotCacheField& rBaseCacheField,
                            PivotCacheGroupItemVector& orItemNames );

    /** Returns the associated data pilot field for the specified pivot table field. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >
                        getDataPilotField( const ::rtl::OUString& rFieldName ) const;
    /** Returns the associated data pilot field for the specified pivot table field. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >
                        getDataPilotField( sal_Int32 nFieldIdx ) const;
    /** Returns the data layout field used to store all data fields in row/col dimension. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotField >
                        getDataLayoutField() const;

    /** Returns the cache field with the specified index. */
    const PivotCacheField* getCacheField( sal_Int32 nFieldIdx ) const;
    /** Returns the base cache field of the data field item with the specified index. */
    const PivotCacheField* getCacheFieldOfDataField( sal_Int32 nDataItemIdx ) const;
    /** Returns the source column index of the pivot field with the passed index, or -1. */
    sal_Int32           getCacheDatabaseIndex( sal_Int32 nFieldIdx ) const;

private:
    typedef RefVector< PivotTableField >        PivotTableFieldVector;
    typedef RefVector< PivotTableFilter >       PivotTableFilterVector;
    typedef ::std::vector< sal_Int32 >          IndexVector;
    typedef ::std::vector< PTPageFieldModel >   PageFieldVector;
    typedef ::std::vector< PTDataFieldModel >   DataFieldVector;

private:
    /** Returns a pivot table field by its index. */
    PivotTableField*    getTableField( sal_Int32 nFieldIdx );

    /** Reads a field index for the row or column dimension. */
    static void         importField( IndexVector& orFields, const AttributeList& rAttribs );
    /** Reads an array of field indexes for the row or column dimension. */
    static void         importFields( IndexVector& orFields, RecordInputStream& rStrm );
    /** Reads an array of field indexes for the row or column dimension. */
    static void         importFields( IndexVector& orFields, BiffInputStream& rStrm, sal_Int32 nCount );

private:
    PivotTableFieldVector maFields;         /// All pivot table fields.
    PivotTableField     maDataField;        /// Data layout field.
    IndexVector         maRowFields;        /// Indexes to fields in row dimension.
    IndexVector         maColFields;        /// Indexes to fields in column dimension.
    PageFieldVector     maPageFields;       /// Settings for all fields in page dimension.
    DataFieldVector     maDataFields;       /// Settings for all fields in data area.
    PivotTableFilterVector maFilters;       /// All field filters.
    PTDefinitionModel   maDefModel;         /// Global pivot table settings.
    PTLocationModel     maLocationModel;    /// Location settings of the pivot table.
    const PivotCache*   mpPivotCache;       /// The pivot cache this table is based on.
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotDescriptor >
                        mxDPDescriptor;     /// Descriptor of the DataPilot object.
};

// ============================================================================

class PivotTableBuffer : public WorkbookHelper
{
public:
    explicit            PivotTableBuffer( const WorkbookHelper& rHelper );

    /** Creates and returns a new pivot table. */
    PivotTable&         createPivotTable();

    /** Inserts all pivot tables into the sheet. */
    void                finalizeImport();

private:
    typedef RefVector< PivotTable > PivotTableVector;
    PivotTableVector    maTables;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

