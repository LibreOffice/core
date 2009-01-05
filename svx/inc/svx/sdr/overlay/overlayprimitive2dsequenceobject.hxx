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

#ifndef _SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX
#define _SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayPrimitive2DSequenceObject : public OverlayObjectWithBasePosition
        {
        protected:
            // the sequence of primitives to show
            const drawinglayer::primitive2d::Primitive2DSequence    maSequence;

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

            // Hittest with logical coordinates. Default tests against maBaseRange.
            virtual sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol = 0.0) const;

        private:
            // internal helper to create a drawinglayer::geometry::ViewInformation2D
            // using a OutputDevice and local knowledge
            drawinglayer::geometry::ViewInformation2D impCreateViewInformation2D(OutputDevice& rOutputDevice) const;

        public:
            OverlayPrimitive2DSequenceObject(const drawinglayer::primitive2d::Primitive2DSequence& rSequence);

            virtual ~OverlayPrimitive2DSequenceObject();

            // data read access
            const drawinglayer::primitive2d::Primitive2DSequence& getSequence() const { return maSequence; }
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX

// eof
