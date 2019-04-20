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

namespace chart
{
class BarPositionHelper;

class BarChart : public VSeriesPlotter
{
    // public methods
public:
    BarChart() = delete;

    BarChart( const css::uno::Reference< css::chart2::XChartType >& xChartTypeModel
            , sal_Int32 nDimensionCount );
    virtual ~BarChart() override;

    virtual void createShapes() override;
    virtual void addSeries( std::unique_ptr<VDataSeries> pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot ) override;

    virtual css::drawing::Direction3D  getPreferredDiagramAspectRatio() const override;

private: //methods
    css::uno::Reference< css::drawing::XShape >
        createDataPoint3D_Bar(
                          const css::uno::Reference< css::drawing::XShapes >& xTarget
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

private: //member
    std::unique_ptr<BarPositionHelper>   m_pMainPosHelper;
    css::uno::Sequence< sal_Int32 >      m_aOverlapSequence;
    css::uno::Sequence< sal_Int32 >      m_aGapwidthSequence;
};
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
