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

#ifndef INCLUDED_SVX_SDR_OVERLAY_OVERLAYBITMAPEX_HXX
#define INCLUDED_SVX_SDR_OVERLAY_OVERLAYBITMAPEX_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/bitmapex.hxx>



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

            // optional transparency, shear and rotation
            double                                  mfAlpha;
            double                                  mfShearX;
            double                                  mfRotation;

            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence() override;

        public:
            OverlayBitmapEx(
                const basegfx::B2DPoint& rBasePos,
                const BitmapEx& rBitmapEx,
                sal_uInt16 nCenX = 0,
                sal_uInt16 nCenY = 0,
                double fAlpha = 0.0,
                double fShearX = 0.0,
                double fRotation = 0.0);
            virtual ~OverlayBitmapEx();

            const BitmapEx& getBitmapEx() const { return maBitmapEx; }

            sal_uInt16 getCenterX() const { return mnCenterX; }
            sal_uInt16 getCenterY() const { return mnCenterY; }

            // get shearX and rotation
            double getShearX() const { return mfShearX; }
            double getRotation() const { return mfRotation; }
        };
    } // end of namespace overlay
} // end of namespace sdr



#endif // INCLUDED_SVX_SDR_OVERLAY_OVERLAYBITMAPEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
