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
#include "helperwrongspellrenderer.hxx"
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/hatch.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <cstdio>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <vcl/window.hxx>
#include <svtools/borderhelper.hxx>
#include <editeng/borderline.hxx>

#include <com/sun/star/table/BorderLineStyle.hpp>

using namespace com::sun::star;

namespace drawinglayer
{
    namespace processor2d
    {
        struct VclPixelProcessor2D::Impl
        {
            AntialiasingFlags m_nOrigAntiAliasing;

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
            mpOutputDevice->Push(PushFlags::MAPMODE);
            mpOutputDevice->SetMapMode();

            // react on AntiAliasing settings
            if(getOptionsDrawinglayer().IsAntiAliasing())
            {
                mpOutputDevice->SetAntialiasing(
                   m_pImpl->m_nOrigAntiAliasing | AntialiasingFlags::EnableB2dDraw);
            }
            else
            {
                mpOutputDevice->SetAntialiasing(
                   m_pImpl->m_nOrigAntiAliasing & ~AntialiasingFlags::EnableB2dDraw);
            }
        }

        VclPixelProcessor2D::~VclPixelProcessor2D()
        {
            // restore MapMode
            mpOutputDevice->Pop();

            // restore AntiAliasing
            mpOutputDevice->SetAntialiasing(m_pImpl->m_nOrigAntiAliasing);
        }

        void VclPixelProcessor2D::tryDrawPolyPolygonColorPrimitive2DDirect(const drawinglayer::primitive2d::PolyPolygonColorPrimitive2D& rSource, double fTransparency)
        {
            if(!rSource.getB2DPolyPolygon().count())
            {
                // no geometry, done
                return;
            }

            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rSource.getBColor()));

