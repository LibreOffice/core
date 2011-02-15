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
#ifndef _CHART2_EXPLICITSCALEVALUES_HXX
#define _CHART2_EXPLICITSCALEVALUES_HXX

#include "chartviewdllapi.hxx"
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart2/AxisOrientation.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XScaling.hpp>
#include <tools/date.hxx>
#include <vector>
//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/** This structure contains the explicit values for a scale like Minimum and Maximum.
    See also ::com::sun::star::chart2::ScaleData.
*/

struct OOO_DLLPUBLIC_CHARTVIEW ExplicitScaleData
{
    ExplicitScaleData();

    double Minimum;
    double Maximum;
    double Origin;

    ::com::sun::star::chart2::AxisOrientation     Orientation;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling > Scaling;

    sal_Int32   AxisType;//see ::com::sun::star::chart2::AxisType
    bool        ShiftedCategoryPosition;
    sal_Int32   TimeResolution; //constant of type <type>::com::sun::star::chart::TimeUnit</type>
    Date        NullDate;
};

struct OOO_DLLPUBLIC_CHARTVIEW ExplicitSubIncrement
{
    ExplicitSubIncrement();

    /** Numbers of intervals between two superior ticks.  For an axis
        this usually means, that <code>IntervalCount - 1</code>
        sub-tick-marks are displayed between two superior ticks.

     */
    sal_Int32 IntervalCount;

    /** If <TRUE/>, the distance between two sub-tick-marks on the
        screen is always the same.  If <FALSE/>, the distances may
        differ depending on the <type>XScaling</type>.
     */
    bool PostEquidistant;
};

/** describes how tickmarks are positioned on the scale of an axis.
*/
struct OOO_DLLPUBLIC_CHARTVIEW ExplicitIncrementData
{
    ExplicitIncrementData();

    /** the following two members are only for date-time axis
    */
    ::com::sun::star::chart::TimeInterval   MajorTimeInterval;
    ::com::sun::star::chart::TimeInterval   MinorTimeInterval;

    /** the other members are for *not* date-time axis
    */

    /** <member>Distance</member> describes the distance between two
    neighboring main tickmarks on a <type>Scale</type> of an axis.
    All neighboring main tickmarks have the same constant distance.

    <p>If the Scale has a <type>XScaling</type> the <member>Distance</member>
    may be measured in two different ways - that is - before or after the
    scaling is applied.</p>

    <p>On a logarithmic scale for example the distance between two main
    tickmarks is typically measured after the scaling is applied:
    Distance = log(tick2)-log(tick1)
    ( log(1000)-log(100)==log(100)-log(10)==log(10)-log(1)==1==Distance ).
    The resulting tickmarks will always look equidistant on the screen.
    The other possibility is to have a Distance = tick2-tick1 measured constant
    before a scaling is applied, which may lead to non equidistant tickmarks
    on the screen.</p>

    <p><member>PostEquidistant</member> rules wether the <member>Distance</member>
    is meant to be a value before or after scaling.</p>
    */
    double      Distance;

    /**
    <member>PostEquidistant</member> rules wether the member <member>Distance</member>
    describes a distance before or after the scaling is applied.

    <p>If <member>PostEquidistant</member> equals <TRUE/> <member>Distance</member>
    is given in values after <type>XScaling</type> is applied, thus resulting
    main tickmarks will always look equidistant on the screen.
    If <member>PostEquidistant</member> equals <FALSE/> <member>Distance</member>
    is given in values before <type>XScaling</type> is applied.</p>
    */
    bool        PostEquidistant;

    /** The <member>BaseValue</member> gives a starting point on the scale
    to which all further main tickmarks are relatively positioned.

    <p>The <member>BaseValue</member> is always a value on the scale before
    a possible scaling is applied. If the given value is not valid in the
    associated scaling the minimum of the scaling is assumed,
    if there is no minimum any other obvious value will be assumed.</p>

    <p>E.g.: assume a scale from 0 to 6 with identical scaling.
    Further assume this Increment to have Distance==2 and PostEquidistant==false.
    Setting BaseValue=0 would lead to main tickmarks 0; 2; 4; 6;
    Setting BaseValue=1,3 would lead to main tickmarks 1,3; 3,3; 5,3;
    Setting BaseValue=-0,7 would also lead to main tickmarks 1,3; 3,3; 5,3;
    And setting BaseValue to 2, -2, 4, -4 etc. in this example
    leads to the same result as BaseValue=0.</p>
    */
    double      BaseValue;

    /** <member>SubIncrements</member> describes the positioning of further
    sub tickmarks on the scale of an axis.

    <p>The first SubIncrement in this sequence determines how the
    distance between two neighboring main tickmarks is divided for positioning
    of further sub tickmarks. Every following SubIncrement determines the
    positions of subsequent tickmarks in relation to their parent tickmarks
    iven by the preceding SubIncrement.</p>
    */
    ::std::vector< ExplicitSubIncrement > SubIncrements;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
