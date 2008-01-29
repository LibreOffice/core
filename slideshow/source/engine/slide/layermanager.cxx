/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layermanager.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:03:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <basegfx/range/b1drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <boost/bind.hpp>
#include <algorithm>

#include "layermanager.hxx"

using namespace ::com::sun::star;

namespace boost
{
    // add operator!= for weak_ptr
    inline bool operator!=( slideshow::internal::LayerWeakPtr const& rLHS,
                            slideshow::internal::LayerWeakPtr const& rRHS )
    {
        return (rLHS<rRHS) || (rRHS<rLHS);
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
            LayerShapeSet::const_iterator       aIter( maAllShapes.begin() );
            const LayerShapeSet::const_iterator aEnd ( maAllShapes.end() );
            while( aIter != aEnd )
            {
                LayerSharedPtr pLayer = aIter->mpLayer.lock();
                if( pLayer && pLayer != pCurrLayer )
                {
                    pCurrLayer = pLayer;
                    pCurrViewLayer = layerFunc(pCurrLayer);
                }

                if( pCurrViewLayer )
                    shapeFunc(aIter->mpShape,pCurrViewLayer);

                ++aIter;
            }
        }

        LayerManager::LayerManager( const UnoViewContainer&    rViews,
                                    const ::basegfx::B2DRange& rPageBounds,
                                    bool                       bDisableAnimationZOrder ) :
            mrViews(rViews),
            maLayers(),
            maXShapeHash( 101 ),
            maAllShapes(),
            maUpdateShapes(),
            maPageBounds( rPageBounds ),
            mnActiveSprites(0),
            mbLayerAssociationDirty(false),
            mbActive(false),
            mbDisableAnimationZOrder(bDisableAnimationZOrder)
        {
            // prevent frequent resizes (won't have more than 4 layers
            // for 99.9% of the cases)
            maLayers.reserve(4);

            // create initial background layer
            maLayers.push_back(
                    Layer::createBackgroundLayer(
                        maPageBounds ));

            // init views
            std::for_each( mrViews.begin(),
                           mrViews.end(),
                           ::boost::bind(&LayerManager::viewAdded,
                                         this,
                                         _1) );
        }

        void LayerManager::activate( bool bSlideBackgoundPainted )
        {
            mbActive = true;
            maUpdateShapes.clear(); // update gets forced via area, or
                                    // has happend outside already

            if( !bSlideBackgoundPainted )
            {
                std::for_each(mrViews.begin(),
                              mrViews.end(),
                              boost::mem_fn(&View::clearAll));

                // force update of whole slide area
                std::for_each( maLayers.begin(),
                               maLayers.end(),
                               boost::bind( &Layer::addUpdateRange,
                                            _1,
                                            boost::cref(maPageBounds) ));
            }
            else
            {
                // clear all possibly pending update areas - content
                // is there, already
                std::for_each( maLayers.begin(),
                               maLayers.end(),
                               boost::mem_fn( &Layer::clearUpdateRanges ));
            }

            updateShapeLayers( bSlideBackgoundPainted );
        }

        void LayerManager::deactivate()
        {
            // TODO(F3): This is mostly a hack. Problem is, there's
            // currently no smart way of telling shapes "remove your
            // sprites". Others, like MediaShapes, listen to
            // start/stop animation events, which is too much overhead
            // for all shapes, though.

            const bool bMoreThanOneLayer(maLayers.size() > 1);
            if( mnActiveSprites || bMoreThanOneLayer )
            {
                // clear all viewlayers, dump everything but the
                // background layer - this will also remove all shape
                // sprites
                std::for_each(maAllShapes.begin(),
                              maAllShapes.end(),
                              boost::bind( &Shape::clearAllViewLayers,
                                           boost::bind( &ShapeEntry::getShape,
                                                        _1 )));

                if( bMoreThanOneLayer )
                    maLayers.erase(maLayers.begin()+1,
                                   maLayers.end());

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
                boost::bind(&Layer::addView,
                            _1,
                            boost::cref(rView)),
                // repaint on view add
                boost::bind(&Shape::addViewLayer,
                            _1,
                            _2,
                            true) );

            // in case we haven't reached all layers from the
            // maAllShapes, issue addView again for good measure
            std::for_each( maLayers.begin(),
                           maLayers.end(),
                           boost::bind( &Layer::addView,
                                        _1,
                                        boost::cref(rView) ));
        }

