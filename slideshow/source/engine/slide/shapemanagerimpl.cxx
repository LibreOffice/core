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

ShapeManagerImpl::ShapeManagerImpl( const UnoViewContainer&      rViews,
                                    EventMultiplexer&            rMultiplexer,
                                    CursorManager&               rCursorManager,
                                    const ShapeEventListenerMap& rGlobalListenersMap,
                                    const ShapeCursorMap&        rGlobalCursorMap ):
    maXShapeHash( 101 ),
    maAllShapes(),
    maUpdateShapes(),
    mrViews(rViews),
    mrMultiplexer(rMultiplexer),
    mrCursorManager(rCursorManager),
    mrGlobalListenersMap(rGlobalListenersMap),
    mrGlobalCursorMap(rGlobalCursorMap),
    maShapeListenerMap(),
    maShapeCursorMap(),
    maHyperlinkShapes(),
    mbEnabled(false)
{
}

template<typename ShapeFunc> void ShapeManagerImpl::manageViews(
             ShapeFunc shapeFunc )
{
    ViewLayerSharedPtr                  pCurrViewLayer;
    LayerShapeMap::const_iterator       aIter( maAllShapes.begin() );
    const LayerShapeMap::const_iterator aEnd ( maAllShapes.end() );
    while( aIter != aEnd )
    {
        if( pCurrViewLayer )
            shapeFunc(aIter->first,pCurrViewLayer);

        ++aIter;
    }
}

void ShapeManagerImpl::activate( bool bSlideBackgoundPainted )
{
    if( !mbEnabled )
    {
        mbEnabled = true;

        // register this handler on EventMultiplexer.
        // Higher prio (overrides other engine handlers)
        mrMultiplexer.addMouseMoveHandler( shared_from_this(), 2.0 );
        mrMultiplexer.addClickHandler( shared_from_this(), 2.0 );
        mrMultiplexer.addShapeListenerHandler( shared_from_this() );

        // clone listener map
        uno::Reference<presentation::XShapeEventListener> xDummyListener;
        std::for_each( mrGlobalListenersMap.begin(),
                       mrGlobalListenersMap.end(),
                       boost::bind( &ShapeManagerImpl::listenerAdded,
                                    this,
                                    boost::cref(xDummyListener),
                                    boost::bind(
                                        o3tl::select1st<ShapeEventListenerMap::value_type>(),
                                        _1 )));

        // clone cursor map
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

      /*  if( mpLayerManager )
            mpLayerManager->activate( bSlideBackgoundPainted );*/
    }
}

void ShapeManagerImpl::deactivate()
{
    if( mbEnabled )
    {
        mbEnabled = false;

        maShapeListenerMap.clear();
        maShapeCursorMap.clear();

        mrMultiplexer.removeShapeListenerHandler( shared_from_this() );
        mrMultiplexer.removeMouseMoveHandler( shared_from_this() );
        mrMultiplexer.removeClickHandler( shared_from_this() );
    }
}

void ShapeManagerImpl::dispose()
{
    // remove listeners (EventMultiplexer holds shared_ptr on us)
    deactivate();

    maHyperlinkShapes.clear();
    maShapeCursorMap.clear();
    maShapeListenerMap.clear();
}

bool ShapeManagerImpl::handleMousePressed( awt::MouseEvent const& )
{
    // not used here
    return false; // did not handle the event
}

bool ShapeManagerImpl::handleMouseReleased( awt::MouseEvent const& e )
{
    if( !mbEnabled || e.Buttons != awt::MouseButton::LEFT)
        return false;

    basegfx::B2DPoint const aPosition( e.X, e.Y );

    // first check for hyperlinks, because these have
    // highest prio:
    OUString const hyperlink( checkForHyperlink(aPosition) );
    if( !hyperlink.isEmpty() )
    {
        mrMultiplexer.notifyHyperlinkClicked(hyperlink);
        return true; // event consumed
    }

    // find matching shape (scan reversely, to coarsely match
    // paint order)
    ShapeToListenersMap::reverse_iterator aCurrBroadcaster(
        maShapeListenerMap.rbegin() );
    ShapeToListenersMap::reverse_iterator const aEndBroadcasters(
        maShapeListenerMap.rend() );
    while( aCurrBroadcaster != aEndBroadcasters )
    {
        // TODO(F2): Get proper geometry polygon from the
        // shape, to avoid having areas outside the shape
        // react on the mouse
        if( aCurrBroadcaster->first->getBounds().isInside( aPosition ) &&
            aCurrBroadcaster->first->isVisible() )
        {
            // shape hit, and shape is visible. Raise
            // event.

            boost::shared_ptr<cppu::OInterfaceContainerHelper> const pCont(
                aCurrBroadcaster->second );
            uno::Reference<drawing::XShape> const xShape(
                aCurrBroadcaster->first->getXShape() );

            // DON'T do anything with /this/ after this point!
            pCont->forEach<presentation::XShapeEventListener>(
                boost::bind( &presentation::XShapeEventListener::click,
                             _1,
                             boost::cref(xShape),
                             boost::cref(e) ));

            return true; // handled this event
        }

        ++aCurrBroadcaster;
    }

    return false; // did not handle this event
}

