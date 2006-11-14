/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaybitmapex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:33:05 $
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

#ifndef _SDR_OVERLAY_OVERLAYBITMAPEX_HXX
#include <svx/sdr/overlay/overlaybitmapex.hxx>
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
        void OverlayBitmapEx::drawGeometry(OutputDevice& rOutputDevice)
        {
            // calculate position in pixel
            Point aPositionPixel(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            aPositionPixel = rOutputDevice.LogicToPixel(aPositionPixel);
            aPositionPixel.X() -= (sal_Int32)mnCenterX;
            aPositionPixel.Y() -= (sal_Int32)mnCenterY;

            // remember MapMode and switch to pixels
            const sal_Bool bMapModeWasEnabled(rOutputDevice.IsMapModeEnabled());
            rOutputDevice.EnableMapMode(sal_False);

            // draw the bitmap
            rOutputDevice.DrawBitmapEx(aPositionPixel, maBitmapEx);

            // restore MapMode
            rOutputDevice.EnableMapMode(bMapModeWasEnabled);
        }

        void OverlayBitmapEx::createBaseRange(OutputDevice& rOutputDevice)
        {
            // calculate bitmap rectangle in pixel
            Point aPositionPixel(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            aPositionPixel = rOutputDevice.LogicToPixel(aPositionPixel);
            aPositionPixel.X() -= (sal_Int32)mnCenterX;
            aPositionPixel.Y() -= (sal_Int32)mnCenterY;
            const Rectangle aRectanglePixel(aPositionPixel, maBitmapEx.GetSizePixel());

            // go back to logical coordinates
            const Rectangle aRectangleLogic(rOutputDevice.PixelToLogic(aRectanglePixel));

            // reset range and expand it
            maBaseRange.reset();
            maBaseRange.expand(basegfx::B2DPoint(aRectangleLogic.Left(), aRectangleLogic.Top()));
            maBaseRange.expand(basegfx::B2DPoint(aRectangleLogic.Right(), aRectangleLogic.Bottom()));
        }

        OverlayBitmapEx::OverlayBitmapEx(
            const basegfx::B2DPoint& rBasePos,
            const BitmapEx& rBitmapEx,
            sal_uInt16 nCenX, sal_uInt16 nCenY)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_WHITE)),
            maBitmapEx(rBitmapEx),
            mnCenterX(nCenX),
            mnCenterY(nCenY)
        {
        }

        OverlayBitmapEx::~OverlayBitmapEx()
        {
        }

        void OverlayBitmapEx::setBitmapEx(const BitmapEx& rNew)
        {
            if(rNew != maBitmapEx)
            {
                // remember new Bitmap
                maBitmapEx = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayBitmapEx::setCenterXY(sal_uInt16 nNewX, sal_uInt16 nNewY)
        {
            if(nNewX != mnCenterX || nNewY != mnCenterY)
            {
                // remember new values
                if(nNewX != mnCenterX)
                {
                    mnCenterX = nNewX;
                }

                if(nNewY != mnCenterY)
                {
                    mnCenterY = nNewY;
                }

                // register change (after change)
                objectChange();
            }
        }

        void OverlayBitmapEx::zoomHasChanged()
        {
            // reset validity of range in logical coor to force recalculation
            mbIsChanged = sal_True;
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
