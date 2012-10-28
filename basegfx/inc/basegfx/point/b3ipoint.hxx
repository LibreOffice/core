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

#ifndef _BGFX_POINT_B3IPOINT_HXX
#define _BGFX_POINT_B3IPOINT_HXX

#include <basegfx/tuple/b3ituple.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    // predeclaration
    class B3DHomMatrix;

    /** Base Point class with three sal_Int32 values

        This class derives all operators and common handling for
        a 3D data class from B3ITuple. All necessary extensions
        which are special for points will be added here.

        @see B3ITuple
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED B3IPoint : public ::basegfx::B3ITuple
    {
    public:
        /** Create a 3D Point

            The point is initialized to (0, 0, 0)
        */
        B3IPoint()
        :   B3ITuple()
        {}

        /** Create a 3D Point

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 3D Point.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 3D Point.

            @param nZ
            This parameter is used to initialize the Z-coordinate
            of the 3D Point.
        */
        B3IPoint(sal_Int32 nX, sal_Int32 nY, sal_Int32 nZ)
        :   B3ITuple(nX, nY, nZ)
        {}

        /** Create a copy of a 3D Point

            @param rVec
            The 3D Point which will be copied.
        */
        B3IPoint(const B3IPoint& rVec)
        :   B3ITuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B3ITuple-based classes
        */
        B3IPoint(const ::basegfx::B3ITuple& rTuple)
        :   B3ITuple(rTuple)
        {}

        ~B3IPoint()
        {}

        /** *=operator to allow usage from B3IPoint, too
        */
        B3IPoint& operator*=( const B3IPoint& rPnt )
        {
            mnX *= rPnt.mnX;
            mnY *= rPnt.mnY;
            mnZ *= rPnt.mnZ;
            return *this;
        }

        /** *=operator to allow usage from B3IPoint, too
        */
        B3IPoint& operator*=(sal_Int32 t)
        {
            mnX *= t;
            mnY *= t;
            mnZ *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B3ITuple calculations
        */
        B3IPoint& operator=( const ::basegfx::B3ITuple& rVec )
        {
            mnX = rVec.getX();
            mnY = rVec.getY();
            mnZ = rVec.getZ();
            return *this;
        }

        /** Transform point by given transformation matrix.

            The translational components of the matrix are, in
            contrast to B3DVector, applied.
        */
        B3IPoint& operator*=( const ::basegfx::B3DHomMatrix& rMat );
    };
} // end of namespace basegfx

#endif /* _BGFX_POINT_B3IPOINT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
