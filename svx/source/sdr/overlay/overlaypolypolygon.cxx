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
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <drawinglayer/primitive2d/PolyPolygonMarkerPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonSelectionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>


namespace sdr::overlay
{
        OverlayPolyPolygon::OverlayPolyPolygon(
                            const basegfx::B2DPolyPolygon& rLinePolyPolygon,
                            Color const & rLineColor,
                            double fLineWidth,
                            Color const & rFillColor)
            : OverlayObject(rLineColor)
            , maLinePolyPolygon(rLinePolyPolygon)
            , mfLineWidth(fLineWidth)
            , maFillColor(rFillColor)
        {
        }

        OverlayPolyPolygon::~OverlayPolyPolygon() = default;

        drawinglayer::primitive2d::Primitive2DContainer OverlayPolyPolygon::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DContainer aReturnContainer;

            if (getOverlayManager())
            {
                const drawinglayer::attribute::LineAttribute aLineAttribute(getBaseColor().getBColor(), mfLineWidth);

                aReturnContainer = drawinglayer::primitive2d::Primitive2DContainer {
                    new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D(maLinePolyPolygon, aLineAttribute) };

                if (maFillColor.GetAlpha() != 0)
                {
                    aReturnContainer.push_back(new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(maLinePolyPolygon, maFillColor.getBColor()));
                }

                sal_uInt8 nTransparency = 255 - getBaseColor().GetAlpha();
                if (nTransparency > 0)
                {
                    const drawinglayer::primitive2d::Primitive2DReference aTransparencePrimitive(
                        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(aReturnContainer, nTransparency / 255.0));
                    aReturnContainer = drawinglayer::primitive2d::Primitive2DContainer{ aTransparencePrimitive };
                }
            }

            return aReturnContainer;
        }

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
        :   OverlayObject(COL_BLACK),
            maLinePolyPolygon(rLinePolyPolygon)
        {
        }

        OverlayPolyPolygonStripedAndFilled::~OverlayPolyPolygonStripedAndFilled()
        {
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
