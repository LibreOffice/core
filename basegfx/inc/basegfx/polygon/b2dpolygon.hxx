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

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#define _BGFX_POLYGON_B2DPOLYGON_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/range/b2drange.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class ImplB2DPolygon;

namespace basegfx
{
    class B2DPolygon;
    class B2DPoint;
    class B2DVector;
    class B2DHomMatrix;
    class B2DCubicBezier;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB2DPolygon > ImplType;

    private:
        // internal data.
        ImplType                                    mpPolygon;

    public:
        /// diverse constructors
        B2DPolygon();
        B2DPolygon(const B2DPolygon& rPolygon);
        B2DPolygon(const B2DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount);
        ~B2DPolygon();

        /// assignment operator
        B2DPolygon& operator=(const B2DPolygon& rPolygon);

        /// unshare this polygon with all internally shared instances
        void makeUnique();

        /// compare operators
        bool operator==(const B2DPolygon& rPolygon) const;
        bool operator!=(const B2DPolygon& rPolygon) const;

        /// member count
        sal_uInt32 count() const;

        /// Coordinate interface
        basegfx::B2DPoint getB2DPoint(sal_uInt32 nIndex) const;
        void setB2DPoint(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue);

        /// Coordinate insert/append
        void insert(sal_uInt32 nIndex, const basegfx::B2DPoint& rPoint, sal_uInt32 nCount = 1);
        void append(const basegfx::B2DPoint& rPoint, sal_uInt32 nCount);
        void append(const basegfx::B2DPoint& rPoint);
        void reserve(sal_uInt32 nCount);

        /// Basic ControlPoint interface
        basegfx::B2DPoint getPrevControlPoint(sal_uInt32 nIndex) const;
        basegfx::B2DPoint getNextControlPoint(sal_uInt32 nIndex) const;
        void setPrevControlPoint(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue);
        void setNextControlPoint(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue);
        void setControlPoints(sal_uInt32 nIndex, const basegfx::B2DPoint& rPrev, const basegfx::B2DPoint& rNext);

        /// ControlPoint resets
        void resetPrevControlPoint(sal_uInt32 nIndex);
        void resetNextControlPoint(sal_uInt32 nIndex);
        void resetControlPoints(sal_uInt32 nIndex);
        void resetControlPoints();

        /// Bezier segment append with control points. The current last polygon point is implicitly taken as start point.
        void appendBezierSegment(const basegfx::B2DPoint& rNextControlPoint, const basegfx::B2DPoint& rPrevControlPoint, const basegfx::B2DPoint& rPoint);

        /// ControlPoint checks
        bool areControlPointsUsed() const;
        bool isPrevControlPointUsed(sal_uInt32 nIndex) const;
        bool isNextControlPointUsed(sal_uInt32 nIndex) const;
        B2VectorContinuity getContinuityInPoint(sal_uInt32 nIndex) const;

        /** check edge for being a bezier segment

            This test the existance of control vectors, but do not apply
            testAndSolveTrivialBezier() to the bezier segment, so it is still useful
            to do so.
            Since it can use internal data representations, it is faster
            than using getBezierSegment() and applying isBezier() on it.

            @param nIndex
            Index of the addressed edge's start point

            @return
            true if edge exists and at least one control vector is used
        */
        bool isBezierSegment(sal_uInt32 nIndex) const;

        /** bezier segment access

            This method also works when it is no bezier segment at all and will fill
            the given B2DCubicBezier as needed.
            In any case, the given B2DCubicBezier will be filled, if necessary with
            the single start point (if no valid edge exists).

            @param nIndex
            Index of the addressed edge's start point

            @param rTarget
            The B2DCubicBezier to be filled. It's data WILL be changed.
        */
        void getBezierSegment(sal_uInt32 nIndex, B2DCubicBezier& rTarget) const;

