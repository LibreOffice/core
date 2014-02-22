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

#include "vclpixelprocessor2d.hxx"
#include <vcl/outdev.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <com/sun/star/awt/XWindow2.hpp>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <helperwrongspellrenderer.hxx>
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/hatch.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/awt/PosSize.hpp>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <cstdio>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <svtools/borderhelper.hxx>

#include <com/sun/star/table/BorderLineStyle.hpp>



using namespace com::sun::star;

namespace {

basegfx::B2DPolygon makeRectPolygon( double fX, double fY, double fW, double fH )
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(fX, fY));
    aPoly.append(basegfx::B2DPoint(fX+fW, fY));
    aPoly.append(basegfx::B2DPoint(fX+fW, fY+fH));
    aPoly.append(basegfx::B2DPoint(fX, fY+fH));
    aPoly.setClosed(true);
    return aPoly;
}

void drawHairLine(
    OutputDevice* pOutDev, double fX1, double fY1, double fX2, double fY2,
    const basegfx::BColor& rColor )
{
    basegfx::B2DPolygon aTarget;
    aTarget.append(basegfx::B2DPoint(fX1, fY1));
    aTarget.append(basegfx::B2DPoint(fX2, fY2));

    pOutDev->SetFillColor();
    pOutDev->SetLineColor(Color(rColor));
    pOutDev->DrawPolyLine(aTarget);
}

}

namespace drawinglayer
{
    namespace processor2d
    {
        struct VclPixelProcessor2D::Impl
        {
            sal_uInt16 m_nOrigAntiAliasing;

            explicit Impl(OutputDevice const& rOutDev)
                : m_nOrigAntiAliasing(rOutDev.GetAntialiasing())
            { }
        };

        VclPixelProcessor2D::VclPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation, OutputDevice& rOutDev)
            :   VclProcessor2D(rViewInformation, rOutDev)
            ,   m_pImpl(new Impl(rOutDev))
        {
            
            maCurrentTransformation = rViewInformation.getObjectToViewTransformation();

            
               mpOutputDevice->Push(PUSH_MAPMODE);
            mpOutputDevice->SetMapMode();

            
            if(getOptionsDrawinglayer().IsAntiAliasing())
            {
                mpOutputDevice->SetAntialiasing(
                   m_pImpl->m_nOrigAntiAliasing | ANTIALIASING_ENABLE_B2DDRAW);
            }
            else
            {
                mpOutputDevice->SetAntialiasing(
                   m_pImpl->m_nOrigAntiAliasing & ~ANTIALIASING_ENABLE_B2DDRAW);
            }
        }

        VclPixelProcessor2D::~VclPixelProcessor2D()
        {
            
               mpOutputDevice->Pop();

            
            mpOutputDevice->SetAntialiasing(m_pImpl->m_nOrigAntiAliasing);
        }

