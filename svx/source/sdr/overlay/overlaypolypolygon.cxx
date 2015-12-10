/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>



namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DContainer OverlayPolyPolygonStripedAndFilled::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DContainer aRetval;

            if(getOverlayManager())
            {
                const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
                const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());
                const double fStripeLengthPixel(getOverlayManager()->getStripeLengthPixel());
                const drawinglayer::primitive2d::Primitive2DReference aStriped(
                    new drawinglayer::primitive2d::PolyPolygonMarkerPrimitive2D(
                        getLinePolyPolygon(),
                        aRGBColorA,
                        aRGBColorB,
                        fStripeLengthPixel));

                aRetval = drawinglayer::primitive2d::Primitive2DContainer { aStriped };

                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                const basegfx::BColor aHilightColor(aSvtOptionsDrawinglayer.getHilightColor().getBColor());
                const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);

                const drawinglayer::primitive2d::Primitive2DReference aFilled(
                    new drawinglayer::primitive2d::PolyPolygonSelectionPrimitive2D(
                        getLinePolyPolygon(),
                        aHilightColor,
                        fTransparence,
                        3.0,
                        false));

                aRetval.push_back(aFilled);
            }

            return aRetval;
        }

        void OverlayPolyPolygonStripedAndFilled::stripeDefinitionHasChanged()
        {
            // react on OverlayManager's stripe definition change
            objectChange();
        }

        OverlayPolyPolygonStripedAndFilled::OverlayPolyPolygonStripedAndFilled(
            const basegfx::B2DPolyPolygon& rLinePolyPolygon)
        :   OverlayObject(Color(COL_BLACK)),
            maLinePolyPolygon(rLinePolyPolygon)
        {
        }

        OverlayPolyPolygonStripedAndFilled::~OverlayPolyPolygonStripedAndFilled()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
