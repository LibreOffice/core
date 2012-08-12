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

#ifndef OOX_XLS_CONDFORMATBUFFER_HXX
#define OOX_XLS_CONDFORMATBUFFER_HXX

#include <com/sun/star/sheet/ConditionOperator2.hpp>
#include "formulaparser.hxx"
#include "worksheethelper.hxx"
#include <boost/scoped_ptr.hpp>
#include <tools/color.hxx>

namespace com { namespace sun { namespace star {
    namespace sheet { class XSheetConditionalEntries; }
} } }
class ScColorScaleFormat;
class ScDataBarFormat;
struct ScDataBarFormatData;
class ScConditionalFormat;

namespace oox {
namespace xls {

class CondFormat;

// ============================================================================

/** Model for a single rule in a conditional formatting. */
struct CondFormatRuleModel
{
    typedef ::std::vector< ApiTokenSequence > ApiTokenSequenceVector;

    ApiTokenSequenceVector maFormulas;      /// Formulas for rule conditions.
    ::rtl::OUString     maText;             /// Text for 'contains' rules.
    sal_Int32           mnPriority;         /// Priority of this rule.
    sal_Int32           mnType;             /// Type of the rule.
    sal_Int32           mnOperator;         /// In cell-is rules: Comparison operator.
    sal_Int32           mnTimePeriod;       /// In time-period rules: Type of time period.
    sal_Int32           mnRank;             /// In top-10 rules: True = bottom, false = top.
    sal_Int32           mnStdDev;           /// In average rules: Number of std deviations.
    sal_Int32           mnDxfId;            /// Differential formatting identifier.
    bool                mbStopIfTrue;       /// True = stop evaluating rules, if this rule is true.
    bool                mbBottom;           /// In top-10 rules: True = bottom, false = top.
    bool                mbPercent;          /// In top-10 rules: True = percent, false = rank.
    bool                mbAboveAverage;     /// In average rules: True = above average, false = below.
    bool                mbEqualAverage;     /// In average rules: True = include average, false = exclude.

    explicit            CondFormatRuleModel();

    /** Sets the passed BIFF operator for condition type cellIs. */
    void                setBiffOperator( sal_Int32 nOperator );

    /** Sets the passed BIFF12 text comparison type and operator. */
    void                setBiff12TextType( sal_Int32 nOperator );
};

struct ColorScaleRuleModelEntry
{
    ::Color maColor;
    double mnVal;

    bool mbMin;
    bool mbMax;
    bool mbPercent;
    bool mbPercentile;
    rtl::OUString maFormula;

    ColorScaleRuleModelEntry():
        maColor(),
        mnVal(0),
        mbMin(false),
        mbMax(false),
        mbPercent(false),
        mbPercentile(false) {}
};

class ColorScaleRule : public WorksheetHelper
{
public:
    ColorScaleRule( const CondFormat& rFormat );

    void importCfvo( const AttributeList& rAttribs );
    void importColor( const AttributeList& rAttribs );

    void AddEntries( ScColorScaleFormat* pFormat, ScDocument* pDoc, const ScAddress& rAddr );

private:
    const CondFormat& mrCondFormat;
    std::vector< ColorScaleRuleModelEntry > maColorScaleRuleEntries;

    sal_uInt32 mnCfvo;
    sal_uInt32 mnCol;
};

class DataBarRule : public WorksheetHelper
{
public:
    DataBarRule( const CondFormat& rFormat );
    void importCfvo( const AttributeList& rAttribs );
    void importColor( const AttributeList& rAttribs );
    void importAttribs( const AttributeList& rAttribs );

    void SetData( ScDataBarFormat* pFormat, ScDocument* pDoc, const ScAddress& rAddr );

    ScDataBarFormatData* getDataBarFormatData() { return mpFormat; }

private:
    const CondFormat& mrCondFormat;
    ScDataBarFormatData* mpFormat;

