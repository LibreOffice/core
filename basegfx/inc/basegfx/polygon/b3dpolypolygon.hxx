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

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#define _BGFX_POLYGON_B3DPOLYPOLYGON_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>

// predeclarations
class ImplB3DPolyPolygon;

namespace basegfx
{
    class B3DPolygon;
    class B3DHomMatrix;
    class B2DHomMatrix;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B3DPolyPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB3DPolyPolygon > ImplType;

    private:
        ImplType                                        mpPolyPolygon;

    public:
        B3DPolyPolygon();
        B3DPolyPolygon(const B3DPolyPolygon& rPolyPolygon);
        explicit B3DPolyPolygon(const B3DPolygon& rPolygon);
        ~B3DPolyPolygon();

        // assignment operator
        B3DPolyPolygon& operator=(const B3DPolyPolygon& rPolyPolygon);

        /// unshare this poly-polygon (and all included polygons) with all internally shared instances
        void makeUnique();

        // compare operators
        bool operator==(const B3DPolyPolygon& rPolyPolygon) const;
        bool operator!=(const B3DPolyPolygon& rPolyPolygon) const;

        // polygon interface
        sal_uInt32 count() const;

        // B3DPolygon interface
        B3DPolygon getB3DPolygon(sal_uInt32 nIndex) const;
        void setB3DPolygon(sal_uInt32 nIndex, const B3DPolygon& rPolygon);

        // BColor interface
        bool areBColorsUsed() const;
        void clearBColors();

        // Normals interface
        void transformNormals(const B3DHomMatrix& rMatrix);
        bool areNormalsUsed() const;
        void clearNormals();

        // TextureCoordinate interface
        void transformTextureCoordiantes(const B2DHomMatrix& rMatrix);
        bool areTextureCoordinatesUsed() const;
        void clearTextureCoordinates();

        // insert/append single polygon
        void insert(sal_uInt32 nIndex, const B3DPolygon& rPolygon, sal_uInt32 nCount = 1);
        void append(const B3DPolygon& rPolygon, sal_uInt32 nCount = 1);

        // insert/append multiple polygons
        void insert(sal_uInt32 nIndex, const B3DPolyPolygon& rPolyPolygon);
        void append(const B3DPolyPolygon& rPolyPolygon);

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
        void transform(const basegfx::B3DHomMatrix& rMatrix);
    };
} // end of namespace basegfx

#endif /* _BGFX_POLYGON_B3DPOLYPOLYGON_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
