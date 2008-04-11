/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlayhelpline.cxx,v $
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
#include <svx/sdr/overlay/overlayhelpline.hxx>
#include <tools/gen.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/vector/b2dvector.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayHelplineStriped::drawGeometry(OutputDevice& rOutputDevice)
        {
            // prepare OutputDevice
            const Point aEmptyPoint;
            const Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
            const Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));

            switch(meKind)
            {
                case SDRHELPLINE_VERTICAL :
                {
                    const basegfx::B2DPoint aStart(getBasePosition().getX(), aVisibleLogic.Top());
                    const basegfx::B2DPoint aEnd(getBasePosition().getX(), aVisibleLogic.Bottom());
                    ImpDrawLineStriped(rOutputDevice, aStart, aEnd);
                    break;
                }

                case SDRHELPLINE_HORIZONTAL :
                {
                    const basegfx::B2DPoint aStart(aVisibleLogic.Left(), getBasePosition().getY());
                    const basegfx::B2DPoint aEnd(aVisibleLogic.Right(), getBasePosition().getY());
                    ImpDrawLineStriped(rOutputDevice, aStart, aEnd);
                    break;
                }

                case SDRHELPLINE_POINT :
                {
                    const Size aPixelSize(SDRHELPLINE_POINT_PIXELSIZE, SDRHELPLINE_POINT_PIXELSIZE);
                    const Size aLogicSize(rOutputDevice.PixelToLogic(aPixelSize));

                    const basegfx::B2DPoint aStartA(getBasePosition().getX(), getBasePosition().getY() - aLogicSize.Height());
                    const basegfx::B2DPoint aEndA(getBasePosition().getX(), getBasePosition().getY() + aLogicSize.Height());
                    ImpDrawLineStriped(rOutputDevice, aStartA, aEndA);

                    const basegfx::B2DPoint aStartB(getBasePosition().getX() - aLogicSize.Width(), getBasePosition().getY());
                    const basegfx::B2DPoint aEndB(getBasePosition().getX() + aLogicSize.Width(), getBasePosition().getY());
                    ImpDrawLineStriped(rOutputDevice, aStartB, aEndB);

                    break;
                }
            }
        }

        void OverlayHelplineStriped::createBaseRange(OutputDevice& rOutputDevice)
        {
            // reset range and expand it
            maBaseRange.reset();

            if(SDRHELPLINE_POINT == meKind)
            {
                const Size aPixelSize(SDRHELPLINE_POINT_PIXELSIZE, SDRHELPLINE_POINT_PIXELSIZE);
                const Size aLogicSize(rOutputDevice.PixelToLogic(aPixelSize));

                maBaseRange.expand(basegfx::B2DPoint(getBasePosition().getX() - aLogicSize.Width(), getBasePosition().getY() - aLogicSize.Height()));
                maBaseRange.expand(basegfx::B2DPoint(getBasePosition().getX() + aLogicSize.Width(), getBasePosition().getY() + aLogicSize.Height()));
            }
            else
            {
                const Point aEmptyPoint;
                const Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
                const Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));

                if(SDRHELPLINE_HORIZONTAL == meKind)
                {
                    maBaseRange.expand(basegfx::B2DPoint(aVisibleLogic.Left(), getBasePosition().getY()));
                    maBaseRange.expand(basegfx::B2DPoint(aVisibleLogic.Right(), getBasePosition().getY()));
                }
                else if(SDRHELPLINE_VERTICAL == meKind)
                {
                    maBaseRange.expand(basegfx::B2DPoint(getBasePosition().getX(), aVisibleLogic.Top()));
                    maBaseRange.expand(basegfx::B2DPoint(getBasePosition().getX(), aVisibleLogic.Bottom()));
                }
            }
        }

        OverlayHelplineStriped::OverlayHelplineStriped(
            const basegfx::B2DPoint& rBasePos,
            SdrHelpLineKind eNewKind)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            meKind(eNewKind)
        {
        }

        OverlayHelplineStriped::~OverlayHelplineStriped()
        {
        }

        sal_Bool OverlayHelplineStriped::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                if(SDRHELPLINE_POINT == meKind)
                {
                    // use distance to BasePosition
                    const basegfx::B2DVector aVector(rPos - getBasePosition());

                    return (aVector.getLength() < fTol);
                }
                else
                {
                    if(SDRHELPLINE_HORIZONTAL == meKind)
                    {
                        // test vertical
                        if(rPos.getY() >= (getBasePosition().getY() - fTol)
                            && rPos.getY() <= (getBasePosition().getY() + fTol))
                        {
                            return sal_True;
                        }
                    }
                    else if(SDRHELPLINE_VERTICAL == meKind)
                    {
                        // test horizontal
                        if(rPos.getX() >= (getBasePosition().getX() - fTol)
                            && rPos.getX() <= (getBasePosition().getX() + fTol))
                        {
                            return sal_True;
                        }
                    }
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
        void OverlayHelpline::drawGeometry(OutputDevice& rOutputDevice)
        {
            Point aBasePos(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));

            rOutputDevice.SetLineColor(getBaseColor());
            rOutputDevice.SetFillColor();

            if(SDRHELPLINE_POINT == meKind)
            {
                Size aPixelSize(SDRHELPLINE_POINT_PIXELSIZE, SDRHELPLINE_POINT_PIXELSIZE);
                Size aLogicSize(rOutputDevice.PixelToLogic(aPixelSize));

                rOutputDevice.DrawLine(
                    Point(aBasePos.X() - aLogicSize.Width(), aBasePos.Y()),
                    Point(aBasePos.X() + aLogicSize.Width(), aBasePos.Y()));
                rOutputDevice.DrawLine(
                    Point(aBasePos.X(), aBasePos.Y() - aLogicSize.Height()),
                    Point(aBasePos.X(), aBasePos.Y() + aLogicSize.Height()));
            }
            else
            {
                Point aEmptyPoint;
                Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
                Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));

                if(SDRHELPLINE_HORIZONTAL == meKind)
                {
                    rOutputDevice.DrawLine(Point(aVisibleLogic.Left(), aBasePos.Y()), Point(aVisibleLogic.Right(), aBasePos.Y()));
                }
                else if(SDRHELPLINE_VERTICAL == meKind)
                {
                    rOutputDevice.DrawLine(Point(aBasePos.X(), aVisibleLogic.Top()), Point(aBasePos.X(), aVisibleLogic.Bottom()));
                }
            }
        }

        OverlayHelpline::OverlayHelpline(
            const basegfx::B2DPoint& rBasePos,
            Color aLineColor,
            SdrHelpLineKind eNewKind)
        :   OverlayHelplineStriped(rBasePos, eNewKind)
        {
            // set base color here, OverlayCrosshairStriped constructor has set
            // it to it's own default.
            maBaseColor = aLineColor;
        }

        OverlayHelpline::~OverlayHelpline()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
