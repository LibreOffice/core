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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/curve/b2dquadraticbezier.hxx>
#include <basegfx/numeric/ftools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    B2DQuadraticBezier::B2DQuadraticBezier(const B2DQuadraticBezier& rBezier)
    :   maStartPoint(rBezier.maStartPoint),
        maEndPoint(rBezier.maEndPoint),
        maControlPoint(rBezier.maControlPoint)
    {
    }

    B2DQuadraticBezier::B2DQuadraticBezier()
    {
    }

    B2DQuadraticBezier::B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd)
    :   maStartPoint(rStart),
        maEndPoint(rEnd)
    {
    }

    B2DQuadraticBezier::B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rControl, const ::basegfx::B2DPoint& rEnd)
    :   maStartPoint(rStart),
        maEndPoint(rEnd),
        maControlPoint(rControl)
    {
    }

    B2DQuadraticBezier::~B2DQuadraticBezier()
    {
    }

    // assignment operator
    B2DQuadraticBezier& B2DQuadraticBezier::operator=(const B2DQuadraticBezier& rBezier)
    {
        maStartPoint = rBezier.maStartPoint;
        maEndPoint = rBezier.maEndPoint;
        maControlPoint = rBezier.maControlPoint;

        return *this;
    }

    // compare operators
    bool B2DQuadraticBezier::operator==(const B2DQuadraticBezier& rBezier) const
    {
        return (
            maStartPoint == rBezier.maStartPoint
            && maEndPoint == rBezier.maEndPoint
            && maControlPoint == rBezier.maControlPoint
        );
    }

    bool B2DQuadraticBezier::operator!=(const B2DQuadraticBezier& rBezier) const
    {
        return (
            maStartPoint != rBezier.maStartPoint
            || maEndPoint != rBezier.maEndPoint
            || maControlPoint != rBezier.maControlPoint
        );
    }

    // test if control vector is used
    bool B2DQuadraticBezier::isBezier() const
    {
        // if control vector is empty, bezier is not used
        if(maControlPoint == maStartPoint || maControlPoint == maEndPoint)
            return false;

        return true;
    }
} // end of namespace basegfx

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
