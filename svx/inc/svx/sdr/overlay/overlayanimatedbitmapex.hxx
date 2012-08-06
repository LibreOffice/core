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

#ifndef _SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX
#define _SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayAnimatedBitmapEx : public OverlayObjectWithBasePosition
        {
        protected:
            // the Bitmaps
            BitmapEx                                maBitmapEx1;
            BitmapEx                                maBitmapEx2;

            // position of the basePosition inside the Bitmaps, in pixels
            sal_uInt16                              mnCenterX1;
            sal_uInt16                              mnCenterY1;
            sal_uInt16                              mnCenterX2;
            sal_uInt16                              mnCenterY2;

            // #i53216# added CursorBlinkTime (in ms)
            sal_uInt32                              mnBlinkTime;

            // bitfield
            // Flag to remember which state to draw. Inited with sal_False (0)
            unsigned                                mbOverlayState : 1;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

            // #i53216# check blink time value range (currently 25 < mnBlinkTime < 10000)
            void impCheckBlinkTimeValueRange();

        public:
            OverlayAnimatedBitmapEx(
                const basegfx::B2DPoint& rBasePos,
                const BitmapEx& rBitmapEx1,
                const BitmapEx& rBitmapEx2,
                sal_uInt32 nBlinkTime = 500,
                sal_uInt16 nCenX1 = 0,
                sal_uInt16 nCenY1 = 0,
                sal_uInt16 nCenX2 = 0,
                sal_uInt16 nCenY2 = 0);
            virtual ~OverlayAnimatedBitmapEx();

            const BitmapEx& getBitmapEx1() const { return maBitmapEx1; }
            const BitmapEx& getBitmapEx2() const { return maBitmapEx2; }

            sal_uInt16 getCenterX1() const { return mnCenterX1; }
            sal_uInt16 getCenterY1() const { return mnCenterY1; }
            sal_uInt16 getCenterX2() const { return mnCenterX2; }
            sal_uInt16 getCenterY2() const { return mnCenterY2; }

            // #i53216# added CursorBlinkTime (in ms)
            sal_uInt32 getBlinkTime() const { return mnBlinkTime; }

            // execute event from base class ::sdr::animation::Event. Default
            // implementation does nothing and does not create a new event.
            virtual void Trigger(sal_uInt32 nTime);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
