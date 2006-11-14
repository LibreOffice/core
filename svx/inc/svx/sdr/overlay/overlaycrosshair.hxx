/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaycrosshair.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:07:02 $
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

#ifndef _SDR_OVERLAY_OVERLAYCROSSHAIR_HXX
#define _SDR_OVERLAY_OVERLAYCROSSHAIR_HXX

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayCrosshairStriped : public OverlayObjectWithBasePosition
        {
        protected:
            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

        public:
            OverlayCrosshairStriped(const basegfx::B2DPoint& rBasePos);
            virtual ~OverlayCrosshairStriped();

            // Hittest with logical coordinates
            virtual sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol = 0.0) const;
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayCrosshair : public OverlayCrosshairStriped
        {
        protected:
            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

        public:
            OverlayCrosshair(
                const basegfx::B2DPoint& rBasePos,
                Color aLineColor = Color(COL_BLACK));
            virtual ~OverlayCrosshair();
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYCROSSHAIR_HXX

// eof
