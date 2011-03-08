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

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#define _BGFX_POLYGON_B3DPOLYGON_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class ImplB3DPolygon;

namespace basegfx
{
    class B3DPolygon;
    class B3DPoint;
    class B3DHomMatrix;
    class B3DVector;
    class B2DPoint;
    class B2DHomMatrix;
    class BColor;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B3DPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB3DPolygon > ImplType;

    private:
        // internal data.
        ImplType                                    mpPolygon;

    public:
        B3DPolygon();
        B3DPolygon(const B3DPolygon& rPolygon);
        B3DPolygon(const B3DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount);
        ~B3DPolygon();

        // assignment operator
        B3DPolygon& operator=(const B3DPolygon& rPolygon);

        /// unshare this polygon with all internally shared instances
        void makeUnique();

        // compare operators
        bool operator==(const B3DPolygon& rPolygon) const;
        bool operator!=(const B3DPolygon& rPolygon) const;

        // member count
        sal_uInt32 count() const;

        // Coordinate interface
        B3DPoint getB3DPoint(sal_uInt32 nIndex) const;
        void setB3DPoint(sal_uInt32 nIndex, const B3DPoint& rValue);

        // Coordinate insert/append
        void insert(sal_uInt32 nIndex, const B3DPoint& rPoint, sal_uInt32 nCount = 1);
        void append(const B3DPoint& rPoint, sal_uInt32 nCount = 1);

        // BColor interface
        BColor getBColor(sal_uInt32 nIndex) const;
        void setBColor(sal_uInt32 nIndex, const BColor& rValue);
        bool areBColorsUsed() const;
        void clearBColors();

        // Normals interface
        B3DVector getNormal() const; // plane normal
        B3DVector getNormal(sal_uInt32 nIndex) const; // normal in each point
        void setNormal(sal_uInt32 nIndex, const B3DVector& rValue);
        void transformNormals(const B3DHomMatrix& rMatrix);
        bool areNormalsUsed() const;
        void clearNormals();

        // TextureCoordinate interface
        B2DPoint getTextureCoordinate(sal_uInt32 nIndex) const;
        void setTextureCoordinate(sal_uInt32 nIndex, const B2DPoint& rValue);
        void transformTextureCoordiantes(const B2DHomMatrix& rMatrix);
        bool areTextureCoordinatesUsed() const;
        void clearTextureCoordinates();

        // insert/append other 2D polygons
        void insert(sal_uInt32 nIndex, const B3DPolygon& rPoly, sal_uInt32 nIndex2 = 0, sal_uInt32 nCount = 0);
        void append(const B3DPolygon& rPoly, sal_uInt32 nIndex = 0, sal_uInt32 nCount = 0);

        // remove
        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

        // clear all points
        void clear();

        // closed state
        bool isClosed() const;
        void setClosed(bool bNew);

        // flip polygon direction
        void flip();

        // test if Polygon has double points
        bool hasDoublePoints() const;

        // remove double points, at the begin/end and follow-ups, too
        void removeDoublePoints();

        // apply transformation given in matrix form to the polygon
        void transform(const B3DHomMatrix& rMatrix);
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////


#endif /* _BGFX_POLYGON_B3DPOLYGON_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
