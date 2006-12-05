/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaybitmap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:11:47 $
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

#ifndef _SDR_OVERLAY_OVERLAYBITMAP_HXX
#include <svx/sdr/overlay/overlaybitmap.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayBitmap::drawGeometry(OutputDevice& rOutputDevice)
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
            if(mbUseTransparenceColor)
            {
                BitmapEx aBitmapEx(maBitmap, getBaseColor());
                rOutputDevice.DrawBitmapEx(aPositionPixel, aBitmapEx);
            }
            else
            {
                rOutputDevice.DrawBitmap(aPositionPixel, maBitmap);
            }

            // restore MapMode
            rOutputDevice.EnableMapMode(bMapModeWasEnabled);
        }

        void OverlayBitmap::createBaseRange(OutputDevice& rOutputDevice)
        {
            // calculate bitmap rectangle in pixel
            Point aPositionPixel(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            aPositionPixel = rOutputDevice.LogicToPixel(aPositionPixel);
            aPositionPixel.X() -= (sal_Int32)mnCenterX;
            aPositionPixel.Y() -= (sal_Int32)mnCenterY;
            const Rectangle aRectanglePixel(aPositionPixel, maBitmap.GetSizePixel());

            // go back to logical coordinates
            const Rectangle aRectangleLogic(rOutputDevice.PixelToLogic(aRectanglePixel));

            // reset range and expand it
            maBaseRange.reset();
            maBaseRange.expand(basegfx::B2DPoint(aRectangleLogic.Left(), aRectangleLogic.Top()));
            maBaseRange.expand(basegfx::B2DPoint(aRectangleLogic.Right(), aRectangleLogic.Bottom()));
        }

        OverlayBitmap::OverlayBitmap(
            const basegfx::B2DPoint& rBasePos,
            const Bitmap& rBitmap,
            sal_uInt16 nCenX, sal_uInt16 nCenY,
            sal_Bool bUseTransCol,
            Color aTransColor)
        :   OverlayObjectWithBasePosition(rBasePos, aTransColor),
            maBitmap(rBitmap),
            mnCenterX(nCenX),
            mnCenterY(nCenY),
            mbUseTransparenceColor(bUseTransCol)
        {
        }

        OverlayBitmap::~OverlayBitmap()
        {
        }

        void OverlayBitmap::setBitmap(const Bitmap& rNew)
        {
            if(rNew != maBitmap)
            {
                // remember new Bitmap
                maBitmap = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayBitmap::setTransparenceUsed(sal_Bool bNew)
        {
            if(bNew != mbUseTransparenceColor)
            {
                // remember new value
                mbUseTransparenceColor = bNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayBitmap::setCenterXY(sal_uInt16 nNewX, sal_uInt16 nNewY)
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

        void OverlayBitmap::zoomHasChanged()
        {
            // reset validity of range in logical coor to force recalculation
            mbIsChanged = sal_True;
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
