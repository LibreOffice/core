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

#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayBitmapEx::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                    getBitmapEx(),
                    getBasePosition(),
                    getCenterX(),
                    getCenterY()));

            if(basegfx::fTools::more(mfAlpha, 0.0))
            {
                const drawinglayer::primitive2d::Primitive2DSequence aNewTransPrimitiveVector(&aReference, 1L);
                aReference = drawinglayer::primitive2d::Primitive2DReference(
                                new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(aNewTransPrimitiveVector, mfAlpha));
            }

            return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
        }

        OverlayBitmapEx::OverlayBitmapEx(
            const basegfx::B2DPoint& rBasePos,
            const BitmapEx& rBitmapEx,
            sal_uInt16 nCenX, sal_uInt16 nCenY, double fAlpha)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_WHITE)),
            maBitmapEx(rBitmapEx),
            mnCenterX(nCenX),
            mnCenterY(nCenY),
            mfAlpha(fAlpha)
        {
        }

        OverlayBitmapEx::~OverlayBitmapEx()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
