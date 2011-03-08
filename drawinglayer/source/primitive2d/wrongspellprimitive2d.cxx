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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence WrongSpellPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // ATM this decompose is view-independent, what the original VCL-Display is not. To mimic
            // the old behaviour here if wanted it is necessary to add get2DDecomposition and implement
            // it similar to the usage in e.g. HelplinePrimitive2D. Remembering the ViewTransformation
            // should be enough then.
            // The view-independent wavelines work well (if You ask me). Maybe the old VCL-Behaviour is only
            // in place because it was not possible/too expensive at that time to scale the wavelines with the
            // view...
            // With the VCL-PixelRenderer this will not even be used since it implements WrongSpellPrimitive2D
            // directly and mimics the old VCL-Display there. If You implemented a new renderer without
            // direct WrongSpellPrimitive2D support, You may want to do the described change here.

            // get the font height (part of scale), so decompose the matrix
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

            // calculate distances based on a static default (to allow testing in debugger)
            static double fDefaultDistance(0.03);
            const double fFontHeight(aScale.getY());
            const double fUnderlineDistance(fFontHeight * fDefaultDistance);
            const double fWaveWidth(2.0 * fUnderlineDistance);

            // the Y-distance needs to be relativated to FontHeight since the points get
            // transformed with the transformation containing that scale already.
            const double fRelativeUnderlineDistance(basegfx::fTools::equalZero(aScale.getY()) ? 0.0 : fUnderlineDistance / aScale.getY());
            basegfx::B2DPoint aStart(getStart(), fRelativeUnderlineDistance);
            basegfx::B2DPoint aStop(getStop(), fRelativeUnderlineDistance);
            basegfx::B2DPolygon aPolygon;

            aPolygon.append(getTransformation() * aStart);
            aPolygon.append(getTransformation() * aStop);

            // prepare line attribute
            const attribute::LineAttribute aLineAttribute(getColor());

            // create the waveline primitive
            Primitive2DReference xPrimitive(new PolygonWavePrimitive2D(aPolygon, aLineAttribute, fWaveWidth, 0.5 * fWaveWidth));
            Primitive2DSequence xRetval(&xPrimitive, 1);

            return xRetval;
        }

        WrongSpellPrimitive2D::WrongSpellPrimitive2D(
            const basegfx::B2DHomMatrix& rTransformation,
            double fStart,
            double fStop,
            const basegfx::BColor& rColor)
        :   BufferedDecompositionPrimitive2D(),
            maTransformation(rTransformation),
            mfStart(fStart),
            mfStop(fStop),
            maColor(rColor)
        {
        }

        bool WrongSpellPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const WrongSpellPrimitive2D& rCompare = (WrongSpellPrimitive2D&)rPrimitive;

                return (getTransformation() == rCompare.getTransformation()
                    && getStart() == rCompare.getStart()
                    && getStop() == rCompare.getStop()
                    && getColor() == rCompare.getColor());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(WrongSpellPrimitive2D, PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
