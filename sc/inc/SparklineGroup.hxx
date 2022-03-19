/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "scdllapi.h"
#include <tools/color.hxx>
#include <optional>

namespace sc
{
/** Supported sparkline types */
enum class SparklineType
{
    Line,
    Column,
    Stacked
};

/** The method of calculating the axis min or max value */
enum class AxisType
{
    Individual, // calculate the min/max of a sparkline
    Group, // calculate the min or max of the whole sparkline group
    Custom // user defined
};

/** Determines how to display the empty cells */
enum class DisplayEmptyCellAs
{
    Span,
    Gap,
    Zero // empty cell equals zero
};

/** Common properties for a group of sparklines */
class SC_DLLPUBLIC SparklineGroup
{
public:
    Color m_aColorSeries;
    Color m_aColorNegative;
    Color m_aColorAxis;
    Color m_aColorMarkers;
    Color m_aColorFirst;
    Color m_aColorLast;
    Color m_aColorHigh;
    Color m_aColorLow;

    AxisType m_eMinAxisType;
    AxisType m_eMaxAxisType;

    double m_fLineWeight; // In pt

    SparklineType m_eType;

    bool m_bDateAxis;

    DisplayEmptyCellAs m_eDisplayEmptyCellsAs;

    bool m_bMarkers;
    bool m_bHigh;
    bool m_bLow;
    bool m_bFirst;
    bool m_bLast;
    bool m_bNegative;
    bool m_bDisplayXAxis;
    bool m_bDisplayHidden;
    bool m_bRightToLeft;

    std::optional<double> m_aManualMax; // if m_sMinAxisType is "custom"
    std::optional<double> m_aManualMin; // if m_sMaxAxisType is "custom"
    OUString m_sUID;

    SparklineGroup()
        : m_aColorSeries(COL_BLUE)
        , m_aColorNegative(COL_RED)
        , m_aColorAxis(COL_RED)
        , m_aColorMarkers(COL_RED)
        , m_aColorFirst(COL_RED)
        , m_aColorLast(COL_RED)
        , m_aColorHigh(COL_RED)
        , m_aColorLow(COL_RED)
        , m_eMinAxisType(AxisType::Individual)
        , m_eMaxAxisType(AxisType::Individual)
        , m_fLineWeight(0.75)
        , m_eType(SparklineType::Line)
        , m_bDateAxis(false)
        , m_eDisplayEmptyCellsAs(DisplayEmptyCellAs::Zero)
        , m_bMarkers(false)
        , m_bHigh(false)
        , m_bLow(false)
        , m_bFirst(false)
        , m_bLast(false)
        , m_bNegative(false)
        , m_bDisplayXAxis(false)
        , m_bDisplayHidden(false)
        , m_bRightToLeft(false)
    {
    }

    SparklineGroup(SparklineGroup const& pOtherSparkline)
        : m_aColorSeries(pOtherSparkline.m_aColorSeries)
        , m_aColorNegative(pOtherSparkline.m_aColorNegative)
        , m_aColorAxis(pOtherSparkline.m_aColorAxis)
        , m_aColorMarkers(pOtherSparkline.m_aColorMarkers)
        , m_aColorFirst(pOtherSparkline.m_aColorFirst)
        , m_aColorLast(pOtherSparkline.m_aColorLast)
        , m_aColorHigh(pOtherSparkline.m_aColorHigh)
        , m_aColorLow(pOtherSparkline.m_aColorLow)
        , m_eMinAxisType(pOtherSparkline.m_eMinAxisType)
        , m_eMaxAxisType(pOtherSparkline.m_eMaxAxisType)
        , m_fLineWeight(pOtherSparkline.m_fLineWeight)
        , m_eType(pOtherSparkline.m_eType)
        , m_bDateAxis(pOtherSparkline.m_bDateAxis)
        , m_eDisplayEmptyCellsAs(pOtherSparkline.m_eDisplayEmptyCellsAs)
        , m_bMarkers(pOtherSparkline.m_bMarkers)
        , m_bHigh(pOtherSparkline.m_bHigh)
        , m_bLow(pOtherSparkline.m_bLow)
        , m_bFirst(pOtherSparkline.m_bFirst)
        , m_bLast(pOtherSparkline.m_bLast)
        , m_bNegative(pOtherSparkline.m_bNegative)
        , m_bDisplayXAxis(pOtherSparkline.m_bDisplayXAxis)
        , m_bDisplayHidden(pOtherSparkline.m_bDisplayHidden)
        , m_bRightToLeft(pOtherSparkline.m_bRightToLeft)
        , m_aManualMax(pOtherSparkline.m_aManualMax)
        , m_aManualMin(pOtherSparkline.m_aManualMin)
        , m_sUID(pOtherSparkline.m_sUID)
    {
    }

    SparklineGroup& operator=(const SparklineGroup&) = delete;
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
