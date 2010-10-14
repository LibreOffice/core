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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGON_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/range/b2drange.hxx>

// predeclarations
class ImplB2DPolyPolygon;

namespace basegfx
{
    class B2DPolygon;
    class B2DHomMatrix;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DPolyPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB2DPolyPolygon > ImplType;

    private:
        ImplType                                        mpPolyPolygon;

    public:
        B2DPolyPolygon();
        B2DPolyPolygon(const B2DPolyPolygon& rPolyPolygon);
        explicit B2DPolyPolygon(const B2DPolygon& rPolygon);
        ~B2DPolyPolygon();

        // assignment operator
        B2DPolyPolygon& operator=(const B2DPolyPolygon& rPolyPolygon);

        /// unshare this poly-polygon (and all included polygons) with all internally shared instances
        void makeUnique();

        // compare operators
        bool operator==(const B2DPolyPolygon& rPolyPolygon) const;
        bool operator!=(const B2DPolyPolygon& rPolyPolygon) const;

        // polygon interface
        sal_uInt32 count() const;

        B2DPolygon getB2DPolygon(sal_uInt32 nIndex) const;
        void setB2DPolygon(sal_uInt32 nIndex, const B2DPolygon& rPolygon);

        // test for curve
        bool areControlPointsUsed() const;

        // insert/append single polygon
        void insert(sal_uInt32 nIndex, const B2DPolygon& rPolygon, sal_uInt32 nCount = 1);
        void append(const B2DPolygon& rPolygon, sal_uInt32 nCount = 1);

        /** Default adaptive subdivision access

            For details refer to B2DPolygon::getDefaultAdaptiveSubdivision()

            @return
            The default subdivision of this polygon
        */
        B2DPolyPolygon getDefaultAdaptiveSubdivision() const;

        /** Get the B2DRange (Rectangle dimensions) of this B2DPolyPolygon

            For details refer to B2DPolygon::getB2DRange()

            @return
            The outer range of the bezier curve/polygon
        */
        B2DRange getB2DRange() const;

        // insert/append multiple polygons
        void insert(sal_uInt32 nIndex, const B2DPolyPolygon& rPolyPolygon);
        void append(const B2DPolyPolygon& rPolyPolygon);

        // remove
        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

        // reset to empty state
        void clear();

        // closed state
        bool isClosed() const;
        void setClosed(bool bNew);

        // flip polygon direction
        void flip();

        // test if PolyPolygon has double points
        bool hasDoublePoints() const;

        // remove double points, at the begin/end and follow-ups, too
        void removeDoublePoints();

        // apply transformation given in matrix form to the polygon
        void transform(const basegfx::B2DHomMatrix& rMatrix);

        // polygon iterators (same iterator validity conditions as for vector)
        const B2DPolygon* begin() const;
        const B2DPolygon* end() const;
        B2DPolygon* begin();
        B2DPolygon* end();
    };
} // end of namespace basegfx

#endif /* _BGFX_POLYGON_B2DPOLYPOLYGON_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
