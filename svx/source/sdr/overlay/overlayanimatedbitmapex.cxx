/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayanimatedbitmapex.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 10:54:05 $
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

// #i77674#
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        // #i53216# check blink time value range
        void OverlayAnimatedBitmapEx::impCheckBlinkTimeValueRange()
        {
            if(mnBlinkTime < 25)
            {
                mnBlinkTime = 25;
            }
            else if(mnBlinkTime > 10000)
            {
                mnBlinkTime = 10000;
            }
        }

        void OverlayAnimatedBitmapEx::drawGeometry(OutputDevice& rOutputDevice)
        {
            // #i77674# calculate discrete top-left
            basegfx::B2DPoint aDiscreteTopLeft(rOutputDevice.GetViewTransformation() * getBasePosition());
            aDiscreteTopLeft -= (mbOverlayState)
                ? basegfx::B2DPoint((double)mnCenterX1, (double)mnCenterY1)
                : basegfx::B2DPoint((double)mnCenterX2, (double)mnCenterY2);

            // remember MapMode and switch to pixels
            const bool bMapModeWasEnabled(rOutputDevice.IsMapModeEnabled());
            rOutputDevice.EnableMapMode(false);

            // draw the bitmap
            const Point aPixelTopLeft((sal_Int32)floor(aDiscreteTopLeft.getX()), (sal_Int32)floor(aDiscreteTopLeft.getY()));
            rOutputDevice.DrawBitmapEx(aPixelTopLeft, (mbOverlayState) ? maBitmapEx1 : maBitmapEx2);

            // restore MapMode
            rOutputDevice.EnableMapMode(bMapModeWasEnabled);
        }

        void OverlayAnimatedBitmapEx::createBaseRange(OutputDevice& rOutputDevice)
        {
            // #i77674# calculate discrete top-left
            basegfx::B2DPoint aDiscreteTopLeft(rOutputDevice.GetViewTransformation() * getBasePosition());
            aDiscreteTopLeft -= (mbOverlayState)
                ? basegfx::B2DPoint((double)mnCenterX1, (double)mnCenterY1)
                : basegfx::B2DPoint((double)mnCenterX2, (double)mnCenterY2);

            // calculate discrete range
            const Size aBitmapPixelSize((mbOverlayState) ? maBitmapEx1.GetSizePixel() : maBitmapEx2.GetSizePixel());
            const basegfx::B2DRange aDiscreteRange(
                aDiscreteTopLeft.getX(), aDiscreteTopLeft.getY(),
                aDiscreteTopLeft.getX() + (double)aBitmapPixelSize.getWidth(), aDiscreteTopLeft.getY() + (double)aBitmapPixelSize.getHeight());

            // set and go back to logic range
            maBaseRange = aDiscreteRange;
            maBaseRange.transform(rOutputDevice.GetInverseViewTransformation());
        }

        OverlayAnimatedBitmapEx::OverlayAnimatedBitmapEx(
            const basegfx::B2DPoint& rBasePos,
            const BitmapEx& rBitmapEx1,
            const BitmapEx& rBitmapEx2,
            sal_uInt32 nBlinkTime,
            sal_uInt16 nCenX1,
            sal_uInt16 nCenY1,
            sal_uInt16 nCenX2,
            sal_uInt16 nCenY2)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_WHITE)),
            maBitmapEx1(rBitmapEx1),
            maBitmapEx2(rBitmapEx2),
            mnCenterX1(nCenX1), mnCenterY1(nCenY1),
            mnCenterX2(nCenX2), mnCenterY2(nCenY2),
            mnBlinkTime(nBlinkTime),
            mbOverlayState(sal_False)
        {
            // set AllowsAnimation flag to mark this object as animation capable
            mbAllowsAnimation = sal_True;

            // #i53216# check blink time value range
            impCheckBlinkTimeValueRange();
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

        void OverlayAnimatedBitmapEx::setBlinkTime(sal_uInt32 nNew)
        {
            if(mnBlinkTime != nNew)
            {
                // remember new value
                mnBlinkTime = nNew;

                // #i53216# check blink time value range
                impCheckBlinkTimeValueRange();

                // register change (after change)
                objectChange();
            }
        }

        void OverlayAnimatedBitmapEx::Trigger(sal_uInt32 nTime)
        {
            if(getOverlayManager())
            {
                // #i53216# produce event after nTime + x
                SetTime(nTime + mnBlinkTime);

                // switch state
                if(mbOverlayState)
                {
                    mbOverlayState = sal_False;
                }
                else
                {
                    mbOverlayState = sal_True;
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
