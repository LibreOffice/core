/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaysdrobject.cxx,v $
 * $Revision: 1.7 $
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

#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>
#include <svx/sdr/contact/objectcontacttools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayPrimitive2DSequenceObject::drawGeometry(OutputDevice& rOutputDevice)
        {
            if(getOverlayManager())
            {
                // prepare ViewInformation2D
                const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                    basegfx::B2DHomMatrix(),
                    rOutputDevice.GetViewTransformation(),
                    basegfx::B2DRange(),
                    0,
                    0.0,
                    0);

                // create processor
                drawinglayer::processor2d::BaseProcessor2D* pProcessor = ::sdr::contact::createBaseProcessor2DFromOutputDevice(
                    rOutputDevice,
                    aViewInformation2D);

                if(pProcessor)
                {
                    pProcessor->process(getSequence());

                    delete pProcessor;
                }
            }
        }

        void OverlayPrimitive2DSequenceObject::createBaseRange(OutputDevice& rOutputDevice)
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                basegfx::B2DHomMatrix(),
                rOutputDevice.GetViewTransformation(),
                basegfx::B2DRange(),
                0,
                0.0,
                0);

            maBaseRange = drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                getSequence(), aViewInformation2D);
        }

        sal_Bool OverlayPrimitive2DSequenceObject::isHit(const basegfx::B2DPoint& /*rPos*/, double /*fTol*/) const
        {
            if(isHittable())
            {
                return false;
            }

            return false;
        }

        OverlayPrimitive2DSequenceObject::OverlayPrimitive2DSequenceObject(const drawinglayer::primitive2d::Primitive2DSequence& rSequence)
        :   OverlayObjectWithBasePosition(basegfx::B2DPoint(), Color(COL_BLACK)),
            maSequence(rSequence)
        {
        }

        OverlayPrimitive2DSequenceObject::~OverlayPrimitive2DSequenceObject()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
