/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <SparklineAttributes.hxx>

namespace sc
{
/** Holder of sparkline attributes */
class SparklineAttributes::Implementation
{
public:
    model::ComplexColor m_aColorSeries;
    model::ComplexColor m_aColorNegative;
    model::ComplexColor m_aColorAxis;
    model::ComplexColor m_aColorMarkers;
    model::ComplexColor m_aColorFirst;
    model::ComplexColor m_aColorLast;
    model::ComplexColor m_aColorHigh;
    model::ComplexColor m_aColorLow;

    AxisType m_eMinAxisType;
    AxisType m_eMaxAxisType;

    double m_fLineWeight; // In pt

    SparklineType m_eType;

    bool m_bDateAxis;

    DisplayEmptyCellsAs m_eDisplayEmptyCellsAs; // span, gap, zero

    bool m_bMarkers;
    bool m_bHigh;
    bool m_bLow;
    bool m_bFirst;
    bool m_bLast;
    bool m_bNegative;
    bool m_bDisplayXAxis;
    bool m_bDisplayHidden;
    bool m_bRightToLeft;

    std::optional<double> m_aManualMax;
    std::optional<double> m_aManualMin;
    static constexpr Color COL_STANDARD_RED = 0xff0000;
    static constexpr Color COL_STANDARD_BLUE = 0x2a6099;

    Implementation()
        : m_aColorSeries(model::ComplexColor::setRGB(COL_STANDARD_BLUE))
        , m_aColorNegative(model::ComplexColor::setRGB(COL_STANDARD_RED))
        , m_aColorAxis(model::ComplexColor::setRGB(COL_STANDARD_RED))
        , m_aColorMarkers(model::ComplexColor::setRGB(COL_STANDARD_RED))
        , m_aColorFirst(model::ComplexColor::setRGB(COL_STANDARD_RED))
        , m_aColorLast(model::ComplexColor::setRGB(COL_STANDARD_RED))
        , m_aColorHigh(model::ComplexColor::setRGB(COL_STANDARD_RED))
        , m_aColorLow(model::ComplexColor::setRGB(COL_STANDARD_RED))
        , m_eMinAxisType(AxisType::Individual)
        , m_eMaxAxisType(AxisType::Individual)
        , m_fLineWeight(0.75)
        , m_eType(SparklineType::Line)
        , m_bDateAxis(false)
        , m_eDisplayEmptyCellsAs(DisplayEmptyCellsAs::Zero)
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

    Implementation(Implementation const& pOther)
        : m_aColorSeries(pOther.m_aColorSeries)
        , m_aColorNegative(pOther.m_aColorNegative)
        , m_aColorAxis(pOther.m_aColorAxis)
        , m_aColorMarkers(pOther.m_aColorMarkers)
        , m_aColorFirst(pOther.m_aColorFirst)
        , m_aColorLast(pOther.m_aColorLast)
        , m_aColorHigh(pOther.m_aColorHigh)
        , m_aColorLow(pOther.m_aColorLow)
        , m_eMinAxisType(pOther.m_eMinAxisType)
        , m_eMaxAxisType(pOther.m_eMaxAxisType)
        , m_fLineWeight(pOther.m_fLineWeight)
        , m_eType(pOther.m_eType)
        , m_bDateAxis(pOther.m_bDateAxis)
        , m_eDisplayEmptyCellsAs(pOther.m_eDisplayEmptyCellsAs)
        , m_bMarkers(pOther.m_bMarkers)
        , m_bHigh(pOther.m_bHigh)
        , m_bLow(pOther.m_bLow)
        , m_bFirst(pOther.m_bFirst)
        , m_bLast(pOther.m_bLast)
        , m_bNegative(pOther.m_bNegative)
        , m_bDisplayXAxis(pOther.m_bDisplayXAxis)
        , m_bDisplayHidden(pOther.m_bDisplayHidden)
        , m_bRightToLeft(pOther.m_bRightToLeft)
        , m_aManualMax(pOther.m_aManualMax)
        , m_aManualMin(pOther.m_aManualMin)
    {
    }