    boost::scoped_ptr<ColorScaleRuleModelEntry> mpUpperLimit;
    boost::scoped_ptr<ColorScaleRuleModelEntry> mpLowerLimit;
};


// ============================================================================


/** Represents a single rule in a conditional formatting. */
class CondFormatRule : public WorksheetHelper
{
public:
    explicit            CondFormatRule( const CondFormat& rCondFormat, ScConditionalFormat* pFormat );

    /** Imports rule settings from the cfRule element. */
    void                importCfRule( const AttributeList& rAttribs );
    /** Appends a new condition formula string. */
    void                appendFormula( const ::rtl::OUString& rFormula );

    /** Imports rule settings from a CFRULE record. */
    void                importCfRule( SequenceInputStream& rStrm );

    /** Creates a conditional formatting rule in the Calc document. */
    void                finalizeImport();

    /** Returns the priority of this rule. */
    inline sal_Int32    getPriority() const { return maModel.mnPriority; }

    ColorScaleRule*     getColorScale();
    DataBarRule*        getDataBar();

private:
    const CondFormat&   mrCondFormat;
    CondFormatRuleModel maModel;
    ScConditionalFormat* mpFormat;
    boost::scoped_ptr<ColorScaleRule> mpColor;
    boost::scoped_ptr<DataBarRule> mpDataBar;
};

typedef ::boost::shared_ptr< CondFormatRule > CondFormatRuleRef;

// ============================================================================

/** Model for a conditional formatting object. */
struct CondFormatModel
{
    ApiCellRangeList    maRanges;           /// Cell ranges for this conditional format.
    bool                mbPivot;            /// Conditional formatting belongs to pivot table.

    explicit            CondFormatModel();
};

// ============================================================================

/** Represents a conditional formatting object with a list of affected cell ranges. */
class CondFormat : public WorksheetHelper
{
public:
    explicit            CondFormat( const WorksheetHelper& rHelper );

    /** Imports settings from the conditionalFormatting element. */
    void                importConditionalFormatting( const AttributeList& rAttribs );
    /** Imports a conditional formatting rule from the cfRule element. */
    CondFormatRuleRef   importCfRule( const AttributeList& rAttribs );

    /** Imports settings from the CONDFORMATTING record. */
    void                importCondFormatting( SequenceInputStream& rStrm );
    /** Imports a conditional formatting rule from the CFRULE record. */
    void                importCfRule( SequenceInputStream& rStrm );

    /** Creates the conditional formatting in the Calc document. */
    void                finalizeImport();

    /** Returns the cell ranges this conditional formatting belongs to. */
    inline const ApiCellRangeList& getRanges() const { return maModel.maRanges; }

private:
    CondFormatRuleRef   createRule();
    void                insertRule( CondFormatRuleRef xRule );

private:
    typedef RefMap< sal_Int32, CondFormatRule > CondFormatRuleMap;

    CondFormatModel     maModel;            /// Model of this conditional formatting.
    CondFormatRuleMap   maRules;            /// Maps formatting rules by priority.
    ScConditionalFormat* mpFormat;
};

typedef ::boost::shared_ptr< CondFormat > CondFormatRef;

// ============================================================================

class CondFormatBuffer : public WorksheetHelper
{
public:
    explicit            CondFormatBuffer( const WorksheetHelper& rHelper );

    /** Imports settings from the conditionalFormatting element. */
    CondFormatRef       importConditionalFormatting( const AttributeList& rAttribs );
    /** Imports settings from the CONDFORMATTING record. */
    CondFormatRef       importCondFormatting( SequenceInputStream& rStrm );

    /** Converts an OOXML condition operator token to the API constant. */
    static sal_Int32    convertToApiOperator( sal_Int32 nToken );

private:
    CondFormatRef       createCondFormat();

private:
    typedef RefVector< CondFormat > CondFormatVec;
    CondFormatVec       maCondFormats;      /// All conditional formatting in a sheet.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
