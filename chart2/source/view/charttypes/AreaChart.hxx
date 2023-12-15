/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <memory>
#include <VSeriesPlotter.hxx>
#include <PlottingPositionHelper.hxx>
#include <com/sun/star/chart2/CurveStyle.hpp>

namespace chart
{

class AreaChart : public VSeriesPlotter
{
    // public methods
public:
    AreaChart() = delete;

    AreaChart( const rtl::Reference< ::chart::ChartType >& xChartTypeModel
             , sal_Int32 nDimensionCount
             , bool bCategoryXAxis, bool bNoArea=false
             );
    virtual ~AreaChart() override;

    virtual void createShapes() override;
    virtual void addSeries( std::unique_ptr<VDataSeries> pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot ) override;

    virtual css::drawing::Direction3D  getPreferredDiagramAspectRatio() const override;

    // MinimumAndMaximumSupplier
    virtual bool isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex ) override;

    virtual LegendSymbolStyle getLegendSymbolStyle() override;
    virtual css::uno::Any getExplicitSymbol( const VDataSeries& rSeries, sal_Int32 nPointIndex/*-1 for series symbol*/ ) override;

private: //methods
    void impl_createSeriesShapes();
    bool impl_createArea( VDataSeries* pSeries
                , std::vector<std::vector<css::drawing::Position3D>> const * pSeriesPoly
                , std::vector<std::vector<css::drawing::Position3D>> const * pPreviousSeriesPoly
                , PlottingPositionHelper const * pPosHelper );
    bool impl_createLine( VDataSeries* pSeries
                , std::vector<std::vector<css::drawing::Position3D>> const * pSeriesPoly
                , PlottingPositionHelper* pPosHelper );
    static bool create_stepped_line( std::vector<std::vector<css::drawing::Position3D>> aStartPoly
                , css::chart2::CurveStyle eCurveStyle
                , PlottingPositionHelper const * pPosHelper
                , std::vector<std::vector<css::drawing::Position3D>> &aPoly );

private: //member
    PlottingPositionHelper              m_aMainPosHelper;

    bool                                m_bArea;//false -> line or symbol only
    bool                                m_bLine;
    bool                                m_bSymbol;

    //Properties for splines:
    css::chart2::CurveStyle             m_eCurveStyle;
    sal_Int32                           m_nCurveResolution;
    sal_Int32                           m_nSplineOrder;

    rtl::Reference<SvxShapeGroupAnyD> m_xSeriesTarget;
    rtl::Reference<SvxShapeGroupAnyD> m_xErrorBarTarget;
    rtl::Reference<SvxShapeGroupAnyD> m_xTextTarget;
    rtl::Reference<SvxShapeGroupAnyD> m_xRegressionCurveEquationTarget;
};
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
