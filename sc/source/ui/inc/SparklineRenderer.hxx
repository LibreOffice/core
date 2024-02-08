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

#include <document.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <comphelper/scopeguard.hxx>

#include <Sparkline.hxx>
#include <SparklineGroup.hxx>
#include <SparklineAttributes.hxx>

namespace sc
{
/** Contains the marker polygon and the color of a marker */
struct SparklineMarker
{
    basegfx::B2DPolygon maPolygon;
    Color maColor;
};

/** Sparkline value and action that needs to me performed on the value */
struct SparklineValue
{
    enum class Action
    {
        None, // No action on the value
        Skip, // Skip the value
        Interpolate // Interpolate the value
    };

    double maValue;
    Action meAction;

    SparklineValue(double aValue, Action eAction)
        : maValue(aValue)
        , meAction(eAction)
    {
    }
};

/** Contains and manages the values of the sparkline.
 *
 * It automatically keeps track of the minimums and maximums, and
 * skips or interpolates the sparkline values if needed, depending on
 * the input. This is done so it is easier to handle the sparkline
 * values later on.
 */
class SparklineValues
{
private:
    double mfPreviousValue = 0.0;
    size_t mnPreviousIndex = std::numeric_limits<size_t>::max();

    std::vector<size_t> maToInterpolateIndex;

    std::vector<SparklineValue> maValueList;

public:
    size_t mnFirstIndex = std::numeric_limits<size_t>::max();
    size_t mnLastIndex = 0;

    double mfMinimum = std::numeric_limits<double>::max();
    double mfMaximum = std::numeric_limits<double>::lowest();

    std::vector<SparklineValue> const& getValuesList() const { return maValueList; }

    void add(double fValue, SparklineValue::Action eAction)
    {
        maValueList.emplace_back(fValue, eAction);
        size_t nCurrentIndex = maValueList.size() - 1;

        if (eAction == SparklineValue::Action::None)
        {
            mnLastIndex = nCurrentIndex;

            if (mnLastIndex < mnFirstIndex)
                mnFirstIndex = mnLastIndex;

            if (fValue < mfMinimum)
                mfMinimum = fValue;

            if (fValue > mfMaximum)
                mfMaximum = fValue;

            interpolatePastValues(fValue, nCurrentIndex);

            mnPreviousIndex = nCurrentIndex;
            mfPreviousValue = fValue;
        }
        else if (eAction == SparklineValue::Action::Interpolate)
        {
            maToInterpolateIndex.push_back(nCurrentIndex);
            maValueList.back().meAction = SparklineValue::Action::Skip;
        }
    }

    static constexpr double interpolate(double x1, double y1, double x2, double y2, double x)
    {
        return (y1 * (x2 - x) + y2 * (x - x1)) / (x2 - x1);
    }

    void interpolatePastValues(double nCurrentValue, size_t nCurrentIndex)
    {
        if (maToInterpolateIndex.empty())
            return;

        if (mnPreviousIndex == std::numeric_limits<size_t>::max())
        {
            for (size_t nIndex : maToInterpolateIndex)
            {
                auto& rValue = maValueList[nIndex];
                rValue.meAction = SparklineValue::Action::Skip;
            }
        }
        else
        {
            for (size_t nIndex : maToInterpolateIndex)
            {
                double fInterpolated = interpolate(mnPreviousIndex, mfPreviousValue, nCurrentIndex,
                                                   nCurrentValue, nIndex);

                auto& rValue = maValueList[nIndex];
                rValue.maValue = fInterpolated;
                rValue.meAction = SparklineValue::Action::None;
            }
        }
        maToInterpolateIndex.clear();
    }

    void convertToStacked()
    {
        // transform the data to 1, -1
        for (auto& rValue : maValueList)
        {
            if (rValue.maValue != 0.0)
            {
                double fNewValue = rValue.maValue > 0.0 ? 1.0 : -1.0;

                if (rValue.maValue == mfMinimum)
                    fNewValue -= 0.01;

                if (rValue.maValue == mfMaximum)
                    fNewValue += 0.01;

                rValue.maValue = fNewValue;
            }
        }
        mfMinimum = -1.01;
        mfMaximum = 1.01;
    }

