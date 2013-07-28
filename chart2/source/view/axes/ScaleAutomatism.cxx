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

#include "ScaleAutomatism.hxx"
#include "macros.hxx"
#include "Tickmarks_Equidistant.hxx"
#include "DateHelper.hxx"
#include "DateScaling.hxx"
#include "AxisHelper.hxx"
#include <com/sun/star/chart/TimeUnit.hpp>

#include <rtl/math.hxx>
#include <limits>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::chart::TimeUnit::DAY;
using ::com::sun::star::chart::TimeUnit::MONTH;
using ::com::sun::star::chart::TimeUnit::YEAR;

const sal_Int32 MAXIMUM_MANUAL_INCREMENT_COUNT = 500;
const sal_Int32 MAXIMUM_SUB_INCREMENT_COUNT = 100;

sal_Int32 lcl_getMaximumAutoIncrementCount( sal_Int32 nAxisType )
{
    sal_Int32 nMaximumAutoIncrementCount = 10;
    if( nAxisType==AxisType::DATE )
        nMaximumAutoIncrementCount = MAXIMUM_MANUAL_INCREMENT_COUNT;
    return nMaximumAutoIncrementCount;
}

namespace
{

void lcl_ensureMaximumSubIncrementCount( sal_Int32& rnSubIntervalCount )
{
    if( rnSubIntervalCount > MAXIMUM_SUB_INCREMENT_COUNT )
        rnSubIntervalCount = MAXIMUM_SUB_INCREMENT_COUNT;
}

}//end anonymous namespace

ExplicitScaleData::ExplicitScaleData()
    : Minimum(0.0)
    , Maximum(10.0)
    , Origin(0.0)
    , Orientation(::com::sun::star::chart2::AxisOrientation_MATHEMATICAL)
    , Scaling()
    , AxisType(::com::sun::star::chart2::AxisType::REALNUMBER)
    , ShiftedCategoryPosition(false)
    , TimeResolution(::com::sun::star::chart::TimeUnit::DAY)
    , NullDate(30,12,1899)
{
}

ExplicitSubIncrement::ExplicitSubIncrement()
    : IntervalCount(2)
    , PostEquidistant(true)
{
}

ExplicitIncrementData::ExplicitIncrementData()
    : MajorTimeInterval(1,::com::sun::star::chart::TimeUnit::DAY)
    , MinorTimeInterval(1,::com::sun::star::chart::TimeUnit::DAY)
    , Distance(1.0)
    , PostEquidistant(true)
    , BaseValue(0.0)
    , SubIncrements()
{
}

ScaleAutomatism::ScaleAutomatism( const ScaleData& rSourceScale, const Date& rNullDate )
                    : m_aSourceScale( rSourceScale )
                    , m_fValueMinimum( 0.0 )
                    , m_fValueMaximum( 0.0 )
                    , m_nMaximumAutoMainIncrementCount( lcl_getMaximumAutoIncrementCount( rSourceScale.AxisType ) )
                    , m_bExpandBorderToIncrementRhythm( false )
                    , m_bExpandIfValuesCloseToBorder( false )
                    , m_bExpandWideValuesToZero( false )
                    , m_bExpandNarrowValuesTowardZero( false )
                    , m_nTimeResolution(::com::sun::star::chart::TimeUnit::DAY)
                    , m_aNullDate(rNullDate)
{
    ::rtl::math::setNan( &m_fValueMinimum );
    ::rtl::math::setNan( &m_fValueMaximum );

    double fExplicitOrigin = 0.0;
    if( m_aSourceScale.Origin >>= fExplicitOrigin )
        expandValueRange( fExplicitOrigin, fExplicitOrigin);
}
ScaleAutomatism::~ScaleAutomatism()
{
}

void ScaleAutomatism::expandValueRange( double fMinimum, double fMaximum )
{
    if( (fMinimum < m_fValueMinimum) || ::rtl::math::isNan( m_fValueMinimum ) )
        m_fValueMinimum = fMinimum;
    if( (fMaximum > m_fValueMaximum) || ::rtl::math::isNan( m_fValueMaximum ) )
        m_fValueMaximum = fMaximum;
}

void ScaleAutomatism::setAutoScalingOptions(
        bool bExpandBorderToIncrementRhythm,
        bool bExpandIfValuesCloseToBorder,
        bool bExpandWideValuesToZero,
        bool bExpandNarrowValuesTowardZero )
{
    // if called multiple times, enable an option, if it is set in at least one call
    m_bExpandBorderToIncrementRhythm |= bExpandBorderToIncrementRhythm;
    m_bExpandIfValuesCloseToBorder   |= bExpandIfValuesCloseToBorder;
    m_bExpandWideValuesToZero        |= bExpandWideValuesToZero;
    m_bExpandNarrowValuesTowardZero  |= bExpandNarrowValuesTowardZero;

    if( m_aSourceScale.AxisType==AxisType::PERCENT )
        m_bExpandIfValuesCloseToBorder = false;
}

void ScaleAutomatism::setMaximumAutoMainIncrementCount( sal_Int32 nMaximumAutoMainIncrementCount )
{
    if( nMaximumAutoMainIncrementCount < 2 )
        m_nMaximumAutoMainIncrementCount = 2; //#i82006
    else if( nMaximumAutoMainIncrementCount > lcl_getMaximumAutoIncrementCount( m_aSourceScale.AxisType ) )
        m_nMaximumAutoMainIncrementCount = lcl_getMaximumAutoIncrementCount( m_aSourceScale.AxisType );
    else
        m_nMaximumAutoMainIncrementCount = nMaximumAutoMainIncrementCount;
}

