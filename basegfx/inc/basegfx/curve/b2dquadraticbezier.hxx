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

#ifndef _BGFX_CURVE_B2DQUADRATICBEZIER_HXX
#define _BGFX_CURVE_B2DQUADRATICBEZIER_HXX

#include <basegfx/point/b2dpoint.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DQuadraticBezier
    {
        ::basegfx::B2DPoint                         maStartPoint;
        ::basegfx::B2DPoint                         maEndPoint;
        ::basegfx::B2DPoint                         maControlPoint;

    public:
        B2DQuadraticBezier();
        B2DQuadraticBezier(const B2DQuadraticBezier& rBezier);
        B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd);
        B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart,
            const ::basegfx::B2DPoint& rControlPoint, const ::basegfx::B2DPoint& rEnd);
        ~B2DQuadraticBezier();

        // assignment operator
        B2DQuadraticBezier& operator=(const B2DQuadraticBezier& rBezier);

        // compare operators
        bool operator==(const B2DQuadraticBezier& rBezier) const;
        bool operator!=(const B2DQuadraticBezier& rBezier) const;

        // test if control point is placed on the edge
        bool isBezier() const;

        // data interface
        ::basegfx::B2DPoint getStartPoint() const { return maStartPoint; }
        void setStartPoint(const ::basegfx::B2DPoint& rValue) { maStartPoint = rValue; }

        ::basegfx::B2DPoint getEndPoint() const { return maEndPoint; }
        void setEndPoint(const ::basegfx::B2DPoint& rValue) { maEndPoint = rValue; }

        ::basegfx::B2DPoint getControlPoint() const { return maControlPoint; }
        void setControlPoint(const ::basegfx::B2DPoint& rValue) { maControlPoint = rValue; }
    };
} // end of namespace basegfx

#endif /* _BGFX_CURVE_B2DQUADRATICBEZIER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
