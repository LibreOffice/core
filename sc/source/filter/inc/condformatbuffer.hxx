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
class ScIconSetFormat;
struct ScIconSetFormatData;

namespace oox {
namespace xls {

class CondFormat;

// ============================================================================

/** Model for a single rule in a conditional formatting. */
struct CondFormatRuleModel
{
    typedef ::std::vector< ApiTokenSequence > ApiTokenSequenceVector;

    ApiTokenSequenceVector maFormulas;      /// Formulas for rule conditions.
    OUString     maText;             /// Text for 'contains' rules.
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
    OUString maFormula;

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
    ScDataBarFormatData* mpFormat;

    boost::scoped_ptr<ColorScaleRuleModelEntry> mpUpperLimit;
    boost::scoped_ptr<ColorScaleRuleModelEntry> mpLowerLimit;
};

class IconSetRule : public WorksheetHelper
{
public:
    IconSetRule( const CondFormat& rFormat );
    void importCfvo( const AttributeList& rAttribs );
    void importAttribs( const AttributeList& rAttribs );

    void SetData( ScIconSetFormat* pFormat, ScDocument* pDoc, const ScAddress& rAddr );

private:
    std::vector< ColorScaleRuleModelEntry > maEntries;
    ScIconSetFormatData* mpFormatData;
    OUString maIconSetType;
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
    void                appendFormula( const OUString& rFormula );

    /** Imports rule settings from a CFRULE record. */
    void                importCfRule( SequenceInputStream& rStrm );

    /** Creates a conditional formatting rule in the Calc document. */
    void                finalizeImport();

    /** Returns the priority of this rule. */
    inline sal_Int32    getPriority() const { return maModel.mnPriority; }

    ColorScaleRule*     getColorScale();
    DataBarRule*        getDataBar();
    IconSetRule*            getIconSet();

private:
    const CondFormat&   mrCondFormat;
    CondFormatRuleModel maModel;
    ScConditionalFormat* mpFormat;
    boost::scoped_ptr<ColorScaleRule> mpColor;
    boost::scoped_ptr<DataBarRule> mpDataBar;
    boost::scoped_ptr<IconSetRule> mpIconSet;
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
    static sal_Int32    convertToInternalOperator( sal_Int32 nToken );

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
