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

#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>
#include <vcl/outdev.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
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

//////////////////////////////////////////////////////////////////////////////

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
            // prepare maCurrentTransformation matrix with viewTransformation to target directly to pixels
            maCurrentTransformation = rViewInformation.getObjectToViewTransformation();

            // prepare output directly to pixels
               mpOutputDevice->Push(PUSH_MAPMODE);
            mpOutputDevice->SetMapMode();

            // react on AntiAliasing settings
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
            // restore MapMode
               mpOutputDevice->Pop();

            // restore AntiAliasing
            mpOutputDevice->SetAntialiasing(m_pImpl->m_nOrigAntiAliasing);
        }

        bool VclPixelProcessor2D::tryDrawPolyPolygonColorPrimitive2DDirect(const drawinglayer::primitive2d::PolyPolygonColorPrimitive2D& rSource, double fTransparency)
        {
            basegfx::B2DPolyPolygon aLocalPolyPolygon(rSource.getB2DPolyPolygon());

            if(!aLocalPolyPolygon.count())
            {
                // no geometry, done
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
                // Vertical line.
            }
            else if (fY1 == fY2)
            {
                // Horizontal line.
                bHorizontal = true;
            }
            else
                // Neither.  Bail out.
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
                        // Horizontal line.  Draw it as a rectangle.

                        aTarget = makeRectPolygon(fX1, fY1, fX2-fX1, nThick);
                        aTarget.transform(maCurrentTransformation);

                        basegfx::B2DRange aRange = aTarget.getB2DRange();
                        double fH = aRange.getHeight();

                        if (fH <= 1.0 || bDouble)
                        {
                            // Draw it as a line.
                            drawHairLine(
                                mpOutputDevice, aRange.getMinX(), aRange.getMinY(), aRange.getMaxX(), aRange.getMinY(),
                                aLineColor);

                            if (bDouble)
                            {
                                drawHairLine(
                                    mpOutputDevice, aRange.getMinX(), aRange.getMinY()+2.0, aRange.getMaxX(), aRange.getMinY()+2.0,
                                    aLineColor);
                            }

                            return true;
                        }
                    }
                    else
                    {
                        // Vertical line.  Draw it as a rectangle.

                        aTarget = makeRectPolygon(fX1, fY1, nThick, fY2-fY1);
                        aTarget.transform(maCurrentTransformation);

                        basegfx::B2DRange aRange = aTarget.getB2DRange();
                        double fW = aRange.getWidth();

                        if (fW <= 1.0 || bDouble)
                        {
                            // Draw it as a line.
                            drawHairLine(
                                mpOutputDevice, aRange.getMinX(), aRange.getMinY(), aRange.getMinX(), aRange.getMaxY(),
                                aLineColor);

                            if (bDouble)
                            {
                                drawHairLine(
                                    mpOutputDevice, aRange.getMinX()+2.0, aRange.getMinY(), aRange.getMinX()+2.0, aRange.getMaxY(),
                                    aLineColor);
                            }
                            return true;
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
                        // Failed to get pattern values.
                        return false;

                    double nThick = rtl::math::round(rSource.getLeftWidth());
                    const basegfx::BColor aLineColor =
                        maBColorModifierStack.getModifiedColor(rSource.getRGBColorLeft());

                    // Transform the current line range before using it for rendering.
                    basegfx::B2DRange aRange(fX1, fY1, fX2, fY2);
                    aRange.transform(maCurrentTransformation);
                    fX1 = aRange.getMinX();
                    fX2 = aRange.getMaxX();
                    fY1 = aRange.getMinY();
                    fY2 = aRange.getMaxY();

                    basegfx::B2DPolyPolygon aTarget;

                    if (bHorizontal)
                    {
                        // Horizontal line.

                        if (basegfx::fTools::equalZero(nThick))
                        {
                            // Dash line segment too small to draw.  Substitute it with a solid line.
                            drawHairLine(mpOutputDevice, fX1, fY1, fX2, fY1, aLineColor);
                            return true;
                        }

                        // Create a dash unit polygon set.
                        basegfx::B2DPolyPolygon aDashes;
                        std::vector<double>::const_iterator it = aPattern.begin(), itEnd = aPattern.end();
                        for (; it != itEnd; ++it)
                            aDashes.append(makeRectPolygon(0, 0, *it, nThick));

                        aDashes.transform(maCurrentTransformation);
                        rtl::math::setNan(&nThick);

                        // Pixelize the dash unit.  We use the same height for
                        // all dash polygons.
                        basegfx::B2DPolyPolygon aDashesPix;

                        for (sal_uInt32 i = 0, n = aDashes.count(); i < n; ++i)
                        {
                            basegfx::B2DPolygon aPoly = aDashes.getB2DPolygon(i);
                            aRange = aPoly.getB2DRange();
                            double fW = rtl::math::round(aRange.getWidth());
                            if (basegfx::fTools::equalZero(fW))
                            {
                                // Dash line segment too small to draw.  Substitute it with a solid line.
                                drawHairLine(mpOutputDevice, fX1, fY1, fX2, fY1, aLineColor);
                                return true;
                            }

                            if (rtl::math::isNan(nThick))
                                nThick = rtl::math::round(aRange.getHeight());

                            aDashesPix.append(makeRectPolygon(0, 0, fW, nThick));
                        }

                        // Make all dash polygons and render them.
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
                                    // Clip the right end in case it spills over the range.
                                    fBlockW = fX2 - fX + 1;

                                double fH = aRange.getHeight();
                                if (basegfx::fTools::equalZero(fH))
                                    fH = 1.0;

                                aTarget.append(makeRectPolygon(fX, fY1, fBlockW, fH));
                            }

                            bLine = !bLine; // line and blank alternate.
                            fX += aRange.getWidth();

                            ++i;
                            if (i >= n)
                                i = 0;
                        }
                    }
                    else
                    {
                        // Vertical line.

                        if (basegfx::fTools::equalZero(nThick))
                        {
                            // Dash line segment too small to draw.  Substitute it with a solid line.
                            drawHairLine(mpOutputDevice, fX1, fY1, fX1, fY2, aLineColor);
                            return true;
                        }

                        // Create a dash unit polygon set.
                        basegfx::B2DPolyPolygon aDashes;
                        std::vector<double>::const_iterator it = aPattern.begin(), itEnd = aPattern.end();
                        for (; it != itEnd; ++it)
                            aDashes.append(makeRectPolygon(0, 0, nThick, *it));

                        aDashes.transform(maCurrentTransformation);
                        rtl::math::setNan(&nThick);

                        // Pixelize the dash unit.  We use the same width for
                        // all dash polygons.
                        basegfx::B2DPolyPolygon aDashesPix;

                        for (sal_uInt32 i = 0, n = aDashes.count(); i < n; ++i)
                        {
                            basegfx::B2DPolygon aPoly = aDashes.getB2DPolygon(i);
                            aRange = aPoly.getB2DRange();
                            double fH = rtl::math::round(aRange.getHeight());
                            if (basegfx::fTools::equalZero(fH))
                            {
                                // Dash line segment too small to draw.  Substitute it with a solid line.
                                drawHairLine(mpOutputDevice, fX1, fY1, fX1, fY2, aLineColor);
                                return true;
                            }

                            if (rtl::math::isNan(nThick))
                                nThick = rtl::math::round(aRange.getWidth());

                            aDashesPix.append(makeRectPolygon(0, 0, nThick, fH));
                        }

                        // Make all dash polygons and render them.
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
                                    // Clip the bottom end in case it spills over the range.
                                    fBlockH = fY2 - fY + 1;

                                double fW = aRange.getWidth();
                                if (basegfx::fTools::equalZero(fW))
                                    fW = 1.0;

                                aTarget.append(makeRectPolygon(fX1, fY, fW, fBlockH));
                            }

                            bLine = !bLine; // line and blank alternate.
                            fY += aRange.getHeight();

                            ++i;
                            if (i >= n)
                                i = 0;
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
                    // directdraw of wrong spell primitive; added test possibility to check wrong spell decompose
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
                            // fallback to decomposition (MetaFile)
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
                    // directdraw of text simple portion; added test possibility to check text decompose
                    static bool bForceSimpleTextDecomposition(false);

                    // Adapt evtl. used special DrawMode
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

                    // restore DrawMode
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // directdraw of text simple portion; added test possibility to check text decompose
                    static bool bForceComplexTextDecomposition(false);

                    // Adapt evtl. used special DrawMode
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

                    // restore DrawMode
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    // direct draw of hairline
                    RenderPolygonHairlinePrimitive2D(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate), true);
                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    // direct draw of transformed BitmapEx primitive
                    RenderBitmapPrimitive2D(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_FILLBITMAPPRIMITIVE2D :
                {
                    // direct draw of fillBitmapPrimitive
                    RenderFillBitmapPrimitive2D(static_cast< const primitive2d::FillBitmapPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D :
                {
                    // direct draw of gradient
                    RenderPolyPolygonGradientPrimitive2D(static_cast< const primitive2d::PolyPolygonGradientPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONBITMAPPRIMITIVE2D :
                {
                    // direct draw of bitmap
                    RenderPolyPolygonBitmapPrimitive2D(static_cast< const primitive2d::PolyPolygonBitmapPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    // try to use directly
                    const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D = static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate);
                    basegfx::B2DPolyPolygon aLocalPolyPolygon;
                    static bool bAllowed(true);

                    if(bAllowed && tryDrawPolyPolygonColorPrimitive2DDirect(rPolyPolygonColorPrimitive2D, 0.0))
                    {
                        // okay, done. In this case no gaps should have to be repaired, too
                    }
                    else
                    {
                        // direct draw of PolyPolygon with color
                        const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPolyPolygonColorPrimitive2D.getBColor()));

                        mpOutputDevice->SetFillColor(Color(aPolygonColor));
                        mpOutputDevice->SetLineColor();
                        aLocalPolyPolygon = rPolyPolygonColorPrimitive2D.getB2DPolyPolygon();
                        aLocalPolyPolygon.transform(maCurrentTransformation);
                        mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
                    }

                    // when AA is on and this filled polygons are the result of stroked line geometry,
                    // draw the geometry once extra as lines to avoid AA 'gaps' between partial polygons
                    // Caution: This is needed in both cases (!)
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
                       // #i98289#
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
                        // use new Metafile decomposition
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    else
                    {
                        // direct draw of MetaFile
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
                    // mask group.
                    RenderMaskPrimitive2DPixel(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    // modified color group. Force output to unified color.
                    RenderModifiedColorPrimitive2D(static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D :
                {
                    // Detect if a single PolyPolygonColorPrimitive2D is contained; in that case,
                    // use the faster OutputDevice::DrawTransparent method
                    const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate = static_cast< const primitive2d::UnifiedTransparencePrimitive2D& >(rCandidate);
                    const primitive2d::Primitive2DSequence rContent = rUniTransparenceCandidate.getChildren();

                    if(rContent.hasElements())
                    {
                        if(0.0 == rUniTransparenceCandidate.getTransparence())
                        {
                            // not transparent at all, use content
                            process(rUniTransparenceCandidate.getChildren());
                        }
                        else if(rUniTransparenceCandidate.getTransparence() > 0.0 && rUniTransparenceCandidate.getTransparence() < 1.0)
                        {
                            bool bDrawTransparentUsed(false);

                            // since DEV300 m33 DrawTransparent is supported in VCL (for some targets
                            // natively), so i am now enabling this shortcut
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
                                            // single transparent PolyPolygon identified, use directly
                                            const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor = static_cast< const primitive2d::PolyPolygonColorPrimitive2D* >(pBasePrimitive);
                                            OSL_ENSURE(pPoPoColor, "OOps, PrimitiveID and PrimitiveType do not match (!)");
                                            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(pPoPoColor->getBColor()));
                                            mpOutputDevice->SetFillColor(Color(aPolygonColor));
                                            mpOutputDevice->SetLineColor();

                                            basegfx::B2DPolyPolygon aLocalPolyPolygon(pPoPoColor->getB2DPolyPolygon());
                                            aLocalPolyPolygon.transform(maCurrentTransformation);

                                            mpOutputDevice->DrawTransparent(aLocalPolyPolygon, rUniTransparenceCandidate.getTransparence());
                                            bDrawTransparentUsed = true;
                                            break;
                                        }
                                        // #i# need to wait for #i101378# which is in CWS vcl112 to directly paint transparent hairlines
                                        //case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
                                        //{
                                        //  // single transparent PolygonHairlinePrimitive2D identified, use directly
                                        //  const primitive2d::PolygonHairlinePrimitive2D* pPoHair = static_cast< const primitive2d::PolygonHairlinePrimitive2D* >(pBasePrimitive);
                                        //  OSL_ENSURE(pPoHair, "OOps, PrimitiveID and PrimitiveType do not match (!)");
                                        //  break;
                                        //}
                                    }
                                }
                            }

                            if(!bDrawTransparentUsed)
                            {
                                // unified sub-transparence. Draw to VDev first.
                                RenderUnifiedTransparencePrimitive2D(rUniTransparenceCandidate);
                            }
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                {
                    // sub-transparence group. Draw to VDev first.
                    RenderTransparencePrimitive2D(static_cast< const primitive2d::TransparencePrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // transform group.
                    RenderTransformPrimitive2D(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D :
                {
                    // new XDrawPage for ViewInformation2D
                    RenderPagePreviewPrimitive2D(static_cast< const primitive2d::PagePreviewPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                {
                    // marker array
                    RenderMarkerArrayPrimitive2D(static_cast< const primitive2d::MarkerArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    // point array
                    RenderPointArrayPrimitive2D(static_cast< const primitive2d::PointArrayPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                {
                    // control primitive
                    const primitive2d::ControlPrimitive2D& rControlPrimitive = static_cast< const primitive2d::ControlPrimitive2D& >(rCandidate);
                    const uno::Reference< awt::XControl >& rXControl(rControlPrimitive.getXControl());

                    try
                    {
                        // remember old graphics and create new
                        uno::Reference< awt::XView > xControlView(rXControl, uno::UNO_QUERY_THROW);
                        const uno::Reference< awt::XGraphics > xOriginalGraphics(xControlView->getGraphics());
                        const uno::Reference< awt::XGraphics > xNewGraphics(mpOutputDevice->CreateUnoGraphics());

                        if(xNewGraphics.is())
                        {
                            // link graphics and view
                            xControlView->setGraphics(xNewGraphics);

                            // get position
                            const basegfx::B2DHomMatrix aObjectToPixel(maCurrentTransformation * rControlPrimitive.getTransform());
                            const basegfx::B2DPoint aTopLeftPixel(aObjectToPixel * basegfx::B2DPoint(0.0, 0.0));

                            // find out if the control is already visualized as a VCL-ChildWindow. If yes,
                            // it does not need to be painted at all.
                            uno::Reference< awt::XWindow2 > xControlWindow(rXControl, uno::UNO_QUERY_THROW);
                            const bool bControlIsVisibleAsChildWindow(rXControl->getPeer().is() && xControlWindow->isVisible());

                            if(!bControlIsVisibleAsChildWindow)
                            {
                                // draw it. Do not forget to use the evtl. offsetted origin of the target device,
                                // e.g. when used with mask/transparence buffer device
                                const Point aOrigin(mpOutputDevice->GetMapMode().GetOrigin());
                                xControlView->draw(
                                    aOrigin.X() + basegfx::fround(aTopLeftPixel.getX()),
                                    aOrigin.Y() + basegfx::fround(aTopLeftPixel.getY()));
                            }

                            // restore original graphics
                            xControlView->setGraphics(xOriginalGraphics);
                        }
                    }
                    catch(const uno::Exception&)
                    {
                        // #i116763# removing since there is a good alternative when the xControlView
                        // is not found and it is allowed to happen
                        // DBG_UNHANDLED_EXCEPTION();

                        // process recursively and use the decomposition as Bitmap
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                {
                    // the stroke primitive may be decomposed to filled polygons. To keep
                    // evtl. set DrawModes aka DRAWMODE_BLACKLINE, DRAWMODE_GRAYLINE,
                    // DRAWMODE_GHOSTEDLINE, DRAWMODE_WHITELINE or DRAWMODE_SETTINGSLINE
                    // working, these need to be copied to the corresponding fill modes
                    const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
                    adaptLineToFillDrawMode();

                    // polygon stroke primitive
                    static bool bSuppressFatToHairlineCorrection(false);

                    if(bSuppressFatToHairlineCorrection)
                    {
                        // remember that we enter a PolygonStrokePrimitive2D decomposition,
                        // used for AA thick line drawing
                        mnPolygonStrokePrimitive2D++;

                        // with AA there is no need to handle thin lines special
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));

                        // leave PolygonStrokePrimitive2D
                        mnPolygonStrokePrimitive2D--;
                    }
                    else
                    {
                        // Lines with 1 and 2 pixel width without AA need special treatment since their vsiualisation
                        // as filled polygons is geometrically corret but looks wrong since polygon filling avoids
                        // the right and bottom pixels. The used method evaluates that and takes the correct action,
                        // including calling recursively with decomposition if line is wide enough
                        const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokePrimitive = static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate);

                        RenderPolygonStrokePrimitive2D(rPolygonStrokePrimitive);
                    }

                    // restore DrawMode
                    mpOutputDevice->SetDrawMode(nOriginalDrawMode);

                    break;
                }
                case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D :
                {
                    static bool bForceIgnoreHatchSmoothing(false);

                    if(bForceIgnoreHatchSmoothing || getOptionsDrawinglayer().IsAntiAliasing())
                    {
                        // if AA is used (or ignore smoothing is on), there is no need to smooth
                        // hatch painting, use decomposition
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    else
                    {
                        // without AA, use VCL to draw the hatch. It snaps hatch distances to the next pixel
                        // and forces hatch distance to be >= 3 pixels to make the hatch display look smoother.
                        // This is wrong in principle, but looks nicer. This could also be done here directly
                        // without VCL usage if needed
                        const primitive2d::FillHatchPrimitive2D& rFillHatchPrimitive = static_cast< const primitive2d::FillHatchPrimitive2D& >(rCandidate);
                        const attribute::FillHatchAttribute& rFillHatchAttributes = rFillHatchPrimitive.getFillHatch();

                        // create hatch polygon in range size and discrete coordinates
                        basegfx::B2DRange aHatchRange(rFillHatchPrimitive.getObjectRange());
                        aHatchRange.transform(maCurrentTransformation);
                        const basegfx::B2DPolygon aHatchPolygon(basegfx::tools::createPolygonFromRect(aHatchRange));

                        if(rFillHatchAttributes.isFillBackground())
                        {
                            // #i111846# background fill is active; draw fill polygon
                            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rFillHatchPrimitive.getBColor()));

                            mpOutputDevice->SetFillColor(Color(aPolygonColor));
                            mpOutputDevice->SetLineColor();
                            mpOutputDevice->DrawPolygon(aHatchPolygon);
                        }

                        // set hatch line color
                        const basegfx::BColor aHatchColor(maBColorModifierStack.getModifiedColor(rFillHatchPrimitive.getBColor()));
                        mpOutputDevice->SetFillColor();
                        mpOutputDevice->SetLineColor(Color(aHatchColor));

                        // get hatch style
                        HatchStyle eHatchStyle(HATCH_SINGLE);

                        switch(rFillHatchAttributes.getStyle())
                        {
                            default : // HATCHSTYLE_SINGLE
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

                        // create hatch
                        const basegfx::B2DVector aDiscreteDistance(maCurrentTransformation * basegfx::B2DVector(rFillHatchAttributes.getDistance(), 0.0));
                        const sal_uInt32 nDistance(basegfx::fround(aDiscreteDistance.getLength()));
                        const sal_uInt16 nAngle10((sal_uInt16)basegfx::fround(rFillHatchAttributes.getAngle() / F_PI1800));
                        ::Hatch aVCLHatch(eHatchStyle, Color(rFillHatchAttributes.getColor()), nDistance, nAngle10);

                        // draw hatch using VCL
                        mpOutputDevice->DrawHatch(PolyPolygon(Polygon(aHatchPolygon)), aVCLHatch);
                    }
                    break;
                }
                case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D :
                {
                    // #i98404# Handle directly, especially when AA is active
                    const primitive2d::BackgroundColorPrimitive2D& rPrimitive = static_cast< const primitive2d::BackgroundColorPrimitive2D& >(rCandidate);
                    const sal_uInt16 nOriginalAA(mpOutputDevice->GetAntialiasing());

                    // switch AA off in all cases
                    mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() & ~ANTIALIASING_ENABLE_B2DDRAW);

                    // create color for fill
                    const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPrimitive.getBColor()));
                    mpOutputDevice->SetFillColor(Color(aPolygonColor));
                    mpOutputDevice->SetLineColor();

                    // create rectangle for fill
                    const basegfx::B2DRange& aViewport(getViewInformation2D().getDiscreteViewport());
                    const Rectangle aRectangle(
                        (sal_Int32)floor(aViewport.getMinX()), (sal_Int32)floor(aViewport.getMinY()),
                        (sal_Int32)ceil(aViewport.getMaxX()), (sal_Int32)ceil(aViewport.getMaxY()));
                    mpOutputDevice->DrawRect(aRectangle);

                    // restore AA setting
                    mpOutputDevice->SetAntialiasing(nOriginalAA);
                    break;
                }
                case PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D :
                {
                    // #i97628#
                    // This primitive means that the content is derived from an active text edit,
                    // not from model data itself. Some renderers need to suppress this content, e.g.
                    // the pixel renderer used for displaying the edit view (like this one). It's
                    // not to be suppressed by the MetaFile renderers, so that the edited text is
                    // part of the MetaFile, e.g. needed for presentation previews.
                    // Action: Ignore here, do nothing.
                    break;
                }
                case PRIMITIVE2D_ID_INVERTPRIMITIVE2D :
                {
                    // invert primitive (currently only used for HighContrast fallback for selection in SW and SC).
                    // Set OutDev to XOR and switch AA off (XOR does not work with AA)
                    mpOutputDevice->Push();
                    mpOutputDevice->SetRasterOp( ROP_XOR );
                    const sal_uInt16 nAntiAliasing(mpOutputDevice->GetAntialiasing());
                    mpOutputDevice->SetAntialiasing(nAntiAliasing & ~ANTIALIASING_ENABLE_B2DDRAW);

                    // process content recursively
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));

                    // restore OutDev
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
                    // process recursively, but turn off anti-aliasing. Border
                    // lines are always rectangular, and look horrible when
                    // the anti-aliasing is enabled.
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
                    // process recursively
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
