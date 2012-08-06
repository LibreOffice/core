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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
