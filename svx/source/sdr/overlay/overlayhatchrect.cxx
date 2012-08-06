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