    void reverse() { std::reverse(maValueList.begin(), maValueList.end()); }
};

/** Iterator to traverse the addresses in a range if the range is one dimensional.
 *
 * The direction to traverse is detected automatically or hasNext returns
 * false if it is not possible to detect.
 *
 */
class RangeTraverser
{
    enum class Direction
    {
        UNKNOWN,
        ROW,
        COLUMN
    };

    ScAddress m_aCurrent;
    ScRange m_aRange;
    Direction m_eDirection;

public:
    RangeTraverser(ScRange const& rRange)
        : m_aCurrent(ScAddress::INITIALIZE_INVALID)
        , m_aRange(rRange)
        , m_eDirection(Direction::UNKNOWN)

    {
    }

    ScAddress const& first()
    {
        m_aCurrent.SetInvalid();

        if (m_aRange.aStart.Row() == m_aRange.aEnd.Row())
        {
            m_eDirection = Direction::COLUMN;
            m_aCurrent = m_aRange.aStart;
        }
        else if (m_aRange.aStart.Col() == m_aRange.aEnd.Col())
        {
            m_eDirection = Direction::ROW;
            m_aCurrent = m_aRange.aStart;
        }

        return m_aCurrent;
    }

    bool hasNext()
    {
        if (m_eDirection == Direction::COLUMN)
            return m_aCurrent.Col() <= m_aRange.aEnd.Col();
        else if (m_eDirection == Direction::ROW)
            return m_aCurrent.Row() <= m_aRange.aEnd.Row();
        else
            return false;
    }

    void next()
    {
        if (hasNext())
        {
            if (m_eDirection == Direction::COLUMN)
                m_aCurrent.IncCol();
            else if (m_eDirection == Direction::ROW)
                m_aCurrent.IncRow();
        }
    }
};

/** Render a provided sparkline into the input rectangle */
class SparklineRenderer
{
private:
    ScDocument& mrDocument;
    tools::Long mnOneX;
    tools::Long mnOneY;

    double mfScaleX;
    double mfScaleY;

    void createMarker(std::vector<SparklineMarker>& rMarkers, double x, double y,
                      Color const& rColor)
    {
        auto& rMarker = rMarkers.emplace_back();
        const double nHalfSizeX = double(mnOneX * 2 * mfScaleX);
        const double nHalfSizeY = double(mnOneY * 2 * mfScaleY);
        basegfx::B2DRectangle aRectangle(std::round(x - nHalfSizeX), std::round(y - nHalfSizeY),
                                         std::round(x + nHalfSizeX), std::round(y + nHalfSizeY));
        rMarker.maPolygon = basegfx::utils::createPolygonFromRect(aRectangle);
        rMarker.maColor = rColor;
    }

