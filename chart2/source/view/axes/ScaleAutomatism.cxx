/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScaleAutomatism.cxx,v $
 * $Revision: 1.12.24.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ScaleAutomatism.hxx"
#include "macros.hxx"
#include "TickmarkHelper.hxx"
#include "AxisHelper.hxx"
#include <rtl/math.hxx>
#include <tools/debug.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

const sal_Int32 MAXIMUM_MANUAL_INCREMENT_COUNT = 500;
const sal_Int32 MAXIMUM_AUTO_INCREMENT_COUNT = 10;
const sal_Int32 MAXIMUM_SUB_INCREMENT_COUNT = 100;

namespace
{

void lcl_ensureMaximumSubIncrementCount( sal_Int32& rnSubIntervalCount )
{
    if( rnSubIntervalCount > MAXIMUM_SUB_INCREMENT_COUNT )
        rnSubIntervalCount = MAXIMUM_SUB_INCREMENT_COUNT;
}

}//end anonymous namespace

ScaleAutomatism::ScaleAutomatism( const ScaleData& rSourceScale )
                    : m_aSourceScale( rSourceScale )
                    , m_fValueMinimum( 0.0 )
                    , m_fValueMaximum( 0.0 )
                    , m_nMaximumAutoMainIncrementCount( MAXIMUM_AUTO_INCREMENT_COUNT )
                    , m_bExpandBorderToIncrementRhythm( false )
                    , m_bExpandIfValuesCloseToBorder( false )
                    , m_bExpandWideValuesToZero( false )
                    , m_bExpandNarrowValuesTowardZero( false )
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
    else if( nMaximumAutoMainIncrementCount > MAXIMUM_AUTO_INCREMENT_COUNT )
        m_nMaximumAutoMainIncrementCount = MAXIMUM_AUTO_INCREMENT_COUNT;
    else
        m_nMaximumAutoMainIncrementCount = nMaximumAutoMainIncrementCount;
}

