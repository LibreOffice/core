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

#ifndef _BGFX_POINT_B2IPOINT_HXX
#define _BGFX_POINT_B2IPOINT_HXX

#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    // predeclaration
    class B2DHomMatrix;

    /** Base Point class with two sal_Int32 values

        This class derives all operators and common handling for
        a 2D data class from B2ITuple. All necessary extensions
        which are special for points will be added here.

        @see B2ITuple
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED B2IPoint : public ::basegfx::B2ITuple
    {
    public:
        /** Create a 2D Point

            The point is initialized to (0, 0)
        */
        B2IPoint()
        :   B2ITuple()
        {}

        /** Create a 2D Point

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 2D Point.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 2D Point.
        */
        B2IPoint(sal_Int32 nX, sal_Int32 nY)
        :   B2ITuple(nX, nY)
        {}

        /** Create a copy of a 2D Point

            @param rPoint
            The 2D Point which will be copied.
        */
        B2IPoint(const B2IPoint& rPoint)
        :   B2ITuple(rPoint)
        {}

        /** constructor with tuple to allow copy-constructing
            from B2ITuple-based classes
        */
        B2IPoint(const ::basegfx::B2ITuple& rTuple)
        :   B2ITuple(rTuple)
        {}

        ~B2IPoint()
        {}

        /** *=operator to allow usage from B2IPoint, too
        */
        B2IPoint& operator*=( const B2IPoint& rPnt )
        {
            mnX *= rPnt.mnX;
            mnY *= rPnt.mnY;
            return *this;
        }

        /** *=operator to allow usage from B2IPoint, too
        */
        B2IPoint& operator*=(sal_Int32 t)
        {
            mnX *= t;
            mnY *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B2ITuple calculations
        */
        B2IPoint& operator=( const ::basegfx::B2ITuple& rPoint );

        /** Transform point by given transformation matrix.

            The translational components of the matrix are, in
            contrast to B2DVector, applied.
        */
        B2IPoint& operator*=( const ::basegfx::B2DHomMatrix& rMat );
    };
} // end of namespace basegfx

#endif /* _BGFX_POINT_B2IPOINT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
