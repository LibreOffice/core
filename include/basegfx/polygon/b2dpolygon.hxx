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

#include <memory>
#include <ostream>
#include <vector>

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/basegfxdllapi.h>

class ImplB2DPolygon;

namespace basegfx
{
    class B2DPoint;
    class B2DRange;
    class B2DHomMatrix;
    class B2DCubicBezier;
    class SystemDependentData;
    class SystemDependentDataManager;
    typedef std::shared_ptr<SystemDependentData> SystemDependentData_SharedPtr;
}

namespace basegfx
{
    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC B2DPolygon
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
        B2DPolygon(B2DPolygon&& rPolygon);
        B2DPolygon(const B2DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount);
        B2DPolygon(std::initializer_list<basegfx::B2DPoint> rPoints);

        ~B2DPolygon();

        /// assignment operator
        B2DPolygon& operator=(const B2DPolygon& rPolygon);
        B2DPolygon& operator=(B2DPolygon&& rPolygon);

        /// unshare this polygon with all internally shared instances
        void makeUnique();

        /// compare operators
        bool operator==(const B2DPolygon& rPolygon) const;
        bool operator!=(const B2DPolygon& rPolygon) const;

        /// member count
        sal_uInt32 count() const;

        /// Coordinate interface
        basegfx::B2DPoint const & getB2DPoint(sal_uInt32 nIndex) const;
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
        void resetControlPoints();

        /// Bezier segment append with control points. The current last polygon point is implicitly taken as start point.
        void appendBezierSegment(const basegfx::B2DPoint& rNextControlPoint,
                                 const basegfx::B2DPoint& rPrevControlPoint,
                                 const basegfx::B2DPoint& rPoint);

        /// This is a shortcut to append a quadratic bezier segment. The current last polygon point is implicitly taken as start point.
        /// Note that the quadratic bezier control points will be converted to cubic bezier with 2 control points.
        void appendQuadraticBezierSegment(const basegfx::B2DPoint& rQuadControlPoint,
                                          const basegfx::B2DPoint& rPoint);

        /// ControlPoint checks
        bool areControlPointsUsed() const;
        bool isPrevControlPointUsed(sal_uInt32 nIndex) const;
        bool isNextControlPointUsed(sal_uInt32 nIndex) const;
        B2VectorContinuity getContinuityInPoint(sal_uInt32 nIndex) const;

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

            This method will return a default adaptive subdivision of the polygon.
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
            between the different kinds of subdivisioned and passing them one
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
        B2DPolygon const & getDefaultAdaptiveSubdivision() const;

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
        B2DRange const & getB2DRange() const;

        /** append other 2D polygons

            The default (nIndex ==0 && nCount == 0) will append
            the whole rPoly

            @param rPoly
            The source polygon

            @param nIndex
            The index to the first point of rPoly to append

            @param nCount
            The number of points to append from rPoly, starting
            from nIndex. If zero, as much as possible is appended
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

        // exclusive management op's for SystemDependentData at B2DPolygon
        template<class T>
        std::shared_ptr<T> getSystemDependentData() const
        {
            return std::static_pointer_cast<T>(getSystemDependantDataInternal(typeid(T).hash_code()));
        }

        template<class T, class... Args>
        std::shared_ptr<T> addOrReplaceSystemDependentData(SystemDependentDataManager& manager, Args&&... args) const
        {
            std::shared_ptr<T> r = std::make_shared<T>(manager, std::forward<Args>(args)...);

            // tdf#129845 only add to buffer if a relevant buffer time is estimated
            if(r->calculateCombinedHoldCyclesInSeconds() > 0)
            {
                basegfx::SystemDependentData_SharedPtr r2(r);
                addOrReplaceSystemDependentDataInternal(r2);
            }

            return r;
        }

    private:
        void addOrReplaceSystemDependentDataInternal(SystemDependentData_SharedPtr& rData) const;
        SystemDependentData_SharedPtr getSystemDependantDataInternal(size_t hash_code) const;
    };

    // typedef for a vector of B2DPolygons
    typedef ::std::vector< B2DPolygon > B2DPolygonVector;

    template< typename charT, typename traits >
    inline std::basic_ostream<charT, traits> & operator <<(
        std::basic_ostream<charT, traits> & stream, const B2DPolygon& poly )
    {
        stream << "<" << poly.count() << ":";
        for (sal_uInt32 i = 0; i < poly.count(); i++)
        {
            if (i > 0)
                stream << "--";
            stream << poly.getB2DPoint(i);
        }
        stream << ">";

        return stream;
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
