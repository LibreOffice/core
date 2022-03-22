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
#include <sal/types.h>
#include <tools/color.hxx>
#include <optional>
#include <o3tl/cow_wrapper.hxx>

namespace sc
{
enum class SparklineType
{
    Line,
    Column,
    Stacked
};

enum class AxisType
{
    Individual,
    Group,
    Custom
};

enum class DisplayEmptyCellsAs
{
    Span,
    Gap,
    Zero
};

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

    Color getColorSeries() const;
    void setColorSeries(Color aColorSeries);

    Color getColorNegative() const;
    void setColorNegative(Color aColorSeries);

    Color getColorAxis() const;
    void setColorAxis(Color aColorSeries);

    Color getColorMarkers() const;
    void setColorMarkers(Color aColorSeries);

    Color getColorFirst() const;
    void setColorFirst(Color aColorSeries);

    Color getColorLast() const;
    void setColorLast(Color aColorSeries);

    Color getColorHigh() const;
    void setColorHigh(Color aColorSeries);

    Color getColorLow() const;
    void setColorLow(Color aColorSeries);

    AxisType getMinAxisType() const;
    void setMinAxisType(AxisType eAxisType);

    AxisType getMaxAxisType() const;
    void setMaxAxisType(AxisType eAxisType);

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
