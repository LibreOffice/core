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

#ifndef SD_SLIDESORTER_CONTROLLER_ANIMATION_FUNCTION_HXX
#define SD_SLIDESORTER_CONTROLLER_ANIMATION_FUNCTION_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include <basegfx/point/b2dpoint.hxx>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <tools/gen.hxx>
#include <vector>

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }



namespace sd { namespace slidesorter { namespace controller {

/** A collection of functions that are usefull when creating animations.
    They are collected here until a better place is found.
*/
class AnimationFunction
    : private ::boost::noncopyable
{
public:
    /** Acceleration function that maps [0,1] to [0,1] linearly, ie it
        returns the given time value unaltered.
    */
    static double Linear (const double nTime);

    /** Acceleration function that maps [0,1] to [0,1].  Speed starts fast
        and ends slow following the sine function.
    */
    static double FastInSlowOut_Sine (const double nTime);

    /** Acceleration function that maps [0,1] to [0,1].  Speed starts fast
        and ends slow following the square root function.
    */
    static double FastInSlowOut_Root (const double nTime);

    /** Acceleration function that maps [0,1] to [0,0].  Speed starts slow,
        rises, drops and ends slow following the sine function.
    */
    static double SlowInSlowOut_0to0_Sine (const double nTime);

    /** Acceleration function that maps [0,1] to [0,0].  Speed starts slow,
        rises and drops several times and ends slow following multiple
        cycles of the the sine function.
    */
    static double Vibrate_Sine (const double nTime);

    /** Scale point linearly.
    */
    static Point ScalePoint (const Point& rPoint, const double nTime);

    /** Blend two points together according to the given weight.
    */
    static double Blend (const double nStartValue, const double nEndValue, const double nWeight);

    /** Apply a gradual visual state change.  The kind of change, i.e. the
        previous and the new states are expected to be already set.  This
        method only adjusts the blending of the visual representation from
        one state to the other.
    */
    static void ApplyVisualStateChange (
        const model::SharedPageDescriptor& rpDescriptor,
        view::SlideSorterView& rView,
        const double nTime);

    /** Apply a gradual change of a previously set offset to the location of
        a page object.
    */
    static void ApplyLocationOffsetChange (
        const model::SharedPageDescriptor& rpDescriptor,
        view::SlideSorterView& rView,
        const Point aLocationOffset);

    /** Apply a gradual change the alpha value from the old value to a
        new value (set prior to this call.)
    */
    static void ApplyButtonAlphaChange(
        const model::SharedPageDescriptor& rpDescriptor,
        view::SlideSorterView& rView,
        const double nButtonAlpha,
        const double nButtonBarAlpha);
};




class AnimationBezierFunction
{
public:
    /** Create a cubic bezier curve whose start and end points are given
        implicitly as P0=(0,0) and P3=(1,1).
    */
    AnimationBezierFunction (
        const double nX1,
        const double nY1,
        const double nX2,
        const double nY2);

    /** Create a cubic bezier curve whose start and end points are given
        implicitly as P0=(0,0) and P3=(1,1).  The second control point is
        implicitly given as P2=(1-nY1,1-nX1).
    */
    AnimationBezierFunction (
        const double nX1,
        const double nY1);

    ::basegfx::B2DPoint operator() (const double nT);

private:
    const double mnX1;
    const double mnY1;
    const double mnX2;
    const double mnY2;

    double EvaluateComponent (
        const double nT,
        const double nV1,
        const double nV2);
};




/** Turn a parametric function into one whose y-Values depend on its
    x-Values.  Note a lot of interpolation takes place.  The resulting
    accuracy should be good enough for the purpose of acceleration
    function for animations.
*/
class AnimationParametricFunction
{
public:
    typedef ::boost::function<basegfx::B2DPoint(double)> ParametricFunction;
    AnimationParametricFunction (const ParametricFunction& rFunction);

    double operator() (const double nX);

private:
    /** y-Values of the parametric function given to the constructor
        evaluated (and interpolated) for evenly spaced x-Values.
    */
    ::std::vector<double> maY;
};




} } } // end of namespace ::sd::slidesorter::controller

#endif
