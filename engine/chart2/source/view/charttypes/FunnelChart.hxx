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
class FunnelChart : public BarChart
{
public:
    FunnelChart() = delete;

    FunnelChart(const rtl::Reference<ChartType>& xChartTypeModel, sal_Int32 nDimensionCount);
    virtual ~FunnelChart() override;

    virtual void createShapes() override;

private:
#if 0
    virtual PlottingPositionHelper& getPlottingPositionHelper(sal_Int32 nAxisIndex) const override;

    css::awt::Point getLabelScreenPositionAndAlignment(LabelAlignment& rAlignment,
                                                       sal_Int32 nLabelPlacement, double fScaledX,
                                                       double fScaledLowerYValue,
                                                       double fScaledUpperYValue, double fBaseValue,
                                                       BarPositionHelper const* pPosHelper) const;
#endif

private:
    BarPositionHelper m_aMainPosHelper;
    cpo::uno::Sequence<sal_Int32> m_aGapwidthSequence;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
