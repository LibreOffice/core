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

#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/texteffectprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>



using namespace com::sun::star;



namespace
{
    
    
    
    
    
    
    basegfx::B2DVector getCorrectedScaleAndFontScale(basegfx::B2DVector& rScale)
    {
        
        basegfx::B2DVector aFontScale(rScale);

        
        if(basegfx::fTools::equalZero(aFontScale.getY()))
        {
            
            static double fDefaultFontScale(100.0);
            rScale.setY(1.0 / fDefaultFontScale);
            aFontScale.setY(fDefaultFontScale);
        }
        else if(basegfx::fTools::less(aFontScale.getY(), 0.0))
        {
            
            aFontScale.setY(-aFontScale.getY());
            rScale.setY(-1.0);
        }
        else
        {
            
            rScale.setY(1.0);
        }

        
        if(basegfx::fTools::equal(aFontScale.getX(), aFontScale.getY()))
        {
            
            rScale.setX(1.0);
        }
        else
        {
            
            
            rScale.setX(aFontScale.getX() / aFontScale.getY());
            aFontScale.setX(aFontScale.getY());
        }

        return aFontScale;
    }
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        void TextSimplePortionPrimitive2D::getTextOutlinesAndTransformation(basegfx::B2DPolyPolygonVector& rTarget, basegfx::B2DHomMatrix& rTransformation) const
        {
            if(getTextLength())
            {
                
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;

                
                
                if(getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    
                    
                    if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
                    {
                        aScale = basegfx::absolute(aScale);
                        fRotate += F_PI;
                    }

                    
                    
                    
                    
                    const basegfx::B2DVector aFontScale(getCorrectedScaleAndFontScale(aScale));

                    
                    TextLayouterDevice aTextLayouter;
                    aTextLayouter.setFontAttribute(
                        getFontAttribute(),
                        aFontScale.getX(),
                        aFontScale.getY(),
                        getLocale());

                    
                    
                    
                    if(getDXArray().size() && !basegfx::fTools::equal(aScale.getX(), 1.0))
                    {
                        ::std::vector< double > aScaledDXArray = getDXArray();
                        const double fDXArrayScale(1.0 / aScale.getX());

                        for(sal_uInt32 a(0); a < aScaledDXArray.size(); a++)
                        {
                            aScaledDXArray[a] *= fDXArrayScale;
                        }

                        
                        aTextLayouter.getTextOutlines(
                            rTarget,
                            getText(),
                            getTextPosition(),
                            getTextLength(),
                            aScaledDXArray);
                    }
                    else
                    {
                        
                        aTextLayouter.getTextOutlines(
                            rTarget,
                            getText(),
                            getTextPosition(),
                            getTextLength(),
                            getDXArray());
                    }

                    
                    const sal_uInt32 nCount(rTarget.size());

                    if(nCount)
                    {
                        
                        rTransformation = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            aScale, fShearX, fRotate, aTranslate);
                    }
                }
            }
        }

        Primitive2DSequence TextSimplePortionPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            if(getTextLength())
            {
                basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
                basegfx::B2DHomMatrix aPolygonTransform;

                
                getTextOutlinesAndTransformation(aB2DPolyPolyVector, aPolygonTransform);

                
                const sal_uInt32 nCount(aB2DPolyPolyVector.size());

                if(nCount)
                {
                    
                    aRetval.realloc(nCount);

                    
                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        
                        basegfx::B2DPolyPolygon& rPolyPolygon = aB2DPolyPolyVector[a];
                        rPolyPolygon.transform(aPolygonTransform);
                        aRetval[a] = new PolyPolygonColorPrimitive2D(rPolyPolygon, getFontColor());
                    }

                    if(getFontAttribute().getOutline())
                    {
                        
                        basegfx::B2DVector aScale, aTranslate;
                        double fRotate, fShearX;
                        aPolygonTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                        
                        Primitive2DReference aNewTextEffect(new TextEffectPrimitive2D(
                            aRetval,
                            aTranslate,
                            fRotate,
                            TEXTEFFECTSTYLE2D_OUTLINE));

                        aRetval = Primitive2DSequence(&aNewTextEffect, 1);
                    }
                }
            }

            return aRetval;
        }

        TextSimplePortionPrimitive2D::TextSimplePortionPrimitive2D(
            const basegfx::B2DHomMatrix& rNewTransform,
            const OUString& rText,
            sal_Int32 nTextPosition,
            sal_Int32 nTextLength,
            const ::std::vector< double >& rDXArray,
            const attribute::FontAttribute& rFontAttribute,
            const ::com::sun::star::lang::Locale& rLocale,
            const basegfx::BColor& rFontColor,
            bool bFilled,
            long nWidthToFill)
        :   BufferedDecompositionPrimitive2D(),
            maTextTransform(rNewTransform),
            maText(rText),
            mnTextPosition(nTextPosition),
            mnTextLength(nTextLength),
            maDXArray(rDXArray),
            maFontAttribute(rFontAttribute),
            maLocale(rLocale),
            maFontColor(rFontColor),
            maB2DRange(),
            mbFilled(bFilled),
            mnWidthToFill(nWidthToFill)
        {
#if OSL_DEBUG_LEVEL > 0
            const sal_Int32 aStringLength(getText().getLength());
            OSL_ENSURE(aStringLength >= getTextPosition() && aStringLength >= getTextPosition() + getTextLength(),
                "TextSimplePortionPrimitive2D with text out of range (!)");
#endif
        }

        bool LocalesAreEqual(const ::com::sun::star::lang::Locale& rA, const ::com::sun::star::lang::Locale& rB)
        {
            return (rA.Language == rB.Language
                && rA.Country == rB.Country
                && rA.Variant == rB.Variant);
        }

        bool TextSimplePortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const TextSimplePortionPrimitive2D& rCompare = (TextSimplePortionPrimitive2D&)rPrimitive;

                return (getTextTransform() == rCompare.getTextTransform()
                    && getText() == rCompare.getText()
                    && getTextPosition() == rCompare.getTextPosition()
                    && getTextLength() == rCompare.getTextLength()
                    && getDXArray() == rCompare.getDXArray()
                    && getFontAttribute() == rCompare.getFontAttribute()
                    && LocalesAreEqual(getLocale(), rCompare.getLocale())
                    && getFontColor() == rCompare.getFontColor()
                    && mbFilled == rCompare.mbFilled
                    && mnWidthToFill == rCompare.mnWidthToFill);
            }

            return false;
        }

        basegfx::B2DRange TextSimplePortionPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(maB2DRange.isEmpty() && getTextLength())
            {
                
                
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;

                if(getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    
                    
                    
                    
                    const basegfx::B2DVector aFontScale(getCorrectedScaleAndFontScale(aScale));

                    
                    TextLayouterDevice aTextLayouter;
                    aTextLayouter.setFontAttribute(
                        getFontAttribute(),
                        aFontScale.getX(),
                        aFontScale.getY(),
                        getLocale());

                    
                    basegfx::B2DRange aNewRange(aTextLayouter.getTextBoundRect(getText(), getTextPosition(), getTextLength()));

                    
                    if(!aNewRange.isEmpty())
                    {
                        
                        const basegfx::B2DHomMatrix aRangeTransformation(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            aScale, fShearX, fRotate, aTranslate));

                        
                        aNewRange.transform(aRangeTransformation);

                        
                        const_cast< TextSimplePortionPrimitive2D* >(this)->maB2DRange = aNewRange;
                    }
                }
            }

            return maB2DRange;
        }

        
        ImplPrimitive2DIDBlock(TextSimplePortionPrimitive2D, PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
