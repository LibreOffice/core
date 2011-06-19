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

#ifndef OOX_XLS_AUTOFILTERBUFFER_HXX
#define OOX_XLS_AUTOFILTERBUFFER_HXX

#include <com/sun/star/table/CellRangeAddress.hpp>
#include "oox/helper/refvector.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { struct TableFilterField2; }
    namespace sheet { class XDatabaseRange; }
    namespace sheet { class XSheetFilterDescriptor2; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Contains UNO API filter settings for a column in a filtered range. */
struct ApiFilterSettings
{
    typedef ::std::vector< ::com::sun::star::sheet::TableFilterField2 > FilterFieldVector;

    FilterFieldVector   maFilterFields;     /// List of UNO API filter settings.
    OptValue< bool >    mobNeedsRegExp;     /// If set, requires regular expressions to be enabled/disabled.

    explicit            ApiFilterSettings();

    void                appendField( bool bAnd, sal_Int32 nOperator, double fValue );
    void                appendField( bool bAnd, sal_Int32 nOperator, const ::rtl::OUString& rValue );
};

// ============================================================================

/** Base class for specific filter settings for a column in a filtered range.
 */
class FilterSettingsBase : public WorkbookHelper
{
public:
    explicit            FilterSettingsBase( const WorkbookHelper& rHelper );

    /** Derived classes import filter settings from the passed attribute list. */
    virtual void        importAttribs( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Derived classes import filter settings from the passed record. */
    virtual void        importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );
    /** Derived classes import filter settings from the FILTERCOLUMN record. */
    virtual void        importBiffRecord( BiffInputStream& rStrm, sal_uInt16 nFlags );

    /** Derived classes return converted UNO API filter settings representing all filter settings. */
    virtual ApiFilterSettings finalizeImport( sal_Int32 nMaxCount );
};

typedef ::boost::shared_ptr< FilterSettingsBase > FilterSettingsRef;

// ============================================================================

/** Settings for a discrete filter, specifying a list of values to be shown in
    the filtered range.
 */
class DiscreteFilter : public FilterSettingsBase
{
public:
    explicit            DiscreteFilter( const WorkbookHelper& rHelper );

    /** Imports filter settings from the filters and filter elements. */
    virtual void        importAttribs( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Imports filter settings from the FILTERS and FILTER records. */
    virtual void        importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );

    /** Returns converted UNO API filter settings representing all filter settings. */
    virtual ApiFilterSettings finalizeImport( sal_Int32 nMaxCount );

private:
    typedef ::std::vector< ::rtl::OUString > FilterValueVector;

    FilterValueVector   maValues;
    sal_Int32           mnCalendarType;
    bool                mbShowBlank;
};

// ============================================================================

/** Settings for a top-10 filter. */
class Top10Filter : public FilterSettingsBase
{
public:
    explicit            Top10Filter( const WorkbookHelper& rHelper );

    /** Imports filter settings from the filters and filter elements. */
    virtual void        importAttribs( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Imports filter settings from the FILTERS and FILTER records. */
    virtual void        importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );
    /** Imports filter settings from the FILTERCOLUMN record. */
    virtual void        importBiffRecord( BiffInputStream& rStrm, sal_uInt16 nFlags );

    /** Returns converted UNO API filter settings representing all filter settings. */
    virtual ApiFilterSettings finalizeImport( sal_Int32 nMaxCount );

private:
    double              mfValue;        /// Number of items or percentage.
    bool                mbTop;          /// True = show top (greatest) items/percentage.
    bool                mbPercent;      /// True = percentage, false = number of items.
};

// ============================================================================

/** A filter criterion for a custom filter. */
struct FilterCriterionModel
{
    ::com::sun::star::uno::Any maValue; /// Comparison operand.
    sal_Int32           mnOperator;     /// Comparison operator.
    sal_uInt8           mnDataType;     /// Operand data type (BIFF only).
    sal_uInt8           mnStrLen;       /// Length of string operand (BIFF5-BIFF8 only).

    explicit            FilterCriterionModel();

    /** Sets the passed BIFF operator constant. */
    void                setBiffOperator( sal_uInt8 nOperator );

    /** Imports the criterion model from the passed BIFF12 stream. */
    void                readBiffData( SequenceInputStream& rStrm );
    /** Imports the initial criterion data from the passed BIFF5/BIFF8 stream. */
    void                readBiffData( BiffInputStream& rStrm );
    /** Imports the trailing string data from the passed BIFF5/BIFF8 stream. */
    void                readString( BiffInputStream& rStrm, BiffType eBiff, rtl_TextEncoding eTextEnc );
};

// ----------------------------------------------------------------------------

/** Settings for a custom filter, specifying one or two comparison operators
    associated with some values.
 */
class CustomFilter : public FilterSettingsBase
{
public:
    explicit            CustomFilter( const WorkbookHelper& rHelper );

    /** Imports filter settings from the filters and filter elements. */
    virtual void        importAttribs( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Imports filter settings from the FILTERS and FILTER records. */
    virtual void        importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );
    /** Imports filter settings from the FILTERCOLUMN record. */
    virtual void        importBiffRecord( BiffInputStream& rStrm, sal_uInt16 nFlags );

    /** Returns converted UNO API filter settings representing all filter settings. */
    virtual ApiFilterSettings finalizeImport( sal_Int32 nMaxCount );

private:
    /** Apeends the passed filter criteriom, if it contains valid settings. */
    void                appendCriterion( const FilterCriterionModel& rCriterion );

private:
    typedef ::std::vector< FilterCriterionModel > FilterCriterionVector;

    FilterCriterionVector maCriteria;
    bool                mbAnd;
};

// ============================================================================

/** A column in a filtered range. Contains an object with specific filter
    settings for the cells in the column.
 */
class FilterColumn : public WorkbookHelper
{
public:
    explicit            FilterColumn( const WorkbookHelper& rHelper );

    /** Imports auto filter column settings from the filterColumn element. */
    void                importFilterColumn( const AttributeList& rAttribs );
    /** Imports auto filter column settings from the FILTERCOLUMN record. */
    void                importFilterColumn( SequenceInputStream& rStrm );
    /** Imports auto filter column settings from the FILTERCOLUMN record. */
    void                importFilterColumn( BiffInputStream& rStrm );

    /** Creates and returns the specified filter settings object. */
    template< typename FilterSettingsType >
    inline FilterSettingsBase& createFilterSettings()
        { mxSettings.reset( new FilterSettingsType( *this ) ); return *mxSettings; }

    /** Returns the index of the column in the filtered range this object is related to. */
    inline sal_Int32    getColumnId() const { return mnColId; }

    /** Returns converted UNO API filter settings representing all filter
        settings of this column. */
    ApiFilterSettings   finalizeImport( sal_Int32 nMaxCount );

private:
    FilterSettingsRef   mxSettings;
    sal_Int32           mnColId;
    bool                mbHiddenButton;
    bool                mbShowButton;
};

// ============================================================================

class AutoFilter : public WorkbookHelper
{
public:
    explicit            AutoFilter( const WorkbookHelper& rHelper );

    /** Imports auto filter settings from the autoFilter element. */
    void                importAutoFilter( const AttributeList& rAttribs, sal_Int16 nSheet );
    /** Imports auto filter settings from the AUTOFILTER record. */
    void                importAutoFilter( SequenceInputStream& rStrm, sal_Int16 nSheet );

    /** Creates a new auto filter column and stores it internally. */
    FilterColumn&       createFilterColumn();

    /** Applies the filter to the passed filter descriptor. */
    void                finalizeImport( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetFilterDescriptor2 >& rxFilterDesc );

private:
    typedef RefVector< FilterColumn > FilterColumnVector;

    FilterColumnVector  maFilterColumns;
    ::com::sun::star::table::CellRangeAddress maRange;
};

// ============================================================================

class AutoFilterBuffer : public WorkbookHelper
{
public:
    explicit            AutoFilterBuffer( const WorkbookHelper& rHelper );

    /** Creates a new auto filter and stores it internally. */
    AutoFilter&         createAutoFilter();

    /** Applies filter settings to a new database range object (used for sheet
        autofilter or advanced filter as specified by built-in defined names). */
    void                finalizeImport( sal_Int16 nSheet );

    /** Applies the filters to the passed database range object.
        @return  True = this buffer contains valid auto filter settings. */
    bool                finalizeImport( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDatabaseRange >& rxDatabaseRange );

private:
    /** Returns the auto filter object used to perform auto filtering. */
    AutoFilter*         getActiveAutoFilter();

private:
    typedef RefVector< AutoFilter > AutoFilterVector;
    AutoFilterVector    maAutoFilters;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
