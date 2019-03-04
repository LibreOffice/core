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
#include <basegfx/range/b1drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <sal/log.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <functional>
#include <algorithm>

#include "layermanager.hxx"

using namespace ::com::sun::star;

namespace std
{
    // add operator!= for weak_ptr
    static bool operator!=( slideshow::internal::LayerWeakPtr const& rLHS,
                            slideshow::internal::LayerWeakPtr const& rRHS )
    {
        return rLHS.lock().get() != rRHS.lock().get();
    }
}

namespace slideshow
{
    namespace internal
    {
        template<typename LayerFunc,
                 typename ShapeFunc> void LayerManager::manageViews(
                     LayerFunc layerFunc,
                     ShapeFunc shapeFunc )
        {
            LayerSharedPtr                      pCurrLayer;
            ViewLayerSharedPtr                  pCurrViewLayer;
            for( const auto& rShape : maAllShapes )
            {
                LayerSharedPtr pLayer = rShape.second.lock();
                if( pLayer && pLayer != pCurrLayer )
                {
                    pCurrLayer = pLayer;
                    pCurrViewLayer = layerFunc(pCurrLayer);
                }

                if( pCurrViewLayer )
                    shapeFunc(rShape.first,pCurrViewLayer);
            }
        }

        LayerManager::LayerManager( const UnoViewContainer&    rViews,
                                    bool                       bDisableAnimationZOrder ) :
            mrViews(rViews),
            maLayers(),
            maXShapeHash( 101 ),
            maAllShapes(),
            maUpdateShapes(),
            mnActiveSprites(0),
            mbLayerAssociationDirty(false),
            mbActive(false),
            mbDisableAnimationZOrder(bDisableAnimationZOrder)
        {
            // prevent frequent resizes (won't have more than 4 layers
            // for 99.9% of the cases)
            maLayers.reserve(4);

            // create initial background layer
            maLayers.push_back( Layer::createBackgroundLayer() );

            // init views
            for( const auto& rView : mrViews )
                viewAdded( rView );
        }

        void LayerManager::activate()
        {
            mbActive = true;
            maUpdateShapes.clear(); // update gets forced via area, or
                                    // has happened outside already

            // clear all possibly pending update areas - content
            // is there, already
            for( const auto& pLayer : maLayers )
                pLayer->clearUpdateRanges();

            updateShapeLayers( true/*bSlideBackgoundPainted*/ );
        }

        void LayerManager::deactivate()
        {
            // TODO(F3): This is mostly a hack. Problem is, there's
            // currently no smart way of telling shapes "remove your
            // sprites". Others, like MediaShapes, listen to
            // start/stop animation events, which is too much overhead
            // for all shapes, though.

            const bool bMoreThanOneLayer(maLayers.size() > 1);
            if (mnActiveSprites || bMoreThanOneLayer)
            {
                // clear all viewlayers, dump everything but the
                // background layer - this will also remove all shape
                // sprites
                for (auto& rShape : maAllShapes)
                    rShape.first->clearAllViewLayers();

                for (auto& rShape : maAllShapes)
                    rShape.second.reset();

                if (bMoreThanOneLayer)
                    maLayers.erase(maLayers.begin() + 1, maLayers.end());

                mbLayerAssociationDirty = true;
            }

            mbActive = false;

            // only background layer left
            OSL_ASSERT( maLayers.size() == 1 && maLayers.front()->isBackgroundLayer() );
        }

        void LayerManager::viewAdded( const UnoViewSharedPtr& rView )
        {
            // view must be member of mrViews container
            OSL_ASSERT( std::find(mrViews.begin(),
                                  mrViews.end(),
                                  rView) != mrViews.end() );

            // init view content
            if( mbActive )
                rView->clearAll();

            // add View to all registered shapes
            manageViews(
                [&rView]( const LayerSharedPtr& pLayer )
                { return pLayer->addView( rView ); },
                []( const ShapeSharedPtr& pShape, const ViewLayerSharedPtr& pLayer )
                { return pShape->addViewLayer( pLayer, true ); } );

            // in case we haven't reached all layers from the
            // maAllShapes, issue addView again for good measure
            for( const auto& pLayer : maLayers )
                pLayer->addView( rView );
        }

