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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_AXES_VCARTESIANAXIS_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_AXES_VCARTESIANAXIS_HXX

#include "VAxisBase.hxx"
#include <basegfx/vector/b2dvector.hxx>

namespace chart
{

class VCartesianAxis : public VAxisBase
{
    // public methods
public:
    VCartesianAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
           , PlottingPositionHelper* pPosHelper = NULL //takes ownership
           );

    virtual ~VCartesianAxis();

    virtual void createMaximumLabels() override;
    virtual void createLabels() override;
    virtual void updatePositions() override;

    virtual void createShapes() override;

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount() override;
    virtual void createAllTickInfos( TickInfoArraysType& rAllTickInfos ) override;
    void createAllTickInfosFromComplexCategories( TickInfoArraysType& rAllTickInfos, bool bShiftedPosition );

    TickIter* createLabelTickIterator( sal_Int32 nTextLevel );
    TickIter* createMaximumLabelTickIterator( sal_Int32 nTextLevel );
    sal_Int32 getTextLevelCount() const;

    virtual TickFactory* createTickFactory() override;

    /**
     * Get the value at which the other axis crosses.
     */
    double getAxisIntersectionValue() const;

    /**
     * Get the value at which label line crosses the other axis.
     */
    double getLabelLineIntersectionValue() const;

    /**
     * Get the value at which extra line crosses the other axis.
     *
     * @return a NaN if the line doesn't cross the other axis, a non-NaN value
     *         otherwise.
     */
    double getExtraLineIntersectionValue() const;

    void get2DAxisMainLine(
        basegfx::B2DVector& rStart, basegfx::B2DVector& rEnd, AxisLabelAlignment& rLabelAlignment,
        double fCrossesOtherAxis ) const;

    //Layout interface for cartesian axes:

    //the returned value describes the minimum size that is necessary
    //for the text labels in the direction orthogonal to the axis
    //(for an y-axis a width is returned; in case of an x-axis the value describes a height)
    //the return value is measured in screen dimension
    //As an example the MinimumOrthogonalSize of an x-axis equals the
    //Font Height if the label properties allow for labels parallel to the axis.
//    sal_Int32 calculateMinimumOrthogonalSize( /*... parallel...*/ );
    //Minimum->Preferred

    //returns true if the MinimumOrthogonalSize can be calculated
    //with the creation of at most one text shape
    //(this is e.g. true if the parameters allow for labels parallel to the axis.)
//    sal_bool  canQuicklyCalculateMinimumOrthogonalSize();

    struct ScreenPosAndLogicPos
    {
        double fLogicX;
        double fLogicY;
        double fLogicZ;

        ::basegfx::B2DVector aScreenPos;
    };

private: //methods
    /**
     * Go through all tick label positions and decide which labels to display
     * based on the text shape geometry, overlap setting, tick interval,
     * auto-stagger setting etc.
     *
     * When the auto-stagger setting is on, try to avoid overlaps by
     * staggering labels or set the labels at an angle.  This method may
     * change the axis label properties especially when the auto staggering is
     * performed.  But the screen label positions will not be shifted in this
     * method; it will be done in the doStaggeringOfLabels method.
     *
     * @return true if the text shapes have been successfully created,
     *         otherwise false.  Returning false means the AxisLabelProperties
     *         have changed during the call, and the caller needs to call this
     *         method once again to get the text shapes created.
     */
    bool createTextShapes(
        const css::uno::Reference<css::drawing::XShapes >& xTarget,
        TickIter& rTickIter, AxisLabelProperties& rAxisLabelProperties,
        TickFactory2D* pTickFactory, sal_Int32 nScreenDistanceBetweenTicks );

    /**
     * Variant of createTextShapes where none of auto-staggering and
     * link-breaking are allowed in case of overlaps.  Overlaps of text shapes
     * are to be resolved only by adjusting the label tick interval.
     */
    bool createTextShapesSimple(
        const css::uno::Reference<css::drawing::XShapes >& xTarget,
        TickIter& rTickIter, AxisLabelProperties& rAxisLabelProperties,
        TickFactory2D* pTickFactory );

    void createTickMarkLineShapes( TickInfoArrayType& rTickInfos, const TickmarkProperties& rTickmarkProperties, TickFactory2D& rTickFactory2D, bool bOnlyAtLabels );

    TickFactory2D* createTickFactory2D();
    void hideIdenticalScreenValues( TickInfoArraysType& rTickInfos ) const;

    /**
     * Shift the screen positions of the tick labels according to the stagger
     * settings.  Final stagger setting is decided during the createTextShapes
     * call, but this method does the physical shifting of the label
     * positions based on the final stagger setting.
     */
    void    doStaggeringOfLabels( const AxisLabelProperties& rAxisLabelProperties
                            , TickFactory2D* pTickFactory2D );

    /**
     * @return true if we can try to stagger labels in order to avoid
     *         overlaps, otherwise false.
     */
    static bool isAutoStaggeringOfLabelsAllowed(
        const AxisLabelProperties& rAxisLabelProperties, bool bIsHorizontalAxis, bool bIsVerticalAxis );

    /**
     * @return true if we can break a single line label text into multiple
     *         lines for better fitting, otherwise false.
     */
    bool isBreakOfLabelsAllowed( const AxisLabelProperties& rAxisLabelProperties, bool bIsHorizontalAxis ) const;

    ::basegfx::B2DVector getScreenPosition( double fLogicX, double fLogicY, double fLogicZ ) const;
    ScreenPosAndLogicPos getScreenPosAndLogicPos( double fLogicX, double fLogicY, double fLogicZ ) const;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
