/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayanimatedbitmapex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:32:46 $
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

#ifndef _SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX
#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayAnimatedBitmapEx::drawGeometry(OutputDevice& rOutputDevice)
        {
            // calculate position in pixel
            Point aPositionPixel(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            aPositionPixel = rOutputDevice.LogicToPixel(aPositionPixel);
            aPositionPixel.X() -= (mbOverlayState) ? (sal_Int32)mnCenterX1 : (sal_Int32)mnCenterX2;
            aPositionPixel.Y() -= (mbOverlayState) ? (sal_Int32)mnCenterY1 : (sal_Int32)mnCenterY2;

            // remember MapMode and switch to pixels
            const sal_Bool bMapModeWasEnabled(rOutputDevice.IsMapModeEnabled());
            rOutputDevice.EnableMapMode(sal_False);

            // draw the bitmap
            rOutputDevice.DrawBitmapEx(aPositionPixel, (mbOverlayState) ? maBitmapEx1 : maBitmapEx2);

            // restore MapMode
            rOutputDevice.EnableMapMode(bMapModeWasEnabled);
        }

        void OverlayAnimatedBitmapEx::createBaseRange(OutputDevice& rOutputDevice)
        {
            // calculate bitmap rectangle in pixel
            Point aPositionPixel(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            aPositionPixel = rOutputDevice.LogicToPixel(aPositionPixel);
            aPositionPixel.X() -= (mbOverlayState) ? (sal_Int32)mnCenterX1 : (sal_Int32)mnCenterX2;
            aPositionPixel.Y() -= (mbOverlayState) ? (sal_Int32)mnCenterY1 : (sal_Int32)mnCenterY2;
            const Rectangle aRectanglePixel(aPositionPixel,
                (mbOverlayState) ? maBitmapEx1.GetSizePixel() : maBitmapEx2.GetSizePixel());

            // go back to logical coordinates
            const Rectangle aRectangleLogic(rOutputDevice.PixelToLogic(aRectanglePixel));

            // reset range and expand it
            maBaseRange.reset();
            maBaseRange.expand(basegfx::B2DPoint(aRectangleLogic.Left(), aRectangleLogic.Top()));
            maBaseRange.expand(basegfx::B2DPoint(aRectangleLogic.Right(), aRectangleLogic.Bottom()));
        }

        OverlayAnimatedBitmapEx::OverlayAnimatedBitmapEx(
            const basegfx::B2DPoint& rBasePos,
            const BitmapEx& rBitmapEx1, const BitmapEx& rBitmapEx2,
            sal_uInt16 nCenX1, sal_uInt16 nCenY1,
            sal_uInt16 nCenX2, sal_uInt16 nCenY2)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_WHITE)),
            maBitmapEx1(rBitmapEx1),
            maBitmapEx2(rBitmapEx2),
            mnCenterX1(nCenX1), mnCenterY1(nCenY1),
            mnCenterX2(nCenX2), mnCenterY2(nCenY2),
            mbOverlayState(sal_False)
        {
            // set AllowsAnimation flag to mark this object as animation capable
            mbAllowsAnimation = sal_True;
        }

        OverlayAnimatedBitmapEx::~OverlayAnimatedBitmapEx()
        {
        }

        void OverlayAnimatedBitmapEx::setBitmapEx1(const BitmapEx& rNew)
        {
            if(rNew != maBitmapEx1)
            {
                // remember new Bitmap
                maBitmapEx1 = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayAnimatedBitmapEx::setBitmapEx2(const BitmapEx& rNew)
        {
            if(rNew != maBitmapEx2)
            {
                // remember new Bitmap
                maBitmapEx2 = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayAnimatedBitmapEx::setCenterXY1(sal_uInt16 nNewX, sal_uInt16 nNewY)
        {
            if(nNewX != mnCenterX1 || nNewY != mnCenterY1)
            {
                // remember new values
                if(nNewX != mnCenterX1)
                {
                    mnCenterX1 = nNewX;
                }

                if(nNewY != mnCenterY1)
                {
                    mnCenterY1 = nNewY;
                }

                // register change (after change)
                objectChange();
            }
        }

        void OverlayAnimatedBitmapEx::setCenterXY2(sal_uInt16 nNewX, sal_uInt16 nNewY)
        {
            if(nNewX != mnCenterX2 || nNewY != mnCenterY2)
            {
                // remember new values
                if(nNewX != mnCenterX2)
                {
                    mnCenterX2 = nNewX;
                }

                if(nNewY != mnCenterY2)
                {
                    mnCenterY2 = nNewY;
                }

                // register change (after change)
                objectChange();
            }
        }

        void OverlayAnimatedBitmapEx::Trigger(sal_uInt32 nTime)
        {
            if(getOverlayManager())
            {
                // produce event after nTime + x
                if(mbOverlayState)
                {
                    mbOverlayState = sal_False;
                    SetTime(nTime + 800); // 0.8 seconds
                }
                else
                {
                    mbOverlayState = sal_True;
                    SetTime(nTime + 300); // 0.3 seconds
                }

                // re-insert me as event
                getOverlayManager()->InsertEvent(this);

                // register change (after change)
                objectChange();
            }
        }

        void OverlayAnimatedBitmapEx::zoomHasChanged()
        {
            // reset validity of range in logical coor to force recalculation
            mbIsChanged = sal_True;
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
