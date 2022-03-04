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
        , m_aColorNegative(COL_TRANSPARENT)
        , m_aColorAxis(COL_TRANSPARENT)
        , m_aColorMarkers(COL_TRANSPARENT)
        , m_aColorFirst(COL_TRANSPARENT)
        , m_aColorLast(COL_TRANSPARENT)
        , m_aColorHigh(COL_TRANSPARENT)
        , m_aColorLow(COL_TRANSPARENT)
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

    SparklineGroup(const SparklineGroup&) = delete;
    SparklineGroup& operator=(const SparklineGroup&) = delete;
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