        bool VclPixelProcessor2D::tryDrawPolyPolygonColorPrimitive2DDirect(const drawinglayer::primitive2d::PolyPolygonColorPrimitive2D& rSource, double fTransparency)
        {
            basegfx::B2DPolyPolygon aLocalPolyPolygon(rSource.getB2DPolyPolygon());

            if(!aLocalPolyPolygon.count())
            {
                
                return true;
            }

            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rSource.getBColor()));

            mpOutputDevice->SetFillColor(Color(aPolygonColor));
            mpOutputDevice->SetLineColor();
            aLocalPolyPolygon.transform(maCurrentTransformation);
            mpOutputDevice->DrawTransparent(
                aLocalPolyPolygon,
                fTransparency);

            return true;
        }

        bool VclPixelProcessor2D::tryDrawPolygonHairlinePrimitive2DDirect(const drawinglayer::primitive2d::PolygonHairlinePrimitive2D& rSource, double fTransparency)
        {
            basegfx::B2DPolygon aLocalPolygon(rSource.getB2DPolygon());

            if(!aLocalPolygon.count())
            {
                
                return true;
            }

            const basegfx::BColor aLineColor(maBColorModifierStack.getModifiedColor(rSource.getBColor()));

            mpOutputDevice->SetFillColor();
            mpOutputDevice->SetLineColor(Color(aLineColor));
            aLocalPolygon.transform(maCurrentTransformation);

            
            if(mpOutputDevice->TryDrawPolyLineDirect(
                aLocalPolygon,
                0.0,
                fTransparency))
            {
                return true;
            }

            return false;
        }

        bool VclPixelProcessor2D::tryDrawPolygonStrokePrimitive2DDirect(const drawinglayer::primitive2d::PolygonStrokePrimitive2D& rSource, double fTransparency)
        {
            basegfx::B2DPolygon aLocalPolygon(rSource.getB2DPolygon());

            if(!aLocalPolygon.count())
            {
                
                return true;
            }

            aLocalPolygon = basegfx::tools::simplifyCurveSegments(aLocalPolygon);
            basegfx::B2DPolyPolygon aHairLinePolyPolygon;

            if(rSource.getStrokeAttribute().isDefault() || 0.0 == rSource.getStrokeAttribute().getFullDotDashLen())
            {
                
                aHairLinePolyPolygon.append(aLocalPolygon);
            }
            else
            {
                
                basegfx::tools::applyLineDashing(
                    aLocalPolygon,
                    rSource.getStrokeAttribute().getDotDashArray(),
                    &aHairLinePolyPolygon,
                    0,
                    rSource.getStrokeAttribute().getFullDotDashLen());
            }

            if(!aHairLinePolyPolygon.count())
            {
                
                return true;
            }

            const basegfx::BColor aLineColor(
                maBColorModifierStack.getModifiedColor(
                    rSource.getLineAttribute().getColor()));

            mpOutputDevice->SetFillColor();
            mpOutputDevice->SetLineColor(Color(aLineColor));
            aHairLinePolyPolygon.transform(maCurrentTransformation);

            double fLineWidth(rSource.getLineAttribute().getWidth());

            if(basegfx::fTools::more(fLineWidth, 0.0))
            {
                basegfx::B2DVector aLineWidth(fLineWidth, 0.0);

                aLineWidth = maCurrentTransformation * aLineWidth;
                fLineWidth = aLineWidth.getLength();
            }

            bool bHasPoints(false);
            bool bTryWorked(false);

            for(sal_uInt32 a(0); a < aHairLinePolyPolygon.count(); a++)
            {
                const basegfx::B2DPolygon aSingle(aHairLinePolyPolygon.getB2DPolygon(a));

                if(aSingle.count())
                {
                    bHasPoints = true;

                    if(mpOutputDevice->TryDrawPolyLineDirect(
                        aSingle,
                        fLineWidth,
                        fTransparency,
                        rSource.getLineAttribute().getLineJoin(),
                        rSource.getLineAttribute().getLineCap()))
                    {
                        bTryWorked = true;
                    }
                }
            }

            if(!bTryWorked && !bHasPoints)
            {
                
                bTryWorked = true;
            }

            return bTryWorked;
        }

        bool VclPixelProcessor2D::tryDrawBorderLinePrimitive2DDirect(
            const drawinglayer::primitive2d::BorderLinePrimitive2D& rSource)
        {
            const basegfx::B2DPoint& rS = rSource.getStart();
            const basegfx::B2DPoint& rE = rSource.getEnd();

            double fX1 = rS.getX();
            double fY1 = rS.getY();
            double fX2 = rE.getX();
            double fY2 = rE.getY();

            bool bHorizontal = false;
            if (fX1 == fX2)
            {
                
            }
            else if (fY1 == fY2)
            {
                
                bHorizontal = true;
            }
            else
                
                return false;

            switch (rSource.getStyle())
            {
                case table::BorderLineStyle::SOLID:
                case table::BorderLineStyle::DOUBLE:
                {
                    const basegfx::BColor aLineColor =
                        maBColorModifierStack.getModifiedColor(rSource.getRGBColorLeft());
                    double nThick = rtl::math::round(rSource.getLeftWidth());

                    bool bDouble = rSource.getStyle() == table::BorderLineStyle::DOUBLE;

                    basegfx::B2DPolygon aTarget;

                    if (bHorizontal)
                    {
                        

                        aTarget = makeRectPolygon(fX1, fY1, fX2-fX1, nThick);
                        aTarget.transform(maCurrentTransformation);

                        basegfx::B2DRange aRange = aTarget.getB2DRange();
                        double fH = aRange.getHeight();

                        if (bDouble)
                        {
                            
                            drawHairLine(
                                mpOutputDevice, aRange.getMinX(), aRange.getMinY()-1.0, aRange.getMaxX(), aRange.getMinY()-1.0,
                                aLineColor);

                            drawHairLine(
                                mpOutputDevice, aRange.getMinX(), aRange.getMinY()+1.0, aRange.getMaxX(), aRange.getMinY()+1.0,
                                aLineColor);

                            return true;
                        }

                        if (fH <= 1.0)
                        {
                            
                            drawHairLine(
                                mpOutputDevice, aRange.getMinX(), aRange.getMinY(), aRange.getMaxX(), aRange.getMinY(),
                                aLineColor);

                            return true;
                        }

                        double fOffset = rtl::math::round(fH/2.0, 0, rtl_math_RoundingMode_Down);
                        if (fOffset != 0.0)
                        {
                            
                            basegfx::B2DHomMatrix aMat;
                            aMat.set(1, 2, -fOffset);
                            aTarget.transform(aMat);
                        }
                    }
                    else
                    {
                        

                        aTarget = makeRectPolygon(fX1, fY1, nThick, fY2-fY1);
                        aTarget.transform(maCurrentTransformation);

                        basegfx::B2DRange aRange = aTarget.getB2DRange();
                        double fW = aRange.getWidth();

                        if (bDouble)
                        {
                            
                            drawHairLine(
                                mpOutputDevice, aRange.getMinX()-1.0, aRange.getMinY(), aRange.getMinX()-1.0, aRange.getMaxY(),
                                aLineColor);

                            drawHairLine(
                                mpOutputDevice, aRange.getMinX()+1.0, aRange.getMinY(), aRange.getMinX()+1.0, aRange.getMaxY(),
                                aLineColor);

                            return true;
                        }

                        if (fW <= 1.0)
                        {
                            
                            drawHairLine(
                                mpOutputDevice, aRange.getMinX(), aRange.getMinY(), aRange.getMinX(), aRange.getMaxY(),
                                aLineColor);

                            return true;
                        }

                        double fOffset = rtl::math::round(fW/2.0, 0, rtl_math_RoundingMode_Down);
                        if (fOffset != 0.0)
                        {
                            
                            basegfx::B2DHomMatrix aMat;
                            aMat.set(0, 2, -fOffset);
                            aTarget.transform(aMat);
                        }
                    }

                    mpOutputDevice->SetFillColor(Color(aLineColor));
                    mpOutputDevice->SetLineColor();
                    mpOutputDevice->DrawPolygon(aTarget);
                    return true;
                }
                break;
                case table::BorderLineStyle::DOTTED:
                case table::BorderLineStyle::DASHED:
                case table::BorderLineStyle::FINE_DASHED:
                {
                    std::vector<double> aPattern =
                        svtools::GetLineDashing(rSource.getStyle(), rSource.getPatternScale()*10.0);

                    if (aPattern.empty())
                        
                        return false;

                    double nThick = rtl::math::round(rSource.getLeftWidth());

                    const basegfx::BColor aLineColor =
                        maBColorModifierStack.getModifiedColor(rSource.getRGBColorLeft());

                    
                    basegfx::B2DRange aRange(fX1, fY1, fX2, fY2);
                    aRange.transform(maCurrentTransformation);
                    fX1 = aRange.getMinX();
                    fX2 = aRange.getMaxX();
                    fY1 = aRange.getMinY();
                    fY2 = aRange.getMaxY();

                    basegfx::B2DPolyPolygon aTarget;

                    if (bHorizontal)
                    {
                        

                        if (basegfx::fTools::equalZero(nThick))
                        {
                            
                            drawHairLine(mpOutputDevice, fX1, fY1, fX2, fY1, aLineColor);
                            return true;
                        }

                        
                        basegfx::B2DPolyPolygon aDashes;
                        std::vector<double>::const_iterator it = aPattern.begin(), itEnd = aPattern.end();
                        for (; it != itEnd; ++it)
                            aDashes.append(makeRectPolygon(0, 0, *it, nThick));

                        aDashes.transform(maCurrentTransformation);
                        rtl::math::setNan(&nThick);

                        
                        
                        basegfx::B2DPolyPolygon aDashesPix;

                        for (sal_uInt32 i = 0, n = aDashes.count(); i < n; ++i)
                        {
                            basegfx::B2DPolygon aPoly = aDashes.getB2DPolygon(i);
                            aRange = aPoly.getB2DRange();
                            double fW = rtl::math::round(aRange.getWidth());
                            if (basegfx::fTools::equalZero(fW))
                            {
                                
                                drawHairLine(mpOutputDevice, fX1, fY1, fX2, fY1, aLineColor);
                                return true;
                            }

                            if (rtl::math::isNan(nThick))
                                nThick = rtl::math::round(aRange.getHeight());

                            aDashesPix.append(makeRectPolygon(0, 0, fW, nThick));
                        }

                        
                        double fX = fX1;
                        bool bLine = true;
                        sal_uInt32 i = 0, n = aDashesPix.count();
                        while (fX <= fX2)
                        {
                            basegfx::B2DPolygon aPoly = aDashesPix.getB2DPolygon(i);
                            aRange = aPoly.getB2DRange();
                            if (bLine)
                            {
                                double fBlockW = aRange.getWidth();
                                if (fX + fBlockW > fX2)
                                    
                                    fBlockW = fX2 - fX + 1;

                                double fH = aRange.getHeight();
                                if (basegfx::fTools::equalZero(fH))
                                    fH = 1.0;

                                aTarget.append(makeRectPolygon(fX, fY1, fBlockW, fH));
                            }

                            bLine = !bLine; 
                            fX += aRange.getWidth();

                            ++i;
                            if (i >= n)
                                i = 0;
                        }

                        double fOffset = rtl::math::round(nThick/2.0, 0, rtl_math_RoundingMode_Down);
                        if (fOffset != 0.0)
                        {
                            
                            basegfx::B2DHomMatrix aMat;
                            aMat.set(1, 2, -fOffset);
                            aTarget.transform(aMat);
                        }
                    }
                    else
                    {
                        

                        if (basegfx::fTools::equalZero(nThick))
                        {
                            
                            drawHairLine(mpOutputDevice, fX1, fY1, fX1, fY2, aLineColor);
                            return true;
                        }

                        
                        basegfx::B2DPolyPolygon aDashes;
                        std::vector<double>::const_iterator it = aPattern.begin(), itEnd = aPattern.end();
                        for (; it != itEnd; ++it)
                            aDashes.append(makeRectPolygon(0, 0, nThick, *it));

                        aDashes.transform(maCurrentTransformation);
                        rtl::math::setNan(&nThick);

                        
                        
                        basegfx::B2DPolyPolygon aDashesPix;

                        for (sal_uInt32 i = 0, n = aDashes.count(); i < n; ++i)
                        {
                            basegfx::B2DPolygon aPoly = aDashes.getB2DPolygon(i);
                            aRange = aPoly.getB2DRange();
                            double fH = rtl::math::round(aRange.getHeight());
                            if (basegfx::fTools::equalZero(fH))
                            {
                                
                                drawHairLine(mpOutputDevice, fX1, fY1, fX1, fY2, aLineColor);
                                return true;
                            }

                            if (rtl::math::isNan(nThick))
                                nThick = rtl::math::round(aRange.getWidth());

                            aDashesPix.append(makeRectPolygon(0, 0, nThick, fH));
                        }

                        
                        double fY = fY1;
                        bool bLine = true;
                        sal_uInt32 i = 0, n = aDashesPix.count();
                        while (fY <= fY2)
                        {
                            basegfx::B2DPolygon aPoly = aDashesPix.getB2DPolygon(i);
                            aRange = aPoly.getB2DRange();
                            if (bLine)
                            {
                                double fBlockH = aRange.getHeight();
                                if (fY + fBlockH > fY2)
                                    
                                    fBlockH = fY2 - fY + 1;

                                double fW = aRange.getWidth();
                                if (basegfx::fTools::equalZero(fW))
                                    fW = 1.0;

                                aTarget.append(makeRectPolygon(fX1, fY, fW, fBlockH));
                            }

                            bLine = !bLine; 
                            fY += aRange.getHeight();

                            ++i;
                            if (i >= n)
                                i = 0;
                        }

                        double fOffset = rtl::math::round(nThick/2.0, 0, rtl_math_RoundingMode_Down);
                        if (fOffset != 0.0)
                        {
                            
                            basegfx::B2DHomMatrix aMat;
                            aMat.set(0, 2, -fOffset);
                            aTarget.transform(aMat);
                        }
                    }

                    mpOutputDevice->SetFillColor(Color(aLineColor));
                    mpOutputDevice->SetLineColor();
                    mpOutputDevice->DrawPolyPolygon(aTarget);

                    return true;
                }
                break;
                default:
                    ;
            }
            return false;
        }

        void VclPixelProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                {
                    
                    static bool bHandleWrongSpellDirectly(true);

                    if(bHandleWrongSpellDirectly)
                    {
                        const primitive2d::WrongSpellPrimitive2D& rWrongSpellPrimitive = static_cast< const primitive2d::WrongSpellPrimitive2D& >(rCandidate);

                        if(!renderWrongSpellPrimitive2D(
                            rWrongSpellPrimitive,
                            *mpOutputDevice,
                            maCurrentTransformation,
                            maBColorModifierStack))
                        {
                            
                            process(rWrongSpellPrimitive.get2DDecomposition(getViewInformation2D()));
                        }
                    }
                    else
                    {
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                {
                    
                    static bool bForceSimpleTextDecomposition(false);

                    
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptTextToFillDrawMode();

                    if(!bForceSimpleTextDecomposition && getOptionsDrawinglayer().IsRenderSimpleTextDirect())
                    {
                        RenderTextSimpleOrDecoratedPortionPrimitive2D(static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(rCandidate));
                    }
                    else
                    {
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }

                    
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    
                    static bool bForceComplexTextDecomposition(false);

                    
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptTextToFillDrawMode();

                    if(!bForceComplexTextDecomposition && getOptionsDrawinglayer().IsRenderDecoratedTextDirect())
                    {
                        RenderTextSimpleOrDecoratedPortionPrimitive2D(static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(rCandidate));
                    }
                    else
                    {
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }

                    
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    
                    const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D = static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate);
                    static bool bAllowed(true);

                    if(bAllowed && tryDrawPolygonHairlinePrimitive2DDirect(rPolygonHairlinePrimitive2D, 0.0))
                    {
                        break;
                    }

                    
                    RenderPolygonHairlinePrimitive2D(rPolygonHairlinePrimitive2D, true);
                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    
                    const primitive2d::BitmapPrimitive2D& rBitmapCandidate = static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate);

                    
                    const basegfx::B2DRange& rDiscreteViewPort(getViewInformation2D().getDiscreteViewport());
                    const basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rBitmapCandidate.getTransform());

                    if(!rDiscreteViewPort.isEmpty())
                    {
                        basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);

                        aUnitRange.transform(aLocalTransform);

                        if(!aUnitRange.overlaps(rDiscreteViewPort))
                        {
                            
                            break;
                        }
                    }

                    RenderBitmapPrimitive2D(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D :
                {
                    
                    RenderFillGraphicPrimitive2D(static_cast< const primitive2d::FillGraphicPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D :
                {
                    
                    const primitive2d::PolyPolygonGradientPrimitive2D& rPolygonCandidate = static_cast< const primitive2d::PolyPolygonGradientPrimitive2D& >(rCandidate);
                    const attribute::FillGradientAttribute& rGradient(rPolygonCandidate.getFillGradient());
                    basegfx::BColor aStartColor(maBColorModifierStack.getModifiedColor(rGradient.getStartColor()));
                    basegfx::BColor aEndColor(maBColorModifierStack.getModifiedColor(rGradient.getEndColor()));
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());

                    if(aLocalPolyPolygon.count())
                    {
                        aLocalPolyPolygon.transform(maCurrentTransformation);

                        if(aStartColor == aEndColor)
                        {
                            
                            mpOutputDevice->SetLineColor();
                            mpOutputDevice->SetFillColor(Color(aStartColor));
                            mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
                        }
                        else
                        {
                            
                            process(rPolygonCandidate.get2DDecomposition(getViewInformation2D()));
                        }
                    }
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D :
                {
                    
                    RenderPolyPolygonGraphicPrimitive2D(static_cast< const primitive2d::PolyPolygonGraphicPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    
                    const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D = static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate);
                    basegfx::B2DPolyPolygon aLocalPolyPolygon;
                    static bool bAllowed(true);

                    if(bAllowed && tryDrawPolyPolygonColorPrimitive2DDirect(rPolyPolygonColorPrimitive2D, 0.0))
                    {
                        
                    }
                    else
                    {
                        
                        const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPolyPolygonColorPrimitive2D.getBColor()));

                        mpOutputDevice->SetFillColor(Color(aPolygonColor));
                        mpOutputDevice->SetLineColor();
                        aLocalPolyPolygon = rPolyPolygonColorPrimitive2D.getB2DPolyPolygon();
                        aLocalPolyPolygon.transform(maCurrentTransformation);
                        mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
                    }

                    
                    
                    
                    if(mnPolygonStrokePrimitive2D
                        && getOptionsDrawinglayer().IsAntiAliasing()
                        && (mpOutputDevice->GetAntialiasing() & ANTIALIASING_ENABLE_B2DDRAW))
                    {
                        const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPolyPolygonColorPrimitive2D.getBColor()));
                        sal_uInt32 nCount(aLocalPolyPolygon.count());

                        if(!nCount)
                        {
                            aLocalPolyPolygon = rPolyPolygonColorPrimitive2D.getB2DPolyPolygon();
                            aLocalPolyPolygon.transform(maCurrentTransformation);
                            nCount = aLocalPolyPolygon.count();
                        }

                        mpOutputDevice->SetFillColor();
                        mpOutputDevice->SetLineColor(Color(aPolygonColor));

                        for(sal_uInt32 a(0); a < nCount; a++)
                        {
                            mpOutputDevice->DrawPolyLine(aLocalPolyPolygon.getB2DPolygon(a), 0.0);
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    
                    const bool bForceLineSnap(getOptionsDrawinglayer().IsAntiAliasing() && getOptionsDrawinglayer().IsSnapHorVerLinesToDiscrete());
                    const sal_uInt16 nOldAntiAliase(mpOutputDevice->GetAntialiasing());

                    if(bForceLineSnap)
                    {
                        mpOutputDevice->SetAntialiasing(nOldAntiAliase | ANTIALIASING_PIXELSNAPHAIRLINE);
                    }

                    const primitive2d::MetafilePrimitive2D& rMetafilePrimitive( static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate) );

                    static bool bTestMetaFilePrimitiveDecomposition( true );
                    if( bTestMetaFilePrimitiveDecomposition && !rMetafilePrimitive.getMetaFile().GetUseCanvas() )
                    {
                        
                        
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    else
                    {
                        
                        RenderMetafilePrimitive2D( rMetafilePrimitive );
                    }

                    if(bForceLineSnap)
                    {
                        mpOutputDevice->SetAntialiasing(nOldAntiAliase);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    
                    RenderMaskPrimitive2DPixel(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    
                    RenderModifiedColorPrimitive2D(static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D :
                {
                    
                    
                    const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate = static_cast< const primitive2d::UnifiedTransparencePrimitive2D& >(rCandidate);
                    const primitive2d::Primitive2DSequence rContent = rUniTransparenceCandidate.getChildren();

                    if(rContent.hasElements())
                    {
                        if(0.0 == rUniTransparenceCandidate.getTransparence())
                        {
                            
                            process(rUniTransparenceCandidate.getChildren());
                        }
                        else if(rUniTransparenceCandidate.getTransparence() > 0.0 && rUniTransparenceCandidate.getTransparence() < 1.0)
                        {
                            bool bDrawTransparentUsed(false);

                            
                            
                            static bool bAllowUsingDrawTransparent(true);

                            if(bAllowUsingDrawTransparent && 1 == rContent.getLength())
                            {
                                const primitive2d::Primitive2DReference xReference(rContent[0]);
                                const primitive2d::BasePrimitive2D* pBasePrimitive = dynamic_cast< const primitive2d::BasePrimitive2D* >(xReference.get());

                                if(pBasePrimitive)
                                {
                                    switch(pBasePrimitive->getPrimitive2DID())
                                    {
                                        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
                                        {
                                            
                                            const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor = static_cast< const primitive2d::PolyPolygonColorPrimitive2D* >(pBasePrimitive);
                                            OSL_ENSURE(pPoPoColor, "OOps, PrimitiveID and PrimitiveType do not match (!)");
                                            bDrawTransparentUsed = tryDrawPolyPolygonColorPrimitive2DDirect(*pPoPoColor, rUniTransparenceCandidate.getTransparence());
                                            break;
                                        }
                                        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
                                        {
                                            
                                            const primitive2d::PolygonHairlinePrimitive2D* pPoHair = static_cast< const primitive2d::PolygonHairlinePrimitive2D* >(pBasePrimitive);
                                            OSL_ENSURE(pPoHair, "OOps, PrimitiveID and PrimitiveType do not match (!)");

                                            
                                            
                                            
                                            
                                            
                                            static bool bAllowed(false);

                                            bDrawTransparentUsed = bAllowed && tryDrawPolygonHairlinePrimitive2DDirect(*pPoHair, rUniTransparenceCandidate.getTransparence());
                                            break;
                                        }
                                        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                                        {
                                            
                                            const primitive2d::PolygonStrokePrimitive2D* pPoStroke = static_cast< const primitive2d::PolygonStrokePrimitive2D* >(pBasePrimitive);
                                            OSL_ENSURE(pPoStroke, "OOps, PrimitiveID and PrimitiveType do not match (!)");

                                            
                                            
                                            
                                            
                                            
                                            static bool bAllowed(false);

                                            bDrawTransparentUsed = bAllowed && tryDrawPolygonStrokePrimitive2DDirect(*pPoStroke, rUniTransparenceCandidate.getTransparence());
                                            break;
                                        }
                                    }
                                }
                            }

                            if(!bDrawTransparentUsed)
                            {
                                
                                RenderUnifiedTransparencePrimitive2D(rUniTransparenceCandidate);
                            }
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                {
                    
                    RenderTransparencePrimitive2D(static_cast< const primitive2d::TransparencePrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    
                    RenderTransformPrimitive2D(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D :
                {
                    
                    RenderPagePreviewPrimitive2D(static_cast< const primitive2d::PagePreviewPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                {
                    
                    RenderMarkerArrayPrimitive2D(static_cast< const primitive2d::MarkerArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    
                    RenderPointArrayPrimitive2D(static_cast< const primitive2d::PointArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                {
                    
                    const primitive2d::ControlPrimitive2D& rControlPrimitive = static_cast< const primitive2d::ControlPrimitive2D& >(rCandidate);
                    const uno::Reference< awt::XControl >& rXControl(rControlPrimitive.getXControl());

                    try
                    {
                        
                        uno::Reference< awt::XView > xControlView(rXControl, uno::UNO_QUERY_THROW);
                        const uno::Reference< awt::XGraphics > xOriginalGraphics(xControlView->getGraphics());
                        const uno::Reference< awt::XGraphics > xNewGraphics(mpOutputDevice->CreateUnoGraphics());

                        if(xNewGraphics.is())
                        {
                            
                            xControlView->setGraphics(xNewGraphics);

                            
                            const basegfx::B2DHomMatrix aObjectToPixel(maCurrentTransformation * rControlPrimitive.getTransform());
                            const basegfx::B2DPoint aTopLeftPixel(aObjectToPixel * basegfx::B2DPoint(0.0, 0.0));

                            
                            
                            uno::Reference< awt::XWindow2 > xControlWindow(rXControl, uno::UNO_QUERY_THROW);
                            const bool bControlIsVisibleAsChildWindow(rXControl->getPeer().is() && xControlWindow->isVisible());

                            if(!bControlIsVisibleAsChildWindow)
                            {
                                
                                
                                const Point aOrigin(mpOutputDevice->GetMapMode().GetOrigin());
                                xControlView->draw(
                                    aOrigin.X() + basegfx::fround(aTopLeftPixel.getX()),
                                    aOrigin.Y() + basegfx::fround(aTopLeftPixel.getY()));
                            }

                            
                            xControlView->setGraphics(xOriginalGraphics);
                        }
                    }
                    catch(const uno::Exception&)
                    {
                        
                        
                        

                        
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                {
                    
                    const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokePrimitive2D = static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate);

                    if(tryDrawPolygonStrokePrimitive2DDirect(rPolygonStrokePrimitive2D, 0.0))
                    {
                        break;
                    }

                    
                    
                    
                    
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptLineToFillDrawMode();

                    
                    static bool bSuppressFatToHairlineCorrection(false);

                    if(bSuppressFatToHairlineCorrection)
                    {
                        
                        
                        mnPolygonStrokePrimitive2D++;

                        
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));

                        
                        mnPolygonStrokePrimitive2D--;
                    }
                    else
                    {
                        
                        
                        
                        
                        RenderPolygonStrokePrimitive2D(rPolygonStrokePrimitive2D);
                    }

                    
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D :
                {
                    static bool bForceIgnoreHatchSmoothing(false);

                    if(bForceIgnoreHatchSmoothing || getOptionsDrawinglayer().IsAntiAliasing())
                    {
                        
                        
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    else
                    {
                        
                        
                        
                        
                        const primitive2d::FillHatchPrimitive2D& rFillHatchPrimitive = static_cast< const primitive2d::FillHatchPrimitive2D& >(rCandidate);
                        const attribute::FillHatchAttribute& rFillHatchAttributes = rFillHatchPrimitive.getFillHatch();

                        
                        basegfx::B2DRange aHatchRange(rFillHatchPrimitive.getObjectRange());
                        aHatchRange.transform(maCurrentTransformation);
                        const basegfx::B2DPolygon aHatchPolygon(basegfx::tools::createPolygonFromRect(aHatchRange));

                        if(rFillHatchAttributes.isFillBackground())
                        {
                            
                            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rFillHatchPrimitive.getBColor()));

                            mpOutputDevice->SetFillColor(Color(aPolygonColor));
                            mpOutputDevice->SetLineColor();
                            mpOutputDevice->DrawPolygon(aHatchPolygon);
                        }

                        
                        const basegfx::BColor aHatchColor(maBColorModifierStack.getModifiedColor(rFillHatchPrimitive.getBColor()));
                        mpOutputDevice->SetFillColor();
                        mpOutputDevice->SetLineColor(Color(aHatchColor));

                        
                        HatchStyle eHatchStyle(HATCH_SINGLE);

                        switch(rFillHatchAttributes.getStyle())
                        {
                            default : 
                            {
                                break;
                            }
                            case attribute::HATCHSTYLE_DOUBLE :
                            {
                                eHatchStyle = HATCH_DOUBLE;
                                break;
                            }
                            case attribute::HATCHSTYLE_TRIPLE :
                            {
                                eHatchStyle = HATCH_TRIPLE;
                                break;
                            }
                        }

                        
                        const basegfx::B2DVector aDiscreteDistance(maCurrentTransformation * basegfx::B2DVector(rFillHatchAttributes.getDistance(), 0.0));
                        const sal_uInt32 nDistance(basegfx::fround(aDiscreteDistance.getLength()));
                        const sal_uInt16 nAngle10((sal_uInt16)basegfx::fround(rFillHatchAttributes.getAngle() / F_PI1800));
                        ::Hatch aVCLHatch(eHatchStyle, Color(rFillHatchAttributes.getColor()), nDistance, nAngle10);

                        
                        mpOutputDevice->DrawHatch(PolyPolygon(Polygon(aHatchPolygon)), aVCLHatch);
                    }
                    break;
                }
                case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D :
                {
                    
                    const primitive2d::BackgroundColorPrimitive2D& rPrimitive = static_cast< const primitive2d::BackgroundColorPrimitive2D& >(rCandidate);
                    const sal_uInt16 nOriginalAA(mpOutputDevice->GetAntialiasing());

                    
                    mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() & ~ANTIALIASING_ENABLE_B2DDRAW);

                    
                    const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPrimitive.getBColor()));
                    mpOutputDevice->SetFillColor(Color(aPolygonColor));
                    mpOutputDevice->SetLineColor();

                    
                    const basegfx::B2DRange& aViewport(getViewInformation2D().getDiscreteViewport());
                    const Rectangle aRectangle(
                        (sal_Int32)floor(aViewport.getMinX()), (sal_Int32)floor(aViewport.getMinY()),
                        (sal_Int32)ceil(aViewport.getMaxX()), (sal_Int32)ceil(aViewport.getMaxY()));
                    mpOutputDevice->DrawRect(aRectangle);

                    
                    mpOutputDevice->SetAntialiasing(nOriginalAA);
                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D :
                {
                    
                    
                    
                    
                    
                    
                    
                    break;
                }
                case PRIMITIVE2D_ID_INVERTPRIMITIVE2D :
                {
                    
                    
                    mpOutputDevice->Push();
                    mpOutputDevice->SetRasterOp( ROP_XOR );
                    const sal_uInt16 nAntiAliasing(mpOutputDevice->GetAntialiasing());
                    mpOutputDevice->SetAntialiasing(nAntiAliasing & ~ANTIALIASING_ENABLE_B2DDRAW);

                    
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));

                    
                    mpOutputDevice->Pop();
                    mpOutputDevice->SetAntialiasing(nAntiAliasing);
                    break;
                }
                case PRIMITIVE2D_ID_EPSPRIMITIVE2D :
                {
                    RenderEpsPrimitive2D(static_cast< const primitive2d::EpsPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D:
                {
                    RenderSvgLinearAtomPrimitive2D(static_cast< const primitive2d::SvgLinearAtomPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D:
                {
                    RenderSvgRadialAtomPrimitive2D(static_cast< const primitive2d::SvgRadialAtomPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D:
                {
                    
                    
                    
                    sal_uInt16 nAntiAliasing = mpOutputDevice->GetAntialiasing();
                    mpOutputDevice->SetAntialiasing(nAntiAliasing & ~ANTIALIASING_ENABLE_B2DDRAW);

                    const drawinglayer::primitive2d::BorderLinePrimitive2D& rBorder =
                        static_cast<const drawinglayer::primitive2d::BorderLinePrimitive2D&>(rCandidate);

                    if (!tryDrawBorderLinePrimitive2DDirect(rBorder))
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));

                    mpOutputDevice->SetAntialiasing(nAntiAliasing);
                    break;
                }
                default :
                {
                    
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
            }
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