void ScaleAutomatism::setAutomaticTimeResolution( sal_Int32 nTimeResolution )
{
    m_nTimeResolution = nTimeResolution;
}

void ScaleAutomatism::calculateExplicitScaleAndIncrement(
        ExplicitScaleData& rExplicitScale, ExplicitIncrementData& rExplicitIncrement ) const
{
    // fill explicit scale
    rExplicitScale.Orientation = m_aSourceScale.Orientation;
    rExplicitScale.Scaling = m_aSourceScale.Scaling;
    rExplicitScale.AxisType = m_aSourceScale.AxisType;
    rExplicitScale.NullDate = m_aNullDate;

    bool bAutoMinimum  = !(m_aSourceScale.Minimum >>= rExplicitScale.Minimum);
    bool bAutoMaximum = !(m_aSourceScale.Maximum >>= rExplicitScale.Maximum);
    bool bAutoOrigin = !(m_aSourceScale.Origin >>= rExplicitScale.Origin);

    // automatic scale minimum
    if( bAutoMinimum )
    {
        if( m_aSourceScale.AxisType==AxisType::PERCENT )
            rExplicitScale.Minimum = 0.0;
        else if( ::rtl::math::isNan( m_fValueMinimum ) )
        {
            if( m_aSourceScale.AxisType==AxisType::DATE )
                rExplicitScale.Minimum = 36526.0; //1.1.2000
            else
                rExplicitScale.Minimum = 0.0;   //@todo get Minimum from scaling or from plotter????
        }
        else
            rExplicitScale.Minimum = m_fValueMinimum;
    }

    // automatic scale maximum
    if( bAutoMaximum )
    {
        if( m_aSourceScale.AxisType==AxisType::PERCENT )
            rExplicitScale.Maximum = 1.0;
        else if( ::rtl::math::isNan( m_fValueMaximum ) )
        {
            if( m_aSourceScale.AxisType==AxisType::DATE )
                rExplicitScale.Maximum = 40179.0; //1.1.2010
            else
                rExplicitScale.Maximum = 10.0;  //@todo get Maximum from scaling or from plotter????
        }
        else
            rExplicitScale.Maximum = m_fValueMaximum;
    }

    //fill explicit increment

    rExplicitScale.ShiftedCategoryPosition = m_aSourceScale.ShiftedCategoryPosition;
    bool bIsLogarithm = false;

    //minimum and maximum of the ExplicitScaleData may be changed if allowed
    if( m_aSourceScale.AxisType==AxisType::DATE )
        calculateExplicitIncrementAndScaleForDateTimeAxis( rExplicitScale, rExplicitIncrement, bAutoMinimum, bAutoMaximum );
    else if( m_aSourceScale.AxisType==AxisType::CATEGORY || m_aSourceScale.AxisType==AxisType::SERIES )
        calculateExplicitIncrementAndScaleForCategory( rExplicitScale, rExplicitIncrement, bAutoMinimum, bAutoMaximum );
    else
    {
        bIsLogarithm = AxisHelper::isLogarithmic( rExplicitScale.Scaling );
        if( bIsLogarithm )
            calculateExplicitIncrementAndScaleForLogarithmic( rExplicitScale, rExplicitIncrement, bAutoMinimum, bAutoMaximum );
        else
            calculateExplicitIncrementAndScaleForLinear( rExplicitScale, rExplicitIncrement, bAutoMinimum, bAutoMaximum );
    }

    // automatic origin
    if( bAutoOrigin )
    {
        // #i71415# automatic origin for logarithmic axis
        double fDefaulOrigin = bIsLogarithm ? 1.0 : 0.0;

        if( fDefaulOrigin < rExplicitScale.Minimum )
            fDefaulOrigin = rExplicitScale.Minimum;
        else if( fDefaulOrigin > rExplicitScale.Maximum )
            fDefaulOrigin = rExplicitScale.Maximum;

        rExplicitScale.Origin = fDefaulOrigin;
    }
}

ScaleData ScaleAutomatism::getScale() const
{
    return m_aSourceScale;
}

Date ScaleAutomatism::getNullDate() const
{
    return m_aNullDate;
}

