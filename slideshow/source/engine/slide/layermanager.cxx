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
#include <basegfx/range/b1drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <boost/bind.hpp>
#include <algorithm>

#include <o3tl/compat_functional.hxx>

#include "layermanager.hxx"

using namespace ::com::sun::star;

namespace boost
{
    
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
            LayerShapeMap::const_iterator       aIter( maAllShapes.begin() );
            const LayerShapeMap::const_iterator aEnd ( maAllShapes.end() );
            while( aIter != aEnd )
            {
                LayerSharedPtr pLayer = aIter->second.lock();
                if( pLayer && pLayer != pCurrLayer )
                {
                    pCurrLayer = pLayer;
                    pCurrViewLayer = layerFunc(pCurrLayer);
                }

                if( pCurrViewLayer )
                    shapeFunc(aIter->first,pCurrViewLayer);

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
            
            
            maLayers.reserve(4);

            
            maLayers.push_back(
                    Layer::createBackgroundLayer(
                        maPageBounds ));

            
            std::for_each( mrViews.begin(),
                           mrViews.end(),
                           ::boost::bind(&LayerManager::viewAdded,
                                         this,
                                         _1) );
        }

        void LayerManager::activate( bool bSlideBackgoundPainted )
        {
            mbActive = true;
            maUpdateShapes.clear(); 
                                    

            if( !bSlideBackgoundPainted )
            {
                std::for_each(mrViews.begin(),
                              mrViews.end(),
                              boost::mem_fn(&View::clearAll));

                
                std::for_each( maLayers.begin(),
                               maLayers.end(),
                               boost::bind( &Layer::addUpdateRange,
                                            _1,
                                            boost::cref(maPageBounds) ));
            }
            else
            {
                
                
                std::for_each( maLayers.begin(),
                               maLayers.end(),
                               boost::mem_fn( &Layer::clearUpdateRanges ));
            }

            updateShapeLayers( bSlideBackgoundPainted );
        }

        void LayerManager::deactivate()
        {
            
            
            
            
            

            const bool bMoreThanOneLayer(maLayers.size() > 1);
            if( mnActiveSprites || bMoreThanOneLayer )
            {
                
                
                
                std::for_each(maAllShapes.begin(),
                              maAllShapes.end(),
                              boost::bind( &Shape::clearAllViewLayers,
                                           boost::bind( o3tl::select1st<LayerShapeMap::value_type>(),
                                                        _1 )));

                for (LayerShapeMap::iterator
                         iShape (maAllShapes.begin()),
                         iEnd (maAllShapes.end());
                     iShape!=iEnd;
                     ++iShape)
                {
                    iShape->second.reset();
                }

                if( bMoreThanOneLayer )
                    maLayers.erase(maLayers.begin()+1,
                                   maLayers.end());

                mbLayerAssociationDirty = true;
            }

            mbActive = false;

            
            OSL_ASSERT( maLayers.size() == 1 && maLayers.front()->isBackgroundLayer() );
        }

        void LayerManager::viewAdded( const UnoViewSharedPtr& rView )
        {
            
            OSL_ASSERT( std::find(mrViews.begin(),
                                  mrViews.end(),
                                  rView) != mrViews.end() );

            
            if( mbActive )
                rView->clearAll();

            
            manageViews(
                boost::bind(&Layer::addView,
                            _1,
                            boost::cref(rView)),
                
                boost::bind(&Shape::addViewLayer,
                            _1,
                            _2,
                            true) );

            
            
            std::for_each( maLayers.begin(),
                           maLayers.end(),
                           boost::bind( &Layer::addView,
                                        _1,
                                        boost::cref(rView) ));
        }

        void LayerManager::viewRemoved( const UnoViewSharedPtr& rView )
        {
            
            OSL_ASSERT( std::find(mrViews.begin(),
                                  mrViews.end(),
                                  rView) == mrViews.end() );

            
            manageViews(
                boost::bind(&Layer::removeView,
                            _1,
                            boost::cref(rView)),
                boost::bind(&Shape::removeViewLayer,
                            _1,
                            _2) );

            
            
            std::for_each( maLayers.begin(),
                           maLayers.end(),
                           boost::bind( &Layer::removeView,
                                        _1,
                                        boost::cref(rView) ));
        }

        void LayerManager::viewChanged( const UnoViewSharedPtr& rView )
        {
            (void)rView;

            
            OSL_ASSERT( std::find(mrViews.begin(),
                                  mrViews.end(),
                                  rView) != mrViews.end() );

            
            viewsChanged();
        }

