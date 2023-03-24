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

#include "vclpixelprocessor2d.hxx"
#include "vclhelperbufferdevice.hxx"
#include "helperwrongspellrenderer.hxx"
#include <comphelper/lok.hxx>

#include <sal/log.hxx>
#include <vcl/outdev.hxx>
#include <vcl/hatch.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/utils/gradienttools.hxx>

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGraphicPrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/glowprimitive2d.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XControl.hpp>

#include <svtools/optionsdrawinglayer.hxx>
#include <vcl/gradient.hxx>

using namespace com::sun::star;

namespace drawinglayer::processor2d
{
VclPixelProcessor2D::VclPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                                         OutputDevice& rOutDev,
                                         const basegfx::BColorModifierStack& rInitStack)
    : VclProcessor2D(rViewInformation, rOutDev, rInitStack)
    , m_nOrigAntiAliasing(rOutDev.GetAntialiasing())
{
    // prepare maCurrentTransformation matrix with viewTransformation to target directly to pixels
    maCurrentTransformation = rViewInformation.getObjectToViewTransformation();

    // prepare output directly to pixels
    mpOutputDevice->Push(vcl::PushFlags::MAPMODE);
    mpOutputDevice->SetMapMode();

    // react on AntiAliasing settings
    if (rViewInformation.getUseAntiAliasing())
    {
        mpOutputDevice->SetAntialiasing(m_nOrigAntiAliasing | AntialiasingFlags::Enable);
    }
    else
    {
        mpOutputDevice->SetAntialiasing(m_nOrigAntiAliasing & ~AntialiasingFlags::Enable);
    }
}

VclPixelProcessor2D::~VclPixelProcessor2D()
{
    // restore MapMode
    mpOutputDevice->Pop();

    // restore AntiAliasing
    mpOutputDevice->SetAntialiasing(m_nOrigAntiAliasing);
}

void VclPixelProcessor2D::tryDrawPolyPolygonColorPrimitive2DDirect(
    const drawinglayer::primitive2d::PolyPolygonColorPrimitive2D& rSource, double fTransparency)
{
    if (!rSource.getB2DPolyPolygon().count() || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        // no geometry, done
        return;
    }

    const basegfx::BColor aPolygonColor(
        maBColorModifierStack.getModifiedColor(rSource.getBColor()));

    mpOutputDevice->SetFillColor(Color(aPolygonColor));
    mpOutputDevice->SetLineColor();
    mpOutputDevice->DrawTransparent(maCurrentTransformation, rSource.getB2DPolyPolygon(),
                                    fTransparency);
}

bool VclPixelProcessor2D::tryDrawPolygonHairlinePrimitive2DDirect(
    const drawinglayer::primitive2d::PolygonHairlinePrimitive2D& rSource, double fTransparency)
{
    const basegfx::B2DPolygon& rLocalPolygon(rSource.getB2DPolygon());

    if (!rLocalPolygon.count() || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        // no geometry, done
        return true;
    }

    const basegfx::BColor aLineColor(maBColorModifierStack.getModifiedColor(rSource.getBColor()));

    mpOutputDevice->SetFillColor();
    mpOutputDevice->SetLineColor(Color(aLineColor));
    //aLocalPolygon.transform(maCurrentTransformation);

    // try drawing; if it did not work, use standard fallback
    return mpOutputDevice->DrawPolyLineDirect(maCurrentTransformation, rLocalPolygon, 0.0,
                                              fTransparency);
}

bool VclPixelProcessor2D::tryDrawPolygonStrokePrimitive2DDirect(
    const drawinglayer::primitive2d::PolygonStrokePrimitive2D& rSource, double fTransparency)
{
    const basegfx::B2DPolygon& rLocalPolygon(rSource.getB2DPolygon());

    if (!rLocalPolygon.count() || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        // no geometry, done
        return true;
    }

    if (basegfx::B2DLineJoin::NONE == rSource.getLineAttribute().getLineJoin()
        && css::drawing::LineCap_BUTT != rSource.getLineAttribute().getLineCap())
    {
        // better use decompose to get that combination done for now, see discussion
        // at https://bugs.documentfoundation.org/show_bug.cgi?id=130478#c17 and ff
        return false;
    }

    // MM01: Radically change here - no dismantle/applyLineDashing,
    // let that happen low-level at DrawPolyLineDirect implementations
    // to open up for buffering and evtl. direct draw with sys-dep
    // graphic systems. Check for stroke is in use
    const bool bStrokeAttributeNotUsed(rSource.getStrokeAttribute().isDefault()
                                       || 0.0 == rSource.getStrokeAttribute().getFullDotDashLen());

    const basegfx::BColor aLineColor(
        maBColorModifierStack.getModifiedColor(rSource.getLineAttribute().getColor()));

    mpOutputDevice->SetFillColor();
    mpOutputDevice->SetLineColor(Color(aLineColor));

    // MM01 draw direct, hand over dash data if available
    return mpOutputDevice->DrawPolyLineDirect(
        maCurrentTransformation, rLocalPolygon,
        // tdf#124848 use LineWidth direct, do not try to solve for zero-case (aka hairline)
        rSource.getLineAttribute().getWidth(), fTransparency,
        bStrokeAttributeNotUsed ? nullptr : &rSource.getStrokeAttribute().getDotDashArray(),
        rSource.getLineAttribute().getLineJoin(), rSource.getLineAttribute().getLineCap(),
        rSource.getLineAttribute().getMiterMinimumAngle());
}