        void LayerManager::viewRemoved( const UnoViewSharedPtr& rView )
        {
            // view must not be member of mrViews container anymore
            OSL_ASSERT( std::find(mrViews.begin(),
                                  mrViews.end(),
                                  rView) == mrViews.end() );

            // remove View from all registered shapes
            manageViews(
                boost::bind(&Layer::removeView,
                            _1,
                            boost::cref(rView)),
                boost::bind(&Shape::removeViewLayer,
                            _1,
                            _2) );

            // in case we haven't reached all layers from the
            // maAllShapes, issue removeView again for good measure
            std::for_each( maLayers.begin(),
                           maLayers.end(),
                           boost::bind( &Layer::removeView,
                                        _1,
                                        boost::cref(rView) ));
        }

        void LayerManager::viewChanged( const UnoViewSharedPtr& rView )
        {
            (void)rView;

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
            ::std::for_each( mrViews.begin(),
                             mrViews.end(),
                             ::boost::mem_fn(&View::clearAll) );

            // TODO(F3): resize and repaint all layers

            // render all shapes
            std::for_each( maAllShapes.begin(),
                           maAllShapes.end(),
                           boost::bind(&Shape::render,
                                       boost::bind(&ShapeEntry::getShape,
                                                   _1)) );
        }

        void LayerManager::addShape( const ShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_AND_THROW( rShape, "LayerManager::addShape(): invalid Shape" );

            // add shape to XShape hash map
            if( !maXShapeHash.insert(
                    XShapeHash::value_type( rShape->getXShape(),
                                            rShape) ).second )
            {
                // entry already present, nothing to do
                return;
            }

            // add shape to appropriate layer
            implAddShape( rShape );
        }

        void LayerManager::putShape2BackgroundLayer( const ShapeEntry& rShapeEntry )
        {
            LayerSharedPtr& rBgLayer( maLayers.front() );
            rBgLayer->setShapeViews(rShapeEntry.mpShape);
            // changing a part of the ShapeEntry irrelevant for the
            // set sort order
            const_cast<ShapeEntry&>(rShapeEntry).mpLayer = rBgLayer;
        }

        void LayerManager::implAddShape( const ShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_AND_THROW( rShape, "LayerManager::implAddShape(): invalid Shape" );

            ShapeEntry aShapeEntry(rShape);

            OSL_ASSERT( maAllShapes.find(aShapeEntry) == maAllShapes.end() ); // shape must not be added already
            mbLayerAssociationDirty = true;

            if( mbDisableAnimationZOrder )
                putShape2BackgroundLayer(
                    *maAllShapes.insert(aShapeEntry).first );
            else
                maAllShapes.insert(aShapeEntry);

            // update shape, it's just added and not yet painted
            if( rShape->isVisible() )
                notifyShapeUpdate( rShape );
        }

        bool LayerManager::removeShape( const ShapeSharedPtr& rShape )
        {
            // remove shape from XShape hash map
            if( maXShapeHash.erase( rShape->getXShape() ) == 0 )
                return false; // shape not in map

            OSL_ASSERT( maAllShapes.find(
                            ShapeEntry(rShape)) != maAllShapes.end() );

            implRemoveShape( rShape );

            return true;
        }

        void LayerManager::implRemoveShape( const ShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_AND_THROW( rShape, "LayerManager::implRemoveShape(): invalid Shape" );

            const LayerShapeSet::iterator aShapeEntry(
                maAllShapes.find(
                    ShapeEntry(rShape)) );

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
                LayerSharedPtr pLayer = aShapeEntry->mpLayer.lock();
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
            ENSURE_AND_THROW( xShape.is(), "LayerManager::lookupShape(): invalid Shape" );

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
                OSL_ASSERT( maAllShapes.find(
                                ShapeEntry(rSubsetShape)) != maAllShapes.end() );

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
            ENSURE_AND_THROW( rShape, "LayerManager::enterAnimationMode(): invalid Shape" );

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
            ENSURE_AND_THROW( !maLayers.empty(), "LayerManager::leaveAnimationMode(): no layers" );
            ENSURE_AND_THROW( rShape, "LayerManager::leaveAnimationMode(): invalid Shape" );

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

            if( rShape->isVisible() )
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

            const LayerVector::const_iterator aEnd( maLayers.end() );
            if( std::find_if( maLayers.begin(),
                              aEnd,
                              boost::mem_fn(&Layer::isUpdatePending)) != aEnd )
                return true;

            return false;
        }

