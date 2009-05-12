/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaybitmap.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SDR_OVERLAY_OVERLAYBITMAP_HXX
#define _SDR_OVERLAY_OVERLAYBITMAP_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/bitmap.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayBitmap : public OverlayObjectWithBasePosition
        {
        protected:
            // the Bitmap itself
            Bitmap                                  maBitmap;

            // position of the basePosition inside the Bitmap, in pixels
            sal_uInt16                              mnCenterX;
            sal_uInt16                              mnCenterY;

            // bitfield
            // shall the BaseColor be used to create transparence?
            unsigned                                mbUseTransparenceColor : 1;

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

        public:
            OverlayBitmap(
                const basegfx::B2DPoint& rBasePos,
                const Bitmap& rBitmap,
                sal_uInt16 nCenX = 0, sal_uInt16 nCenY = 0,
                sal_Bool bUseTransCol = sal_False,
                Color aTransColor = Color(COL_WHITE));
            virtual ~OverlayBitmap();

            const Bitmap& getBitmap() const { return maBitmap; }
            void setBitmap(const Bitmap& rNew);

            sal_Bool isTransparenceUsed() const { return mbUseTransparenceColor; }
            void setTransparenceUsed(sal_Bool bNew);

            sal_uInt16 getCenterX() const { return mnCenterX; }
            sal_uInt16 getCenterY() const { return mnCenterY; }
            void setCenterXY(sal_uInt16 nNewX, sal_uInt16 nNewY);

            // Zoom has changed. If the objects logical size
            // depends on the MapMode of the used OutputDevice, use this call
            // to invalidate the range in logical coordinates.
            virtual void zoomHasChanged();
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYBITMAP_HXX

// eof
