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

#ifndef INCLUDED_CHART2_SOURCE_VIEW_CHARTTYPES_PIECHART_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_CHARTTYPES_PIECHART_HXX

#include "VSeriesPlotter.hxx"
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2irectangle.hxx>

namespace chart
{
class PiePositionHelper;

class PieChart : public VSeriesPlotter
{
    struct ShapeParam;

public:
    PieChart() = delete;

    PieChart( const css::uno::Reference< css::chart2::XChartType >& xChartTypeModel
            , sal_Int32 nDimensionCount, bool bExcludingPositioning );
    virtual ~PieChart();

    /** This method creates all shapes needed for representing the pie chart.
     */
    virtual void createShapes() override;
    virtual void rearrangeLabelToAvoidOverlapIfRequested( const css::awt::Size& rPageSize ) override;

    virtual void setScales( const ::std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis ) override;
    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 ) override;

    virtual css::drawing::Direction3D  getPreferredDiagramAspectRatio() const override;
    virtual bool shouldSnapRectToUsedArea() override;

    //MinimumAndMaximumSupplier
    virtual double getMinimumX() override;
    virtual double getMaximumX() override;
    virtual double getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex ) override;
    virtual double getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex ) override;

    virtual bool isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex ) override;
    virtual bool isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex ) override;
    virtual bool isExpandWideValuesToZero( sal_Int32 nDimensionIndex ) override;
    virtual bool isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex ) override;
    virtual bool isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex ) override;

private: //methods
    css::uno::Reference<css::drawing::XShape>
        createDataPoint(
            const css::uno::Reference<css::drawing::XShapes>& xTarget,
            const css::uno::Reference<css::beans::XPropertySet>& xObjectProperties,
            tPropertyNameValueMap* pOverWritePropertiesMap,
            const ShapeParam& rParam );

    /** This method creates a text shape for a label of a data point.
     *
     *  @param xTextTarget
     *      where to append the new created text shape.
     *  @param rSeries
     *      the data series, the data point belongs to.
     *  @param nPointIndex
     *      the index of the data point the label is related to.
     *  @param rParam
     *      ShapeParam object.
     */
    void createTextLabelShape(
        const css::uno::Reference<css::drawing::XShapes>& xTextTarget,
        VDataSeries& rSeries, sal_Int32 nPointIndex, ShapeParam& rParam );

    /** This method sets `m_fMaxOffset` to the maximum `Offset` property and
     *  returns it. There is a `Offset` property for each entry in a data
     *  series, moreover there exists a shared `Offset` property attached to
     *  the whole data series. The `Offset` property represents the
     *  relative distance offset of a slice from the pie center.
     *  The shared property is used for exploded pie chart, while the property
     *  attached to single data series entries is used for manual dragging of
     *  a slice.
     *  `m_fMaxOffset` is used by `PiePositionHelper::getInnerAndOuterRadius`.
     *  Note that only the `Offset` properties of the first (x slot) data series
     *  and its entries are utilized for computing the maximum offset.
     */
    double              getMaxOffset();
    bool                detectLabelOverlapsAndMove(const css::awt::Size& rPageSize);//returns true when there might be more to do
    void                resetLabelPositionsToPreviousState();
struct PieLabelInfo;
    bool                tryMoveLabels( PieLabelInfo* pFirstBorder, PieLabelInfo* pSecondBorder
                                , PieLabelInfo* pCenter, bool bSingleCenter, bool& rbAlternativeMoveDirection
                                , const css::awt::Size& rPageSize );

    bool                performLabelBestFitInnerPlacement(ShapeParam& rShapeParam, PieLabelInfo& rPieLabelInfo);
    static bool         performLabelBestFitOuterPlacement(ShapeParam& rShapeParam, PieLabelInfo& rPieLabelInfo);
    void                performLabelBestFit(ShapeParam& rShapeParam, PieLabelInfo& rPieLabelInfo);

private: //member
    PiePositionHelper*    m_pPosHelper;
    bool                  m_bUseRings;
    bool                  m_bSizeExcludesLabelsAndExplodedSegments;

    struct PieLabelInfo
    {
        PieLabelInfo();
        bool moveAwayFrom( const PieLabelInfo* pFix, const css::awt::Size& rPageSize
            , bool bMoveHalfWay, bool bMoveClockwise, bool bAlternativeMoveDirection );

        css::uno::Reference< css::drawing::XShape > xTextShape;
        css::uno::Reference< css::drawing::XShape > xLabelGroupShape;
        ::basegfx::B2IVector aFirstPosition;
        ::basegfx::B2IVector aOrigin;
        double fValue;
        bool bMovementAllowed;
        bool bMoved;
        css::uno::Reference< css::drawing::XShapes > xTextTarget;
        PieLabelInfo* pPrevious;
        PieLabelInfo* pNext;
        css::awt::Point aPreviousPosition;
    };

    ::std::vector< PieLabelInfo > m_aLabelInfoList;

    double m_fMaxOffset;    /// cached max offset value (init'ed to NaN)
};
} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
