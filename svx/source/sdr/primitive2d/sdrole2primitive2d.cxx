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

#include <svx/sdr/primitive2d/sdrole2primitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        SdrOle2Primitive2D::SdrOle2Primitive2D(
            const Primitive2DSequence& rOLEContent,
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute)
        :   BasePrimitive2D(),
            maOLEContent(rOLEContent),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute)
        {
        }

        bool SdrOle2Primitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const SdrOle2Primitive2D& rCompare = (SdrOle2Primitive2D&)rPrimitive;

                
                
                
                
                
                
                if(arePrimitive2DSequencesEqual(getOLEContent(), rCompare.getOLEContent())
                    && getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute())
                {
                    return true;
                }
            }

            return false;
        }

        Primitive2DSequence SdrOle2Primitive2D::get2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            
            
            
            
            static bool bBehaveCompatibleToPaintVersion(false);
            Primitive2DSequence  aRetval;

            
            const basegfx::B2DPolygon aUnitOutline(basegfx::tools::createUnitPolygon());

            
            if(!bBehaveCompatibleToPaintVersion
                && !getSdrLFSTAttribute().getFill().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            
            
            
            if(!bBehaveCompatibleToPaintVersion
                && !getSdrLFSTAttribute().getLine().isDefault())
            {
                
                
                if(0.0 != getSdrLFSTAttribute().getLine().getWidth())
                {
                    
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                    
                    double fHalfLineWidth(getSdrLFSTAttribute().getLine().getWidth() * 0.5);
                    double fScaleX(0.0 != aScale.getX() ? fHalfLineWidth / fabs(aScale.getX()) : 1.0);
                    double fScaleY(0.0 != aScale.getY() ? fHalfLineWidth / fabs(aScale.getY()) : 1.0);
                    const basegfx::B2DRange aExpandedRange(-fScaleX, -fScaleY, 1.0 + fScaleX, 1.0 + fScaleY);
                    basegfx::B2DPolygon aExpandedUnitOutline(basegfx::tools::createPolygonFromRect(aExpandedRange));

                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                        createPolygonLinePrimitive(
                            aExpandedUnitOutline,
                            getTransform(),
                            getSdrLFSTAttribute().getLine(),
                            attribute::SdrLineStartEndAttribute()));
                }
                else
                {
                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                        createPolygonLinePrimitive(
                            aUnitOutline,
                            getTransform(),
                            getSdrLFSTAttribute().getLine(),
                            attribute::SdrLineStartEndAttribute()));
                }
            }
            else
            {
                
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));
            }

            
            appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, getOLEContent());

            
            
            if(!getSdrLFSTAttribute().getText().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getText(),
                        getSdrLFSTAttribute().getLine(),
                        false,
                        false,
                        false));
            }

            
            if(!bBehaveCompatibleToPaintVersion
                && !getSdrLFSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval,
                    getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        
        ImplPrimitive2DIDBlock(SdrOle2Primitive2D, PRIMITIVE2D_ID_SDROLE2PRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
