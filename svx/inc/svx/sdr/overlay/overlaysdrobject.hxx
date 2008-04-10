/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaysdrobject.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SDR_OVERLAY_OVERLAYSDROBJECT_HXX
#define _SDR_OVERLAY_OVERLAYSDROBJECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlaySdrObject : public OverlayObjectWithBasePosition
        {
        protected:
            // the SdrObject to show
            const SdrObject&                        mrSdrObject;

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

            // Hittest with logical coordinates. Default tests against maBaseRange.
            virtual sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol = 0.0) const;

        public:
            OverlaySdrObject(
                const basegfx::B2DPoint& rBasePos,
                const SdrObject& rObject);
            virtual ~OverlaySdrObject();
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYSDROBJECT_HXX

// eof
