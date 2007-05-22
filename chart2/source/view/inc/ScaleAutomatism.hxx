/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScaleAutomatism.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:20:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CHART2_SCALEAUTOMATISM_HXX
#define _CHART2_SCALEAUTOMATISM_HXX

#ifndef _COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_SCALEDATA_HPP_
#include <com/sun/star/chart2/ScaleData.hpp>
#endif

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
                            const ::com::sun::star::chart2::ScaleData& rSourceScale );
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

    /** Fills the passed scale data and increment data according to the own settings. */
    void                calculateExplicitScaleAndIncrement(
                            ::com::sun::star::chart2::ExplicitScaleData& rExplicitScale,
                            ::com::sun::star::chart2::ExplicitIncrementData& rExplicitIncrement ) const;

private:
    /** Fills the passed scale data and increment data for category scaling. */
    void                calculateExplicitIncrementAndScaleForCategory(
                            ::com::sun::star::chart2::ExplicitScaleData& rExplicitScale,
                            ::com::sun::star::chart2::ExplicitIncrementData& rExplicitIncrement,
                            bool bAutoMinimum, bool bAutoMaximum ) const;

    /** Fills the passed scale data and increment data for logarithmic scaling. */
    void                calculateExplicitIncrementAndScaleForLogarithmic(
                            ::com::sun::star::chart2::ExplicitScaleData& rExplicitScale,
                            ::com::sun::star::chart2::ExplicitIncrementData& rExplicitIncrement,
                            bool bAutoMinimum, bool bAutoMaximum ) const;

    /** Fills the passed scale data and increment data for linear scaling. */
    void                calculateExplicitIncrementAndScaleForLinear(
                            ::com::sun::star::chart2::ExplicitScaleData& rExplicitScale,
                            ::com::sun::star::chart2::ExplicitIncrementData& rExplicitIncrement,
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
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
