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

#include <sdr/overlay/overlayrollingrectangle.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sdr/overlay/overlaytools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>


namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DContainer OverlayRollingRectangleStriped::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DContainer aRetval;

            if(getOverlayManager() && (mbShowBounds || mbExtendedLines))
            {
                const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
                const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());
                const double fStripeLengthPixel(getOverlayManager()->getStripeLengthPixel());
                const basegfx::B2DRange aRollingRectangle(getBasePosition(), getSecondPosition());

                if(mbShowBounds)
                {
                    // view-independent part, create directly
                    const basegfx::B2DPolygon aPolygon(basegfx::utils::createPolygonFromRect(aRollingRectangle));

                    aRetval.resize(2);
                    aRetval[0] = new drawinglayer::primitive2d::PolyPolygonMarkerPrimitive2D(
                        basegfx::B2DPolyPolygon(aPolygon),
                        aRGBColorA,
                        aRGBColorB,
                        fStripeLengthPixel);

                    const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                    const basegfx::BColor aHilightColor(aSvtOptionsDrawinglayer.getHilightColor().getBColor());
                    const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);

                    aRetval[1] = new drawinglayer::primitive2d::PolyPolygonSelectionPrimitive2D(
                        basegfx::B2DPolyPolygon(aPolygon),
                        aHilightColor,
                        fTransparence,
                        3.0,
                        false);
                }

                if(mbExtendedLines)
                {
                    // view-dependent part, use helper primitive
                    const drawinglayer::primitive2d::Primitive2DReference aReference(
                        new drawinglayer::primitive2d::OverlayRollingRectanglePrimitive(
                            aRollingRectangle,
                            aRGBColorA,
                            aRGBColorB,
                            fStripeLengthPixel));

                    aRetval.push_back(aReference);
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