        void LayerManager::viewsChanged()
        {
            if( !mbActive )
                return;

            
            ::std::for_each( mrViews.begin(),
                             mrViews.end(),
                             ::boost::mem_fn(&View::clearAll) );

            

            
            std::for_each( maAllShapes.begin(),
                           maAllShapes.end(),
                           boost::bind(&Shape::render,
                               boost::bind( ::o3tl::select1st<LayerShapeMap::value_type>(), _1)) );
        }

        void LayerManager::addShape( const ShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); 
            ENSURE_OR_THROW( rShape, "LayerManager::addShape(): invalid Shape" );

            
            if( !maXShapeHash.insert(
                    XShapeHash::value_type( rShape->getXShape(),
                                            rShape) ).second )
            {
                
                return;
            }

            
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
            OSL_ASSERT( !maLayers.empty() ); 
            ENSURE_OR_THROW( rShape, "LayerManager::implAddShape(): invalid Shape" );

            LayerShapeMap::value_type aValue (rShape, LayerWeakPtr());

            OSL_ASSERT( maAllShapes.find(rShape) == maAllShapes.end() ); 
            mbLayerAssociationDirty = true;

            if( mbDisableAnimationZOrder )
                putShape2BackgroundLayer(
                    *maAllShapes.insert(aValue).first );
            else
                maAllShapes.insert(aValue);

            
            if( rShape->isVisible() )
                notifyShapeUpdate( rShape );
        }

        void LayerManager::implRemoveShape( const ShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); 
            ENSURE_OR_THROW( rShape, "LayerManager::implRemoveShape(): invalid Shape" );

            const LayerShapeMap::iterator aShapeEntry( maAllShapes.find(rShape) );

            if( aShapeEntry == maAllShapes.end() )
                return;

            const bool bShapeUpdateNotified = maUpdateShapes.erase( rShape ) != 0;

            
            
            
            
            
            if( bShapeUpdateNotified ||
                (rShape->isVisible() &&
                 !rShape->isBackgroundDetached()) )
            {
                LayerSharedPtr pLayer = aShapeEntry->second.lock();
                if( pLayer )
                {
                    
                    
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
                return ShapeSharedPtr(); 

            
            return aIter->second;
        }

        AttributableShapeSharedPtr LayerManager::getSubsetShape( const AttributableShapeSharedPtr&  rOrigShape,
                                                                 const DocTreeNode&                 rTreeNode )
        {
            OSL_ASSERT( !maLayers.empty() ); 

            AttributableShapeSharedPtr pSubset;

            
            if( rOrigShape->createSubset( pSubset,
                                          rTreeNode ) )
            {
                OSL_ENSURE( pSubset, "LayerManager::getSubsetShape(): failed to create subset" );

                
                
                

                
                implAddShape( pSubset );

                
                
                
                
                if( rOrigShape->isVisible() )
                    notifyShapeUpdate( rOrigShape );
            }

            return pSubset;
        }

        void LayerManager::revokeSubset( const AttributableShapeSharedPtr& rOrigShape,
                                         const AttributableShapeSharedPtr& rSubsetShape )
        {
            OSL_ASSERT( !maLayers.empty() ); 

            if( rOrigShape->revokeSubset( rSubsetShape ) )
            {
                OSL_ASSERT( maAllShapes.find(rSubsetShape) != maAllShapes.end() );

                implRemoveShape( rSubsetShape );

                
                
                if( rOrigShape->isVisible() )
                    notifyShapeUpdate( rOrigShape );
            }
        }

        void LayerManager::enterAnimationMode( const AnimatableShapeSharedPtr& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); 
            ENSURE_OR_THROW( rShape, "LayerManager::enterAnimationMode(): invalid Shape" );

            const bool bPrevAnimState( rShape->isBackgroundDetached() );

            rShape->enterAnimationMode();

            
            
            
            
            if( bPrevAnimState != rShape->isBackgroundDetached() )
            {
                ++mnActiveSprites;
                mbLayerAssociationDirty = true;

                
                
                
                if( rShape->isVisible() )
                    addUpdateArea( rShape );
            }

            
            
            
        }

        void LayerManager::leaveAnimationMode( const AnimatableShapeSharedPtr& rShape )
        {
            ENSURE_OR_THROW( !maLayers.empty(), "LayerManager::leaveAnimationMode(): no layers" );
            ENSURE_OR_THROW( rShape, "LayerManager::leaveAnimationMode(): invalid Shape" );

            const bool bPrevAnimState( rShape->isBackgroundDetached() );

            rShape->leaveAnimationMode();

            
            
            
            
            if( bPrevAnimState != rShape->isBackgroundDetached() )
            {
                --mnActiveSprites;
                mbLayerAssociationDirty = true;

                
                
                if( rShape->isVisible() )
                    notifyShapeUpdate( rShape );
            }

            
            
            
        }

        void LayerManager::notifyShapeUpdate( const ShapeSharedPtr& rShape )
        {
            if( !mbActive || mrViews.empty() )
                return;

            
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

            
            
            
            const ShapeUpdateSet::const_iterator aEnd=maUpdateShapes.end();
            ShapeUpdateSet::const_iterator       aCurrShape=maUpdateShapes.begin();
            while( aCurrShape != aEnd )
            {
                if( (*aCurrShape)->isBackgroundDetached() )
                {
                    
                    
                    
                    if( !(*aCurrShape)->update() )
                        bRet = false; 
                }
                else
                {
                    
                    
                    
                    

                    
                    
                    
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

            
            
            updateShapeLayers(false);

            
            bRet = updateSprites();

            
            if( std::find_if( maLayers.begin(),
                              maLayers.end(),
                              boost::mem_fn( &Layer::isUpdatePending )) == maLayers.end() )
                return bRet; 

            
            
            bool                                bIsCurrLayerUpdating(false);
            Layer::EndUpdater                   aEndUpdater;
            LayerSharedPtr                      pCurrLayer;
            LayerShapeMap::const_iterator       aIter( maAllShapes.begin() );
            const LayerShapeMap::const_iterator aEnd ( maAllShapes.end() );
            while( aIter != aEnd )
            {
                LayerSharedPtr pLayer = aIter->second.lock();
                if( pLayer != pCurrLayer )
                {
                    pCurrLayer = pLayer;
                    bIsCurrLayerUpdating = pCurrLayer->isUpdatePending();

                    if( bIsCurrLayerUpdating )
                        aEndUpdater = pCurrLayer->beginUpdate();
                }

                if( bIsCurrLayerUpdating &&
                    !aIter->first->isBackgroundDetached() &&
                    pCurrLayer->isInsideUpdateArea(aIter->first) )
                {
                    if( !aIter->first->render() )
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
                    return true; 
                }

                virtual ::cppcanvas::CanvasSharedPtr getCanvas() const
                {
                    return mpCanvas;
                }

                virtual void clear() const
                {
                    
                }

                virtual void clearAll() const
                {
                    
                }

                virtual ::cppcanvas::CustomSpriteSharedPtr createSprite( const ::basegfx::B2DSize& /*rSpriteSizePixel*/,
                                                                         double                    /*nSpritePrio*/ ) const
                {
                    ENSURE_OR_THROW( false,
                                      "DummyLayer::createSprite(): This method is not supposed to be called!" );
                    return ::cppcanvas::CustomSpriteSharedPtr();
                }

                virtual void setPriority( const basegfx::B1DRange& /*rRange*/ )
                {
                    OSL_FAIL( "BitmapView::setPriority(): This method is not supposed to be called!" );
                }

                virtual ::com::sun::star::geometry::IntegerSize2D getTranslationOffset() const
                {
                    return geometry::IntegerSize2D(0,0);
                }

                virtual ::basegfx::B2DHomMatrix getTransformation() const
                {
                    return mpCanvas->getTransformation();
                }

                virtual ::basegfx::B2DHomMatrix getSpriteTransformation() const
                {
                    OSL_FAIL( "BitmapView::getSpriteTransformation(): This method is not supposed to be called!" );
                    return ::basegfx::B2DHomMatrix();
                }

                virtual void setClip( const ::basegfx::B2DPolyPolygon& /*rClip*/ )
                {
                    OSL_FAIL( "BitmapView::setClip(): This method is not supposed to be called!" );
                }

                virtual bool resize( const ::basegfx::B2DRange& /*rArea*/ )
                {
                    OSL_FAIL( "BitmapView::resize(): This method is not supposed to be called!" );
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

            LayerShapeMap::const_iterator       aIter( maAllShapes.begin() );
            const LayerShapeMap::const_iterator aEnd ( maAllShapes.end() );
            while( aIter != aEnd )
            {
                try
                {
                    
                    
                    
                    
                    
                    aIter->first->addViewLayer( pTmpLayer,
                                                true );

                    
                    aIter->first->removeViewLayer( pTmpLayer );
                }
                catch( uno::Exception& )
                {
                    
                    
                    OSL_FAIL( OUStringToOString(
                                    comphelper::anyToString( cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );

                    
                    bRet = false;
                }

                ++aIter;
            }

            return bRet;
        }

        void LayerManager::addUpdateArea( ShapeSharedPtr const& rShape )
        {
            OSL_ASSERT( !maLayers.empty() ); 
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
                                               LayerShapeMap::const_iterator  aEndLayerShapes )
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
                    
                    
                    rLayer->clearContent();

                    
                    while( aFirstLayerShape != aEndLayerShapes )
                    {
                        maUpdateShapes.erase(aFirstLayerShape->first);
                        aFirstLayerShape->first->render();
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

            
            
            ::std::for_each( mrViews.begin(),
                             mrViews.end(),
                             boost::bind( &Layer::addView,
                                          boost::cref(pLayer),
                                          _1 ));

            return pLayer;
        }

        void LayerManager::updateShapeLayers( bool bBackgroundLayerPainted )
        {
            OSL_ASSERT( !maLayers.empty() ); 
            OSL_ASSERT( mbActive );

            
            if( !mbLayerAssociationDirty )
                return;

            if( mbDisableAnimationZOrder )
            {
                
                
                mbLayerAssociationDirty = false;
                return;
            }

            
            
            
            
            

            
            
            std::vector< LayerWeakPtr > aWeakLayers(maLayers.size());
            std::copy(maLayers.begin(),maLayers.end(),aWeakLayers.begin());

            std::size_t                   nCurrLayerIndex(0);
            bool                          bIsBackgroundLayer(true);
            bool                          bLastWasBackgroundDetached(false); 
            LayerShapeMap::iterator       aCurrShapeEntry( maAllShapes.begin() );
            LayerShapeMap::iterator       aCurrLayerFirstShapeEntry( maAllShapes.begin() );
            const LayerShapeMap::iterator aEndShapeEntry ( maAllShapes.end() );
            ShapeUpdateSet                aUpdatedShapes; 
            while( aCurrShapeEntry != aEndShapeEntry )
            {
                const ShapeSharedPtr pCurrShape( aCurrShapeEntry->first );
                const bool bThisIsBackgroundDetached(
                    pCurrShape->isBackgroundDetached() );

                if( bLastWasBackgroundDetached == true &&
                    bThisIsBackgroundDetached == false )
                {
                    
                    
                    

                    
                    commitLayerChanges(nCurrLayerIndex,
                                       aCurrLayerFirstShapeEntry,
                                       aCurrShapeEntry);
                    aCurrLayerFirstShapeEntry=aCurrShapeEntry;
                    ++nCurrLayerIndex;
                    bIsBackgroundLayer = false;

                    if( aWeakLayers.size() <= nCurrLayerIndex ||
                        aWeakLayers.at(nCurrLayerIndex) != aCurrShapeEntry->second )
                    {
                        
                        
                        maLayers.insert( maLayers.begin()+nCurrLayerIndex,
                                         createForegroundLayer() );
                        aWeakLayers.insert( aWeakLayers.begin()+nCurrLayerIndex,
                                            maLayers[nCurrLayerIndex] );
                    }
                }

                OSL_ASSERT( maLayers.size() == aWeakLayers.size() );

                
                
                LayerSharedPtr& rCurrLayer( maLayers.at(nCurrLayerIndex) );
                LayerWeakPtr& rCurrWeakLayer( aWeakLayers.at(nCurrLayerIndex) );
                if( rCurrWeakLayer != aCurrShapeEntry->second )
                {
                    
                    
                    maLayers.at(nCurrLayerIndex)->setShapeViews(
                        pCurrShape );

                    
                    
                    if( !bThisIsBackgroundDetached && pCurrShape->isVisible() )
                    {
                        LayerSharedPtr pOldLayer( aCurrShapeEntry->second.lock() );
                        if( pOldLayer )
                        {
                            
                            
                            pOldLayer->addUpdateRange(
                                pCurrShape->getUpdateArea() );
                        }

                        
                        
                        if( !(bBackgroundLayerPainted && bIsBackgroundLayer) )
                            maUpdateShapes.insert( pCurrShape );
                    }

                    aCurrShapeEntry->second = rCurrWeakLayer;
                }

                
                
                
                
                
                
                if( !bThisIsBackgroundDetached && !bIsBackgroundLayer )
                    rCurrLayer->updateBounds( pCurrShape );

                bLastWasBackgroundDetached = bThisIsBackgroundDetached;
                ++aCurrShapeEntry;
            }

            
            commitLayerChanges(nCurrLayerIndex,
                               aCurrLayerFirstShapeEntry,
                               aCurrShapeEntry);

            
            if( maLayers.size() > nCurrLayerIndex+1 )
                maLayers.erase(maLayers.begin()+nCurrLayerIndex+1,
                               maLayers.end());

            mbLayerAssociationDirty = false;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
