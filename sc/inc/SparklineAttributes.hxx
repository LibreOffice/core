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
#include <docmodel/color/ComplexColor.hxx>
#include <optional>
#include <o3tl/cow_wrapper.hxx>

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
enum class DisplayEmptyCellsAs
{
    Span,
    Gap,
    Zero // empty cell equals zero
};

/** Common properties for a group of sparklines */
class SC_DLLPUBLIC SparklineAttributes
{
private:
    class Implementation;
    o3tl::cow_wrapper<Implementation> m_aImplementation;

public:
    SparklineAttributes();
    ~SparklineAttributes();
    SparklineAttributes(const SparklineAttributes& rOther);
    SparklineAttributes(SparklineAttributes&& rOther);
    SparklineAttributes& operator=(const SparklineAttributes& rOther);
    SparklineAttributes& operator=(SparklineAttributes&& rOther);

    bool operator==(const SparklineAttributes& rOther) const;
    bool operator!=(const SparklineAttributes& rOther) const
    {
        return !(SparklineAttributes::operator==(rOther));
    }

    void resetColors();

    model::ComplexColor getColorSeries() const;
    void setColorSeries(model::ComplexColor const& rColorSeries);

    model::ComplexColor getColorNegative() const;
    void setColorNegative(model::ComplexColor const& rColorSeries);

    model::ComplexColor getColorAxis() const;
    void setColorAxis(model::ComplexColor const& rColorSeries);

    model::ComplexColor getColorMarkers() const;
    void setColorMarkers(model::ComplexColor const& rColorSeries);

    model::ComplexColor getColorFirst() const;
    void setColorFirst(model::ComplexColor const& rColorSeries);

    model::ComplexColor getColorLast() const;
    void setColorLast(model::ComplexColor const& rColorSeries);

    model::ComplexColor getColorHigh() const;
    void setColorHigh(model::ComplexColor const& rColorSeries);

    model::ComplexColor getColorLow() const;
    void setColorLow(model::ComplexColor const& rColorSeries);

    AxisType getMinAxisType() const;
    void setMinAxisType(AxisType eAxisType);

    AxisType getMaxAxisType() const;
    void setMaxAxisType(AxisType eAxisType);

    /** Line weight or width in points */
    double getLineWeight() const;
    void setLineWeight(double nWeight);

    SparklineType getType() const;
    void setType(SparklineType eType);

    bool isDateAxis() const;
    void setDateAxis(bool bValue);

    DisplayEmptyCellsAs getDisplayEmptyCellsAs() const;
    void setDisplayEmptyCellsAs(DisplayEmptyCellsAs eValue);

    bool isMarkers() const;
    void setMarkers(bool bValue);

    bool isHigh() const;
    void setHigh(bool bValue);

    bool isLow() const;
    void setLow(bool bValue);

    bool isFirst() const;
    void setFirst(bool bValue);

    bool isLast() const;
    void setLast(bool bValue);

    bool isNegative() const;
    void setNegative(bool bValue);

    bool shouldDisplayXAxis() const;
    void setDisplayXAxis(bool bValue);

    bool shouldDisplayHidden() const;
    void setDisplayHidden(bool bValue);

    bool isRightToLeft() const;
    void setRightToLeft(bool bValue);

    std::optional<double> getManualMax() const;
    void setManualMax(std::optional<double> aValue);

    std::optional<double> getManualMin() const;
    void setManualMin(std::optional<double> aValue);
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
