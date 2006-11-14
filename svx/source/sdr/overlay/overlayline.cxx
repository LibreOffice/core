/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayline.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:33:39 $
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

#ifndef _SDR_OVERLAY_OVERLAYLINE_HXX
#include <svx/sdr/overlay/overlayline.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayLineStriped::drawGeometry(OutputDevice& rOutputDevice)
        {
            ImpDrawLineStriped(rOutputDevice, getBasePosition(), getSecondPosition());
        }

        void OverlayLineStriped::createBaseRange(OutputDevice& /*rOutputDevice*/)
        {
            // reset range and expand it
            maBaseRange.reset();
            maBaseRange.expand(getBasePosition());
            maBaseRange.expand(getSecondPosition());
        }

        OverlayLineStriped::OverlayLineStriped(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            maSecondPosition(rSecondPos)
        {
        }

        OverlayLineStriped::~OverlayLineStriped()
        {
        }

        void OverlayLineStriped::setSecondPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maSecondPosition)
            {
                // remember new value
                maSecondPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }

        sal_Bool OverlayLineStriped::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                return basegfx::tools::isInEpsilonRange(getBasePosition(), getSecondPosition(), rPos, fTol);
            }

            return sal_False;
        }

        void OverlayLineStriped::transform(const basegfx::B2DHomMatrix& rMatrix)
        {
            if(!rMatrix.isIdentity())
            {
                // transform base position
                OverlayObjectWithBasePosition::transform(rMatrix);

                // transform maSecondPosition
                const basegfx::B2DPoint aNewSecondPosition = rMatrix * getSecondPosition();
                setSecondPosition(aNewSecondPosition);
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayLine::drawGeometry(OutputDevice& rOutputDevice)
        {
            const Point aStart(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            const Point aEnd(FRound(getSecondPosition().getX()), FRound(getSecondPosition().getY()));

            rOutputDevice.SetLineColor(getBaseColor());
            rOutputDevice.SetFillColor();

            rOutputDevice.DrawLine(aStart, aEnd);
        }

        OverlayLine::OverlayLine(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            Color aLineColor)
        :   OverlayLineStriped(rBasePos, rSecondPos)
        {
            // set base color here, OverlayCrosshairStriped constructor has set
            // it to it's own default.
            maBaseColor = aLineColor;
        }

        OverlayLine::~OverlayLine()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