        void LayerManager::viewRemoved( const UnoViewSharedPtr& rView )
        {
            // view must not be member of mrViews container anymore
            OSL_ASSERT( std::find(mrViews.begin(),
                                  mrViews.end(),
                                  rView) == mrViews.end() );

            // remove View from all registered shapes
            manageViews(
                [&rView]( const LayerSharedPtr& pLayer )
                { return pLayer->removeView( rView ); },
                []( const ShapeSharedPtr& pShape, const ViewLayerSharedPtr& pLayer )
                { return pShape->removeViewLayer( pLayer ); } );

            // in case we haven't reached all layers from the
            // maAllShapes, issue removeView again for good measure
            for( const auto& pLayer : maLayers )
                pLayer->removeView( rView );
        }

        void LayerManager::viewChanged( const UnoViewSharedPtr& rView )
        {
            // view must be member of mrViews container
            OSL_ASSERT( std::find(mrViews.begin(),
                                  mrViews.end(),
                                  rView) != mrViews.end() );

            // TODO(P2): selectively update only changed view
            viewsChanged();
        }

        void LayerManager::viewsChanged()
        {
            if( !mbActive )
                return;

            // clear view area
            for( const auto& pView : mrViews )
                pView->clearAll();

            // TODO(F3): resize and repaint all layers

            // render all shapes
            for( const auto& rShape : maAllShapes )
                rShape.first->render();
        }

        void LayerManager::addShape( const ShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_OR_THROW( rShape, "LayerManager::addShape(): invalid Shape" );

            // add shape to XShape hash map
            if( !maXShapeHash.emplace(rShape->getXShape(),
                                      rShape).second )
            {
                // entry already present, nothing to do
                return;
            }

            // add shape to appropriate layer
            implAddShape( rShape );
        }

        void LayerManager::putShape2BackgroundLayer( LayerShapeMap::value_type& rShapeEntry )
        {
            LayerSharedPtr& rBgLayer( maLayers.front() );
            rBgLayer->setShapeViews(rShapeEntry.first);
            rShapeEntry.second = rBgLayer;
        }

        void LayerManager::implAddShape( const ShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_OR_THROW( rShape, "LayerManager::implAddShape(): invalid Shape" );

            LayerShapeMap::value_type aValue (rShape, LayerWeakPtr());

            OSL_ASSERT( maAllShapes.find(rShape) == maAllShapes.end() ); // shape must not be added already
            mbLayerAssociationDirty = true;

            if( mbDisableAnimationZOrder )
                putShape2BackgroundLayer(
                    *maAllShapes.insert(aValue).first );
            else
                maAllShapes.insert(aValue);

            // update shape, it's just added and not yet painted
            if( rShape->isVisible() )
                notifyShapeUpdate( rShape );
        }

        bool LayerManager::removeShape( const ShapeSharedPtr& rShape )
        {
            // remove shape from XShape hash map
            if( maXShapeHash.erase( rShape->getXShape() ) == 0 )
                return false; // shape not in map

            OSL_ASSERT( maAllShapes.find(rShape) != maAllShapes.end() );

            implRemoveShape( rShape );

            return true;
        }

        void LayerManager::implRemoveShape( const ShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_OR_THROW( rShape, "LayerManager::implRemoveShape(): invalid Shape" );

            const LayerShapeMap::iterator aShapeEntry( maAllShapes.find(rShape) );

            if( aShapeEntry == maAllShapes.end() )
                return;

            const bool bShapeUpdateNotified = maUpdateShapes.erase( rShape ) != 0;

            // Enter shape area to the update area, but only if shape
            // is visible and not in sprite mode (otherwise, updating
            // the area doesn't do actual harm, but costs time)
            // Actually, also add it if it was listed in
            // maUpdateShapes (might have just gone invisible).
            if( bShapeUpdateNotified ||
                (rShape->isVisible() &&
                 !rShape->isBackgroundDetached()) )
            {
                LayerSharedPtr pLayer = aShapeEntry->second.lock();
                if( pLayer )
                {
                    // store area early, once the shape is removed from
                    // the layers, it no longer has any view references
                    pLayer->addUpdateRange( rShape->getUpdateArea() );
                }
            }

            rShape->clearAllViewLayers();
            maAllShapes.erase( aShapeEntry );

            mbLayerAssociationDirty = true;
        }

