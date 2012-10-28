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

#ifndef _BGFX_POINT_B3DPOINT_HXX
#define _BGFX_POINT_B3DPOINT_HXX

#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    // predeclaration
    class B3DHomMatrix;

    /** Base Point class with three double values

        This class derives all operators and common handling for
        a 3D data class from B3DTuple. All necessary extensions
        which are special for points will be added here.

        @see B3DTuple
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED B3DPoint : public ::basegfx::B3DTuple
    {
    public:
        /** Create a 3D Point

            The point is initialized to (0.0, 0.0, 0.0)
        */
        B3DPoint()
        :   B3DTuple()
        {}

        /** Create a 3D Point

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 3D Point.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 3D Point.

            @param fZ
            This parameter is used to initialize the Z-coordinate
            of the 3D Point.
        */
        B3DPoint(double fX, double fY, double fZ)
        :   B3DTuple(fX, fY, fZ)
        {}

        /** Create a copy of a 3D Point

            @param rVec
            The 3D Point which will be copied.
        */
        B3DPoint(const B3DPoint& rVec)
        :   B3DTuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B3DTuple-based classes
        */
        B3DPoint(const ::basegfx::B3DTuple& rTuple)
        :   B3DTuple(rTuple)
        {}

        ~B3DPoint()
        {}

        /** *=operator to allow usage from B3DPoint, too
        */
        B3DPoint& operator*=( const B3DPoint& rPnt )
        {
            mfX *= rPnt.mfX;
            mfY *= rPnt.mfY;
            mfZ *= rPnt.mfZ;
            return *this;
        }

        /** *=operator to allow usage from B3DPoint, too
        */
        B3DPoint& operator*=(double t)
        {
            mfX *= t;
            mfY *= t;
            mfZ *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B3DTuple calculations
        */
        B3DPoint& operator=( const ::basegfx::B3DTuple& rVec )
        {
            mfX = rVec.getX();
            mfY = rVec.getY();
            mfZ = rVec.getZ();
            return *this;
        }

        /** Transform point by given transformation matrix.

            The translational components of the matrix are, in
            contrast to B3DVector, applied.
        */
        B3DPoint& operator*=( const ::basegfx::B3DHomMatrix& rMat );

        static const B3DPoint& getEmptyPoint()
        {
            return (const B3DPoint&) ::basegfx::B3DTuple::getEmptyTuple();
        }
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    /** Transform B3DPoint by given transformation matrix.

        Since this is a Point, translational components of the
        matrix are used.
    */
    BASEGFX_DLLPUBLIC B3DPoint operator*( const B3DHomMatrix& rMat, const B3DPoint& rPoint );

} // end of namespace basegfx

#endif /* _BGFX_POINT_B3DPOINT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
