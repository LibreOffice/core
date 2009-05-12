/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaybitmapex.cxx,v $
 * $Revision: 1.5 $
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
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>

// #i77674#
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayBitmapEx::drawGeometry(OutputDevice& rOutputDevice)
        {
            // #i77674# calculate discrete top-left
            basegfx::B2DPoint aDiscreteTopLeft(rOutputDevice.GetViewTransformation() * getBasePosition());
            aDiscreteTopLeft -= basegfx::B2DPoint((double)mnCenterX, (double)mnCenterY);

            // remember MapMode and switch to pixels
            const bool bMapModeWasEnabled(rOutputDevice.IsMapModeEnabled());
            rOutputDevice.EnableMapMode(false);

            // draw the bitmap
            const Point aPixelTopLeft((sal_Int32)floor(aDiscreteTopLeft.getX()), (sal_Int32)floor(aDiscreteTopLeft.getY()));
            rOutputDevice.DrawBitmapEx(aPixelTopLeft, maBitmapEx);

            // restore MapMode
            rOutputDevice.EnableMapMode(bMapModeWasEnabled);
        }

        void OverlayBitmapEx::createBaseRange(OutputDevice& rOutputDevice)
        {
            // #i77674# calculate discrete top-left
            basegfx::B2DPoint aDiscreteTopLeft(rOutputDevice.GetViewTransformation() * getBasePosition());
            aDiscreteTopLeft -= basegfx::B2DPoint((double)mnCenterX, (double)mnCenterY);

            // calculate discrete range
            const Size aBitmapPixelSize(maBitmapEx.GetSizePixel());
            const basegfx::B2DRange aDiscreteRange(
                aDiscreteTopLeft.getX(), aDiscreteTopLeft.getY(),
                aDiscreteTopLeft.getX() + (double)aBitmapPixelSize.getWidth(), aDiscreteTopLeft.getY() + (double)aBitmapPixelSize.getHeight());

            // set and go back to logic range
            maBaseRange = aDiscreteRange;
            maBaseRange.transform(rOutputDevice.GetInverseViewTransformation());
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
