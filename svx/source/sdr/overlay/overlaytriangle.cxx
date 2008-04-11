/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaytriangle.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_svx.hxx"
#include <svx/sdr/overlay/overlaytriangle.hxx>
#include <tools/poly.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayTriangleStriped::drawGeometry(OutputDevice& rOutputDevice)
        {
            basegfx::B2DPolygon aPolygon;

            aPolygon.append(getBasePosition());
            aPolygon.append(getSecondPosition());
            aPolygon.append(getThirdPosition());
            aPolygon.setClosed(true);

            ImpDrawPolygonStriped(rOutputDevice, aPolygon);
        }

        void OverlayTriangleStriped::createBaseRange(OutputDevice& /*rOutputDevice*/)
        {
            // reset range and expand it
            maBaseRange.reset();
            maBaseRange.expand(getBasePosition());
            maBaseRange.expand(getSecondPosition());
            maBaseRange.expand(getThirdPosition());
        }

        OverlayTriangleStriped::OverlayTriangleStriped(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            const basegfx::B2DPoint& rThirdPos)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            maSecondPosition(rSecondPos),
            maThirdPosition(rThirdPos)
        {
        }

        OverlayTriangleStriped::~OverlayTriangleStriped()
        {
        }

        void OverlayTriangleStriped::setSecondPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maSecondPosition)
            {
                // remember new value
                maSecondPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayTriangleStriped::setThirdPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maThirdPosition)
            {
                // remember new value
                maThirdPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }

        sal_Bool OverlayTriangleStriped::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                // test with all lines and epsilon-range
                if(basegfx::tools::isInEpsilonRange(getBasePosition(), getThirdPosition(), rPos, fTol))
                {
                    return sal_True;
                }
                else if(basegfx::tools::isInEpsilonRange(getSecondPosition(), getBasePosition(), rPos, fTol))
                {
                    return sal_True;
                }
                else if(basegfx::tools::isInEpsilonRange(getThirdPosition(), getSecondPosition(), rPos, fTol))
                {
                    return sal_True;
                }

                // test if inside triangle
                basegfx::B2DPolygon aTestPoly;
                aTestPoly.append(getBasePosition());
                aTestPoly.append(getSecondPosition());
                aTestPoly.append(getThirdPosition());
                aTestPoly.setClosed(true);

                return basegfx::tools::isInside(aTestPoly, rPos);
            }

            return sal_False;
        }

        void OverlayTriangleStriped::transform(const basegfx::B2DHomMatrix& rMatrix)
        {
            if(!rMatrix.isIdentity())
            {
                // transform base position
                OverlayObjectWithBasePosition::transform(rMatrix);

                // transform maSecondPosition
                const basegfx::B2DPoint aNewSecondPosition = rMatrix * getSecondPosition();
                setSecondPosition(aNewSecondPosition);

                // transform maThirdPosition
                const basegfx::B2DPoint aNewThirdPosition = rMatrix * getThirdPosition();
                setThirdPosition(aNewThirdPosition);
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayTriangle::drawGeometry(OutputDevice& rOutputDevice)
        {
            Polygon aPolygon(4);
            Point aPosition(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            aPolygon[0] = aPolygon[3] = aPosition;
            aPosition.X() = FRound(getSecondPosition().getX());
            aPosition.Y() = FRound(getSecondPosition().getY());
            aPolygon[1] = aPosition;
            aPosition.X() = FRound(getThirdPosition().getX());
            aPosition.Y() = FRound(getThirdPosition().getY());
            aPolygon[2] = aPosition;

            rOutputDevice.SetLineColor();
            rOutputDevice.SetFillColor(getBaseColor());

            rOutputDevice.DrawPolygon(aPolygon);
        }

        OverlayTriangle::OverlayTriangle(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            const basegfx::B2DPoint& rThirdPos,
            Color aTriangleColor)
        :   OverlayTriangleStriped(rBasePos, rSecondPos, rThirdPos)
        {
            // set base color here, OverlayCrosshairStriped constructor has set
            // it to it's own default.
            maBaseColor = aTriangleColor;
        }

        OverlayTriangle::~OverlayTriangle()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
