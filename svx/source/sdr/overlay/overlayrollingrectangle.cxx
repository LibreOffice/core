/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayrollingrectangle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:34:54 $
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

#ifndef _SDR_OVERLAY_OVERLAYROOLINGRECTANGLE_HXX
#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

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
