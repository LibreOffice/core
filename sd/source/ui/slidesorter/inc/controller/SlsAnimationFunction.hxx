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

#pragma once

#include <basegfx/point/b2dpoint.hxx>

#include <functional>
#include <vector>


namespace sd::slidesorter::controller {

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

    static double EvaluateComponent (
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
    typedef ::std::function<basegfx::B2DPoint (double)> ParametricFunction;
    AnimationParametricFunction (const ParametricFunction& rFunction);

    double operator() (const double nX);

private:
    /** y-Values of the parametric function given to the constructor
        evaluated (and interpolated) for evenly spaced x-Values.
    */
    ::std::vector<double> maY;
};

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
