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

#include "controller/SlsAnimationFunction.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"


#include <osl/diagnose.hxx>
#include <rtl/math.hxx>

namespace sd { namespace slidesorter { namespace controller {


double AnimationFunction::Linear (const double nTime)
{
    OSL_ASSERT(nTime>=0.0 && nTime<=1.0);
    return nTime;
}




double AnimationFunction::Blend (
    const double nStartValue,
    const double nEndValue,
    const double nTime)
{
    return nStartValue*(1-nTime) + nEndValue*nTime;
}




//===== AnimationBezierFunction ===============================================

AnimationBezierFunction::AnimationBezierFunction (
    const double nX1,
    const double nY1)
    : mnX1(nX1),
      mnY1(nY1),
      mnX2(1-nY1),
      mnY2(1-nX1)
{
}




::basegfx::B2DPoint AnimationBezierFunction::operator() (const double nT)
{
    return ::basegfx::B2DPoint(
        EvaluateComponent(nT, mnX1, mnX2),
        EvaluateComponent(nT, mnY1, mnY2));
}




double AnimationBezierFunction::EvaluateComponent (
    const double nT,
    const double nV1,
    const double nV2)
{
    const double nS (1-nT);

    // While the control point values 1 and 2 are explicitly given the start
    // and end values are implicitly given.
    const double nV0 (0);
    const double nV3 (1);

    const double nV01 (nS*nV0 + nT*nV1);
    const double nV12 (nS*nV1 + nT*nV2);
    const double nV23 (nS*nV2 + nT*nV3);

    const double nV012 (nS*nV01 + nT*nV12);
    const double nV123 (nS*nV12 + nT*nV23);

    const double nV0123 (nS*nV012 + nT*nV123);

    return nV0123;
}




//===== AnimationParametricFunction ===========================================

AnimationParametricFunction::AnimationParametricFunction (const ParametricFunction& rFunction)
    : maY()
{
    const sal_Int32 nSampleCount (64);

    // Sample the given parametric function.
    ::std::vector<basegfx::B2DPoint> aPoints;
    aPoints.reserve(nSampleCount);
    for (sal_Int32 nIndex=0; nIndex<nSampleCount; ++nIndex)
    {
        const double nT (nIndex/double(nSampleCount-1));
        aPoints.push_back(basegfx::B2DPoint(rFunction(nT)));
    }

    // Interpolate at evenly spaced points.
    maY.clear();
    maY.reserve(nSampleCount);
    double nX0 (aPoints[0].getX());
    double nY0 (aPoints[0].getY());
    double nX1 (aPoints[1].getX());
    double nY1 (aPoints[1].getY());
    sal_Int32 nIndex (1);
    for (sal_Int32 nIndex2=0; nIndex2<nSampleCount; ++nIndex2)
    {
        const double nX (nIndex2 / double(nSampleCount-1));
        while (nX > nX1 && nIndex<nSampleCount)
        {
            nX0 = nX1;
            nY0 = nY1;
            nX1 = aPoints[nIndex].getX();
            nY1 = aPoints[nIndex].getY();
            ++nIndex;
        }
        const double nU ((nX-nX1) / (nX0 - nX1));
        const double nY (nY0*nU + nY1*(1-nU));
        maY.push_back(nY);
    }
}




double AnimationParametricFunction::operator() (const double nX)
{
    const sal_Int32 nIndex0 (static_cast<sal_Int32>(nX * maY.size()));
    const double nX0 (nIndex0 / double(maY.size()-1));
    const sal_uInt32 nIndex1 (nIndex0 + 1);
    const double nX1 (nIndex1 / double(maY.size()-1));

    if (nIndex0<=0)
        return maY[0];
    else if (sal_uInt32(nIndex0)>=maY.size() || nIndex1>=maY.size())
        return maY[maY.size()-1];

    const double nU ((nX-nX1) / (nX0 - nX1));
    return maY[nIndex0]*nU + maY[nIndex1]*(1-nU);
}


} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