    void drawLine(vcl::RenderContext& rRenderContext, tools::Rectangle const& rRectangle,
                  SparklineValues const& rSparklineValues,
                  sc::SparklineAttributes const& rAttributes)
    {
        double nMax = rSparklineValues.mfMaximum;
        if (rAttributes.getMaxAxisType() == sc::AxisType::Custom && rAttributes.getManualMax())
            nMax = *rAttributes.getManualMax();

        double nMin = rSparklineValues.mfMinimum;
        if (rAttributes.getMinAxisType() == sc::AxisType::Custom && rAttributes.getManualMin())
            nMin = *rAttributes.getManualMin();

        std::vector<SparklineValue> const& rValueList = rSparklineValues.getValuesList();
        std::vector<basegfx::B2DPolygon> aPolygons;
        aPolygons.emplace_back();
        double numebrOfSteps = rValueList.size() - 1;
        double xStep = 0;
        double nDelta = nMax - nMin;

        std::vector<SparklineMarker> aMarkers;
        size_t nValueIndex = 0;

        for (auto const& rSparklineValue : rValueList)
        {
            if (rSparklineValue.meAction == SparklineValue::Action::Skip)
            {
                aPolygons.emplace_back();
            }
            else
            {
                auto& aPolygon = aPolygons.back();
                double nValue = rSparklineValue.maValue;

                double nP = (nValue - nMin) / nDelta;
                double x = rRectangle.GetWidth() * (xStep / numebrOfSteps);
                double y = rRectangle.GetHeight() - rRectangle.GetHeight() * nP;

                aPolygon.append({ x, y });

                if (rAttributes.isFirst() && nValueIndex == rSparklineValues.mnFirstIndex)
                {
                    createMarker(aMarkers, x, y, rAttributes.getColorFirst().getFinalColor());
                }
                else if (rAttributes.isLast() && nValueIndex == rSparklineValues.mnLastIndex)
                {
                    createMarker(aMarkers, x, y, rAttributes.getColorLast().getFinalColor());
                }
                else if (rAttributes.isHigh() && nValue == rSparklineValues.mfMaximum)
                {
                    createMarker(aMarkers, x, y, rAttributes.getColorHigh().getFinalColor());
                }
                else if (rAttributes.isLow() && nValue == rSparklineValues.mfMinimum)
                {
                    createMarker(aMarkers, x, y, rAttributes.getColorLow().getFinalColor());
                }
                else if (rAttributes.isNegative() && nValue < 0.0)
                {
                    createMarker(aMarkers, x, y, rAttributes.getColorNegative().getFinalColor());
                }
                else if (rAttributes.isMarkers())
                {
                    createMarker(aMarkers, x, y, rAttributes.getColorMarkers().getFinalColor());
                }
            }

            xStep++;
            nValueIndex++;
        }

        basegfx::B2DHomMatrix aMatrix;
        aMatrix.translate(rRectangle.Left(), rRectangle.Top());

        if (rAttributes.shouldDisplayXAxis())
        {
            double nZero = 0 - nMin / nDelta;

            if (nZero >= 0) // if nZero < 0, the axis is not visible
            {
                double x1 = 0.0;
                double x2 = double(rRectangle.GetWidth());
                double y = rRectangle.GetHeight() - rRectangle.GetHeight() * nZero;

                basegfx::B2DPolygon aAxisPolygon;
                aAxisPolygon.append({ x1, y });
                aAxisPolygon.append({ x2, y });

                rRenderContext.SetLineColor(rAttributes.getColorAxis().getFinalColor());
                rRenderContext.DrawPolyLineDirect(aMatrix, aAxisPolygon, 0.2 * mfScaleX);
            }
        }

        rRenderContext.SetLineColor(rAttributes.getColorSeries().getFinalColor());

        for (auto& rPolygon : aPolygons)
        {
            rRenderContext.DrawPolyLineDirect(aMatrix, rPolygon,
                                              rAttributes.getLineWeight() * mfScaleX, 0.0, nullptr,
                                              basegfx::B2DLineJoin::Round);
        }

        for (auto& rMarker : aMarkers)
        {
            rRenderContext.SetLineColor(rMarker.maColor);
            rRenderContext.SetFillColor(rMarker.maColor);
            auto& rPolygon = rMarker.maPolygon;
            rPolygon.transform(aMatrix);
            rRenderContext.DrawPolygon(rPolygon);
        }
    }

