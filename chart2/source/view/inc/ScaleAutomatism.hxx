/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHART2_SCALEAUTOMATISM_HXX
#define _CHART2_SCALEAUTOMATISM_HXX

#include "chartview/ExplicitScaleValues.hxx"
#include <com/sun/star/chart2/ScaleData.hpp>

#include <tools/date.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------

/** This class implements the calculation of automatic axis limits.
*/
class ScaleAutomatism
{
public:
    explicit            ScaleAutomatism(
                            const ::com::sun::star::chart2::ScaleData& rSourceScale, const Date& rNullDate );
    virtual             ~ScaleAutomatism();

    /** Expands own value range with the passed minimum and maximum. */
    void                expandValueRange( double fMinimum, double fMaximum );

    /** Sets additional auto scaling options.
        @param bExpandBorderToIncrementRhythm  If true, expands automatic
            borders to the fixed or calculated increment rhythm.
        @param bExpandIfValuesCloseToBorder  If true, expands automatic borders
            if values are too close (closer than 1/21 of visible area).
        @param bExpandWideValuesToZero  If true, expands automatic border to
            zero, if source values are positive only or negative only, and if
            the absolute values are wide spread (at least one value is less
            than 5/6 of absolute maximum), or if all values are equal.
        @param bExpandNarrowValuesTowardZero  If true, expands automatic border
            toward zero (50% of the visible range), if source values are
            positive only or negative only, and if the absolute values are
            close to the absolute maximum (no value is less than 5/6 of
            absolute maximum). */
    void                setAutoScalingOptions(
                            bool bExpandBorderToIncrementRhythm,
                            bool bExpandIfValuesCloseToBorder,
                            bool bExpandWideValuesToZero,
                            bool bExpandNarrowValuesTowardZero );

    /** Sets the maximum allowed number of automatic main increments.
        @descr  The number of main increments may be limited e.g. by the length
                of the axis and the font size of the axis caption text. */
    void                setMaximumAutoMainIncrementCount( sal_Int32 nMaximumAutoMainIncrementCount );

    /** Sets the time resolution to be used in case it is not set explicitly within the scale
    */
    void setAutomaticTimeResolution( sal_Int32 nTimeResolution );

    /** Fills the passed scale data and increment data according to the own settings. */
    void                calculateExplicitScaleAndIncrement(
                            ExplicitScaleData& rExplicitScale,
                            ExplicitIncrementData& rExplicitIncrement ) const;

    ::com::sun::star::chart2::ScaleData getScale() const;
    Date getNullDate() const;

private:
    /** Fills the passed scale data and increment data for category scaling. */
    void                calculateExplicitIncrementAndScaleForCategory(
                            ExplicitScaleData& rExplicitScale,
                            ExplicitIncrementData& rExplicitIncrement,
                            bool bAutoMinimum, bool bAutoMaximum ) const;

    /** Fills the passed scale data and increment data for logarithmic scaling. */
    void                calculateExplicitIncrementAndScaleForLogarithmic(
                            ExplicitScaleData& rExplicitScale,
                            ExplicitIncrementData& rExplicitIncrement,
                            bool bAutoMinimum, bool bAutoMaximum ) const;

    /** Fills the passed scale data and increment data for linear scaling. */
    void                calculateExplicitIncrementAndScaleForLinear(
                            ExplicitScaleData& rExplicitScale,
                            ExplicitIncrementData& rExplicitIncrement,
                            bool bAutoMinimum, bool bAutoMaximum ) const;

    /** Fills the passed scale data and increment data for date-time axis. */
    void                calculateExplicitIncrementAndScaleForDateTimeAxis(
                            ExplicitScaleData& rExplicitScale,
                            ExplicitIncrementData& rExplicitIncrement,
                            bool bAutoMinimum, bool bAutoMaximum ) const;

private:
    ::com::sun::star::chart2::ScaleData             m_aSourceScale;

    double              m_fValueMinimum;                    /// Minimum of all source values.
    double              m_fValueMaximum;                    /// Maximum of all source values.
    sal_Int32           m_nMaximumAutoMainIncrementCount;   /// Maximum number of automatic main increments.
    bool                m_bExpandBorderToIncrementRhythm;   /// true = Expand to main increments.
    bool                m_bExpandIfValuesCloseToBorder;     /// true = Expand if values are too close to the borders.
    bool                m_bExpandWideValuesToZero;          /// true = Expand wide spread values to zero.
    bool                m_bExpandNarrowValuesTowardZero;    /// true = Expand narrow range toward zero (add half of range).
    sal_Int32           m_nTimeResolution;// a constant out of ::com::sun::star::chart::TimeUnit

    Date                m_aNullDate;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