        ShapeSharedPtr LayerManager::lookupShape( const uno::Reference< drawing::XShape >& xShape ) const
        {
            ENSURE_OR_THROW( xShape.is(), "LayerManager::lookupShape(): invalid Shape" );

            const XShapeHash::const_iterator aIter( maXShapeHash.find( xShape ));
            if( aIter == maXShapeHash.end() )
                return ShapeSharedPtr(); // not found

            // found, return data part of entry pair.
            return aIter->second;
        }

        AttributableShapeSharedPtr LayerManager::getSubsetShape( const AttributableShapeSharedPtr&  rOrigShape,
                                                                 const DocTreeNode&                 rTreeNode )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer

            AttributableShapeSharedPtr pSubset;

            // shape already added?
            if( rOrigShape->createSubset( pSubset,
                                          rTreeNode ) )
            {
                OSL_ENSURE( pSubset, "LayerManager::getSubsetShape(): failed to create subset" );

                // don't add to shape hash, we're dupes to the
                // original XShape anyway - all subset shapes return
                // the same XShape as the original one.

                // add shape to corresponding layer
                implAddShape( pSubset );

                // update original shape, it now shows less content
                // (the subset is removed from its displayed
                // output). Subset shape is updated within
                // implAddShape().
                if( rOrigShape->isVisible() )
                    notifyShapeUpdate( rOrigShape );
            }

            return pSubset;
        }

        void LayerManager::revokeSubset( const AttributableShapeSharedPtr& rOrigShape,
                                         const AttributableShapeSharedPtr& rSubsetShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer

            if( rOrigShape->revokeSubset( rSubsetShape ) )
            {
                OSL_ASSERT( maAllShapes.find(rSubsetShape) != maAllShapes.end() );

                implRemoveShape( rSubsetShape );

                // update original shape, it now shows more content
                // (the subset is added back to its displayed output)
                if( rOrigShape->isVisible() )
                    notifyShapeUpdate( rOrigShape );
            }
        }

        void LayerManager::enterAnimationMode( const AnimatableShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_OR_THROW( rShape, "LayerManager::enterAnimationMode(): invalid Shape" );

            const bool bPrevAnimState( rShape->isBackgroundDetached() );

            rShape->enterAnimationMode();

            // if this call _really_ enabled the animation mode at
            // rShape, insert it to our enter animation queue, to
            // perform the necessary layer reorg lazily on
            // LayerManager::update()/render().
            if( bPrevAnimState != rShape->isBackgroundDetached() )
            {
                ++mnActiveSprites;
                mbLayerAssociationDirty = true;

                // area needs update (shape is removed from normal
                // slide, and now rendered as an autonomous
                // sprite). store in update set
                if( rShape->isVisible() )
                    addUpdateArea( rShape );
            }

            // TODO(P1): this can lead to potential wasted effort, if
            // a shape gets toggled animated/unanimated a few times
            // between two frames, returning to the original state.
        }

        void LayerManager::leaveAnimationMode( const AnimatableShapeSharedPtr& rShape )
        {
            ENSURE_OR_THROW( !maLayers.empty(), "LayerManager::leaveAnimationMode(): no layers" );
            ENSURE_OR_THROW( rShape, "LayerManager::leaveAnimationMode(): invalid Shape" );

            const bool bPrevAnimState( rShape->isBackgroundDetached() );

            rShape->leaveAnimationMode();

            // if this call _really_ ended the animation mode at
            // rShape, insert it to our leave animation queue, to
            // perform the necessary layer reorg lazily on
            // LayerManager::update()/render().
            if( bPrevAnimState != rShape->isBackgroundDetached() )
            {
                --mnActiveSprites;
                mbLayerAssociationDirty = true;

                // shape needs update, no previous rendering, fast
                // update possible.
                if( rShape->isVisible() )
                    notifyShapeUpdate( rShape );
            }

            // TODO(P1): this can lead to potential wasted effort, if
            // a shape gets toggled animated/unanimated a few times
            // between two frames, returning to the original state.
        }

        void LayerManager::notifyShapeUpdate( const ShapeSharedPtr& rShape )
        {
            if( !mbActive || mrViews.empty() )
                return;

            // hidden sprite-shape needs render() call still, to hide sprite
            if( rShape->isVisible() || rShape->isBackgroundDetached() )
                maUpdateShapes.insert( rShape );
            else
                addUpdateArea( rShape );
        }

