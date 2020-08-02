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

#include <sdr/overlay/overlayrectangle.hxx>
#include <sdr/overlay/overlaytools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>


namespace sdr::overlay
{
        drawinglayer::primitive2d::Primitive2DContainer OverlayRectangle::createOverlayObjectPrimitive2DSequence()
        {
            const basegfx::B2DRange aHatchRange(getBasePosition(), maSecondPosition);
            basegfx::BColor aColor(getBaseColor().getBColor());
            static const double fChange(0.1); // just small optical change, do not make it annoying

            if(mbOverlayState)
            {
                aColor += basegfx::B3DTuple(fChange, fChange, fChange);
                aColor.clamp();
            }
            else
            {
                aColor -= basegfx::B3DTuple(fChange, fChange, fChange);
                aColor.clamp();
            }

            const drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::OverlayRectanglePrimitive(
                    aHatchRange,
                    aColor,
                    mfTransparence,
                    mfDiscreteGrow,
                    mfDiscreteShrink,
                    mfRotation));

            return drawinglayer::primitive2d::Primitive2DContainer { aReference };
        }

        OverlayRectangle::OverlayRectangle(
            const basegfx::B2DPoint& rBasePosition,
            const basegfx::B2DPoint& rSecondPosition,
            const Color& rHatchColor,
            double fTransparence,
            double fDiscreteGrow,
            double fDiscreteShrink,
            double fRotation,
            bool bAnimate)
        :   OverlayObjectWithBasePosition(rBasePosition, rHatchColor),
            maSecondPosition(rSecondPosition),
            mfTransparence(fTransparence),
            mfDiscreteGrow(fDiscreteGrow),
            mfDiscreteShrink(fDiscreteShrink),
            mfRotation(fRotation),
            mnBlinkTime(impCheckBlinkTimeValueRange(500)),
            mbOverlayState(false)
        {
            if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
            {
                // no animation in high contrast mode
                bAnimate = false;
            }

            // set AllowsAnimation flag to mark this object as animation capable
            mbAllowsAnimation = bAnimate;
        }

        void OverlayRectangle::Trigger(sal_uInt32 nTime)
        {
            if(!getOverlayManager())
                return;

            // #i53216# produce event after nTime + x
            SetTime(nTime + mnBlinkTime);

            // switch state
            if(mbOverlayState)
            {
                mbOverlayState = false;
            }
            else
            {
                mbOverlayState = true;
            }

            // re-insert me as event
            getOverlayManager()->InsertEvent(*this);

            // register change (after change)
            objectChange();
        }
} // end of namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