void ScaleAutomatism::calculateExplicitIncrementAndScaleForCategory(
        ExplicitScaleData& rExplicitScale,
        ExplicitIncrementData& rExplicitIncrement,
        bool bAutoMinimum, bool bAutoMaximum ) const
{
    // no scaling for categories
    rExplicitScale.Scaling.clear();

    if( rExplicitScale.ShiftedCategoryPosition )
        rExplicitScale.Maximum += 1.0;

    // ensure that at least one category is visible
    if( rExplicitScale.Maximum <= rExplicitScale.Minimum )
        rExplicitScale.Maximum = rExplicitScale.Minimum + 1.0;

    // default increment settings
    rExplicitIncrement.PostEquidistant = sal_True;  // does not matter anyhow
    rExplicitIncrement.Distance = 1.0;              // category axis always have a main increment of 1
    rExplicitIncrement.BaseValue = 0.0;             // category axis always have a base of 0

    // automatic minimum and maximum
    if( bAutoMinimum && m_bExpandBorderToIncrementRhythm )
        rExplicitScale.Minimum = EquidistantTickFactory::getMinimumAtIncrement( rExplicitScale.Minimum, rExplicitIncrement );
    if( bAutoMaximum && m_bExpandBorderToIncrementRhythm )
        rExplicitScale.Maximum = EquidistantTickFactory::getMaximumAtIncrement( rExplicitScale.Maximum, rExplicitIncrement );

    //prevent performace killover
    double fDistanceCount = ::rtl::math::approxFloor( (rExplicitScale.Maximum-rExplicitScale.Minimum) / rExplicitIncrement.Distance );
    if( static_cast< sal_Int32 >( fDistanceCount ) > MAXIMUM_MANUAL_INCREMENT_COUNT )
    {
        double fMinimumFloor = ::rtl::math::approxFloor( rExplicitScale.Minimum );
        double fMaximumCeil = ::rtl::math::approxCeil( rExplicitScale.Maximum );
        rExplicitIncrement.Distance = ::rtl::math::approxCeil( (fMaximumCeil - fMinimumFloor) / MAXIMUM_MANUAL_INCREMENT_COUNT );
    }

    //fill explicit sub increment
    sal_Int32 nSubCount = m_aSourceScale.IncrementData.SubIncrements.getLength();
    for( sal_Int32 nN=0; nN<nSubCount; nN++ )
    {
        ExplicitSubIncrement aExplicitSubIncrement;
        const SubIncrement& rSubIncrement= m_aSourceScale.IncrementData.SubIncrements[nN];
        if(!(rSubIncrement.IntervalCount>>=aExplicitSubIncrement.IntervalCount))
        {
            //scaling dependent
            //@todo autocalculate IntervalCount dependent on MainIncrement and scaling
            aExplicitSubIncrement.IntervalCount = 2;
        }
        lcl_ensureMaximumSubIncrementCount( aExplicitSubIncrement.IntervalCount );
        if(!(rSubIncrement.PostEquidistant>>=aExplicitSubIncrement.PostEquidistant))
        {
            //scaling dependent
            aExplicitSubIncrement.PostEquidistant = sal_False;
        }
        rExplicitIncrement.SubIncrements.push_back(aExplicitSubIncrement);
    }
}

