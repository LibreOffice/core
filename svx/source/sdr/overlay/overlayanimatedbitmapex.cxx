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
#include <vcl/outdev.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>



namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayAnimatedBitmapEx::createOverlayObjectPrimitive2DSequence()
        {
            if(mbOverlayState)
            {
                const drawinglayer::primitive2d::Primitive2DReference aPrimitive(
                    new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                        getBitmapEx1(),
                        getBasePosition(),
                        getCenterX1(),
                        getCenterY1(),
                        getShearX(),
                        getRotation()));

                return drawinglayer::primitive2d::Primitive2DSequence(&aPrimitive, 1);
            }
            else
            {
                const drawinglayer::primitive2d::Primitive2DReference aPrimitive(
                    new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                        getBitmapEx2(),
                        getBasePosition(),
                        getCenterX2(),
                        getCenterY2(),
                        getShearX(),
                        getRotation()));

                return drawinglayer::primitive2d::Primitive2DSequence(&aPrimitive, 1);
            }
        }

        OverlayAnimatedBitmapEx::OverlayAnimatedBitmapEx(
            const basegfx::B2DPoint& rBasePos,
            const BitmapEx& rBitmapEx1,
            const BitmapEx& rBitmapEx2,
            sal_uInt32 nBlinkTime,
            sal_uInt16 nCenX1,
            sal_uInt16 nCenY1,
            sal_uInt16 nCenX2,
            sal_uInt16 nCenY2,
            double fShearX,
            double fRotation)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_WHITE)),
            maBitmapEx1(rBitmapEx1),
            maBitmapEx2(rBitmapEx2),
            mnCenterX1(nCenX1), mnCenterY1(nCenY1),
            mnCenterX2(nCenX2), mnCenterY2(nCenY2),
            mnBlinkTime(nBlinkTime),
            mfShearX(fShearX),
            mfRotation(fRotation),
            mbOverlayState(false)
        {
            // set AllowsAnimation flag to mark this object as animation capable
            mbAllowsAnimation = true;

            // #i53216# check blink time value range
            mnBlinkTime = impCheckBlinkTimeValueRange(mnBlinkTime);
        }

        OverlayAnimatedBitmapEx::~OverlayAnimatedBitmapEx()
        {
        }

        void OverlayAnimatedBitmapEx::Trigger(sal_uInt32 nTime)
        {
            if(getOverlayManager())
            {
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
                getOverlayManager()->InsertEvent(this);

                // register change (after change)
                objectChange();
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
