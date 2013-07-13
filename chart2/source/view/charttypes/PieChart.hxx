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

#ifndef _CHART2_PIECHART_HXX
#define _CHART2_PIECHART_HXX

#include "VSeriesPlotter.hxx"
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2irectangle.hxx>

namespace chart
{
class PiePositionHelper;

class PieChart : public VSeriesPlotter
{
    // public methods
public:
    PieChart( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >& xChartTypeModel
            , sal_Int32 nDimensionCount, bool bExcludingPositioning );
    virtual ~PieChart();

    virtual void createShapes();
    virtual void rearrangeLabelToAvoidOverlapIfRequested( const ::com::sun::star::awt::Size& rPageSize );

    virtual void setScales( const ::std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis );
    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    virtual ::com::sun::star::drawing::Direction3D  getPreferredDiagramAspectRatio() const;
    virtual bool keepAspectRatio() const;
    virtual bool shouldSnapRectToUsedArea();

    //MinimumAndMaximumSupplier
    virtual double getMinimumX();
    virtual double getMaximumX();
    virtual double getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex );
    virtual double getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex );

    virtual bool isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex );
    virtual bool isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex );
    virtual bool isExpandWideValuesToZero( sal_Int32 nDimensionIndex );
    virtual bool isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex );
    virtual bool isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex );

private: //methods
    //no default constructor
    PieChart();

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createDataPoint(  const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::uno::Reference<
                                ::com::sun::star::beans::XPropertySet >& xObjectProperties
                        , double fUnitCircleStartAngleDegree, double fWidthAngleDegree
                        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                        , double fLogicZ, double fDepth, double fExplodePercentage
                        , tPropertyNameValueMap* pOverWritePropertiesMap );

    double              getMaxOffset();
    bool                detectLabelOverlapsAndMove(const ::com::sun::star::awt::Size& rPageSize);//returns true when there might be more to do
    void                resetLabelPositionsToPreviousState();
struct PieLabelInfo;
    bool                tryMoveLabels( PieLabelInfo* pFirstBorder, PieLabelInfo* pSecondBorder
                                , PieLabelInfo* pCenter, bool bSingleCenter, bool& rbAlternativeMoveDirection
                                , const ::com::sun::star::awt::Size& rPageSize );

private: //member
    PiePositionHelper*    m_pPosHelper;
    bool                  m_bUseRings;
    bool                  m_bSizeExcludesLabelsAndExplodedSegments;

    struct PieLabelInfo
    {
        PieLabelInfo();
        bool moveAwayFrom( const PieLabelInfo* pFix, const ::com::sun::star::awt::Size& rPageSize
            , bool bMoveHalfWay, bool bMoveClockwise, bool bAlternativeMoveDirection );

        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xTextShape;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xLabelGroupShape;
        ::basegfx::B2IVector aFirstPosition;
        ::basegfx::B2IVector aOrigin;
        double fValue;
        bool bMovementAllowed;
        bool bMoved;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > xTextTarget;
        PieLabelInfo* pPrevious;
        PieLabelInfo* pNext;
        ::com::sun::star::awt::Point aPreviousPosition;
    };

    ::std::vector< PieLabelInfo > m_aLabelInfoList;

    double m_fMaxOffset;    /// cached max offset value (init'ed to NaN)
};
} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