        bool LayerManager::updateSprites()
        {
            bool bRet(true);

            // send update() calls to every shape in the
            // maUpdateShapes set, which is _animated_ (i.e. a
            // sprite).
            const ShapeUpdateSet::const_iterator aEnd=maUpdateShapes.end();
            ShapeUpdateSet::const_iterator       aCurrShape=maUpdateShapes.begin();
            while( aCurrShape != aEnd )
            {
                if( (*aCurrShape)->isBackgroundDetached() )
                {
                    // can update shape directly, without
                    // affecting layer content (shape is
                    // currently displayed in a sprite)
                    if( !(*aCurrShape)->update() )
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
                    addUpdateArea( *aCurrShape );
                }

                ++aCurrShape;
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
            if( std::find_if( maLayers.begin(),
                              maLayers.end(),
                              boost::mem_fn( &Layer::isUpdatePending )) == maLayers.end() )
                return bRet; // nope, done.

            // update each shape on each layer, that has
            // isUpdatePending()
            bool                                bIsCurrLayerUpdating(false);
            Layer::EndUpdater                   aEndUpdater;
            LayerSharedPtr                      pCurrLayer;
            LayerShapeSet::const_iterator       aIter( maAllShapes.begin() );
            const LayerShapeSet::const_iterator aEnd ( maAllShapes.end() );
            while( aIter != aEnd )
            {
                LayerSharedPtr pLayer = aIter->mpLayer.lock();
                if( pLayer != pCurrLayer )
                {
                    pCurrLayer = pLayer;
                    bIsCurrLayerUpdating = pCurrLayer->isUpdatePending();

                    if( bIsCurrLayerUpdating )
                        aEndUpdater = pCurrLayer->beginUpdate();
                }

                if( bIsCurrLayerUpdating &&
                    !aIter->mpShape->isBackgroundDetached() &&
                    pCurrLayer->isInsideUpdateArea(aIter->mpShape) )
                {
                    if( !aIter->mpShape->render() )
                        bRet = false;
                }

                ++aIter;
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

                virtual bool isOnView(boost::shared_ptr<View> const& /*rView*/) const
                {
                    return true; // visible on all views
                }

                virtual ::cppcanvas::CanvasSharedPtr getCanvas() const
                {
                    return mpCanvas;
                }

                virtual void clear() const
                {
                    // NOOP
                }

                virtual void clearAll() const
                {
                    // NOOP
                }

                virtual ::cppcanvas::CustomSpriteSharedPtr createSprite( const ::basegfx::B2DSize& /*rSpriteSizePixel*/,
                                                                         double                    /*nSpritePrio*/ ) const
                {
                    ENSURE_AND_THROW( false,
                                      "DummyLayer::createSprite(): This method is not supposed to be called!" );
                    return ::cppcanvas::CustomSpriteSharedPtr();
                }

                virtual void setPriority( const basegfx::B1DRange& /*rRange*/ )
                {
                    OSL_ENSURE( false,
                                "BitmapView::setPriority(): This method is not supposed to be called!" );
                }

                virtual ::basegfx::B2DHomMatrix getTransformation() const
                {
                    return mpCanvas->getTransformation();
                }

                virtual ::basegfx::B2DHomMatrix getSpriteTransformation() const
                {
                    OSL_ENSURE( false,
                                "BitmapView::getSpriteTransformation(): This method is not supposed to be called!" );
                    return ::basegfx::B2DHomMatrix();
                }

                virtual void setClip( const ::basegfx::B2DPolyPolygon& /*rClip*/ )
                {
                    OSL_ENSURE( false,
                                "BitmapView::setClip(): This method is not supposed to be called!" );
                }

                virtual bool resize( const ::basegfx::B2DRange& /*rArea*/ )
                {
                    OSL_ENSURE( false,
                                "BitmapView::resize(): This method is not supposed to be called!" );
                    return false;
                }

            private:
                ::cppcanvas::CanvasSharedPtr mpCanvas;
            };
        }

        bool LayerManager::renderTo( const ::cppcanvas::CanvasSharedPtr& rTargetCanvas ) const
        {
            bool bRet( true );
            ViewLayerSharedPtr pTmpLayer( new DummyLayer( rTargetCanvas ) );

            LayerShapeSet::const_iterator       aIter( maAllShapes.begin() );
            const LayerShapeSet::const_iterator aEnd ( maAllShapes.end() );
            while( aIter != aEnd )
            {
                try
                {
                    // forward to all shape's addViewLayer method (which
                    // we request to render the Shape on the new
                    // ViewLayer. Since we add the shapes in the
                    // maShapeSet order (which is also the render order),
                    // this is equivalent to a subsequent render() call)
                    aIter->mpShape->addViewLayer( pTmpLayer,
                                                  true );

                    // and remove again, this is only temporary
                    aIter->mpShape->removeViewLayer( pTmpLayer );
                }
                catch( uno::Exception& )
                {
                    // TODO(E1): Might be superfluous. Nowadays,
                    // addViewLayer swallows all errors, anyway.
                    OSL_ENSURE( false,
                                rtl::OUStringToOString(
                                    comphelper::anyToString( cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );

                    // at least one shape could not be rendered
                    bRet = false;
                }

                ++aIter;
            }

            return bRet;
        }

        void LayerManager::addUpdateArea( ShapeSharedPtr const& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); // always at least background layer
            ENSURE_AND_THROW( rShape, "LayerManager::addUpdateArea(): invalid Shape" );

            const LayerShapeSet::const_iterator aShapeEntry(
                maAllShapes.find(
                    ShapeEntry(rShape)) );

            if( aShapeEntry == maAllShapes.end() )
                return;

            LayerSharedPtr pLayer = aShapeEntry->mpLayer.lock();
            if( pLayer )
                pLayer->addUpdateRange( rShape->getUpdateArea() );
        }

        void LayerManager::commitLayerChanges( std::size_t              nCurrLayerIndex,
                                               LayerShapeSet::const_iterator  aFirstLayerShape,
                                               LayerShapeSet::const_iterator  aEndLayerShapes )
        {
            const bool bLayerExists( maLayers.size() > nCurrLayerIndex );
            if( bLayerExists )
            {
                const LayerSharedPtr& rLayer( maLayers.at(nCurrLayerIndex) );
                const bool bLayerResized( rLayer->commitBounds() );
                rLayer->setPriority( basegfx::B1DRange(nCurrLayerIndex,
                                                       nCurrLayerIndex+1) );

                if( bLayerResized )
                {
                    // need to re-render whole layer - start from
                    // clean state
                    rLayer->clearContent();

                    // render and remove from update set
                    while( aFirstLayerShape != aEndLayerShapes )
                    {
                        maUpdateShapes.erase(aFirstLayerShape->mpShape);
                        aFirstLayerShape->mpShape->render();
                        ++aFirstLayerShape;
                    }
                }
            }
        }

        LayerSharedPtr LayerManager::createForegroundLayer() const
        {
            OSL_ASSERT( mbActive );

            LayerSharedPtr pLayer( Layer::createLayer(
                                       maPageBounds ));

            // create ViewLayers for all registered views, and add to
            // newly created layer.
            ::std::for_each( mrViews.begin(),
                             mrViews.end(),
                             boost::bind( &Layer::addView,
                                          boost::cref(pLayer),
                                          _1 ));

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
            LayerShapeSet::iterator       aCurrShapeEntry( maAllShapes.begin() );
            LayerShapeSet::iterator       aCurrLayerFirstShapeEntry( maAllShapes.begin() );
            const LayerShapeSet::iterator aEndShapeEntry ( maAllShapes.end() );
            ShapeUpdateSet                aUpdatedShapes; // shapes that need update
            while( aCurrShapeEntry != aEndShapeEntry )
            {
                const ShapeSharedPtr pCurrShape( aCurrShapeEntry->mpShape );
                const bool bThisIsBackgroundDetached(
                    pCurrShape->isBackgroundDetached() );

                if( bLastWasBackgroundDetached == true &&
                    bThisIsBackgroundDetached == false )
                {
                    // discontinuity found - current shape needs to
                    // get into a new layer
                    // --------------------------------------------

                    // commit changes to previous layer
                    commitLayerChanges(nCurrLayerIndex,
                                       aCurrLayerFirstShapeEntry,
                                       aCurrShapeEntry);
                    aCurrLayerFirstShapeEntry=aCurrShapeEntry;
                    ++nCurrLayerIndex;
                    bIsBackgroundLayer = false;

                    if( aWeakLayers.size() <= nCurrLayerIndex ||
                        aWeakLayers.at(nCurrLayerIndex) != aCurrShapeEntry->mpLayer )
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
                if( rCurrWeakLayer != aCurrShapeEntry->mpLayer )
                {
                    // mismatch: shape is not contained in current
                    // layer - move shape to that layer, then.
                    maLayers.at(nCurrLayerIndex)->setShapeViews(
                        pCurrShape );

                    // layer got new shape(s), need full repaint, if
                    // non-sprite shape
                    if( !bThisIsBackgroundDetached && pCurrShape->isVisible() )
                    {
                        LayerSharedPtr pOldLayer( aCurrShapeEntry->mpLayer.lock() );
                        if( pOldLayer )
                        {
                            // old layer still valid? then we need to
                            // repaint former shape area
                            pOldLayer->addUpdateRange(
                                pCurrShape->getUpdateArea() );
                        }

                        // render on new layer (only if not
                        // explicitely disabled)
                        if( !(bBackgroundLayerPainted && bIsBackgroundLayer) )
                            maUpdateShapes.insert( pCurrShape );
                    }

                    // std::set iterators are const for a reason - but
                    // here, we need modify an aspect of the
                    // ShapeEntry that has no influence on sort order
                    const_cast<ShapeEntry&>(*aCurrShapeEntry).mpLayer = rCurrWeakLayer;
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
