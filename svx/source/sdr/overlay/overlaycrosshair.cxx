/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaycrosshair.cxx,v $
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
#include <svx/sdr/overlay/overlaycrosshair.hxx>
#include <tools/gen.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>

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