        /** Default adaptive subdivision access

            This method will return a default adapive subdivision of the polygon.
            If the polygon does not contain any bezier curve segments, it will
            just return itself.

            The subdivision is created on first request and buffered, so when using
            this subdivision You have the guarantee for fast accesses for multiple
            usages. It is intended for tooling usage for tasks which would be hard
            to accomplish on bezier segments (e.g. isInEpsilonRange).

            The current default subdivision uses adaptiveSubdivideByCount with 9
            subdivisions which gives 10 edges and 11 points per segment and is
            usually pretty usable for processing purposes. There is no parameter
            passing here ATM but it may be changed on demand. If needed, a TYPE
            and PARAMETER (both defaulted) may be added to allow for switching
            between the different kinds of subdivisiond and passing them one
            parameter.

            The lifetime of the buffered subdivision is based on polygon changes.
            When changing the polygon, it will be flushed. It is buffered at the
            refcounted implementation class, so it will survive copy by value and
            combinations in PolyPolygons.

            @return
            The default (and buffered) subdivision of this polygon. It may
            be this polygon itself when it has no bezier segments. It is guaranteed
            to have no more bezier segments
        */
        B2DPolygon getDefaultAdaptiveSubdivision() const;

        /** Get the B2DRange (Rectangle dimensions) of this B2DPolygon

            A polygon may have up to three ranges:

            (a) the range of the polygon points
            (b) the range of the polygon points and control points
            (c) the outer range of the subdivided bezier curve

            Ranges (a) and (c) are produced by tools::getRange(); resp. this
            getB2DRange(). tools::getRangeWithControlPoints handles case (b).

            To get range (c) a simple solution would be to subdivide the polygon
            and use getRange() on it. Since subdivision is expensive and decreases
            the polygon quality, i added this new method. It will use a
            methodology suggested by HDU. First, it gets the range (a).
            Then it iterates over the bezier segments and for each it
            first tests if the outer range of the bezier segment is already
            contained in the result range.

            The subdivision itself uses getAllExtremumPositions() to only
            calculate extremum points and to expand the result accordingly.
            Thus it calculates maximal four extremum points on the bezier
            segment, no split is used at all.

            @return
            The outer range of the bezier curve/polygon
        */
        B2DRange getB2DRange() const;

        /** insert other 2D polygons

            The default (with nIndex2 == 0 && nCount == 0) inserts the whole
            rPoly at position nIndex

            @param nIndex
            Target index for points to be inserted

            @param rPoly
            The source for new points

            @param nIndex2
            The index to the first source point into rPoly

            @param nCount
            How many points to add from rPoly to this polygon. Null
            means to copy all (starting from nIndex2)
        */
        void insert(sal_uInt32 nIndex, const B2DPolygon& rPoly, sal_uInt32 nIndex2 = 0, sal_uInt32 nCount = 0);

        /** append other 2D polygons

            The default (nIndex ==0 && nCount == 0) will append
            the whole rPoly

            @param rPoly
            The source polygon

            @param nIndex
            The index to the first point of rPoly to append

            @param nCount
            The number of points to append from rPoly, starting
            from nIndex. If zero, as much as possibel is appended
        */
        void append(const B2DPolygon& rPoly, sal_uInt32 nIndex = 0, sal_uInt32 nCount = 0);

        /// remove points
        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

        /// clear all points
        void clear();

        /// closed state interface
        bool isClosed() const;
        void setClosed(bool bNew);

        /// flip polygon direction
        void flip();

        /// test if Polygon has double points
        bool hasDoublePoints() const;

        /// remove double points, at the begin/end and follow-ups, too
        void removeDoublePoints();

        /// apply transformation given in matrix form
        void transform(const basegfx::B2DHomMatrix& rMatrix);

        // point iterators (same iterator validity conditions as for vector)
        const B2DPoint* begin() const;
        const B2DPoint* end() const;
        B2DPoint* begin();
        B2DPoint* end();
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_B2DPOLYGON_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