void ScaleAutomatism::calculateExplicitIncrementAndScaleForLogarithmic(
        ExplicitScaleData& rExplicitScale,
        ExplicitIncrementData& rExplicitIncrement,
        bool bAutoMinimum, bool bAutoMaximum ) const
{
    // *** STEP 1: initialize the range data ***

    const double fInputMinimum = rExplicitScale.Minimum;
    const double fInputMaximum = rExplicitScale.Maximum;

    double fSourceMinimum = rExplicitScale.Minimum;
    double fSourceMaximum = rExplicitScale.Maximum;

    // set automatic PostEquidistant to true (maybe scaling dependent?)
    // Note: scaling with PostEquidistant==false is untested and needs review
    if( !(m_aSourceScale.IncrementData.PostEquidistant >>= rExplicitIncrement.PostEquidistant) )
        rExplicitIncrement.PostEquidistant = sal_True;

    /*  All following scaling code will operate on the logarithms of the source
        values. In the last step, the original values will be restored. */
    uno::Reference< XScaling > xScaling = rExplicitScale.Scaling;
    if( !xScaling.is() )
        xScaling.set( AxisHelper::createLogarithmicScaling() );
    uno::Reference< XScaling > xInverseScaling = xScaling->getInverseScaling();

    fSourceMinimum = xScaling->doScaling( fSourceMinimum );
    if( !::rtl::math::isFinite( fSourceMinimum ) )
        fSourceMinimum = 0.0;
    else if( ::rtl::math::approxEqual( fSourceMinimum, ::rtl::math::approxFloor( fSourceMinimum ) ) )
        fSourceMinimum = ::rtl::math::approxFloor( fSourceMinimum );

    fSourceMaximum = xScaling->doScaling( fSourceMaximum );
    if( !::rtl::math::isFinite( fSourceMaximum ) )
        fSourceMaximum = 0.0;
    else if( ::rtl::math::approxEqual( fSourceMaximum, ::rtl::math::approxFloor( fSourceMaximum ) ) )
        fSourceMaximum = ::rtl::math::approxFloor( fSourceMaximum );

    /*  If range is invalid (minimum greater than maximum), change one of the
        variable limits to validate the range. In this step, a zero-sized range
        is still allowed. */
    if( fSourceMinimum > fSourceMaximum )
    {
        // force changing the maximum, if both limits are fixed
        if( bAutoMaximum || !bAutoMinimum )
            fSourceMaximum = fSourceMinimum;
        else
            fSourceMinimum = fSourceMaximum;
    }

    /*  If maximum is less than 0 (and therefore minimum too), minimum and
        maximum will be negated and swapped to make the following algorithms
        easier. Example: Both ranges [2,5] and [-5,-2] will be processed as
        [2,5], and the latter will be swapped back later. The range [0,0] is
        explicitly excluded from swapping (this would result in [-1,0] instead
        of the expected [0,1]). */
    bool bSwapAndNegateRange = (fSourceMinimum < 0.0) && (fSourceMaximum <= 0.0);
    if( bSwapAndNegateRange )
    {
        double fTempValue = fSourceMinimum;
        fSourceMinimum = -fSourceMaximum;
        fSourceMaximum = -fTempValue;
        ::std::swap( bAutoMinimum, bAutoMaximum );
    }

    // *** STEP 2: find temporary (unrounded) axis minimum and maximum ***

    double fTempMinimum = fSourceMinimum;
    double fTempMaximum = fSourceMaximum;

    /*  If minimum is variable and greater than 0 (and therefore maximum too),
        means all original values are greater than 1 (or all values are less
        than 1, and the range has been swapped above), then: */
    if( bAutoMinimum && (fTempMinimum > 0.0) )
    {
        /*  If minimum is less than 5 (i.e. original source values less than
            B^5, B being the base of the scaling), or if minimum and maximum
            are in different increment intervals (means, if minimum and maximum
            are not both in the range [B^n,B^(n+1)] for a whole number n), set
            minimum to 0, which results in B^0=1 on the axis. */
        double fMinimumFloor = ::rtl::math::approxFloor( fTempMinimum );
        double fMaximumFloor = ::rtl::math::approxFloor( fTempMaximum );
        // handle the exact value B^(n+1) to be in the range [B^n,B^(n+1)]
        if( ::rtl::math::approxEqual( fTempMaximum, fMaximumFloor ) )
            fMaximumFloor -= 1.0;

        if( (fMinimumFloor < 5.0) || (fMinimumFloor < fMaximumFloor) )
        {
            if( m_bExpandWideValuesToZero )
                fTempMinimum = 0.0;
        }
        /*  Else (minimum and maximum are in one increment interval), expand
            minimum toward 0 to make the 'shorter' data points visible. */
        else
        {
            if( m_bExpandNarrowValuesTowardZero )
                fTempMinimum -= 1.0;
        }
    }

    /*  If range is still zero-sized (e.g. when minimum is fixed), set minimum
        to 0, which makes the axis start/stop at the value 1. */
    if( fTempMinimum == fTempMaximum )
    {
        if( bAutoMinimum && (fTempMaximum > 0.0) )
            fTempMinimum = 0.0;
        else
            fTempMaximum += 1.0;    // always add one interval, even if maximum is fixed
    }

    // *** STEP 3: calculate main interval size ***

    // base value (anchor position of the intervals), already scaled
    if( !(m_aSourceScale.IncrementData.BaseValue >>= rExplicitIncrement.BaseValue) )
    {
        //scaling dependent
        //@maybe todo is this default also plotter dependent ??
        if( !bAutoMinimum )
            rExplicitIncrement.BaseValue = fTempMinimum;
        else if( !bAutoMaximum )
            rExplicitIncrement.BaseValue = fTempMaximum;
        else
            rExplicitIncrement.BaseValue = 0.0;
    }

    // calculate automatic interval
    bool bAutoDistance = !(m_aSourceScale.IncrementData.Distance >>= rExplicitIncrement.Distance);
    if( bAutoDistance )
        rExplicitIncrement.Distance = 0.0;

    /*  Restrict number of allowed intervals with user-defined distance to
        MAXIMUM_MANUAL_INCREMENT_COUNT. */
    sal_Int32 nMaxMainIncrementCount = bAutoDistance ?
        m_nMaximumAutoMainIncrementCount : MAXIMUM_MANUAL_INCREMENT_COUNT;

    // repeat calculation until number of intervals are valid
    bool bNeedIteration = true;
    bool bHasCalculatedDistance = false;
    while( bNeedIteration )
    {
        if( bAutoDistance )
        {
            // first iteration: calculate interval size from axis limits
            if( !bHasCalculatedDistance )
            {
                double fMinimumFloor = ::rtl::math::approxFloor( fTempMinimum );
                double fMaximumCeil = ::rtl::math::approxCeil( fTempMaximum );
                rExplicitIncrement.Distance = ::rtl::math::approxCeil( (fMaximumCeil - fMinimumFloor) / nMaxMainIncrementCount );
            }
            else
            {
                // following iterations: increase distance
                rExplicitIncrement.Distance += 1.0;
            }

            // for next iteration: distance calculated -> use else path to increase
            bHasCalculatedDistance = true;
        }

        // *** STEP 4: additional space above or below the data points ***

        double fAxisMinimum = fTempMinimum;
        double fAxisMaximum = fTempMaximum;

        // round to entire multiples of the distance and add additional space
        if( bAutoMinimum && m_bExpandBorderToIncrementRhythm )
        {
            fAxisMinimum = EquidistantTickFactory::getMinimumAtIncrement( fAxisMinimum, rExplicitIncrement );

            //ensure valid values after scaling #i100995#
            if( !bAutoDistance )
            {
                double fCheck = xInverseScaling->doScaling( fAxisMinimum );
                if( !::rtl::math::isFinite( fCheck ) || fCheck <= 0 )
                {
                    bAutoDistance = true;
                    bHasCalculatedDistance = false;
                    continue;
                }
            }
        }
        if( bAutoMaximum && m_bExpandBorderToIncrementRhythm )
        {
            fAxisMaximum = EquidistantTickFactory::getMaximumAtIncrement( fAxisMaximum, rExplicitIncrement );

            //ensure valid values after scaling #i100995#
            if( !bAutoDistance )
            {
                double fCheck = xInverseScaling->doScaling( fAxisMaximum );
                if( !::rtl::math::isFinite( fCheck ) || fCheck <= 0 )
                {
                    bAutoDistance = true;
                    bHasCalculatedDistance = false;
                    continue;
                }
            }
        }

        // set the resulting limits (swap back to negative range if needed)
        if( bSwapAndNegateRange )
        {
            rExplicitScale.Minimum = -fAxisMaximum;
            rExplicitScale.Maximum = -fAxisMinimum;
        }
        else
        {
            rExplicitScale.Minimum = fAxisMinimum;
            rExplicitScale.Maximum = fAxisMaximum;
        }

        /*  If the number of intervals is too high (e.g. due to invalid fixed
            distance or due to added space above or below data points),
            calculate again with increased distance. */
        double fDistanceCount = ::rtl::math::approxFloor( (fAxisMaximum - fAxisMinimum) / rExplicitIncrement.Distance );
        bNeedIteration = static_cast< sal_Int32 >( fDistanceCount ) > nMaxMainIncrementCount;
        // if manual distance is invalid, trigger automatic calculation
        if( bNeedIteration )
            bAutoDistance = true;

        // convert limits back to logarithmic scale
        rExplicitScale.Minimum = xInverseScaling->doScaling( rExplicitScale.Minimum );
        rExplicitScale.Maximum = xInverseScaling->doScaling( rExplicitScale.Maximum );

        //ensure valid values after scaling #i100995#
        if( !::rtl::math::isFinite( rExplicitScale.Minimum ) || rExplicitScale.Minimum <= 0)
        {
            rExplicitScale.Minimum = fInputMinimum;
            if( !::rtl::math::isFinite( rExplicitScale.Minimum ) || rExplicitScale.Minimum <= 0 )
                rExplicitScale.Minimum = 1.0;
        }
        if( !::rtl::math::isFinite( rExplicitScale.Maximum) || rExplicitScale.Maximum <= 0 )
        {
            rExplicitScale.Maximum= fInputMaximum;
            if( !::rtl::math::isFinite( rExplicitScale.Maximum) || rExplicitScale.Maximum <= 0 )
                rExplicitScale.Maximum = 10.0;
        }
        if( rExplicitScale.Maximum < rExplicitScale.Minimum )
            ::std::swap( rExplicitScale.Maximum, rExplicitScale.Minimum );
    }

    //fill explicit sub increment
    sal_Int32 nSubCount = m_aSourceScale.IncrementData.SubIncrements.getLength();
    for( sal_Int32 nN=0; nN<nSubCount; nN++ )
    {
        ExplicitSubIncrement aExplicitSubIncrement;
        const SubIncrement& rSubIncrement = m_aSourceScale.IncrementData.SubIncrements[nN];
        if(!(rSubIncrement.IntervalCount>>=aExplicitSubIncrement.IntervalCount))
        {
            //scaling dependent
            //@todo autocalculate IntervalCount dependent on MainIncrement and scaling
            aExplicitSubIncrement.IntervalCount = 9;
        }
        lcl_ensureMaximumSubIncrementCount( aExplicitSubIncrement.IntervalCount );
        if(!(rSubIncrement.PostEquidistant>>=aExplicitSubIncrement.PostEquidistant))
        {
            //scaling dependent
            aExplicitSubIncrement.PostEquidistant = sal_False;
        }
        rExplicitIncrement.SubIncrements.push_back(aExplicitSubIncrement);
    }
}

