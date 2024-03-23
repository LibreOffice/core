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
#include <basegfx/range/b2drange.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B2DPolygon;
}

namespace basegfx
{
    class BASEGFX_DLLPUBLIC B2DCubicBezier
    {
    private:
        B2DPoint                                        maStartPoint;
        B2DPoint                                        maEndPoint;
        B2DPoint                                        maControlPointA;
        B2DPoint                                        maControlPointB;

    public:
        B2DCubicBezier();
        B2DCubicBezier(const B2DCubicBezier& rBezier);
        B2DCubicBezier(const B2DPoint& rStart, const B2DPoint& rControlPointA, const B2DPoint& rControlPointB, const B2DPoint& rEnd);

        // assignment operator
        B2DCubicBezier& operator=(const B2DCubicBezier& rBezier);

        // compare operators
        bool operator==(const B2DCubicBezier& rBezier) const;
        bool equal(const B2DCubicBezier& rBezier) const;

        // test if vectors are used
        bool isBezier() const;

        // test if contained bezier is trivial and reset vectors accordingly
        void testAndSolveTrivialBezier();

        /** get length of edge

            This method handles beziers and simple edges. For
            beziers, the deviation describes the maximum allowed
            deviation from the real edge length. The default
            allows a deviation of 1% from the correct length.

            For beziers, there is no direct way to get the length,
            thus this method may subdivide the bezier edge and may
            not be cheap.

            @param fDeviation
            The maximal allowed deviation between correct length
            and bezier edge length

            @return
            The length of the edge
        */
        double getLength(double fDeviation = 0.01) const;

        // get distance between start and end point
        SAL_DLLPRIVATE double getEdgeLength() const;

        // get length of control polygon
        SAL_DLLPRIVATE double getControlPolygonLength() const;

        // data interface
        const B2DPoint& getStartPoint() const { return maStartPoint; }
        void setStartPoint(const B2DPoint& rValue) { maStartPoint = rValue; }

        const B2DPoint& getEndPoint() const { return maEndPoint; }
        void setEndPoint(const B2DPoint& rValue) { maEndPoint = rValue; }

        const B2DPoint& getControlPointA() const { return maControlPointA; }
        void setControlPointA(const B2DPoint& rValue) { maControlPointA = rValue; }

        const B2DPoint& getControlPointB() const { return maControlPointB; }
        void setControlPointB(const B2DPoint& rValue) { maControlPointB = rValue; }

        /** get the tangent in point t

            This method handles all the exceptions, e.g. when control point
            A is equal to start point and/or control point B is equal to end
            point

            @param t
            The bezier index in the range [0.0 .. 1.0]. It will be truncated.

            @return
            The tangent vector in point t
        */
        B2DVector getTangent(double t) const;

        /** adaptive subdivide by angle criteria
            no start point is added, but all necessary created edges
            and the end point
            #i37443# allow the criteria to get unsharp in recursions
        */
        SAL_DLLPRIVATE void adaptiveSubdivideByAngle(B2DPolygon& rTarget, double fAngleBound) const;

        /** #i37443# adaptive subdivide by nCount subdivisions
            no start point is added, but all necessary created edges
            and the end point
        */
        SAL_DLLPRIVATE void adaptiveSubdivideByCount(B2DPolygon& rTarget, sal_uInt32 nCount) const;

        /** Subdivide cubic bezier segment.

            This function adaptively subdivides the bezier
            segment into as much straight line segments as necessary,
            such that the maximal orthogonal distance from any of the
            segments to the true curve is less than the given error
            value.
            No start point is added, but all necessary created edges
            and the end point

            @param rPoly
            Output polygon. The subdivided bezier segment is added to
            this polygon via B2DPolygon::append().

            @param rCurve
            The cubic bezier curve to subdivide

            @param fDistanceBound
            Bound on the maximal distance of the approximation to the
            true curve.

            @param nRecurseLimit
            Bound on recursion for the bezier case.
        */
        void adaptiveSubdivideByDistance(B2DPolygon& rTarget, double fDistanceBound, int nRecurseLimit = 30) const;

        // get point at given relative position
        B2DPoint interpolatePoint(double t) const;

        // calculate the smallest distance from given point to this cubic bezier segment
        // and return the value. The relative position on the segment is returned in rCut.
        SAL_DLLPRIVATE double getSmallestDistancePointToBezierSegment(const B2DPoint& rTestPoint, double& rCut) const;

        // do a split at position t and fill both resulting segments
        void split(double t, B2DCubicBezier* pBezierA, B2DCubicBezier* pBezierB) const;

        // extract snippet from fStart to fEnd from this bezier
        SAL_DLLPRIVATE B2DCubicBezier snippet(double fStart, double fEnd) const;

        // get range including control points
        B2DRange getRange() const;

        /** Get the minimum extremum position t

            @param rfResult
            Will be changed and might possibly be set to a found split value, which should be in the
            range [0.0 .. 1.0]. It will be the smallest current extremum; there may be more

            @return
            Returns true if there was at least one extremum found
        */
        SAL_DLLPRIVATE bool getMinimumExtremumPosition(double& rfResult) const;

        /** Get all extremum pos of this segment

            This method will calculate all extremum positions of the segment
            and add them to rResults if they are in the range ]0.0 .. 1.0[

            @param rResults
            The vector of doubles where the results will be added. Evtl.
            existing contents will be removed since an empty vector is a
            necessary result to express that there are no extreme positions
            anymore. Since there is an upper maximum of 4 values, it makes
            sense to use reserve(4) at the vector as preparation.
        */
        SAL_DLLPRIVATE void getAllExtremumPositions(::std::vector< double >& rResults) const;

        /// apply transformation given in matrix form
        void transform(const basegfx::B2DHomMatrix& rMatrix);

        /// fround content
        SAL_DLLPRIVATE void fround();
    };
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
