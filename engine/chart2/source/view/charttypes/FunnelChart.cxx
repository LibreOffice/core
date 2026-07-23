/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "FunnelChart.hxx"

#include <ChartType.hxx>
#include <ChartModel.hxx>

#include <AxisIndexDefines.hxx>
#include <Clipping.hxx>
#include <CommonConverters.hxx>
#include <DateHelper.hxx>
#include <ObjectIdentifier.hxx>
#include <LabelPositionHelper.hxx>
#include <ShapeFactory.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/scopeguard.hxx>
#include <rtl/math.hxx>
#include <svx/scene3d.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>

namespace chart
{
FunnelChart::FunnelChart(const rtl::Reference<ChartType>& xChartTypeModel,
                         sal_Int32 nDimensionCount)
    : BarChart(xChartTypeModel, nDimensionCount)
{
    // We only support 2 dimensional funnel charts
    assert(nDimensionCount == 2 && "FunnelChart only supports 2D charts");

    // Runtime check for all builds
    if (nDimensionCount != 2)
    {
        // Log a warning or throw an exception if appropriate
        SAL_WARN("chart2", "FunnelChart created with invalid dimension count. Forcing 2D.");
    }

    PlotterBase::m_pPosHelper = &m_aMainPosHelper;
    VSeriesPlotter::m_pMainPosHelper = &m_aMainPosHelper;

    try
    {
        if (m_xChartTypeModel.is())
            m_xChartTypeModel->getPropertyValue(u"GapwidthSequence"_ustr) >>= m_aGapwidthSequence;
    }
    catch (const cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "");
    }

    m_bAlignCenter = true;
}

FunnelChart::~FunnelChart() {}

#if 0
PlottingPositionHelper& FunnelChart::getPlottingPositionHelper(sal_Int32 nAxisIndex) const
{
    PlottingPositionHelper& rPosHelper = VSeriesPlotter::getPlottingPositionHelper(nAxisIndex);
    BarPositionHelper* pFunnelPosHelper = dynamic_cast<BarPositionHelper*>(&rPosHelper);
    if (pFunnelPosHelper && nAxisIndex >= 0)
    {
        if (nAxisIndex < m_aGapwidthSequence.getLength())
            pFunnelPosHelper->setOuterDistance(m_aGapwidthSequence[nAxisIndex] / 100.0);
    }

    return rPosHelper;
}

css::awt::Point FunnelChart::getLabelScreenPositionAndAlignment(
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

    css::drawing::Position3D aScenePosition3D(
        pPosHelper->transformScaledLogicToScene(fScaledX, fY, 0.0, true));
    return LabelPositionHelper(2, m_xLogicTarget).transformSceneToScreenPosition(aScenePosition3D);
}
#endif

void FunnelChart::createShapes()
{
    if (m_aZSlots.empty()) //No data series Available
        return;

    OSL_ENSURE(m_xLogicTarget.is() && m_xFinalTarget.is(),
               "FunnelChart is not properly initialized"); // Ensuring chart is properly initialized
    if (!(m_xLogicTarget.is() && m_xFinalTarget.is()))
        return;

    if (!m_pMainPosHelper)
        m_pMainPosHelper = new BarPositionHelper();

    BarChart::createShapes();
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
