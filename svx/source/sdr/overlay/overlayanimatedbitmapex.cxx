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

#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <sdr/overlay/overlaytools.hxx>


namespace sdr::overlay
{
        drawinglayer::primitive2d::Primitive2DContainer OverlayAnimatedBitmapEx::createOverlayObjectPrimitive2DSequence()
        {
            if(mbOverlayState)
            {
                const drawinglayer::primitive2d::Primitive2DReference aPrimitive(
                    new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                        maBitmapEx1,
                        getBasePosition(),
                        mnCenterX1,
                        mnCenterY1,
                        getShearX(),
                        getRotation()));

                return drawinglayer::primitive2d::Primitive2DContainer { aPrimitive };
            }
            else
            {
                const drawinglayer::primitive2d::Primitive2DReference aPrimitive(
                    new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                        maBitmapEx2,
                        getBasePosition(),
                        mnCenterX2,
                        mnCenterY2,
                        getShearX(),
                        getRotation()));

                return drawinglayer::primitive2d::Primitive2DContainer { aPrimitive };
            }
        }

        OverlayAnimatedBitmapEx::OverlayAnimatedBitmapEx(
            const basegfx::B2DPoint& rBasePos,
            const BitmapEx& rBitmapEx1,
            const BitmapEx& rBitmapEx2,
            sal_uInt64 nBlinkTime,
            sal_uInt16 nCenX1,
            sal_uInt16 nCenY1,
            sal_uInt16 nCenX2,
            sal_uInt16 nCenY2,
            double fShearX,
            double fRotation)
        :   OverlayObjectWithBasePosition(rBasePos, COL_WHITE),
            maBitmapEx1(rBitmapEx1),
            maBitmapEx2(rBitmapEx2),
            mnCenterX1(nCenX1), mnCenterY1(nCenY1),
            mnCenterX2(nCenX2), mnCenterY2(nCenY2),
            mnBlinkTime(impCheckBlinkTimeValueRange(nBlinkTime)),
            mfShearX(fShearX),
            mfRotation(fRotation),
            mbOverlayState(false)
        {
            // set AllowsAnimation flag to mark this object as animation capable
            mbAllowsAnimation = true;
        }

        OverlayAnimatedBitmapEx::~OverlayAnimatedBitmapEx()
        {
        }

        void OverlayAnimatedBitmapEx::Trigger(sal_uInt32 nTime)
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
