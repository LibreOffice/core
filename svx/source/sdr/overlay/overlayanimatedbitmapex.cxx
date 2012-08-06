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

#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <vcl/outdev.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        // #i53216# check blink time value range
        void OverlayAnimatedBitmapEx::impCheckBlinkTimeValueRange()
        {
            if(mnBlinkTime < 25)
            {
                mnBlinkTime = 25;
            }
            else if(mnBlinkTime > 10000)
            {
                mnBlinkTime = 10000;
            }
        }

        drawinglayer::primitive2d::Primitive2DSequence OverlayAnimatedBitmapEx::createOverlayObjectPrimitive2DSequence()
        {
            if(mbOverlayState)
            {
                const drawinglayer::primitive2d::Primitive2DReference aPrimitive(
                    new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                        getBitmapEx1(),
                        getBasePosition(),
                        getCenterX1(),
                        getCenterY1()));

                return drawinglayer::primitive2d::Primitive2DSequence(&aPrimitive, 1);
            }
            else
            {
                const drawinglayer::primitive2d::Primitive2DReference aPrimitive(
                    new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                        getBitmapEx2(),
                        getBasePosition(),
                        getCenterX2(),
                        getCenterY2()));

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
            sal_uInt16 nCenY2)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_WHITE)),
            maBitmapEx1(rBitmapEx1),
            maBitmapEx2(rBitmapEx2),
            mnCenterX1(nCenX1), mnCenterY1(nCenY1),
            mnCenterX2(nCenX2), mnCenterY2(nCenY2),
            mnBlinkTime(nBlinkTime),
            mbOverlayState(false)
        {
            // set AllowsAnimation flag to mark this object as animation capable
            mbAllowsAnimation = true;

            // #i53216# check blink time value range
            impCheckBlinkTimeValueRange();
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
