/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "BarChart.hxx"
#include "BarPositionHelper.hxx"
#include <memory>

namespace chart
{
class HistogramChart : public BarChart
{
public:
    HistogramChart() = delete;

    HistogramChart(const rtl::Reference<ChartType>& xChartTypeModel, sal_Int32 nDimensionCount);
    virtual ~HistogramChart() override;

    virtual void createShapes() override;
    virtual void addSeries(std::unique_ptr<VDataSeries> pSeries, sal_Int32 zSlot, sal_Int32 xSlot,
                           sal_Int32 ySlot) override;

    virtual css::drawing::Direction3D getPreferredDiagramAspectRatio() const override;

private:
    css::awt::Point getLabelScreenPositionAndAlignment(LabelAlignment& rAlignment,
                                                       sal_Int32 nLabelPlacement, double fScaledX,
                                                       double fScaledLowerYValue,
                                                       double fScaledUpperYValue, double fBaseValue,
                                                       BarPositionHelper const* pPosHelper) const;

    virtual PlottingPositionHelper& getPlottingPositionHelper(sal_Int32 nAxisIndex) const override;

    void adaptGapWidthForHistogram();

private:
    BarPositionHelper m_aMainPosHelper;
    css::uno::Sequence<sal_Int32> m_aGapwidthSequence;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
