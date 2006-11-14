/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayanimatedbitmapex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:06:30 $
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
#define _SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayAnimatedBitmapEx : public OverlayObjectWithBasePosition
        {
        protected:
            // the Bitmaps
            BitmapEx                                maBitmapEx1;
            BitmapEx                                maBitmapEx2;

            // position of the basePosition inside the Bitmaps, in pixels
            sal_uInt16                              mnCenterX1;
            sal_uInt16                              mnCenterY1;
            sal_uInt16                              mnCenterX2;
            sal_uInt16                              mnCenterY2;

            // bitfield
            // Flag to remember which state to draw. Inited with sal_False (0)
            unsigned                                mbOverlayState : 1;

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

        public:
            OverlayAnimatedBitmapEx(
                const basegfx::B2DPoint& rBasePos,
                const BitmapEx& rBitmapEx1, const BitmapEx& rBitmapEx2,
                sal_uInt16 nCenX1 = 0, sal_uInt16 nCenY1 = 0,
                sal_uInt16 nCenX2 = 0, sal_uInt16 nCenY2 = 0);
            virtual ~OverlayAnimatedBitmapEx();

            const BitmapEx& getBitmapEx1() const { return maBitmapEx1; }
            const BitmapEx& getBitmapEx2() const { return maBitmapEx2; }
            void setBitmapEx1(const BitmapEx& rNew);
            void setBitmapEx2(const BitmapEx& rNew);

            sal_uInt16 getCenterX1() const { return mnCenterX1; }
            sal_uInt16 getCenterY1() const { return mnCenterY1; }
            sal_uInt16 getCenterX2() const { return mnCenterX2; }
            sal_uInt16 getCenterY2() const { return mnCenterY2; }
            void setCenterXY1(sal_uInt16 nNewX, sal_uInt16 nNewY);
            void setCenterXY2(sal_uInt16 nNewX, sal_uInt16 nNewY);

            // execute event from base class ::sdr::animation::Event. Default
            // implementation does nothing and does not create a new event.
            virtual void Trigger(sal_uInt32 nTime);

            // Zoom has changed. If the objects logical size
            // depends on the MapMode of the used OutputDevice, use this call
            // to invalidate the range in logical coordinates.
            virtual void zoomHasChanged();
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX

// eof