    bool operator==(const Implementation& rImpl) const
    {
        return (m_aColorSeries == rImpl.m_aColorSeries)
               && (m_aColorNegative == rImpl.m_aColorNegative)
               && (m_aColorAxis == rImpl.m_aColorAxis) && (m_aColorMarkers == rImpl.m_aColorMarkers)
               && (m_aColorFirst == rImpl.m_aColorFirst) && (m_aColorLast == rImpl.m_aColorLast)
               && (m_aColorHigh == rImpl.m_aColorHigh) && (m_aColorLow == rImpl.m_aColorLow)
               && (m_eMinAxisType == rImpl.m_eMinAxisType)
               && (m_eMaxAxisType == rImpl.m_eMaxAxisType) && (m_fLineWeight == rImpl.m_fLineWeight)
               && (m_eType == rImpl.m_eType) && (m_bDateAxis == rImpl.m_bDateAxis)
               && (m_eDisplayEmptyCellsAs == rImpl.m_eDisplayEmptyCellsAs)
               && (m_bMarkers == rImpl.m_bMarkers) && (m_bHigh == rImpl.m_bHigh)
               && (m_bLow == rImpl.m_bLow) && (m_bFirst == rImpl.m_bFirst)
               && (m_bLast == rImpl.m_bLast) && (m_bNegative == rImpl.m_bNegative)
               && (m_bDisplayXAxis == rImpl.m_bDisplayXAxis)
               && (m_bDisplayHidden == rImpl.m_bDisplayHidden)
               && (m_bRightToLeft == rImpl.m_bRightToLeft) && (m_aManualMax == rImpl.m_aManualMax)
               && (m_aManualMin == rImpl.m_aManualMin);
    }
};

SparklineAttributes::SparklineAttributes() = default;

SparklineAttributes::~SparklineAttributes() = default;

SparklineAttributes::SparklineAttributes(SparklineAttributes const&) = default;

SparklineAttributes::SparklineAttributes(SparklineAttributes&&) = default;

SparklineAttributes& SparklineAttributes::operator=(SparklineAttributes const&) = default;

SparklineAttributes& SparklineAttributes::operator=(SparklineAttributes&&) = default;

bool SparklineAttributes::operator==(SparklineAttributes const& rOther) const
{
    return m_aImplementation == rOther.m_aImplementation;
}

void SparklineAttributes::resetColors()
{
    m_aImplementation->m_aColorSeries = model::ComplexColor();
    m_aImplementation->m_aColorNegative = model::ComplexColor();
    m_aImplementation->m_aColorAxis = model::ComplexColor();
    m_aImplementation->m_aColorMarkers = model::ComplexColor();
    m_aImplementation->m_aColorFirst = model::ComplexColor();
    m_aImplementation->m_aColorLast = model::ComplexColor();
    m_aImplementation->m_aColorHigh = model::ComplexColor();
    m_aImplementation->m_aColorLow = model::ComplexColor();
}

model::ComplexColor SparklineAttributes::getColorSeries() const
{
    return m_aImplementation->m_aColorSeries;
}

void SparklineAttributes::setColorSeries(model::ComplexColor const& rColor)
{
    m_aImplementation->m_aColorSeries = rColor;
}

model::ComplexColor SparklineAttributes::getColorNegative() const
{
    return m_aImplementation->m_aColorNegative;
}

void SparklineAttributes::setColorNegative(model::ComplexColor const& rColor)
{
    m_aImplementation->m_aColorNegative = rColor;
}

model::ComplexColor SparklineAttributes::getColorAxis() const
{
    return m_aImplementation->m_aColorAxis;
}

void SparklineAttributes::setColorAxis(model::ComplexColor const& rColor)
{
    m_aImplementation->m_aColorAxis = rColor;
}

model::ComplexColor SparklineAttributes::getColorMarkers() const
{
    return m_aImplementation->m_aColorMarkers;
}
void SparklineAttributes::setColorMarkers(model::ComplexColor const& rColor)
{
    m_aImplementation->m_aColorMarkers = rColor;
}

model::ComplexColor SparklineAttributes::getColorFirst() const
{
    return m_aImplementation->m_aColorFirst;
}
void SparklineAttributes::setColorFirst(model::ComplexColor const& rColor)
{
    m_aImplementation->m_aColorFirst = rColor;
}

model::ComplexColor SparklineAttributes::getColorLast() const
{
    return m_aImplementation->m_aColorLast;
}
void SparklineAttributes::setColorLast(model::ComplexColor const& rColor)
{
    m_aImplementation->m_aColorLast = rColor;
}

model::ComplexColor SparklineAttributes::getColorHigh() const
{
    return m_aImplementation->m_aColorHigh;
}
void SparklineAttributes::setColorHigh(model::ComplexColor const& rColor)
{
    m_aImplementation->m_aColorHigh = rColor;
}

model::ComplexColor SparklineAttributes::getColorLow() const
{
    return m_aImplementation->m_aColorLow;
}
void SparklineAttributes::setColorLow(model::ComplexColor const& rColor)
{
    m_aImplementation->m_aColorLow = rColor;
}

AxisType SparklineAttributes::getMinAxisType() const { return m_aImplementation->m_eMinAxisType; }
void SparklineAttributes::setMinAxisType(AxisType eAxisType)
{
    m_aImplementation->m_eMinAxisType = eAxisType;
}

AxisType SparklineAttributes::getMaxAxisType() const { return m_aImplementation->m_eMaxAxisType; }
void SparklineAttributes::setMaxAxisType(AxisType eAxisType)
{
    m_aImplementation->m_eMaxAxisType = eAxisType;
}

double SparklineAttributes::getLineWeight() const { return m_aImplementation->m_fLineWeight; }
void SparklineAttributes::setLineWeight(double nWeight)
{
    m_aImplementation->m_fLineWeight = nWeight;
}

SparklineType SparklineAttributes::getType() const { return m_aImplementation->m_eType; }
void SparklineAttributes::setType(SparklineType eType) { m_aImplementation->m_eType = eType; }

bool SparklineAttributes::isDateAxis() const { return m_aImplementation->m_bDateAxis; }
void SparklineAttributes::setDateAxis(bool bValue) { m_aImplementation->m_bDateAxis = bValue; }

DisplayEmptyCellsAs SparklineAttributes::getDisplayEmptyCellsAs() const
{
    return m_aImplementation->m_eDisplayEmptyCellsAs;
}
void SparklineAttributes::setDisplayEmptyCellsAs(DisplayEmptyCellsAs eValue)
{
    m_aImplementation->m_eDisplayEmptyCellsAs = eValue;
}

bool SparklineAttributes::isMarkers() const { return m_aImplementation->m_bMarkers; }
void SparklineAttributes::setMarkers(bool bValue) { m_aImplementation->m_bMarkers = bValue; }

bool SparklineAttributes::isHigh() const { return m_aImplementation->m_bHigh; }
void SparklineAttributes::setHigh(bool bValue) { m_aImplementation->m_bHigh = bValue; }

bool SparklineAttributes::isLow() const { return m_aImplementation->m_bLow; }
void SparklineAttributes::setLow(bool bValue) { m_aImplementation->m_bLow = bValue; }

bool SparklineAttributes::isFirst() const { return m_aImplementation->m_bFirst; }
void SparklineAttributes::setFirst(bool bValue) { m_aImplementation->m_bFirst = bValue; }

bool SparklineAttributes::isLast() const { return m_aImplementation->m_bLast; }
void SparklineAttributes::setLast(bool bValue) { m_aImplementation->m_bLast = bValue; }

bool SparklineAttributes::isNegative() const { return m_aImplementation->m_bNegative; }
void SparklineAttributes::setNegative(bool bValue) { m_aImplementation->m_bNegative = bValue; }

bool SparklineAttributes::shouldDisplayXAxis() const { return m_aImplementation->m_bDisplayXAxis; }
void SparklineAttributes::setDisplayXAxis(bool bValue)
{
    m_aImplementation->m_bDisplayXAxis = bValue;
}

bool SparklineAttributes::shouldDisplayHidden() const
{
    return m_aImplementation->m_bDisplayHidden;
}
void SparklineAttributes::setDisplayHidden(bool bValue)
{
    m_aImplementation->m_bDisplayHidden = bValue;
}

bool SparklineAttributes::isRightToLeft() const { return m_aImplementation->m_bRightToLeft; }
void SparklineAttributes::setRightToLeft(bool bValue)
{
    m_aImplementation->m_bRightToLeft = bValue;
}

std::optional<double> SparklineAttributes::getManualMax() const
{
    return m_aImplementation->m_aManualMax;
}
void SparklineAttributes::setManualMax(std::optional<double> aValue)
{
    m_aImplementation->m_aManualMax = aValue;
}

std::optional<double> SparklineAttributes::getManualMin() const
{
    return m_aImplementation->m_aManualMin;
}
void SparklineAttributes::setManualMin(std::optional<double> aValue)
{
    m_aImplementation->m_aManualMin = aValue;
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
