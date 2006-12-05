/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaytriangle.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:11:29 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_OVERLAY_OVERLAYTRIANGLE_HXX
#include <svx/sdr/overlay/overlaytriangle.hxx>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

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
