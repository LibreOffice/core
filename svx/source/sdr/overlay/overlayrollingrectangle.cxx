/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
#include <tools/gen.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayRollingRectangleStriped::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval;

            if(getOverlayManager() && (getShowBounds() || getExtendedLines()))
            {
                const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
                const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());
                const double fStripeLengthPixel(getOverlayManager()->getStripeLengthPixel());
                const basegfx::B2DRange aRollingRectangle(getBasePosition(), getSecondPosition());

                if(getShowBounds())
                {
                    // view-independent part, create directly
                    const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(aRollingRectangle));
                    const drawinglayer::primitive2d::Primitive2DReference aReference(
                        new drawinglayer::primitive2d::PolygonMarkerPrimitive2D(
                            aPolygon,
                            aRGBColorA,
                            aRGBColorB,
                            fStripeLengthPixel));

                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aReference);
                }

                if(getExtendedLines())
                {
                    // view-dependent part, use helper primitive
                    const drawinglayer::primitive2d::Primitive2DReference aReference(
                        new drawinglayer::primitive2d::OverlayRollingRectanglePrimitive(
                            aRollingRectangle,
                            aRGBColorA,
                            aRGBColorB,
                            fStripeLengthPixel));

                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aReference);
                }
            }

            return aRetval;
        }

        void OverlayRollingRectangleStriped::stripeDefinitionHasChanged()
        {
            // react on OverlayManager's stripe definition change
            objectChange();
        }

        OverlayRollingRectangleStriped::OverlayRollingRectangleStriped(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            bool bExtendedLines,
            bool bShowBounds)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            maSecondPosition(rSecondPos),
            mbExtendedLines(bExtendedLines),
            mbShowBounds(bShowBounds)
        {
        }

        OverlayRollingRectangleStriped::~OverlayRollingRectangleStriped()
        {
        }

        void OverlayRollingRectangleStriped::setSecondPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maSecondPosition)
            {
                // remember new value
                maSecondPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
