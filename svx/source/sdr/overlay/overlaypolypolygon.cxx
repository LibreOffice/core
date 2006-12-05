/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaypolypolygon.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:13:54 $
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

#ifndef _SDR_OVERLAY_OVERLAYPOLYPOLYGON_HXX
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
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

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

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

        void OverlayPolyPolygonStriped::preparePolygonData()
        {
            if(maPolyPolygon.areControlPointsUsed())
            {
                maPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(maPolyPolygon);
            }
        }

        OverlayPolyPolygonStriped::OverlayPolyPolygonStriped(
            const basegfx::B2DPolyPolygon& rPolyPolygon)
        :   OverlayObject(Color(COL_BLACK)),
            maPolyPolygon(rPolyPolygon)
        {
            // evtl. do preparations with given polygon data
            preparePolygonData();
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

                // evtl. do preparations with given polygon data
                preparePolygonData();

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

        void OverlayPolyPolygon::preparePolygonData()
        {
            // do not call parent here, keep curved data
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
