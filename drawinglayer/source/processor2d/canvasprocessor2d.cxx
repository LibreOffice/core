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

#include "drawinglayer/processor2d/canvasprocessor2d.hxx"

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/PanoseProportion.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/StrokeAttributes.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>

#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>

#include <canvas/canvastools.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/graph.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include "getdigitlanguage.hxx"

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        //////////////////////////////////////////////////////////////////////////////
        // single primitive renderers

        void CanvasProcessor2D::impRenderTextSimplePortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate)
        {
            if(rTextCandidate.getTextLength())
            {
                const attribute::FontAttribute& rFontAttr(rTextCandidate.getFontAttribute());
                rendering::FontRequest aFontRequest;

                aFontRequest.FontDescription.FamilyName = rFontAttr.getFamilyName();
                aFontRequest.FontDescription.StyleName = rFontAttr.getStyleName();
                aFontRequest.FontDescription.IsSymbolFont = rFontAttr.getSymbol() ? util::TriState_YES : util::TriState_NO;
                aFontRequest.FontDescription.IsVertical = rFontAttr.getVertical() ? util::TriState_YES : util::TriState_NO;
                // TODO(F2): improve vclenum->panose conversion
                aFontRequest.FontDescription.FontDescription.Weight = static_cast< sal_uInt8 >(rFontAttr.getWeight());
                aFontRequest.FontDescription.FontDescription.Proportion =
                    rFontAttr.getMonospaced()
                    ? rendering::PanoseProportion::MONO_SPACED
                    : rendering::PanoseProportion::ANYTHING;
                aFontRequest.FontDescription.FontDescription.Letterform = rFontAttr.getItalic() ? 9 : 0;

                // init CellSize to 1.0, actual height comes TextTransform
                aFontRequest.CellSize = 1.0;
                aFontRequest.Locale = rTextCandidate.getLocale();

                // font matrix should only be used for glyph rotations etc.
                css::geometry::Matrix2D aFontMatrix;
                canvas::tools::setIdentityMatrix2D(aFontMatrix);

                uno::Reference<rendering::XCanvasFont> xFont(
                    mxCanvas->createFont(
                        aFontRequest,
                        uno::Sequence< beans::PropertyValue >(),
                        aFontMatrix));

                if(xFont.is())
                {
                    // got a font, now try to get a TextLayout
                    const rendering::StringContext aStringContext(
                        rTextCandidate.getText(),
                        rTextCandidate.getTextPosition(),
                        rTextCandidate.getTextLength());
                    uno::Reference<rendering::XTextLayout> xLayout(
                        xFont->createTextLayout(
                            aStringContext,
                            rendering::TextDirection::WEAK_LEFT_TO_RIGHT, 0));

                    if(xLayout.is())
                    {
                        // got a text layout, apply DXArray if given
                        const ::std::vector< double >& rDXArray = rTextCandidate.getDXArray();
                        const sal_uInt32 nDXCount(rDXArray.size());

                        // TODO(F3): this needs proper scaling
                        if(false) //nDXCount)
                        {
                            // DXArray does not need to be adapted to getTextPosition/getTextLength,
                            // it is already provided correctly
                            const uno::Sequence< double > aDXSequence(&rDXArray[0], nDXCount);
                            xLayout->applyLogicalAdvancements(aDXSequence);
                        }

                        // set text color
                        const basegfx::BColor aRGBColor(
                            maBColorModifierStack.getModifiedColor(
                                rTextCandidate.getFontColor()));
                        maVolatileRenderState.DeviceColor =
                            aRGBColor.colorToDoubleSequence(mxCanvas->getDevice());

                        // set text transformation
                        canvas::tools::setRenderStateTransform(
                            maVolatileRenderState,
                            getViewInformation2D().getObjectTransformation()
                            * rTextCandidate.getTextTransform());

                        // paint
                        mxCanvas->drawTextLayout(xLayout, maViewState, maVolatileRenderState);

                        // and restore
                        canvas::tools::setRenderStateTransform(
                            maVolatileRenderState,
                            getViewInformation2D().getObjectTransformation());
                    }
                }
            }
        }

        void CanvasProcessor2D::impRenderPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate)
        {
            const basegfx::BColor aHairlineColor(
                maBColorModifierStack.getModifiedColor(
                    rPolygonCandidate.getBColor()));

            maVolatileRenderState.DeviceColor = aHairlineColor.colorToDoubleSequence(
                mxCanvas->getDevice());

            mxCanvas->drawPolyPolygon(
                basegfx::unotools::xPolyPolygonFromB2DPolygon(
                    mxCanvas->getDevice(),
                    rPolygonCandidate.getB2DPolygon()),
                maViewState,  maVolatileRenderState);
        }

        void CanvasProcessor2D::impRenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
        {
            BitmapEx aBitmapEx(rBitmapCandidate.getBitmapEx());
            if(maBColorModifierStack.count())
            {
                aBitmapEx = aBitmapEx.ModifyBitmapEx(maBColorModifierStack);
                if(aBitmapEx.IsEmpty())
                {
                    // replace with color filled polygon
                    const basegfx::BColor aModifiedColor(
                        maBColorModifierStack.getModifiedColor(basegfx::BColor()));
                    const basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());

                    maVolatileRenderState.DeviceColor = aModifiedColor.colorToDoubleSequence(
                        mxCanvas->getDevice());
                    canvas::tools::setRenderStateTransform(
                        maVolatileRenderState,
                        getViewInformation2D().getObjectTransformation() *
                        rBitmapCandidate.getTransform());

                    mxCanvas->fillPolyPolygon(
                        basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                            mxCanvas->getDevice(), basegfx::B2DPolyPolygon(aPolygon)),
                        maViewState,  maVolatileRenderState);

                    // and restore
                    canvas::tools::setRenderStateTransform(
                        maVolatileRenderState,
                        getViewInformation2D().getObjectTransformation());

                    return;
                }
            }

            // adapt object's transformation to the correct scale
            basegfx::B2DVector aScale, aTranslate;
            const Size aSizePixel(aBitmapEx.GetSizePixel());

            if(0 != aSizePixel.Width() && 0 != aSizePixel.Height())
            {
                // scale object transform with bitmap matrix (and
                // align with canvas conventions, which places bitmaps
                // into [0,1]^2 unit range
                canvas::tools::setRenderStateTransform(
                    maVolatileRenderState,
                    getViewInformation2D().getObjectTransformation()
                    * rBitmapCandidate.getTransform()
                    * basegfx::tools::createScaleB2DHomMatrix(
                        basegfx::B2DTuple(1.0/aSizePixel.Width(),
                                          1.0/aSizePixel.Height())) );

                mxCanvas->drawBitmap(
                    vcl::unotools::xBitmapFromBitmapEx(mxCanvas->getDevice(),
                                                       aBitmapEx),
                    maViewState, maVolatileRenderState);

                // and restore
                canvas::tools::setRenderStateTransform(
                    maVolatileRenderState,
                    getViewInformation2D().getObjectTransformation());
            }
        }

        void CanvasProcessor2D::impRenderFillGraphicPrimitive2D(const primitive2d::FillGraphicPrimitive2D& rFillBitmapCandidate)
        {
            // TODO(P2): if we've a plain bitmap fill, use XCanvas
            // texturing. Otherwise, fallback to decomposition
            // if( rFillGraphicAttribute.getTiling()
            //     && GRAPHIC_BITMAP == rFillGraphicAttribute.getGraphic().GetType()
            //     && !rFillGraphicAttribute.getGraphic().getSvgData().get()
            //     && !rFillGraphicAttribute.getGraphic().IsAnimated() )
            // {
            // }
            process(rFillBitmapCandidate.get2DDecomposition(getViewInformation2D()));
        }

        void CanvasProcessor2D::impRenderPolyPolygonGraphicPrimitive2D(const primitive2d::PolyPolygonGraphicPrimitive2D& rPolygonCandidate)
        {
            const basegfx::B2DPolyPolygon& rPolyPolygon = rPolygonCandidate.getB2DPolyPolygon();
            if(!rPolyPolygon.count())
                return; // clearly a no-op

            const attribute::FillGraphicAttribute& rFillGraphicAttribute =
                rPolygonCandidate.getFillGraphic();

            // try to catch cases where the graphic will be color-modified to a single
            // color (e.g. shadow)
            if( GRAPHIC_BITMAP == rFillGraphicAttribute.getGraphic().GetType() )
            {
                if( !rFillGraphicAttribute.getGraphic().IsTransparent()
                    && !rFillGraphicAttribute.getGraphic().IsAlpha())
                {
                    // bitmap is not transparent and has no alpha
                    const sal_uInt32 nBColorModifierStackCount(maBColorModifierStack.count());
                    if(nBColorModifierStackCount)
                    {
                        const basegfx::BColorModifier& rTopmostModifier =
                            maBColorModifierStack.getBColorModifier(
                                nBColorModifierStackCount - 1);

                        if(basegfx::BCOLORMODIFYMODE_REPLACE == rTopmostModifier.getMode())
                        {
                            // the bitmap fill is in unified color, so we can replace it with
                            // a single polygon fill. The form of the fill depends on tiling
                            if(rFillGraphicAttribute.getTiling())
                            {
                                // with tiling, fill the whole PolyPolygon with the modifier color
                                const basegfx::BColor aPolygonColor(
                                    rTopmostModifier.getBColor());

                                maVolatileRenderState.DeviceColor =
                                    aPolygonColor.colorToDoubleSequence(
                                        mxCanvas->getDevice());

                                mxCanvas->fillPolyPolygon(
                                    basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                        mxCanvas->getDevice(),
                                        rPolyPolygon),
                                    maViewState,  maVolatileRenderState);
                            }
                            else
                            {
                                // without tiling, only the area common to the bitmap tile and the
                                // PolyPolygon is filled. Create the bitmap tile area in object
                                // coordinates. For this, the object transformation needs to be created
                                // from the already scaled PolyPolygon. The tile area in object
                                // coordinates wil always be non-rotated, so it's not necessary to
                                // work with a polygon here
                                basegfx::B2DRange aTileRange(rFillGraphicAttribute.getGraphicRange());
                                const basegfx::B2DRange aPolyPolygonRange(rPolyPolygon.getB2DRange());
                                const basegfx::B2DHomMatrix aNewObjectTransform(
                                    basegfx::tools::createScaleTranslateB2DHomMatrix(
                                        aPolyPolygonRange.getRange(),
                                        aPolyPolygonRange.getMinimum()));

                                aTileRange.transform(aNewObjectTransform);

                                // now clip the object polyPolygon against the tile range
                                // to get the common area
                                basegfx::B2DPolyPolygon aTarget = basegfx::tools::clipPolyPolygonOnRange(
                                    rPolyPolygon,
                                    aTileRange,
                                    true,
                                    false);

                                if(aTarget.count())
                                {
                                    const basegfx::BColor aPolygonColor(
                                        rTopmostModifier.getBColor());

                                    maVolatileRenderState.DeviceColor =
                                        aPolygonColor.colorToDoubleSequence(
                                            mxCanvas->getDevice());

                                    mxCanvas->fillPolyPolygon(
                                        basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                            mxCanvas->getDevice(),
                                            aTarget),
                                        maViewState,  maVolatileRenderState);
                                }
                            }

                            // rendering done
                            return;
                        }
                    }
                }

                // TODO(P2): if we've a plain bitmap fill, use XCanvas
                // texturing. Otherwise, fallback to decomposition
            }

            // use default decomposition, if all else failed
            process(rPolygonCandidate.get2DDecomposition(getViewInformation2D()));
        }

        void CanvasProcessor2D::impRenderPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate)
        {
            const basegfx::BColor aPolygonColor(
                maBColorModifierStack.getModifiedColor(
                    rPolygonCandidate.getBColor()));

            maVolatileRenderState.DeviceColor = aPolygonColor.colorToDoubleSequence(
                mxCanvas->getDevice());

            mxCanvas->fillPolyPolygon(
                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(),
                    rPolygonCandidate.getB2DPolyPolygon()),
                maViewState,  maVolatileRenderState);
        }

        void CanvasProcessor2D::impRenderMaskPrimitive2D(const primitive2d::MaskPrimitive2D& rMaskCandidate)
        {
            const primitive2d::Primitive2DSequence& rChildren = rMaskCandidate.getChildren();
            if(rChildren.hasElements())
            {
                basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

                if(!aMask.count())
                {
                    // no mask, no clipping. just paint content as-is
                    process(rChildren);
                }
                else
                {
                    // transform new mask polygon to view coordinates for processing. All masks
                    // are processed in view coordinates and clipped against each other evtl. to
                    // create multi-clips
                    aMask.transform(getViewInformation2D().getObjectTransformation());

                    // remember last current clip polygon
                    const basegfx::B2DPolyPolygon aLastClipPolyPolygon(maClipPolyPolygon);
                    const uno::Reference< rendering::XPolyPolygon2D > xLastClipPolyPolygon =
                        maVolatileRenderState.Clip;

                    if(maClipPolyPolygon.count())
                    {
                        // there is already a clip polygon set; build clipped union of
                        // current mask polygon and new one
                        maClipPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                            aMask, maClipPolyPolygon, false, false);
                    }
                    else
                    {
                        // use mask directly
                        maClipPolyPolygon = aMask;
                    }

                    // set at ViewState
                    if(maClipPolyPolygon.count())
                    {
                        // set new as clip polygon
                        maVolatileRenderState.Clip = basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                            mxCanvas->getDevice(), maClipPolyPolygon);
                    }
                    else
                    {
                        // empty, reset
                        maVolatileRenderState.Clip.clear();
                    }

                    // now paint children (clipped to aMask)
                    process(rChildren);

                    // restore clip
                    maClipPolyPolygon = aLastClipPolyPolygon;
                    maVolatileRenderState.Clip = xLastClipPolyPolygon;
                }
            }
        }

        void CanvasProcessor2D::impRenderUnifiedTransparencePrimitive2D(const primitive2d::UnifiedTransparencePrimitive2D& rUniTransparenceCandidate)
        {
            if( !rUniTransparenceCandidate.getChildren().hasElements() )
                return; // now, that was easy

            if(0.0 == rUniTransparenceCandidate.getTransparence())
            {
                // not transparent at all, directly use content
                process(rUniTransparenceCandidate.getChildren());
            }
            else if( rUniTransparenceCandidate.getTransparence() > 0.0
                     && rUniTransparenceCandidate.getTransparence() < 1.0 )
            {
                const primitive2d::Primitive2DSequence rChildren =
                    rUniTransparenceCandidate.getChildren();
                if(rChildren.hasElements())
                {
                    // Detect if a single PolyPolygonColorPrimitive2D
                    // is contained; in that case, use the
                    // fillPolyPolygon method with correctly set
                    // transparence. This is a often used case, so
                    // detectiong it is valuable
                    if(1 == rChildren.getLength())
                    {
                        const primitive2d::Primitive2DReference xReference(rChildren[0]);
                        const primitive2d::PolyPolygonColorPrimitive2D* pPoPoColor =
                            dynamic_cast< const primitive2d::PolyPolygonColorPrimitive2D* >(
                                xReference.get());

                        if(pPoPoColor && PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D == pPoPoColor->getPrimitive2DID())
                        {
                            // direct draw of PolyPolygon with color and transparence
                            const basegfx::BColor aPolygonColor(
                                maBColorModifierStack.getModifiedColor(pPoPoColor->getBColor()));

                            // add transparence modulation value to DeviceColor
                            uno::Sequence< double > aColor(4);

                            aColor[0] = aPolygonColor.getRed();
                            aColor[1] = aPolygonColor.getGreen();
                            aColor[2] = aPolygonColor.getBlue();
                            aColor[3] = 1.0 - rUniTransparenceCandidate.getTransparence();
                            maVolatileRenderState.DeviceColor = aColor;

                            mxCanvas->fillPolyPolygon(
                                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                    mxCanvas->getDevice(),
                                    pPoPoColor->getB2DPolyPolygon()),
                                maViewState,  maVolatileRenderState);

                            return; // done
                        }
                    }

                    // process decomposition. This will be decomposed to an TransparencePrimitive2D
                    // with the same child context and a single polygon for transparent context. This could be
                    // directly handled here with known VCL-buffer technology, but would only
                    // make a small difference compared to directly rendering the TransparencePrimitive2D
                    // using impRenderTransparencePrimitive2D above.
                    process(rUniTransparenceCandidate.get2DDecomposition(getViewInformation2D()));
                }
            }
        }

        void CanvasProcessor2D::impRenderTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransCandidate)
        {
            if(rTransCandidate.getChildren().hasElements())
            {
                // this is the full mask primitive - render mask
                // content as alpha, then blit original content
                // through that alphamask.
                //
                // See also http://www.w3.org/TR/SVG/masking.html#Masking

                // how big is the resulting image, in device pixel?
                basegfx::B2DRange aRange(primitive2d::getB2DRangeFromPrimitive2DSequence(
                                             rTransCandidate.getChildren(),
                                             getViewInformation2D()));
                aRange.transform(getViewInformation2D().getViewTransformation()
                                 * getViewInformation2D().getObjectTransformation());

                // just in case clip against viewport, pointless to
                // render outside visible area
                aRange.intersect( getViewInformation2D().getDiscreteViewport() );

                const basegfx::B2IRange aIntRange(
                    canvas::tools::spritePixelAreaFromB2DRange(aRange));
                const css::geometry::IntegerSize2D aMaskSize(
                    aIntRange.getWidth(), aIntRange.getHeight());

                SAL_INFO("drawinglayer.canvas", "TransparencePrimitive2D rendering into ("
                         << aMaskSize.Width << "," << aMaskSize.Height << ") bitmap");

                // TODO(E1): in theory, could also get a float bitmap here...
                uno::Reference< rendering::XIntegerBitmap > xContent(
                    mxCanvas->getDevice()->createCompatibleAlphaBitmap(
                        aMaskSize.Width, aMaskSize.Height),
                    uno::UNO_QUERY_THROW);

                // TODO(E1): in theory, could also get a float bitmap here...
                uno::Reference< rendering::XIntegerBitmap > xMask(
                    mxCanvas->getDevice()->createCompatibleAlphaBitmap(
                        aMaskSize.Width, aMaskSize.Height),
                    uno::UNO_QUERY_THROW);

                // buffer current XCanvas output instance, then divert
                // output to content and mask
                uno::Reference< rendering::XCanvas > xLastCanvas(mxCanvas);
                mxCanvas.set(xContent, uno::UNO_QUERY_THROW);

                // paint content to it
                process(rTransCandidate.getChildren());

                // set to mask
                mxCanvas.set(xMask, uno::UNO_QUERY_THROW);

                // when painting transparence masks, reset the color stack
                basegfx::BColorModifierStack aLastBColorModifierStack(maBColorModifierStack);
                maBColorModifierStack = basegfx::BColorModifierStack();

                // paint mask to it
                process(rTransCandidate.getTransparence());

                // back to old color stack
                maBColorModifierStack = aLastBColorModifierStack;

                // back to old XCanvas instance
                mxCanvas = xLastCanvas;

                // now merge the two bitmaps
                // TODO(P3): the below uses IEEE double images. worst
                // possible performance to blend masks, evar
                rendering::IntegerBitmapLayout aMaskLayout;
                const css::geometry::IntegerRectangle2D aMaskArea(
                    0,0,
                    aMaskSize.Width,
                    aMaskSize.Height);
                uno::Sequence< sal_Int8 > aRawMask =
                    xMask->getData(aMaskLayout,
                                   aMaskArea);
                uno::Sequence< rendering::ARGBColor > aARGBMask =
                    aMaskLayout.ColorSpace->convertIntegerToARGB(aRawMask);

                rendering::IntegerBitmapLayout aContentLayout;
                uno::Sequence< sal_Int8 > aRawContent =
                    xContent->getData(
                        aContentLayout,
                        aMaskArea);
                uno::Sequence< rendering::ARGBColor > aARGBContent =
                    aContentLayout.ColorSpace->convertIntegerToARGB(aRawContent);

                rendering::ARGBColor* pARGBContent( aARGBContent.getArray() );
                const rendering::ARGBColor* const pARGBContentEnd(
                    pARGBContent+aARGBContent.getLength() );
                const rendering::ARGBColor* pARGBMask( aARGBMask.getConstArray() );
                while( pARGBContent != pARGBContentEnd )
                {
                    // blend mask with content alpha - std sRGB luminance formula below
                    pARGBContent->Alpha *=
                        pARGBMask->Alpha * (0.2126*pARGBMask->Red
                                            + 0.7152*pARGBMask->Green
                                            + 0.0722*pARGBMask->Blue);
                    ++pARGBContent;
                    ++pARGBMask;
                }

                xContent->setData(
                    aContentLayout.ColorSpace->convertIntegerFromARGB( aARGBContent ),
                    aContentLayout,
                    aMaskArea);

                // render to target - with identity transformation, we
                // want 1:1 pixel
                basegfx::B2DHomMatrix aIdentMatrix;
                canvas::tools::setViewStateTransform(maViewState,
                                                     aIdentMatrix);
                canvas::tools::setRenderStateTransform(maVolatileRenderState,
                                                       aIdentMatrix);

                mxCanvas->drawBitmap(xContent,
                                     maViewState,
                                     maVolatileRenderState);

                canvas::tools::setViewStateTransform(maViewState,
                                                     getViewInformation2D().getViewTransformation());
                canvas::tools::setRenderStateTransform(maVolatileRenderState,
                                                       getViewInformation2D().getObjectTransformation());
            }
        }

        void CanvasProcessor2D::impRenderTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate)
        {
            // buffer last transform, to later restore it
            const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

            // create new local ViewInformation2D with new transformation
            const geometry::ViewInformation2D aViewInformation2D(
                getViewInformation2D().getObjectTransformation() * rTransformCandidate.getTransformation(),
                getViewInformation2D().getViewTransformation(),
                getViewInformation2D().getViewport(),
                getViewInformation2D().getVisualizedPage(),
                getViewInformation2D().getViewTime(),
                getViewInformation2D().getExtendedInformationSequence());
            updateViewInformation(aViewInformation2D);

            // set at canvas
            canvas::tools::setRenderStateTransform(maVolatileRenderState,
                                                   getViewInformation2D().getObjectTransformation());

            // proccess content
            process(rTransformCandidate.getChildren());

            // restore transformations
            updateViewInformation(aLastViewInformation2D);

            // restore at canvas
            canvas::tools::setRenderStateTransform(maVolatileRenderState,
                                                   getViewInformation2D().getObjectTransformation());
        }

        void CanvasProcessor2D::impRenderPagePreviewPrimitive2D(const primitive2d::PagePreviewPrimitive2D& rPagePreviewCandidate)
        {
            // remember current transformation and ViewInformation
            const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

            // create new local ViewInformation2D
            const geometry::ViewInformation2D aViewInformation2D(
                getViewInformation2D().getObjectTransformation(),
                getViewInformation2D().getViewTransformation(),
                getViewInformation2D().getViewport(),
                rPagePreviewCandidate.getXDrawPage(),
                getViewInformation2D().getViewTime(),
                getViewInformation2D().getExtendedInformationSequence());
            updateViewInformation(aViewInformation2D);

            // proccess decomposed content
            process(rPagePreviewCandidate.get2DDecomposition(getViewInformation2D()));

            // restore transformations
            updateViewInformation(aLastViewInformation2D);
        }

        void CanvasProcessor2D::impRenderPointArrayPrimitive2D(const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate)
        {
            // set point color
            const basegfx::BColor aRGBColor(
                maBColorModifierStack.getModifiedColor(
                    rPointArrayCandidate.getRGBColor()));
            maVolatileRenderState.DeviceColor = aRGBColor.colorToDoubleSequence(
                mxCanvas->getDevice());

            const std::vector< basegfx::B2DPoint >& rPointVector =
                rPointArrayCandidate.getPositions();
            std::vector< basegfx::B2DPoint >::const_iterator aIter=rPointVector.begin();
            const std::vector< basegfx::B2DPoint >::const_iterator aEnd=rPointVector.end();
            for( ; aIter != aEnd; ++aIter )
            {
                mxCanvas->drawPoint(
                    basegfx::unotools::point2DFromB2DPoint(*aIter),
                    maViewState, maVolatileRenderState);
            }
        }

        void CanvasProcessor2D::impRenderPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokePrimitive)
        {
            const attribute::LineAttribute& rLineAttribute =
                rPolygonStrokePrimitive.getLineAttribute();
            const attribute::StrokeAttribute& rStrokeAttribute =
                rPolygonStrokePrimitive.getStrokeAttribute();

            if(0.0 < rLineAttribute.getWidth() || 0 != rStrokeAttribute.getDotDashArray().size())
            {
                rendering::StrokeAttributes aStrokeAttribute;

                aStrokeAttribute.StrokeWidth = rLineAttribute.getWidth();
                aStrokeAttribute.MiterLimit = F_PI180 * 15.0; // 15 degrees
                const ::std::vector< double >& rDotDashArray = rStrokeAttribute.getDotDashArray();

                if(!rDotDashArray.empty())
                    aStrokeAttribute.DashArray =
                        uno::Sequence< double >(&rDotDashArray[0], rDotDashArray.size());

                switch(rLineAttribute.getLineJoin())
                {
                    default: // B2DLINEJOIN_NONE, B2DLINEJOIN_MIDDLE
                        aStrokeAttribute.JoinType = rendering::PathJoinType::NONE;
                        break;
                    case basegfx::B2DLINEJOIN_BEVEL:
                        aStrokeAttribute.JoinType = rendering::PathJoinType::BEVEL;
                        break;
                    case basegfx::B2DLINEJOIN_MITER:
                        aStrokeAttribute.JoinType = rendering::PathJoinType::MITER;
                        break;
                    case basegfx::B2DLINEJOIN_ROUND:
                        aStrokeAttribute.JoinType = rendering::PathJoinType::ROUND;
                        break;
                }

                switch(rLineAttribute.getLineCap())
                {
                    case com::sun::star::drawing::LineCap_ROUND:
                        aStrokeAttribute.StartCapType = rendering::PathCapType::ROUND;
                        aStrokeAttribute.EndCapType = rendering::PathCapType::ROUND;
                        break;
                    case com::sun::star::drawing::LineCap_SQUARE:
                        aStrokeAttribute.StartCapType = rendering::PathCapType::SQUARE;
                        aStrokeAttribute.EndCapType = rendering::PathCapType::SQUARE;
                        break;
                    default: // com::sun::star::drawing::LineCap_BUTT
                        aStrokeAttribute.StartCapType = rendering::PathCapType::BUTT;
                        aStrokeAttribute.EndCapType = rendering::PathCapType::BUTT;
                        break;
                }

                const basegfx::BColor aHairlineColor(
                    maBColorModifierStack.getModifiedColor(rLineAttribute.getColor()));
                maVolatileRenderState.DeviceColor =
                    aHairlineColor.colorToDoubleSequence(mxCanvas->getDevice());

                mxCanvas->strokePolyPolygon(
                    basegfx::unotools::xPolyPolygonFromB2DPolygon(
                        mxCanvas->getDevice(),
                        rPolygonStrokePrimitive.getB2DPolygon()),
                    maViewState,  maVolatileRenderState, aStrokeAttribute);

                return; // and done
            }

            // falback: process decomposition
            process(rPolygonStrokePrimitive.get2DDecomposition(getViewInformation2D()));
        }

        void CanvasProcessor2D::impRenderBackgroundColorPrimitive2D(const primitive2d::BackgroundColorPrimitive2D& rCandidate)
        {
            const basegfx::BColor aFillColor(
                maBColorModifierStack.getModifiedColor(rCandidate.getBColor()));
            maVolatileRenderState.DeviceColor =
                aFillColor.colorToDoubleSequence(mxCanvas->getDevice());

            const basegfx::B2DRange& aViewport(getViewInformation2D().getDiscreteViewport());

            // render to target - with identity transformation, we
            // want the viewport 1:1 filled
            basegfx::B2DHomMatrix aIdentMatrix;
            canvas::tools::setViewStateTransform(maViewState,
                                                 aIdentMatrix);
            canvas::tools::setRenderStateTransform(maVolatileRenderState,
                                                   aIdentMatrix);
            mxCanvas->fillPolyPolygon(
                basegfx::unotools::xPolyPolygonFromB2DPolygon(
                    mxCanvas->getDevice(),
                    basegfx::tools::createPolygonFromRect(aViewport)),
                maViewState,  maVolatileRenderState);

            // and reset transforms to default again
            canvas::tools::setViewStateTransform(maViewState,
                                                 getViewInformation2D().getViewTransformation());
            canvas::tools::setRenderStateTransform(maVolatileRenderState,
                                                   getViewInformation2D().getObjectTransformation());
        }

        void CanvasProcessor2D::impRenderSvgLinearGradientPrimitive2D(const primitive2d::SvgLinearGradientPrimitive2D& rCandidate)
        {
            if( rCandidate.getSpreadMethod() == primitive2d::Spread_reflect )
            {
                // TODO(P2): drats. we miss reflection as a texture
                // mode on XCanvas!
                process(rCandidate.get2DDecomposition(getViewInformation2D()));
                return;
            }

            const basegfx::B2DPolyPolygon& rPoly = rCandidate.getPolyPolygon();
            const primitive2d::SvgGradientEntryVector& rEntries = rCandidate.getGradientEntries();
            if( !rPoly.count() || rEntries.empty() )
                return; // empty output, quit

            // create matching linear gradient stops
            const size_t nEntries = rEntries.size();
            uno::Sequence< uno::Sequence < double > > aColors(nEntries);
            uno::Sequence< double > aStops(nEntries);

            for( size_t i=0; i<nEntries; ++i )
            {
                aStops[i] = rEntries[i].getOffset();
                aColors[i] = rEntries[i].getColor().colorToDoubleSequence(
                    mxCanvas->getDevice());
                aColors[i][3] = rEntries[i].getOpacity();
            }

            // fill service params
            uno::Sequence<uno::Any> args(2);
            beans::PropertyValue aProp;
            aProp.Name = "Colors";
            aProp.Value <<= aColors;
            args[0] <<= aProp;
            aProp.Name = "Stops";
            aProp.Value <<= aStops;
            args[1] <<= aProp;

            // setup texture transform (from gradient transform and
            // start points)

            // gradient goes from start to end - instead of
            // (0,0)-(1,0) for the default case.
            const basegfx::B2DVector aDir(rCandidate.getEnd() - rCandidate.getStart());

            // get up/down-scale, relative to [0,1] default len
            const double fScale( aDir.getLength() );

            // get rotation angle away from default (which is horizontal)
            const double fRot( acos(aDir.scalar( basegfx::B2DVector(1.0, 0.0))) );

            // tweak gradient transform to accomodate above changes
            basegfx::B2DHomMatrix aTotalTransform = rCandidate.getGradientTransform();
            aTotalTransform *= basegfx::tools::createTranslateB2DHomMatrix(rCandidate.getStart());
            aTotalTransform *= basegfx::tools::createRotateB2DHomMatrix(fRot);
            aTotalTransform *= basegfx::tools::createScaleB2DHomMatrix(fScale, fScale);

            // fill texture params
            rendering::Texture aTexture;
            // TODO(F2): drats. we miss reflect mode here!
            aTexture.RepeatModeX = aTexture.RepeatModeY =
                rCandidate.getSpreadMethod() == primitive2d::Spread_pad ?
                rendering::TexturingMode::CLAMP :
                rendering::TexturingMode::REPEAT;

            aTexture.Alpha = 1.0;
            ::basegfx::unotools::affineMatrixFromHomMatrix(
                aTexture.AffineTransform,
                aTotalTransform);
            aTexture.Gradient.set(
                mxParametricPolygonFactory->createInstanceWithArguments(
                    "LinearGradient",
                    args),
                uno::UNO_QUERY_THROW);

            // and render, into given polypolygon
            mxCanvas->drawPolyPolygon(
                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(),
                    rCandidate.getPolyPolygon()),
                maViewState,  maVolatileRenderState);
        }

        void CanvasProcessor2D::impRenderSvgRadialGradientPrimitive2D(const primitive2d::SvgRadialGradientPrimitive2D& rCandidate)
        {
            if( rCandidate.getSpreadMethod() == primitive2d::Spread_reflect ||
                rCandidate.isFocalSet() )
            {
                // TODO(P2): drats. we miss reflection as a texture
                // mode on XCanvas!
                // TODO(P2): drats. we also miss focal point setting
                // for radial gradient on XCanvas!
                process(rCandidate.get2DDecomposition(getViewInformation2D()));
                return;
            }

            const basegfx::B2DPolyPolygon& rPoly = rCandidate.getPolyPolygon();
            const primitive2d::SvgGradientEntryVector& rEntries = rCandidate.getGradientEntries();
            if( !rPoly.count() || rEntries.empty() )
                return; // empty output, quit

            // create matching radial gradient stops
            const size_t nEntries = rEntries.size();
            uno::Sequence< uno::Sequence < double > > aColors(nEntries);
            uno::Sequence< double > aStops(nEntries);

            for( size_t i=0; i<nEntries; ++i )
            {
                aStops[i] = rEntries[i].getOffset();
                aColors[i] = rEntries[i].getColor().colorToDoubleSequence(
                    mxCanvas->getDevice());
                aColors[i][3] = rEntries[i].getOpacity();
            }

            // fill service params
            uno::Sequence<uno::Any> args(2);
            beans::PropertyValue aProp;
            aProp.Name = "Colors";
            aProp.Value <<= aColors;
            args[0] <<= aProp;
            aProp.Name = "Stops";
            aProp.Value <<= aStops;
            args[1] <<= aProp;

            // setup texture transform (from gradient transform and
            // start points)

            // get up/down-scale, relative to 0.5 default radius
            const double fScale( rCandidate.getRadius() * 2.0 );

            // tweak gradient transform to accomodate above changes
            // (default center is 0.5/0.5)
            basegfx::B2DHomMatrix aTotalTransform = rCandidate.getGradientTransform();
            aTotalTransform *= basegfx::tools::createTranslateB2DHomMatrix(
                rCandidate.getStart()-basegfx::B2DPoint(0.5, 0.5));
            aTotalTransform *= basegfx::tools::createScaleB2DHomMatrix(fScale, fScale);

            // fill texture params
            rendering::Texture aTexture;
            // TODO(F2): drats. we miss reflect mode here!
            aTexture.RepeatModeX = aTexture.RepeatModeY =
                rCandidate.getSpreadMethod() == primitive2d::Spread_pad ?
                rendering::TexturingMode::CLAMP :
                rendering::TexturingMode::REPEAT;

            aTexture.Alpha = 1.0;
            ::basegfx::unotools::affineMatrixFromHomMatrix(
                aTexture.AffineTransform,
                aTotalTransform);
            aTexture.Gradient.set(
                mxParametricPolygonFactory->createInstanceWithArguments(
                    "EllipticalGradient",
                    args),
                uno::UNO_QUERY_THROW);

            // and render, into given polypolygon
            mxCanvas->drawPolyPolygon(
                basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(),
                    rCandidate.getPolyPolygon()),
                maViewState,  maVolatileRenderState);
        }

        //////////////////////////////////////////////////////////////////////////////
        // internal processing support

        void CanvasProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                // The following base primitives are not handled on
                // purpose - we rely on the recursive breaking up
                // (either because emulation would be ~comparable in
                // effort to breaking up, or needs vcl, or is only
                // relevant for edit mode currently, or stuff like
                // e.g. hatches is currently not implemented in
                // canvas):
                //   case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D           (fallback good)
                //   case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D             (fallback excellent)
                //   case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D          (fallback good)
                //   case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D  (fallback good)
                //   case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D              (fallback needs vcl)
                //   case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D (fallback needs vcl)
                //   case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D            (NYI in canvas)
                //   case PRIMITIVE2D_ID_INVERTPRIMITIVE2D               (edit mode only)
                //   case PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D        (should not happen, and if: fallback good)
                //   case PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D        (should not happen, and if: fallback good)
                //
                // TODO(F1):
                //   case PRIMITIVE2D_ID_EPSPRIMITIVE2D                  (needs ghostscript/Xpost help)


                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D :
                    // new XDrawPage for ViewInformation2D
                    impRenderPagePreviewPrimitive2D(static_cast< const primitive2d::PagePreviewPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                    // directdraw of text simple portion; added test possibility to check text decompose
                    if(getOptionsDrawinglayer().IsRenderSimpleTextDirect())
                    {
                        impRenderTextSimplePortionPrimitive2D(static_cast< const primitive2d::TextSimplePortionPrimitive2D& >(rCandidate));
                    }
                    else
                    {
                        process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    }
                    break;

                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                    // direct draw of hairline
                    impRenderPolygonHairlinePrimitive2D(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                    // direct draw of bitmaps
                    impRenderBitmapPrimitive2D(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D :
                    // direct draw of fillBitmapPrimitive
                    impRenderFillGraphicPrimitive2D(static_cast< const primitive2d::FillGraphicPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D :
                    // direct draw of bitmap
                    impRenderPolyPolygonGraphicPrimitive2D(static_cast< const primitive2d::PolyPolygonGraphicPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                    // direct draw of PolyPolygon with color
                    impRenderPolyPolygonColorPrimitive2D(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                    // mask group.
                    impRenderMaskPrimitive2D(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    // modified color group. Force output to unified color.
                    const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate = static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate);

                    if(rModifiedCandidate.getChildren().hasElements())
                    {
                        maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
                        process(rModifiedCandidate.getChildren());
                        maBColorModifierStack.pop();
                    }
                    break;
                }

                case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D :
                    // unified sub-transparence
                    impRenderUnifiedTransparencePrimitive2D(static_cast< const primitive2d::UnifiedTransparencePrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                    // sub-transparence group
                    impRenderTransparencePrimitive2D(static_cast< const primitive2d::TransparencePrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                    // transform group.
                    impRenderTransformPrimitive2D(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                    // point array
                    impRenderPointArrayPrimitive2D(static_cast< const primitive2d::PointArrayPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
                    // direct draw of stroked polygon
                    impRenderPolygonStrokePrimitive2D(static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D :
                    impRenderBackgroundColorPrimitive2D(static_cast< const primitive2d::BackgroundColorPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D :
                    impRenderSvgLinearGradientPrimitive2D(static_cast< const primitive2d::SvgLinearGradientPrimitive2D& >(rCandidate));
                    break;

                case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
                    impRenderSvgRadialGradientPrimitive2D(static_cast< const primitive2d::SvgRadialGradientPrimitive2D& >(rCandidate));
                    break;

                default:
                    // handle recursively
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
            }
        }

        CanvasProcessor2D::CanvasProcessor2D( const geometry::ViewInformation2D& rViewInformation,
                                              const uno::Reference< rendering::XCanvas >& xCanvas ) :
            BaseProcessor2D(rViewInformation),
            mxCanvas(xCanvas),
            mxParametricPolygonFactory(
                mxCanvas->getDevice()->getParametricPolyPolygonFactory()),
            maViewState(),
            maVolatileRenderState(),
            maBColorModifierStack(),
            maDrawinglayerOpt(),
            maClipPolyPolygon()
        {
            canvas::tools::initViewState(maViewState);
            canvas::tools::initRenderState(maVolatileRenderState);
            canvas::tools::setViewStateTransform(maViewState,
                                                 getViewInformation2D().getViewTransformation());
            canvas::tools::setRenderStateTransform(maVolatileRenderState,
                                                   getViewInformation2D().getObjectTransformation());
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
