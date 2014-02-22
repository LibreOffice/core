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
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/presentation/XShapeEventListener.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>

#include "shapemanagerimpl.hxx"

#include <boost/bind.hpp>

#include <o3tl/compat_functional.hxx>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

ShapeManagerImpl::ShapeManagerImpl( EventMultiplexer&            rMultiplexer,
                                    LayerManagerSharedPtr const& rLayerManager,
                                    CursorManager&               rCursorManager,
                                    const ShapeEventListenerMap& rGlobalListenersMap,
                                    const ShapeCursorMap&        rGlobalCursorMap ):
    mrMultiplexer(rMultiplexer),
    mpLayerManager(rLayerManager),
    mrCursorManager(rCursorManager),
    mrGlobalListenersMap(rGlobalListenersMap),
    mrGlobalCursorMap(rGlobalCursorMap),
    maShapeListenerMap(),
    maShapeCursorMap(),
    maHyperlinkShapes(),
    mbEnabled(false)
{
}

void ShapeManagerImpl::activate( bool bSlideBackgoundPainted )
{
    if( !mbEnabled )
    {
        mbEnabled = true;

        
        
        mrMultiplexer.addMouseMoveHandler( shared_from_this(), 2.0 );
        mrMultiplexer.addClickHandler( shared_from_this(), 2.0 );
        mrMultiplexer.addShapeListenerHandler( shared_from_this() );

        
        uno::Reference<presentation::XShapeEventListener> xDummyListener;
        std::for_each( mrGlobalListenersMap.begin(),
                       mrGlobalListenersMap.end(),
                       boost::bind( &ShapeManagerImpl::listenerAdded,
                                    this,
                                    boost::cref(xDummyListener),
                                    boost::bind(
                                        o3tl::select1st<ShapeEventListenerMap::value_type>(),
                                        _1 )));

        
        std::for_each( mrGlobalCursorMap.begin(),
                       mrGlobalCursorMap.end(),
                       boost::bind( &ShapeManagerImpl::cursorChanged,
                                    this,
                                    boost::bind(
                                        o3tl::select1st<ShapeCursorMap::value_type>(),
                                        _1 ),
                                    boost::bind(
                                        o3tl::select2nd<ShapeCursorMap::value_type>(),
                                        _1 )));

        if( mpLayerManager )
            mpLayerManager->activate( bSlideBackgoundPainted );
    }
}

void ShapeManagerImpl::deactivate()
{
    if( mbEnabled )
    {
        mbEnabled = false;

        if( mpLayerManager )
            mpLayerManager->deactivate();

        maShapeListenerMap.clear();
        maShapeCursorMap.clear();

        mrMultiplexer.removeShapeListenerHandler( shared_from_this() );
        mrMultiplexer.removeMouseMoveHandler( shared_from_this() );
        mrMultiplexer.removeClickHandler( shared_from_this() );
    }
}

void ShapeManagerImpl::dispose()
{
    
    deactivate();

    maHyperlinkShapes.clear();
    maShapeCursorMap.clear();
    maShapeListenerMap.clear();
    mpLayerManager.reset();
}

bool ShapeManagerImpl::handleMousePressed( awt::MouseEvent const& )
{
    
    return false; 
}

bool ShapeManagerImpl::handleMouseReleased( awt::MouseEvent const& e )
{
    if( !mbEnabled || e.Buttons != awt::MouseButton::LEFT)
        return false;

    basegfx::B2DPoint const aPosition( e.X, e.Y );

    
    
    OUString const hyperlink( checkForHyperlink(aPosition) );
    if( !hyperlink.isEmpty() )
    {
        mrMultiplexer.notifyHyperlinkClicked(hyperlink);
        return true; 
    }

    
    
    ShapeToListenersMap::reverse_iterator aCurrBroadcaster(
        maShapeListenerMap.rbegin() );
    ShapeToListenersMap::reverse_iterator const aEndBroadcasters(
        maShapeListenerMap.rend() );
    while( aCurrBroadcaster != aEndBroadcasters )
    {
        
        
        
        if( aCurrBroadcaster->first->getBounds().isInside( aPosition ) &&
            aCurrBroadcaster->first->isVisible() )
        {
            
            

            boost::shared_ptr<cppu::OInterfaceContainerHelper> const pCont(
                aCurrBroadcaster->second );
            uno::Reference<drawing::XShape> const xShape(
                aCurrBroadcaster->first->getXShape() );

            
            pCont->forEach<presentation::XShapeEventListener>(
                boost::bind( &presentation::XShapeEventListener::click,
                             _1,
                             boost::cref(xShape),
                             boost::cref(e) ));

            return true; 
        }

        ++aCurrBroadcaster;
    }

    return false; 
}