void ScaleAutomatism::calculateExplicitIncrementAndScaleForDateTimeAxis(
        ExplicitScaleData& rExplicitScale,
        ExplicitIncrementData& rExplicitIncrement,
        bool bAutoMinimum, bool bAutoMaximum ) const
{
    Date aMinDate(m_aNullDate); aMinDate += static_cast<long>(::rtl::math::approxFloor(rExplicitScale.Minimum));
    Date aMaxDate(m_aNullDate); aMaxDate += static_cast<long>(::rtl::math::approxFloor(rExplicitScale.Maximum));
    rExplicitIncrement.PostEquidistant = sal_False;

    if( aMinDate > aMaxDate )
    {
        std::swap(aMinDate,aMaxDate);
    }

    if( !(m_aSourceScale.TimeIncrement.TimeResolution >>= rExplicitScale.TimeResolution) )
        rExplicitScale.TimeResolution = m_nTimeResolution;

    rExplicitScale.Scaling = new DateScaling(m_aNullDate,rExplicitScale.TimeResolution,false);

    // choose min and max suitable to time resolution
    switch( rExplicitScale.TimeResolution )
    {
    case DAY:
        if( rExplicitScale.ShiftedCategoryPosition )
            aMaxDate++;//for explicit scales we need one interval more (maximum excluded)
        break;
    case MONTH:
        aMinDate.SetDay(1);
        aMaxDate.SetDay(1);
        if( rExplicitScale.ShiftedCategoryPosition )
            aMaxDate = DateHelper::GetDateSomeMonthsAway(aMaxDate,1);//for explicit scales we need one interval more (maximum excluded)
        if( DateHelper::IsLessThanOneMonthAway( aMinDate, aMaxDate ) )
        {
            if( bAutoMaximum || !bAutoMinimum )
                aMaxDate = DateHelper::GetDateSomeMonthsAway(aMinDate,1);
            else
                aMinDate = DateHelper::GetDateSomeMonthsAway(aMaxDate,-1);
        }
        break;
    case YEAR:
        aMinDate.SetDay(1);
        aMinDate.SetMonth(1);
        aMaxDate.SetDay(1);
        aMaxDate.SetMonth(1);
        if( rExplicitScale.ShiftedCategoryPosition )
            aMaxDate = DateHelper::GetDateSomeYearsAway(aMaxDate,1);//for explicit scales we need one interval more (maximum excluded)
        if( DateHelper::IsLessThanOneYearAway( aMinDate, aMaxDate ) )
        {
            if( bAutoMaximum || !bAutoMinimum )
                aMaxDate = DateHelper::GetDateSomeYearsAway(aMinDate,1);
            else
                aMinDate = DateHelper::GetDateSomeYearsAway(aMaxDate,-1);
        }
        break;
    }

    // set the resulting limits (swap back to negative range if needed)
    rExplicitScale.Minimum = aMinDate - m_aNullDate;
    rExplicitScale.Maximum = aMaxDate - m_aNullDate;

    bool bAutoMajor = !(m_aSourceScale.TimeIncrement.MajorTimeInterval >>= rExplicitIncrement.MajorTimeInterval);
    bool bAutoMinor = !(m_aSourceScale.TimeIncrement.MinorTimeInterval >>= rExplicitIncrement.MinorTimeInterval);

    sal_Int32 nMaxMainIncrementCount = bAutoMajor ?
        m_nMaximumAutoMainIncrementCount : MAXIMUM_MANUAL_INCREMENT_COUNT;
    if( nMaxMainIncrementCount > 1 )
        nMaxMainIncrementCount--;

    //choose major time interval:
    long nDayCount = (aMaxDate-aMinDate);
    long nMainIncrementCount = 1;
    if( !bAutoMajor )
    {
        long nIntervalDayCount = rExplicitIncrement.MajorTimeInterval.Number;
        if( rExplicitIncrement.MajorTimeInterval.TimeUnit < rExplicitScale.TimeResolution )
            rExplicitIncrement.MajorTimeInterval.TimeUnit = rExplicitScale.TimeResolution;
        switch( rExplicitIncrement.MajorTimeInterval.TimeUnit )
        {
        case DAY:
            break;
        case MONTH:
            nIntervalDayCount*=31;//todo: maybe different for other calendars... get localized calendar according to set number format at axis ...
            break;
        case YEAR:
            nIntervalDayCount*=365;//todo: maybe different for other calendars... get localized calendar according to set number format at axis ...
            break;
        }
        nMainIncrementCount = nDayCount/nIntervalDayCount;
        if( nMainIncrementCount > nMaxMainIncrementCount )
            bAutoMajor = true;
    }
    if( bAutoMajor )
    {
        long nNumer = 1;
        long nIntervalDays =  nDayCount / nMaxMainIncrementCount;
        double nDaysPerInterval = 1.0;
        if( nIntervalDays>365 || YEAR==rExplicitScale.TimeResolution )
        {
            rExplicitIncrement.MajorTimeInterval.TimeUnit = YEAR;
            nDaysPerInterval = 365.0;//todo: maybe different for other calendars... get localized calendar according to set number format at axis ...
        }
        else if( nIntervalDays>31 || MONTH==rExplicitScale.TimeResolution )
        {
            rExplicitIncrement.MajorTimeInterval.TimeUnit = MONTH;
            nDaysPerInterval = 31.0;//todo: maybe different for other calendars... get localized calendar according to set number format at axis ...
        }
        else
        {
            rExplicitIncrement.MajorTimeInterval.TimeUnit = DAY;
            nDaysPerInterval = 1.0;
        }

        nNumer = static_cast<sal_Int32>( rtl::math::approxCeil( nIntervalDays/nDaysPerInterval ) );
        if(nNumer<=0)
            nNumer=1;
        if( rExplicitIncrement.MajorTimeInterval.TimeUnit == DAY )
        {
            if( nNumer>2 && nNumer<7 )
                nNumer=7;
            else if( nNumer>7 )
            {
                rExplicitIncrement.MajorTimeInterval.TimeUnit = MONTH;
                nDaysPerInterval = 31.0;
                nNumer = static_cast<sal_Int32>( rtl::math::approxCeil( nIntervalDays/nDaysPerInterval ) );
                if(nNumer<=0)
                    nNumer=1;
            }
        }
        rExplicitIncrement.MajorTimeInterval.Number = nNumer;
        nMainIncrementCount = static_cast<long>(nDayCount/(nNumer*nDaysPerInterval));
    }

    //choose minor time interval:
    if( !bAutoMinor )
    {
        if( rExplicitIncrement.MinorTimeInterval.TimeUnit > rExplicitIncrement.MajorTimeInterval.TimeUnit )
            rExplicitIncrement.MinorTimeInterval.TimeUnit = rExplicitIncrement.MajorTimeInterval.TimeUnit;
        long nIntervalDayCount = rExplicitIncrement.MinorTimeInterval.Number;
        switch( rExplicitIncrement.MinorTimeInterval.TimeUnit )
        {
        case DAY:
            break;
        case MONTH:
            nIntervalDayCount*=31;//todo: maybe different for other calendars... get localized calendar according to set number format at axis ...
            break;
        case YEAR:
            nIntervalDayCount*=365;//todo: maybe different for other calendars... get localized calendar according to set number format at axis ...
            break;
        }
        if( nDayCount/nIntervalDayCount > nMaxMainIncrementCount )
            bAutoMinor = true;
    }
    if( bAutoMinor )
    {
        rExplicitIncrement.MinorTimeInterval.TimeUnit = rExplicitIncrement.MajorTimeInterval.TimeUnit;
        rExplicitIncrement.MinorTimeInterval.Number = 1;
        if( nMainIncrementCount > 100 )
            rExplicitIncrement.MinorTimeInterval.Number = rExplicitIncrement.MajorTimeInterval.Number;
        else
        {
            if( rExplicitIncrement.MajorTimeInterval.Number >= 2 )
            {
                if( !(rExplicitIncrement.MajorTimeInterval.Number%2) )
                    rExplicitIncrement.MinorTimeInterval.Number = rExplicitIncrement.MajorTimeInterval.Number/2;
                else if( !(rExplicitIncrement.MajorTimeInterval.Number%3) )
                    rExplicitIncrement.MinorTimeInterval.Number = rExplicitIncrement.MajorTimeInterval.Number/3;
                else if( !(rExplicitIncrement.MajorTimeInterval.Number%5) )
                    rExplicitIncrement.MinorTimeInterval.Number = rExplicitIncrement.MajorTimeInterval.Number/5;
                else if( rExplicitIncrement.MajorTimeInterval.Number > 50 )
                    rExplicitIncrement.MinorTimeInterval.Number = rExplicitIncrement.MajorTimeInterval.Number;
            }
            else
            {
                switch( rExplicitIncrement.MajorTimeInterval.TimeUnit )
                {
                    case DAY:
                        break;
                    case MONTH:
                        if( rExplicitScale.TimeResolution == DAY )
                            rExplicitIncrement.MinorTimeInterval.TimeUnit = DAY;
                        break;
                    case YEAR:
                        if( rExplicitScale.TimeResolution <= MONTH )
                            rExplicitIncrement.MinorTimeInterval.TimeUnit = MONTH;
                        break;
                }
            }
        }
    }

}

