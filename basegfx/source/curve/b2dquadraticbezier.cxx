/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dquadraticbezier.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:40:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BGFX_CURVE_B2DQUADRATICBEZIER_HXX
#include <basegfx/curve/b2dquadraticbezier.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

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
