/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaycrosshair.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/overlay/overlaycrosshair.hxx>
#include <tools/gen.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayCrosshairStriped::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval;

            if(getOverlayManager())
            {
                const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
                const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());
                const double fStripeLengthPixel(getOverlayManager()->getStripeLengthPixel());

                const drawinglayer::primitive2d::Primitive2DReference aReference(
                    new drawinglayer::primitive2d::OverlayCrosshairPrimitive(
                        getBasePosition(),
                        aRGBColorA,
                        aRGBColorB,
                        fStripeLengthPixel));

                aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
            }

            return aRetval;
        }

        void OverlayCrosshairStriped::stripeDefinitionHasChanged()
        {
            // react on OverlayManager's stripe definition change
            objectChange();
        }

        OverlayCrosshairStriped::OverlayCrosshairStriped(const basegfx::B2DPoint& rBasePos)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK))
        {
        }

        OverlayCrosshairStriped::~OverlayCrosshairStriped()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
