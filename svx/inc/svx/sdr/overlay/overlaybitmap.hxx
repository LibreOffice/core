/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaybitmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:06:40 $
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

#ifndef _SDR_OVERLAY_OVERLAYBITMAP_HXX
#define _SDR_OVERLAY_OVERLAYBITMAP_HXX

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

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
