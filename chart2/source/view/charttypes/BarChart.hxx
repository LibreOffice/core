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
#include "BarPositionHelper.hxx"

namespace chart
{

class BarChart : public VSeriesPlotter
{
    // public methods
public:
    BarChart() = delete;

    BarChart( const rtl::Reference< ::chart::ChartType >& xChartTypeModel
            , sal_Int32 nDimensionCount );
    virtual ~BarChart() override;

    virtual void createShapes() override;
    virtual void addSeries( std::unique_ptr<VDataSeries> pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot ) override;

    virtual css::drawing::Direction3D  getPreferredDiagramAspectRatio() const override;

private: //methods
    static rtl::Reference< SvxShape >
        createDataPoint3D_Bar(
                          const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
                        , const css::uno::Reference< css::beans::XPropertySet >& xObjectProperties
                        , sal_Int32 nGeometry3D );

    css::awt::Point getLabelScreenPositionAndAlignment(
                        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement
                        , double fScaledX, double fScaledLowerYValue, double fScaledUpperYValue, double fScaledZ
                        , double fScaledLowerBarDepth, double fScaledUpperBarDepth, double fBaseValue
                        , BarPositionHelper const * pPosHelper ) const;

    virtual PlottingPositionHelper& getPlottingPositionHelper( sal_Int32 nAxisIndex ) const override;//nAxisIndex indicates whether the position belongs to the main axis ( nAxisIndex==0 ) or secondary axis ( nAxisIndex==1 )

    void adaptOverlapAndGapwidthForGroupBarsPerAxis();

    //better performance for big data
    struct FormerBarPoint
    {
        FormerBarPoint( double fX, double fUpperY, double fLowerY, double fZ )
            : m_fX(fX), m_fUpperY(fUpperY), m_fLowerY(fLowerY), m_fZ(fZ)
            {}
        FormerBarPoint()
            : m_fX(std::numeric_limits<double>::quiet_NaN())
            , m_fUpperY(std::numeric_limits<double>::quiet_NaN())
            , m_fLowerY(std::numeric_limits<double>::quiet_NaN())
            , m_fZ(std::numeric_limits<double>::quiet_NaN())
        {
        }

        double m_fX;
        double m_fUpperY;
        double m_fLowerY;
        double m_fZ;
    };

    void doZSlot(
            bool& bDrawConnectionLines, bool& bDrawConnectionLinesInited, const std::vector< VDataSeriesGroup >& rZSlot,
            sal_Int32 nZ, sal_Int32 nPointIndex, sal_Int32 nStartIndex,
            const rtl::Reference<SvxShapeGroupAnyD>& xSeriesTarget,
            const rtl::Reference<SvxShapeGroupAnyD>& xRegressionCurveTarget,
            const rtl::Reference<SvxShapeGroupAnyD>& xRegressionCurveEquationTarget,
            const rtl::Reference<SvxShapeGroupAnyD>& xTextTarget,
            std::unordered_set<rtl::Reference<SvxShape>>& aShapeSet,
            std::map< VDataSeries*, FormerBarPoint >& aSeriesFormerPointMap,
            std::map< sal_Int32,  double >& aLogicYSumMap);

    void doXSlot(
            const VDataSeriesGroup& rXSlot,
            bool& bDrawConnectionLines, bool& bDrawConnectionLinesInited,
            sal_Int32 nZ, sal_Int32 nPointIndex, sal_Int32 nStartIndex,
            const rtl::Reference<SvxShapeGroupAnyD>& xSeriesTarget,
            const rtl::Reference<SvxShapeGroupAnyD>& xRegressionCurveTarget,
            const rtl::Reference<SvxShapeGroupAnyD>& xRegressionCurveEquationTarget,
            const rtl::Reference<SvxShapeGroupAnyD>& xTextTarget,
            std::unordered_set<rtl::Reference<SvxShape>>& aShapeSet,
            std::map< VDataSeries*, FormerBarPoint >& aSeriesFormerPointMap,
            std::map< sal_Int32,  double >& aLogicYSumMap,
            double fLogicBaseWidth, double fSlotX,
            BarPositionHelper* const pPosHelper,
            double fLogicPositiveYSum, double fLogicNegativeYSum,
            sal_Int32 nAttachedAxisIndex);

private: //member
    BarPositionHelper                    m_aMainPosHelper;
    css::uno::Sequence< sal_Int32 >      m_aOverlapSequence;
    css::uno::Sequence< sal_Int32 >      m_aGapwidthSequence;
};
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