            mpOutputDevice->SetFillColor(Color(aPolygonColor));
            mpOutputDevice->SetLineColor();
            mpOutputDevice->DrawTransparent(
                maCurrentTransformation,
                rSource.getB2DPolyPolygon(),
                fTransparency);
        }

        bool VclPixelProcessor2D::tryDrawPolygonHairlinePrimitive2DDirect(const drawinglayer::primitive2d::PolygonHairlinePrimitive2D& rSource, double fTransparency)
        {
            const basegfx::B2DPolygon& rLocalPolygon(rSource.getB2DPolygon());

            if(!rLocalPolygon.count())
            {
                // no geometry, done
                return true;
            }

            const basegfx::BColor aLineColor(maBColorModifierStack.getModifiedColor(rSource.getBColor()));

            mpOutputDevice->SetFillColor();
            mpOutputDevice->SetLineColor(Color(aLineColor));
            //aLocalPolygon.transform(maCurrentTransformation);

            // try drawing; if it did not work, use standard fallback
            return mpOutputDevice->DrawPolyLineDirect(
                maCurrentTransformation,
                rLocalPolygon,
                0.0,
                fTransparency);
        }

        bool VclPixelProcessor2D::tryDrawPolygonStrokePrimitive2DDirect(const drawinglayer::primitive2d::PolygonStrokePrimitive2D& rSource, double fTransparency)
        {
            if(!rSource.getB2DPolygon().count())
            {
                // no geometry, done
                return true;
            }

            // get geometry data, prepare hairline data
            const basegfx::B2DPolygon& aLocalPolygon(rSource.getB2DPolygon());
            basegfx::B2DPolyPolygon aHairLinePolyPolygon;

            // simplify curve segments
            // moved to PolygonStrokePrimitive2D::PolygonStrokePrimitive2D
            // aLocalPolygon = basegfx::utils::simplifyCurveSegments(aLocalPolygon);

            if(rSource.getStrokeAttribute().isDefault() || 0.0 == rSource.getStrokeAttribute().getFullDotDashLen())
            {
                // no line dashing, just copy
                aHairLinePolyPolygon.append(aLocalPolygon);
            }
            else
            {
                // apply LineStyle
                basegfx::utils::applyLineDashing(
                    aLocalPolygon,
                    rSource.getStrokeAttribute().getDotDashArray(),
                    &aHairLinePolyPolygon,
                    nullptr,
                    rSource.getStrokeAttribute().getFullDotDashLen());
            }

            if(!aHairLinePolyPolygon.count())
            {
                // no geometry, done
                return true;
            }

            // check if LineWidth can be simplified in world coordinates
            double fLineWidth(rSource.getLineAttribute().getWidth());

            if(basegfx::fTools::more(fLineWidth, 0.0))
            {
                basegfx::B2DVector aLineWidth(fLineWidth, 0.0);
                aLineWidth = maCurrentTransformation * aLineWidth;
                const double fWorldLineWidth(aLineWidth.getLength());

                // draw simple hairline for small line widths
                // see also RenderPolygonStrokePrimitive2D which is used if this try fails
                bool bIsAntiAliasing = getOptionsDrawinglayer().IsAntiAliasing();
                if (   (basegfx::fTools::lessOrEqual(fWorldLineWidth, 1.0) && bIsAntiAliasing)
                    || (basegfx::fTools::lessOrEqual(fWorldLineWidth, 1.5) && !bIsAntiAliasing))
                {
                    // draw simple hairline
                    fLineWidth = 0.0;
                }
            }

            const basegfx::BColor aLineColor(
                maBColorModifierStack.getModifiedColor(
                    rSource.getLineAttribute().getColor()));

            mpOutputDevice->SetFillColor();
            mpOutputDevice->SetLineColor(Color(aLineColor));

            // do not transform self
            // aHairLinePolyPolygon.transform(maCurrentTransformation);

            bool bHasPoints(false);
            bool bTryWorked(false);

            for(sal_uInt32 a(0); a < aHairLinePolyPolygon.count(); a++)
            {
                const basegfx::B2DPolygon& aSingle(aHairLinePolyPolygon.getB2DPolygon(a));

                if(aSingle.count())
                {
                    bHasPoints = true;

                    if(mpOutputDevice->DrawPolyLineDirect(
                        maCurrentTransformation,
                        aSingle,
                        fLineWidth,
                        fTransparency,
                        rSource.getLineAttribute().getLineJoin(),
                        rSource.getLineAttribute().getLineCap(),
                        rSource.getLineAttribute().getMiterMinimumAngle()
                        /* false bBypassAACheck, default*/))
                    {
                        bTryWorked = true;
                    }
                }
            }

            if(!bTryWorked && !bHasPoints)
            {
                // no geometry despite try
                bTryWorked = true;
            }

            return bTryWorked;
        }

        void VclPixelProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                {
                    processWrongSpellPrimitive2D(static_cast<const primitive2d::WrongSpellPrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                {
                    processTextSimplePortionPrimitive2D(static_cast<const primitive2d::TextSimplePortionPrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    processTextDecoratedPortionPrimitive2D(static_cast<const primitive2d::TextSimplePortionPrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    processPolygonHairlinePrimitive2D(static_cast<const primitive2d::PolygonHairlinePrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    // direct draw of transformed BitmapEx primitive
                    processBitmapPrimitive2D(static_cast<const primitive2d::BitmapPrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D :
                {
                    // direct draw of fillBitmapPrimitive
                    RenderFillGraphicPrimitive2D(static_cast< const primitive2d::FillGraphicPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D :
                {
                    processPolyPolygonGradientPrimitive2D(static_cast<const primitive2d::PolyPolygonGradientPrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D :
                {
                    // direct draw of bitmap
                    RenderPolyPolygonGraphicPrimitive2D(static_cast< const primitive2d::PolyPolygonGraphicPrimitive2D& >(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    processPolyPolygonColorPrimitive2D(static_cast<const primitive2d::PolyPolygonColorPrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    processMetaFilePrimitive2D(rCandidate);
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
                    processUnifiedTransparencePrimitive2D(static_cast<const primitive2d::UnifiedTransparencePrimitive2D&>(rCandidate));
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
                    processControlPrimitive2D(static_cast<const primitive2d::ControlPrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                {
                    processPolygonStrokePrimitive2D(static_cast<const primitive2d::PolygonStrokePrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D :
                {
                    processFillHatchPrimitive2D(static_cast<const primitive2d::FillHatchPrimitive2D&>(rCandidate));
                    break;
                }
                case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D :
                {
                    processBackgroundColorPrimitive2D(static_cast<const primitive2d::BackgroundColorPrimitive2D&>(rCandidate));
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
                    processInvertPrimitive2D(rCandidate);
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
                    processBorderLinePrimitive2D(static_cast<const drawinglayer::primitive2d::BorderLinePrimitive2D&>(rCandidate));
                    break;
                }
                default :
                {
                    SAL_INFO("drawinglayer", "default case for " << drawinglayer::primitive2d::idToString(rCandidate.getPrimitive2DID()));
                    // process recursively
                    process(rCandidate);
                    break;
                }
            }
        }

        void VclPixelProcessor2D::processWrongSpellPrimitive2D(const primitive2d::WrongSpellPrimitive2D& rWrongSpellPrimitive)
        {
            if(!renderWrongSpellPrimitive2D(
                rWrongSpellPrimitive,
                *mpOutputDevice,
                maCurrentTransformation,
                maBColorModifierStack))
            {
                // fallback to decomposition (MetaFile)
                process(rWrongSpellPrimitive);
            }
        }

        void VclPixelProcessor2D::processTextSimplePortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rCandidate)
        {
            // Adapt evtl. used special DrawMode
            const DrawModeFlags nOriginalDrawMode(mpOutputDevice->GetDrawMode());
            adaptTextToFillDrawMode();

            if(getOptionsDrawinglayer().IsRenderSimpleTextDirect())
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

        void VclPixelProcessor2D::processTextDecoratedPortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rCandidate)
        {
            // Adapt evtl. used special DrawMode
            const DrawModeFlags nOriginalDrawMode(mpOutputDevice->GetDrawMode());
            adaptTextToFillDrawMode();

            if(getOptionsDrawinglayer().IsRenderDecoratedTextDirect())
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

        void VclPixelProcessor2D::processPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D)
        {
            if(tryDrawPolygonHairlinePrimitive2DDirect(rPolygonHairlinePrimitive2D, 0.0))
            {
                return;
            }

            // direct draw of hairline
            RenderPolygonHairlinePrimitive2D(rPolygonHairlinePrimitive2D, true);
        }

        void VclPixelProcessor2D::processBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
        {
            // check if graphic content is inside discrete local ViewPort
            const basegfx::B2DRange& rDiscreteViewPort(getViewInformation2D().getDiscreteViewport());
            const basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rBitmapCandidate.getTransform());

            if(!rDiscreteViewPort.isEmpty())
            {
                basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);

                aUnitRange.transform(aLocalTransform);

                if(!aUnitRange.overlaps(rDiscreteViewPort))
                {
                    // content is outside discrete local ViewPort
                    return;
                }
            }

            RenderBitmapPrimitive2D(rBitmapCandidate);
        }

        void VclPixelProcessor2D::processPolyPolygonGradientPrimitive2D(const primitive2d::PolyPolygonGradientPrimitive2D& rPolygonCandidate)
        {
            // direct draw of gradient
            const attribute::FillGradientAttribute& rGradient(rPolygonCandidate.getFillGradient());
            basegfx::BColor aStartColor(maBColorModifierStack.getModifiedColor(rGradient.getStartColor()));
            basegfx::BColor aEndColor(maBColorModifierStack.getModifiedColor(rGradient.getEndColor()));
            basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());

            if(aLocalPolyPolygon.count())
            {
                aLocalPolyPolygon.transform(maCurrentTransformation);

                if(aStartColor == aEndColor)
                {
                    // no gradient at all, draw as polygon in AA and non-AA case
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
        }

        void VclPixelProcessor2D::processPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D)
        {
            // try to use directly
            basegfx::B2DPolyPolygon aLocalPolyPolygon;

            tryDrawPolyPolygonColorPrimitive2DDirect(rPolyPolygonColorPrimitive2D, 0.0);
            // okay, done. In this case no gaps should have to be repaired, too

            // when AA is on and this filled polygons are the result of stroked line geometry,
            // draw the geometry once extra as lines to avoid AA 'gaps' between partial polygons
            // Caution: This is needed in both cases (!)
            if(mnPolygonStrokePrimitive2D
                && getOptionsDrawinglayer().IsAntiAliasing()
                && (mpOutputDevice->GetAntialiasing() & AntialiasingFlags::EnableB2dDraw))
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
        }

        void VclPixelProcessor2D::processUnifiedTransparencePrimitive2D(const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate)
        {
            // Detect if a single PolyPolygonColorPrimitive2D is contained; in that case,
            // use the faster OutputDevice::DrawTransparent method
            const primitive2d::Primitive2DContainer& rContent = rUniTransparenceCandidate.getChildren();

            if(!rContent.empty())
            {
                if(0.0 == rUniTransparenceCandidate.getTransparence())
                {
                    // not transparent at all, use content
                    process(rUniTransparenceCandidate.getChildren());
                }
                else if(rUniTransparenceCandidate.getTransparence() > 0.0 && rUniTransparenceCandidate.getTransparence() < 1.0)
                {
                    bool bDrawTransparentUsed(false);

                    if(1 == rContent.size())
                    {
                        const primitive2d::Primitive2DReference xReference(rContent[0]);
                        const primitive2d::BasePrimitive2D* pBasePrimitive = dynamic_cast< const primitive2d::BasePrimitive2D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            switch(pBasePrimitive->getPrimitive2DID())
                            {
                                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
                                {
                                    // single transparent tools::PolyPolygon identified, use directly
                                    const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor = static_cast< const primitive2d::PolyPolygonColorPrimitive2D* >(pBasePrimitive);
                                    OSL_ENSURE(pPoPoColor, "OOps, PrimitiveID and PrimitiveType do not match (!)");
                                    bDrawTransparentUsed = true;
                                    tryDrawPolyPolygonColorPrimitive2DDirect(*pPoPoColor, rUniTransparenceCandidate.getTransparence());
                                    break;
                                }
                                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
                                {
                                    // single transparent PolygonHairlinePrimitive2D identified, use directly
                                    const primitive2d::PolygonHairlinePrimitive2D* pPoHair = static_cast< const primitive2d::PolygonHairlinePrimitive2D* >(pBasePrimitive);
                                    OSL_ENSURE(pPoHair, "OOps, PrimitiveID and PrimitiveType do not match (!)");

                                    // do no tallow by default - problem is that self-overlapping parts of this geometry will
                                    // not be in a all-same transparency but will already alpha-cover themselves with blending.
                                    // This is not what the UnifiedTransparencePrimitive2D defines: It requires all its
                                    // content to be uniformly transparent.
                                    // For hairline the effect is pretty minimal, but still not correct.
                                    bDrawTransparentUsed = false;
                                    break;
                                }
                                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                                {
                                    // single transparent PolygonStrokePrimitive2D identified, use directly
                                    const primitive2d::PolygonStrokePrimitive2D* pPoStroke = static_cast< const primitive2d::PolygonStrokePrimitive2D* >(pBasePrimitive);
                                    OSL_ENSURE(pPoStroke, "OOps, PrimitiveID and PrimitiveType do not match (!)");

                                    // do no tallow by default - problem is that self-overlapping parts of this geometry will
                                    // not be in a all-same transparency but will already alpha-cover themselves with blending.
                                    // This is not what the UnifiedTransparencePrimitive2D defines: It requires all its
                                    // content to be uniformly transparent.
                                    // To check, activate and draw a wide transparent self-crossing line/curve
                                    bDrawTransparentUsed = false;
                                    break;
                                }
                            default:
                                SAL_INFO("drawinglayer", "default case for " << drawinglayer::primitive2d::idToString(rUniTransparenceCandidate.getPrimitive2DID()));
                                break;
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
        }

        void VclPixelProcessor2D::processControlPrimitive2D(const primitive2d::ControlPrimitive2D& rControlPrimitive)
        {
            // control primitive
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
                process(rControlPrimitive);
            }
        }

        void VclPixelProcessor2D::processPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokePrimitive2D)
        {
            // try to use directly
            if(tryDrawPolygonStrokePrimitive2DDirect(rPolygonStrokePrimitive2D, 0.0))
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

            // Lines with 1 and 2 pixel width without AA need special treatment since their vsiualisation
            // as filled polygons is geometrically correct but looks wrong since polygon filling avoids
            // the right and bottom pixels. The used method evaluates that and takes the correct action,
            // including calling recursively with decomposition if line is wide enough
            RenderPolygonStrokePrimitive2D(rPolygonStrokePrimitive2D);

            // restore DrawMode
            mpOutputDevice->SetDrawMode(nOriginalDrawMode);
        }

        void VclPixelProcessor2D::processFillHatchPrimitive2D(const primitive2d::FillHatchPrimitive2D& rFillHatchPrimitive)
        {
            // without AA, use VCL to draw the hatch. It snaps hatch distances to the next pixel
            // and forces hatch distance to be >= 3 pixels to make the hatch display look smoother.
            // This is wrong in principle, but looks nicer. This could also be done here directly
            // without VCL usage if needed
            const attribute::FillHatchAttribute& rFillHatchAttributes = rFillHatchPrimitive.getFillHatch();

            // create hatch polygon in range size and discrete coordinates
            basegfx::B2DRange aHatchRange(rFillHatchPrimitive.getOutputRange());
            aHatchRange.transform(maCurrentTransformation);
            const basegfx::B2DPolygon aHatchPolygon(basegfx::utils::createPolygonFromRect(aHatchRange));

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
            HatchStyle eHatchStyle(HatchStyle::Single);

            switch(rFillHatchAttributes.getStyle())
            {
                default : // HatchStyle::Single
                {
                    break;
                }
                case attribute::HatchStyle::Double :
                {
                    eHatchStyle = HatchStyle::Double;
                    break;
                }
                case attribute::HatchStyle::Triple :
                {
                    eHatchStyle = HatchStyle::Triple;
                    break;
                }
            }

            // create hatch
            const basegfx::B2DVector aDiscreteDistance(maCurrentTransformation * basegfx::B2DVector(rFillHatchAttributes.getDistance(), 0.0));
            const sal_uInt32 nDistance(basegfx::fround(aDiscreteDistance.getLength()));
            const sal_uInt16 nAngle10(static_cast<sal_uInt16>(basegfx::fround(rFillHatchAttributes.getAngle() / F_PI1800)));
            ::Hatch aVCLHatch(eHatchStyle, Color(rFillHatchAttributes.getColor()), nDistance, nAngle10);

            // draw hatch using VCL
            mpOutputDevice->DrawHatch(::tools::PolyPolygon(::tools::Polygon(aHatchPolygon)), aVCLHatch);
        }

        void VclPixelProcessor2D::processBackgroundColorPrimitive2D(const primitive2d::BackgroundColorPrimitive2D& rPrimitive)
        {
            // #i98404# Handle directly, especially when AA is active
            const AntialiasingFlags nOriginalAA(mpOutputDevice->GetAntialiasing());

            // switch AA off in all cases
            mpOutputDevice->SetAntialiasing(mpOutputDevice->GetAntialiasing() & ~AntialiasingFlags::EnableB2dDraw);

            // create color for fill
            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPrimitive.getBColor()));
            Color aFillColor(aPolygonColor);
            aFillColor.SetTransparency(sal_uInt8((rPrimitive.getTransparency() * 255.0) + 0.5));
            mpOutputDevice->SetFillColor(aFillColor);
            mpOutputDevice->SetLineColor();

            // create rectangle for fill
            const basegfx::B2DRange& aViewport(getViewInformation2D().getDiscreteViewport());
            const ::tools::Rectangle aRectangle(
                static_cast<sal_Int32>(floor(aViewport.getMinX())), static_cast<sal_Int32>(floor(aViewport.getMinY())),
                static_cast<sal_Int32>(ceil(aViewport.getMaxX())), static_cast<sal_Int32>(ceil(aViewport.getMaxY())));
            mpOutputDevice->DrawRect(aRectangle);

            // restore AA setting
            mpOutputDevice->SetAntialiasing(nOriginalAA);
        }

        void VclPixelProcessor2D::processBorderLinePrimitive2D(const drawinglayer::primitive2d::BorderLinePrimitive2D& rBorder)
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
            static const char* pSwitchOffAntiAliasingForHorVerBorderlines(getenv("SAL_SWITCH_OFF_ANTIALIASING_FOR_HOR_VER_BORTDERLINES"));
            static bool bSwitchOffAntiAliasingForHorVerBorderlines(nullptr != pSwitchOffAntiAliasingForHorVerBorderlines);

            if (bSwitchOffAntiAliasingForHorVerBorderlines && rBorder.isHorizontalOrVertical(getViewInformation2D()))
            {
                AntialiasingFlags nAntiAliasing = mpOutputDevice->GetAntialiasing();
                mpOutputDevice->SetAntialiasing(nAntiAliasing & ~AntialiasingFlags::EnableB2dDraw);
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
            mpOutputDevice->SetRasterOp( RasterOp::Xor );
            const AntialiasingFlags nAntiAliasing(mpOutputDevice->GetAntialiasing());
            mpOutputDevice->SetAntialiasing(nAntiAliasing & ~AntialiasingFlags::EnableB2dDraw);

            // process content recursively
            process(rCandidate);

            // restore OutDev
            mpOutputDevice->Pop();
            mpOutputDevice->SetAntialiasing(nAntiAliasing);
        }

        void VclPixelProcessor2D::processMetaFilePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            // #i98289#
            const bool bForceLineSnap(getOptionsDrawinglayer().IsAntiAliasing() && getOptionsDrawinglayer().IsSnapHorVerLinesToDiscrete());
            const AntialiasingFlags nOldAntiAliase(mpOutputDevice->GetAntialiasing());

            if(bForceLineSnap)
            {
                mpOutputDevice->SetAntialiasing(nOldAntiAliase | AntialiasingFlags::PixelSnapHairline);
            }

            process(rCandidate);

            if(bForceLineSnap)
            {
                mpOutputDevice->SetAntialiasing(nOldAntiAliase);
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