namespace
{
GradientStyle convertGradientStyle(drawinglayer::attribute::GradientStyle eGradientStyle)
{
    switch (eGradientStyle)
    {
        case drawinglayer::attribute::GradientStyle::Axial:
            return GradientStyle::Axial;
        case drawinglayer::attribute::GradientStyle::Radial:
            return GradientStyle::Radial;
        case drawinglayer::attribute::GradientStyle::Elliptical:
            return GradientStyle::Elliptical;
        case drawinglayer::attribute::GradientStyle::Square:
            return GradientStyle::Square;
        case drawinglayer::attribute::GradientStyle::Rect:
            return GradientStyle::Rect;
        case drawinglayer::attribute::GradientStyle::Linear:
            return GradientStyle::Linear;
        default:
            assert(false);
            return GradientStyle::Linear;
    }
}

} // end anonymous namespace

void VclPixelProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    switch (rCandidate.getPrimitive2DID())
    {
        case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D:
        {
            processWrongSpellPrimitive2D(
                static_cast<const primitive2d::WrongSpellPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
        {
            processTextSimplePortionPrimitive2D(
                static_cast<const primitive2d::TextSimplePortionPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
        {
            processTextDecoratedPortionPrimitive2D(
                static_cast<const primitive2d::TextSimplePortionPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
        {
            processPolygonHairlinePrimitive2D(
                static_cast<const primitive2d::PolygonHairlinePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
        {
            // direct draw of transformed BitmapEx primitive
            processBitmapPrimitive2D(
                static_cast<const primitive2d::BitmapPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D:
        {
            // direct draw of fillBitmapPrimitive
            RenderFillGraphicPrimitive2D(
                static_cast<const primitive2d::FillGraphicPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D:
        {
            processPolyPolygonGradientPrimitive2D(
                static_cast<const primitive2d::PolyPolygonGradientPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
        {
            // direct draw of bitmap
            RenderPolyPolygonGraphicPrimitive2D(
                static_cast<const primitive2d::PolyPolygonGraphicPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
        {
            processPolyPolygonColorPrimitive2D(
                static_cast<const primitive2d::PolyPolygonColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D:
        {
            processMetaFilePrimitive2D(rCandidate);
            break;
        }
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
        {
            // mask group.
            RenderMaskPrimitive2DPixel(
                static_cast<const primitive2d::MaskPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
        {
            // modified color group. Force output to unified color.
            RenderModifiedColorPrimitive2D(
                static_cast<const primitive2d::ModifiedColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
        {
            processUnifiedTransparencePrimitive2D(
                static_cast<const primitive2d::UnifiedTransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D:
        {
            // sub-transparence group. Draw to VDev first.
            RenderTransparencePrimitive2D(
                static_cast<const primitive2d::TransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
        {
            // transform group.
            RenderTransformPrimitive2D(
                static_cast<const primitive2d::TransformPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D:
        {
            // new XDrawPage for ViewInformation2D
            RenderPagePreviewPrimitive2D(
                static_cast<const primitive2d::PagePreviewPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D:
        {
            // marker array
            RenderMarkerArrayPrimitive2D(
                static_cast<const primitive2d::MarkerArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
        {
            // point array
            RenderPointArrayPrimitive2D(
                static_cast<const primitive2d::PointArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D:
        {
            processControlPrimitive2D(
                static_cast<const primitive2d::ControlPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
        {
            processPolygonStrokePrimitive2D(
                static_cast<const primitive2d::PolygonStrokePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D:
        {
            processFillHatchPrimitive2D(
                static_cast<const primitive2d::FillHatchPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D:
        {
            processBackgroundColorPrimitive2D(
                static_cast<const primitive2d::BackgroundColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_INVERTPRIMITIVE2D:
        {
            processInvertPrimitive2D(rCandidate);
            break;
        }
        case PRIMITIVE2D_ID_EPSPRIMITIVE2D:
        {
            RenderEpsPrimitive2D(static_cast<const primitive2d::EpsPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D:
        {
            RenderSvgLinearAtomPrimitive2D(
                static_cast<const primitive2d::SvgLinearAtomPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D:
        {
            RenderSvgRadialAtomPrimitive2D(
                static_cast<const primitive2d::SvgRadialAtomPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D:
        {
            processBorderLinePrimitive2D(
                static_cast<const drawinglayer::primitive2d::BorderLinePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D:
        {
            processFillGradientPrimitive2D(
                static_cast<const drawinglayer::primitive2d::FillGradientPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_PATTERNFILLPRIMITIVE2D:
        {
            processPatternFillPrimitive2D(
                static_cast<const drawinglayer::primitive2d::PatternFillPrimitive2D&>(rCandidate));
            break;
        }
        default:
        {
            SAL_INFO("drawinglayer", "default case for " << drawinglayer::primitive2d::idToString(
                                         rCandidate.getPrimitive2DID()));
            // process recursively
            process(rCandidate);
            break;
        }
    }
}

void VclPixelProcessor2D::processWrongSpellPrimitive2D(
    const primitive2d::WrongSpellPrimitive2D& rWrongSpellPrimitive)
{
    if (!renderWrongSpellPrimitive2D(rWrongSpellPrimitive, *mpOutputDevice, maCurrentTransformation,
                                     maBColorModifierStack))
    {
        // fallback to decomposition (MetaFile)
        process(rWrongSpellPrimitive);
    }
}

void VclPixelProcessor2D::processTextSimplePortionPrimitive2D(
    const primitive2d::TextSimplePortionPrimitive2D& rCandidate)
{
    // Adapt evtl. used special DrawMode
    const DrawModeFlags nOriginalDrawMode(mpOutputDevice->GetDrawMode());
    adaptTextToFillDrawMode();

    if (SvtOptionsDrawinglayer::IsRenderSimpleTextDirect())
    {
        RenderTextSimpleOrDecoratedPortionPrimitive2D(rCandidate);
    }
    else
    {
        process(rCandidate);
    }

    // restore DrawMode
    mpOutputDevice->SetDrawMode(nOriginalDrawMode);
}

void VclPixelProcessor2D::processTextDecoratedPortionPrimitive2D(
    const primitive2d::TextSimplePortionPrimitive2D& rCandidate)
{
    // Adapt evtl. used special DrawMode
    const DrawModeFlags nOriginalDrawMode(mpOutputDevice->GetDrawMode());
    adaptTextToFillDrawMode();

    if (SvtOptionsDrawinglayer::IsRenderDecoratedTextDirect())
    {
        RenderTextSimpleOrDecoratedPortionPrimitive2D(rCandidate);
    }
    else
    {
        process(rCandidate);
    }

    // restore DrawMode
    mpOutputDevice->SetDrawMode(nOriginalDrawMode);
}

void VclPixelProcessor2D::processPolygonHairlinePrimitive2D(
    const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D)
{
    if (tryDrawPolygonHairlinePrimitive2DDirect(rPolygonHairlinePrimitive2D, 0.0))
    {
        return;
    }

    // direct draw of hairline
    RenderPolygonHairlinePrimitive2D(rPolygonHairlinePrimitive2D, true);
}

void VclPixelProcessor2D::processBitmapPrimitive2D(
    const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
{
    // check if graphic content is inside discrete local ViewPort
    const basegfx::B2DRange& rDiscreteViewPort(getViewInformation2D().getDiscreteViewport());
    const basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation
                                                * rBitmapCandidate.getTransform());

    if (!rDiscreteViewPort.isEmpty())
    {
        basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);

        aUnitRange.transform(aLocalTransform);

        if (!aUnitRange.overlaps(rDiscreteViewPort))
        {
            // content is outside discrete local ViewPort
            return;
        }
    }

    RenderBitmapPrimitive2D(rBitmapCandidate);
}

void VclPixelProcessor2D::processPolyPolygonGradientPrimitive2D(
    const primitive2d::PolyPolygonGradientPrimitive2D& rPolygonCandidate)
{
    basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());

    // no geometry, no need to render, done
    if (!aLocalPolyPolygon.count())
        return;

    // *try* direct draw (AKA using old VCL stuff) to render gradient
    const attribute::FillGradientAttribute& rGradient(rPolygonCandidate.getFillGradient());

    // MCGR: *many* - and not only GradientStops - cases cannot be handled by VCL
    // so use decomposition
    // NOTE: There may be even more reasons to detect, e.g. a ViewTransformation
    // that uses shear/rotate/mirror (what VCL cannot handle at all), see
    // other checks already in processFillGradientPrimitive2D
    if (rGradient.cannotBeHandledByVCL())
    {
        process(rPolygonCandidate);
        return;
    }

    basegfx::BColor aStartColor(
        maBColorModifierStack.getModifiedColor(rGradient.getColorStops().front().getStopColor()));
    basegfx::BColor aEndColor(
        maBColorModifierStack.getModifiedColor(rGradient.getColorStops().back().getStopColor()));

    if (aStartColor == aEndColor)
    {
        // no gradient at all, draw as polygon in AA and non-AA case
        aLocalPolyPolygon.transform(maCurrentTransformation);
        mpOutputDevice->SetLineColor();
        mpOutputDevice->SetFillColor(Color(aStartColor));
        mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
        return;
    }

    // use the primitive decomposition
    process(rPolygonCandidate);
}

void VclPixelProcessor2D::processPolyPolygonColorPrimitive2D(
    const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D)
{
    // try to use directly
    basegfx::B2DPolyPolygon aLocalPolyPolygon;

    tryDrawPolyPolygonColorPrimitive2DDirect(rPolyPolygonColorPrimitive2D, 0.0);
    // okay, done. In this case no gaps should have to be repaired, too

    // when AA is on and this filled polygons are the result of stroked line geometry,
    // draw the geometry once extra as lines to avoid AA 'gaps' between partial polygons
    // Caution: This is needed in both cases (!)
    if (!(mnPolygonStrokePrimitive2D && getViewInformation2D().getUseAntiAliasing()
          && (mpOutputDevice->GetAntialiasing() & AntialiasingFlags::Enable)))
        return;

    const basegfx::BColor aPolygonColor(
        maBColorModifierStack.getModifiedColor(rPolyPolygonColorPrimitive2D.getBColor()));
    sal_uInt32 nCount(aLocalPolyPolygon.count());

    if (!nCount)
    {
        aLocalPolyPolygon = rPolyPolygonColorPrimitive2D.getB2DPolyPolygon();
        aLocalPolyPolygon.transform(maCurrentTransformation);
        nCount = aLocalPolyPolygon.count();
    }

    mpOutputDevice->SetFillColor();
    mpOutputDevice->SetLineColor(Color(aPolygonColor));

    for (sal_uInt32 a(0); a < nCount; a++)
    {
        mpOutputDevice->DrawPolyLine(aLocalPolyPolygon.getB2DPolygon(a), 0.0);
    }
}

void VclPixelProcessor2D::processUnifiedTransparencePrimitive2D(
    const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate)
{
    // Detect if a single PolyPolygonColorPrimitive2D is contained; in that case,
    // use the faster OutputDevice::DrawTransparent method
    const primitive2d::Primitive2DContainer& rContent = rUniTransparenceCandidate.getChildren();

    if (rContent.empty())
        return;

    if (0.0 == rUniTransparenceCandidate.getTransparence())
    {
        // not transparent at all, use content
        process(rUniTransparenceCandidate.getChildren());
    }
    else if (rUniTransparenceCandidate.getTransparence() > 0.0
             && rUniTransparenceCandidate.getTransparence() < 1.0)
    {
        bool bDrawTransparentUsed(false);

        if (1 == rContent.size())
        {
            const primitive2d::BasePrimitive2D* pBasePrimitive = rContent[0].get();

            switch (pBasePrimitive->getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
                {
                    // single transparent tools::PolyPolygon identified, use directly
                    const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor
                        = static_cast<const primitive2d::PolyPolygonColorPrimitive2D*>(
                            pBasePrimitive);
                    SAL_WARN_IF(!pPoPoColor, "drawinglayer",
                                "OOps, PrimitiveID and PrimitiveType do not match (!)");
                    bDrawTransparentUsed = true;
                    tryDrawPolyPolygonColorPrimitive2DDirect(
                        *pPoPoColor, rUniTransparenceCandidate.getTransparence());
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
                {
                    // single transparent PolygonHairlinePrimitive2D identified, use directly
                    const primitive2d::PolygonHairlinePrimitive2D* pPoHair
                        = static_cast<const primitive2d::PolygonHairlinePrimitive2D*>(
                            pBasePrimitive);
                    SAL_WARN_IF(!pPoHair, "drawinglayer",
                                "OOps, PrimitiveID and PrimitiveType do not match (!)");

                    // do no tallow by default - problem is that self-overlapping parts of this geometry will
                    // not be in an all-same transparency but will already alpha-cover themselves with blending.
                    // This is not what the UnifiedTransparencePrimitive2D defines: It requires all its
                    // content to be uniformly transparent.
                    // For hairline the effect is pretty minimal, but still not correct.
                    bDrawTransparentUsed = false;
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                {
                    // single transparent PolygonStrokePrimitive2D identified, use directly
                    const primitive2d::PolygonStrokePrimitive2D* pPoStroke
                        = static_cast<const primitive2d::PolygonStrokePrimitive2D*>(pBasePrimitive);
                    SAL_WARN_IF(!pPoStroke, "drawinglayer",
                                "OOps, PrimitiveID and PrimitiveType do not match (!)");

                    // do no tallow by default - problem is that self-overlapping parts of this geometry will
                    // not be in an all-same transparency but will already alpha-cover themselves with blending.
                    // This is not what the UnifiedTransparencePrimitive2D defines: It requires all its
                    // content to be uniformly transparent.
                    // To check, activate and draw a wide transparent self-crossing line/curve
                    bDrawTransparentUsed = false;
                    break;
                }
                default:
                    SAL_INFO("drawinglayer",
                             "default case for " << drawinglayer::primitive2d::idToString(
                                 rUniTransparenceCandidate.getPrimitive2DID()));
                    break;
            }
        }

        if (!bDrawTransparentUsed)
        {
            // unified sub-transparence. Draw to VDev first.
            RenderUnifiedTransparencePrimitive2D(rUniTransparenceCandidate);
        }
    }
}

void VclPixelProcessor2D::processControlPrimitive2D(
    const primitive2d::ControlPrimitive2D& rControlPrimitive)
{
    // control primitive
    const uno::Reference<awt::XControl>& rXControl(rControlPrimitive.getXControl());

    try
    {
        // remember old graphics and create new
        uno::Reference<awt::XView> xControlView(rXControl, uno::UNO_QUERY_THROW);
        const uno::Reference<awt::XGraphics> xOriginalGraphics(xControlView->getGraphics());
        const uno::Reference<awt::XGraphics> xNewGraphics(mpOutputDevice->CreateUnoGraphics());

        if (xNewGraphics.is())
        {
            // find out if the control is already visualized as a VCL-ChildWindow. If yes,
            // it does not need to be painted at all.
            uno::Reference<awt::XWindow2> xControlWindow(rXControl, uno::UNO_QUERY_THROW);
            bool bControlIsVisibleAsChildWindow(rXControl->getPeer().is()
                                                && xControlWindow->isVisible());

            // tdf#131281 The FormControls are not painted when using the Tiled Rendering for a simple
            // reason: when e.g. bControlIsVisibleAsChildWindow is true. This is the case because the
            // office is in non-layout mode (default for controls at startup). For the common office
            // this means that there exists a real VCL-System-Window for the control, so it is *not*
            // painted here due to being exactly obscured by that real Window (and creates danger of
            // flickering, too).
            // Tiled Rendering clients usually do *not* have real VCL-Windows for the controls, but
            // exactly that would be needed on each client displaying the tiles (what would be hard
            // to do but also would have advantages - the clients would have real controls in the
            //  shape of their target system which could be interacted with...). It is also what the
            // office does.
            // For now, fallback to just render these controls when Tiled Rendering is active to just
            // have them displayed on all clients.
            if (bControlIsVisibleAsChildWindow && comphelper::LibreOfficeKit::isActive())
            {
                // Do force paint when we are in Tiled Renderer and FormControl is 'visible'
                bControlIsVisibleAsChildWindow = false;
            }

            if (!bControlIsVisibleAsChildWindow)
            {
                // Needs to be drawn. Link new graphics and view
                xControlView->setGraphics(xNewGraphics);

                // get position
                const basegfx::B2DHomMatrix aObjectToPixel(maCurrentTransformation
                                                           * rControlPrimitive.getTransform());
                const basegfx::B2DPoint aTopLeftPixel(aObjectToPixel * basegfx::B2DPoint(0.0, 0.0));

                // Do not forget to use the evtl. offsetted origin of the target device,
                // e.g. when used with mask/transparence buffer device
                const Point aOrigin(mpOutputDevice->GetMapMode().GetOrigin());
                xControlView->draw(aOrigin.X() + basegfx::fround(aTopLeftPixel.getX()),
                                   aOrigin.Y() + basegfx::fround(aTopLeftPixel.getY()));

                // restore original graphics
                xControlView->setGraphics(xOriginalGraphics);
            }
        }
    }
    catch (const uno::Exception&)
    {
        // #i116763# removing since there is a good alternative when the xControlView
        // is not found and it is allowed to happen
        // DBG_UNHANDLED_EXCEPTION();

        // process recursively and use the decomposition as Bitmap
        process(rControlPrimitive);
    }
}

void VclPixelProcessor2D::processPolygonStrokePrimitive2D(
    const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokePrimitive2D)
{
    // try to use directly
    if (tryDrawPolygonStrokePrimitive2DDirect(rPolygonStrokePrimitive2D, 0.0))
    {
        return;
    }

    // the stroke primitive may be decomposed to filled polygons. To keep
    // evtl. set DrawModes aka DrawModeFlags::BlackLine, DrawModeFlags::GrayLine,
    // DrawModeFlags::GhostedLine, DrawModeFlags::WhiteLine or DrawModeFlags::SettingsLine
    // working, these need to be copied to the corresponding fill modes
    const DrawModeFlags nOriginalDrawMode(mpOutputDevice->GetDrawMode());
    adaptLineToFillDrawMode();

    // polygon stroke primitive

    // Lines with 1 and 2 pixel width without AA need special treatment since their visualization
    // as filled polygons is geometrically correct but looks wrong since polygon filling avoids
    // the right and bottom pixels. The used method evaluates that and takes the correct action,
    // including calling recursively with decomposition if line is wide enough
    RenderPolygonStrokePrimitive2D(rPolygonStrokePrimitive2D);

    // restore DrawMode
    mpOutputDevice->SetDrawMode(nOriginalDrawMode);
}

void VclPixelProcessor2D::processFillHatchPrimitive2D(
    const primitive2d::FillHatchPrimitive2D& rFillHatchPrimitive)
{
    if (getViewInformation2D().getUseAntiAliasing())
    {
        // if AA is used (or ignore smoothing is on), there is no need to smooth
        // hatch painting, use decomposition
        process(rFillHatchPrimitive);
    }
    else
    {
        // without AA, use VCL to draw the hatch. It snaps hatch distances to the next pixel
        // and forces hatch distance to be >= 3 pixels to make the hatch display look smoother.
        // This is wrong in principle, but looks nicer. This could also be done here directly
        // without VCL usage if needed
        const attribute::FillHatchAttribute& rFillHatchAttributes
            = rFillHatchPrimitive.getFillHatch();

        // create hatch polygon in range size and discrete coordinates
        basegfx::B2DRange aHatchRange(rFillHatchPrimitive.getOutputRange());
        aHatchRange.transform(maCurrentTransformation);
        const basegfx::B2DPolygon aHatchPolygon(basegfx::utils::createPolygonFromRect(aHatchRange));

        if (rFillHatchAttributes.isFillBackground())
        {
            // #i111846# background fill is active; draw fill polygon
            const basegfx::BColor aPolygonColor(
                maBColorModifierStack.getModifiedColor(rFillHatchPrimitive.getBColor()));

            mpOutputDevice->SetFillColor(Color(aPolygonColor));
            mpOutputDevice->SetLineColor();
            mpOutputDevice->DrawPolygon(aHatchPolygon);
        }

        // set hatch line color
        const basegfx::BColor aHatchColor(
            maBColorModifierStack.getModifiedColor(rFillHatchPrimitive.getBColor()));
        mpOutputDevice->SetFillColor();
        mpOutputDevice->SetLineColor(Color(aHatchColor));

        // get hatch style
        HatchStyle eHatchStyle(HatchStyle::Single);

        switch (rFillHatchAttributes.getStyle())
        {
            default: // HatchStyle::Single
            {
                break;
            }
            case attribute::HatchStyle::Double:
            {
                eHatchStyle = HatchStyle::Double;
                break;
            }
            case attribute::HatchStyle::Triple:
            {
                eHatchStyle = HatchStyle::Triple;
                break;
            }
        }

        // create hatch
        const basegfx::B2DVector aDiscreteDistance(
            maCurrentTransformation * basegfx::B2DVector(rFillHatchAttributes.getDistance(), 0.0));
        const sal_uInt32 nDistance(basegfx::fround(aDiscreteDistance.getLength()));
        const sal_uInt32 nAngle10(
            basegfx::rad2deg<10>(basegfx::fround(rFillHatchAttributes.getAngle())));
        ::Hatch aVCLHatch(eHatchStyle, Color(rFillHatchAttributes.getColor()), nDistance,
                          Degree10(nAngle10));

        // draw hatch using VCL
        mpOutputDevice->DrawHatch(::tools::PolyPolygon(::tools::Polygon(aHatchPolygon)), aVCLHatch);
    }
}

void VclPixelProcessor2D::processBackgroundColorPrimitive2D(
    const primitive2d::BackgroundColorPrimitive2D& rPrimitive)
{
    // #i98404# Handle directly, especially when AA is active
    const AntialiasingFlags nOriginalAA(mpOutputDevice->GetAntialiasing());

    // switch AA off in all cases
    mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() & ~AntialiasingFlags::Enable);

    // create color for fill
    const basegfx::BColor aPolygonColor(
        maBColorModifierStack.getModifiedColor(rPrimitive.getBColor()));
    Color aFillColor(aPolygonColor);
    aFillColor.SetAlpha(255 - sal_uInt8((rPrimitive.getTransparency() * 255.0) + 0.5));
    mpOutputDevice->SetFillColor(aFillColor);
    mpOutputDevice->SetLineColor();

    // create rectangle for fill
    const basegfx::B2DRange& aViewport(getViewInformation2D().getDiscreteViewport());
    const ::tools::Rectangle aRectangle(static_cast<sal_Int32>(floor(aViewport.getMinX())),
                                        static_cast<sal_Int32>(floor(aViewport.getMinY())),
                                        static_cast<sal_Int32>(ceil(aViewport.getMaxX())),
                                        static_cast<sal_Int32>(ceil(aViewport.getMaxY())));
    mpOutputDevice->DrawRect(aRectangle);

    // restore AA setting
    mpOutputDevice->SetAntialiasing(nOriginalAA);
}

void VclPixelProcessor2D::processBorderLinePrimitive2D(
    const drawinglayer::primitive2d::BorderLinePrimitive2D& rBorder)
{
    // Process recursively, but switch off AntiAliasing for
    // horizontal/vertical lines (*not* diagonal lines).
    // Checked using AntialiasingFlags::PixelSnapHairline instead,
    // but with AntiAliasing on the display really is too 'ghosty' when
    // using fine stroking. Correct, but 'ghosty'.

    // It has shown that there are quite some problems here:
    // - vcl OutDev renderer methods still use fallbacks to paint
    //   multiple single lines between discrete sizes of < 3.5 what
    //   looks bad and does not match
    // - mix of filled Polygons and Lines is bad when AA switched off
    // - Alignment of AA with non-AA may be bad in diverse different
    //   renderers
    //
    // Due to these reasons I change the strategy: Always draw AAed, but
    // allow fallback to test/check and if needed. The normal case
    // where BorderLines will be system-dependently snapped to have at
    // least a single discrete width per partial line (there may be up to
    // three) works well nowadays due to most renderers moving the AA stuff
    // by 0.5 pixels (discrete units) to match well with the non-AAed parts.
    //
    // Env-Switch for steering this, default is off.
    // Enable by setting at all (and to something)
    static const char* pSwitchOffAntiAliasingForHorVerBorderlines(
        getenv("SAL_SWITCH_OFF_ANTIALIASING_FOR_HOR_VER_BORTDERLINES"));
    static bool bSwitchOffAntiAliasingForHorVerBorderlines(
        nullptr != pSwitchOffAntiAliasingForHorVerBorderlines);

    if (bSwitchOffAntiAliasingForHorVerBorderlines
        && rBorder.isHorizontalOrVertical(getViewInformation2D()))
    {
        AntialiasingFlags nAntiAliasing = mpOutputDevice->GetAntialiasing();
        mpOutputDevice->SetAntialiasing(nAntiAliasing & ~AntialiasingFlags::Enable);
        process(rBorder);
        mpOutputDevice->SetAntialiasing(nAntiAliasing);
    }
    else
    {
        process(rBorder);
    }
}

void VclPixelProcessor2D::processInvertPrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    // invert primitive (currently only used for HighContrast fallback for selection in SW and SC).
    // (Not true, also used at least for the drawing of dragged column and row boundaries in SC.)
    // Set OutDev to XOR and switch AA off (XOR does not work with AA)
    mpOutputDevice->Push();
    mpOutputDevice->SetRasterOp(RasterOp::Xor);
    const AntialiasingFlags nAntiAliasing(mpOutputDevice->GetAntialiasing());
    mpOutputDevice->SetAntialiasing(nAntiAliasing & ~AntialiasingFlags::Enable);

    // process content recursively
    process(rCandidate);

    // restore OutDev
    mpOutputDevice->Pop();
    mpOutputDevice->SetAntialiasing(nAntiAliasing);
}

void VclPixelProcessor2D::processMetaFilePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    // #i98289#
    const bool bForceLineSnap(getViewInformation2D().getPixelSnapHairline());
    const AntialiasingFlags nOldAntiAliase(mpOutputDevice->GetAntialiasing());

    if (bForceLineSnap)
    {
        mpOutputDevice->SetAntialiasing(nOldAntiAliase | AntialiasingFlags::PixelSnapHairline);
    }

    process(rCandidate);

    if (bForceLineSnap)
    {
        mpOutputDevice->SetAntialiasing(nOldAntiAliase);
    }
}

void VclPixelProcessor2D::processFillGradientPrimitive2D(
    const primitive2d::FillGradientPrimitive2D& rPrimitive)
{
    const attribute::FillGradientAttribute& rFillGradient = rPrimitive.getFillGradient();
    bool useDecompose(false);

    // MCGR: *many* - and not only GradientStops - cases cannot be handled by VCL
    // so use decomposition
    if (rFillGradient.cannotBeHandledByVCL())
    {
        useDecompose = true;
    }

    // tdf#149754 VCL gradient draw is not capable to handle all primitive gradient definitions,
    // what should be clear due to being developed to extend/replace them in
    // capabilities & precision.
    // It is e.g. not capable to correctly paint if the OutputRange is not completely
    // inside the DefinitionRange, thus forcing to paint gradient parts *outside* the
    // DefinitionRange.
    // This happens for Writer with Frames anchored in Frames (and was broken due to
    // falling back to VCL Gradient paint here), and for the new SlideBackgroundFill
    // Fill mode for objects using it that reach outside the page (which is the
    // DefinitionRange in that case).
    // I see no real reason to fallback here to OutputDevice::DrawGradient and VCL
    // gradient paint at all (system-dependent renderers wouldn't in the future), but
    // will for convenience only add that needed additional correcting case
    if (!useDecompose && !rPrimitive.getDefinitionRange().isInside(rPrimitive.getOutputRange()))
    {
        useDecompose = true;
    }

    // tdf#151081 need to use regular primitive decomposition when the gradient
    // is transformed in any other way then just translate & scale
    if (!useDecompose)
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;

        maCurrentTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

        // detect if transformation is rotated, sheared or mirrored in X and/or Y
        if (!basegfx::fTools::equalZero(fRotate) || !basegfx::fTools::equalZero(fShearX)
            || aScale.getX() < 0.0 || aScale.getY() < 0.0)
        {
            useDecompose = true;
        }
    }

    if (useDecompose)
    {
        // default is to use the direct render below. For security,
        // keep the (simple) fallback to decompose in place here
        static bool bTryDirectRender(true);

        if (bTryDirectRender)
        {
            // MCGR: Avoid one level of primitive creation, use FillGradientPrimitive2D
            // tooling to directly create needed geoemtry & color for getting better
            // performance (partially compensate for potentially more expensive multi
            // color gradients).
            // To handle a primitive that needs paint, either use decompose, or - when you
            // do not want that for any reason, e.g. extra primitives created - implement
            // a direct handling in your primitive renderer. This is always possible
            // since primitives by definition are self-contained what means they have all
            // needed data locally available to do so.
            // The question is the complexity to invest - the implemented decompose
            // is always a good hint what is neeed to do this. In this case I decided
            // to add some tooling methods to the primitive itself to support this. These
            // are used in decompose and can be used - as here now - for direct handling,
            // too. This is always a possibility in primitive handling - you can, but do not
            // have to.
            mpOutputDevice->SetFillColor(
                Color(maBColorModifierStack.getModifiedColor(rPrimitive.getOuterColor())));
            mpOutputDevice->SetLineColor();
            mpOutputDevice->DrawTransparent(
                maCurrentTransformation,
                basegfx::B2DPolyPolygon(
                    basegfx::utils::createPolygonFromRect(rPrimitive.getOutputRange())),
                0.0);

            // paint solid fill steps by providing callback as lambda
            auto aCallback([&rPrimitive, this](const basegfx::B2DHomMatrix& rMatrix,
                                               const basegfx::BColor& rColor) {
                // create part polygon
                basegfx::B2DPolygon aNewPoly(rPrimitive.getUnitPolygon());
                aNewPoly.transform(rMatrix);

                // create solid fill
                mpOutputDevice->SetFillColor(Color(maBColorModifierStack.getModifiedColor(rColor)));
                mpOutputDevice->DrawTransparent(maCurrentTransformation,
                                                basegfx::B2DPolyPolygon(aNewPoly), 0.0);
            });

            // call value generator to trigger callbacks
            rPrimitive.generateMatricesAndColors(aCallback);
        }
        else
        {
            // use the decompose
            process(rPrimitive);
        }

        return;
    }

    // try to use vcl - since vcl uses the old gradient paint mechanisms this may
    // create wrong geometries. If so, add another case above for useDecompose
    GradientStyle eGradientStyle = convertGradientStyle(rFillGradient.getStyle());

    Gradient aGradient(eGradientStyle, Color(rFillGradient.getColorStops().front().getStopColor()),
                       Color(rFillGradient.getColorStops().back().getStopColor()));

    aGradient.SetAngle(Degree10(static_cast<int>(basegfx::rad2deg<10>(rFillGradient.getAngle()))));
    aGradient.SetBorder(rFillGradient.getBorder() * 100);
    aGradient.SetOfsX(rFillGradient.getOffsetX() * 100.0);
    aGradient.SetOfsY(rFillGradient.getOffsetY() * 100.0);
    aGradient.SetSteps(rFillGradient.getSteps());

    basegfx::B2DRange aOutputRange(rPrimitive.getOutputRange());
    aOutputRange.transform(maCurrentTransformation);
    basegfx::B2DRange aFullRange(rPrimitive.getDefinitionRange());
    aFullRange.transform(maCurrentTransformation);

    const tools::Rectangle aOutputRectangle(
        std::floor(aOutputRange.getMinX()), std::floor(aOutputRange.getMinY()),
        std::ceil(aOutputRange.getMaxX()), std::ceil(aOutputRange.getMaxY()));
    const tools::Rectangle aFullRectangle(
        std::floor(aFullRange.getMinX()), std::floor(aFullRange.getMinY()),
        std::ceil(aFullRange.getMaxX()), std::ceil(aFullRange.getMaxY()));

    mpOutputDevice->Push(vcl::PushFlags::CLIPREGION);
    mpOutputDevice->IntersectClipRegion(aOutputRectangle);
    mpOutputDevice->DrawGradient(aFullRectangle, aGradient);
    mpOutputDevice->Pop();
}

void VclPixelProcessor2D::processPatternFillPrimitive2D(
    const primitive2d::PatternFillPrimitive2D& rPrimitive)
{
    const basegfx::B2DRange& rReferenceRange = rPrimitive.getReferenceRange();
    if (rReferenceRange.isEmpty() || rReferenceRange.getWidth() <= 0.0
        || rReferenceRange.getHeight() <= 0.0)
        return;

    basegfx::B2DPolyPolygon aMask = rPrimitive.getMask();
    aMask.transform(maCurrentTransformation);
    const basegfx::B2DRange aMaskRange(aMask.getB2DRange());

    if (aMaskRange.isEmpty() || aMaskRange.getWidth() <= 0.0 || aMaskRange.getHeight() <= 0.0)
        return;

    sal_uInt32 nTileWidth, nTileHeight;
    rPrimitive.getTileSize(nTileWidth, nTileHeight, getViewInformation2D());
    if (nTileWidth == 0 || nTileHeight == 0)
        return;
    BitmapEx aTileImage = rPrimitive.createTileImage(nTileWidth, nTileHeight);
    tools::Rectangle aMaskRect = vcl::unotools::rectangleFromB2DRectangle(aMaskRange);

    // Unless smooth edges are needed, simply use clipping.
    if (basegfx::utils::isRectangle(aMask) || !getViewInformation2D().getUseAntiAliasing())
    {
        mpOutputDevice->Push(vcl::PushFlags::CLIPREGION);
        mpOutputDevice->IntersectClipRegion(vcl::Region(aMask));
        Wallpaper aWallpaper(aTileImage);
        aWallpaper.SetColor(COL_TRANSPARENT);
        mpOutputDevice->DrawWallpaper(aMaskRect, aWallpaper);
        mpOutputDevice->Pop();
        return;
    }

    impBufferDevice aBufferDevice(*mpOutputDevice, aMaskRange);

    if (!aBufferDevice.isVisible())
        return;

    // remember last OutDev and set to content
    OutputDevice* pLastOutputDevice = mpOutputDevice;
    mpOutputDevice = &aBufferDevice.getContent();

    // if the tile is a single pixel big, just flood fill with that pixel color
    if (nTileWidth == 1 && nTileHeight == 1)
    {
        Color col = aTileImage.GetPixelColor(0, 0);
        mpOutputDevice->SetLineColor(col);
        mpOutputDevice->SetFillColor(col);
        mpOutputDevice->DrawRect(aMaskRect);
    }
    else
    {
        Wallpaper aWallpaper(aTileImage);
        aWallpaper.SetColor(COL_TRANSPARENT);
        mpOutputDevice->DrawWallpaper(aMaskRect, aWallpaper);
    }

    // back to old OutDev
    mpOutputDevice = pLastOutputDevice;

    // draw mask
    VirtualDevice& rMask = aBufferDevice.getTransparence();
    rMask.SetLineColor();
    rMask.SetFillColor(COL_BLACK);
    rMask.DrawPolyPolygon(aMask);

    // dump buffer to outdev
    aBufferDevice.paint();
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
