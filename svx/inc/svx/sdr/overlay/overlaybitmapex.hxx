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

#ifndef _SDR_OVERLAY_OVERLAYBITMAPEX_HXX
#define _SDR_OVERLAY_OVERLAYBITMAPEX_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayBitmapEx : public OverlayObjectWithBasePosition
        {
        protected:
            // the Bitmap itself
            BitmapEx                                maBitmapEx;

            // position of the basePosition inside the Bitmap, in pixels
            sal_uInt16                              mnCenterX;
            sal_uInt16                              mnCenterY;

            // optional transparency
            double mfAlpha;

            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlayBitmapEx(
                const basegfx::B2DPoint& rBasePos,
                const BitmapEx& rBitmapEx,
                sal_uInt16 nCenX = 0, sal_uInt16 nCenY = 0, double fAlpha = 0.0 );
            virtual ~OverlayBitmapEx();

            const BitmapEx& getBitmapEx() const { return maBitmapEx; }

            sal_uInt16 getCenterX() const { return mnCenterX; }
            sal_uInt16 getCenterY() const { return mnCenterY; }
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYBITMAPEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