bool ShapeManagerImpl::handleMouseEntered( const awt::MouseEvent& )
{
    
    return false; 
}

bool ShapeManagerImpl::handleMouseExited( const awt::MouseEvent& )
{
    
    return false; 
}

bool ShapeManagerImpl::handleMouseDragged( const awt::MouseEvent& )
{
    
    return false; 
}

bool ShapeManagerImpl::handleMouseMoved( const awt::MouseEvent& e )
{
    if( !mbEnabled )
        return false;

    
    const ::basegfx::B2DPoint aPosition( e.X, e.Y );
    sal_Int16                 nNewCursor(-1);

    if( !checkForHyperlink(aPosition).isEmpty() )
    {
        nNewCursor = awt::SystemPointer::REFHAND;
    }
    else
    {
        
        
        ShapeToCursorMap::reverse_iterator aCurrCursor(
            maShapeCursorMap.rbegin() );
        ShapeToCursorMap::reverse_iterator const aEndCursors(
            maShapeCursorMap.rend() );
        while( aCurrCursor != aEndCursors )
        {
            
            
            
            if( aCurrCursor->first->getBounds().isInside( aPosition ) &&
                aCurrCursor->first->isVisible() )
            {
                
                
                nNewCursor = aCurrCursor->second;
                break;
            }

            ++aCurrCursor;
        }
    }

    if( nNewCursor == -1 )
        mrCursorManager.resetCursor();
    else
        mrCursorManager.requestCursor( nNewCursor );

    return false; 
                  
}

bool ShapeManagerImpl::update()
{
    if( mbEnabled && mpLayerManager )
        return mpLayerManager->update();

    return false;
}

bool ShapeManagerImpl::update( ViewSharedPtr const& /*rView*/ )
{
    
    return false;
}

bool ShapeManagerImpl::needsUpdate() const
{
    if( mbEnabled && mpLayerManager )
        return mpLayerManager->isUpdatePending();

    return false;
}

void ShapeManagerImpl::enterAnimationMode( const AnimatableShapeSharedPtr& rShape )
{
    if( mbEnabled && mpLayerManager )
        mpLayerManager->enterAnimationMode(rShape);
}

void ShapeManagerImpl::leaveAnimationMode( const AnimatableShapeSharedPtr& rShape )
{
    if( mbEnabled && mpLayerManager )
        mpLayerManager->leaveAnimationMode(rShape);
}

void ShapeManagerImpl::notifyShapeUpdate( const ShapeSharedPtr& rShape )
{
    if( mbEnabled && mpLayerManager )
        mpLayerManager->notifyShapeUpdate(rShape);
}

ShapeSharedPtr ShapeManagerImpl::lookupShape( uno::Reference< drawing::XShape > const & xShape ) const
{
    if( mpLayerManager )
        return mpLayerManager->lookupShape(xShape);

    return ShapeSharedPtr();
}

void ShapeManagerImpl::addHyperlinkArea( const HyperlinkAreaSharedPtr& rArea )
{
    maHyperlinkShapes.insert(rArea);
}

void ShapeManagerImpl::removeHyperlinkArea( const HyperlinkAreaSharedPtr& rArea )
{
    maHyperlinkShapes.erase(rArea);
}

AttributableShapeSharedPtr ShapeManagerImpl::getSubsetShape( const AttributableShapeSharedPtr& rOrigShape,
                                                             const DocTreeNode&                rTreeNode )
{
    if( mpLayerManager )
        return mpLayerManager->getSubsetShape(rOrigShape,rTreeNode);

    return AttributableShapeSharedPtr();
}

