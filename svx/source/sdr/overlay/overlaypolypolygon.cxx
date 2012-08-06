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

#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayPolyPolygonStriped::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval;

            if(getOverlayManager())
            {
                const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
                const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());
                const double fStripeLengthPixel(getOverlayManager()->getStripeLengthPixel());

                const drawinglayer::primitive2d::Primitive2DReference aReference(
                    new drawinglayer::primitive2d::PolyPolygonMarkerPrimitive2D(
                        getPolyPolygon(),
                        aRGBColorA,
                        aRGBColorB,
                        fStripeLengthPixel));

                aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
            }

            return aRetval;
        }

        void OverlayPolyPolygonStriped::stripeDefinitionHasChanged()
        {
            // react on OverlayManager's stripe definition change
            objectChange();
        }

        OverlayPolyPolygonStriped::OverlayPolyPolygonStriped(
            const basegfx::B2DPolyPolygon& rPolyPolygon)
        :   OverlayObject(Color(COL_BLACK)),
            maPolyPolygon(rPolyPolygon)
        {
        }

        OverlayPolyPolygonStriped::~OverlayPolyPolygonStriped()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
