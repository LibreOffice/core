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

#include <svx/sdr/overlay/overlayline.hxx>
#include <tools/gen.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayLineStriped::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval;

            if(getOverlayManager())
            {
                const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
                const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());
                const double fStripeLengthPixel(getOverlayManager()->getStripeLengthPixel());
                basegfx::B2DPolygon aLine;

                aLine.append(getBasePosition());
                aLine.append(getSecondPosition());

                const drawinglayer::primitive2d::Primitive2DReference aReference(
                    new drawinglayer::primitive2d::PolygonMarkerPrimitive2D(
                        aLine,
                        aRGBColorA,
                        aRGBColorB,
                        fStripeLengthPixel));

                aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
            }

            return aRetval;
        }

        void OverlayLineStriped::stripeDefinitionHasChanged()
        {
            // react on OverlayManager's stripe definition change
            objectChange();
        }

        OverlayLineStriped::OverlayLineStriped(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            maSecondPosition(rSecondPos)
        {
        }

        OverlayLineStriped::~OverlayLineStriped()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
