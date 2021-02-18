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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_AUTOFILTERBUFFER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_AUTOFILTERBUFFER_HXX

#include <oox/helper/helper.hxx>
#include <oox/helper/refvector.hxx>
#include "workbookhelper.hxx"
#include <com/sun/star/sheet/TableFilterField3.hpp>

namespace com::sun::star {
    namespace sheet { class XDatabaseRange; }
    namespace sheet { class XSheetFilterDescriptor3; }
}

namespace oox { class AttributeList; }
namespace oox { class SequenceInputStream; }

namespace oox {
namespace xls {

/** Contains UNO API filter settings for a column in a filtered range. */
struct ApiFilterSettings
{
    typedef ::std::vector<css::sheet::TableFilterField3> FilterFieldVector;

    FilterFieldVector   maFilterFields;     /// List of UNO API filter settings.
    OptValue< bool >    mobNeedsRegExp;     /// If set, requires regular expressions to be enabled/disabled.

    explicit            ApiFilterSettings();

    void appendField( bool bAnd, sal_Int32 nOperator, double fValue );
    void appendField( bool bAnd, sal_Int32 nOperator, const OUString& rValue );
    void appendField( bool bAnd, const std::vector<std::pair<OUString, bool>>& rValues );
};

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

    /** Derived classes return converted UNO API filter settings representing all filter settings. */
    virtual ApiFilterSettings finalizeImport();
};


/** Settings for a discrete filter, specifying a list of values to be shown in
    the filtered range.
 */
class DiscreteFilter : public FilterSettingsBase
{
public:
    explicit            DiscreteFilter( const WorkbookHelper& rHelper );

    /** Imports filter settings from the filters and filter elements. */
    virtual void        importAttribs( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    /** Imports filter settings from the FILTERS and FILTER records. */
    virtual void        importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;

    /** Returns converted UNO API filter settings representing all filter settings. */
    virtual ApiFilterSettings finalizeImport() override;

private:

    std::vector<std::pair<OUString, bool>> maValues; // first->values, second->bDatefFormat
    sal_Int32           mnCalendarType;
    bool                mbShowBlank;
};

/** Settings for a top-10 filter. */
class Top10Filter : public FilterSettingsBase
{
public:
    explicit            Top10Filter( const WorkbookHelper& rHelper );

    /** Imports filter settings from the filters and filter elements. */
    virtual void        importAttribs( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    /** Imports filter settings from the FILTERS and FILTER records. */
    virtual void        importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;

    /** Returns converted UNO API filter settings representing all filter settings. */
    virtual ApiFilterSettings finalizeImport() override;

private:
    double              mfValue;        /// Number of items or percentage.
    bool                mbTop;          /// True = show top (greatest) items/percentage.
    bool                mbPercent;      /// True = percentage, false = number of items.
};

/** A filter criterion for a custom filter. */
struct FilterCriterionModel
{
    css::uno::Any       maValue; /// Comparison operand.
    sal_Int32           mnOperator;     /// Comparison operator.
    sal_uInt8           mnDataType;     /// Operand data type (BIFF only).

    explicit            FilterCriterionModel();

    /** Sets the passed BIFF operator constant. */
    void                setBiffOperator( sal_uInt8 nOperator );

    /** Imports the criterion model from the passed BIFF12 stream. */
    void                readBiffData( SequenceInputStream& rStrm );
};

/** Settings for a custom filter, specifying one or two comparison operators
    associated with some values.
 */
class CustomFilter : public FilterSettingsBase
{
public:
    explicit            CustomFilter( const WorkbookHelper& rHelper );

    /** Imports filter settings from the filters and filter elements. */
    virtual void        importAttribs( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    /** Imports filter settings from the FILTERS and FILTER records. */
    virtual void        importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;

    /** Returns converted UNO API filter settings representing all filter settings. */
    virtual ApiFilterSettings finalizeImport() override;

private:
    /** Appends the passed filter criterion, if it contains valid settings. */
    void                appendCriterion( const FilterCriterionModel& rCriterion );

private:
    typedef ::std::vector< FilterCriterionModel > FilterCriterionVector;

    FilterCriterionVector maCriteria;
    bool                mbAnd;
};

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

    /** Creates and returns the specified filter settings object. */
    template< typename FilterSettingsType >
    FilterSettingsBase& createFilterSettings()
        { mxSettings = std::make_shared<FilterSettingsType>( *this ); return *mxSettings; }

    /** Returns converted UNO API filter settings representing all filter
        settings of this column. */
    ApiFilterSettings   finalizeImport();

private:
    std::shared_ptr< FilterSettingsBase >
                        mxSettings;
    sal_Int32           mnColId;
    bool                mbHiddenButton;
    bool                mbShowButton;
};

// class SortCondition

class SortCondition : public WorkbookHelper
{
public:
    explicit SortCondition( const WorkbookHelper& rHelper );

    void importSortCondition( const AttributeList& rAttribs, sal_Int16 nSheet );

    ScRange maRange; // Column/Row that this sort condition applies to.
    OUString maSortCustomList; // Sort by a custom list.
    bool mbDescending;
};

// class AutoFilter

class AutoFilter : public WorkbookHelper
{
public:
    explicit            AutoFilter( const WorkbookHelper& rHelper );

    /** Imports auto filter settings from the autoFilter element. */
    void                importAutoFilter( const AttributeList& rAttribs, sal_Int16 nSheet );
    /** Imports auto filter settings from the AUTOFILTER record. */
    void                importAutoFilter( SequenceInputStream& rStrm, sal_Int16 nSheet );

    void                importSortState( const AttributeList& rAttribs, sal_Int16 nSheet );

    /** Creates a new auto filter column and stores it internally. */
    FilterColumn&       createFilterColumn();

    SortCondition&      createSortCondition();

    /** Applies the filter to the passed filter descriptor. */
    void                finalizeImport( const css::uno::Reference< css::sheet::XDatabaseRange >& rxDatabaseRange,
                                        sal_Int16 nSheet );

private:
    typedef RefVector< FilterColumn > FilterColumnVector;

    FilterColumnVector  maFilterColumns;
    ScRange             maRange;

    ScRange maSortRange; // The whole range of data to sort (not just the sort-by column).
    typedef RefVector< SortCondition > SortConditionVector;
    SortConditionVector maSortConditions;
};

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
    bool                finalizeImport( const css::uno::Reference< css::sheet::XDatabaseRange >& rxDatabaseRange,
                                        sal_Int16 nSheet );

private:
    /** Returns the auto filter object used to perform auto filtering. */
    AutoFilter*         getActiveAutoFilter();

private:
    typedef RefVector< AutoFilter > AutoFilterVector;
    AutoFilterVector    maAutoFilters;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
