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
#include <vcl/BitmapBasicMorphologyFilter.hxx>
#include <vcl/BitmapFilterStackBlur.hxx>
#include <vcl/outdev.hxx>
#include <vcl/hatch.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
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
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XControl.hpp>

#include <svtools/optionsdrawinglayer.hxx>
#include <vcl/gradient.hxx>

using namespace com::sun::star;

namespace drawinglayer::processor2d
{
struct VclPixelProcessor2D::Impl
{
    AntialiasingFlags m_nOrigAntiAliasing;

    explicit Impl(OutputDevice const& rOutDev)
        : m_nOrigAntiAliasing(rOutDev.GetAntialiasing())
    {
    }
};

VclPixelProcessor2D::VclPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                                         OutputDevice& rOutDev,
                                         const basegfx::BColorModifierStack& rInitStack)
    : VclProcessor2D(rViewInformation, rOutDev, rInitStack)
    , m_pImpl(new Impl(rOutDev))
{
    // prepare maCurrentTransformation matrix with viewTransformation to target directly to pixels
    maCurrentTransformation = rViewInformation.getObjectToViewTransformation();

    // prepare output directly to pixels
    mpOutputDevice->Push(vcl::PushFlags::MAPMODE);
    mpOutputDevice->SetMapMode();

    // react on AntiAliasing settings
    if (SvtOptionsDrawinglayer::IsAntiAliasing())
    {
        mpOutputDevice->SetAntialiasing(m_pImpl->m_nOrigAntiAliasing | AntialiasingFlags::Enable);
    }
    else
    {
        mpOutputDevice->SetAntialiasing(m_pImpl->m_nOrigAntiAliasing & ~AntialiasingFlags::Enable);
    }
}

