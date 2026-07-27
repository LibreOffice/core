/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CorrelationCircleChart.hxx"
#include <CorrelationCoefficient.hxx>
#include <PlottingPositionHelper.hxx>
#include <ShapeFactory.hxx>
#include <ObjectIdentifier.hxx>
#include <LabelPositionHelper.hxx>
#include <VLineProperties.hxx>
#include <AxisIndexDefines.hxx>
#include <ChartType.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <osl/diagnose.h>

#include <span>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace
{
// The circle and the two lines through the origin are the frame the arrows are
// read against, so they are drawn in a colour that stays behind the data.
constexpr sal_Int32 nGuideColor = 0x808080;

// The dimensions sit at a depth of one, as elsewhere in the two dimensional
// plotters.
constexpr double fLogicZ = 1.0;

std::span<const double> lclAsSpan(const uno::Sequence<double>& rValues)
{
    return std::span<const double>(rValues.getConstArray(), rValues.getLength());
}

} // anonymous namespace

CorrelationCircleChart::CorrelationCircleChart(const rtl::Reference<ChartType>& xChartTypeModel,
                                               sal_Int32 nDimensionCount)
    : VSeriesPlotter(xChartTypeModel, nDimensionCount, false)
{
    // A correlation circle plot has two dimensions and no more.
    assert(nDimensionCount == 2);

    if (!m_pMainPosHelper)
        m_pMainPosHelper = new PlottingPositionHelper();
    PlotterBase::m_pPosHelper = m_pMainPosHelper;
}

CorrelationCircleChart::~CorrelationCircleChart() { delete m_pMainPosHelper; }

double CorrelationCircleChart::getMinimumX() { return -1.0; }

double CorrelationCircleChart::getMaximumX() { return 1.0; }

std::pair<double, double>
CorrelationCircleChart::getMinimumAndMaximumYInRange(double /*fMinimumX*/, double /*fMaximumX*/,
                                                     sal_Int32 /*nAxisIndex*/)
{
    return { -1.0, 1.0 };
}

bool CorrelationCircleChart::isExpandBorderToIncrementRhythm(sal_Int32 /*nDimensionIndex*/)
{
    return false;
}

bool CorrelationCircleChart::isExpandIfValuesCloseToBorder(sal_Int32 /*nDimensionIndex*/)
{
    return false;
}

bool CorrelationCircleChart::isExpandWideValuesToZero(sal_Int32 /*nDimensionIndex*/)
{
    return false;
}

bool CorrelationCircleChart::isExpandNarrowValuesTowardZero(sal_Int32 /*nDimensionIndex*/)
{
    return false;
}

bool CorrelationCircleChart::isSeparateStackingForDifferentSigns(sal_Int32 /*nDimensionIndex*/)
{
    return false;
}

LegendSymbolStyle CorrelationCircleChart::getLegendSymbolStyle() { return LegendSymbolStyle::Line; }

uno::Any CorrelationCircleChart::getExplicitSymbol(const VDataSeries& rSeries,
                                                   sal_Int32 nPointIndex)
{
    uno::Any aRet;

    Symbol* pSymbolProperties = rSeries.getSymbolProperties(nPointIndex);
    if (pSymbolProperties)
        aRet <<= *pSymbolProperties;

    return aRet;
}

drawing::Direction3D CorrelationCircleChart::getPreferredDiagramAspectRatio() const
{
    // Equal width and height keep the circle round and the two correlations at
    // the same scale.
    return drawing::Direction3D(1, 1, 1);
}