        bool LayerManager::isUpdatePending() const
        {
            if( !mbActive )
                return false;

            if( mbLayerAssociationDirty || !maUpdateShapes.empty() )
                return true;

            return std::any_of( maLayers.begin(),
                                maLayers.end(),
                                std::mem_fn(&Layer::isUpdatePending) );
        }

        bool LayerManager::updateSprites()
        {
            bool bRet(true);

            // send update() calls to every shape in the
            // maUpdateShapes set, which is _animated_ (i.e. a
            // sprite).
            for( const auto& pShape : maUpdateShapes )
            {
                if( pShape->isBackgroundDetached() )
                {
                    // can update shape directly, without
                    // affecting layer content (shape is
                    // currently displayed in a sprite)
                    if( !pShape->update() )
                        bRet = false; // delay error exit
                }
                else
                {
                    // TODO(P2): addUpdateArea() involves log(n)
                    // search for shape layer. Have a frequent
                    // shape/layer association cache, or ptr back to
                    // layer at the shape?

                    // cannot update shape directly, it's not
                    // animated and update() calls will prolly
                    // overwrite other page content.
                    addUpdateArea( pShape );
                }
            }

            maUpdateShapes.clear();

            return bRet;
        }

        bool LayerManager::update()
        {
            bool bRet = true;

            if( !mbActive )
                return bRet;

            // going to render - better flush any pending layer reorg
            // now
            updateShapeLayers(false);

            // all sprites
            bRet = updateSprites();

            // any non-sprite update areas left?
            if( std::none_of( maLayers.begin(),
                              maLayers.end(),
                              std::mem_fn( &Layer::isUpdatePending ) ) )
                return bRet; // nope, done.

            // update each shape on each layer, that has
            // isUpdatePending()
            bool                                bIsCurrLayerUpdating(false);
            Layer::EndUpdater                   aEndUpdater;
            LayerSharedPtr                      pCurrLayer;
            for( const auto& rShape : maAllShapes )
            {
                LayerSharedPtr pLayer = rShape.second.lock();
                if( pLayer != pCurrLayer )
                {
                    pCurrLayer = pLayer;
                    bIsCurrLayerUpdating = pCurrLayer->isUpdatePending();

                    if( bIsCurrLayerUpdating )
                        aEndUpdater = pCurrLayer->beginUpdate();
                }

                if( bIsCurrLayerUpdating &&
                    !rShape.first->isBackgroundDetached() &&
                    pCurrLayer->isInsideUpdateArea(rShape.first) )
                {
                    if( !rShape.first->render() )
                        bRet = false;
                }
            }

            return bRet;
        }

        namespace
        {
            /** Little wrapper around a Canvas, to render one-shot
                into a canvas
             */
            class DummyLayer : public ViewLayer
            {
            public:
                explicit DummyLayer( const ::cppcanvas::CanvasSharedPtr& rCanvas ) :
                    mpCanvas( rCanvas )
                {
                }

                virtual bool isOnView(ViewSharedPtr const& /*rView*/) const override
                {
                    return true; // visible on all views
                }

                virtual ::cppcanvas::CanvasSharedPtr getCanvas() const override
                {
                    return mpCanvas;
                }

                virtual void clear() const override
                {
                    // NOOP
                }

                virtual void clearAll() const override
                {
                    // NOOP
                }

                virtual ::cppcanvas::CustomSpriteSharedPtr createSprite( const ::basegfx::B2DSize& /*rSpriteSizePixel*/,
                                                                         double                    /*nSpritePrio*/ ) const override
                {
                    ENSURE_OR_THROW( false,
                                      "DummyLayer::createSprite(): This method is not supposed to be called!" );
                    return ::cppcanvas::CustomSpriteSharedPtr();
                }

                virtual void setPriority( const basegfx::B1DRange& /*rRange*/ ) override
                {
                    OSL_FAIL( "BitmapView::setPriority(): This method is not supposed to be called!" );
                }

                virtual css::geometry::IntegerSize2D getTranslationOffset() const override
                {
                    return geometry::IntegerSize2D(0,0);
                }

                virtual ::basegfx::B2DHomMatrix getTransformation() const override
                {
                    return mpCanvas->getTransformation();
                }

