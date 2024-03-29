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

#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <sdr/overlay/overlaytools.hxx>


namespace sdr::overlay
{
        drawinglayer::primitive2d::Primitive2DContainer OverlayBitmapEx::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                    maBitmapEx,
                    getBasePosition(),
                    mnCenterX,
                    mnCenterY,
                    mfShearX,
                    mfRotation));

            if(mfAlpha > 0.0)
            {
                drawinglayer::primitive2d::Primitive2DContainer aNewTransPrimitiveVector { aReference };
                aReference = new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(std::move(aNewTransPrimitiveVector), mfAlpha);
            }

            return drawinglayer::primitive2d::Primitive2DContainer { aReference };
        }

        OverlayBitmapEx::OverlayBitmapEx(
            const basegfx::B2DPoint& rBasePos,
            const BitmapEx& rBitmapEx,
            sal_uInt16 nCenX,
            sal_uInt16 nCenY,
            double fAlpha,
            double fShearX,
            double fRotation)
        :   OverlayObjectWithBasePosition(rBasePos, COL_WHITE),
            maBitmapEx(rBitmapEx),
            mnCenterX(nCenX),
            mnCenterY(nCenY),
            mfAlpha(fAlpha),
            mfShearX(fShearX),
            mfRotation(fRotation)
        {
        }

        OverlayBitmapEx::~OverlayBitmapEx()
        {
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