    static void setFillAndLineColor(vcl::RenderContext& rRenderContext,
                                    sc::SparklineAttributes const& rAttributes, double nValue,
                                    size_t nValueIndex, SparklineValues const& rSparklineValues)
    {
        if (rAttributes.isFirst() && nValueIndex == rSparklineValues.mnFirstIndex)
        {
            rRenderContext.SetLineColor(rAttributes.getColorFirst().getFinalColor());
            rRenderContext.SetFillColor(rAttributes.getColorFirst().getFinalColor());
        }
        else if (rAttributes.isLast() && nValueIndex == rSparklineValues.mnLastIndex)
        {
            rRenderContext.SetLineColor(rAttributes.getColorLast().getFinalColor());
            rRenderContext.SetFillColor(rAttributes.getColorLast().getFinalColor());
        }
        else if (rAttributes.isHigh() && nValue == rSparklineValues.mfMaximum)
        {
            rRenderContext.SetLineColor(rAttributes.getColorHigh().getFinalColor());
            rRenderContext.SetFillColor(rAttributes.getColorHigh().getFinalColor());
        }
        else if (rAttributes.isLow() && nValue == rSparklineValues.mfMinimum)
        {
            rRenderContext.SetLineColor(rAttributes.getColorLow().getFinalColor());
            rRenderContext.SetFillColor(rAttributes.getColorLow().getFinalColor());
        }
        else if (rAttributes.isNegative() && nValue < 0.0)
        {
            rRenderContext.SetLineColor(rAttributes.getColorNegative().getFinalColor());
            rRenderContext.SetFillColor(rAttributes.getColorNegative().getFinalColor());
        }
        else
        {
            rRenderContext.SetLineColor(rAttributes.getColorSeries().getFinalColor());
            rRenderContext.SetFillColor(rAttributes.getColorSeries().getFinalColor());
        }
    }

    void drawColumn(vcl::RenderContext& rRenderContext, tools::Rectangle const& rRectangle,
                    SparklineValues const& rSparklineValues,
                    sc::SparklineAttributes const& rAttributes)
    {
        double nMax = rSparklineValues.mfMaximum;
        if (rAttributes.getMaxAxisType() == sc::AxisType::Custom && rAttributes.getManualMax())
            nMax = *rAttributes.getManualMax();

        double nMin = rSparklineValues.mfMinimum;
        if (rAttributes.getMinAxisType() == sc::AxisType::Custom && rAttributes.getManualMin())
            nMin = *rAttributes.getManualMin();

        std::vector<SparklineValue> const& rValueList = rSparklineValues.getValuesList();

        basegfx::B2DPolygon aPolygon;
        basegfx::B2DHomMatrix aMatrix;
        aMatrix.translate(rRectangle.Left(), rRectangle.Top());

        double xStep = 0;
        double numberOfSteps = rValueList.size();
        double nDelta = nMax - nMin;

        double nColumnSize = rRectangle.GetWidth() / numberOfSteps;
        nColumnSize = nColumnSize - (nColumnSize * 0.3);

        double nZero = (0 - nMin) / nDelta;
        double nZeroPosition = 0.0;
        if (nZero >= 0)
        {
            nZeroPosition = rRectangle.GetHeight() - rRectangle.GetHeight() * nZero;

            if (rAttributes.shouldDisplayXAxis())
            {
                double x1 = 0.0;
                double x2 = double(rRectangle.GetWidth());

                basegfx::B2DPolygon aAxisPolygon;
                aAxisPolygon.append({ x1, nZeroPosition });
                aAxisPolygon.append({ x2, nZeroPosition });

                rRenderContext.SetLineColor(rAttributes.getColorAxis().getFinalColor());
                rRenderContext.DrawPolyLineDirect(aMatrix, aAxisPolygon, 0.2 * mfScaleX);
            }
        }
        else
            nZeroPosition = rRectangle.GetHeight();

        size_t nValueIndex = 0;

        for (auto const& rSparklineValue : rValueList)
        {
            double nValue = rSparklineValue.maValue;

            if (nValue != 0.0)
            {
                setFillAndLineColor(rRenderContext, rAttributes, nValue, nValueIndex,
                                    rSparklineValues);

                double nP = (nValue - nMin) / nDelta;
                double x = rRectangle.GetWidth() * (xStep / numberOfSteps);
                double y = rRectangle.GetHeight() - rRectangle.GetHeight() * nP;

                basegfx::B2DRectangle aRectangle(x, y, x + nColumnSize, nZeroPosition);
                aPolygon = basegfx::utils::createPolygonFromRect(aRectangle);

                aPolygon.transform(aMatrix);
                rRenderContext.DrawPolygon(aPolygon);
            }
            xStep++;
            nValueIndex++;
        }
    }

