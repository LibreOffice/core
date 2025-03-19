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

#include <ostream>
#include <tuple>

#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/basegfxdllapi.h>
#include <basegfx/tuple/Size2D.hxx>

namespace basegfx
{
    class B2DHomMatrix;

    /** Base Point class with two double values

        This class derives all operators and common handling for
        a 2D data class from B2DTuple. All necessary extensions
        which are special for points will be added here.

        @see B2DTuple
    */
    class SAL_WARN_UNUSED B2DPoint : public ::basegfx::B2DTuple
    {
    public:
        /** Create a 2D Point

            The point is initialized to (0.0, 0.0)
        */
        B2DPoint()
        {}

        /** Create a 2D Point

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 2D Point.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 2D Point.
        */
        B2DPoint(double fX, double fY)
        :   B2DTuple(fX, fY)
        {}

        /** Create a copy of a 2D Point

            @param rPoint
            The 2D Point which will be copied.
        */
        explicit B2DPoint(const ::basegfx::B2IPoint& rPoint)
        :   B2DTuple(rPoint)
        {}

        /** constructor with tuple to allow copy-constructing
            from B2DTuple-based classes
        */
        B2DPoint(Tuple2D<double> const& rTuple)
        :   B2DTuple(rTuple)
        {}

        /** create a point from a size object */
        explicit B2DPoint(Size2D<double> const& rSize)
            : B2DTuple(rSize.getWidth(), rSize.getHeight())
        {}

        /** *=operator to allow usage from B2DPoint, too
        */
        B2DPoint& operator*=( const B2DPoint& rPnt )
        {
            mnX *= rPnt.mnX;
            mnY *= rPnt.mnY;
            return *this;
        }

        /** *=operator to allow usage from B2DPoint, too
        */
        B2DPoint& operator*=(double t)
        {
            mnX *= t;
            mnY *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B2DTuple calculations
        */
        BASEGFX_DLLPUBLIC B2DPoint& operator=(const Tuple2D<double>& rPoint)
        {
            mnX = rPoint.getX();
            mnY = rPoint.getY();
            return *this;
        }

        /** Transform point by given transformation matrix.

            The translational components of the matrix are, in
            contrast to B2DVector, applied.
        */
        BASEGFX_DLLPUBLIC B2DPoint& operator*=( const ::basegfx::B2DHomMatrix& rMat );

        static const B2DPoint& getEmptyPoint()
        {
            return static_cast<const B2DPoint&>( ::basegfx::B2DTuple::getEmptyTuple() );
        }

        friend auto operator <=>(B2DPoint const & a, B2DPoint const & b)
        {
            return std::tie(a.mnX, a.mnY) <=> std::tie(b.mnX, b.mnY);
        }
    };

    // external operators

    /** Transform B2DPoint by given transformation matrix.

        Since this is a Point, translational components of the
        matrix are used.
    */
    BASEGFX_DLLPUBLIC B2DPoint operator*( const B2DHomMatrix& rMat, const B2DPoint& rPoint );

    template< typename charT, typename traits >
    inline std::basic_ostream<charT, traits> & operator <<(
        std::basic_ostream<charT, traits> & stream, const B2DPoint& point )
    {
        return stream << "(" << point.getX() << "," << point.getY() << ")";
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
