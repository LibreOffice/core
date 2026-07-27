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
#pragma once

#include <VSeriesPlotter.hxx>
#include <com/sun/star/drawing/Direction3D.hpp>

namespace chart
{
/** Draws a correlation circle plot: one arrow per series, from the origin to
    the point whose coordinates are the correlation of that series' feature
    column with each of the two dimension columns. Both axes cover minus one to
    one, the range a correlation has.
 */
class CorrelationCircleChart final : public VSeriesPlotter
{
public:
    CorrelationCircleChart() = delete;

    CorrelationCircleChart(const rtl::Reference<::chart::ChartType>& xChartTypeModel,
                           sal_Int32 nDimensionCount);
    virtual ~CorrelationCircleChart() override;

    virtual void createShapes() override;

    virtual css::drawing::Direction3D getPreferredDiagramAspectRatio() const override;

    // MinimumAndMaximumSupplier
    virtual double getMinimumX() override;
    virtual double getMaximumX() override;
    virtual std::pair<double, double>
    getMinimumAndMaximumYInRange(double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex) override;
    virtual bool isExpandBorderToIncrementRhythm(sal_Int32 nDimensionIndex) override;
    virtual bool isExpandIfValuesCloseToBorder(sal_Int32 nDimensionIndex) override;
    virtual bool isExpandWideValuesToZero(sal_Int32 nDimensionIndex) override;
    virtual bool isExpandNarrowValuesTowardZero(sal_Int32 nDimensionIndex) override;
    virtual bool isSeparateStackingForDifferentSigns(sal_Int32 nDimensionIndex) override;

    virtual LegendSymbolStyle getLegendSymbolStyle() override;
    virtual css::uno::Any getExplicitSymbol(const VDataSeries& rSeries,
                                            sal_Int32 nPointIndex) override;

private:
    void createUnitCircleAndAxisLines(const rtl::Reference<SvxShapeGroupAnyD>& xTarget);
    void createSeriesShapes(const std::unique_ptr<VDataSeries>& pSeries,
                            const rtl::Reference<SvxShapeGroupAnyD>& xSeriesTarget,
                            const rtl::Reference<SvxShapeGroupAnyD>& xTextTarget);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