void CorrelationCircleChart::createUnitCircleAndAxisLines(
    const rtl::Reference<SvxShapeGroupAnyD>& xTarget)
{
    PlottingPositionHelper& rPosHelper = getPlottingPositionHelper(MAIN_AXIS_INDEX);

    const drawing::Position3D aOrigin(rPosHelper.transformLogicToScene(0.0, 0.0, fLogicZ, false));
    const drawing::Position3D aLowerLeft(
        rPosHelper.transformLogicToScene(-1.0, -1.0, fLogicZ, false));
    const drawing::Position3D aUpperRight(
        rPosHelper.transformLogicToScene(1.0, 1.0, fLogicZ, false));

    const drawing::Direction3D aCircleSize(std::abs(aUpperRight.PositionX - aLowerLeft.PositionX),
                                           std::abs(aUpperRight.PositionY - aLowerLeft.PositionY),
                                           0.0);

    rtl::Reference<SvxShapeCircle> xCircle
        = ShapeFactory::createCircle2D(xTarget, aOrigin, aCircleSize);
    if (xCircle.is())
    {
        xCircle->SvxShape::setPropertyValue(u"FillStyle"_ustr, uno::Any(drawing::FillStyle_NONE));
        xCircle->SvxShape::setPropertyValue(u"LineStyle"_ustr, uno::Any(drawing::LineStyle_SOLID));
        xCircle->SvxShape::setPropertyValue(u"LineColor"_ustr, uno::Any(nGuideColor));
    }

    VLineProperties aLineProperties;
    aLineProperties.Color <<= nGuideColor;
    aLineProperties.LineStyle <<= drawing::LineStyle_SOLID;

    // The two lines span the whole plot area, which may be wider than the
    // circle when the axis rounds its range outwards.
    const double fMinX = rPosHelper.getLogicMinX();
    const double fMaxX = rPosHelper.getLogicMaxX();
    const double fMinY = rPosHelper.getLogicMinY();
    const double fMaxY = rPosHelper.getLogicMaxY();

    std::vector<std::vector<drawing::Position3D>> aHorizontalLine{
        { rPosHelper.transformLogicToScene(fMinX, 0.0, fLogicZ, false),
          rPosHelper.transformLogicToScene(fMaxX, 0.0, fLogicZ, false) }
    };
    ShapeFactory::createLine2D(xTarget, aHorizontalLine, &aLineProperties);

    std::vector<std::vector<drawing::Position3D>> aVerticalLine{
        { rPosHelper.transformLogicToScene(0.0, fMinY, fLogicZ, false),
          rPosHelper.transformLogicToScene(0.0, fMaxY, fLogicZ, false) }
    };
    ShapeFactory::createLine2D(xTarget, aVerticalLine, &aLineProperties);
}

