/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlayrollingrectangle.cxx,v $
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
#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
#include <tools/gen.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayRollingRectangleStriped::drawGeometry(OutputDevice& rOutputDevice)
        {
            const basegfx::B2DRange aRange(getBasePosition(), getSecondPosition());

            if(getShowBounds())
            {
                ImpDrawRangeStriped(rOutputDevice, aRange);
            }

            if(getExtendedLines())
            {
                const Point aEmptyPoint;
                const Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
                const Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));

                // Left lines
                ImpDrawLineStriped(rOutputDevice, aVisibleLogic.Left(), aRange.getMinY(), aRange.getMinX(), aRange.getMinY());
                ImpDrawLineStriped(rOutputDevice, aVisibleLogic.Left(), aRange.getMaxY(), aRange.getMinX(), aRange.getMaxY());

                // Right lines
                ImpDrawLineStriped(rOutputDevice, aRange.getMaxX(), aRange.getMinY(), aVisibleLogic.Right(), aRange.getMinY());
                ImpDrawLineStriped(rOutputDevice, aRange.getMaxX(), aRange.getMaxY(), aVisibleLogic.Right(), aRange.getMaxY());

                // Top lines
                ImpDrawLineStriped(rOutputDevice, aRange.getMinX(), aVisibleLogic.Top(), aRange.getMinX(), aRange.getMinY());
                ImpDrawLineStriped(rOutputDevice, aRange.getMaxX(), aVisibleLogic.Top(), aRange.getMaxX(), aRange.getMinY());

                // Bottom lines
                ImpDrawLineStriped(rOutputDevice, aRange.getMinX(), aRange.getMaxY(), aRange.getMinX(), aVisibleLogic.Bottom());
                ImpDrawLineStriped(rOutputDevice, aRange.getMaxX(), aRange.getMaxY(), aRange.getMaxX(), aVisibleLogic.Bottom());
            }
        }

        void OverlayRollingRectangleStriped::createBaseRange(OutputDevice& rOutputDevice)
        {
            // reset range and expand it
            maBaseRange.reset();

            if(getExtendedLines())
            {
                const Point aEmptyPoint;
                const Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
                const Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));
                maBaseRange.expand(basegfx::B2DPoint(aVisibleLogic.Left(), aVisibleLogic.Top()));
                maBaseRange.expand(basegfx::B2DPoint(aVisibleLogic.Right(), aVisibleLogic.Bottom()));
            }

            if(getShowBounds())
            {
                maBaseRange.expand(getBasePosition());
                maBaseRange.expand(getSecondPosition());
            }
        }

        OverlayRollingRectangleStriped::OverlayRollingRectangleStriped(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            sal_Bool bExtendedLines,
            sal_Bool bShowBounds)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            maSecondPosition(rSecondPos),
            mbExtendedLines(bExtendedLines),
            mbShowBounds(bShowBounds)
        {
        }

        OverlayRollingRectangleStriped::~OverlayRollingRectangleStriped()
        {
        }

        void OverlayRollingRectangleStriped::setSecondPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maSecondPosition)
            {
                // remember new value
                maSecondPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayRollingRectangleStriped::setExtendedLines(sal_Bool bNew)
        {
            if(bNew != mbExtendedLines)
            {
                // remember new value
                mbExtendedLines = bNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayRollingRectangleStriped::setShowBounds(sal_Bool bNew)
        {
            if(bNew != mbShowBounds)
            {
                // remember new value
                mbShowBounds = bNew;

                // register change (after change)
                objectChange();
            }
        }

        sal_Bool OverlayRollingRectangleStriped::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                if(getExtendedLines())
                {
                    const basegfx::B2DRange aRange(getBaseRange());
                    const basegfx::B2DPoint aMinimum(aRange.getMinimum());
                    const basegfx::B2DPoint aMaximum(aRange.getMaximum());

                    // test upper line horizontal
                    if(rPos.getY() > (aMinimum.getY() - fTol) && rPos.getY() < (aMinimum.getY() + fTol))
                    {
                        return sal_True;
                    }

                    // test lower line horizontal
                    if(rPos.getY() > (aMaximum.getY() - fTol) && rPos.getY() < (aMaximum.getY() + fTol))
                    {
                        return sal_True;
                    }

                    // test left line vertical
                    if(rPos.getX() > (aMinimum.getX() - fTol) && rPos.getX() < (aMinimum.getX() + fTol))
                    {
                        return sal_True;
                    }

                    // test rightline vertical
                    if(rPos.getX() > (aMaximum.getX() - fTol) && rPos.getX() < (aMaximum.getX() + fTol))
                    {
                        return sal_True;
                    }
                }

                if(getShowBounds())
                {
                    // test for inside grown range, outside shrinked one to test for border
                    // hit without interiour
                    basegfx::B2DRange aOuterRange(getBaseRange());
                    aOuterRange.grow(fTol);

                    if(aOuterRange.isInside(rPos))
                    {
                        basegfx::B2DRange aInnerRange(getBaseRange());
                        aInnerRange.grow(-fTol);

                        return !aInnerRange.isInside(rPos);
                    }
                }
            }

            return sal_False;
        }

        void OverlayRollingRectangleStriped::transform(const basegfx::B2DHomMatrix& rMatrix)
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
        void OverlayRollingRectangle::drawGeometry(OutputDevice& rOutputDevice)
        {
            const Point aStart(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            const Point aEnd(FRound(getSecondPosition().getX()), FRound(getSecondPosition().getY()));
            Rectangle aRectangle(aStart, aEnd);
            aRectangle.Justify();

            if(getShowBounds())
            {
                rOutputDevice.SetLineColor(getBaseColor());
                rOutputDevice.SetFillColor();

                rOutputDevice.DrawRect(aRectangle);
            }

            if(getExtendedLines())
            {
                const Point aEmptyPoint;
                const Rectangle aVisiblePixel(aEmptyPoint, rOutputDevice.GetOutputSizePixel());
                const Rectangle aVisibleLogic(rOutputDevice.PixelToLogic(aVisiblePixel));

                // Left lines
                rOutputDevice.DrawLine(Point(aVisibleLogic.Left(), aRectangle.Top()), aRectangle.TopLeft());
                rOutputDevice.DrawLine(Point(aVisibleLogic.Left(), aRectangle.Bottom()), aRectangle.BottomLeft());

                // Right lines
                rOutputDevice.DrawLine(aRectangle.TopRight(), Point(aVisibleLogic.Right(), aRectangle.Top()));
                rOutputDevice.DrawLine(aRectangle.BottomRight(), Point(aVisibleLogic.Right(), aRectangle.Bottom()));

                // Top lines
                rOutputDevice.DrawLine(Point(aRectangle.Left(), aVisibleLogic.Top()), aRectangle.TopLeft());
                rOutputDevice.DrawLine(Point(aRectangle.Right(), aVisibleLogic.Top()), aRectangle.TopRight());

                // Bottom lines
                rOutputDevice.DrawLine(aRectangle.BottomLeft(), Point(aRectangle.Left(), aVisibleLogic.Bottom()));
                rOutputDevice.DrawLine(aRectangle.BottomRight(), Point(aRectangle.Right(), aVisibleLogic.Bottom()));
            }
        }

        OverlayRollingRectangle::OverlayRollingRectangle(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            Color aLineColor,
            sal_Bool bExtendedLines,
            sal_Bool bShowBounds)
        :   OverlayRollingRectangleStriped(rBasePos, rSecondPos, bExtendedLines, bShowBounds)
        {
            // set base color here, OverlayCrosshairStriped constructor has set
            // it to it's own default.
            maBaseColor = aLineColor;
        }

        OverlayRollingRectangle::~OverlayRollingRectangle()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