void ShapeManagerImpl::revokeSubset( const AttributableShapeSharedPtr& rOrigShape,
                                     const AttributableShapeSharedPtr& rSubsetShape )
{
    if( mpLayerManager )
        mpLayerManager->revokeSubset(rOrigShape,rSubsetShape);
}

bool ShapeManagerImpl::listenerAdded(
    const uno::Reference<presentation::XShapeEventListener>& /*xListener*/,
    const uno::Reference<drawing::XShape>&                   xShape )
{
    ShapeEventListenerMap::const_iterator aIter;
    if( (aIter = mrGlobalListenersMap.find( xShape )) ==
        mrGlobalListenersMap.end() )
    {
        ENSURE_OR_RETURN_FALSE(false,
                          "ShapeManagerImpl::listenerAdded(): global "
                          "shape listener map inconsistency!");
    }

    
    ShapeSharedPtr pShape( lookupShape(xShape) );
    if( pShape )
    {
        maShapeListenerMap.insert(
            ShapeToListenersMap::value_type(
                pShape,
                aIter->second));
    }

    return true;
}

bool ShapeManagerImpl::listenerRemoved(
    const uno::Reference<presentation::XShapeEventListener>& /*xListener*/,
    const uno::Reference<drawing::XShape>&                   xShape )
{
    
    
    if( mrGlobalListenersMap.find(xShape) == mrGlobalListenersMap.end() )
    {
        
        ShapeSharedPtr pShape( lookupShape(xShape) );
        if( pShape )
            maShapeListenerMap.erase(pShape);
    }

    return true;
}

bool ShapeManagerImpl::cursorChanged( const uno::Reference<drawing::XShape>&   xShape,
                                      sal_Int16                                nCursor )
{
    ShapeSharedPtr pShape( lookupShape(xShape) );

    
    if( !pShape )
        return false;

    if( mrGlobalCursorMap.find(xShape) == mrGlobalCursorMap.end() )
    {
        
        maShapeCursorMap.erase(pShape);
    }
    else
    {
        
        ShapeToCursorMap::iterator aIter;
        if( (aIter = maShapeCursorMap.find(pShape))
            == maShapeCursorMap.end() )
        {
            maShapeCursorMap.insert(
                ShapeToCursorMap::value_type(
                    pShape,
                    nCursor ));
        }
        else
        {
            aIter->second = nCursor;
        }
    }

    return true;
}

OUString ShapeManagerImpl::checkForHyperlink( basegfx::B2DPoint const& hitPos ) const
{
    
    
    AreaSet::const_reverse_iterator iPos( maHyperlinkShapes.rbegin() );
    AreaSet::const_reverse_iterator const iEnd( maHyperlinkShapes.rend() );
    for( ; iPos != iEnd; ++iPos )
    {
        HyperlinkAreaSharedPtr const& pArea = *iPos;

        HyperlinkArea::HyperlinkRegions const linkRegions(
            pArea->getHyperlinkRegions() );

        for( std::size_t i = linkRegions.size(); i--; )
        {
            basegfx::B2DRange const& region = linkRegions[i].first;
            if( region.isInside(hitPos) )
                return linkRegions[i].second;
        }
    }

    return OUString();
}

void ShapeManagerImpl::addIntrinsicAnimationHandler( const IntrinsicAnimationEventHandlerSharedPtr& rHandler )
{
    maIntrinsicAnimationEventHandlers.add( rHandler );
}

void ShapeManagerImpl::removeIntrinsicAnimationHandler( const IntrinsicAnimationEventHandlerSharedPtr& rHandler )
{
    maIntrinsicAnimationEventHandlers.remove( rHandler );
}

bool ShapeManagerImpl::notifyIntrinsicAnimationsEnabled()
{
    return maIntrinsicAnimationEventHandlers.applyAll(
        boost::mem_fn(&IntrinsicAnimationEventHandler::enableAnimations));
}

bool ShapeManagerImpl::notifyIntrinsicAnimationsDisabled()
{
    return maIntrinsicAnimationEventHandlers.applyAll(
        boost::mem_fn(&IntrinsicAnimationEventHandler::disableAnimations));
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