VclPixelProcessor2D::~VclPixelProcessor2D()
{
    // restore MapMode
    mpOutputDevice->Pop();

    // restore AntiAliasing
    mpOutputDevice->SetAntialiasing(m_pImpl->m_nOrigAntiAliasing);
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
        case PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D:
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
        case PRIMITIVE2D_ID_GLOWPRIMITIVE2D:
        {
            processGlowPrimitive2D(
                static_cast<const drawinglayer::primitive2d::GlowPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SOFTEDGEPRIMITIVE2D:
        {
            processSoftEdgePrimitive2D(
                static_cast<const drawinglayer::primitive2d::SoftEdgePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D:
        {
            processShadowPrimitive2D(
                static_cast<const drawinglayer::primitive2d::ShadowPrimitive2D&>(rCandidate));
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
    // direct draw of gradient
    const attribute::FillGradientAttribute& rGradient(rPolygonCandidate.getFillGradient());
    basegfx::BColor aStartColor(maBColorModifierStack.getModifiedColor(rGradient.getStartColor()));
    basegfx::BColor aEndColor(maBColorModifierStack.getModifiedColor(rGradient.getEndColor()));
    basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());

    if (!aLocalPolyPolygon.count())
        return;

    if (aStartColor == aEndColor)
    {
        // no gradient at all, draw as polygon in AA and non-AA case
        aLocalPolyPolygon.transform(maCurrentTransformation);
        mpOutputDevice->SetLineColor();
        mpOutputDevice->SetFillColor(Color(aStartColor));
        mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
    }
    else
    {
        // use the primitive decomposition of the metafile
        process(rPolygonCandidate);
    }
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
    if (!(mnPolygonStrokePrimitive2D && SvtOptionsDrawinglayer::IsAntiAliasing()
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
            const primitive2d::Primitive2DReference xReference(rContent[0]);
            const primitive2d::BasePrimitive2D* pBasePrimitive
                = static_cast<const primitive2d::BasePrimitive2D*>(xReference.get());

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
    if (SvtOptionsDrawinglayer::IsAntiAliasing())
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
    const bool bForceLineSnap(SvtOptionsDrawinglayer::IsAntiAliasing()
                              && SvtOptionsDrawinglayer::IsSnapHorVerLinesToDiscrete());
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

namespace
{
/* Returns 8-bit alpha mask created from passed mask.

   Negative fErodeDilateRadius values mean erode, positive - dilate.
   nTransparency defines minimal transparency level.
*/
AlphaMask ProcessAndBlurAlphaMask(const Bitmap& rMask, double fErodeDilateRadius,
                                  double fBlurRadius, sal_uInt8 nTransparency)
{
    // Only completely white pixels on the initial mask must be considered for transparency. Any
    // other color must be treated as black. This creates 1-bit B&W bitmap.
    BitmapEx mask(rMask.CreateMask(COL_WHITE));

    // Scaling down increases performance without noticeable quality loss. Additionally,
    // current blur implementation can only handle blur radius between 2 and 254.
    Size aSize = mask.GetSizePixel();
    double fScale = 1.0;
    while (fBlurRadius > 254 || aSize.Height() > 1000 || aSize.Width() > 1000)
    {
        fScale /= 2;
        fBlurRadius /= 2;
        fErodeDilateRadius /= 2;
        aSize.setHeight(aSize.Height() / 2);
        aSize.setWidth(aSize.Width() / 2);
    }

    // BmpScaleFlag::Fast is important for following color replacement
    mask.Scale(fScale, fScale, BmpScaleFlag::Fast);

    if (fErodeDilateRadius > 0)
        BitmapFilter::Filter(mask, BitmapDilateFilter(fErodeDilateRadius));
    else if (fErodeDilateRadius < 0)
        BitmapFilter::Filter(mask, BitmapErodeFilter(-fErodeDilateRadius, 0xFF));

    if (nTransparency)
    {
        const Color aTransparency(nTransparency, nTransparency, nTransparency);
        mask.Replace(COL_BLACK, aTransparency);
    }

    // We need 8-bit grey mask for blurring
    mask.Convert(BmpConversion::N8BitGreys);

    // calculate blurry effect
    BitmapFilter::Filter(mask, BitmapFilterStackBlur(fBlurRadius));

    mask.Scale(rMask.GetSizePixel());

    return AlphaMask(mask.GetBitmap());
}
}

void VclPixelProcessor2D::processGlowPrimitive2D(const primitive2d::GlowPrimitive2D& rCandidate)
{
    basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));
    aRange.transform(maCurrentTransformation);
    basegfx::B2DVector aGlowRadiusVector(rCandidate.getGlowRadius(), 0);
    // Calculate the pixel size of glow radius in current transformation
    aGlowRadiusVector *= maCurrentTransformation;
    // Glow radius is the size of the halo from each side of the object. The halo is the
    // border of glow color that fades from glow transparency level to fully transparent
    // When blurring a sharp boundary (our case), it gets 50% of original intensity, and
    // fades to both sides by the blur radius; thus blur radius is half of glow radius.
    const double fBlurRadius = aGlowRadiusVector.getLength() / 2;
    // Consider glow transparency (initial transparency near the object edge)
    const sal_uInt8 nAlpha = rCandidate.getGlowColor().GetAlpha();

    impBufferDevice aBufferDevice(*mpOutputDevice, aRange);
    if (aBufferDevice.isVisible())
    {
        // remember last OutDev and set to content
        OutputDevice* pLastOutputDevice = mpOutputDevice;
        mpOutputDevice = &aBufferDevice.getContent();
        // We don't need antialiased mask here, which would only make effect thicker
        const auto aPrevAA = mpOutputDevice->GetAntialiasing();
        mpOutputDevice->SetAntialiasing(AntialiasingFlags::NONE);
        process(rCandidate);

        // Limit the bitmap size to the visible area.
        basegfx::B2DRange viewRange(getViewInformation2D().getDiscreteViewport());
        basegfx::B2DRange bitmapRange(aRange);
        bitmapRange.intersect(viewRange);
        if (!bitmapRange.isEmpty())
        {
            const tools::Rectangle aRect(
                static_cast<tools::Long>(std::floor(bitmapRange.getMinX())),
                static_cast<tools::Long>(std::floor(bitmapRange.getMinY())),
                static_cast<tools::Long>(std::ceil(bitmapRange.getMaxX())),
                static_cast<tools::Long>(std::ceil(bitmapRange.getMaxY())));
            BitmapEx bmpEx = mpOutputDevice->GetBitmapEx(aRect.TopLeft(), aRect.GetSize());
            mpOutputDevice->SetAntialiasing(aPrevAA);

            AlphaMask mask
                = ProcessAndBlurAlphaMask(bmpEx.GetAlpha(), fBlurRadius, fBlurRadius, 255 - nAlpha);

            // The end result is the bitmap filled with glow color and blurred 8-bit alpha mask
            const basegfx::BColor aGlowColor(
                maBColorModifierStack.getModifiedColor(rCandidate.getGlowColor().getBColor()));
            Bitmap bmp = bmpEx.GetBitmap();
            bmp.Erase(Color(aGlowColor));
            BitmapEx result(bmp, mask);

            // back to old OutDev
            mpOutputDevice = pLastOutputDevice;
            mpOutputDevice->DrawBitmapEx(aRect.TopLeft(), result);
        }
        else
        {
            mpOutputDevice = pLastOutputDevice;
        }
    }
    else
        SAL_WARN("drawinglayer", "Temporary buffered virtual device is not visible");
}

void VclPixelProcessor2D::processSoftEdgePrimitive2D(
    const primitive2d::SoftEdgePrimitive2D& rCandidate)
{
    // TODO: don't limit the object at view range. This is needed to not blur objects at window
    // borders, where they don't end. Ideally, process the full object once at maximal reasonable
    // resolution, and store the resulting alpha mask in primitive's cache; then reuse it later,
    // applying the transform.
    basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));
    aRange.transform(maCurrentTransformation);
    basegfx::B2DVector aRadiusVector(rCandidate.getRadius(), 0);
    // Calculate the pixel size of soft edge radius in current transformation
    aRadiusVector *= maCurrentTransformation;
    // Blur radius is equal to soft edge radius
    const double fBlurRadius = aRadiusVector.getLength();

    impBufferDevice aBufferDevice(*mpOutputDevice, aRange);
    if (aBufferDevice.isVisible())
    {
        // remember last OutDev and set to content
        OutputDevice* pLastOutputDevice = mpOutputDevice;
        mpOutputDevice = &aBufferDevice.getContent();
        // Since the effect converts all children to bitmap, we can't disable antialiasing here,
        // because it would result in poor quality in areas not affected by the effect
        process(rCandidate);

        // Limit the bitmap size to the visible area.
        basegfx::B2DRange viewRange(getViewInformation2D().getDiscreteViewport());
        basegfx::B2DRange bitmapRange(aRange);
        bitmapRange.intersect(viewRange);
        if (!bitmapRange.isEmpty())
        {
            const tools::Rectangle aRect(
                static_cast<tools::Long>(std::floor(bitmapRange.getMinX())),
                static_cast<tools::Long>(std::floor(bitmapRange.getMinY())),
                static_cast<tools::Long>(std::ceil(bitmapRange.getMaxX())),
                static_cast<tools::Long>(std::ceil(bitmapRange.getMaxY())));
            BitmapEx bitmap = mpOutputDevice->GetBitmapEx(aRect.TopLeft(), aRect.GetSize());

            AlphaMask aMask = bitmap.GetAlpha();
            AlphaMask blurMask = ProcessAndBlurAlphaMask(aMask, -fBlurRadius, fBlurRadius, 0);

            aMask.BlendWith(blurMask);

            // The end result is the original bitmap with blurred 8-bit alpha mask
            BitmapEx result(bitmap.GetBitmap(), aMask);

            // back to old OutDev
            mpOutputDevice = pLastOutputDevice;
            mpOutputDevice->DrawBitmapEx(aRect.TopLeft(), result);
        }
        else
        {
            mpOutputDevice = pLastOutputDevice;
        }
    }
    else
        SAL_WARN("drawinglayer", "Temporary buffered virtual device is not visible");
}

void VclPixelProcessor2D::processShadowPrimitive2D(const primitive2d::ShadowPrimitive2D& rCandidate)
{
    if (rCandidate.getShadowBlur() == 0)
    {
        process(rCandidate);
        return;
    }

    basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));
    aRange.transform(maCurrentTransformation);
    basegfx::B2DVector aBlurRadiusVector(rCandidate.getShadowBlur(), 0);
    aBlurRadiusVector *= maCurrentTransformation;
    const double fBlurRadius = aBlurRadiusVector.getLength();

    impBufferDevice aBufferDevice(*mpOutputDevice, aRange);
    if (aBufferDevice.isVisible() && !aRange.isEmpty())
    {
        OutputDevice* pLastOutputDevice = mpOutputDevice;
        mpOutputDevice = &aBufferDevice.getContent();

        process(rCandidate);

        const tools::Rectangle aRect(static_cast<tools::Long>(std::floor(aRange.getMinX())),
                                     static_cast<tools::Long>(std::floor(aRange.getMinY())),
                                     static_cast<tools::Long>(std::ceil(aRange.getMaxX())),
                                     static_cast<tools::Long>(std::ceil(aRange.getMaxY())));

        BitmapEx bitmapEx = mpOutputDevice->GetBitmapEx(aRect.TopLeft(), aRect.GetSize());

        AlphaMask mask = ProcessAndBlurAlphaMask(bitmapEx.GetAlpha(), 0, fBlurRadius, 0);

        const basegfx::BColor aShadowColor(
            maBColorModifierStack.getModifiedColor(rCandidate.getShadowColor()));

        Bitmap bitmap = bitmapEx.GetBitmap();
        bitmap.Erase(Color(aShadowColor));
        BitmapEx result(bitmap, mask);

        mpOutputDevice = pLastOutputDevice;
        mpOutputDevice->DrawBitmapEx(aRect.TopLeft(), result);
    }
    else
        SAL_WARN("drawinglayer", "Temporary buffered virtual device is not visible");
}

void VclPixelProcessor2D::processFillGradientPrimitive2D(
    const primitive2d::FillGradientPrimitive2D& rPrimitive)
{
    const attribute::FillGradientAttribute& rFillGradient = rPrimitive.getFillGradient();

    // VCL should be able to handle all styles, but for tdf#133477 the VCL result
    // is different from processing the gradient manually by drawinglayer
    // (and the Writer unittest for it fails). Keep using the drawinglayer code
    // until somebody founds out what's wrong and fixes it.
    if (rFillGradient.getStyle() != drawinglayer::attribute::GradientStyle::Linear
        && rFillGradient.getStyle() != drawinglayer::attribute::GradientStyle::Axial
        && rFillGradient.getStyle() != drawinglayer::attribute::GradientStyle::Radial)
    {
        process(rPrimitive);
        return;
    }

    GradientStyle eGradientStyle = convertGradientStyle(rFillGradient.getStyle());

    Gradient aGradient(eGradientStyle, Color(rFillGradient.getStartColor()),
                       Color(rFillGradient.getEndColor()));

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
    if (basegfx::utils::isRectangle(aMask) || !SvtOptionsDrawinglayer::IsAntiAliasing())
    {
        mpOutputDevice->Push(vcl::PushFlags::CLIPREGION);
        mpOutputDevice->IntersectClipRegion(vcl::Region(aMask));
        mpOutputDevice->DrawWallpaper(aMaskRect, Wallpaper(aTileImage));
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
        mpOutputDevice->DrawWallpaper(aMaskRect, Wallpaper(aTileImage));

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