    bool isCellHidden(ScAddress const& rAddress)
    {
        return mrDocument.RowHidden(rAddress.Row(), rAddress.Tab())
               || mrDocument.ColHidden(rAddress.Col(), rAddress.Tab());
    }

public:
    SparklineRenderer(ScDocument& rDocument)
        : mrDocument(rDocument)
        , mnOneX(1)
        , mnOneY(1)
        , mfScaleX(1.0)
        , mfScaleY(1.0)
    {
    }

    void render(std::shared_ptr<sc::Sparkline> const& pSparkline,
                vcl::RenderContext& rRenderContext, tools::Rectangle const& rRectangle,
                tools::Long nOneX, tools::Long nOneY, double fScaleX, double fScaleY)
    {
        rRenderContext.Push();
        comphelper::ScopeGuard aPushPopGuard([&rRenderContext]() { rRenderContext.Pop(); });

        rRenderContext.SetAntialiasing(AntialiasingFlags::Enable);
        rRenderContext.SetClipRegion(vcl::Region(rRectangle));

        tools::Rectangle aOutputRectangle(rRectangle);
        aOutputRectangle.shrink(6); // provide border

        mnOneX = nOneX;
        mnOneY = nOneY;
        mfScaleX = fScaleX;
        mfScaleY = fScaleY;

        auto const& rRangeList = pSparkline->getInputRange();

        if (rRangeList.empty())
        {
            return;
        }

        auto pSparklineGroup = pSparkline->getSparklineGroup();
        auto const& rAttributes = pSparklineGroup->getAttributes();

        ScRange aRange = rRangeList[0];

        SparklineValues aSparklineValues;

        RangeTraverser aTraverser(aRange);
        for (ScAddress const& rCurrent = aTraverser.first(); aTraverser.hasNext();
             aTraverser.next())
        {
            // Skip if the cell is hidden and "displayHidden" attribute is not selected
            if (!rAttributes.shouldDisplayHidden() && isCellHidden(rCurrent))
                continue;

            double fCellValue = 0.0;
            SparklineValue::Action eAction = SparklineValue::Action::None;
            CellType eType = mrDocument.GetCellType(rCurrent);

            if (eType == CELLTYPE_NONE) // if cell is empty
            {
                auto eDisplayEmpty = rAttributes.getDisplayEmptyCellsAs();
                if (eDisplayEmpty == sc::DisplayEmptyCellsAs::Gap)
                    eAction = SparklineValue::Action::Skip;
                else if (eDisplayEmpty == sc::DisplayEmptyCellsAs::Span)
                    eAction = SparklineValue::Action::Interpolate;
            }
            else
            {
                fCellValue = mrDocument.GetValue(rCurrent);
            }

            aSparklineValues.add(fCellValue, eAction);
        }

        if (rAttributes.isRightToLeft())
            aSparklineValues.reverse();

        if (rAttributes.getType() == sc::SparklineType::Column)
        {
            drawColumn(rRenderContext, aOutputRectangle, aSparklineValues,
                       pSparklineGroup->getAttributes());
        }
        else if (rAttributes.getType() == sc::SparklineType::Stacked)
        {
            aSparklineValues.convertToStacked();
            drawColumn(rRenderContext, aOutputRectangle, aSparklineValues,
                       pSparklineGroup->getAttributes());
        }
        else if (rAttributes.getType() == sc::SparklineType::Line)
        {
            drawLine(rRenderContext, aOutputRectangle, aSparklineValues,
                     pSparklineGroup->getAttributes());
        }
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
