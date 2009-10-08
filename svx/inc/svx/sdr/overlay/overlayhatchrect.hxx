/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaybitmap.cxx,v $
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

#ifndef _SDR_OVERLAY_OVERLAYHATCHRECT_HXX
#define _SDR_OVERLAY_OVERLAYHATCHRECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/hatch.hxx>
#include <tools/gen.hxx>

//////////////////////////////////////////////////////////////////////////////

class PolyPolygon;

namespace sdr
{
    namespace overlay
    {
        class OverlayHatchRect : public OverlayObjectWithBasePosition
        {
            // geometric definitions
            basegfx::B2DPoint               maSecondPosition;
            const double                    mfDiscreteWidth;
            const double                    mfHatchRotation;
            const double                    mfRotation;

            // helper to create visualisation geometry from above values
            basegfx::B2DPolyPolygon getGeometry(OutputDevice& rOutputDevice);

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

        public:
            OverlayHatchRect(
                const basegfx::B2DPoint& rBasePosition,
                const basegfx::B2DPoint& rSecondPosition,
                const Color& rHatchColor,
                double fDiscreteWidth,
                double fHatchRotation,
                double fRotation);

            const basegfx::B2DPoint& getSecondPosition() const { return maSecondPosition; }
            void setSecondPosition(const basegfx::B2DPoint&);

            // Zoom has changed. If the objects logical size
            // depends on the MapMode of the used OutputDevice, use this call
            // to invalidate the range in logical coordinates.
            virtual void zoomHasChanged();

            // data read access
            double getDiscreteWidth() const { return mfDiscreteWidth; }
            double getHatchRotation() const { return mfHatchRotation; }
            double getRotation() const { return mfRotation; }
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYHATCHRECT_HXX

// eof
