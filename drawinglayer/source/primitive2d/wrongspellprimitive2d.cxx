/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence WrongSpellPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            
            
            
            
            
            
            
            
            
            

            
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

            
            static double fDefaultDistance(0.03);
            const double fFontHeight(aScale.getY());
            const double fUnderlineDistance(fFontHeight * fDefaultDistance);
            const double fWaveWidth(2.0 * fUnderlineDistance);

            
            
            const double fRelativeUnderlineDistance(basegfx::fTools::equalZero(aScale.getY()) ? 0.0 : fUnderlineDistance / aScale.getY());
            basegfx::B2DPoint aStart(getStart(), fRelativeUnderlineDistance);
            basegfx::B2DPoint aStop(getStop(), fRelativeUnderlineDistance);
            basegfx::B2DPolygon aPolygon;

            aPolygon.append(getTransformation() * aStart);
            aPolygon.append(getTransformation() * aStop);

            
            const attribute::LineAttribute aLineAttribute(getColor());

            
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

        
        ImplPrimitive2DIDBlock(WrongSpellPrimitive2D, PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
