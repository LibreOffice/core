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

#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <com/sun/star/drawing/LineCap.hpp>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        PolygonHairlinePrimitive2D::PolygonHairlinePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const basegfx::BColor& rBColor)
        :   BasePrimitive2D(),
            maPolygon(rPolygon),
            maBColor(rBColor)
        {
        }

        bool PolygonHairlinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PolygonHairlinePrimitive2D& rCompare = (PolygonHairlinePrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B2DRange PolygonHairlinePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            
            
            basegfx::B2DRange aRetval(getB2DPolygon().getB2DRange());

            if(!aRetval.isEmpty())
            {
                
                const basegfx::B2DVector aDiscreteSize(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
                const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

                if(basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
                {
                    aRetval.grow(fDiscreteHalfLineWidth);
                }
            }

            
            return aRetval;
        }

        
        ImplPrimitive2DIDBlock(PolygonHairlinePrimitive2D, PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D)

    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonMarkerPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            
            const basegfx::B2DVector aDashVector(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(getDiscreteDashLength(), 0.0));
            const double fLogicDashLength(aDashVector.getX());

            if(fLogicDashLength > 0.0 && !getRGBColorA().equal(getRGBColorB()))
            {
                
                ::std::vector< double > aDash;
                basegfx::B2DPolyPolygon aDashedPolyPolyA;
                basegfx::B2DPolyPolygon aDashedPolyPolyB;

                aDash.push_back(fLogicDashLength);
                aDash.push_back(fLogicDashLength);
                basegfx::tools::applyLineDashing(getB2DPolygon(), aDash, &aDashedPolyPolyA, &aDashedPolyPolyB, 2.0 * fLogicDashLength);

                
                Primitive2DSequence aRetval(2);

                aRetval[0] = Primitive2DReference(new PolyPolygonHairlinePrimitive2D(aDashedPolyPolyA, getRGBColorA()));
                aRetval[1] = Primitive2DReference(new PolyPolygonHairlinePrimitive2D(aDashedPolyPolyB, getRGBColorB()));

                return aRetval;
            }
            else
            {
                const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(getB2DPolygon(), getRGBColorA()));
                return Primitive2DSequence(&xRef, 1L);
            }
        }

        PolygonMarkerPrimitive2D::PolygonMarkerPrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const basegfx::BColor& rRGBColorA,
            const basegfx::BColor& rRGBColorB,
            double fDiscreteDashLength)
        :   BufferedDecompositionPrimitive2D(),
            maPolygon(rPolygon),
            maRGBColorA(rRGBColorA),
            maRGBColorB(rRGBColorB),
            mfDiscreteDashLength(fDiscreteDashLength),
            maLastInverseObjectToViewTransformation()
        {
        }

        bool PolygonMarkerPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const PolygonMarkerPrimitive2D& rCompare = (PolygonMarkerPrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getRGBColorA() == rCompare.getRGBColorA()
                    && getRGBColorB() == rCompare.getRGBColorB()
                    && getDiscreteDashLength() == rCompare.getDiscreteDashLength());
            }

            return false;
        }

        basegfx::B2DRange PolygonMarkerPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            
            
            basegfx::B2DRange aRetval(getB2DPolygon().getB2DRange());

            if(!aRetval.isEmpty())
            {
                
                const basegfx::B2DVector aDiscreteSize(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
                const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

                if(basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
                {
                    aRetval.grow(fDiscreteHalfLineWidth);
                }
            }

            
            return aRetval;
        }

        Primitive2DSequence PolygonMarkerPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            bool bNeedNewDecomposition(false);

            if(getBuffered2DDecomposition().hasElements())
            {
                if(rViewInformation.getInverseObjectToViewTransformation() != maLastInverseObjectToViewTransformation)
                {
                    bNeedNewDecomposition = true;
                }
            }

            if(bNeedNewDecomposition)
            {
                
                const_cast< PolygonMarkerPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                
                PolygonMarkerPrimitive2D* pThat = const_cast< PolygonMarkerPrimitive2D* >(this);
                pThat->maLastInverseObjectToViewTransformation = rViewInformation.getInverseObjectToViewTransformation();
            }

            
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }

        
        ImplPrimitive2DIDBlock(PolygonMarkerPrimitive2D, PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D)

    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonStrokePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getB2DPolygon().count())
            {
                
                const basegfx::B2DPolygon aB2DPolygon(basegfx::tools::simplifyCurveSegments(getB2DPolygon()));
                basegfx::B2DPolyPolygon aHairLinePolyPolygon;

                if(getStrokeAttribute().isDefault() || 0.0 == getStrokeAttribute().getFullDotDashLen())
                {
                    
                    aHairLinePolyPolygon.append(aB2DPolygon);
                }
                else
                {
                    
                    basegfx::tools::applyLineDashing(
                        aB2DPolygon, getStrokeAttribute().getDotDashArray(),
                        &aHairLinePolyPolygon, 0, getStrokeAttribute().getFullDotDashLen());
                }

                const sal_uInt32 nCount(aHairLinePolyPolygon.count());

                if(!getLineAttribute().isDefault() && getLineAttribute().getWidth())
                {
                    
                    const double fHalfLineWidth(getLineAttribute().getWidth() / 2.0);
                    const basegfx::B2DLineJoin aLineJoin(getLineAttribute().getLineJoin());
                    const com::sun::star::drawing::LineCap aLineCap(getLineAttribute().getLineCap());
                    basegfx::B2DPolyPolygon aAreaPolyPolygon;

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        
                        aAreaPolyPolygon.append(basegfx::tools::createAreaGeometry(
                            aHairLinePolyPolygon.getB2DPolygon(a),
                            fHalfLineWidth,
                            aLineJoin,
                            aLineCap));
                    }

                    
                    Primitive2DSequence aRetval(aAreaPolyPolygon.count());

                    
                    for(sal_uInt32 b(0L); b < aAreaPolyPolygon.count(); b++)
                    {
                        
                        
                        
                        const basegfx::B2DPolyPolygon aNewPolyPolygon(aAreaPolyPolygon.getB2DPolygon(b));
                        static bool bTestByUsingRandomColor(false);
                        const basegfx::BColor aColor(bTestByUsingRandomColor
                            ? basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)
                            : getLineAttribute().getColor());
                        const Primitive2DReference xRef(new PolyPolygonColorPrimitive2D(aNewPolyPolygon, aColor));
                        aRetval[b] = xRef;
                    }

                    return aRetval;
                }
                else
                {
                    
                    const Primitive2DReference xRef(
                        new PolyPolygonHairlinePrimitive2D(
                            aHairLinePolyPolygon,
                            getLineAttribute().getColor()));

                    return Primitive2DSequence(&xRef, 1);
                }
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute)
        :   BufferedDecompositionPrimitive2D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute(rStrokeAttribute)
        {
        }

        PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute)
        :   BufferedDecompositionPrimitive2D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute()
        {
        }

        bool PolygonStrokePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const PolygonStrokePrimitive2D& rCompare = (PolygonStrokePrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getLineAttribute() == rCompare.getLineAttribute()
                    && getStrokeAttribute() == rCompare.getStrokeAttribute());
            }

            return false;
        }

        basegfx::B2DRange PolygonStrokePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(getLineAttribute().getWidth())
            {
                bool bUseDecomposition(false);

                if(basegfx::B2DLINEJOIN_MITER == getLineAttribute().getLineJoin())
                {
                    
                    
                    bUseDecomposition = true;
                }

                if(!bUseDecomposition && com::sun::star::drawing::LineCap_SQUARE == getLineAttribute().getLineCap())
                {
                    
                    
                    
                    
                    
                    bUseDecomposition = true;
                }

                if(bUseDecomposition)
                {
                    
                    aRetval = BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
                }
                else
                {
                    
                    
                    aRetval = getB2DPolygon().getB2DRange();
                    aRetval.grow(getLineAttribute().getWidth() * 0.5);
                }
            }
            else
            {
                
                
                aRetval = getB2DPolygon().getB2DRange();

                if(!aRetval.isEmpty())
                {
                    
                    const basegfx::B2DVector aDiscreteSize(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
                    const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

                    if(basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
                    {
                        aRetval.grow(fDiscreteHalfLineWidth);
                    }
                }
            }

            return aRetval;
        }

        
        ImplPrimitive2DIDBlock(PolygonStrokePrimitive2D, PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D)

    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonWavePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            if(getB2DPolygon().count())
            {
                const bool bHasWidth(!basegfx::fTools::equalZero(getWaveWidth()));
                const bool bHasHeight(!basegfx::fTools::equalZero(getWaveHeight()));

                if(bHasWidth && bHasHeight)
                {
                    
                    const basegfx::B2DPolygon aWaveline(basegfx::tools::createWaveline(getB2DPolygon(), getWaveWidth(), getWaveHeight()));
                    const Primitive2DReference xRef(new PolygonStrokePrimitive2D(aWaveline, getLineAttribute(), getStrokeAttribute()));
                    aRetval = Primitive2DSequence(&xRef, 1);
                }
                else
                {
                    
                    const Primitive2DReference xRef(new PolygonStrokePrimitive2D(getB2DPolygon(), getLineAttribute(), getStrokeAttribute()));
                    aRetval = Primitive2DSequence(&xRef, 1);
                }
            }

            return aRetval;
        }

        PolygonWavePrimitive2D::PolygonWavePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute,
            double fWaveWidth,
            double fWaveHeight)
        :   PolygonStrokePrimitive2D(rPolygon, rLineAttribute, rStrokeAttribute),
            mfWaveWidth(fWaveWidth),
            mfWaveHeight(fWaveHeight)
        {
            if(mfWaveWidth < 0.0)
            {
                mfWaveWidth = 0.0;
            }

            if(mfWaveHeight < 0.0)
            {
                mfWaveHeight = 0.0;
            }
        }

        PolygonWavePrimitive2D::PolygonWavePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            double fWaveWidth,
            double fWaveHeight)
        :   PolygonStrokePrimitive2D(rPolygon, rLineAttribute),
            mfWaveWidth(fWaveWidth),
            mfWaveHeight(fWaveHeight)
        {
            if(mfWaveWidth < 0.0)
            {
                mfWaveWidth = 0.0;
            }

            if(mfWaveHeight < 0.0)
            {
                mfWaveHeight = 0.0;
            }
        }

        bool PolygonWavePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(PolygonStrokePrimitive2D::operator==(rPrimitive))
            {
                const PolygonWavePrimitive2D& rCompare = (PolygonWavePrimitive2D&)rPrimitive;

                return (getWaveWidth() == rCompare.getWaveWidth()
                    && getWaveHeight() == rCompare.getWaveHeight());
            }

            return false;
        }

        basegfx::B2DRange PolygonWavePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            
            basegfx::B2DRange aRetval(PolygonStrokePrimitive2D::getB2DRange(rViewInformation));

            
            if(basegfx::fTools::more(getWaveHeight(), 0.0))
            {
                aRetval.grow(getWaveHeight());
            }

            
            if(basegfx::fTools::more(getLineAttribute().getWidth(), 0.0))
            {
                aRetval.grow(getLineAttribute().getWidth() * 0.5);
            }

            return aRetval;
        }

        
        ImplPrimitive2DIDBlock(PolygonWavePrimitive2D, PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D)

    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonStrokeArrowPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            
            basegfx::B2DPolygon aLocalPolygon(getB2DPolygon());
            aLocalPolygon.removeDoublePoints();
            basegfx::B2DPolyPolygon aArrowA;
            basegfx::B2DPolyPolygon aArrowB;

            if(!aLocalPolygon.isClosed() && aLocalPolygon.count() > 1)
            {
                
                const double fPolyLength(basegfx::tools::getLength(aLocalPolygon));
                double fStart(0.0);
                double fEnd(0.0);
                double fStartOverlap(0.0);
                double fEndOverlap(0.0);

                if(!getStart().isDefault() && getStart().isActive())
                {
                    
                    aArrowA = basegfx::tools::createAreaGeometryForLineStartEnd(
                        aLocalPolygon, getStart().getB2DPolyPolygon(), true, getStart().getWidth(),
                        fPolyLength, getStart().isCentered() ? 0.5 : 0.0, &fStart);

                    
                    
                    fStartOverlap = getStart().getWidth() / 15.0;
                }

                if(!getEnd().isDefault() && getEnd().isActive())
                {
                    
                    aArrowB = basegfx::tools::createAreaGeometryForLineStartEnd(
                        aLocalPolygon, getEnd().getB2DPolyPolygon(), false, getEnd().getWidth(),
                        fPolyLength, getEnd().isCentered() ? 0.5 : 0.0, &fEnd);

                    
                    fEndOverlap = getEnd().getWidth() / 15.0;
                }

                if(0.0 != fStart || 0.0 != fEnd)
                {
                    
                    aLocalPolygon = basegfx::tools::getSnippetAbsolute(aLocalPolygon, fStart-fStartOverlap, fPolyLength - fEnd + fEndOverlap, fPolyLength);
                }
            }

            
            Primitive2DSequence aRetval(1L + (aArrowA.count() ? 1L : 0L) + (aArrowB.count() ? 1L : 0L));
            sal_uInt32 nInd(0L);

            
            const Primitive2DReference xRefShaft(new
                PolygonStrokePrimitive2D(
                    aLocalPolygon, getLineAttribute(), getStrokeAttribute()));
            aRetval[nInd++] = xRefShaft;

            if(aArrowA.count())
            {
                const Primitive2DReference xRefA(
                    new PolyPolygonColorPrimitive2D(
                        aArrowA, getLineAttribute().getColor()));
                aRetval[nInd++] = xRefA;
            }

            if(aArrowB.count())
            {
                const Primitive2DReference xRefB(
                    new PolyPolygonColorPrimitive2D(
                        aArrowB, getLineAttribute().getColor()));
                aRetval[nInd++] = xRefB;
            }

            return aRetval;
        }

        PolygonStrokeArrowPrimitive2D::PolygonStrokeArrowPrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute,
            const attribute::LineStartEndAttribute& rStart,
            const attribute::LineStartEndAttribute& rEnd)
        :   PolygonStrokePrimitive2D(rPolygon, rLineAttribute, rStrokeAttribute),
            maStart(rStart),
            maEnd(rEnd)
        {
        }

        PolygonStrokeArrowPrimitive2D::PolygonStrokeArrowPrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::LineStartEndAttribute& rStart,
            const attribute::LineStartEndAttribute& rEnd)
        :   PolygonStrokePrimitive2D(rPolygon, rLineAttribute),
            maStart(rStart),
            maEnd(rEnd)
        {
        }

        bool PolygonStrokeArrowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(PolygonStrokePrimitive2D::operator==(rPrimitive))
            {
                const PolygonStrokeArrowPrimitive2D& rCompare = (PolygonStrokeArrowPrimitive2D&)rPrimitive;

                return (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd());
            }

            return false;
        }

        basegfx::B2DRange PolygonStrokeArrowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(getStart().isActive() || getEnd().isActive())
            {
                
                return BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
            }
            else
            {
                
                return PolygonStrokePrimitive2D::getB2DRange(rViewInformation);
            }
        }

        
        ImplPrimitive2DIDBlock(PolygonStrokeArrowPrimitive2D, PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
