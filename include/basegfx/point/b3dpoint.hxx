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

#ifndef INCLUDED_BASEGFX_POINT_B3DPOINT_HXX
#define INCLUDED_BASEGFX_POINT_B3DPOINT_HXX

#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B3DHomMatrix;

    /** Base Point class with three double values

        This class derives all operators and common handling for
        a 3D data class from B3DTuple. All necessary extensions
        which are special for points will be added here.

        @see B3DTuple
    */
    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC B3DPoint : public ::basegfx::B3DTuple
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

        /** constructor with tuple to allow copy-constructing
            from B3DTuple-based classes
        */
        B3DPoint(const ::basegfx::B3DTuple& rTuple)
        :   B3DTuple(rTuple)
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
    };

    // external operators


    /** Transform B3DPoint by given transformation matrix.

        Since this is a Point, translational components of the
        matrix are used.
    */
    BASEGFX_DLLPUBLIC B3DPoint operator*( const B3DHomMatrix& rMat, const B3DPoint& rPoint );

} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_POINT_B3DPOINT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
