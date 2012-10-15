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

class AnimationBezierFunction
{
public:
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
