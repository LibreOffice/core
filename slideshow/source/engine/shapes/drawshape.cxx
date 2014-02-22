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
#include <canvas/verbosetrace.hxx>

#include <osl/diagnose.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/wrkwin.hxx>

#include <basegfx/numeric/ftools.hxx>

#include <rtl/math.hxx>

#include <com/sun/star/drawing/TextAnimationKind.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <comphelper/scopeguard.hxx>
#include <canvas/canvastools.hxx>

#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

#include "drawshapesubsetting.hxx"
#include "drawshape.hxx"
#include "eventqueue.hxx"
#include "wakeupevent.hxx"
#include "subsettableshapemanager.hxx"
#include "intrinsicanimationactivity.hxx"
#include "slideshowexceptions.hxx"
#include "tools.hxx"
#include "gdimtftools.hxx"
#include "drawinglayeranimation.hxx"

#include <boost/bind.hpp>
#include <math.h>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        
        //
        
        //
        

        GDIMetaFileSharedPtr DrawShape::forceScrollTextMetaFile()
        {
            if ((mnCurrMtfLoadFlags & MTF_LOAD_SCROLL_TEXT_MTF) != MTF_LOAD_SCROLL_TEXT_MTF)
            {
                
                mpCurrMtf.reset( new GDIMetaFile );
                mnCurrMtfLoadFlags |= MTF_LOAD_SCROLL_TEXT_MTF;
                getMetaFile(
                    uno::Reference<lang::XComponent>(mxShape, uno::UNO_QUERY),
                    mxPage, *mpCurrMtf, mnCurrMtfLoadFlags,
                    mxComponentContext );

                
                
                
                
                maSubsetting.reset( mpCurrMtf );

                
                
                
                ::basegfx::B2DRectangle aScrollRect, aPaintRect;
                ENSURE_OR_THROW( getRectanglesFromScrollMtf( aScrollRect,
                                                              aPaintRect,
                                                              mpCurrMtf ),
                                  "DrawShape::forceScrollTextMetaFile(): Could "
                                  "not extract scroll anim rectangles from mtf" );

                
                
                
                if( aScrollRect.isInside( aPaintRect ) )
                    maBounds = aScrollRect;
                else
                    maBounds = aPaintRect;
            }
            return mpCurrMtf;
        }

        void DrawShape::updateStateIds() const
        {
            
            
            if( mpAttributeLayer )
            {
                mnAttributeTransformationState = mpAttributeLayer->getTransformationState();
                mnAttributeClipState = mpAttributeLayer->getClipState();
                mnAttributeAlphaState = mpAttributeLayer->getAlphaState();
                mnAttributePositionState = mpAttributeLayer->getPositionState();
                mnAttributeContentState = mpAttributeLayer->getContentState();
                mnAttributeVisibilityState = mpAttributeLayer->getVisibilityState();
            }
        }

        ViewShape::RenderArgs DrawShape::getViewRenderArgs() const
        {
            return ViewShape::RenderArgs(
                maBounds,
                getUpdateArea(),
                getBounds(),
                getActualUnitShapeBounds(),
                mpAttributeLayer,
                maSubsetting.getActiveSubsets(),
                mnPriority);
        }

        bool DrawShape::implRender( int nUpdateFlags ) const
        {
            SAL_INFO( "slideshow", "::presentation::internal::DrawShape::implRender()" );
            SAL_INFO( "slideshow", "::presentation::internal::DrawShape: 0x" << std::hex << this );

            
            
            mbForceUpdate = false;
            mbAttributeLayerRevoked = false;

            ENSURE_OR_RETURN_FALSE( !maViewShapes.empty(),
                               "DrawShape::implRender(): render called on DrawShape without views" );

            if( maBounds.isEmpty() )
            {
                
                
                return true;
            }

            
            if( ::std::count_if( maViewShapes.begin(),
                                 maViewShapes.end(),
                                 ::boost::bind<bool>(
                                     ::boost::mem_fn( &ViewShape::update ), 
                                                                             
                                                                             
                                                                             
                                                                             
                                     _1,
                                     ::boost::cref( mpCurrMtf ),
                                     ::boost::cref(
                                         getViewRenderArgs() ),
                                     nUpdateFlags,
                                     isVisible() ) )
                != static_cast<ViewShapeVector::difference_type>(maViewShapes.size()) )
            {
                
                
                return false;
            }

            
            updateStateIds();

            return true;
        }

        int DrawShape::getUpdateFlags() const
        {
            
            
            int nUpdateFlags(ViewShape::NONE);

            
            if( mbAttributeLayerRevoked )
                nUpdateFlags = ViewShape::CONTENT;


            
            

            
            if( mpAttributeLayer )
            {
                
                
                if (mpAttributeLayer->getVisibility()
                    || mpAttributeLayer->getVisibilityState() != mnAttributeVisibilityState )
                {
                    if (mpAttributeLayer->getVisibilityState() != mnAttributeVisibilityState )
                    {
                        
                        
                        
                        
                        nUpdateFlags |= ViewShape::CONTENT;
                    }

                    
                    
                    if( mpAttributeLayer->getPositionState() != mnAttributePositionState )
                    {
                        nUpdateFlags |= ViewShape::POSITION;
                    }
                    if( mpAttributeLayer->getAlphaState() != mnAttributeAlphaState )
                    {
                        nUpdateFlags |= ViewShape::ALPHA;
                    }
                    if( mpAttributeLayer->getClipState() != mnAttributeClipState )
                    {
                        nUpdateFlags |= ViewShape::CLIP;
                    }
                    if( mpAttributeLayer->getTransformationState() != mnAttributeTransformationState )
                    {
                        nUpdateFlags |= ViewShape::TRANSFORMATION;
                    }
                    if( mpAttributeLayer->getContentState() != mnAttributeContentState )
                    {
                        nUpdateFlags |= ViewShape::CONTENT;
                    }
                }
            }

            return nUpdateFlags;
        }

        ::basegfx::B2DRectangle DrawShape::getActualUnitShapeBounds() const
        {
            ENSURE_OR_THROW( !maViewShapes.empty(),
                              "DrawShape::getActualUnitShapeBounds(): called on DrawShape without views" );

            const VectorOfDocTreeNodes& rSubsets(
                maSubsetting.getActiveSubsets() );

            const ::basegfx::B2DRectangle aDefaultBounds( 0.0,0.0,1.0,1.0 );

            
            if( rSubsets.empty() )
            {
                
                
                
                
                return aDefaultBounds;
            }
            else
            {
                OSL_ENSURE( rSubsets.size() != 1 ||
                            !rSubsets.front().isEmpty(),
                            "DrawShape::getActualUnitShapeBounds() expects a "
                            "_non-empty_ subset vector for a subsetted shape!" );

                
                if( !maCurrentShapeUnitBounds )
                {
                    
                    

                    
                    
                    maCurrentShapeUnitBounds.reset( aDefaultBounds );

                    
                    
                    
                    
                    

                    ::cppcanvas::CanvasSharedPtr pDestinationCanvas(
                        maViewShapes.front()->getViewLayer()->getCanvas() );

                    
                    
                    
                    
                    ::cppcanvas::RendererSharedPtr pRenderer(
                        maViewShapes.front()->getRenderer(
                            pDestinationCanvas, mpCurrMtf, mpAttributeLayer ) );

                    
                    
                    if( pRenderer )
                    {
                        
                        
                        
                        ::basegfx::B2DHomMatrix      aEmptyTransformation;

                        ::basegfx::B2DHomMatrix      aOldTransform( pDestinationCanvas->getTransformation() );
                        pDestinationCanvas->setTransformation( aEmptyTransformation );
                        pRenderer->setTransformation( aEmptyTransformation );

                        
                        const ::comphelper::ScopeGuard aGuard(
                            boost::bind( &::cppcanvas::Canvas::setTransformation,
                                         pDestinationCanvas, aOldTransform ) );


                        
                        

                        ::basegfx::B2DRange aTotalBounds;

                        
                        
                        VectorOfDocTreeNodes::const_iterator        aCurr( rSubsets.begin() );
                        const VectorOfDocTreeNodes::const_iterator  aEnd( rSubsets.end() );
                        while( aCurr != aEnd )
                        {
                            aTotalBounds.expand( pRenderer->getSubsetArea(
                                                     aCurr->getStartIndex(),
                                                     aCurr->getEndIndex() )  );
                            ++aCurr;
                        }

                        OSL_ENSURE( aTotalBounds.getMinX() >= -0.1 &&
                                    aTotalBounds.getMinY() >= -0.1 &&
                                    aTotalBounds.getMaxX() <= 1.1 &&
                                    aTotalBounds.getMaxY() <= 1.1,
                                    "DrawShape::getActualUnitShapeBounds(): bounds noticeably larger than original shape - clipping!" );

                        
                        
                        
                        
                        aTotalBounds.intersect(
                            ::basegfx::B2DRange( 0.0, 0.0,
                                                 1.0, 1.0 ));

                        maCurrentShapeUnitBounds.reset( aTotalBounds );
                    }
                }

                return *maCurrentShapeUnitBounds;
            }
        }

        DrawShape::DrawShape( const uno::Reference< drawing::XShape >&      xShape,
                              const uno::Reference< drawing::XDrawPage >&   xContainingPage,
                              double                                        nPrio,
                              bool                                          bForeignSource,
                              const SlideShowContext&                       rContext ) :
            mxShape( xShape ),
            mxPage( xContainingPage ),
            maAnimationFrames(), 
            mnCurrFrame(0),
            mpCurrMtf(),
            mnCurrMtfLoadFlags( bForeignSource
                                ? MTF_LOAD_FOREIGN_SOURCE : MTF_LOAD_NONE ),
            maCurrentShapeUnitBounds(),
            mnPriority( nPrio ), 
            maBounds( getAPIShapeBounds( xShape ) ),
            mpAttributeLayer(),
            mpIntrinsicAnimationActivity(),
            mnAttributeTransformationState(0),
            mnAttributeClipState(0),
            mnAttributeAlphaState(0),
            mnAttributePositionState(0),
            mnAttributeContentState(0),
            mnAttributeVisibilityState(0),
            maViewShapes(),
            mxComponentContext( rContext.mxComponentContext ),
            maHyperlinkIndices(),
            maHyperlinkRegions(),
            maSubsetting(),
            mnIsAnimatedCount(0),
            mnAnimationLoopCount(0),
            meCycleMode(CYCLE_LOOP),
            mbIsVisible( true ),
            mbForceUpdate( false ),
            mbAttributeLayerRevoked( false ),
            mbDrawingLayerAnim( false )
        {
            ENSURE_OR_THROW( mxShape.is(), "DrawShape::DrawShape(): Invalid XShape" );
            ENSURE_OR_THROW( mxPage.is(), "DrawShape::DrawShape(): Invalid containing page" );

            
            drawing::TextAnimationKind eKind = drawing::TextAnimationKind_NONE;
            uno::Reference<beans::XPropertySet> xPropSet( mxShape,
                                                          uno::UNO_QUERY );
            if( xPropSet.is() )
                getPropertyValue( eKind, xPropSet,
                                  "TextAnimationKind" );
            mbDrawingLayerAnim = (eKind != drawing::TextAnimationKind_NONE);

            
            
            mpCurrMtf.reset( new GDIMetaFile );
            getMetaFile(
                uno::Reference<lang::XComponent>(xShape, uno::UNO_QUERY),
                xContainingPage, *mpCurrMtf, mnCurrMtfLoadFlags,
                mxComponentContext );
            ENSURE_OR_THROW( mpCurrMtf,
                              "DrawShape::DrawShape(): Invalid metafile" );
            maSubsetting.reset( mpCurrMtf );

            prepareHyperlinkIndices();
        }

        DrawShape::DrawShape( const uno::Reference< drawing::XShape >&      xShape,
                              const uno::Reference< drawing::XDrawPage >&   xContainingPage,
                              double                                        nPrio,
                              const Graphic&                                rGraphic,
                              const SlideShowContext&                       rContext ) :
            mxShape( xShape ),
            mxPage( xContainingPage ),
            maAnimationFrames(),
            mnCurrFrame(0),
            mpCurrMtf(),
            mnCurrMtfLoadFlags( MTF_LOAD_NONE ),
            maCurrentShapeUnitBounds(),
            mnPriority( nPrio ), 
            maBounds( getAPIShapeBounds( xShape ) ),
            mpAttributeLayer(),
            mpIntrinsicAnimationActivity(),
            mnAttributeTransformationState(0),
            mnAttributeClipState(0),
            mnAttributeAlphaState(0),
            mnAttributePositionState(0),
            mnAttributeContentState(0),
            mnAttributeVisibilityState(0),
            maViewShapes(),
            mxComponentContext( rContext.mxComponentContext ),
            maHyperlinkIndices(),
            maHyperlinkRegions(),
            maSubsetting(),
            mnIsAnimatedCount(0),
            mnAnimationLoopCount(0),
            meCycleMode(CYCLE_LOOP),
            mbIsVisible( true ),
            mbForceUpdate( false ),
            mbAttributeLayerRevoked( false ),
            mbDrawingLayerAnim( false )
        {
            ENSURE_OR_THROW( rGraphic.IsAnimated(),
                              "DrawShape::DrawShape(): Graphic is no animation" );

            getAnimationFromGraphic( maAnimationFrames,
                                     mnAnimationLoopCount,
                                     meCycleMode,
                                     rGraphic );

            ENSURE_OR_THROW( !maAnimationFrames.empty() &&
                              maAnimationFrames.front().mpMtf,
                              "DrawShape::DrawShape(): " );
            mpCurrMtf = maAnimationFrames.front().mpMtf;

            ENSURE_OR_THROW( mxShape.is(), "DrawShape::DrawShape(): Invalid XShape" );
            ENSURE_OR_THROW( mxPage.is(), "DrawShape::DrawShape(): Invalid containing page" );
            ENSURE_OR_THROW( mpCurrMtf, "DrawShape::DrawShape(): Invalid metafile" );
        }

        DrawShape::DrawShape( const DrawShape&      rSrc,
                              const DocTreeNode&    rTreeNode,
                              double                nPrio ) :
            mxShape( rSrc.mxShape ),
            mxPage( rSrc.mxPage ),
            maAnimationFrames(), 
                                 
            mnCurrFrame(0),
            mpCurrMtf( rSrc.mpCurrMtf ),
            mnCurrMtfLoadFlags( rSrc.mnCurrMtfLoadFlags ),
            maCurrentShapeUnitBounds(),
            mnPriority( nPrio ),
            maBounds( rSrc.maBounds ),
            mpAttributeLayer(),
            mpIntrinsicAnimationActivity(),
            mnAttributeTransformationState(0),
            mnAttributeClipState(0),
            mnAttributeAlphaState(0),
            mnAttributePositionState(0),
            mnAttributeContentState(0),
            mnAttributeVisibilityState(0),
            maViewShapes(),
            mxComponentContext( rSrc.mxComponentContext ),
            maHyperlinkIndices(),
            maHyperlinkRegions(),
            maSubsetting( rTreeNode, mpCurrMtf ),
            mnIsAnimatedCount(0),
            mnAnimationLoopCount(0),
            meCycleMode(CYCLE_LOOP),
            mbIsVisible( rSrc.mbIsVisible ),
            mbForceUpdate( false ),
            mbAttributeLayerRevoked( false ),
            mbDrawingLayerAnim( false )
        {
            ENSURE_OR_THROW( mxShape.is(), "DrawShape::DrawShape(): Invalid XShape" );
            ENSURE_OR_THROW( mpCurrMtf, "DrawShape::DrawShape(): Invalid metafile" );

            
            
            
            
        }

        
        //
        
        //
        

        DrawShapeSharedPtr DrawShape::create(
            const uno::Reference< drawing::XShape >&    xShape,
            const uno::Reference< drawing::XDrawPage >& xContainingPage,
            double                                      nPrio,
            bool                                        bForeignSource,
            const SlideShowContext&                     rContext )
        {
            DrawShapeSharedPtr pShape( new DrawShape(xShape,
                                                     xContainingPage,
                                                     nPrio,
                                                     bForeignSource,
                                                     rContext) );

            if( pShape->hasIntrinsicAnimation() )
            {
                OSL_ASSERT( pShape->maAnimationFrames.empty() );
                if( pShape->getNumberOfTreeNodes(
                        DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH) > 0 )
                {
                    pShape->mpIntrinsicAnimationActivity =
                        createDrawingLayerAnimActivity(
                            rContext,
                            pShape);
                }
            }

            if( pShape->hasHyperlinks() )
                rContext.mpSubsettableShapeManager->addHyperlinkArea( pShape );

            return pShape;
        }

        DrawShapeSharedPtr DrawShape::create(
            const uno::Reference< drawing::XShape >&    xShape,
            const uno::Reference< drawing::XDrawPage >& xContainingPage,
            double                                      nPrio,
            const Graphic&                              rGraphic,
            const SlideShowContext&                     rContext )
        {
            DrawShapeSharedPtr pShape( new DrawShape(xShape,
                                                     xContainingPage,
                                                     nPrio,
                                                     rGraphic,
                                                     rContext) );

            if( pShape->hasIntrinsicAnimation() )
            {
                OSL_ASSERT( !pShape->maAnimationFrames.empty() );

                std::vector<double> aTimeout;
                std::transform(
                    pShape->maAnimationFrames.begin(),
                    pShape->maAnimationFrames.end(),
                    std::back_insert_iterator< std::vector<double> >( aTimeout ),
                    boost::mem_fn(&MtfAnimationFrame::getDuration) );

                WakeupEventSharedPtr pWakeupEvent(
                    new WakeupEvent( rContext.mrEventQueue.getTimer(),
                                     rContext.mrActivitiesQueue ) );

                ActivitySharedPtr pActivity =
                    createIntrinsicAnimationActivity(
                        rContext,
                        pShape,
                        pWakeupEvent,
                        aTimeout,
                        pShape->mnAnimationLoopCount,
                        pShape->meCycleMode);

                pWakeupEvent->setActivity( pActivity );
                pShape->mpIntrinsicAnimationActivity = pActivity;
            }

            OSL_ENSURE( !pShape->hasHyperlinks(),
                        "DrawShape::create(): graphic-only shapes must not have hyperlinks!" );

            return pShape;
        }

        DrawShape::~DrawShape()
        {
            try
            {
                
                
                ActivitySharedPtr pActivity( mpIntrinsicAnimationActivity.lock() );
                if( pActivity )
                    pActivity->dispose();
            }
            catch (uno::Exception &)
            {
                SAL_WARN( "slideshow", "" << comphelper::anyToString(cppu::getCaughtException() ) );
            }
        }

        uno::Reference< drawing::XShape > DrawShape::getXShape() const
        {
            return mxShape;
        }

        void DrawShape::addViewLayer( const ViewLayerSharedPtr& rNewLayer,
                                      bool                      bRedrawLayer )
        {
            ViewShapeVector::iterator aEnd( maViewShapes.end() );

            
            if( ::std::find_if( maViewShapes.begin(),
                                aEnd,
                                ::boost::bind<bool>(
                                    ::std::equal_to< ViewLayerSharedPtr >(),
                                    ::boost::bind( &ViewShape::getViewLayer,
                                                   _1 ),
                                    ::boost::cref( rNewLayer ) ) ) != aEnd )
            {
                
                return;
            }

            ViewShapeSharedPtr pNewShape( new ViewShape( rNewLayer ) );

            maViewShapes.push_back( pNewShape );

            
            if( mnIsAnimatedCount )
            {
                for( int i=0; i<mnIsAnimatedCount; ++i )
                    pNewShape->enterAnimationMode();
            }

            
            if( bRedrawLayer )
            {
                pNewShape->update( mpCurrMtf,
                                   getViewRenderArgs(),
                                   ViewShape::FORCE,
                                   isVisible() );
            }
        }

        bool DrawShape::removeViewLayer( const ViewLayerSharedPtr& rLayer )
        {
            const ViewShapeVector::iterator aEnd( maViewShapes.end() );

            OSL_ENSURE( ::std::count_if(maViewShapes.begin(),
                                        aEnd,
                                        ::boost::bind<bool>(
                                            ::std::equal_to< ViewLayerSharedPtr >(),
                                            ::boost::bind( &ViewShape::getViewLayer,
                                                           _1 ),
                                            ::boost::cref( rLayer ) ) ) < 2,
                        "DrawShape::removeViewLayer(): Duplicate ViewLayer entries!" );

            ViewShapeVector::iterator aIter;

            if( (aIter=::std::remove_if( maViewShapes.begin(),
                                         aEnd,
                                         ::boost::bind<bool>(
                                             ::std::equal_to< ViewLayerSharedPtr >(),
                                             ::boost::bind( &ViewShape::getViewLayer,
                                                            _1 ),
                                             ::boost::cref( rLayer ) ) )) == aEnd )
            {
                
                return false;
            }

            
            maViewShapes.erase( aIter, aEnd );

            return true;
        }

        bool DrawShape::clearAllViewLayers()
        {
            maViewShapes.clear();
            return true;
        }

        bool DrawShape::update() const
        {
            if( mbForceUpdate )
            {
                return render();
            }
            else
            {
                return implRender( getUpdateFlags() );
            }
        }

        bool DrawShape::render() const
        {
            
            
            
            
            
            return implRender( ViewShape::FORCE | getUpdateFlags() );
        }

        bool DrawShape::isContentChanged() const
        {
            return mbForceUpdate ?
                true :
                getUpdateFlags() != ViewShape::NONE;
        }


        ::basegfx::B2DRectangle DrawShape::getBounds() const
        {
            
            
            
            return getShapePosSize( maBounds,
                                    mpAttributeLayer );
        }

        ::basegfx::B2DRectangle DrawShape::getDomBounds() const
        {
            return maBounds;
        }

        namespace
        {
            /** Functor expanding AA border for each passed ViewShape

                Could not use ::boost::bind here, since
                B2DRange::expand is overloaded (which yields one or
                the other template type deduction ambiguous)
             */
            class Expander
            {
            public:
                Expander( ::basegfx::B2DSize& rBounds ) :
                    mrBounds( rBounds )
                {
                }

                void operator()( const ViewShapeSharedPtr& rShape ) const
                {
                    const ::basegfx::B2DSize& rShapeBorder( rShape->getAntialiasingBorder() );

                    mrBounds.setX(
                        ::std::max(
                            rShapeBorder.getX(),
                            mrBounds.getX() ) );
                    mrBounds.setY(
                        ::std::max(
                            rShapeBorder.getY(),
                            mrBounds.getY() ) );
                }

            private:
                ::basegfx::B2DSize& mrBounds;
            };
        }

        ::basegfx::B2DRectangle DrawShape::getUpdateArea() const
        {
            ::basegfx::B2DRectangle aBounds;

            
            
            
            
            if( !maBounds.isEmpty() )
            {
                basegfx::B2DRectangle aUnitBounds(0.0,0.0,1.0,1.0);

                if( !maViewShapes.empty() )
                    aUnitBounds = getActualUnitShapeBounds();

                if( !aUnitBounds.isEmpty() )
                {
                    if( mpAttributeLayer )
                    {
                        
                        
                        
                        aBounds = getShapeUpdateArea( aUnitBounds,
                                                      getShapeTransformation( getBounds(),
                                                                              mpAttributeLayer ),
                                                      mpAttributeLayer );
                    }
                    else
                    {
                        
                        
                        
                        aBounds = getShapeUpdateArea( aUnitBounds,
                                                      maBounds );
                    }

                    if( !maViewShapes.empty() )
                    {
                        
                        ::basegfx::B2DSize aAABorder(0.0,0.0);

                        
                        
                        ::std::for_each( maViewShapes.begin(),
                                         maViewShapes.end(),
                                         Expander( aAABorder ) );

                        
                        aBounds = ::basegfx::B2DRectangle( aBounds.getMinX() - aAABorder.getX(),
                                                           aBounds.getMinY() - aAABorder.getY(),
                                                           aBounds.getMaxX() + aAABorder.getX(),
                                                           aBounds.getMaxY() + aAABorder.getY() );
                    }
                }
            }

            return aBounds;
        }

        bool DrawShape::isVisible() const
        {
            bool bIsVisible( mbIsVisible );

            if( mpAttributeLayer )
            {
                
                
                
                
                if( mpAttributeLayer->isVisibilityValid() )
                    bIsVisible = mpAttributeLayer->getVisibility();

                
                
                
                
                if( bIsVisible && mpAttributeLayer->isAlphaValid() )
                    bIsVisible = !::basegfx::fTools::equalZero( mpAttributeLayer->getAlpha() );
            }

            return bIsVisible;
        }

        double DrawShape::getPriority() const
        {
            return mnPriority;
        }

        bool DrawShape::isBackgroundDetached() const
        {
            return mnIsAnimatedCount > 0;
        }

        bool DrawShape::hasIntrinsicAnimation() const
        {
            return (!maAnimationFrames.empty() || mbDrawingLayerAnim);
        }

        bool DrawShape::setIntrinsicAnimationFrame( ::std::size_t nCurrFrame )
        {
            ENSURE_OR_RETURN_FALSE( nCurrFrame < maAnimationFrames.size(),
                               "DrawShape::setIntrinsicAnimationFrame(): frame index out of bounds" );

            if( mnCurrFrame != nCurrFrame )
            {
                mnCurrFrame   = nCurrFrame;
                mpCurrMtf     = maAnimationFrames[ mnCurrFrame ].mpMtf;
                mbForceUpdate = true;
            }

            return true;
        }

        
        void DrawShape::prepareHyperlinkIndices() const
        {
            if ( !maHyperlinkIndices.empty())
            {
                maHyperlinkIndices.clear();
                maHyperlinkRegions.clear();
            }

            sal_Int32 nIndex = 0;
            for ( MetaAction * pCurrAct = mpCurrMtf->FirstAction();
                  pCurrAct != 0; pCurrAct = mpCurrMtf->NextAction() )
            {
                if (pCurrAct->GetType() == META_COMMENT_ACTION) {
                    MetaCommentAction * pAct =
                        static_cast<MetaCommentAction *>(pCurrAct);
                    
                    if (pAct->GetComment().equalsIgnoreAsciiCase("FIELD_SEQ_BEGIN") &&
                        
                        
                        
                        pAct->GetData() != 0 &&
                        pAct->GetDataSize() > 0)
                    {
                        if (!maHyperlinkIndices.empty() &&
                            maHyperlinkIndices.back().second == -1) {
                            SAL_WARN( "slideshow", "### pending FIELD_SEQ_END!" );
                            maHyperlinkIndices.pop_back();
                            maHyperlinkRegions.pop_back();
                        }
                        maHyperlinkIndices.push_back(
                            HyperlinkIndexPair( nIndex + 1,
                                                -1 /* to be filled below */ ) );
                        maHyperlinkRegions.push_back(
                            HyperlinkRegion(
                                basegfx::B2DRectangle(),
                                OUString(
                                    reinterpret_cast<sal_Unicode const*>(
                                        pAct->GetData()),
                                    pAct->GetDataSize() / sizeof(sal_Unicode) )
                                ) );
                    }
                    else if (pAct->GetComment().equalsIgnoreAsciiCase("FIELD_SEQ_END") &&
                             
                             !maHyperlinkIndices.empty() &&
                             maHyperlinkIndices.back().second == -1)
                    {
                        maHyperlinkIndices.back().second = nIndex;
                    }
                    ++nIndex;
                }
                else
                    nIndex += getNextActionOffset(pCurrAct);
            }
            if (!maHyperlinkIndices.empty() &&
                maHyperlinkIndices.back().second == -1) {
                SAL_WARN( "slideshow", "### pending FIELD_SEQ_END!" );
                maHyperlinkIndices.pop_back();
                maHyperlinkRegions.pop_back();
            }
            OSL_ASSERT( maHyperlinkIndices.size() == maHyperlinkRegions.size());
        }

        bool DrawShape::hasHyperlinks() const
        {
            return ! maHyperlinkRegions.empty();
        }

        HyperlinkArea::HyperlinkRegions DrawShape::getHyperlinkRegions() const
        {
            OSL_ASSERT( !maViewShapes.empty() );

            if( !isVisible() )
                return HyperlinkArea::HyperlinkRegions();

            
            if( !maHyperlinkRegions.empty() &&
                !maViewShapes.empty() &&
                
                maHyperlinkRegions.front().first.getWidth() == 0 &&
                maHyperlinkRegions.front().first.getHeight() == 0 &&
                maHyperlinkRegions.size() == maHyperlinkIndices.size() )
            {
                
                
                ViewShapeSharedPtr const& pViewShape = maViewShapes.front();
                cppcanvas::CanvasSharedPtr const pCanvas(
                    pViewShape->getViewLayer()->getCanvas() );

                
                cppcanvas::RendererSharedPtr const pRenderer(
                    pViewShape->getRenderer(
                        pCanvas, mpCurrMtf, mpAttributeLayer ) );

                OSL_ASSERT( pRenderer );

                if (pRenderer)
                {
                    basegfx::B2DHomMatrix const aOldTransform(
                        pCanvas->getTransformation() );
                    basegfx::B2DHomMatrix aTransform;
                    pCanvas->setTransformation( aTransform /* empty */ );

                    comphelper::ScopeGuard const resetOldTransformation(
                        boost::bind( &cppcanvas::Canvas::setTransformation,
                                     pCanvas.get(),
                                     boost::cref(aOldTransform) ));

                    aTransform.scale( maBounds.getWidth(),
                                      maBounds.getHeight() );
                    pRenderer->setTransformation( aTransform );
                    pRenderer->setClip();

                    for( std::size_t pos = maHyperlinkRegions.size(); pos--; )
                    {
                        
                        HyperlinkIndexPair const& rIndices = maHyperlinkIndices[pos];
                        basegfx::B2DRectangle const region(
                            pRenderer->getSubsetArea( rIndices.first,
                                                      rIndices.second ));
                        maHyperlinkRegions[pos].first = region;
                    }
                }
            }

            
            

            HyperlinkRegions aTranslatedRegions;
            const basegfx::B2DPoint& rOffset(getBounds().getMinimum());
            HyperlinkRegions::const_iterator       aIter( maHyperlinkRegions.begin() );
            HyperlinkRegions::const_iterator const aEnd ( maHyperlinkRegions.end() );
            while( aIter != aEnd )
            {
                basegfx::B2DRange const& relRegion( aIter->first );
                aTranslatedRegions.push_back(
                    std::make_pair(
                        basegfx::B2DRange(
                            relRegion.getMinimum() + rOffset,
                            relRegion.getMaximum() + rOffset),
                        aIter->second) );
                ++aIter;
            }

            return aTranslatedRegions;
        }

        double DrawShape::getHyperlinkPriority() const
        {
            return getPriority();
        }


        
        

        void DrawShape::enterAnimationMode()
        {
            OSL_ENSURE( !maViewShapes.empty(),
                        "DrawShape::enterAnimationMode(): called on DrawShape without views" );

            if( mnIsAnimatedCount == 0 )
            {
                
                
                ::std::for_each( maViewShapes.begin(),
                                 maViewShapes.end(),
                                 ::boost::mem_fn( &ViewShape::enterAnimationMode ) );
            }

            ++mnIsAnimatedCount;
        }

        void DrawShape::leaveAnimationMode()
        {
            OSL_ENSURE( !maViewShapes.empty(),
                        "DrawShape::leaveAnimationMode(): called on DrawShape without views" );

            --mnIsAnimatedCount;

            if( mnIsAnimatedCount == 0 )
            {
                
                
                ::std::for_each( maViewShapes.begin(),
                                 maViewShapes.end(),
                                 ::boost::mem_fn( &ViewShape::leaveAnimationMode ) );
            }
        }


        
        

        ShapeAttributeLayerSharedPtr DrawShape::createAttributeLayer()
        {
            
            mpAttributeLayer.reset( new ShapeAttributeLayer( mpAttributeLayer ) );

            
            updateStateIds();

            return mpAttributeLayer;
        }

        bool DrawShape::revokeAttributeLayer( const ShapeAttributeLayerSharedPtr& rLayer )
        {
            if( !mpAttributeLayer )
                return false; 

            if( mpAttributeLayer == rLayer )
            {
                
                mpAttributeLayer = mpAttributeLayer->getChildLayer();

                
                
                mbAttributeLayerRevoked = true;

                return true;
            }
            else
            {
                
                return mpAttributeLayer->revokeChildLayer( rLayer );
            }
        }

        ShapeAttributeLayerSharedPtr DrawShape::getTopmostAttributeLayer() const
        {
            return mpAttributeLayer;
        }

        void DrawShape::setVisibility( bool bVisible )
        {
            if( mbIsVisible != bVisible )
            {
                mbIsVisible = bVisible;
                mbForceUpdate = true;
            }
        }

        const DocTreeNodeSupplier& DrawShape::getTreeNodeSupplier() const
        {
            return *this;
        }

        DocTreeNodeSupplier& DrawShape::getTreeNodeSupplier()
        {
            return *this;
        }

        DocTreeNode DrawShape::getSubsetNode() const
        {
            
            return maSubsetting.getSubsetNode();
        }

        AttributableShapeSharedPtr DrawShape::getSubset( const DocTreeNode& rTreeNode ) const
        {
            
            return maSubsetting.getSubsetShape( rTreeNode );
        }

        bool DrawShape::createSubset( AttributableShapeSharedPtr&   o_rSubset,
                                      const DocTreeNode&            rTreeNode )
        {
            
            AttributableShapeSharedPtr pSubset( maSubsetting.getSubsetShape( rTreeNode ) );

            
            
            bool bNewlyCreated( false );

            if( pSubset )
            {
                o_rSubset = pSubset;

                
            }
            else
            {
                
                o_rSubset.reset( new DrawShape( *this,
                                                rTreeNode,
                                                
                                                
                                                
                                                
                                                
                                                mnPriority +
                                                rTreeNode.getStartIndex()/double(SAL_MAX_INT16) ));

                bNewlyCreated = true; 
            }

            
            
            maSubsetting.addSubsetShape( o_rSubset );

            
            maCurrentShapeUnitBounds.reset();

            return bNewlyCreated;
        }

        bool DrawShape::revokeSubset( const AttributableShapeSharedPtr& rShape )
        {
            
            maCurrentShapeUnitBounds.reset();

            
            if( maSubsetting.revokeSubsetShape( rShape ) )
            {
                
                
                
                mbForceUpdate = true;

                
                

                
                
                
                
                
                
                const ShapeAttributeLayerSharedPtr& rAttrLayer(
                    rShape->getTopmostAttributeLayer() );
                if( rAttrLayer &&
                    rAttrLayer->isVisibilityValid() &&
                    rAttrLayer->getVisibility() != isVisible() )
                {
                    const bool bVisibility( rAttrLayer->getVisibility() );

                    
                    if( mpAttributeLayer )
                        mpAttributeLayer->setVisibility( bVisibility );
                    else
                        mbIsVisible = bVisibility;
                }

                

                return true;
            }

            return false;
        }

        sal_Int32 DrawShape::getNumberOfTreeNodes( DocTreeNode::NodeType eNodeType ) const 
        {
            return maSubsetting.getNumberOfTreeNodes( eNodeType );
        }

        DocTreeNode DrawShape::getTreeNode( sal_Int32               nNodeIndex,
                                            DocTreeNode::NodeType   eNodeType ) const 
        {
            if ( hasHyperlinks())
            {
                prepareHyperlinkIndices();
            }

            return maSubsetting.getTreeNode( nNodeIndex, eNodeType );
        }

        sal_Int32 DrawShape::getNumberOfSubsetTreeNodes ( const DocTreeNode&    rParentNode,
                                                          DocTreeNode::NodeType eNodeType ) const 
        {
            return maSubsetting.getNumberOfSubsetTreeNodes( rParentNode, eNodeType );
        }

        DocTreeNode DrawShape::getSubsetTreeNode( const DocTreeNode&    rParentNode,
                                                  sal_Int32             nNodeIndex,
                                                  DocTreeNode::NodeType eNodeType ) const 
        {
            return maSubsetting.getSubsetTreeNode( rParentNode, nNodeIndex, eNodeType );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
