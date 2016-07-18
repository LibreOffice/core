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

#include <tools/diagnose_ex.h>

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
#include <iterator>
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

#include <math.h>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {


        // Private methods


        GDIMetaFileSharedPtr const & DrawShape::forceScrollTextMetaFile()
        {
            if ((mnCurrMtfLoadFlags & MTF_LOAD_SCROLL_TEXT_MTF) != MTF_LOAD_SCROLL_TEXT_MTF)
            {
                // reload with added flags:
                mpCurrMtf.reset( new GDIMetaFile );
                mnCurrMtfLoadFlags |= MTF_LOAD_SCROLL_TEXT_MTF;
                getMetaFile(
                    uno::Reference<lang::XComponent>(mxShape, uno::UNO_QUERY),
                    mxPage, *mpCurrMtf, mnCurrMtfLoadFlags,
                    mxComponentContext );

                // TODO(F1): Currently, the scroll metafile will
                // never contain any verbose text comments. Thus,
                // can only display the full mtf content, no
                // subsets.
                maSubsetting.reset( mpCurrMtf );

                // adapt maBounds. the requested scroll text metafile
                // will typically have dimension different from the
                // actual shape
                ::basegfx::B2DRectangle aScrollRect, aPaintRect;
                ENSURE_OR_THROW( getRectanglesFromScrollMtf( aScrollRect,
                                                              aPaintRect,
                                                              mpCurrMtf ),
                                  "DrawShape::forceScrollTextMetaFile(): Could "
                                  "not extract scroll anim rectangles from mtf" );

                // take the larger one of the two rectangles (that
                // should be the bound rect of the retrieved
                // metafile)
                if( aScrollRect.isInside( aPaintRect ) )
                    maBounds = aScrollRect;
                else
                    maBounds = aPaintRect;
            }
            return mpCurrMtf;
        }

        void DrawShape::updateStateIds() const
        {
            // Update the states, we've just redrawn or created a new
            // attribute layer.
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

            // will perform the update now, clear update-enforcing
            // flags
            mbForceUpdate = false;
            mbAttributeLayerRevoked = false;

            ENSURE_OR_RETURN_FALSE( !maViewShapes.empty(),
                               "DrawShape::implRender(): render called on DrawShape without views" );

            if( maBounds.isEmpty() )
            {
                // zero-sized shapes are effectively invisible,
                // thus, we save us the rendering...
                return true;
            }

            // redraw all view shapes, by calling their update() method
            ViewShape::RenderArgs renderArgs( getViewRenderArgs() );
            bool bVisible = isVisible();
            if( ::std::count_if( maViewShapes.begin(),
                                 maViewShapes.end(),
                                 [this, &bVisible, &renderArgs, &nUpdateFlags]
                                 ( const ViewShapeSharedPtr& pShape )
                                 { return pShape->update( this->mpCurrMtf,
                                                          renderArgs,
                                                          nUpdateFlags,
                                                          bVisible ); } )
                != static_cast<ViewShapeVector::difference_type>(maViewShapes.size()) )
            {
                // at least one of the ViewShape::update() calls did return
                // false - update failed on at least one ViewLayer
                return false;
            }

            // successfully redrawn - update state IDs to detect next changes
            updateStateIds();

            return true;
        }

        int DrawShape::getUpdateFlags() const
        {
            // default: update nothing, unless ShapeAttributeStack
            // tells us below, or if the attribute layer was revoked
            int nUpdateFlags(ViewShape::NONE);

            // possibly the whole shape content changed
            if( mbAttributeLayerRevoked )
                nUpdateFlags = ViewShape::CONTENT;


            // determine what has to be updated


            // do we have an attribute layer?
            if( mpAttributeLayer )
            {
                // Prevent nUpdateFlags to be modified when the shape is not
                // visible, except when it just was hidden.
                if (mpAttributeLayer->getVisibility()
                    || mpAttributeLayer->getVisibilityState() != mnAttributeVisibilityState )
                {
                    if (mpAttributeLayer->getVisibilityState() != mnAttributeVisibilityState )
                    {
                        // Change of the visibility state is mapped to
                        // content change because when the visibility
                        // changes then usually a sprite is shown or hidden
                        // and the background under has to be painted once.
                        nUpdateFlags |= ViewShape::CONTENT;
                    }

                    // TODO(P1): This can be done without conditional branching.
                    // See HAKMEM.
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

            // perform the cheapest check first
            if( rSubsets.empty() )
            {
                // if subset contains the whole shape, no need to call
                // the somewhat expensive bound calculation, since as
                // long as the subset is empty, this branch will be
                // taken.
                return aDefaultBounds;
            }
            else
            {
                OSL_ENSURE( rSubsets.size() != 1 ||
                            !rSubsets.front().isEmpty(),
                            "DrawShape::getActualUnitShapeBounds() expects a "
                            "_non-empty_ subset vector for a subsetted shape!" );

                // are the cached bounds still valid?
                if( !maCurrentShapeUnitBounds )
                {
                    // no, (re)generate them
                    // =====================

                    // setup cached values to defaults (might fail to
                    // retrieve true bounds below)
                    maCurrentShapeUnitBounds.reset( aDefaultBounds );

                    // TODO(P2): the subset of the master shape (that from
                    // which the subsets are subtracted) changes
                    // relatively often (every time a subset shape is
                    // added or removed). Maybe we should exclude it here,
                    // always assuming full bounds?

                    ::cppcanvas::CanvasSharedPtr pDestinationCanvas(
                        maViewShapes.front()->getViewLayer()->getCanvas() );

                    // TODO(Q2): Although this _is_ currently
                    // view-agnostic, it might not stay like
                    // that. Maybe this method should again be moved
                    // to the ViewShape
                    ::cppcanvas::RendererSharedPtr pRenderer(
                        maViewShapes.front()->getRenderer(
                            pDestinationCanvas, mpCurrMtf, mpAttributeLayer ) );

                    // If we cannot not prefetch, be defensive and assume
                    // full shape size
                    if( pRenderer )
                    {
                        // temporarily, switch total transformation to identity
                        // (need the bounds in the [0,1]x[0,1] unit coordinate
                        // system.
                        ::basegfx::B2DHomMatrix      aEmptyTransformation;

                        ::basegfx::B2DHomMatrix      aOldTransform( pDestinationCanvas->getTransformation() );
                        pDestinationCanvas->setTransformation( aEmptyTransformation );
                        pRenderer->setTransformation( aEmptyTransformation );

                        // restore old transformation when leaving the scope
                        const ::comphelper::ScopeGuard aGuard(
                            [&pDestinationCanvas, &aOldTransform]()
                            { return pDestinationCanvas->setTransformation( aOldTransform ); } );


                        // retrieve bounds for subset of whole metafile


                        ::basegfx::B2DRange aTotalBounds;

                        // cannot use ::boost::bind, ::basegfx::B2DRange::expand()
                        // is overloaded.
                        for( const auto& rDocTreeNode : rSubsets )
                            aTotalBounds.expand( pRenderer->getSubsetArea(
                                                     rDocTreeNode.getStartIndex(),
                                                     rDocTreeNode.getEndIndex() ) );

                        OSL_ENSURE( aTotalBounds.getMinX() >= -0.1 &&
                                    aTotalBounds.getMinY() >= -0.1 &&
                                    aTotalBounds.getMaxX() <= 1.1 &&
                                    aTotalBounds.getMaxY() <= 1.1,
                                    "DrawShape::getActualUnitShapeBounds(): bounds noticeably larger than original shape - clipping!" );

                        // really make sure no shape appears larger than its
                        // original bounds (there _are_ some pathologic cases,
                        // especially when imported from PPT, that have
                        // e.g. obscenely large polygon bounds)
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
            maAnimationFrames(), // empty, we don't have no intrinsic animation
            mnCurrFrame(0),
            mpCurrMtf(),
            mnCurrMtfLoadFlags( bForeignSource
                                ? MTF_LOAD_FOREIGN_SOURCE : MTF_LOAD_NONE ),
            maCurrentShapeUnitBounds(),
            mnPriority( nPrio ), // TODO(F1): When ZOrder someday becomes usable: make this ( getAPIShapePrio( xShape ) ),
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

            // check for drawing layer animations:
            drawing::TextAnimationKind eKind = drawing::TextAnimationKind_NONE;
            uno::Reference<beans::XPropertySet> xPropSet( mxShape,
                                                          uno::UNO_QUERY );
            if( xPropSet.is() )
                getPropertyValue( eKind, xPropSet,
                                  "TextAnimationKind" );
            mbDrawingLayerAnim = (eKind != drawing::TextAnimationKind_NONE);

            // must NOT be called from within initializer list, uses
            // state from mnCurrMtfLoadFlags!
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
            mnPriority( nPrio ), // TODO(F1): When ZOrder someday becomes usable: make this ( getAPIShapePrio( xShape ) ),
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
            maAnimationFrames(), // don't copy animations for subsets,
                                 // only the current frame!
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

            // xxx todo: currently not implemented for subsetted shapes;
            //           would mean modifying set of hyperlink regions when
            //           subsetting text portions. N.B.: there's already an
            //           issue for this #i72828#
        }


        // Public methods


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
                    std::mem_fn(&MtfAnimationFrame::getDuration) );

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
                // dispose intrinsic animation activity, else, it will
                // linger forever
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
            // already added?
            if( ::std::any_of( maViewShapes.begin(),
                               maViewShapes.end(),
                               [&rNewLayer]
                               ( const ViewShapeSharedPtr& pShape )
                               { return rNewLayer == pShape->getViewLayer(); } ) )
            {
                // yes, nothing to do
                return;
            }

            ViewShapeSharedPtr pNewShape( new ViewShape( rNewLayer ) );

            maViewShapes.push_back( pNewShape );

            // pass on animation state
            if( mnIsAnimatedCount )
            {
                for( int i=0; i<mnIsAnimatedCount; ++i )
                    pNewShape->enterAnimationMode();
            }

            // render the Shape on the newly added ViewLayer
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
                                        [&rLayer]
                                        ( const ViewShapeSharedPtr& pShape )
                                        { return rLayer == pShape->getViewLayer(); } ) < 2,
                        "DrawShape::removeViewLayer(): Duplicate ViewLayer entries!" );

            ViewShapeVector::iterator aIter;

            if( (aIter=::std::remove_if( maViewShapes.begin(),
                                         aEnd,
                                         [&rLayer]
                                         ( const ViewShapeSharedPtr& pShape )
                                         { return rLayer == pShape->getViewLayer(); } ) )  == aEnd )
            {
                // view layer seemingly was not added, failed
                return false;
            }

            // actually erase from container
            maViewShapes.erase( aIter, aEnd );

            return true;
        }

        void DrawShape::clearAllViewLayers()
        {
            maViewShapes.clear();
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
            // force redraw. Have to also pass on the update flags,
            // because e.g. content update (regeneration of the
            // metafile renderer) is normally not performed. A simple
            // ViewShape::FORCE would only paint the metafile in its
            // old state.
            return implRender( ViewShape::FORCE | getUpdateFlags() );
        }

        bool DrawShape::isContentChanged() const
        {
            return mbForceUpdate ||
                getUpdateFlags() != ViewShape::NONE;
        }


        ::basegfx::B2DRectangle DrawShape::getBounds() const
        {
            // little optimization: for non-modified shapes, we don't
            // create an ShapeAttributeStack, and therefore also don't
            // have to check it.
            return getShapePosSize( maBounds,
                                    mpAttributeLayer );
        }

        ::basegfx::B2DRectangle DrawShape::getDomBounds() const
        {
            return maBounds;
        }

        ::basegfx::B2DRectangle DrawShape::getUpdateArea() const
        {
            ::basegfx::B2DRectangle aBounds;

            // an already empty shape bound need no further
            // treatment. In fact, any changes applied below would
            // actually remove the special empty state, thus, don't
            // change!
            if( !maBounds.isEmpty() )
            {
                basegfx::B2DRectangle aUnitBounds(0.0,0.0,1.0,1.0);

                if( !maViewShapes.empty() )
                    aUnitBounds = getActualUnitShapeBounds();

                if( !aUnitBounds.isEmpty() )
                {
                    if( mpAttributeLayer )
                    {
                        // calc actual shape area (in user coordinate
                        // space) from the transformation as given by the
                        // shape attribute layer
                        aBounds = getShapeUpdateArea( aUnitBounds,
                                                      getShapeTransformation( getBounds(),
                                                                              mpAttributeLayer ),
                                                      mpAttributeLayer );
                    }
                    else
                    {
                        // no attribute layer, thus, the true shape bounds
                        // can be directly derived from the XShape bound
                        // attribute
                        aBounds = getShapeUpdateArea( aUnitBounds,
                                                      maBounds );
                    }

                    if( !maViewShapes.empty() )
                    {
                        // determine border needed for antialiasing the shape
                        ::basegfx::B2DSize aAABorder(0.0,0.0);

                        // for every view, get AA border and 'expand' aAABorder
                        // appropriately.
                        for( const auto& rViewShape : maViewShapes )
                        {
                            const ::basegfx::B2DSize rShapeBorder( rViewShape->getAntialiasingBorder() );

                            aAABorder.setX( ::std::max(
                                    rShapeBorder.getX(),
                                    aAABorder.getX() ) );
                            aAABorder.setY( ::std::max(
                                    rShapeBorder.getY(),
                                    aAABorder.getY() ) );
                        }

                        // add calculated AA border to aBounds
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
                // check whether visibility and alpha are not default
                // (mpAttributeLayer->isVisibilityValid() returns true
                // then): bVisible becomes true, if shape visibility
                // is on and alpha is not 0.0 (fully transparent)
                if( mpAttributeLayer->isVisibilityValid() )
                    bIsVisible = mpAttributeLayer->getVisibility();

                // only touch bIsVisible, if the shape is still
                // visible - if getVisibility already made us
                // invisible, no alpha value will make us appear
                // again.
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

        void DrawShape::setIntrinsicAnimationFrame( ::std::size_t nCurrFrame )
        {
            ENSURE_OR_RETURN_VOID( nCurrFrame < maAnimationFrames.size(),
                               "DrawShape::setIntrinsicAnimationFrame(): frame index out of bounds" );

            if( mnCurrFrame != nCurrFrame )
            {
                mnCurrFrame   = nCurrFrame;
                mpCurrMtf     = maAnimationFrames[ mnCurrFrame ].mpMtf;
                mbForceUpdate = true;
            }
        }

        // hyperlink support
        void DrawShape::prepareHyperlinkIndices() const
        {
            if ( !maHyperlinkIndices.empty())
            {
                maHyperlinkIndices.clear();
                maHyperlinkRegions.clear();
            }

            sal_Int32 nIndex = 0;
            for ( MetaAction * pCurrAct = mpCurrMtf->FirstAction();
                  pCurrAct != nullptr; pCurrAct = mpCurrMtf->NextAction() )
            {
                if (pCurrAct->GetType() == MetaActionType::COMMENT) {
                    MetaCommentAction * pAct =
                        static_cast<MetaCommentAction *>(pCurrAct);
                    // skip comment if not a special XTEXT comment
                    if (pAct->GetComment().equalsIgnoreAsciiCase("FIELD_SEQ_BEGIN") &&
                        // e.g. date field doesn't have data!
                        // currently assuming that only url field, this is
                        // somehow fragile! xxx todo if possible
                        pAct->GetData() != nullptr &&
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
                             // pending end is expected:
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

            // late init, determine regions:
            if( !maHyperlinkRegions.empty() &&
                !maViewShapes.empty() &&
                // region already inited?
                maHyperlinkRegions.front().first.getWidth() == 0 &&
                maHyperlinkRegions.front().first.getHeight() == 0 &&
                maHyperlinkRegions.size() == maHyperlinkIndices.size() )
            {
                // TODO(Q2): Although this _is_ currently
                // view-agnostic, it might not stay like that.
                ViewShapeSharedPtr const& pViewShape = maViewShapes.front();
                cppcanvas::CanvasSharedPtr const pCanvas(
                    pViewShape->getViewLayer()->getCanvas() );

                // reuse Renderer of first view shape:
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


                    ::cppcanvas::Canvas* pTmpCanvas = pCanvas.get();
                    comphelper::ScopeGuard const resetOldTransformation(
                        [&aOldTransform, &pTmpCanvas]()
                        { return pTmpCanvas->setTransformation( aOldTransform ); } );

                    aTransform.scale( maBounds.getWidth(),
                                      maBounds.getHeight() );
                    pRenderer->setTransformation( aTransform );
                    pRenderer->setClip();

                    for( std::size_t pos = maHyperlinkRegions.size(); pos--; )
                    {
                        // get region:
                        HyperlinkIndexPair const& rIndices = maHyperlinkIndices[pos];
                        basegfx::B2DRectangle const region(
                            pRenderer->getSubsetArea( rIndices.first,
                                                      rIndices.second ));
                        maHyperlinkRegions[pos].first = region;
                    }
                }
            }

            // shift shape-relative hyperlink regions to
            // slide-absolute position

            HyperlinkRegions aTranslatedRegions;

            // increase capacity to same size as the container for
            // shape-relative hyperlink regions to avoid reallocation
            aTranslatedRegions.reserve( maHyperlinkRegions.size() );
            const basegfx::B2DPoint& rOffset(getBounds().getMinimum());
            for( const auto& cp : maHyperlinkRegions )
            {
                basegfx::B2DRange const& relRegion( cp.first );
                aTranslatedRegions.push_back(
                    std::make_pair(
                        basegfx::B2DRange(
                            relRegion.getMinimum() + rOffset,
                            relRegion.getMaximum() + rOffset),
                        cp.second) );
            }

            return aTranslatedRegions;
        }

        double DrawShape::getHyperlinkPriority() const
        {
            return getPriority();
        }


        // AnimatableShape methods


        void DrawShape::enterAnimationMode()
        {
            OSL_ENSURE( !maViewShapes.empty(),
                        "DrawShape::enterAnimationMode(): called on DrawShape without views" );

            if( mnIsAnimatedCount == 0 )
            {
                // notify all ViewShapes, by calling their enterAnimationMode method.
                // We're now entering animation mode
                for( const auto& rViewShape : maViewShapes )
                    rViewShape->enterAnimationMode();
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
                // notify all ViewShapes, by calling their leaveAnimationMode method.
                // we're now leaving animation mode
                for( const auto& rViewShape : maViewShapes )
                    rViewShape->leaveAnimationMode();
            }
        }


        // AttributableShape methods


        ShapeAttributeLayerSharedPtr DrawShape::createAttributeLayer()
        {
            // create new layer, with last as its new child
            mpAttributeLayer.reset( new ShapeAttributeLayer( mpAttributeLayer ) );

            // Update the local state ids to reflect those of the new layer.
            updateStateIds();

            return mpAttributeLayer;
        }

        bool DrawShape::revokeAttributeLayer( const ShapeAttributeLayerSharedPtr& rLayer )
        {
            if( !mpAttributeLayer )
                return false; // no layers

            if( mpAttributeLayer == rLayer )
            {
                // it's the toplevel layer
                mpAttributeLayer = mpAttributeLayer->getChildLayer();

                // force content redraw, all state variables have
                // possibly changed
                mbAttributeLayerRevoked = true;

                return true;
            }
            else
            {
                // pass on to the layer, to try its children
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
            // forward to delegate
            return maSubsetting.getSubsetNode();
        }

        AttributableShapeSharedPtr DrawShape::getSubset( const DocTreeNode& rTreeNode ) const
        {
            // forward to delegate
            return maSubsetting.getSubsetShape( rTreeNode );
        }

        bool DrawShape::createSubset( AttributableShapeSharedPtr&   o_rSubset,
                                      const DocTreeNode&            rTreeNode )
        {
            // subset shape already created for this DocTreeNode?
            AttributableShapeSharedPtr pSubset( maSubsetting.getSubsetShape( rTreeNode ) );

            // when true, this method has created a new subset
            // DrawShape
            bool bNewlyCreated( false );

            if( pSubset )
            {
                o_rSubset = pSubset;

                // reusing existing subset
            }
            else
            {
                // not yet created, init entry
                o_rSubset.reset( new DrawShape( *this,
                                                rTreeNode,
                                                // TODO(Q3): That's a
                                                // hack. We assume
                                                // that start and end
                                                // index will always
                                                // be less than 65535
                                                mnPriority +
                                                rTreeNode.getStartIndex()/double(SAL_MAX_INT16) ));

                bNewlyCreated = true; // subset newly created
            }

            // always register shape at DrawShapeSubsetting, to keep
            // refcount up-to-date
            maSubsetting.addSubsetShape( o_rSubset );

            // flush bounds cache
            maCurrentShapeUnitBounds.reset();

            return bNewlyCreated;
        }

        bool DrawShape::revokeSubset( const AttributableShapeSharedPtr& rShape )
        {
            // flush bounds cache
            maCurrentShapeUnitBounds.reset();

            // forward to delegate
            if( maSubsetting.revokeSubsetShape( rShape ) )
            {
                // force redraw, our content has possibly changed (as
                // one of the subsets now display within our shape
                // again).
                mbForceUpdate = true;

                // #i47428# TEMP FIX: synchronize visibility of subset
                // with parent.

                // TODO(F3): Remove here, and implement
                // TEXT_ONLY/BACKGROUND_ONLY with the proverbial
                // additional level of indirection: create a
                // persistent subset, containing all text/only the
                // background respectively. From _that_ object,
                // generate the temporary character subset shapes.
                const ShapeAttributeLayerSharedPtr& rAttrLayer(
                    rShape->getTopmostAttributeLayer() );
                if( rAttrLayer &&
                    rAttrLayer->isVisibilityValid() &&
                    rAttrLayer->getVisibility() != isVisible() )
                {
                    const bool bVisibility( rAttrLayer->getVisibility() );

                    // visibilities differ - adjust ours, then
                    if( mpAttributeLayer )
                        mpAttributeLayer->setVisibility( bVisibility );
                    else
                        mbIsVisible = bVisibility;
                }

                // END TEMP FIX

                return true;
            }

            return false;
        }

        sal_Int32 DrawShape::getNumberOfTreeNodes( DocTreeNode::NodeType eNodeType ) const // throw ShapeLoadFailedException
        {
            return maSubsetting.getNumberOfTreeNodes( eNodeType );
        }

        DocTreeNode DrawShape::getTreeNode( sal_Int32               nNodeIndex,
                                            DocTreeNode::NodeType   eNodeType ) const // throw ShapeLoadFailedException
        {
            if ( hasHyperlinks())
            {
                prepareHyperlinkIndices();
            }

            return maSubsetting.getTreeNode( nNodeIndex, eNodeType );
        }

        sal_Int32 DrawShape::getNumberOfSubsetTreeNodes ( const DocTreeNode&    rParentNode,
                                                          DocTreeNode::NodeType eNodeType ) const // throw ShapeLoadFailedException
        {
            return maSubsetting.getNumberOfSubsetTreeNodes( rParentNode, eNodeType );
        }

        DocTreeNode DrawShape::getSubsetTreeNode( const DocTreeNode&    rParentNode,
                                                  sal_Int32             nNodeIndex,
                                                  DocTreeNode::NodeType eNodeType ) const // throw ShapeLoadFailedException
        {
            return maSubsetting.getSubsetTreeNode( rParentNode, nNodeIndex, eNodeType );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
