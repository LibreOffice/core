/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaypolypolygon.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:08:32 $
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

#ifndef _SDR_OVERLAY_OVERLAYPOLYPOLYGON_HXX
#define _SDR_OVERLAY_OVERLAYPOLYPOLYGON_HXX

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayPolyPolygonStriped : public OverlayObject
        {
        protected:
            // geometry
            basegfx::B2DPolyPolygon             maPolyPolygon;

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

            // eventually remove curves from polygon data (maPolyPolygon)
            virtual void preparePolygonData();

        public:
            OverlayPolyPolygonStriped(const basegfx::B2DPolyPolygon& rPolyPolygon);
            virtual ~OverlayPolyPolygonStriped();

            // change geometry
            basegfx::B2DPolyPolygon getPolyPolygon() const { return maPolyPolygon; }
            void setPolyPolygon(const basegfx::B2DPolyPolygon& rNew);

            // Hittest with logical coordinates
            virtual sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol = 0.0) const;

            // transform object coordinates. Needs to transform maSecondPosition
            // and maThirdPosition.
            virtual void transform(const basegfx::B2DHomMatrix& rMatrix);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayPolyPolygon : public OverlayPolyPolygonStriped
        {
        protected:
            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // eventually remove curves from polygon data (maPolyPolygon)
            virtual void preparePolygonData();

        public:
            OverlayPolyPolygon(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                Color aPolygonColor = Color(COL_BLACK));
            virtual ~OverlayPolyPolygon();
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYPOLYPOLYGON_HXX

// eof
