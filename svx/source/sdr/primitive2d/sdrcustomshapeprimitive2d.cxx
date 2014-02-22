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

#include <svx/sdr/primitive2d/sdrcustomshapeprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrCustomShapePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval(getSubPrimitives());

            
            if(!getSdrSTAttribute().getText().isDefault())
            {
                const basegfx::B2DPolygon aUnitOutline(basegfx::tools::createUnitPolygon());

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTextBox(),
                        getSdrSTAttribute().getText(),
                        attribute::SdrLineAttribute(),
                        false,
                        getWordWrap(),
                        isForceTextClipToTextRange()));
            }

            
            if(aRetval.hasElements() && !getSdrSTAttribute().getShadow().isDefault())
            {
                
                
                
                
                //
                
                
                
                
                
                if(!get3DShape())
                {
                    aRetval = createEmbeddedShadowPrimitive(aRetval, getSdrSTAttribute().getShadow());
                }
            }

            return aRetval;
        }

        SdrCustomShapePrimitive2D::SdrCustomShapePrimitive2D(
            const attribute::SdrShadowTextAttribute& rSdrSTAttribute,
            const Primitive2DSequence& rSubPrimitives,
            const basegfx::B2DHomMatrix& rTextBox,
            bool bWordWrap,
            bool b3DShape,
            bool bForceTextClipToTextRange)
        :   BufferedDecompositionPrimitive2D(),
            maSdrSTAttribute(rSdrSTAttribute),
            maSubPrimitives(rSubPrimitives),
            maTextBox(rTextBox),
            mbWordWrap(bWordWrap),
            mb3DShape(b3DShape),
            mbForceTextClipToTextRange(bForceTextClipToTextRange)
        {
        }

        bool SdrCustomShapePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrCustomShapePrimitive2D& rCompare = (SdrCustomShapePrimitive2D&)rPrimitive;

                return (getSdrSTAttribute() == rCompare.getSdrSTAttribute()
                    && getSubPrimitives() == rCompare.getSubPrimitives()
                    && getTextBox() == rCompare.getTextBox()
                    && getWordWrap() == rCompare.getWordWrap()
                    && get3DShape() == rCompare.get3DShape()
                    && isForceTextClipToTextRange() == rCompare.isForceTextClipToTextRange());
            }

            return false;
        }

        
        ImplPrimitive2DIDBlock(SdrCustomShapePrimitive2D, PRIMITIVE2D_ID_SDRCUSTOMSHAPEPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
