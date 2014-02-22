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

#include <svx/sdr/primitive2d/sdrellipseprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrEllipsePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            
            
            
            
            basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromUnitCircle(1));

            
            const basegfx::B2DHomMatrix aUnitCorrectionMatrix(
                basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));

            
            aUnitOutline.transform(aUnitCorrectionMatrix);

            
            if(!getSdrLFSTAttribute().getFill().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            
            if(getSdrLFSTAttribute().getLine().isDefault())
            {
                
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));
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

            
            if(!getSdrLFSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval,
                    getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrEllipsePrimitive2D::SdrEllipsePrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute)
        {
        }

        bool SdrEllipsePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrEllipsePrimitive2D& rCompare = (SdrEllipsePrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute());
            }

            return false;
        }

        
        ImplPrimitive2DIDBlock(SdrEllipsePrimitive2D, PRIMITIVE2D_ID_SDRELLIPSEPRIMITIVE2D)

    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrEllipseSegmentPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            
            basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromUnitEllipseSegment(mfStartAngle, mfEndAngle));

            if(mbCloseSegment)
            {
                if(mbCloseUsingCenter)
                {
                    
                    
                    aUnitOutline.insert(0L, basegfx::B2DPoint(0.0, 0.0));
                }

                aUnitOutline.setClosed(true);
            }

            
            const basegfx::B2DHomMatrix aUnitCorrectionMatrix(
                basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));

            
            aUnitOutline.transform(aUnitCorrectionMatrix);

            
            if(!getSdrLFSTAttribute().getFill().isDefault() && aUnitOutline.isClosed())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            
            if(getSdrLFSTAttribute().getLine().isDefault())
            {
                
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));
            }
            else
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolygonLinePrimitive(
                        aUnitOutline,
                        getTransform(),
                        getSdrLFSTAttribute().getLine(),
                        getSdrLFSTAttribute().getLineStartEnd()));
            }

            
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

            
            if(!getSdrLFSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval,
                    getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrEllipseSegmentPrimitive2D::SdrEllipseSegmentPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            double fStartAngle,
            double fEndAngle,
            bool bCloseSegment,
            bool bCloseUsingCenter)
        :   SdrEllipsePrimitive2D(rTransform, rSdrLFSTAttribute),
            mfStartAngle(fStartAngle),
            mfEndAngle(fEndAngle),
            mbCloseSegment(bCloseSegment),
            mbCloseUsingCenter(bCloseUsingCenter)
        {
        }

        bool SdrEllipseSegmentPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrEllipsePrimitive2D::operator==(rPrimitive))
            {
                const SdrEllipseSegmentPrimitive2D& rCompare = (SdrEllipseSegmentPrimitive2D&)rPrimitive;

                if( mfStartAngle == rCompare.mfStartAngle
                    && mfEndAngle == rCompare.mfEndAngle
                    && mbCloseSegment == rCompare.mbCloseSegment
                    && mbCloseUsingCenter == rCompare.mbCloseUsingCenter)
                {
                    return true;
                }
            }

            return false;
        }

        
        ImplPrimitive2DIDBlock(SdrEllipseSegmentPrimitive2D, PRIMITIVE2D_ID_SDRELLIPSESEGMENTPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
