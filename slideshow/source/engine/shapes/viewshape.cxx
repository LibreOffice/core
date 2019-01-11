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

#include <sal/config.h>

#include <tools/diagnose_ex.h>

#include <algorithm>
#include <math.h>

#include <rtl/math.hxx>
#include <sal/log.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/PanoseLetterForm.hpp>
#include <com/sun/star/awt/FontSlant.hpp>

#include <cppuhelper/exc_hlp.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <canvas/canvastools.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include "viewshape.hxx"
#include <tools.hxx>

using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {

        // TODO(F2): Provide sensible setup for mtf-related attributes (fill mode,
        // char rotation etc.). Do that via mtf argument at this object

        bool ViewShape::prefetch( RendererCacheEntry&                   io_rCacheEntry,
                                  const ::cppcanvas::CanvasSharedPtr&   rDestinationCanvas,
                                  const GDIMetaFileSharedPtr&           rMtf,
                                  const ShapeAttributeLayerSharedPtr&   rAttr )
        {
            ENSURE_OR_RETURN_FALSE( rMtf,
                               "ViewShape::prefetch(): no valid metafile!" );

            if( rMtf != io_rCacheEntry.mpMtf ||
                rDestinationCanvas != io_rCacheEntry.getDestinationCanvas() )
            {
                // buffered renderer invalid, re-create
                ::cppcanvas::Renderer::Parameters aParms;

                // rendering attribute override parameter struct.  For
                // every valid attribute, the corresponding struct
                // member is filled, which in the metafile renderer
                // forces rendering with the given attribute.
                if( rAttr )
                {
                    if( rAttr->isFillColorValid() )
                    {
                        // convert RGBColor to RGBA32 integer. Note
                        // that getIntegerColor() also truncates
                        // out-of-range values appropriately
                        aParms.maFillColor =
                            rAttr->getFillColor().getIntegerColor();
                    }
                    if( rAttr->isLineColorValid() )
                    {
                        // convert RGBColor to RGBA32 integer. Note
                        // that getIntegerColor() also truncates
                        // out-of-range values appropriately
                        aParms.maLineColor =
                            rAttr->getLineColor().getIntegerColor();
                    }
                    if( rAttr->isCharColorValid() )
                    {
                        // convert RGBColor to RGBA32 integer. Note
                        // that getIntegerColor() also truncates
                        // out-of-range values appropriately
                        aParms.maTextColor =
                            rAttr->getCharColor().getIntegerColor();
                    }
                    if( rAttr->isDimColorValid() )
                    {
                        // convert RGBColor to RGBA32 integer. Note
                        // that getIntegerColor() also truncates
                        // out-of-range values appropriately

                        // dim color overrides all other colors
                        aParms.maFillColor =
                        aParms.maLineColor =
                        aParms.maTextColor =
                            rAttr->getDimColor().getIntegerColor();
                    }
                    if( rAttr->isFontFamilyValid() )
                    {
                        aParms.maFontName =
                            rAttr->getFontFamily();
                    }
                    if( rAttr->isCharScaleValid() )
                    {
                        ::basegfx::B2DHomMatrix aMatrix;

                        // enlarge text by given scale factor. Do that
                        // with the middle of the shape as the center
                        // of scaling.
                        aMatrix.translate( -0.5, -0.5 );
                        aMatrix.scale( rAttr->getCharScale(),
                                       rAttr->getCharScale() );
                        aMatrix.translate( 0.5, 0.5 );

                        aParms.maTextTransformation = aMatrix;
                    }
                    if( rAttr->isCharWeightValid() )
                    {
                        aParms.maFontWeight =
                            static_cast< sal_Int8 >(
                                ::basegfx::fround(
                                    ::std::max( 0.0,
                                                ::std::min( 11.0,
                                                            rAttr->getCharWeight() / 20.0 ) ) ) );
                    }
                    if( rAttr->isCharPostureValid() )
                    {
                        aParms.maFontLetterForm =
                            rAttr->getCharPosture() == sal_Int16(awt::FontSlant_NONE) ?
                            rendering::PanoseLetterForm::ANYTHING :
                            rendering::PanoseLetterForm::OBLIQUE_CONTACT;
                    }
                    if( rAttr->isUnderlineModeValid() )
                    {
                        aParms.maFontUnderline =
                            rAttr->getUnderlineMode();
                    }
                }

                io_rCacheEntry.mpRenderer = ::cppcanvas::VCLFactory::createRenderer( rDestinationCanvas,
                                                                                     *rMtf,
                                                                                     aParms );

                io_rCacheEntry.mpMtf               = rMtf;
                io_rCacheEntry.mpDestinationCanvas = rDestinationCanvas;

                // also invalidate alpha compositing bitmap (created
                // new renderer, which possibly generates different
                // output). Do NOT invalidate, if we're incidentally
                // rendering INTO it.
                if( rDestinationCanvas != io_rCacheEntry.mpLastBitmapCanvas )
                {
                    io_rCacheEntry.mpLastBitmapCanvas.reset();
                    io_rCacheEntry.mpLastBitmap.reset();
                }
            }

            return static_cast< bool >(io_rCacheEntry.mpRenderer);
        }

        bool ViewShape::draw( const ::cppcanvas::CanvasSharedPtr&   rDestinationCanvas,
                              const GDIMetaFileSharedPtr&           rMtf,
                              const ShapeAttributeLayerSharedPtr&   rAttr,
                              const ::basegfx::B2DHomMatrix&        rTransform,
                              const ::basegfx::B2DPolyPolygon*      pClip,
                              const VectorOfDocTreeNodes&           rSubsets ) const
        {
            ::cppcanvas::RendererSharedPtr pRenderer(
                getRenderer( rDestinationCanvas, rMtf, rAttr ) );

            ENSURE_OR_RETURN_FALSE( pRenderer, "ViewShape::draw(): Invalid renderer" );

            pRenderer->setTransformation( rTransform );
#if OSL_DEBUG_LEVEL >= 2
            rendering::RenderState aRenderState;
            ::canvas::tools::initRenderState(aRenderState);
            ::canvas::tools::setRenderStateTransform(aRenderState,
                                                     rTransform);
            aRenderState.DeviceColor.realloc(4);
            aRenderState.DeviceColor[0] = 1.0;
            aRenderState.DeviceColor[1] = 0.0;
            aRenderState.DeviceColor[2] = 0.0;
            aRenderState.DeviceColor[3] = 1.0;

            try
            {
                rDestinationCanvas->getUNOCanvas()->drawLine( geometry::RealPoint2D(0.0,0.0),
                                                              geometry::RealPoint2D(1.0,1.0),
                                                              rDestinationCanvas->getViewState(),
                                                              aRenderState );
                rDestinationCanvas->getUNOCanvas()->drawLine( geometry::RealPoint2D(1.0,0.0),
                                                              geometry::RealPoint2D(0.0,1.0),
                                                              rDestinationCanvas->getViewState(),
                                                              aRenderState );
            }
            catch( uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("slideshow");
            }
#endif
            if( pClip )
                pRenderer->setClip( *pClip );
            else
                pRenderer->setClip();

            if( rSubsets.empty() )
            {
                return pRenderer->draw();
            }
            else
            {
                // render subsets of whole metafile


                bool bRet(true);
                for( const auto& rSubset : rSubsets )
                {
                    if( !pRenderer->drawSubset( rSubset.getStartIndex(),
                                                rSubset.getEndIndex() ) )
                        bRet = false;
                }

                return bRet;
            }
        }

        namespace
        {
            /// Convert untransformed shape update area to device pixel.
            ::basegfx::B2DRectangle shapeArea2AreaPixel( const ::basegfx::B2DHomMatrix& rCanvasTransformation,
                                                         const ::basegfx::B2DRectangle& rUntransformedArea      )
            {
                // convert area to pixel, and add anti-aliasing border

                // TODO(P1): Should the view transform some
                // day contain rotation/shear, transforming
                // the original bounds with the total
                // transformation might result in smaller
                // overall bounds.

                ::basegfx::B2DRectangle aBoundsPixel;
                ::canvas::tools::calcTransformedRectBounds( aBoundsPixel,
                                                            rUntransformedArea,
                                                            rCanvasTransformation );

                // add antialiasing border around the shape (AA
                // touches pixel _outside_ the nominal bound rect)
                aBoundsPixel.grow( ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE );

                return aBoundsPixel;
            }

            /// Convert shape unit rect to device pixel.
            ::basegfx::B2DRectangle calcUpdateAreaPixel( const ::basegfx::B2DRectangle&         rUnitBounds,
                                                         const ::basegfx::B2DHomMatrix&         rShapeTransformation,
                                                         const ::basegfx::B2DHomMatrix&         rCanvasTransformation,
                                                         const ShapeAttributeLayerSharedPtr&    pAttr                   )
            {
                // calc update area for whole shape (including
                // character scaling)
                return shapeArea2AreaPixel( rCanvasTransformation,
                                            getShapeUpdateArea( rUnitBounds,
                                                                rShapeTransformation,
                                                                pAttr ) );
            }
        }

        bool ViewShape::renderSprite( const ViewLayerSharedPtr&             rViewLayer,
                                      const GDIMetaFileSharedPtr&           rMtf,
                                      const ::basegfx::B2DRectangle&        rOrigBounds,
                                      const ::basegfx::B2DRectangle&        rBounds,
                                      const ::basegfx::B2DRectangle&        rUnitBounds,
                                      UpdateFlags                           nUpdateFlags,
                                      const ShapeAttributeLayerSharedPtr&   pAttr,
                                      const VectorOfDocTreeNodes&           rSubsets,
                                      double                                nPrio,
                                      bool                                  bIsVisible ) const
        {
            // TODO(P1): For multiple views, it might pay off to reorg Shape and ViewShape,
            // in that all the common setup steps here are refactored to Shape (would then
            // have to be performed only _once_ per Shape paint).

            if( !bIsVisible ||
                rUnitBounds.isEmpty() ||
                rOrigBounds.isEmpty() ||
                rBounds.isEmpty() )
            {
                // shape is invisible or has zero size, no need to
                // update anything.
                if( mpSprite )
                    mpSprite->hide();

                return true;
            }


            // calc sprite position, size and content transformation
            // =====================================================

            // the shape transformation for a sprite is always a
            // simple scale-up to the nominal shape size. Everything
            // else is handled via the sprite transformation
            ::basegfx::B2DHomMatrix aNonTranslationalShapeTransformation;
            aNonTranslationalShapeTransformation.scale( rOrigBounds.getWidth(),
                                                        rOrigBounds.getHeight() );
            ::basegfx::B2DHomMatrix aShapeTransformation( aNonTranslationalShapeTransformation );
            aShapeTransformation.translate( rOrigBounds.getMinX(),
                                            rOrigBounds.getMinY() );

            const ::basegfx::B2DHomMatrix& rCanvasTransform(
                rViewLayer->getSpriteTransformation() );

            // area actually needed for the sprite
            const ::basegfx::B2DRectangle& rSpriteBoundsPixel(
                calcUpdateAreaPixel( rUnitBounds,
                                     aShapeTransformation,
                                     rCanvasTransform,
                                     pAttr ) );

            // actual area for the shape (without subsetting, but
            // including char scaling)
            const ::basegfx::B2DRectangle& rShapeBoundsPixel(
                calcUpdateAreaPixel( ::basegfx::B2DRectangle(0.0,0.0,1.0,1.0),
                                     aShapeTransformation,
                                     rCanvasTransform,
                                     pAttr ) );

            // nominal area for the shape (without subsetting, without
            // char scaling). NOTE: to cancel the shape translation,
            // contained in rSpriteBoundsPixel, this is _without_ any
            // translational component.
            ::basegfx::B2DRectangle        aLogShapeBounds;
            const ::basegfx::B2DRectangle& rNominalShapeBoundsPixel(
                shapeArea2AreaPixel( rCanvasTransform,
                                     ::canvas::tools::calcTransformedRectBounds(
                                         aLogShapeBounds,
                                         ::basegfx::B2DRectangle(0.0,0.0,1.0,1.0),
                                         aNonTranslationalShapeTransformation ) ) );

            // create (or resize) sprite with sprite's pixel size, if
            // not done already
            const ::basegfx::B2DSize& rSpriteSizePixel(rSpriteBoundsPixel.getRange());
            if( !mpSprite )
            {
                mpSprite.reset(
                    new AnimatedSprite( mpViewLayer,
                                        rSpriteSizePixel,
                                        nPrio ));
            }
            else
            {
                // TODO(F2): when the sprite _actually_ gets resized,
                // content needs a repaint!
                mpSprite->resize( rSpriteSizePixel );
            }

            ENSURE_OR_RETURN_FALSE( mpSprite, "ViewShape::renderSprite(): No sprite" );

            SAL_INFO("slideshow", "ViewShape::renderSprite(): Rendering sprite " <<
                           mpSprite.get() );


            // always show the sprite (might have been hidden before)
            mpSprite->show();

            // determine center of sprite output position in pixel
            // (assumption here: all shape transformations have the
            // shape center as the pivot point). From that, subtract
            // distance of rSpriteBoundsPixel's left, top edge from
            // rShapeBoundsPixel's center. This moves the sprite at
            // the appropriate output position within the virtual
            // rShapeBoundsPixel area.
            ::basegfx::B2DPoint aSpritePosPixel( rBounds.getCenter() );
            aSpritePosPixel *= rCanvasTransform;
            aSpritePosPixel -= rShapeBoundsPixel.getCenter() - rSpriteBoundsPixel.getMinimum();

            // the difference between rShapeBoundsPixel and
            // rSpriteBoundsPixel upper, left corner is: the offset we
            // have to move sprite output to the right, top (to make
            // the desired subset content visible at all)
            const ::basegfx::B2DSize& rSpriteCorrectionOffset(
                rSpriteBoundsPixel.getMinimum() - rNominalShapeBoundsPixel.getMinimum() );

            // offset added top, left for anti-aliasing (otherwise,
            // shapes fully filling the sprite will have anti-aliased
            // pixel cut off)
            const ::basegfx::B2DSize aAAOffset(
                ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE,
                ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE );

            // set pixel output offset to sprite: we always leave
            // ANTIALIASING_EXTRA_SIZE room atop and to the left, and,
            // what's more, for subsetted shapes, we _have_ to cancel
            // the effect of the shape renderer outputting the subset
            // at its absolute position inside the shape, instead of
            // at the origin.
            // NOTE: As for now, sprites are always positioned on
            // integer pixel positions on screen, have to round to
            // nearest integer here, too
            mpSprite->setPixelOffset(
                aAAOffset - ::basegfx::B2DSize(
                    ::basegfx::fround( rSpriteCorrectionOffset.getX() ),
                    ::basegfx::fround( rSpriteCorrectionOffset.getY() ) ) );

            // always set sprite position and transformation, since
            // they do not relate directly to the update flags
            // (e.g. sprite position changes when sprite size changes)
            mpSprite->movePixel( aSpritePosPixel );
            mpSprite->transform( getSpriteTransformation( rSpriteSizePixel,
                                                          rOrigBounds.getRange(),
                                                          pAttr ) );


            // process flags
            // =============

            bool bRedrawRequired( mbForceUpdate || (nUpdateFlags & UpdateFlags::Force) );

            if( mbForceUpdate || (nUpdateFlags & UpdateFlags::Alpha) )
            {
                mpSprite->setAlpha( (pAttr && pAttr->isAlphaValid()) ?
                                    std::clamp(pAttr->getAlpha(),
                                                     0.0,
                                                     1.0) :
                                    1.0 );
            }
            if( mbForceUpdate || (nUpdateFlags & UpdateFlags::Clip) )
            {
                if( pAttr && pAttr->isClipValid() )
                {
                    ::basegfx::B2DPolyPolygon aClipPoly( pAttr->getClip() );

                    // extract linear part of canvas view transformation
                    // (linear means: without translational components)
                    ::basegfx::B2DHomMatrix aViewTransform(
                        mpViewLayer->getTransformation() );
                    aViewTransform.set( 0, 2, 0.0 );
                    aViewTransform.set( 1, 2, 0.0 );

                    // make the clip 2*ANTIALIASING_EXTRA_SIZE larger
                    // such that it's again centered over the sprite.
                    aViewTransform.scale(rSpriteSizePixel.getX()/
                                         (rSpriteSizePixel.getX()-2*::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE),
                                         rSpriteSizePixel.getY()/
                                         (rSpriteSizePixel.getY()-2*::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE));

                    // transform clip polygon from view to device
                    // coordinate space
                    aClipPoly.transform( aViewTransform );

                    mpSprite->clip( aClipPoly );
                }
                else
                    mpSprite->clip();
            }
            if( mbForceUpdate || (nUpdateFlags & UpdateFlags::Content) )
            {
                bRedrawRequired = true;

                // TODO(P1): maybe provide some appearance change methods at
                // the Renderer interface

                // force the renderer to be regenerated below, for the
                // different attributes to take effect
                invalidateRenderer();
            }

            mbForceUpdate = false;

            if( !bRedrawRequired )
                return true;


            // sprite needs repaint - output to sprite canvas
            // ==============================================

            ::cppcanvas::CanvasSharedPtr pContentCanvas( mpSprite->getContentCanvas() );

            return draw( pContentCanvas,
                         rMtf,
                         pAttr,
                         aShapeTransformation,
                         nullptr, // clipping is done via Sprite::clip()
                         rSubsets );
        }

        bool ViewShape::render( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas,
                                const GDIMetaFileSharedPtr&         rMtf,
                                const ::basegfx::B2DRectangle&      rBounds,
                                const ::basegfx::B2DRectangle&      rUpdateBounds,
                                UpdateFlags                         nUpdateFlags,
                                const ShapeAttributeLayerSharedPtr& pAttr,
                                const VectorOfDocTreeNodes&         rSubsets,
                                bool                                bIsVisible ) const
        {
            // TODO(P1): For multiple views, it might pay off to reorg Shape and ViewShape,
            // in that all the common setup steps here are refactored to Shape (would then
            // have to be performed only _once_ per Shape paint).

            if( !bIsVisible )
            {
                SAL_INFO("slideshow", "ViewShape::render(): skipping shape " << this );

                // shape is invisible, no need to update anything.
                return true;
            }

            // since we have no sprite here, _any_ update request
            // translates into a required redraw.
            bool bRedrawRequired( mbForceUpdate || nUpdateFlags != UpdateFlags::NONE );

            if( nUpdateFlags & UpdateFlags::Content )
            {
                // TODO(P1): maybe provide some appearance change methods at
                // the Renderer interface

                // force the renderer to be regenerated below, for the
                // different attributes to take effect
                invalidateRenderer();
            }

            mbForceUpdate = false;

            if( !bRedrawRequired )
                return true;

            SAL_INFO( "slideshow", "ViewShape::render(): rendering shape " <<
                      this <<
                      " at position (" <<
                      rBounds.getMinX() << "," <<
                      rBounds.getMinY() << ")" );


            // shape needs repaint - setup all that's needed


            boost::optional<basegfx::B2DPolyPolygon> aClip;

            if( pAttr )
            {
                // setup clip poly
                if( pAttr->isClipValid() )
                    aClip = pAttr->getClip();

                // emulate global shape alpha by first rendering into
                // a temp bitmap, and then to screen (this would have
                // been much easier if we'd be currently a sprite -
                // see above)
                if( pAttr->isAlphaValid() )
                {
                    const double nAlpha( pAttr->getAlpha() );

                    if( !::basegfx::fTools::equalZero( nAlpha ) &&
                        !::rtl::math::approxEqual(nAlpha, 1.0) )
                    {
                        // render with global alpha - have to prepare
                        // a bitmap, and render that with modulated
                        // alpha


                        const ::basegfx::B2DHomMatrix aTransform(
                            getShapeTransformation( rBounds,
                                                    pAttr ) );

                        // TODO(P1): Should the view transform some
                        // day contain rotation/shear, transforming
                        // the original bounds with the total
                        // transformation might result in smaller
                        // overall bounds.

                        // determine output rect of _shape update
                        // area_ in device pixel
                        const ::basegfx::B2DHomMatrix aCanvasTransform(
                            rDestinationCanvas->getTransformation() );
                        ::basegfx::B2DRectangle aTmpRect;
                        ::canvas::tools::calcTransformedRectBounds( aTmpRect,
                                                                    rUpdateBounds,
                                                                    aCanvasTransform );

                        // pixel size of cache bitmap: round up to
                        // nearest int
                        const ::basegfx::B2ISize aBmpSize( static_cast<sal_Int32>( aTmpRect.getWidth() )+1,
                                                           static_cast<sal_Int32>( aTmpRect.getHeight() )+1 );

                        // try to fetch temporary surface for alpha
                        // compositing (to achieve the global alpha
                        // blend effect, have to first render shape as
                        // a whole, then blit that surface with global
                        // alpha to the destination)
                        const RendererCacheVector::iterator aCompositingSurface(
                            getCacheEntry( rDestinationCanvas ) );

                        if( !aCompositingSurface->mpLastBitmapCanvas ||
                            aCompositingSurface->mpLastBitmapCanvas->getSize() != aBmpSize )
                        {
                            // create a bitmap of appropriate size
                            ::cppcanvas::BitmapSharedPtr pBitmap(
                                ::cppcanvas::BaseGfxFactory::createAlphaBitmap(
                                    rDestinationCanvas,
                                    aBmpSize ) );

                            ENSURE_OR_THROW(pBitmap,
                                             "ViewShape::render(): Could not create compositing surface");

                            aCompositingSurface->mpDestinationCanvas = rDestinationCanvas;
                            aCompositingSurface->mpLastBitmap        = pBitmap;
                            aCompositingSurface->mpLastBitmapCanvas  = pBitmap->getBitmapCanvas();
                        }

                        // buffer aCompositingSurface iterator content
                        // - said one might get invalidated during
                        // draw() below.
                        ::cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas(
                            aCompositingSurface->mpLastBitmapCanvas );

                        ::cppcanvas::BitmapSharedPtr pBitmap(
                            aCompositingSurface->mpLastBitmap);

                        // setup bitmap canvas transformation -
                        // which happens to be the destination
                        // canvas transformation without any
                        // translational components.

                        // But then, the render transformation as
                        // calculated by getShapeTransformation()
                        // above outputs the shape at its real
                        // destination position. Thus, we have to
                        // offset the output back to the origin,
                        // for which we simply plug in the
                        // negative position of the left, top edge
                        // of the shape's bound rect in device
                        // pixel into aLinearTransform below.
                        ::basegfx::B2DHomMatrix aAdjustedCanvasTransform( aCanvasTransform );
                        aAdjustedCanvasTransform.translate( -aTmpRect.getMinX(),
                                                            -aTmpRect.getMinY() );

                        pBitmapCanvas->setTransformation( aAdjustedCanvasTransform );

                        // TODO(P2): If no update flags, or only
                        // alpha_update is set, we can save us the
                        // rendering into the bitmap (uh, it's not
                        // _that_ easy - for a forced redraw,
                        // e.g. when ending an animation, we always
                        // get UPDATE_FORCE here).

                        // render into this bitmap
                        if( !draw( pBitmapCanvas,
                                   rMtf,
                                   pAttr,
                                   aTransform,
                                   !aClip ? nullptr : &(*aClip),
                                   rSubsets ) )
                        {
                            return false;
                        }

                        // render bitmap to screen, with given global
                        // alpha. Since the bitmap already contains
                        // pixel-equivalent output, we have to use the
                        // inverse view transformation, adjusted with
                        // the final shape output position (note:
                        // cannot simply change the view
                        // transformation here, as that would affect a
                        // possibly set clip!)
                        ::basegfx::B2DHomMatrix aBitmapTransform( aCanvasTransform );
                        OSL_ENSURE( aBitmapTransform.isInvertible(),
                                    "ViewShape::render(): View transformation is singular!" );

                        aBitmapTransform.invert();

                        const basegfx::B2DHomMatrix aTranslation(basegfx::utils::createTranslateB2DHomMatrix(
                            aTmpRect.getMinX(), aTmpRect.getMinY()));

                        aBitmapTransform = aBitmapTransform * aTranslation;
                        pBitmap->setTransformation( aBitmapTransform );

                        // finally, render bitmap alpha-modulated
                        pBitmap->drawAlphaModulated( nAlpha );

                        return true;
                    }
                }
            }

            // retrieve shape transformation, _with_ shape translation
            // to actual page position.
            const ::basegfx::B2DHomMatrix aTransform(
                getShapeTransformation( rBounds,
                                        pAttr ) );

            return draw( rDestinationCanvas,
                         rMtf,
                         pAttr,
                         aTransform,
                         !aClip ? nullptr : &(*aClip),
                         rSubsets );
        }


        ViewShape::ViewShape( const ViewLayerSharedPtr& rViewLayer ) :
            mpViewLayer( rViewLayer ),
            maRenderers(),
            mpSprite(),
            mbAnimationMode( false ),
            mbForceUpdate( true )
        {
            ENSURE_OR_THROW( mpViewLayer, "ViewShape::ViewShape(): Invalid View" );
        }

        const ViewLayerSharedPtr& ViewShape::getViewLayer() const
        {
            return mpViewLayer;
        }

        ViewShape::RendererCacheVector::iterator ViewShape::getCacheEntry( const ::cppcanvas::CanvasSharedPtr&  rDestinationCanvas ) const
        {
            // lookup destination canvas - is there already a renderer
            // created for that target?
            RendererCacheVector::iterator       aIter;
            const RendererCacheVector::iterator aEnd( maRenderers.end() );

            // already there?
            if( (aIter=::std::find_if( maRenderers.begin(),
                                       aEnd,
                                       [&rDestinationCanvas]( const RendererCacheEntry& rCacheEntry )
                                       { return rDestinationCanvas == rCacheEntry.getDestinationCanvas(); } ) ) == aEnd )
            {
                if( maRenderers.size() >= MAX_RENDER_CACHE_ENTRIES )
                {
                    // cache size exceeded - prune entries. For now,
                    // simply remove the first one, which of course
                    // breaks for more complex access schemes. But in
                    // general, this leads to most recently used
                    // entries to reside at the end of the vector.
                    maRenderers.erase( maRenderers.begin() );

                    // ATTENTION: after this, both aIter and aEnd are
                    // invalid!
                }

                // not yet in cache - add default-constructed cache
                // entry, to have something to return
                maRenderers.emplace_back( );
                aIter = maRenderers.end()-1;
            }

            return aIter;
        }

        ::cppcanvas::RendererSharedPtr ViewShape::getRenderer( const ::cppcanvas::CanvasSharedPtr&  rDestinationCanvas,
                                                               const GDIMetaFileSharedPtr&          rMtf,
                                                               const ShapeAttributeLayerSharedPtr&  rAttr ) const
        {
            // lookup destination canvas - is there already a renderer
            // created for that target?
            const RendererCacheVector::iterator aIter(
                getCacheEntry( rDestinationCanvas ) );

            // now we have a valid entry, either way. call prefetch()
            // on it, nevertheless - maybe the metafile changed, and
            // the renderer still needs an update (prefetch() will
            // detect that)
            if( prefetch( *aIter,
                          rDestinationCanvas,
                          rMtf,
                          rAttr ) )
            {
                return aIter->mpRenderer;
            }
            else
            {
                // prefetch failed - renderer is invalid
                return ::cppcanvas::RendererSharedPtr();
            }
        }

        void ViewShape::invalidateRenderer() const
        {
            // simply clear the cache. Subsequent getRenderer() calls
            // will regenerate the Renderers.
            maRenderers.clear();
        }

        ::basegfx::B2DSize ViewShape::getAntialiasingBorder() const
        {
            ENSURE_OR_THROW( mpViewLayer->getCanvas(),
                              "ViewShape::getAntialiasingBorder(): Invalid ViewLayer canvas" );

            const ::basegfx::B2DHomMatrix& rViewTransform(
                mpViewLayer->getTransformation() );

            // TODO(F1): As a quick shortcut (did not want to invert
            // whole matrix here), taking only scale components of
            // view transformation matrix. This will be wrong when
            // e.g. shearing is involved.
            const double nXBorder( ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE / rViewTransform.get(0,0) );
            const double nYBorder( ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE / rViewTransform.get(1,1) );

            return ::basegfx::B2DSize( nXBorder,
                                       nYBorder );
        }

        void ViewShape::enterAnimationMode()
        {
            mbForceUpdate   = true;
            mbAnimationMode = true;
        }

        void ViewShape::leaveAnimationMode()
        {
            mpSprite.reset();
            mbAnimationMode = false;
            mbForceUpdate   = true;
        }

        bool ViewShape::update( const GDIMetaFileSharedPtr& rMtf,
                                const RenderArgs&           rArgs,
                                UpdateFlags                 nUpdateFlags,
                                bool                        bIsVisible ) const
        {
            ENSURE_OR_RETURN_FALSE( mpViewLayer->getCanvas(), "ViewShape::update(): Invalid layer canvas" );

            // Shall we render to a sprite, or to a plain canvas?
            if( mbAnimationMode )
                return renderSprite( mpViewLayer,
                                     rMtf,
                                     rArgs.maOrigBounds,
                                     rArgs.maBounds,
                                     rArgs.maUnitBounds,
                                     nUpdateFlags,
                                     rArgs.mrAttr,
                                     rArgs.mrSubsets,
                                     rArgs.mnShapePriority,
                                     bIsVisible );
            else
                return render( mpViewLayer->getCanvas(),
                               rMtf,
                               rArgs.maBounds,
                               rArgs.maUpdateBounds,
                               nUpdateFlags,
                               rArgs.mrAttr,
                               rArgs.mrSubsets,
                               bIsVisible );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
