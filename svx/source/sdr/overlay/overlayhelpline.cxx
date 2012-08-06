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

#include <svx/sdr/overlay/overlayhelpline.hxx>
#include <tools/gen.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayHelplineStriped::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval;

            if(getOverlayManager())
            {
                const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
                const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());
                const double fStripeLengthPixel(getOverlayManager()->getStripeLengthPixel());
                const drawinglayer::primitive2d::HelplineStyle aStyle(
                    SDRHELPLINE_POINT == getKind() ? drawinglayer::primitive2d::HELPLINESTYLE_POINT :
                        SDRHELPLINE_VERTICAL == getKind() ? drawinglayer::primitive2d::HELPLINESTYLE_VERTICAL :
                            drawinglayer::primitive2d::HELPLINESTYLE_HORIZONTAL);

                const drawinglayer::primitive2d::Primitive2DReference aReference(
                    new drawinglayer::primitive2d::OverlayHelplineStripedPrimitive(
                        getBasePosition(),
                        aStyle,
                        aRGBColorA,
                        aRGBColorB,
                        fStripeLengthPixel));

                aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
            }

            return aRetval;
        }

        void OverlayHelplineStriped::stripeDefinitionHasChanged()
        {
            // react on OverlayManager's stripe definition change
            objectChange();
        }

        OverlayHelplineStriped::OverlayHelplineStriped(
            const basegfx::B2DPoint& rBasePos,
            SdrHelpLineKind eNewKind)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            meKind(eNewKind)
        {
        }

        OverlayHelplineStriped::~OverlayHelplineStriped()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
