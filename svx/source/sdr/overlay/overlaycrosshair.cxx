/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaycrosshair.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:13:08 $
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

#ifndef _SDR_OVERLAY_OVERLAYCROSSHAIR_HXX
#include <svx/sdr/overlay/overlaycrosshair.hxx>
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

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayCrosshairStriped::drawGeometry(OutputDevice& rOutputDevice)
        {
            const Point aEmptyPoint;
            const Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
            const Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));

            const basegfx::B2DPoint aStartA(aVisibleLogic.Left(), getBasePosition().getY());
            const basegfx::B2DPoint aEndA(aVisibleLogic.Right(), getBasePosition().getY());
            ImpDrawLineStriped(rOutputDevice, aStartA, aEndA);

            const basegfx::B2DPoint aStartB(getBasePosition().getX(), aVisibleLogic.Top());
            const basegfx::B2DPoint aEndB(getBasePosition().getX(), aVisibleLogic.Bottom());
            ImpDrawLineStriped(rOutputDevice, aStartB, aEndB);
        }

        void OverlayCrosshairStriped::createBaseRange(OutputDevice& rOutputDevice)
        {
            // reset range and expand it
            const Point aEmptyPoint;
            const Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
            const Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));

            maBaseRange.reset();
            maBaseRange.expand(basegfx::B2DPoint(aVisibleLogic.Left(), aVisibleLogic.Top()));
            maBaseRange.expand(basegfx::B2DPoint(aVisibleLogic.Right(), aVisibleLogic.Bottom()));
        }

        OverlayCrosshairStriped::OverlayCrosshairStriped(const basegfx::B2DPoint& rBasePos)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK))
        {
        }

        OverlayCrosshairStriped::~OverlayCrosshairStriped()
        {
        }

        sal_Bool OverlayCrosshairStriped::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                // test vertical
                if(rPos.getY() >= (getBasePosition().getY() - fTol)
                    && rPos.getY() <= (getBasePosition().getY() + fTol))
                {
                    return sal_True;
                }

                // test horizontal
                if(rPos.getX() >= (getBasePosition().getX() - fTol)
                    && rPos.getX() <= (getBasePosition().getX() + fTol))
                {
                    return sal_True;
                }
            }

            return sal_False;
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayCrosshair::drawGeometry(OutputDevice& rOutputDevice)
        {
            const Point aBasePos(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            const Point aEmptyPoint;
            const Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
            const Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));

            rOutputDevice.SetLineColor(getBaseColor());
            rOutputDevice.SetFillColor();

            rOutputDevice.DrawLine(Point(aVisibleLogic.Left(), aBasePos.Y()), Point(aVisibleLogic.Right(), aBasePos.Y()));
            rOutputDevice.DrawLine(Point(aBasePos.X(), aVisibleLogic.Top()), Point(aBasePos.X(), aVisibleLogic.Bottom()));
        }

        OverlayCrosshair::OverlayCrosshair(
            const basegfx::B2DPoint& rBasePos,
            Color aLineColor)
        :   OverlayCrosshairStriped(rBasePos)
        {
            // set base color here, OverlayCrosshairStriped constructor has set
            // it to it's own default.
            maBaseColor = aLineColor;
        }

        OverlayCrosshair::~OverlayCrosshair()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