void CorrelationCircleChart::createSeriesShapes(
    const std::unique_ptr<VDataSeries>& pSeries,
    const rtl::Reference<SvxShapeGroupAnyD>& xSeriesTarget,
    const rtl::Reference<SvxShapeGroupAnyD>& xTextTarget)
{
    const std::span<const double> aFeatureValues(lclAsSpan(pSeries->getAllFeatureValues()));
    const std::optional<double> oCorrelationX(
        calculateCorrelationCoefficient(aFeatureValues, lclAsSpan(pSeries->getAllX())));
    const std::optional<double> oCorrelationY(
        calculateCorrelationCoefficient(aFeatureValues, lclAsSpan(pSeries->getAllY())));

    // A feature that does not vary, or that has fewer than two instances to
    // compare, has no place in the circle.
    if (!oCorrelationX.has_value() || !oCorrelationY.has_value())
        return;

    PlottingPositionHelper& rPosHelper = getPlottingPositionHelper(pSeries->getAttachedAxisIndex());
    m_pPosHelper = &rPosHelper;

    const drawing::Position3D aOrigin(rPosHelper.transformLogicToScene(0.0, 0.0, fLogicZ, false));
    const drawing::Position3D aTip(
        rPosHelper.transformLogicToScene(*oCorrelationX, *oCorrelationY, fLogicZ, false));

    rtl::Reference<SvxShapeGroupAnyD> xSeriesGroupShape_Shapes
        = getSeriesGroupShape(pSeries.get(), xSeriesTarget);

    // The single point of the series stands for the whole feature.
    const sal_Int32 nPointIndex = 0;
    const OUString aPointCID
        = ObjectIdentifier::createPointCID(pSeries->getPointCID_Stub(), nPointIndex);
    rtl::Reference<SvxShapeGroupAnyD> xPointGroupShape_Shapes(
        createGroupShape(xSeriesGroupShape_Shapes, aPointCID));

    uno::Reference<beans::XPropertySet> xPointProperties(
        pSeries->getPropertiesOfPoint(nPointIndex));

    VLineProperties aLineProperties;
    aLineProperties.initFromPropertySet(xPointProperties);
    if (aLineProperties.isLineVisible())
    {
        std::vector<std::vector<drawing::Position3D>> aArrow{ { aOrigin, aTip } };
        rtl::Reference<SvxShapePolyPolygon> xArrow
            = ShapeFactory::createLine2D(xPointGroupShape_Shapes, aArrow);
        PropertyMapper::setMappedProperties(
            *xArrow, xPointProperties, PropertyMapper::getPropertyNameMapForLineSeriesProperties());
    }

    drawing::Direction3D aSymbolSize(0, 0, 0);
    Symbol* pSymbolProperties = pSeries->getSymbolProperties(nPointIndex);
    if (pSymbolProperties && pSymbolProperties->Style != SymbolStyle_NONE)
    {
        aSymbolSize.DirectionX = pSymbolProperties->Size.Width;
        aSymbolSize.DirectionY = pSymbolProperties->Size.Height;

        if (pSymbolProperties->Style == SymbolStyle_STANDARD)
            ShapeFactory::createSymbol2D(
                xPointGroupShape_Shapes, aTip, aSymbolSize, pSymbolProperties->StandardSymbol,
                pSymbolProperties->BorderColor, pSymbolProperties->FillColor);
        else if (pSymbolProperties->Style == SymbolStyle_GRAPHIC)
            ShapeFactory::createGraphic2D(xPointGroupShape_Shapes, aTip, aSymbolSize,
                                          pSymbolProperties->Graphic);
    }

    if (pSeries->getDataPointLabelIfLabel(nPointIndex))
    {
        drawing::Position3D aLabelPosition(aTip.PositionX, aTip.PositionY,
                                           aTip.PositionZ + getTransformedDepth());
        LabelAlignment eAlignment = LABEL_ALIGN_TOP;

        switch (
            pSeries->getLabelPlacement(nPointIndex, m_xChartTypeModel, rPosHelper.isSwapXAndY()))
        {
            case css::chart::DataLabelPlacement::BOTTOM:
                aLabelPosition.PositionY += (aSymbolSize.DirectionY / 2 + 1);
                eAlignment = LABEL_ALIGN_BOTTOM;
                break;
            case css::chart::DataLabelPlacement::LEFT:
                aLabelPosition.PositionX -= (aSymbolSize.DirectionX / 2 + 1);
                eAlignment = LABEL_ALIGN_LEFT;
                break;
            case css::chart::DataLabelPlacement::RIGHT:
                aLabelPosition.PositionX += (aSymbolSize.DirectionX / 2 + 1);
                eAlignment = LABEL_ALIGN_RIGHT;
                break;
            case css::chart::DataLabelPlacement::CENTER:
                eAlignment = LABEL_ALIGN_CENTER;
                break;
            default:
                aLabelPosition.PositionY -= (aSymbolSize.DirectionY / 2 + 1);
                eAlignment = LABEL_ALIGN_TOP;
                break;
        }

        const awt::Point aScreenPosition2D(LabelPositionHelper(m_nDimension, m_xLogicTarget)
                                               .transformSceneToScreenPosition(aLabelPosition));
        const sal_Int32 nOffset = (eAlignment == LABEL_ALIGN_CENTER) ? 0 : 100;

        // The correlation with the second dimension is the number the point
        // stands at, and one is the whole a correlation can be.
        createDataLabel(xTextTarget, *pSeries, nPointIndex, *oCorrelationY, 1.0, aScreenPosition2D,
                        eAlignment, nOffset);
    }

    if (!xPointGroupShape_Shapes->getCount())
        xSeriesGroupShape_Shapes->remove(xPointGroupShape_Shapes);
}

void CorrelationCircleChart::createShapes()
{
    if (m_aZSlots.empty()) //no series
        return;

    OSL_ENSURE(m_xLogicTarget.is() && m_xFinalTarget.is(),
               "CorrelationCircleChart is not proper initialized");
    if (!(m_xLogicTarget.is() && m_xFinalTarget.is()))
        return;

    // The guides go into their own group first, so that the arrows and the
    // labels are drawn over them.
    rtl::Reference<SvxShapeGroupAnyD> xGuideTarget = createGroupShape(m_xLogicTarget);
    rtl::Reference<SvxShapeGroupAnyD> xSeriesTarget = createGroupShape(m_xLogicTarget);
    rtl::Reference<SvxShapeGroup> xTextTarget = ShapeFactory::createGroup2D(m_xFinalTarget);

    createUnitCircleAndAxisLines(xGuideTarget);

    for (auto const& rZSlot : m_aZSlots)
    {
        for (auto const& rXSlot : rZSlot)
        {
            for (std::unique_ptr<VDataSeries> const& pSeries : rXSlot.m_aSeriesVector)
            {
                if (!pSeries)
                    continue;

                createSeriesShapes(pSeries, xSeriesTarget, xTextTarget);
            }
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