                virtual ::basegfx::B2DHomMatrix getSpriteTransformation() const override
                {
                    OSL_FAIL( "BitmapView::getSpriteTransformation(): This method is not supposed to be called!" );
                    return ::basegfx::B2DHomMatrix();
                }

                virtual void setClip( const ::basegfx::B2DPolyPolygon& /*rClip*/ ) override
                {
                    OSL_FAIL( "BitmapView::setClip(): This method is not supposed to be called!" );
                }

                virtual bool resize( const ::basegfx::B2DRange& /*rArea*/ ) override
                {
                    OSL_FAIL( "BitmapView::resize(): This method is not supposed to be called!" );
                    return false;
                }

            private:
                ::cppcanvas::CanvasSharedPtr const mpCanvas;
            };
        }

        bool LayerManager::renderTo( const ::cppcanvas::CanvasSharedPtr& rTargetCanvas ) const
        {
            bool bRet( true );
            ViewLayerSharedPtr pTmpLayer( new DummyLayer( rTargetCanvas ) );

            for( const auto& rShape : maAllShapes )
            {
                try
                {
                    // forward to all shape's addViewLayer method (which
                    // we request to render the Shape on the new
                    // ViewLayer. Since we add the shapes in the
                    // maShapeSet order (which is also the render order),
                    // this is equivalent to a subsequent render() call)
                    rShape.first->addViewLayer( pTmpLayer,
                                                true );

                    // and remove again, this is only temporary
                    rShape.first->removeViewLayer( pTmpLayer );
                }
                catch( uno::Exception& )
                {
                    // TODO(E1): Might be superfluous. Nowadays,
                    // addViewLayer swallows all errors, anyway.
                    SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
                    // at least one shape could not be rendered
                    bRet = false;
                }
            }

            return bRet;
        }

        void LayerManager::addUpdateArea( ShapeSharedPtr const& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_OR_THROW( rShape, "LayerManager::addUpdateArea(): invalid Shape" );

            const LayerShapeMap::const_iterator aShapeEntry( maAllShapes.find(rShape) );

            if( aShapeEntry == maAllShapes.end() )
                return;

            LayerSharedPtr pLayer = aShapeEntry->second.lock();
            if( pLayer )
                pLayer->addUpdateRange( rShape->getUpdateArea() );
        }

        void LayerManager::commitLayerChanges( std::size_t              nCurrLayerIndex,
                                               LayerShapeMap::const_iterator  aFirstLayerShape,
                                               const LayerShapeMap::const_iterator&  aEndLayerShapes )
        {
            const bool bLayerExists( maLayers.size() > nCurrLayerIndex );
            if( !bLayerExists )
                return;

            const LayerSharedPtr& rLayer( maLayers.at(nCurrLayerIndex) );
            const bool bLayerResized( rLayer->commitBounds() );
            rLayer->setPriority( basegfx::B1DRange(nCurrLayerIndex,
                                                   nCurrLayerIndex+1) );

            if( !bLayerResized )
                return;

            // need to re-render whole layer - start from
            // clean state
            rLayer->clearContent();

            // render and remove from update set
            while( aFirstLayerShape != aEndLayerShapes )
            {
                maUpdateShapes.erase(aFirstLayerShape->first);
                aFirstLayerShape->first->render();
                ++aFirstLayerShape;
            }
        }

        LayerSharedPtr LayerManager::createForegroundLayer() const
        {
            OSL_ASSERT( mbActive );

            LayerSharedPtr pLayer( Layer::createLayer() );

            // create ViewLayers for all registered views, and add to
            // newly created layer.
            for( const auto& rView : mrViews )
                pLayer->addView( rView );

            return pLayer;
        }

        void LayerManager::updateShapeLayers( bool bBackgroundLayerPainted )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            OSL_ASSERT( mbActive );

            // do we need to process shapes?
            if( !mbLayerAssociationDirty )
                return;

            if( mbDisableAnimationZOrder )
            {
                // layer setup happened elsewhere, is only bg layer
                // anyway.
                mbLayerAssociationDirty = false;
                return;
            }

            // scan through maAllShapes, and determine shape animation
            // discontinuities: when a shape that has
            // isBackgroundDetached() return false follows a shape
            // with isBackgroundDetached() true, the former and all
            // following ones must be moved into an own layer.