void ScaleAutomatism::calculateExplicitIncrementAndScaleForLinear(
        ExplicitScaleData& rExplicitScale,
        ExplicitIncrementData& rExplicitIncrement,
        bool bAutoMinimum, bool bAutoMaximum ) const
{
    // *** STEP 1: initialize the range data ***

    double fSourceMinimum = rExplicitScale.Minimum;
    double fSourceMaximum = rExplicitScale.Maximum;

    // set automatic PostEquidistant to true (maybe scaling dependent?)
    if( !(m_aSourceScale.IncrementData.PostEquidistant >>= rExplicitIncrement.PostEquidistant) )
        rExplicitIncrement.PostEquidistant = sal_True;

    /*  If range is invalid (minimum greater than maximum), change one of the
        variable limits to validate the range. In this step, a zero-sized range
        is still allowed. */
    if( fSourceMinimum > fSourceMaximum )
    {
        // force changing the maximum, if both limits are fixed
        if( bAutoMaximum || !bAutoMinimum )
            fSourceMaximum = fSourceMinimum;
        else
            fSourceMinimum = fSourceMaximum;
    }

    /*  If maximum is zero or negative (and therefore minimum too), minimum and
        maximum will be negated and swapped to make the following algorithms
        easier. Example: Both ranges [2,5] and [-5,-2] will be processed as
        [2,5], and the latter will be swapped back later. The range [0,0] is
        explicitly excluded from swapping (this would result in [-1,0] instead
        of the expected [0,1]). */
    bool bSwapAndNegateRange = (fSourceMinimum < 0.0) && (fSourceMaximum <= 0.0);
    if( bSwapAndNegateRange )
    {
        double fTempValue = fSourceMinimum;
        fSourceMinimum = -fSourceMaximum;
        fSourceMaximum = -fTempValue;
        ::std::swap( bAutoMinimum, bAutoMaximum );
    }

    // *** STEP 2: find temporary (unrounded) axis minimum and maximum ***

    double fTempMinimum = fSourceMinimum;
    double fTempMaximum = fSourceMaximum;

    /*  If minimum is variable and greater than 0 (and therefore maximum too),
        means all values are positive (or all values are negative, and the
        range has been swapped above), then: */
    if( bAutoMinimum && (fTempMinimum > 0.0) )
    {
        /*  If minimum equals maximum, or if minimum is less than 5/6 of
            maximum, set minimum to 0. */
        if( (fTempMinimum == fTempMaximum) || (fTempMinimum / fTempMaximum < 5.0 / 6.0) )
        {
            if( m_bExpandWideValuesToZero )
                fTempMinimum = 0.0;
        }
        /*  Else (minimum is greater than or equal to 5/6 of maximum), add half
            of the visible range (expand minimum toward 0) to make the
            'shorter' data points visible. */
        else
        {
            if( m_bExpandNarrowValuesTowardZero )
                fTempMinimum -= (fTempMaximum - fTempMinimum) / 2.0;
        }
    }

    /*  If range is still zero-sized (e.g. when minimum is fixed), add some
        space to a variable limit. */
    if( fTempMinimum == fTempMaximum )
    {
        if( bAutoMaximum || !bAutoMinimum )
        {
            // change 0 to 1, otherwise double the value
            if( fTempMaximum == 0.0 )
                fTempMaximum = 1.0;
            else
                fTempMaximum *= 2.0;
        }
        else
        {
            // change 0 to -1, otherwise halve the value
            if( fTempMinimum == 0.0 )
                fTempMinimum = -1.0;
            else
                fTempMinimum /= 2.0;
        }
    }

    // *** STEP 3: calculate main interval size ***

    // base value (anchor position of the intervals)
    if( !(m_aSourceScale.IncrementData.BaseValue >>= rExplicitIncrement.BaseValue) )
    {
        if( !bAutoMinimum )
            rExplicitIncrement.BaseValue = fTempMinimum;
        else if( !bAutoMaximum )
            rExplicitIncrement.BaseValue = fTempMaximum;
        else
            rExplicitIncrement.BaseValue = 0.0;
    }

    // calculate automatic interval
    bool bAutoDistance = !(m_aSourceScale.IncrementData.Distance >>= rExplicitIncrement.Distance);
    /*  Restrict number of allowed intervals with user-defined distance to
        MAXIMUM_MANUAL_INCREMENT_COUNT. */
    sal_Int32 nMaxMainIncrementCount = bAutoDistance ?
        m_nMaximumAutoMainIncrementCount : MAXIMUM_MANUAL_INCREMENT_COUNT;

    double fDistanceMagnitude = 0.0;
    double fDistanceNormalized = 0.0;
    bool bHasNormalizedDistance = false;

    // repeat calculation until number of intervals are valid
    bool bNeedIteration = true;
    while( bNeedIteration )
    {
        if( bAutoDistance )
        {
            // first iteration: calculate interval size from axis limits
            if( !bHasNormalizedDistance )
            {
                // raw size of an interval
                double fDistance = (fTempMaximum - fTempMinimum) / nMaxMainIncrementCount;

                // if distance of is less than 1e-307, do not do anything
                if( fDistance <= 1.0e-307 )
                {
                    fDistanceNormalized = 1.0;
                    fDistanceMagnitude = 1.0e-307;
                }
                else if ( !rtl::math::isFinite(fDistance) )
                {
                    // fdo#43703: Handle values bigger than limits correctly
                    fDistanceNormalized = 1.0;
                    fDistanceMagnitude = std::numeric_limits<double>::max();
                }
                else
                {
                    // distance magnitude (a power of 10)
                    int nExponent = static_cast< int >( ::rtl::math::approxFloor( log10( fDistance ) ) );
                    fDistanceMagnitude = ::rtl::math::pow10Exp( 1.0, nExponent );

                    // stick normalized distance to a few predefined values
                    fDistanceNormalized = fDistance / fDistanceMagnitude;
                    if( fDistanceNormalized <= 1.0 )
                        fDistanceNormalized = 1.0;
                    else if( fDistanceNormalized <= 2.0 )
                        fDistanceNormalized = 2.0;
                    else if( fDistanceNormalized <= 5.0 )
                        fDistanceNormalized = 5.0;
                    else
                    {
                        fDistanceNormalized = 1.0;
                        fDistanceMagnitude *= 10;
                    }
                }
                // for next iteration: distance is normalized -> use else path to increase distance
                bHasNormalizedDistance = true;
            }
            // following iterations: increase distance, use only allowed values
            else
            {
                if( fDistanceNormalized == 1.0 )
                    fDistanceNormalized = 2.0;
                else if( fDistanceNormalized == 2.0 )
                    fDistanceNormalized = 5.0;
                else
                {
                    fDistanceNormalized = 1.0;
                    fDistanceMagnitude *= 10;
                }
            }

            // set the resulting distance
            rExplicitIncrement.Distance = fDistanceNormalized * fDistanceMagnitude;
        }

        // *** STEP 4: additional space above or below the data points ***

        double fAxisMinimum = fTempMinimum;
        double fAxisMaximum = fTempMaximum;

        // round to entire multiples of the distance and add additional space
        if( bAutoMinimum )
        {
            // round to entire multiples of the distance, based on the base value
            if( m_bExpandBorderToIncrementRhythm )
                fAxisMinimum = EquidistantTickFactory::getMinimumAtIncrement( fAxisMinimum, rExplicitIncrement );
            // additional space, if source minimum is to near at axis minimum
            if( m_bExpandIfValuesCloseToBorder )
                if( (fAxisMinimum != 0.0) && ((fAxisMaximum - fSourceMinimum) / (fAxisMaximum - fAxisMinimum) > 20.0 / 21.0) )
                    fAxisMinimum -= rExplicitIncrement.Distance;
        }
        if( bAutoMaximum )
        {
            // round to entire multiples of the distance, based on the base value
            if( m_bExpandBorderToIncrementRhythm )
                fAxisMaximum = EquidistantTickFactory::getMaximumAtIncrement( fAxisMaximum, rExplicitIncrement );
            // additional space, if source maximum is to near at axis maximum
            if( m_bExpandIfValuesCloseToBorder )
                if( (fAxisMaximum != 0.0) && ((fSourceMaximum - fAxisMinimum) / (fAxisMaximum - fAxisMinimum) > 20.0 / 21.0) )
                    fAxisMaximum += rExplicitIncrement.Distance;
        }

        // set the resulting limits (swap back to negative range if needed)
        if( bSwapAndNegateRange )
        {
            rExplicitScale.Minimum = -fAxisMaximum;
            rExplicitScale.Maximum = -fAxisMinimum;
        }
        else
        {
            rExplicitScale.Minimum = fAxisMinimum;
            rExplicitScale.Maximum = fAxisMaximum;
        }

        /*  If the number of intervals is too high (e.g. due to invalid fixed
            distance or due to added space above or below data points),
            calculate again with increased distance. */
        double fDistanceCount = ::rtl::math::approxFloor( (fAxisMaximum - fAxisMinimum) / rExplicitIncrement.Distance );
        bNeedIteration = static_cast< sal_Int32 >( fDistanceCount ) > nMaxMainIncrementCount;
        // if manual distance is invalid, trigger automatic calculation
        if( bNeedIteration )
            bAutoDistance = true;
    }

    //fill explicit sub increment
    sal_Int32 nSubCount = m_aSourceScale.IncrementData.SubIncrements.getLength();
    for( sal_Int32 nN=0; nN<nSubCount; nN++ )
    {
        ExplicitSubIncrement aExplicitSubIncrement;
        const SubIncrement& rSubIncrement= m_aSourceScale.IncrementData.SubIncrements[nN];
        if(!(rSubIncrement.IntervalCount>>=aExplicitSubIncrement.IntervalCount))
        {
            //scaling dependent
            //@todo autocalculate IntervalCount dependent on MainIncrement and scaling
            aExplicitSubIncrement.IntervalCount = 2;
        }
        lcl_ensureMaximumSubIncrementCount( aExplicitSubIncrement.IntervalCount );
        if(!(rSubIncrement.PostEquidistant>>=aExplicitSubIncrement.PostEquidistant))
        {
            //scaling dependent
            aExplicitSubIncrement.PostEquidistant = sal_False;
        }
        rExplicitIncrement.SubIncrements.push_back(aExplicitSubIncrement);
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
