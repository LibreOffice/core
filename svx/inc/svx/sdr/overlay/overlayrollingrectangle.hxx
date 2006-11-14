/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayrollingrectangle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:08:41 $
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
#define _SDR_OVERLAY_OVERLAYROOLINGRECTANGLE_HXX

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayRollingRectangleStriped : public OverlayObjectWithBasePosition
        {
        protected:
            // second position in pixel
            basegfx::B2DPoint                       maSecondPosition;

            // bitfield
            // Flag to switch on/off long lines to the OutputDevice bounds
            unsigned                                mbExtendedLines : 1;

            // Flag to switch on/off the bounds itself
            unsigned                                mbShowBounds : 1;

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

        public:
            OverlayRollingRectangleStriped(
                const basegfx::B2DPoint& rBasePos,
                const basegfx::B2DPoint& rSecondPos,
                sal_Bool bExtendedLines = sal_False,
                sal_Bool bShowBounds = sal_True);
            virtual ~OverlayRollingRectangleStriped();

            // change second position
            const basegfx::B2DPoint& getSecondPosition() const { return maSecondPosition; }
            void setSecondPosition(const basegfx::B2DPoint& rNew);

            // change extended lines
            sal_Bool getExtendedLines() const { return mbExtendedLines; }
            void setExtendedLines(sal_Bool bNew);

            // change show bounds
            sal_Bool getShowBounds() const { return mbShowBounds; }
            void setShowBounds(sal_Bool bNew);

            // Hittest with logical coordinates
            virtual sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol = 0.0) const;

            // transform object coordinates. Needs to transform maSecondPosition.
            virtual void transform(const basegfx::B2DHomMatrix& rMatrix);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayRollingRectangle : public OverlayRollingRectangleStriped
        {
        protected:
            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

        public:
            OverlayRollingRectangle(
                const basegfx::B2DPoint& rBasePos,
                const basegfx::B2DPoint& rSecondPos,
                Color aLineColor = Color(COL_BLACK),
                sal_Bool bExtendedLines = sal_False,
                sal_Bool bShowBounds = sal_True);
            virtual ~OverlayRollingRectangle();
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYROOLINGRECTANGLE_HXX

// eof
