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

#include "worksheethelper.hxx"
#include <tools/color.hxx>
#include <colorscale.hxx>
#include <conditio.hxx>
#include <rangelst.hxx>

#include <memory>
#include <vector>

class ScColorScaleFormat;
class ScDataBarFormat;
struct ScDataBarFormatData;
class ScIconSetFormat;
struct ScIconSetFormatData;

namespace oox { class AttributeList; }

namespace oox::xls {

class CondFormat;

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
    bool mbNum;
    OUString maFormula;

    ColorScaleRuleModelEntry():
        maColor(),
        mnVal(0),
        mbMin(false),
        mbMax(false),
        mbPercent(false),
        mbPercentile(false),
        mbNum(false) {}
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

    ScDataBarFormatData* getDataBarFormatData() { return mxFormat.get(); }

private:
    std::unique_ptr<ScDataBarFormatData> mxFormat;

    std::unique_ptr<ColorScaleRuleModelEntry> mpUpperLimit;
    std::unique_ptr<ColorScaleRuleModelEntry> mpLowerLimit;
};

class IconSetRule : public WorksheetHelper
{
public:
    IconSetRule( const WorksheetHelper& rParent );
    void importCfvo( const AttributeList& rAttribs );
    void importAttribs( const AttributeList& rAttribs );
    void importFormula(const OUString& rFormula);
    void importIcon(const AttributeList& rAttribs);

    void SetData( ScIconSetFormat* pFormat, ScDocument* pDoc, const ScAddress& rAddr );

private:
    std::vector< ColorScaleRuleModelEntry > maEntries;
    std::unique_ptr<ScIconSetFormatData> mxFormatData;
    OUString maIconSetType;
    bool mbCustom;
};

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

    /** Directly set a ScFormatEntry with a priority ready for finalizeImport(). */
    void                setFormatEntry(sal_Int32 nPriority, ScFormatEntry* pEntry);

    /** Creates a conditional formatting rule in the Calc document. */
    void                finalizeImport();

    /** Returns the priority of this rule. */
    sal_Int32    getPriority() const { return maModel.mnPriority; }

    ColorScaleRule*     getColorScale();
    DataBarRule*        getDataBar();
    IconSetRule*            getIconSet();

private:
    const CondFormat&   mrCondFormat;
    CondFormatRuleModel maModel;
    ScConditionalFormat* mpFormat;
    ScFormatEntry*       mpFormatEntry;
    std::unique_ptr<ColorScaleRule> mpColor;
    std::unique_ptr<DataBarRule> mpDataBar;
    std::unique_ptr<IconSetRule> mpIconSet;
};

typedef std::shared_ptr< CondFormatRule > CondFormatRuleRef;

/** Model for a conditional formatting object. */
struct CondFormatModel
{
    ScRangeList         maRanges;           /// Cell ranges for this conditional format.
    bool                mbPivot;            /// Conditional formatting belongs to pivot table.

    explicit            CondFormatModel();
};

class CondFormatBuffer;

/** Represents a conditional formatting object with a list of affected cell ranges. */
class CondFormat : public WorksheetHelper
{
friend class CondFormatBuffer;
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
    const ScRangeList& getRanges() const { return maModel.maRanges; }

    void                setReadyForFinalize() { mbReadyForFinalize = true; }
private:
    CondFormatRuleRef   createRule();
    void                insertRule( CondFormatRuleRef const & xRule );

private:
    typedef RefMap< sal_Int32, CondFormatRule > CondFormatRuleMap;

    CondFormatModel     maModel;            /// Model of this conditional formatting.
    CondFormatRuleMap   maRules;            /// Maps formatting rules by priority.
    ScConditionalFormat* mpFormat;
    bool                mbReadyForFinalize;
};

struct ExCfRuleModel
{
    ExCfRuleModel() : mnAxisColor( ColorTransparency, UNSIGNED_RGB_TRANSPARENT ), mnNegativeColor( ColorTransparency, UNSIGNED_RGB_TRANSPARENT ), mbGradient( false ), mbIsLower( true ) {}
    // AxisColor
    ::Color mnAxisColor;
    // NegativeFillColor
    ::Color mnNegativeColor;
    OUString maAxisPosition; // DataBar
    OUString maColorScaleType; // Cfvo
    bool mbGradient; // DataBar
    bool mbIsLower; // Cfvo
};

class ExtCfDataBarRule : public WorksheetHelper
{
    enum RuleType
    {
        DATABAR,
        NEGATIVEFILLCOLOR,
        AXISCOLOR,
        CFVO,
        UNKNOWN,
    };
    ExCfRuleModel maModel;
    RuleType mnRuleType;
    ScDataBarFormatData* mpTarget;
public:

    ExtCfDataBarRule(ScDataBarFormatData* pTarget, const WorksheetHelper& rParent);
    void finalizeImport();
    void importDataBar(  const AttributeList& rAttribs );
    void importNegativeFillColor(  const AttributeList& rAttribs );
    void importAxisColor(  const AttributeList& rAttribs );
    void importCfvo(  const AttributeList& rAttribs );
    ExCfRuleModel& getModel() { return maModel; }
};

class ExtCfCondFormat
{
public:
    ExtCfCondFormat(const ScRangeList& aRange, std::vector< std::unique_ptr<ScFormatEntry> >& rEntries,
                    const std::vector<sal_Int32>* pPriorities = nullptr);
    ~ExtCfCondFormat();

    const ScRangeList& getRange() const;
    const std::vector< std::unique_ptr<ScFormatEntry> >& getEntries() const;
    const std::vector<sal_Int32>& getPriorities() const { return maPriorities; }

private:
    std::vector< std::unique_ptr<ScFormatEntry> > maEntries;
    std::vector<sal_Int32> maPriorities;
    ScRangeList maRange;
};

typedef std::shared_ptr< CondFormat > CondFormatRef;
typedef std::shared_ptr< ExtCfDataBarRule > ExtCfDataBarRuleRef;

class CondFormatBuffer : public WorksheetHelper
{
public:
    explicit            CondFormatBuffer( const WorksheetHelper& rHelper );

    /** Imports settings from the conditionalFormatting element. */
    CondFormatRef       importConditionalFormatting( const AttributeList& rAttribs );
    /** Imports settings from the CONDFORMATTING record. */
    CondFormatRef       importCondFormatting( SequenceInputStream& rStrm );
    ExtCfDataBarRuleRef createExtCfDataBarRule(ScDataBarFormatData* pTarget);
    std::vector< std::unique_ptr<ExtCfCondFormat> >& importExtCondFormat();

    /** Converts an OOXML condition operator token to the API constant. */
    static sal_Int32    convertToApiOperator( sal_Int32 nToken );
    static ScConditionMode convertToInternalOperator( sal_Int32 nToken );
    void                finalizeImport();
private:
    CondFormatRef       createCondFormat();

private:
    typedef RefVector< CondFormat > CondFormatVec;
    typedef RefVector< ExtCfDataBarRule > ExtCfDataBarRuleVec;
    CondFormatVec       maCondFormats;      /// All conditional formatting in a sheet.
    ExtCfDataBarRuleVec        maCfRules;          /// All external conditional formatting rules in a sheet.
    std::vector< std::unique_ptr<ExtCfCondFormat> > maExtCondFormats;
    sal_Int32 mnNonPrioritizedRuleNextPriority = 1048576;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
