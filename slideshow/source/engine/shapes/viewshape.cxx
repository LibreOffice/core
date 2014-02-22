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



#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <math.h>

#include <rtl/math.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/PanoseLetterForm.hpp>
#include <com/sun/star/awt/FontSlant.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include "viewshape.hxx"
#include "tools.hxx"

#include <boost/bind.hpp>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {

        
        

        bool ViewShape::prefetch( RendererCacheEntry&                   io_rCacheEntry,
                                  const ::cppcanvas::CanvasSharedPtr&   rDestinationCanvas,
                                  const GDIMetaFileSharedPtr&           rMtf,
                                  const ShapeAttributeLayerSharedPtr&   rAttr ) const
        {
            ENSURE_OR_RETURN_FALSE( rMtf,
                               "ViewShape::prefetch(): no valid metafile!" );

            if( rMtf != io_rCacheEntry.mpMtf ||
                rDestinationCanvas != io_rCacheEntry.getDestinationCanvas() )
            {
                
                ::cppcanvas::Renderer::Parameters aParms;

                
                
                
                
                if( rAttr )
                {
                    if( rAttr->isFillColorValid() )
                    {
                        
                        
                        
                        aParms.maFillColor =
                            rAttr->getFillColor().getIntegerColor();
                    }
                    if( rAttr->isLineColorValid() )
                    {
                        
                        
                        
                        aParms.maLineColor =
                            rAttr->getLineColor().getIntegerColor();
                    }
                    if( rAttr->isCharColorValid() )
                    {
                        
                        
                        
                        aParms.maTextColor =
                            rAttr->getCharColor().getIntegerColor();
                    }
                    if( rAttr->isDimColorValid() )
                    {
                        
                        
                        

                        
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
                            rAttr->getCharPosture() == awt::FontSlant_NONE ?
                            rendering::PanoseLetterForm::ANYTHING :
                            rendering::PanoseLetterForm::OBLIQUE_CONTACT;
                    }
                    if( rAttr->isUnderlineModeValid() )
                    {
                        aParms.maFontUnderline =
                            rAttr->getUnderlineMode();
                    }
                }

                io_rCacheEntry.mpRenderer = ::cppcanvas::VCLFactory::getInstance().createRenderer( rDestinationCanvas,
                                                                                                   *rMtf.get(),
                                                                                                   aParms );

                io_rCacheEntry.mpMtf               = rMtf;
                io_rCacheEntry.mpDestinationCanvas = rDestinationCanvas;

                
                
                
                
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
                DBG_UNHANDLED_EXCEPTION();
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
                
                

                bool bRet(true);
                VectorOfDocTreeNodes::const_iterator        aIter( rSubsets.begin() );
                const VectorOfDocTreeNodes::const_iterator  aEnd ( rSubsets.end() );
                while( aIter != aEnd )
                {
                    if( !pRenderer->drawSubset( aIter->getStartIndex(),
                                                aIter->getEndIndex() ) )
                        bRet = false;

                    ++aIter;
                }

                return bRet;
            }
        }

        namespace
        {
            
            ::basegfx::B2DRectangle shapeArea2AreaPixel( const ::basegfx::B2DHomMatrix& rCanvasTransformation,
                                                         const ::basegfx::B2DRectangle& rUntransformedArea      )
            {
                

                
                
                
                
                

                ::basegfx::B2DRectangle aBoundsPixel;
                ::canvas::tools::calcTransformedRectBounds( aBoundsPixel,
                                                            rUntransformedArea,
                                                            rCanvasTransformation );

                
                
                aBoundsPixel.grow( ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE );

                return aBoundsPixel;
            }

            
            ::basegfx::B2DRectangle calcUpdateAreaPixel( const ::basegfx::B2DRectangle&         rUnitBounds,
                                                         const ::basegfx::B2DHomMatrix&         rShapeTransformation,
                                                         const ::basegfx::B2DHomMatrix&         rCanvasTransformation,
                                                         const ShapeAttributeLayerSharedPtr&    pAttr                   )
            {
                
                
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
                                      int                                   nUpdateFlags,
                                      const ShapeAttributeLayerSharedPtr&   pAttr,
                                      const VectorOfDocTreeNodes&           rSubsets,
                                      double                                nPrio,
                                      bool                                  bIsVisible ) const
        {
            
            
            

            if( !bIsVisible ||
                rUnitBounds.isEmpty() ||
                rOrigBounds.isEmpty() ||
                rBounds.isEmpty() )
            {
                
                
                if( mpSprite )
                    mpSprite->hide();

                return true;
            }


            
            

            
            
            
            ::basegfx::B2DHomMatrix aNonTranslationalShapeTransformation;
            aNonTranslationalShapeTransformation.scale( rOrigBounds.getWidth(),
                                                        rOrigBounds.getHeight() );
            ::basegfx::B2DHomMatrix aShapeTransformation( aNonTranslationalShapeTransformation );
            aShapeTransformation.translate( rOrigBounds.getMinX(),
                                            rOrigBounds.getMinY() );

            const ::basegfx::B2DHomMatrix& rCanvasTransform(
                rViewLayer->getSpriteTransformation() );

            
            const ::basegfx::B2DRectangle& rSpriteBoundsPixel(
                calcUpdateAreaPixel( rUnitBounds,
                                     aShapeTransformation,
                                     rCanvasTransform,
                                     pAttr ) );

            
            
            const ::basegfx::B2DRectangle& rShapeBoundsPixel(
                calcUpdateAreaPixel( ::basegfx::B2DRectangle(0.0,0.0,1.0,1.0),
                                     aShapeTransformation,
                                     rCanvasTransform,
                                     pAttr ) );

            
            
            
            
            ::basegfx::B2DRectangle        aLogShapeBounds;
            const ::basegfx::B2DRectangle& rNominalShapeBoundsPixel(
                shapeArea2AreaPixel( rCanvasTransform,
                                     ::canvas::tools::calcTransformedRectBounds(
                                         aLogShapeBounds,
                                         ::basegfx::B2DRectangle(0.0,0.0,1.0,1.0),
                                         aNonTranslationalShapeTransformation ) ) );

            
            
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
                
                
                mpSprite->resize( rSpriteSizePixel );
            }

            ENSURE_OR_RETURN_FALSE( mpSprite, "ViewShape::renderSprite(): No sprite" );

            VERBOSE_TRACE( "ViewShape::renderSprite(): Rendering sprite 0x%p",
                           mpSprite.get() );


            
            mpSprite->show();

            
            
            
            
            
            
            
            ::basegfx::B2DPoint aSpritePosPixel( rBounds.getCenter() );
            aSpritePosPixel *= rCanvasTransform;
            aSpritePosPixel -= rShapeBoundsPixel.getCenter() - rSpriteBoundsPixel.getMinimum();

            
            
            
            
            const ::basegfx::B2DSize& rSpriteCorrectionOffset(
                rSpriteBoundsPixel.getMinimum() - rNominalShapeBoundsPixel.getMinimum() );

            
            
            
            const ::basegfx::B2DSize aAAOffset(
                ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE,
                ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE );

            
            
            
            
            
            
            
            
            
            mpSprite->setPixelOffset(
                aAAOffset - ::basegfx::B2DSize(
                    ::basegfx::fround( rSpriteCorrectionOffset.getX() ),
                    ::basegfx::fround( rSpriteCorrectionOffset.getY() ) ) );

            
            
            
            mpSprite->movePixel( aSpritePosPixel );
            mpSprite->transform( getSpriteTransformation( rSpriteSizePixel,
                                                          rOrigBounds.getRange(),
                                                          pAttr ) );


            
            

            bool bRedrawRequired( mbForceUpdate || (nUpdateFlags & FORCE) );

            if( mbForceUpdate || (nUpdateFlags & ALPHA) )
            {
                mpSprite->setAlpha( (pAttr && pAttr->isAlphaValid()) ?
                                    ::basegfx::clamp(pAttr->getAlpha(),
                                                     0.0,
                                                     1.0) :
                                    1.0 );
            }
            if( mbForceUpdate || (nUpdateFlags & CLIP) )
            {
                if( pAttr && pAttr->isClipValid() )
                {
                    ::basegfx::B2DPolyPolygon aClipPoly( pAttr->getClip() );

                    
                    
                    ::basegfx::B2DHomMatrix aViewTransform(
                        mpViewLayer->getTransformation() );
                    aViewTransform.set( 0, 2, 0.0 );
                    aViewTransform.set( 1, 2, 0.0 );

                    
                    
                    aViewTransform.scale(rSpriteSizePixel.getX()/
                                         (rSpriteSizePixel.getX()-2*::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE),
                                         rSpriteSizePixel.getY()/
                                         (rSpriteSizePixel.getY()-2*::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE));

                    
                    
                    aClipPoly.transform( aViewTransform );

                    mpSprite->clip( aClipPoly );
                }
                else
                    mpSprite->clip();
            }
            if( mbForceUpdate || (nUpdateFlags & CONTENT) )
            {
                bRedrawRequired = true;

                
                

                
                
                invalidateRenderer();
            }

            mbForceUpdate = false;

            if( !bRedrawRequired )
                return true;


            
            

            ::cppcanvas::CanvasSharedPtr pContentCanvas( mpSprite->getContentCanvas() );

            return draw( pContentCanvas,
                         rMtf,
                         pAttr,
                         aShapeTransformation,
                         NULL, 
                         rSubsets );
        }

        bool ViewShape::render( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas,
                                const GDIMetaFileSharedPtr&         rMtf,
                                const ::basegfx::B2DRectangle&      rBounds,
                                const ::basegfx::B2DRectangle&      rUpdateBounds,
                                int                                 nUpdateFlags,
                                const ShapeAttributeLayerSharedPtr& pAttr,
                                const VectorOfDocTreeNodes&         rSubsets,
                                bool                                bIsVisible ) const
        {
            
            
            

            if( !bIsVisible )
            {
                VERBOSE_TRACE( "ViewShape::render(): skipping shape %p", this );

                
                return true;
            }

            
            
            bool bRedrawRequired( mbForceUpdate || nUpdateFlags != 0 );

            if( (nUpdateFlags & CONTENT) )
            {
                
                

                
                
                invalidateRenderer();
            }

            mbForceUpdate = false;

            if( !bRedrawRequired )
                return true;

            VERBOSE_TRACE( "ViewShape::render(): rendering shape %p at position (%f,%f)",
                           this,
                           rBounds.getMinX(),
                           rBounds.getMinY() );


            
            

            boost::optional<basegfx::B2DPolyPolygon> aClip;

            if( pAttr )
            {
                
                if( pAttr->isClipValid() )
                    aClip.reset( pAttr->getClip() );

                
                
                
                
                if( pAttr->isAlphaValid() )
                {
                    const double nAlpha( pAttr->getAlpha() );

                    if( !::basegfx::fTools::equalZero( nAlpha ) &&
                        !::rtl::math::approxEqual(nAlpha, 1.0) )
                    {
                        
                        
                        
                        

                        const ::basegfx::B2DHomMatrix aTransform(
                            getShapeTransformation( rBounds,
                                                    pAttr ) );

                        
                        
                        
                        
                        

                        
                        
                        const ::basegfx::B2DHomMatrix aCanvasTransform(
                            rDestinationCanvas->getTransformation() );
                        ::basegfx::B2DRectangle aTmpRect;
                        ::canvas::tools::calcTransformedRectBounds( aTmpRect,
                                                                    rUpdateBounds,
                                                                    aCanvasTransform );

                        
                        
                        const ::basegfx::B2ISize aBmpSize( static_cast<sal_Int32>( aTmpRect.getWidth() )+1,
                                                           static_cast<sal_Int32>( aTmpRect.getHeight() )+1 );

                        
                        
                        
                        
                        
                        const RendererCacheVector::iterator aCompositingSurface(
                            getCacheEntry( rDestinationCanvas ) );

                        if( !aCompositingSurface->mpLastBitmapCanvas ||
                            aCompositingSurface->mpLastBitmapCanvas->getSize() != aBmpSize )
                        {
                            
                            ::cppcanvas::BitmapSharedPtr pBitmap(
                                ::cppcanvas::BaseGfxFactory::getInstance().createAlphaBitmap(
                                    rDestinationCanvas,
                                    aBmpSize ) );

                            ENSURE_OR_THROW(pBitmap,
                                             "ViewShape::render(): Could not create compositing surface");

                            aCompositingSurface->mpDestinationCanvas = rDestinationCanvas;
                            aCompositingSurface->mpLastBitmap        = pBitmap;
                            aCompositingSurface->mpLastBitmapCanvas  = pBitmap->getBitmapCanvas();
                        }

                        
                        
                        
                        ::cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas(
                            aCompositingSurface->mpLastBitmapCanvas );

                        ::cppcanvas::BitmapSharedPtr pBitmap(
                            aCompositingSurface->mpLastBitmap);

                        
                        
                        
                        
                        //
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        ::basegfx::B2DHomMatrix aAdjustedCanvasTransform( aCanvasTransform );
                        aAdjustedCanvasTransform.translate( -aTmpRect.getMinX(),
                                                            -aTmpRect.getMinY() );

                        pBitmapCanvas->setTransformation( aAdjustedCanvasTransform );

                        
                        
                        
                        
                        
                        

                        
                        if( !draw( pBitmapCanvas,
                                   rMtf,
                                   pAttr,
                                   aTransform,
                                   !aClip ? NULL : &(*aClip),
                                   rSubsets ) )
                        {
                            return false;
                        }

                        
                        
                        
                        
                        
                        
                        
                        
                        ::basegfx::B2DHomMatrix aBitmapTransform( aCanvasTransform );
                        OSL_ENSURE( aBitmapTransform.isInvertible(),
                                    "ViewShape::render(): View transformation is singular!" );

                        aBitmapTransform.invert();

                        const basegfx::B2DHomMatrix aTranslation(basegfx::tools::createTranslateB2DHomMatrix(
                            aTmpRect.getMinX(), aTmpRect.getMinY()));

                        aBitmapTransform = aBitmapTransform * aTranslation;
                        pBitmap->setTransformation( aBitmapTransform );

                        
                        pBitmap->drawAlphaModulated( nAlpha );

                        return true;
                    }
                }
            }

            
            
            const ::basegfx::B2DHomMatrix aTransform(
                getShapeTransformation( rBounds,
                                        pAttr ) );

            return draw( rDestinationCanvas,
                         rMtf,
                         pAttr,
                         aTransform,
                         !aClip ? NULL : &(*aClip),
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

        ViewLayerSharedPtr ViewShape::getViewLayer() const
        {
            return mpViewLayer;
        }

        ViewShape::RendererCacheVector::iterator ViewShape::getCacheEntry( const ::cppcanvas::CanvasSharedPtr&  rDestinationCanvas ) const
        {
            
            
            RendererCacheVector::iterator       aIter;
            const RendererCacheVector::iterator aEnd( maRenderers.end() );

            
            if( (aIter=::std::find_if( maRenderers.begin(),
                                       aEnd,
                                       ::boost::bind(
                                           ::std::equal_to< ::cppcanvas::CanvasSharedPtr >(),
                                           ::boost::cref( rDestinationCanvas ),
                                           ::boost::bind(
                                               &RendererCacheEntry::getDestinationCanvas,
                                               _1 ) ) ) ) == aEnd )
            {
                if( maRenderers.size() >= MAX_RENDER_CACHE_ENTRIES )
                {
                    
                    
                    
                    
                    
                    maRenderers.erase( maRenderers.begin() );

                    
                    
                }

                
                
                maRenderers.push_back( RendererCacheEntry() );
                aIter = maRenderers.end()-1;
            }

            return aIter;
        }

        ::cppcanvas::RendererSharedPtr ViewShape::getRenderer( const ::cppcanvas::CanvasSharedPtr&  rDestinationCanvas,
                                                               const GDIMetaFileSharedPtr&          rMtf,
                                                               const ShapeAttributeLayerSharedPtr&  rAttr ) const
        {
            
            
            const RendererCacheVector::iterator aIter(
                getCacheEntry( rDestinationCanvas ) );

            
            
            
            
            if( prefetch( *aIter,
                          rDestinationCanvas,
                          rMtf,
                          rAttr ) )
            {
                return aIter->mpRenderer;
            }
            else
            {
                
                return ::cppcanvas::RendererSharedPtr();
            }
        }

        void ViewShape::invalidateRenderer() const
        {
            
            
            maRenderers.clear();
        }

        ::basegfx::B2DSize ViewShape::getAntialiasingBorder() const
        {
            ENSURE_OR_THROW( mpViewLayer->getCanvas(),
                              "ViewShape::getAntialiasingBorder(): Invalid ViewLayer canvas" );

            const ::basegfx::B2DHomMatrix& rViewTransform(
                mpViewLayer->getTransformation() );

            
            
            
            
            const double nXBorder( ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE / rViewTransform.get(0,0) );
            const double nYBorder( ::cppcanvas::Canvas::ANTIALIASING_EXTRA_SIZE / rViewTransform.get(1,1) );

            return ::basegfx::B2DSize( nXBorder,
                                       nYBorder );
        }

        bool ViewShape::enterAnimationMode()
        {
            mbForceUpdate   = true;
            mbAnimationMode = true;

            return true;
        }

        void ViewShape::leaveAnimationMode()
        {
            mpSprite.reset();
            mbAnimationMode = false;
            mbForceUpdate   = true;
        }

        bool ViewShape::update( const GDIMetaFileSharedPtr& rMtf,
                                const RenderArgs&           rArgs,
                                int                         nUpdateFlags,
                                bool                        bIsVisible ) const
        {
            ENSURE_OR_RETURN_FALSE( mpViewLayer->getCanvas(), "ViewShape::update(): Invalid layer canvas" );

            
            if( isBackgroundDetached() )
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
