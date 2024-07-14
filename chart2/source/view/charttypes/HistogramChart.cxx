/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "HistogramChart.hxx"

#include <ChartType.hxx>
#include <../chart2/inc/ChartModel.hxx>
#include <ShapeFactory.hxx>
#include <CommonConverters.hxx>
#include <ObjectIdentifier.hxx>
#include <LabelPositionHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <Clipping.hxx>
#include <DateHelper.hxx>
#include <svx/scene3d.hxx>
#include <comphelper/scopeguard.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>

#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <rtl/math.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::com::sun::star::chart2;

HistogramChart::HistogramChart(const rtl::Reference<ChartType>& xChartTypeModel,
                               sal_Int32 nDimensionCount)
    : BarChart(xChartTypeModel, nDimensionCount)
{
    // We only support 2 dimensional histogram charts
    assert(nDimensionCount == 2 && "HistogramChart only supports 2D charts");

    // Runtime check for all builds
    if (nDimensionCount != 2)
    {
        // Log a warning or throw an exception if appropriate
        SAL_WARN("chart2", "HistogramChart created with invalid dimension count. Forcing 2D.");
    }

    PlotterBase::m_pPosHelper = &m_aMainPosHelper;
    VSeriesPlotter::m_pMainPosHelper = &m_aMainPosHelper;

    try
    {
        if (m_xChartTypeModel.is())
            m_xChartTypeModel->getPropertyValue(u"GapWidthSequence"_ustr) >>= m_aGapwidthSequence;
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "");
    }
}

HistogramChart::~HistogramChart() {}

css::drawing::Direction3D HistogramChart::getPreferredDiagramAspectRatio() const
{
    // Return the preferred aspect ratio for the histogram chart
    return css::drawing::Direction3D(-1, -1, -1);
}

PlottingPositionHelper& HistogramChart::getPlottingPositionHelper(sal_Int32 nAxisIndex) const
{
    PlottingPositionHelper& rPosHelper = VSeriesPlotter::getPlottingPositionHelper(nAxisIndex);
    BarPositionHelper* pHistogramPosHelper = dynamic_cast<BarPositionHelper*>(&rPosHelper);
    if (pHistogramPosHelper && nAxisIndex >= 0)
    {
        if (nAxisIndex < m_aGapwidthSequence.getLength())
            pHistogramPosHelper->setOuterDistance(m_aGapwidthSequence[nAxisIndex] / 100.0);
    }
    return rPosHelper;
}

awt::Point HistogramChart::getLabelScreenPositionAndAlignment(
    LabelAlignment& rAlignment, sal_Int32 nLabelPlacement, double fScaledX,
    double fScaledLowerYValue, double fScaledUpperYValue, double fBaseValue,
    BarPositionHelper const* pPosHelper) const
{
    double fY = fScaledUpperYValue;
    bool bReverse = !pPosHelper->isMathematicalOrientationY();
    bool bNormalOutside = (!bReverse == (fBaseValue < fScaledUpperYValue));

    switch (nLabelPlacement)
    {
        case css::chart::DataLabelPlacement::TOP:
            fY = bReverse ? fScaledLowerYValue : fScaledUpperYValue;
            rAlignment = LABEL_ALIGN_TOP;
            break;
        case css::chart::DataLabelPlacement::BOTTOM:
            fY = bReverse ? fScaledUpperYValue : fScaledLowerYValue;
            rAlignment = LABEL_ALIGN_BOTTOM;
            break;
        case css::chart::DataLabelPlacement::OUTSIDE:
            fY = (fBaseValue < fScaledUpperYValue) ? fScaledUpperYValue : fScaledLowerYValue;
            rAlignment = bNormalOutside ? LABEL_ALIGN_TOP : LABEL_ALIGN_BOTTOM;
            break;
        case css::chart::DataLabelPlacement::INSIDE:
            fY = (fBaseValue < fScaledUpperYValue) ? fScaledUpperYValue : fScaledLowerYValue;
            rAlignment = bNormalOutside ? LABEL_ALIGN_BOTTOM : LABEL_ALIGN_TOP;
            break;
        case css::chart::DataLabelPlacement::CENTER:
            fY -= (fScaledUpperYValue - fScaledLowerYValue) / 2.0;
            rAlignment = LABEL_ALIGN_CENTER;
            break;
        default:
            OSL_FAIL("this label alignment is not implemented yet");
            break;
    }

    drawing::Position3D aScenePosition3D(
        pPosHelper->transformScaledLogicToScene(fScaledX, fY, 0.0, true));
    return LabelPositionHelper(2, m_xLogicTarget).transformSceneToScreenPosition(aScenePosition3D);
}

void HistogramChart::addSeries(std::unique_ptr<VDataSeries> pSeries, sal_Int32 zSlot,
                               sal_Int32 xSlot, sal_Int32 ySlot)
{
    if (!pSeries)
        return;

    if (m_nDimension == 2)
    {
        sal_Int32 nAxisIndex = pSeries->getAttachedAxisIndex();
        zSlot = nAxisIndex;

        if (!pSeries->getGroupBarsPerAxis())
            zSlot = 0;
        if (zSlot >= static_cast<sal_Int32>(m_aZSlots.size()))
            m_aZSlots.resize(zSlot + 1);
    }
    VSeriesPlotter::addSeries(std::move(pSeries), zSlot, xSlot, ySlot);
    adaptGapWidthForHistogram();
}

void HistogramChart::adaptGapWidthForHistogram()
{
    VDataSeries* pFirstSeries = getFirstSeries();
    if (!pFirstSeries)
        return;

    sal_Int32 nAxisIndex = pFirstSeries->getAttachedAxisIndex();
    sal_Int32 nUseThisIndex = nAxisIndex;
    if (nUseThisIndex < 0 || nUseThisIndex >= m_aGapwidthSequence.getLength())
        nUseThisIndex = 0;

    auto aGapwidthSequenceRange = asNonConstRange(m_aGapwidthSequence);
    for (sal_Int32 nN = 0; nN < m_aGapwidthSequence.getLength(); nN++)
    {
        aGapwidthSequenceRange[nN] = 0; // Set gap width to 0
    }
}

void HistogramChart::createShapes()
{
    if (m_aZSlots.empty()) //No data series Available
        return;

    OSL_ENSURE(
        m_xLogicTarget.is() && m_xFinalTarget.is(),
        "HistogramChart is not properly initialized"); // Ensuring chart is properly initialized
    if (!(m_xLogicTarget.is() && m_xFinalTarget.is()))
        return;

    adaptGapWidthForHistogram();

    BarChart::createShapes();
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