void ScaleAutomatism::calculateExplicitScaleAndIncrement(
        ExplicitScaleData& rExplicitScale, ExplicitIncrementData& rExplicitIncrement ) const
{
    // fill explicit scale
    rExplicitScale.Orientation = m_aSourceScale.Orientation;
    rExplicitScale.Scaling = m_aSourceScale.Scaling;
    rExplicitScale.Breaks = m_aSourceScale.Breaks;
    rExplicitScale.AxisType = m_aSourceScale.AxisType;

    bool bAutoMinimum  = !(m_aSourceScale.Minimum >>= rExplicitScale.Minimum);
    bool bAutoMaximum = !(m_aSourceScale.Maximum >>= rExplicitScale.Maximum);
    bool bAutoOrigin = !(m_aSourceScale.Origin >>= rExplicitScale.Origin);

    // automatic scale minimum
    if( bAutoMinimum )
    {
        if( m_aSourceScale.AxisType==AxisType::PERCENT )
            rExplicitScale.Minimum = 0.0;
        else if( ::rtl::math::isNan( m_fValueMinimum ) )
            rExplicitScale.Minimum = 0.0;   //@todo get Minimum from scaling or from plotter????
        else
            rExplicitScale.Minimum = m_fValueMinimum;
    }

    // automatic scale maximum
    if( bAutoMaximum )
    {
        if( m_aSourceScale.AxisType==AxisType::PERCENT )
            rExplicitScale.Maximum = 1.0;
        else if( ::rtl::math::isNan( m_fValueMaximum ) )
            rExplicitScale.Maximum = 10.0;  //@todo get Maximum from scaling or from plotter????
        else
            rExplicitScale.Maximum = m_fValueMaximum;
    }

    //---------------------------------------------------------------
    //fill explicit increment

    rExplicitIncrement.ShiftedPosition = (m_aSourceScale.AxisType==AxisType::SERIES) ? true : false;
    bool bIsLogarithm = false;

    //minimum and maximum of the ExplicitScaleData may be changed if allowed
    if( m_aSourceScale.AxisType==AxisType::CATEGORY || m_aSourceScale.AxisType==AxisType::SERIES )
    {
        calculateExplicitIncrementAndScaleForCategory( rExplicitScale, rExplicitIncrement, bAutoMinimum, bAutoMaximum );
    }
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

// private --------------------------------------------------------------------

void ScaleAutomatism::calculateExplicitIncrementAndScaleForCategory(
        ExplicitScaleData& rExplicitScale,
        ExplicitIncrementData& rExplicitIncrement,
        bool bAutoMinimum, bool bAutoMaximum ) const
{
    // no scaling for categories
    rExplicitScale.Scaling.clear();

    // ensure that at least one category is visible
    if( rExplicitScale.Maximum <= rExplicitScale.Minimum )
        rExplicitScale.Maximum = rExplicitScale.Minimum + 1.0;

    // default increment settings
    rExplicitIncrement.PostEquidistant = sal_True;  // does not matter anyhow
    rExplicitIncrement.Distance = 1.0;              // category axis always have a main increment of 1
    rExplicitIncrement.BaseValue = 0.0;             // category axis always have a base of 0

    // automatic minimum and maximum
    if( bAutoMinimum && m_bExpandBorderToIncrementRhythm )
        rExplicitScale.Minimum = TickmarkHelper::getMinimumAtIncrement( rExplicitScale.Minimum, rExplicitIncrement );
    if( bAutoMaximum && m_bExpandBorderToIncrementRhythm )
        rExplicitScale.Maximum = TickmarkHelper::getMaximumAtIncrement( rExplicitScale.Maximum, rExplicitIncrement );

    //prevent performace killover
    double fDistanceCount = ::rtl::math::approxFloor( (rExplicitScale.Maximum-rExplicitScale.Minimum) / rExplicitIncrement.Distance );
    if( static_cast< sal_Int32 >( fDistanceCount ) > MAXIMUM_MANUAL_INCREMENT_COUNT )
    {
        double fMinimumFloor = ::rtl::math::approxFloor( rExplicitScale.Minimum );
        double fMaximumCeil = ::rtl::math::approxCeil( rExplicitScale.Maximum );
        rExplicitIncrement.Distance = ::rtl::math::approxCeil( (fMaximumCeil - fMinimumFloor) / MAXIMUM_MANUAL_INCREMENT_COUNT );
    }

    //---------------------------------------------------------------
    //fill explicit sub increment
    sal_Int32 nSubCount = m_aSourceScale.IncrementData.SubIncrements.getLength();
    rExplicitIncrement.SubIncrements.realloc(nSubCount);
    for( sal_Int32 nN=0; nN<nSubCount; nN++ )
    {
        const SubIncrement&     rSubIncrement         = m_aSourceScale.IncrementData.SubIncrements[nN];
        ExplicitSubIncrement&   rExplicitSubIncrement = rExplicitIncrement.SubIncrements[nN];

        if(!(rSubIncrement.IntervalCount>>=rExplicitSubIncrement.IntervalCount))
        {
            //scaling dependent
            //@todo autocalculate IntervalCount dependent on MainIncrement and scaling
            rExplicitSubIncrement.IntervalCount = 2;
        }
        lcl_ensureMaximumSubIncrementCount( rExplicitSubIncrement.IntervalCount );
        if(!(rSubIncrement.PostEquidistant>>=rExplicitSubIncrement.PostEquidistant))
        {
            //scaling dependent
            rExplicitSubIncrement.PostEquidistant = sal_False;
        }
    }
}

//@todo these method should become part of the scaling interface and implementation somehow
//@todo problem with outparamters at api
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
            fAxisMinimum = TickmarkHelper::getMinimumAtIncrement( fAxisMinimum, rExplicitIncrement );

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
            fAxisMaximum = TickmarkHelper::getMaximumAtIncrement( fAxisMaximum, rExplicitIncrement );

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

    //---------------------------------------------------------------
    //fill explicit sub increment
    sal_Int32 nSubCount = m_aSourceScale.IncrementData.SubIncrements.getLength();
    rExplicitIncrement.SubIncrements.realloc(nSubCount);
    for( sal_Int32 nN=0; nN<nSubCount; nN++ )
    {
        const SubIncrement&     rSubIncrement         = m_aSourceScale.IncrementData.SubIncrements[nN];
        ExplicitSubIncrement&   rExplicitSubIncrement = rExplicitIncrement.SubIncrements[nN];

        if(!(rSubIncrement.IntervalCount>>=rExplicitSubIncrement.IntervalCount))
        {
            //scaling dependent
            //@todo autocalculate IntervalCount dependent on MainIncrement and scaling
            rExplicitSubIncrement.IntervalCount = 5;
        }
        lcl_ensureMaximumSubIncrementCount( rExplicitSubIncrement.IntervalCount );
        if(!(rSubIncrement.PostEquidistant>>=rExplicitSubIncrement.PostEquidistant))
        {
            //scaling dependent
            rExplicitSubIncrement.PostEquidistant = sal_False;
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
                fAxisMinimum = TickmarkHelper::getMinimumAtIncrement( fAxisMinimum, rExplicitIncrement );
            // additional space, if source minimum is to near at axis minimum
            if( m_bExpandIfValuesCloseToBorder )
                if( (fAxisMinimum != 0.0) && ((fAxisMaximum - fSourceMinimum) / (fAxisMaximum - fAxisMinimum) > 20.0 / 21.0) )
                    fAxisMinimum -= rExplicitIncrement.Distance;
        }
        if( bAutoMaximum )
        {
            // round to entire multiples of the distance, based on the base value
            if( m_bExpandBorderToIncrementRhythm )
                fAxisMaximum = TickmarkHelper::getMaximumAtIncrement( fAxisMaximum, rExplicitIncrement );
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

    //---------------------------------------------------------------
    //fill explicit sub increment
    sal_Int32 nSubCount = m_aSourceScale.IncrementData.SubIncrements.getLength();
    rExplicitIncrement.SubIncrements.realloc(nSubCount);
    for( sal_Int32 nN=0; nN<nSubCount; nN++ )
    {
        const SubIncrement&     rSubIncrement         = m_aSourceScale.IncrementData.SubIncrements[nN];
        ExplicitSubIncrement&   rExplicitSubIncrement = rExplicitIncrement.SubIncrements[nN];

        if(!(rSubIncrement.IntervalCount>>=rExplicitSubIncrement.IntervalCount))
        {
            //scaling dependent
            //@todo autocalculate IntervalCount dependent on MainIncrement and scaling
            rExplicitSubIncrement.IntervalCount = 2;
        }
        lcl_ensureMaximumSubIncrementCount( rExplicitSubIncrement.IntervalCount );
        if(!(rSubIncrement.PostEquidistant>>=rExplicitSubIncrement.PostEquidistant))
        {
            //scaling dependent
            rExplicitSubIncrement.PostEquidistant = sal_False;
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