            // to avoid tons of temporaries, create weak_ptr to Layers
            // beforehand
            std::vector< LayerWeakPtr > aWeakLayers(maLayers.size());
            std::copy(maLayers.begin(),maLayers.end(),aWeakLayers.begin());

            std::size_t                   nCurrLayerIndex(0);
            bool                          bIsBackgroundLayer(true);
            bool                          bLastWasBackgroundDetached(false); // last shape sprite state
            LayerShapeMap::iterator       aCurrShapeEntry( maAllShapes.begin() );
            LayerShapeMap::iterator       aCurrLayerFirstShapeEntry( maAllShapes.begin() );
            const LayerShapeMap::iterator aEndShapeEntry ( maAllShapes.end() );
            while( aCurrShapeEntry != aEndShapeEntry )
            {
                const ShapeSharedPtr pCurrShape( aCurrShapeEntry->first );
                const bool bThisIsBackgroundDetached(
                    pCurrShape->isBackgroundDetached() );

                if( bLastWasBackgroundDetached &&
                    !bThisIsBackgroundDetached )
                {
                    // discontinuity found - current shape needs to
                    // get into a new layer


                    // commit changes to previous layer
                    commitLayerChanges(nCurrLayerIndex,
                                       aCurrLayerFirstShapeEntry,
                                       aCurrShapeEntry);
                    aCurrLayerFirstShapeEntry=aCurrShapeEntry;
                    ++nCurrLayerIndex;
                    bIsBackgroundLayer = false;

                    if( aWeakLayers.size() <= nCurrLayerIndex ||
                        aWeakLayers.at(nCurrLayerIndex) != aCurrShapeEntry->second )
                    {
                        // no more layers left, or shape was not
                        // member of this layer - create a new one
                        maLayers.insert( maLayers.begin()+nCurrLayerIndex,
                                         createForegroundLayer() );
                        aWeakLayers.insert( aWeakLayers.begin()+nCurrLayerIndex,
                                            maLayers[nCurrLayerIndex] );
                    }
                }

                OSL_ASSERT( maLayers.size() == aWeakLayers.size() );

                // note: using indices here, since vector::insert
                // above invalidates iterators
                LayerSharedPtr& rCurrLayer( maLayers.at(nCurrLayerIndex) );
                LayerWeakPtr& rCurrWeakLayer( aWeakLayers.at(nCurrLayerIndex) );
                if( rCurrWeakLayer != aCurrShapeEntry->second )
                {
                    // mismatch: shape is not contained in current
                    // layer - move shape to that layer, then.
                    maLayers.at(nCurrLayerIndex)->setShapeViews(
                        pCurrShape );

                    // layer got new shape(s), need full repaint, if
                    // non-sprite shape
                    if( !bThisIsBackgroundDetached && pCurrShape->isVisible() )
                    {
                        LayerSharedPtr pOldLayer( aCurrShapeEntry->second.lock() );
                        if( pOldLayer )
                        {
                            // old layer still valid? then we need to
                            // repaint former shape area
                            pOldLayer->addUpdateRange(
                                pCurrShape->getUpdateArea() );
                        }

                        // render on new layer (only if not
                        // explicitly disabled)
                        if( !(bBackgroundLayerPainted && bIsBackgroundLayer) )
                            maUpdateShapes.insert( pCurrShape );
                    }

                    aCurrShapeEntry->second = rCurrWeakLayer;
                }

                // update layerbounds regardless of the fact that the
                // shape might be contained in said layer
                // already. updateBounds() is dumb and needs to
                // collect all shape bounds.
                // of course, no need to expand layer bounds for
                // shapes that reside in sprites themselves.
                if( !bThisIsBackgroundDetached && !bIsBackgroundLayer )
                    rCurrLayer->updateBounds( pCurrShape );

                bLastWasBackgroundDetached = bThisIsBackgroundDetached;
                ++aCurrShapeEntry;
            }

            // commit very last layer data
            commitLayerChanges(nCurrLayerIndex,
                               aCurrLayerFirstShapeEntry,
                               aCurrShapeEntry);

            // any layers left? Bin them!
            if( maLayers.size() > nCurrLayerIndex+1 )
                maLayers.erase(maLayers.begin()+nCurrLayerIndex+1,
                               maLayers.end());

            mbLayerAssociationDirty = false;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
