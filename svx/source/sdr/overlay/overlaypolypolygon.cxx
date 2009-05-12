/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaypolypolygon.cxx,v $
 * $Revision: 1.5 $
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
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayPolyPolygonStriped::drawGeometry(OutputDevice& rOutputDevice)
        {
            if(maPolyPolygon.count())
            {
                for(sal_uInt32 a(0L); a < maPolyPolygon.count(); a++)
                {
                    ImpDrawPolygonStriped(rOutputDevice, maPolyPolygon.getB2DPolygon(a));
                }
            }
        }

        void OverlayPolyPolygonStriped::createBaseRange(OutputDevice& /*rOutputDevice*/)
        {
            // use tooling to get range from PolyPolygon
            maBaseRange = basegfx::tools::getRange(maPolyPolygon);
        }

        OverlayPolyPolygonStriped::OverlayPolyPolygonStriped(
            const basegfx::B2DPolyPolygon& rPolyPolygon)
        :   OverlayObject(Color(COL_BLACK)),
            maPolyPolygon(rPolyPolygon)
        {
        }

        OverlayPolyPolygonStriped::~OverlayPolyPolygonStriped()
        {
        }

        void OverlayPolyPolygonStriped::setPolyPolygon(const basegfx::B2DPolyPolygon& rNew)
        {
            if(rNew != maPolyPolygon)
            {
                // remember new value
                maPolyPolygon = rNew;

                // register change (after change)
                objectChange();
            }
        }

        sal_Bool OverlayPolyPolygonStriped::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                return basegfx::tools::isInEpsilonRange(maPolyPolygon, rPos, fTol);
            }

            return sal_False;
        }

        void OverlayPolyPolygonStriped::transform(const basegfx::B2DHomMatrix& rMatrix)
        {
            if(!rMatrix.isIdentity())
            {
                // transform maPolyPolygon
                maPolyPolygon.transform(rMatrix);

                // register change (after change)
                objectChange();
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayPolyPolygon::drawGeometry(OutputDevice& rOutputDevice)
        {
            if(maPolyPolygon.count())
            {
                rOutputDevice.SetLineColor(getBaseColor());
                rOutputDevice.SetFillColor();

                // iterate self, else the single polygons will be closed when
                // using DrawPolyPolygon
                for(sal_uInt32 a(0L); a < maPolyPolygon.count(); a++)
                {
                    const Polygon aPaintPoly(maPolyPolygon.getB2DPolygon(a));
                    rOutputDevice.DrawPolyLine(aPaintPoly);
                }
            }
        }

        OverlayPolyPolygon::OverlayPolyPolygon(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            Color aPolygonColor)
        :   OverlayPolyPolygonStriped(rPolyPolygon)
        {
            // set base color here, OverlayCrosshairStriped constructor has set
            // it to it's own default.
            maBaseColor = aPolygonColor;
        }

        OverlayPolyPolygon::~OverlayPolyPolygon()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
