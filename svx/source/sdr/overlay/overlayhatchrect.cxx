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


#include <svx/sdr/overlay/overlayhatchrect.hxx>
#include <vcl/hatch.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayHatchRect::createOverlayObjectPrimitive2DSequence()
        {
            const basegfx::B2DRange aHatchRange(getBasePosition(), getSecondPosition());
            const drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::OverlayHatchRectanglePrimitive(
                    aHatchRange,
                    3.0,
                    getHatchRotation(),
                    getBaseColor().getBColor(),
                    getDiscreteGrow(),
                    getDiscreteShrink(),
                    getRotation()));

            return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
        }

        OverlayHatchRect::OverlayHatchRect(
            const basegfx::B2DPoint& rBasePosition,
            const basegfx::B2DPoint& rSecondPosition,
            const Color& rHatchColor,
            double fDiscreteGrow,
            double fDiscreteShrink,
            double fHatchRotation,
            double fRotation)
        :   OverlayObjectWithBasePosition(rBasePosition, rHatchColor),
            maSecondPosition(rSecondPosition),
            mfDiscreteGrow(fDiscreteGrow),
            mfDiscreteShrink(fDiscreteShrink),
            mfHatchRotation(fHatchRotation),
            mfRotation(fRotation)
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