bool ShapeManagerImpl::handleMouseEntered( const awt::MouseEvent& )
{
    // not used here
    return false; // did not handle the event
}

bool ShapeManagerImpl::handleMouseExited( const awt::MouseEvent& )
{
    // not used here
    return false; // did not handle the event
}

bool ShapeManagerImpl::handleMouseDragged( const awt::MouseEvent& )
{
    // not used here
    return false; // did not handle the event
}

bool ShapeManagerImpl::handleMouseMoved( const awt::MouseEvent& e )
{
    if( !mbEnabled )
        return false;

    // find hit shape in map
    const ::basegfx::B2DPoint aPosition( e.X, e.Y );
    sal_Int16                 nNewCursor(-1);

    if( !checkForHyperlink(aPosition).isEmpty() )
    {
        nNewCursor = awt::SystemPointer::REFHAND;
    }
    else
    {
        // find matching shape (scan reversely, to coarsely match
        // paint order)
        ShapeToCursorMap::reverse_iterator aCurrCursor(
            maShapeCursorMap.rbegin() );
        ShapeToCursorMap::reverse_iterator const aEndCursors(
            maShapeCursorMap.rend() );
        while( aCurrCursor != aEndCursors )
        {
            // TODO(F2): Get proper geometry polygon from the
            // shape, to avoid having areas outside the shape
            // react on the mouse
            if( aCurrCursor->first->getBounds().isInside( aPosition ) &&
                aCurrCursor->first->isVisible() )
            {
                // shape found, and it's visible. set
                // requested cursor to shape's
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

    return false; // we don't /eat/ this event. Lower prio
                  // handler should see it, too.
}

void ShapeManagerImpl::viewAdded( const UnoViewSharedPtr& rView )
{
    // view must be member of mrViews container
    OSL_ASSERT( std::find(mrViews.begin(),
                          mrViews.end(),
                          rView) != mrViews.end() );

    // init view content
    rView->clearAll();

    // add View to all registered shapes
    manageViews(boost::bind(&Shape::addViewLayer,
                    _1,
                    _2,
                    true) );
}

void ShapeManagerImpl::viewRemoved( const UnoViewSharedPtr& rView )
{
    // view must not be member of mrViews container anymore
    OSL_ASSERT( std::find(mrViews.begin(),
                          mrViews.end(),
                          rView) == mrViews.end() );

    // remove View from all registered shapes
    manageViews(boost::bind(&Shape::removeViewLayer,
                    _1,
                    _2) );
}

void ShapeManagerImpl::viewChanged( const UnoViewSharedPtr& rView )
{
    (void)rView;

    // view must be member of mrViews container
    OSL_ASSERT( std::find(mrViews.begin(),
                          mrViews.end(),
                          rView) != mrViews.end() );

    // TODO(P2): selectively update only changed view
    viewsChanged();
}

void ShapeManagerImpl::viewsChanged()
{

    // clear view area
    ::std::for_each( mrViews.begin(),
                     mrViews.end(),
                     ::boost::mem_fn(&View::clearAll) );

    // render all shapes
    std::for_each( maAllShapes.begin(),
                   maAllShapes.end(),
                   boost::bind(&Shape::render,
                       boost::bind( ::o3tl::select1st<LayerShapeMap::value_type>(), _1)) );
}

bool ShapeManagerImpl::update()
{
    updateSprites();
    return false;
}

bool ShapeManagerImpl::update( ViewSharedPtr const& /*rView*/ )
{
    // am not doing view-specific updates here.
    return false;
}

bool ShapeManagerImpl::needsUpdate() const
{
    //if( mbEnabled )
       // return mpLayerManager->isUpdatePending();

    return false;
}

void ShapeManagerImpl::enterAnimationMode( const AnimatableShapeSharedPtr& rShape )
{
    if( mbEnabled)
    {
        ENSURE_OR_THROW( rShape, "ShapeManagerImpl::enterAnimationMode(): invalid Shape" );
        rShape->enterAnimationMode();
    }
}

void ShapeManagerImpl::leaveAnimationMode( const AnimatableShapeSharedPtr& rShape )
{
    if( mbEnabled){
        ENSURE_OR_THROW( rShape, "ShapeManagerImpl::leaveAnimationMode(): invalid Shape" );
        rShape->leaveAnimationMode();
    }
}

void ShapeManagerImpl::notifyShapeUpdate( const ShapeSharedPtr& rShape )
{
    if( rShape->isVisible() || rShape->isBackgroundDetached() )
          maUpdateShapes.insert( rShape );
}

bool ShapeManagerImpl::updateSprites()
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

        ++aCurrShape;
    }

    maUpdateShapes.clear();

    return bRet;
}

ShapeSharedPtr ShapeManagerImpl::lookupShape( uno::Reference< drawing::XShape > const & xShape ) const
{
    ENSURE_OR_THROW( xShape.is(), "LayerManager::lookupShape(): invalid Shape" );

    const XShapeHash::const_iterator aIter( maXShapeHash.find( xShape ));
    if( aIter == maXShapeHash.end() )
       return ShapeSharedPtr(); // not found

    // found, return data part of entry pair.
    return aIter->second;
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
    AttributableShapeSharedPtr pSubset;

    // shape already added?
    if( rOrigShape->createSubset( pSubset,
                                  rTreeNode ) )
    {
        OSL_ENSURE( pSubset, "ShapeManagerImpl::getSubsetShape(): failed to create subset" );

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

void ShapeManagerImpl::revokeSubset( const AttributableShapeSharedPtr& rOrigShape,
                                     const AttributableShapeSharedPtr& rSubsetShape )
{
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

    // is this one of our shapes? other shapes are ignored.
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

void ShapeManagerImpl::implAddShape( const ShapeSharedPtr& rShape )
{
    ENSURE_OR_THROW( rShape, "ShapeManagerImpl::implAddShape(): invalid Shape" );

    LayerShapeMap::value_type aValue (rShape, LayerWeakPtr());

    OSL_ASSERT( maAllShapes.find(rShape) == maAllShapes.end() ); // shape must not be added already

    maAllShapes.insert(aValue);

    // update shape, it's just added and not yet painted
    if( rShape->isVisible() )
        notifyShapeUpdate( rShape );
}

void ShapeManagerImpl::addShape( const ShapeSharedPtr& rShape )
{
    ENSURE_OR_THROW( rShape, "ShapeManagerImpl::addShape(): invalid Shape" );

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

void ShapeManagerImpl::implRemoveShape( const ShapeSharedPtr& rShape )
{
    ENSURE_OR_THROW( rShape, "ShapeManagerImpl::implRemoveShape(): invalid Shape" );

    const LayerShapeMap::iterator aShapeEntry( maAllShapes.find(rShape) );

    if( aShapeEntry == maAllShapes.end() )
        return;
    // Enter shape area to the update area, but only if shape
    // is visible and not in sprite mode (otherwise, updating
    // the area doesn't do actual harm, but costs time)
    // Actually, also add it if it was listed in
    // maUpdateShapes (might have just gone invisible).
    rShape->clearAllViewLayers();
    maAllShapes.erase( aShapeEntry );
}

bool ShapeManagerImpl::listenerRemoved(
    const uno::Reference<presentation::XShapeEventListener>& /*xListener*/,
    const uno::Reference<drawing::XShape>&                   xShape )
{
    // shape really erased from map? maybe there are other listeners
    // for the same shape pending...
    if( mrGlobalListenersMap.find(xShape) == mrGlobalListenersMap.end() )
    {
        // is this one of our shapes? other shapes are ignored.
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

    // is this one of our shapes? other shapes are ignored.
    if( !pShape )
        return false;

    if( mrGlobalCursorMap.find(xShape) == mrGlobalCursorMap.end() )
    {
        // erased from global map - erase locally, too
        maShapeCursorMap.erase(pShape);
    }
    else
    {
        // included in global map - update local one
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
    // find matching region (scan reversely, to coarsely match
    // paint order): set is ordered by priority
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



} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
